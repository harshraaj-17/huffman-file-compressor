cmake_minimum_required(VERSION 3.10)
project(HuffmanCompressor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Treat warnings as errors, standard good practice for systems engineering
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

add_executable(huffman src/main.cpp src/HuffmanTree.cpp)
