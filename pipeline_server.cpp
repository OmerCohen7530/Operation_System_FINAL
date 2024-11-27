#include <iostream>           // For standard I/O operations
#include <thread>             // For creating and managing threads
#include <queue>              // For task queues used in ActiveObject
#include <mutex>              // For synchronizing access to shared resources
#include <condition_variable> // For thread synchronization
#include <sys/socket.h>       // For socket programming
#include <netinet/in.h>       // For structures and constants related to internet addresses
#include <unistd.h>           // For close() and other POSIX APIs
#include <cstring>            // For string manipulation and error handling (e.g., strerror)
#include <sstream>            // For string stream manipulation (menu building, parsing)
#include <vector>             // For handling dynamic arrays (adjacency matrices)
#include "graph.hpp"          // Includes custom Graph class
#include "mst.hpp"            // Includes custom MST class
#include <csignal>
#include <functional>

#define PORT 8092 // Defines the port number on which the server will listen for client connections
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
                            if (!running){
                                // std::cout << "Worker exiting: No tasks and stopped." << std::endl;
                                return;
                            }
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        // std::cout << "Executing task..." << std::endl;
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
        tasks.push(task);
        // std::cout << "Task added to queue. Queue size: " << tasks.size() << std::endl;
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


    std::string menu()
{
    std::stringstream ss;
    ss << "Menu:\n";
    ss << "0. Close server\n";
    ss << "1. Create a new graph\n";
    ss << "2. Add an edge\n";
    ss << "3. Remove an edge\n";
    ss << "4. Build MST by prim or boruvka\n";
    ss << "5. Get total weight of MST\n";
    ss << "6. Get longest distance in MST\n";
    ss << "7. Get shortest distance in MST\n";
    ss << "8. Get average distance between two vertices in MST\n";
    ss << "9. Exit\n";
    return ss.str();
}

bool check_is_valid_operation(int newSocket, bool graphExists, bool mstCreated){
    //check if the graph is created, if not return response to create graph first
    if (!graphExists)
    {
        std::string response = "Create a graph first!\n";
        send(newSocket, response.c_str(), response.size(), 0);
        return false;
    }
    // check if the MST is created, if not return response to create MST first
    if (!mstCreated)
    {
        std::string response = "Create an MST first!\n";
        send(newSocket, response.c_str(), response.size(), 0);
        return false;
    }
    return true;
}

void handleClientPipeline(int newSocket)
{
    // Stage 1: Requests to create, add, or remove edges
    // Stage 2: Requests to calculate the MST weight
    // Stage 3: Request to print all calculated values 
    ActiveObject stage1, stage2, stage3;  // ActiveObject instances to handle stages of the pipeline
    static Graph graph = Graph(0); // Empty graph
    static MST mst(graph.getGraph(), graph.getVertexCount()); // empty MST
    static bool mstCreated = false;                    // Tracks whether the MST has been created
    static bool graphExists = false;                          // Tracks whether a graph has been created

    while (true)
    {
        // Send the menu to the client
        std::string welcome = menu();
        send(newSocket, welcome.c_str(), welcome.size(), 0);

        // Read client input
        char buffer[1024] = {0};
        ssize_t bytes_read = read(newSocket, buffer, 1024);
        if (bytes_read <= 0)
        {
            std::cerr << "Client disconnected or error: " << strerror(errno) << std::endl;
            close(newSocket);
            return;
        }

        std::string input(buffer);
        std::istringstream iss(input);
        int choice;
        iss >> choice;

        switch (choice)
        { 
        case 0:
        {//close server
            close(newSocket);
            close_server=true;
            return;
        } 
        case 1:
        { // Create a new graph
            std::string response = "Enter the number of vertices: ";
            send(newSocket, response.c_str(), response.size(), 0);

            char answer[1024] = {0};
            read(newSocket, answer, 1024);
            int numVertices = std::stoi(answer);

            // Create a new graph with the given number of vertices
            graph = Graph(numVertices); 

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
                
                // Add the edge to the graph
                stage1.post([&]()
                            {
                            graph.addEdge(from, to, weight);
                            std::string response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) +" with weight " + std::to_string(weight) + " added successfully!\n";
                            send(newSocket, response.c_str(), response.size(), 0); });
                            

            }

            mstCreated = false;        // Reset the MST flag
            graphExists = true;        // Mark that the graph exists
            response = "New graph created!\n";
            send(newSocket, response.c_str(), response.size(), 0);
            break;
        }
        case 2:
        { // Add an edge
            std::string response = "Enter an edge (from, to, weight): ";
            send(newSocket, response.c_str(), response.size(), 0);

            int from, to, weight;
            char edgeBuffer[1024] = {0};
            read(newSocket, edgeBuffer, 1024);
            std::istringstream edgeStream(edgeBuffer);
            edgeStream >> from >> to >> weight;

            stage1.post([&]()
                        {
                        graph.addEdge(from, to, weight);
                        std::string response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) +" with weight " + std::to_string(weight) + " added successfully!\n";
                        mstCreated = false; // Reset MST flag
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 3:
        { // Remove an edge
            std::string response = "Enter an edge to remove (from, to): ";
            send(newSocket, response.c_str(), response.size(), 0);

            int from, to, weight;
            char edgeBuffer[1024] = {0};
            read(newSocket, edgeBuffer, 1024);
            std::istringstream edgeStream(edgeBuffer);
            edgeStream >> from >> to >> weight;

            stage1.post([&]()
                        {
                        graph.removeEdge(from, to);
                        std::string response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) +" removed successfully!\n";
                        mstCreated = false; // Reset MST flag
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        // hoose algorithm of MST
        case 4:
        {
            //check if the graph is created, if not return response to create graph first
            if (!graphExists)
            {
                std::string response = "Create a graph first!\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            std::string response = "Enter the algorithm of MST (prim or boruvka): ";
            send(newSocket, response.c_str(), response.size(), 0);

            char algoBuffer[1024] = {0};
            read(newSocket, algoBuffer, 1024);
            std::string algo(algoBuffer);

            // Trim whitespace and newline characters
            algo.erase(algo.find_last_not_of(" \t\n\r") + 1);

            // check if the algorithm is prim or boruvka
            if (algo != "prim" && algo != "boruvka") {
                std::string response = "Invalid algorithm. Please try again.\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }

            stage2.post([&]()
                        {
                        mst = MST(graph.getGraph(), graph.getVertexCount(), algo); // Create the MST
                        mstCreated = true;
                        std::string response = "MST created using " + algo + " algorithm\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 5:
        { // Get MST weight

            if(!check_is_valid_operation(newSocket, graphExists, mstCreated))break;

            stage2.post([&]()
                        {
                        int weight = mst.getTotalWeight();
                        std::string response = "Total weight of MST: " + std::to_string(weight) + "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 6:
        { // Get the longest distance in the MST

            if(!check_is_valid_operation(newSocket, graphExists, mstCreated))break;

            std::string response = "Provide the start and end vertices for the longest path: ";
            send(newSocket, response.c_str(), response.size(), 0);

            char pathBuffer[1024] = {0};
            read(newSocket, pathBuffer, 1024);
            std::istringstream pathStream(pathBuffer);
            int from, to;
            pathStream >> from >> to;

            stage3.post([&]()
                        {
                        int path = mst.getLongestDistance(from, to);
                        std::string response = "Longest distance from " + std::to_string(from) + " to " + std::to_string(to) + ": " + std::to_string(path) + "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;

        }
        case 7:
        { // Get the shortest distance in the MST

            if(!check_is_valid_operation(newSocket, graphExists, mstCreated))break;

            std::string response = "Provide the start and end vertices for the shortest path: ";
            send(newSocket, response.c_str(), response.size(), 0);

            char pathBuffer[1024] = {0};
            read(newSocket, pathBuffer, 1024);
            std::istringstream pathStream(pathBuffer);
            int from, to;
            pathStream >> from >> to;

            stage3.post([&]()
                        {
                        int path = mst.getShortestDistance(from, to);
                        std::string response = "shortest distance from " + std::to_string(from) + " to " + std::to_string(to) + ": " + std::to_string(path) + "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 8:
        { // Get the average distance in the MST

            if(!check_is_valid_operation(newSocket, graphExists, mstCreated))break;

            stage3.post([&]()
                        {
                        int avg = mst.getAverageEdgeCount();
                        std::string response = "Average distance in MST: " + std::to_string(avg) + "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 9:
        { // Exit the program
            close(newSocket);
            return;
        }
        default: // Invalid choice
            std::string response = "Invalid choice. Please try again.\n";
            send(newSocket, response.c_str(), response.size(), 0);
            break;
        }
    }
}

/**
 * Main Function: Initializes the server and listens for client connections.
 * The server uses a socket to accept connections and handle multiple clients in parallel.
 */
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
// ********************************************************************************************************************
    // // Create an empty graph with an empty adjacency matrix
    // Graph graph{std::vector<std::vector<int>>{}}; // Empty graph

    std::cout << "Server is running. Waiting for clients..." << std::endl;

    // Accept clients and handle them
    while ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
    {
        std::cout << "Accepted new client" << std::endl;
        handleClientPipeline(newSocket); // Handle client requests
        if(close_server)
        {
            close(serverFd);
            return 0;
        }
    }

    if (newSocket < 0)
    {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
    }

    close(serverFd);
    return 0;
}