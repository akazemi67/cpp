cmake_minimum_required(VERSION 3.22)

project(P2pChat)

find_package(Qt6 COMPONENTS
         Core
         Gui
         Widgets
         REQUIRED)

add_subdirectory(logging)
add_subdirectory(inetui)
add_subdirectory(netlib)
add_subdirectory(ui/graphical)
add_subdirectory(ui/console)


