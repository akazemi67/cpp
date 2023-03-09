cmake_minimum_required(VERSION 3.22)
project(netlib)

set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)

set(PROTO_FILES
        protos/messages.proto
        )

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(netlib SHARED ${PROTO_SRCS} ${PROTO_HDRS} networking.cpp)

target_include_directories(netlib PRIVATE ${CMAKE_SOURCE_DIR}/P2PChat/inetui ${CMAKE_SOURCE_DIR}/P2PChat/logging)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(netlib PUBLIC logging protobuf::libprotobuf)
