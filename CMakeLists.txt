cmake_minimum_required(VERSION 3.25)
project(PACE)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

add_executable(PACE main.cpp dm_english.h dm.c dm.c Distribution.h tree.h mcts.h graph.h solver.h bfs_solver.h quick_solver.h graph_v2.h)
