cmake_minimum_required(VERSION 3.10)

project(MemorySimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src)

add_executable(memsim src/main.cpp)
