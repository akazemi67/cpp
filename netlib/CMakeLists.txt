cmake_minimum_required(VERSION 3.22)
project(netlib)

set(CMAKE_CXX_STANDARD 20)

add_library(netlib SHARED library.cpp)

target_include_directories(netlib PRIVATE ${CMAKE_SOURCE_DIR}/inetui)

