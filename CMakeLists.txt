# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(DataFetcher VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Add the executable
add_executable(data-fetcher src/main.cpp)
