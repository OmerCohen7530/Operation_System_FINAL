#include "mst.hpp"
#include "prim.hpp"      // Include the Prim's algorithm header
#include "boruvka.hpp"    // Include the Boruvka's algorithm header
#include <limits>
#include <queue>

// Constructor
MST::MST(const std::vector<std::vector<int>>& graph, int n)
    : graph(graph), numVertices(n) {}

// Function to calculate MST using Prim's algorithm
void MST::calculateMSTUsingPrim() {
    mstEdges = prim(convertGraphToEdges(), numVertices);
}

// Public function to retrieve MST edges using Prim's algorithm
std::vector<std::tuple<int, int, int, int>> MST::primMST() {
    calculateMSTUsingPrim();
    return mstEdges;
}

// Function to calculate MST using Boruvka's algorithm
void MST::calculateMSTUsingBoruvka() {
    mstEdges = boruvka(convertGraphToEdges(), numVertices);
}

// Public function to retrieve MST edges using Boruvka's algorithm
std::vector<std::tuple<int, int, int, int>> MST::boruvkaMST() {
    calculateMSTUsingBoruvka();
    return mstEdges;
}

// Function to get the total weight of the MST
int MST::getTotalWeight() {
    int totalWeight = 0;
    for (const auto& edge : mstEdges) {
        totalWeight += std::get<2>(edge); // Assuming weight is at position 2 in tuple
    }
    return totalWeight;
}

// Function to find the longest distance between two vertices u and v in the MST
int MST::getLongestDistance(int u, int v) {
    std::vector<int> dist(numVertices, -std::numeric_limits<int>::max());
    std::queue<int> q;
    dist[u] = 0;
    q.push(u);

    while (!q.empty()) {
        int current = q.front();
        q.pop();
        
        for (int neighbor : graph[current]) {
            if (dist[neighbor] == -std::numeric_limits<int>::max()) {
                dist[neighbor] = dist[current] + 1;
                q.push(neighbor);
            }
        }
    }

    return dist[v] == -std::numeric_limits<int>::max() ? -1 : dist[v];
}

// Function to calculate the average edge count in all paths between two vertices u and v
double MST::getAverageEdgeCount(int u, int v) {
    int totalEdges = 0;
    int pathsCount = 0;
    
    std::vector<int> dist(numVertices, std::numeric_limits<int>::max());
    dist[u] = 0;

    std::queue<int> q;
    q.push(u);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        for (int neighbor : graph[current]) {
            if (dist[neighbor] == std::numeric_limits<int>::max()) {
                dist[neighbor] = dist[current] + 1;
                q.push(neighbor);

                if (neighbor == v) {
                    totalEdges += dist[neighbor];
                    pathsCount++;
                }
            }
        }
    }

    return pathsCount > 0 ? static_cast<double>(totalEdges) / pathsCount : 0.0;
}

// Function to find the shortest distance between two vertices u and v in the MST
int MST::getShortestDistance(int u, int v) {
    std::vector<int> dist(numVertices, std::numeric_limits<int>::max());
    std::queue<int> q;

    dist[u] = 0;
    q.push(u);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        for (int neighbor : graph[current]) {
            if (dist[neighbor] == std::numeric_limits<int>::max()) {
                dist[neighbor] = dist[current] + 1;
                q.push(neighbor);
            }
        }
    }

    return dist[v] == std::numeric_limits<int>::max() ? -1 : dist[v];
}

// Helper function to convert graph representation to edges for MST algorithms
std::vector<std::tuple<int, int, int, int>> MST::convertGraphToEdges() {
    std::vector<std::tuple<int, int, int, int>> edges;
    for (int u = 0; u < numVertices; ++u) {
        for (int v : graph[u]) {
            if (u < v) { // Avoid duplicate edges
                edges.emplace_back(u, v, 1, edges.size()); // Assuming weight of 1 for simplicity
            }
        }
    }
    return edges;
}
