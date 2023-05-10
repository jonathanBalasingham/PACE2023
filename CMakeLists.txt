cmake_minimum_required(VERSION 3.25)
project(PACE)

set(CMAKE_CXX_STANDARD 20)

add_executable(PACE main.cpp dm_english.h dm.c dm.c Distribution.h tree.h mcts.h graph.h solver.h)
