cmake_minimum_required(VERSION 3.5)
project(stubgen)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Werror -pedantic")
add_executable(stubgen main.cpp parser.h parser.cpp)