#include "graph.hpp"
#include <algorithm> // for std::find

// Constructor
Graph::Graph(int vertices) : vertexCount(vertices), edgeCount(0) {
    adjList.resize(vertices);
}

// Function to add an edge between vertices u and v
void Graph::addEdge(int u, int v) {
    if (u >= 0 && u < vertexCount && v >= 0 && v < vertexCount) {
        adjList[u].push_back(v);
        adjList[v].push_back(u); // Add edge in both directions
        ++edgeCount;
    }
}

// Function to remove an edge between vertices u and v
void Graph::removeEdge(int u, int v) {
    if (u >= 0 && u < vertexCount && v >= 0 && v < vertexCount) {
        auto itU = std::find(adjList[u].begin(), adjList[u].end(), v);
        if (itU != adjList[u].end()) {
            adjList[u].erase(itU);
        }

        auto itV = std::find(adjList[v].begin(), adjList[v].end(), u);
        if (itV != adjList[v].end()) {
            adjList[v].erase(itV);
        }
        
        --edgeCount;
    }
}

// Getter for vertex count
int Graph::getVertexCount() const {
    return vertexCount;
}

// Getter for edge count
int Graph::getEdgeCount() const {
    return edgeCount;
}

// Function to get the adjacency list representation of the graph
std::vector<std::vector<int>> Graph::getGraph() {
    return adjList;
}
