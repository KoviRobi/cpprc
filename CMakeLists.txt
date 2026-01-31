cmake_minimum_required(VERSION 3.21)

project(cpprc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(test SHARED test/test.cpp)
target_include_directories(test PRIVATE .)
