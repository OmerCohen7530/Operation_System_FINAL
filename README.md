# README for OS Project - Minimal Spanning Tree (MST)

## Project Overview

This project focuses on implementing the **Minimal Spanning Tree (MST)** problem using weighted, directed graphs. It incorporates advanced concepts in operating systems, such as:

- **Design Patterns**: Strategy, Factory.
- **Client-Server Architecture**: Implemented with threads.
- **Concurrency**: Thread pool with Leader-Follower model.
- **Pipeline Processing**: Using Active Object for staged execution.
- **Memory Analysis**: With Valgrind tools like `memcheck`, `helgrind`, and `callgrind`.
- **Code Coverage**: Ensuring comprehensive testing.

The project provides functionality to calculate:
- Total weight of the MST.
- Longest distance between two vertices.
- Average distance between edges.
- Shortest distance between vertices (on the MST).

### Features
1. **Graph Data Structure**: Custom implementation of a graph, supporting addition/removal of edges.
2. **Factory Pattern**: Supports different MST algorithms:
   - Borůvka
   - Prim
3. **Server**:
   - Handles client requests for MST-related operations.
   - Supports **Leader-Follower Thread Pool** and **Pipeline Active Object** for client handling.
4. **Memory and Performance Analysis**:
   - Valgrind for memory leaks, thread race detection, and performance profiling.
5. **Code Coverage**:
   - Ensures all functionalities are tested.

---

## Files in the Project

| **File Name**             | **Description**                                                                                                                                                          |
|---------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `graph.hpp`               | Implementation of the graph data structure, including methods for adding and removing edges.                                                                            |
| `mst.hpp`                 | Implementation of MST algorithms (Borůvka and Prim).                                                                                                                    |
| `prim.hpp`                | Specific implementation of Prim's algorithm.                                                                                                                            |
| `boruvka.hpp`             | Specific implementation of Borůvka's algorithm.                                                                                                                         |
| `leaderFollower_Server.cpp` | Leader-Follower Thread Pool implementation for handling client-server interactions.                                                                                     |
| `pipeline_server.cpp`     | Pipeline implementation for staged client requests using the Active Object pattern.                                                                                     |
| `makefile`                | Automates the build process, ensuring all dependencies are properly compiled.                                                                                           |

---

## Project Architecture

### Factory Design for MST
The factory pattern supports switching between MST algorithms, enabling flexibility based on user requests (`prim` or `boruvka`).

### Thread Pool (Leader-Follower)
Efficiently manages client requests using a fixed pool of threads. Requests are added to a queue and processed by worker threads.

### Pipeline Processing (Active Object)
Encapsulates asynchronous task execution:
1. **Stage 1**: Processes requests to create or modify graphs.
2. **Stage 2**: Processes MST-related computations.
3. **Stage 3**: Outputs results to clients.

---

## How to Build and Run

1. **Building the Project**:
   - Use the provided `makefile`:
     ```bash
     make all
     ```
   - This will compile all source files and create the necessary binaries.

2. **Running the Server**:
   - To run the Leader-Follower server:
     ```bash
     ./leaderFollower_Server
     ```
   - To run the Pipeline server:
     ```bash
     ./pipeline_server
     ```

3. **Connecting Clients**:
   - Use any client capable of socket communication (e.g., Telnet or a custom client).
   - Connect to the server on the specified port (`8094` for Leader-Follower, `8090` for Pipeline).

---

## Server Menu Options

The server provides the following menu to clients:
1. Create a new graph.
2. Add an edge.
3. Remove an edge.
4. Build MST using Prim or Borůvka.
5. Get the total weight of the MST.
6. Get the longest distance in the MST.
7. Get the shortest distance in the MST.
8. Get the average distance in the MST.
9. Exit the program.

---

## Testing and Validation

1. **Code Coverage**:
   - Use `gcov` or similar tools to ensure all branches and functions are covered.

2. **Valgrind Analysis**:
   - To check for memory leaks:
     ```bash
     valgrind --tool=memcheck ./leaderFollower_Server
     ```
   - To detect thread races:
     ```bash
     valgrind --tool=helgrind ./pipeline_server
     ```
   - To profile performance:
     ```bash
     valgrind --tool=callgrind ./leaderFollower_Server
     ```

3. **Test Cases**:
   - Ensure the graph operations and MST algorithms are extensively tested with various scenarios.

---

## Acknowledgments

This project was developed as part of the Operating Systems course. It explores complex concepts in OS and algorithm design, integrating theoretical knowledge with practical implementation.
