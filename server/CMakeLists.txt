cmake_minimum_required(VERSION 3.5)
project(server)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(server main.cpp maths_server.cpp maths.h)
