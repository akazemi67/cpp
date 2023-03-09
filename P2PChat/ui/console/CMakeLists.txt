cmake_minimum_required(VERSION 3.22)
project(ConsoleUI)

set(CMAKE_CXX_STANDARD 20)

add_executable(ConsoleUI main.cpp)

target_include_directories(ConsoleUI PRIVATE ${CMAKE_SOURCE_DIR}/P2PChat/inetui
        ${CMAKE_SOURCE_DIR}/P2PChat/logging ${CMAKE_SOURCE_DIR}/P2PChat/dataio)

target_link_libraries(ConsoleUI PRIVATE logging fmt dataio netlib)

include_directories(${CMAKE_BINARY_DIR}/netlib)

