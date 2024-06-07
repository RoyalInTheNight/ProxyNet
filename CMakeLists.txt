cmake_minimum_required(VERSION 3.18)
project(ProxyNet)

set(CMAKE_CXX_STANDARD 17)

add_library(socket source/socket/socket.cpp)

add_executable(ProxyNet main.cpp)

target_link_libraries(ProxyNet socket)