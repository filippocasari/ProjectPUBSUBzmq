cmake_minimum_required(VERSION 3.7)
project(ProjectPUBSUBzmq)

set(CMAKE_CXX_STANDARD 11)

#add_library(linkedblockingqueue.h STATIC SUB3final.cpp)
if(NOT LINUX)
    message(STATUS ">>> APPLE")
    link_directories("/opt/homebrew/lib")
    include_directories("/opt/homebrew/include")
else(LINUX)
    message(STATUS ">>>LINUX")
endif()
link_libraries("-lpthread")
link_libraries("-lczmq")
link_libraries("-lzmq")
link_libraries("-lm")
link_libraries("-ljson-c")
add_compile_options(-Wall)
add_executable(CLIENT_BEACON discovery_client.cpp Proxy.cpp)
add_executable(SERVER_BEACON discovery_server.cpp)
add_executable(PUB2 simplePUB.cpp)
add_executable(SUB2 SUBfinal.cpp)
add_executable(INPROC_TEST_M InprocessPUBSUB_MULTIPLE_SUB.cpp)
add_executable(SUB_TO_STOP SUB_TO_STOP.cpp)
add_executable(MultiSub SUBACTORS.cpp)
add_executable(PROXY Proxy.cpp)