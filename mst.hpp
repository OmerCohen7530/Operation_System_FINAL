#ifndef MST_HPP
#define MST_HPP

#include <vector>
#include <tuple>
#include <string>

class MST {
public:
    // Constructor
    MST(const std::vector<std::vector<int>>& graph, int n, const std::string& algo);
    // Constructor without algorithm
    MST(const std::vector<std::vector<int>>& graph, int n): graph(graph), numVertices(n) {}
    MST() : graph(), numVertices(0) {}


    // MST calculation functions
    std::vector<std::tuple<int, int, int, int>> boruvkaMST();
    std::vector<std::tuple<int, int, int, int>> primMST();

    // Analysis functions
    int getTotalWeight();
    int getLongestDistance(int u, int v);        // Longest distance between two vertices u and v
    double getAverageEdgeCount();    // Average between all pairs of vertices
    int getShortestDistance(int u, int v);       // Shortest distance between two vertices u and v

private:
    int numVertices;
    std::vector<std::vector<int>> graph;         // Graph representation
    std::vector<std::tuple<int, int, int, int>> mstEdges; // Holds the MST edges

    // Helper functions
    void calculateMSTUsingPrim();
    void calculateMSTUsingBoruvka();
    std::vector<std::tuple<int, int, int, int>> convertGraphToEdges();
};

#endif // MST_HPP
