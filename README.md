README for OS Project - Minimal Spanning Tree (MST)
Project Overview
This project focuses on implementing the Minimal Spanning Tree (MST) problem using weighted, directed graphs. It incorporates advanced concepts in operating systems, such as:

Design Patterns: Strategy, Factory.
Client-Server Architecture: Implemented with threads.
Concurrency: Thread pool with Leader-Follower model.
Pipeline Processing: Using Active Object for staged execution.
Memory Analysis: With Valgrind tools like memcheck, helgrind, and callgrind.
Code Coverage: Ensuring comprehensive testing.
The project provides functionality to calculate:

Total weight of the MST.
Longest distance between two vertices.
Average distance between edges.
Shortest distance between vertices (on the MST).
Features
Graph Data Structure: Custom implementation of a graph, supporting addition/removal of edges.
Factory Pattern: Supports different MST algorithms:
Borůvka
Prim
Kruskal
Tarjan
Server:
Handles client requests for MST-related operations.
Supports Leader-Follower Thread Pool and Pipeline Active Object for client handling.
Memory and Performance Analysis:
Valgrind for memory leaks, thread race detection, and performance profiling.
Code Coverage:
Ensures all functionalities are tested.
Files in the Project
File Name	Description
graph.hpp	Implementation of the graph data structure, including methods for adding and removing edges.
mst.hpp	Implementation of MST algorithms (Borůvka and Prim).
prim.hpp	Specific implementation of Prim's algorithm.
boruvka.hpp	Specific implementation of Borůvka's algorithm.
leaderFollower_Server.cpp	Leader-Follower Thread Pool implementation for handling client-server interactions.
pipeline_server.cpp	Pipeline implementation for staged client requests using the Active Object pattern.
makefile	Automates the build process, ensuring all dependencies are properly compiled.
Project Architecture
Factory Design for MST
The factory pattern supports switching between MST algorithms, enabling flexibility based on user requests (prim or boruvka).

Thread Pool (Leader-Follower)
Efficiently manages client requests using a fixed pool of threads. Requests are added to a queue and processed by worker threads.

Pipeline Processing (Active Object)
Encapsulates asynchronous task execution:

Stage 1: Processes requests to create or modify graphs.
Stage 2: Processes MST-related computations.
Stage 3: Outputs results to clients.
How to Build and Run
Building the Project:

Use the provided makefile:
bash
Copy code
make all
This will compile all source files and create the necessary binaries.
Running the Server:

To run the Leader-Follower server:
bash
Copy code
./leaderFollower_Server
To run the Pipeline server:
bash
Copy code
./pipeline_server
Connecting Clients:

Use any client capable of socket communication (e.g., Telnet or a custom client).
Connect to the server on the specified port (8094 for Leader-Follower, 8090 for Pipeline).
Server Menu Options
The server provides the following menu to clients:

Create a new graph.
Add an edge.
Remove an edge.
Build MST using Prim or Borůvka.
Get the total weight of the MST.
Get the longest distance in the MST.
Get the shortest distance in the MST.
Get the average distance in the MST.
Exit the program.
Testing and Validation
Code Coverage:

Use gcov or similar tools to ensure all branches and functions are covered.
Valgrind Analysis:

To check for memory leaks:
bash
Copy code
valgrind --tool=memcheck ./leaderFollower_Server
To detect thread races:
bash
Copy code
valgrind --tool=helgrind ./pipeline_server
To profile performance:
bash
Copy code
valgrind --tool=callgrind ./leaderFollower_Server
Test Cases:

Ensure the graph operations and MST algorithms are extensively tested with various scenarios.
Acknowledgments
This project was developed as part of the Operating Systems course. It explores complex concepts in OS and algorithm design, integrating theoretical knowledge with practical implementation.
