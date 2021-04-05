cmake_minimum_required(VERSION 3.17)
project(operating_systems)

set(CMAKE_CXX_STANDARD 17)

add_executable(operating_systems_fork fork.cpp)
add_executable(operating_systems_shell shell.cpp)
