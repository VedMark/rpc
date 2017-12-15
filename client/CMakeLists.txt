cmake_minimum_required(VERSION 3.5)
project(client)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(client main.cpp maths_client.cpp maths.h)
