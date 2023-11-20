cmake_minimum_required(VERSION 3.26)
project(Project_K23_SEC)

set(CMAKE_CXX_STANDARD 17)

add_executable(Project_K23_SEC
        lsh_class.cpp
        lsh_class.h
        Hypercube.cpp
        Hypercube.h
        mnist.cpp
        mnist.h
        global_functions.cpp
        global_functions.h
        graph.cpp
        graph.h
        graph_search.cpp)
