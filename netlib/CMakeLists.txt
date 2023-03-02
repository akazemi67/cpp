cmake_minimum_required(VERSION 3.22)
project(netlib)

set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)

set(PROTO_FILES
        protos/message.proto
        )

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(netlib SHARED ${PROTO_SRCS} ${PROTO_HDRS} netlib.cpp)

target_include_directories(netlib PRIVATE ${CMAKE_SOURCE_DIR}/inetui)
target_include_directories(netlib PUBLIC ${PROTOBUF_GENERATED_DIR})

target_link_libraries(netlib PUBLIC protobuf::libprotobuf)
