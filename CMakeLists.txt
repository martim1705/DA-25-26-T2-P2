cmake_minimum_required(VERSION 4.0)
project(projeto2)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(projeto2
        include/Allocator.h
        include/Graph.h
        include/Parser.h
        include/Structures.h
        include/Web.h
        include/InterferenceGraph.h
        src/Allocator.cpp
        src/main.cpp
        src/Parser.cpp
        src/Web.cpp
        src/InterferenceGraph.cpp)
