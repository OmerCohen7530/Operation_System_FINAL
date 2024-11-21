#include "mst.hpp"
#include "prim.hpp"      // Include the Prim's algorithm header
#include "boruvka.hpp"    // Include the Boruvka's algorithm header
#include <limits>
#include <queue>
#include <string>
#include <iostream>

// Constructor
MST::MST(const std::vector<std::vector<int>>& graph, int n, const std::string& algo) : graph(graph), numVertices(n) 
{
    if (algo == "prim") {
        calculateMSTUsingPrim();
    } else if (algo == "boruvka") {
        calculateMSTUsingBoruvka();
    }
}

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

// Helper function to convert graph representation to edges
std::vector<std::tuple<int, int, int, int>> MST::convertGraphToEdges() {
    std::vector<std::tuple<int, int, int, int>> edges;
    for (int u = 0; u < numVertices; ++u) {
        for (int v = u + 1; v < numVertices; ++v) { // Avoid duplicate edges
            if (graph[u][v] > 0) { // Only consider edges with positive weight
                edges.emplace_back(u, v, graph[u][v], edges.size());
            }
        }
    }
    return edges;
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
        
        for (int neighbor = 0; neighbor < numVertices; ++neighbor) {
            if (graph[current][neighbor] > 0 && dist[neighbor] == -std::numeric_limits<int>::max()) {
                dist[neighbor] = dist[current] + graph[current][neighbor];
                q.push(neighbor);
            }
        }
    }

    return dist[v] == -std::numeric_limits<int>::max() ? -1 : dist[v];
}

// Function to calculate the average edge count in all paths between two vertices u and v
double MST::getAverageEdgeCount() {
    // מספר הקודקודים בגרף
    const int INF = std::numeric_limits<int>::max();
    int totalDistance = 0;
    int pairCount = 0;

    // נבצע אלגוריתם פלויד-וורשל למציאת המרחקים הקצרים ביותר
    std::vector<std::vector<int>> shortestPaths = graph;

    for (int k = 0; k < numVertices; ++k) {
        for (int i = 0; i < numVertices; ++i) {
            for (int j = 0; j < numVertices; ++j) {
                if (shortestPaths[i][k] < INF && shortestPaths[k][j] < INF) {
                    shortestPaths[i][j] = std::min(shortestPaths[i][j], shortestPaths[i][k] + shortestPaths[k][j]);
                }
            }
        }
    }

    // מחשבים את סכום המרחקים הקצרים ביותר בין כל זוג קודקודים
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            if (shortestPaths[i][j] < INF) {
                totalDistance += shortestPaths[i][j];
                ++pairCount;
            }
        }
    }

    // מחזירים את הממוצע
    return pairCount > 0 ? static_cast<double>(totalDistance) / pairCount : 0.0;
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

        for (int neighbor = 0; neighbor < numVertices; ++neighbor) {
            if (graph[current][neighbor] > 0 && dist[neighbor] == std::numeric_limits<int>::max()) {
                dist[neighbor] = dist[current] + graph[current][neighbor];
                q.push(neighbor);
            }
        }
    }

    return dist[v] == std::numeric_limits<int>::max() ? -1 : dist[v];
}
