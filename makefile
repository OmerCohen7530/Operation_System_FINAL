# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -g
# CXXFLAGS = -std=c++17 -g -fprofile-arcs -ftest-coverage

# Source files
SOURCES = graph.cpp mst.cpp prim.cpp boruvka.cpp
HEADERS = graph.hpp mst.hpp prim.hpp boruvka.hpp
PIPELINE_SERVER = pipeline_server.cpp
LEADER_FOLLOWER_SERVER = leaderFollower_Server.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable names
PIPELINE_SERVER_EXEC = pipeline_server
LEADER_FOLLOWER_EXEC = leaderFollower_Server

# Default target
all: $(PIPELINE_SERVER_EXEC) $(LEADER_FOLLOWER_EXEC)

# Rule for building the pipeline server
$(PIPELINE_SERVER_EXEC): $(OBJECTS) $(PIPELINE_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule for building the leader-follower server
$(LEADER_FOLLOWER_EXEC): $(OBJECTS) $(LEADER_FOLLOWER_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule for building object files
%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Add this target to run Valgrind analysis
# valgrind:
# 	./run_valgrind.sh


# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(PIPELINE_SERVER_EXEC) $(LEADER_FOLLOWER_EXEC) *.gcno *.gcda *.gcov

# Phony targets
.PHONY: all clean

#./pipeline_server
#nc localhost 9080
