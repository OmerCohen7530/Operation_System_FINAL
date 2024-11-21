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

    std::vector<std::thread> workers;      // בריכת התהליכונים
    std::queue<Task> tasks;                // תור משימות
    std::mutex queueMutex;                 // לנעילה של התור
    std::condition_variable cv;            // לתקשורת בין התהליכונים
    bool stopFlag;                         // דגל לסיום הבריכה

    Graph graph;                           // הגרף הראשי
    MST mst;                               // ה-MST הראשי
    bool graphExists = false;              // האם הגרף נוצר?
    bool mstCreated = false;               // האם ה-MST נוצר?

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

    void processClient(int newSocket) {
        while (true) {
            // שליחת תפריט ללקוח
            std::string welcome = menu();
            send(newSocket, welcome.c_str(), welcome.size(), 0);

            // קריאת הבחירה של הלקוח
            char buffer[1024] = {0};
            ssize_t bytesRead = read(newSocket, buffer, 1024);
            if (bytesRead <= 0) {
                std::cerr << "Client disconnected\n";
                close(newSocket);
                return;
            }

            std::istringstream iss(buffer);
            int choice;
            iss >> choice;

            switch (choice)
            { 
            case 0:
            { // Close server
                close(newSocket);
                close_server = true;
                return;
            }
            case 1:
            { // Create a new graph
                std::string response = "Enter the number of vertices: ";
                send(newSocket, response.c_str(), response.size(), 0);

                char answer[1024] = {0};
                read(newSocket, answer, 1024);
                int numVertices = std::stoi(answer);

                graph = Graph(numVertices);

                response = "Enter the number of edges: ";
                send(newSocket, response.c_str(), response.size(), 0);

                answer[1024] = {0};
                read(newSocket, answer, 1024);
                int numEdges = std::stoi(answer);

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
                    response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) +
                            " with weight " + std::to_string(weight) + " added successfully!\n";
                    send(newSocket, response.c_str(), response.size(), 0);
                }

                mstCreated = false;
                graphExists = true;
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

                graph.addEdge(from, to, weight);
                response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) +
                        " with weight " + std::to_string(weight) + " added successfully!\n";
                mstCreated = false;
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 3:
            { // Remove an edge
                std::string response = "Enter an edge to remove (from, to): ";
                send(newSocket, response.c_str(), response.size(), 0);

                int from, to;
                char edgeBuffer[1024] = {0};
                read(newSocket, edgeBuffer, 1024);
                std::istringstream edgeStream(edgeBuffer);
                edgeStream >> from >> to;

                graph.removeEdge(from, to);
                response = "Edge from " + std::to_string(from) + " -> " + std::to_string(to) + " removed successfully!\n";
                mstCreated = false;
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 4:
            { // Build MST by prim or boruvka
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

                algo.erase(algo.find_last_not_of(" \t\n\r") + 1);

                if (algo != "prim" && algo != "boruvka")
                {
                    response = "Invalid algorithm. Please try again.\n";
                    send(newSocket, response.c_str(), response.size(), 0);
                    break;
                }

                mst = MST(graph.getGraph(), graph.getVertexCount(), algo);
                mstCreated = true;
                response = "MST created using " + algo + " algorithm\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 5:
            { // Get MST weight
                if (!check_is_valid_operation(newSocket, graphExists, mstCreated))
                    break;

                int weight = mst.getTotalWeight();
                std::string response = "Total weight of MST: " + std::to_string(weight) + "\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 6:
            { // Get the longest distance in the MST
                if (!check_is_valid_operation(newSocket, graphExists, mstCreated))
                    break;

                std::string response = "Provide the start and end vertices for the longest path: ";
                send(newSocket, response.c_str(), response.size(), 0);

                char pathBuffer[1024] = {0};
                read(newSocket, pathBuffer, 1024);
                std::istringstream pathStream(pathBuffer);
                int from, to;
                pathStream >> from >> to;

                int path = mst.getLongestDistance(from, to);
                response = "Longest distance from " + std::to_string(from) + " to " + std::to_string(to) +
                        ": " + std::to_string(path) + "\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 7:
            { // Get the shortest distance in the MST
                if (!check_is_valid_operation(newSocket, graphExists, mstCreated))
                    break;

                std::string response = "Provide the start and end vertices for the shortest path: ";
                send(newSocket, response.c_str(), response.size(), 0);

                char pathBuffer[1024] = {0};
                read(newSocket, pathBuffer, 1024);
                std::istringstream pathStream(pathBuffer);
                int from, to;
                pathStream >> from >> to;

                int path = mst.getShortestDistance(from, to);
                response = "Shortest distance from " + std::to_string(from) + " to " + std::to_string(to) +
                        ": " + std::to_string(path) + "\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 8:
            { // Get the average distance in the MST
                if (!check_is_valid_operation(newSocket, graphExists, mstCreated))
                    break;

                int avg = mst.getAverageEdgeCount();
                std::string response = "Average distance in MST: " + std::to_string(avg) + "\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 9:
            { // Exit the program
                close(newSocket);
                return;
            }
            default:
            { // Invalid choice
                std::string response = "Invalid choice. Please try again.\n";
                send(newSocket, response.c_str(), response.size(), 0);
                break;
            }
            }
        }
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
        // graph = Graph(0);                         
        // mst = mst(graph.getGraph(), graph.getVertexCount(),"prim");    
        for (size_t i = 0; i < poolSize; ++i) {
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
