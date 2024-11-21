#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>

using std::vector;

class Graph {
public:
    // Constructor
    Graph(int vertices);
    // empty costructor
    Graph();

    // Functions to add and remove edges
    void addEdge(int u, int v, int weight);
    void removeEdge(int u, int v);

    // Getters
    int getVertexCount() const;
    int getEdgeCount() const;
    vector<vector<int>> getGraph() const;

private:
    int vertexCount;
    int edgeCount;
    vector<vector<int>> adjMatrix; // Adjacency matrix to store weights of edges
};

#endif // GRAPH_HPP
