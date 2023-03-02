cmake_minimum_required(VERSION 3.22)
project(GraphicalUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(GraphicalUI main.cpp)
target_link_libraries(GraphicalUI
        Qt::Core
        Qt::Gui
        Qt::Widgets
        )

target_include_directories(GraphicalUI PRIVATE ${CMAKE_SOURCE_DIR}/inetui)

target_link_libraries(GraphicalUI netlib)

