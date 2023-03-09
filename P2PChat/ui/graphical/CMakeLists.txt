cmake_minimum_required(VERSION 3.5)

project(QtChatPanel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

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
        ImageWidget.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/P2PChat/inetui
        ${CMAKE_SOURCE_DIR}/P2PChat/logging ${CMAKE_SOURCE_DIR}/P2PChat/dataio)

qt_add_executable(QtChatPanel
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

target_link_libraries(QtChatPanel PRIVATE logging Qt6::Core Qt6::Gui Qt6::Widgets fmt dataio netlib)

qt_finalize_executable(QtChatPanel)

