#include "graph.hpp"
#include <stdexcept> // For exceptions

// Constructor
Graph::Graph(int vertices) : vertexCount(vertices), edgeCount(0) {
    // Initialize the adjacency matrix with zeros
    adjMatrix.resize(vertices, vector<int>(vertices, 0));
}

// Function to add an edge between vertices u and v with a given weight
void Graph::addEdge(int u, int v, int weight) {
    if (u < 0 || u >= vertexCount || v < 0 || v >= vertexCount) {
        throw std::out_of_range("Vertex index out of range");
    }
    if (weight <= 0) {
        throw std::invalid_argument("Weight must be positive");
    }
    // Add the weight to the adjacency matrix
    if (adjMatrix[u][v] == 0) {
        edgeCount++; // Only increment edge count if the edge is new
    }
    adjMatrix[u][v] = weight;
    adjMatrix[v][u] = weight; // For undirected graph
}

// Function to remove an edge between vertices u and v
void Graph::removeEdge(int u, int v) {
    if (u < 0 || u >= vertexCount || v < 0 || v >= vertexCount) {
        throw std::out_of_range("Vertex index out of range");
    }
    if (adjMatrix[u][v] != 0) {
        adjMatrix[u][v] = 0;
        adjMatrix[v][u] = 0; // For undirected graph
        edgeCount--; // Decrement edge count
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

// Getter for the adjacency matrix
vector<vector<int>> Graph::getGraph() const {
    return adjMatrix;
}
