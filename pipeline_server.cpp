#include <iostream>           
#include <thread>            
#include <queue>              
#include <mutex> 
#include <condition_variable> 
#include <sys/socket.h>   
#include <netinet/in.h>    
#include <unistd.h>          
#include <cstring>      
#include <sstream>       
#include <vector>          
#include "graph.hpp"       
#include "mst.hpp"          
#include <csignal>
#include <functional>

#define PORT 8074 // Defines the port number on which the server will listen for client connections
bool close_server=false;
/**
 * Class: ActiveObject
 * Implements the Active Object design pattern. This class encapsulates an asynchronous task execution model,
 * where tasks (functions) are posted to an internal queue, and a dedicated worker thread processes each task
 * in sequence. This enables asynchronous processing.
 */
class ActiveObject
{
private:
    std::thread worker;                      // Worker thread that processes the tasks
    std::queue<std::function<void()>> tasks; // Queue of tasks to be executed
    std::mutex mutex;                        // Mutex to protect access to the task queue
    std::condition_variable cv;              // Condition variable to signal the worker thread when tasks are available
    bool running = true;                     // Indicates whether the worker thread should continue running

public:
    
    /**
     * Constructor: Starts the worker thread.
     * The worker thread runs in an infinite loop, waiting for tasks to be posted in the queue.
     */
    ActiveObject()
    {
        worker = std::thread([this]()
                             {
                try {
                    while (running) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex);
                            cv.wait(lock, [this]() { return !tasks.empty() || !running; });
                            if (!running && tasks.empty()) {
                                // std::cout << "Worker exiting: No tasks and stopped." << std::endl;
                                return;
                            }
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        std::cout << "Executing task..." << std::endl;
                        task();  // Execute the task
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Exception in ActiveObject worker thread: " << e.what() << std::endl;
                } });
    }

    /**
     * Function: post
     * Adds a new task to the queue and notifies the worker thread to process it.
     *
     * @param task A function (lambda or otherwise) to be executed by the ActiveObject.
     */
    void post(std::function<void()> task)
    {
        {
        std::unique_lock<std::mutex> lock(mutex);
        // std::cout << "Adding task to queue..." << std::endl;
        tasks.push(task);
        std::cout << "Task added to queue. Queue size: " << tasks.size() << std::endl;
        }
        cv.notify_one();
    }

    /**
     * Function: stop
     * Stops the worker thread by setting running to false and waking up the thread if it's waiting.
     */
    void stop()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            running = false;
            cv.notify_all();
        }
        if (worker.joinable())
        {
            worker.join();
        }
    }
    ~ActiveObject()
    {
        stop();
    }
};

Graph build_graph(int newSocket)
{
    std::string response = "----------Graph creation----------\nEnter the number of vertices: ";
    send(newSocket, response.c_str(), response.size(), 0);

    char answer[1024] = {0};
    read(newSocket, answer, 1024);
    int numVertices = std::stoi(answer);

    // Create a new graph with the given number of vertices
    Graph graph = Graph(numVertices); 

    response = "Enter the number of edges: ";
    send(newSocket, response.c_str(), response.size(), 0);

    answer[1024] = {0};
    read(newSocket, answer, 1024);
    int numEdges = std::stoi(answer);

    // Add edges to the graph
    for (int i = 0; i < numEdges; ++i)
    {
        response = "Enter an edge (from, to, weight): ";
        send(newSocket, response.c_str(), response.size(), 0);

        int from, to, weight;
        char edgeBuffer[1024] = {0};
        read(newSocket, edgeBuffer, 1024);
        std::istringstream edgeStream(edgeBuffer);
        edgeStream >> from >> to >> weight;
        graph.addEdge(from, to, weight);
        std::string response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) +" with weight " + std::to_string(weight) + " added successfully!\n";
        send(newSocket, response.c_str(), response.size(), 0);
    }
    response = "New graph created!\n";
    send(newSocket, response.c_str(), response.size(), 0);

    return graph;
}

MST build_mst(Graph graph, int newSocket)
{
    std::string response = "----------MST creation----------\nEnter the algorithm of MST (prim or boruvka): ";
    send(newSocket, response.c_str(), response.size(), 0);

    char algoBuffer[1024] = {0};
    read(newSocket, algoBuffer, 1024);
    std::string algo(algoBuffer);

    // if (algo != "prim" && algo != "boruvka") so make it prim
    if (algo != "prim" && algo != "boruvka") {
        algo = "prim";
    }

    // Trim whitespace and newline characters
    // algo.erase(algo.find_last_not_of(" \t\n\r") + 1);

    MST mst = MST(graph.getGraph(), graph.getVertexCount(), algo); // Create the MST
    response = "MST created using " + algo + " algorithm\n";
    send(newSocket, response.c_str(), response.size(), 0);

    return mst;
}

void analyze_data(MST mst, int newSocket)
{
    std::stringstream ss;

    ss << "----------analyze_data----------\n";
    ss << "Total Weight:  " << mst.getTotalWeight() << "\n";
    ss << "Longest Distance (e.g. 0->1):  " << mst.getLongestDistance(0, 1) << "\n";
    ss << "Shortest Distance (e.g. 0->1):  " << mst.getShortestDistance(0, 1) << "\n";
    ss << "Average Edge Count:  " << mst.getAverageEdgeCount() << "\n";

    send(newSocket, (ss.str()).c_str(), (ss.str()).size(), 0);
}

void handleClientPipeline(int newSocket)
{
    // Stage 1: Graph creation
    // Stage 2: MST creation
    // Stage 3: Analyze data
    ActiveObject stage1, stage2, stage3;  // ActiveObject instances to handle stages of the pipeline

    std::cout << "Pipeline started for client..." << std::endl;

    // Condition variable for synchronization
    std::mutex mutex;
    std::condition_variable cv_2;
    bool stage1Done = false;
    bool stage2Done = false;
    bool stage3Done = false;

    // Stage 1: Build graph
    stage1.post([&stage2, &stage3, &cv_2, &mutex, &stage1Done, &stage2Done, &stage3Done, newSocket]() {
        Graph graph = build_graph(newSocket);

        // Notify Stage 1
        {
            std::lock_guard<std::mutex> lock(mutex);
            stage1Done = true;
        }
        cv_2.notify_one();

        // Pass the result to the next stage
        stage2.post([&stage3, &cv_2, &mutex, &stage2Done, &stage3Done, graph, newSocket]() {
            MST mst = build_mst(graph, newSocket);

            // Notify Stage 2
            {
                std::lock_guard<std::mutex> lock(mutex);
                stage2Done = true;
            }
            cv_2.notify_one();

            std::shared_ptr<MST> mstPtr = std::make_shared<MST>(mst);

            // Pass the result to the final stage
            stage3.post([&cv_2, &mutex, &stage3Done, mstPtr, newSocket]() {
                std::cout << "Analyzing data 2..." << std::endl;
                analyze_data(*mstPtr, newSocket);

                // Notify Stage 3
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    stage3Done = true;
                }
                cv_2.notify_one();
            });
        });
    });

    // Wait for Stage 1 to finish
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv_2.wait(lock, [&stage1Done]() { return stage1Done; });
    }

    // Wait for Stage 2 to finish
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv_2.wait(lock, [&stage2Done]() { return stage2Done; });
    }

    // Wait for Stage 3 to finish
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv_2.wait(lock, [&stage3Done]() { return stage3Done; });
    }

    std::this_thread::sleep_for(std::chrono::seconds(4)); 
    close(newSocket);
}

int main()
{
    
    int serverFd, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    // Create socket
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Allow port reuse
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt failed" << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(serverFd, 3) < 0)
    {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }


    std::cout << "Server is running. Waiting for clients..." << std::endl;

    // Accept clients and handle them
    while (true) {
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        }

        std::cout << "Client connected! Starting the pipeline..." << std::endl;
        handleClientPipeline(newSocket);
        // std::thread clientThread(handleClientPipeline, newSocket);
        // clientThread.detach();
    }

    close(serverFd);
    return 0;
}