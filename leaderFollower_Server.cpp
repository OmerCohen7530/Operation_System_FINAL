#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <functional>
#include "graph.hpp"
#include "mst.hpp"
#include <csignal>

#define PORT 8094
#define THREAD_POOL_SIZE 4

bool close_server = false;

class LeaderFollowerServer {
private:
    struct Task {
        int newSocket;
    };

    std::vector<std::thread> workers;      
    std::queue<Task> tasks;                
    std::mutex queueMutex;               
    std::condition_variable cv;         
    bool stopFlag;                      
         
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

    void processClient(int newSocket) {
        Graph graph = build_graph(newSocket);
        MST mst = build_mst(newSocket, newSocket);
        analyze_data(mst, newSocket);
        close(newSocket);
    }

    void workerLoop() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this]() { return !tasks.empty() || stopFlag; });
                if (stopFlag && tasks.empty()) return;
                task = tasks.front();
                tasks.pop();
            }
            processClient(task.newSocket);
        }
    }

public:
    LeaderFollowerServer(size_t poolSize) : stopFlag(false) {                  
        // this for loop is for creating the threads
        for (size_t i = 0; i < poolSize; ++i) {
            // create a new thread and push it to the workers vector
            // each thread will run the workerLoop function forever and search for tasks
            workers.emplace_back([this]() { this->workerLoop(); });
        }
    }

    ~LeaderFollowerServer() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        cv.notify_all();
        for (auto& worker : workers) {
            worker.join();
        }
    }

    void addTask(int newSocket) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(Task{newSocket});
        }
        cv.notify_one();
    }
};

int main() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed\n";
        close(serverFd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        close(serverFd);
        return -1;
    }

    if (listen(serverFd, 3) < 0) {
        std::cerr << "Listen failed\n";
        close(serverFd);
        return -1;
    }

    // Create the server with a thread pool size of 4
    LeaderFollowerServer server(THREAD_POOL_SIZE);
    std::cout << "Server running...\n";

    while ((newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
        server.addTask(newSocket);
        if (close_server) {
            close(serverFd);
            return 0;
        }
    }

    close(serverFd);
    return 0;
}
