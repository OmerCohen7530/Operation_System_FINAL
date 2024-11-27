#!/bin/bash

# Run Valgrind memcheck
valgrind --tool=memcheck ./pipeline_server > pipeline_memcheck.txt 2>&1
valgrind --tool=memcheck ./leaderFollower_Server > leaderFollower_memcheck.txt 2>&1

# Run Valgrind helgrind
valgrind --tool=helgrind ./pipeline_server > pipeline_helgrind.txt 2>&1
valgrind --tool=helgrind ./leaderFollower_Server > leaderFollower_helgrind.txt 2>&1

# Run Valgrind callgrind
valgrind --tool=callgrind ./pipeline_server > pipeline_callgrind.txt 2>&1
valgrind --tool=callgrind ./leaderFollower_Server > leaderFollower_callgrind.txt 2>&1

echo "Valgrind analysis completed and saved to text files."


# for code coverage
# do not forget to compile with -fprofile-arcs -ftest-coverage flags
# first run the program


# //gcov
gcov -b graph.cpp mst.cpp prim.cpp boruvka.cpp pipeline_server.cpp leaderFollower_Server.cpp

# OR

# //lcov
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory out
google-chrome out/index.html
