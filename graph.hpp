#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>

using std::vector;

class Graph {
public:
    // Constructor
    Graph(int vertices);

    // Functions to add and remove edges
    void addEdge(int u, int v);
    void removeEdge(int u, int v);

    // Getters
    int getVertexCount() const;
    int getEdgeCount() const;
    vector<vector<int>> getGraph();

private:
    int vertexCount;
    int edgeCount;
    std::vector<std::vector<int>> adjList; // Adjacency list using a simple 2D vector

};

#endif // GRAPH_HPP