cmake_minimum_required(VERSION 3.22)
project(dataio)

set(CMAKE_CXX_STANDARD 20)

add_library(dataio SHARED dataio.cpp dataio.h json.hpp)
target_include_directories(dataio PRIVATE ${CMAKE_SOURCE_DIR}/inetui ${CMAKE_SOURCE_DIR}/logging)

target_link_libraries(dataio PUBLIC logging)
