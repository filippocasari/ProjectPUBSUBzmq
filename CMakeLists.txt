cmake_minimum_required(VERSION 3.19)
project(ProjectPUBSUBzmq)

set(CMAKE_CXX_STANDARD 11)

add_library(linkedblockingqueue.h STATIC prova1.cpp)
link_libraries("-lpthread")
link_libraries("-lczmq")
link_libraries("-lzmq")
link_libraries("-lm")
link_libraries("-ljson-c")
add_executable(TEST1 prova1.cpp)
add_executable(PUB simplePUB.c)