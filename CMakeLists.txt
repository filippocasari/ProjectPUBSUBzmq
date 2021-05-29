cmake_minimum_required(VERSION 3.7)
project(ProjectPUBSUBzmq)

set(CMAKE_CXX_STANDARD 11)

add_library(linkedblockingqueue.h STATIC SUBfinal.cpp)

link_libraries("-lpthread")
link_libraries("-lczmq")
link_libraries("-lzmq")
link_libraries("-lm")
link_libraries("-ljson-c")
add_compile_options(-Wall)
add_executable(SUB1 SUBfinal.cpp)
add_executable(PUB simplePUB.c)
add_executable(Item Utils/Item.cpp)
add_executable(SUB2 SUBfinal2.cpp)