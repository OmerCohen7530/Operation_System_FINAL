# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -g
# Files and output
TARGET = pipeline_server
SRCS = pipeline_server.cpp graph.cpp mst.cpp boruvka.cpp prim.cpp
HEADERS = graph.hpp mst.hpp boruvka.hpp prim.hpp

# Rule for compiling the target
$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# Clean up the compiled files
clean:
	rm -f $(TARGET)
