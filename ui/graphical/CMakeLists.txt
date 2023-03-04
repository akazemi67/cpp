cmake_minimum_required(VERSION 3.5)

project(QtChatPanel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        chatwindow.cpp
        chatwindow.h
        chatwindow.ui
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

qt_add_executable(QtChatPanel
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

target_link_libraries(QtChatPanel PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

qt_finalize_executable(QtChatPanel)

