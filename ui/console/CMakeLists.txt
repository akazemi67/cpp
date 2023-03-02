cmake_minimum_required(VERSION 3.22)
project(ConsoleUI)

set(CMAKE_CXX_STANDARD 20)

add_executable(ConsoleUI main.cpp)

target_include_directories(ConsoleUI PRIVATE ${CMAKE_SOURCE_DIR}/inetui)
target_link_libraries(ConsoleUI netlib)