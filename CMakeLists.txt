cmake_minimum_required(VERSION 3.10)
project(xorList)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
add_subdirectory(src)

add_executable(xorList src/main.cpp)