cmake_minimum_required(VERSION 3.22)
project(logging)

set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(${YAML_CPP_INCLUDE_DIR})

add_library(logging SHARED logging.cpp)

target_link_libraries(logging PRIVATE ${YAML_CPP_LIBRARIES} spdlog::spdlog)

