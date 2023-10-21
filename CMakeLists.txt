cmake_minimum_required(VERSION 3.24)
project(ProxyNet)

set(CMAKE_CXX_STANDARD 23)

add_library(socket source/socket/server_socket.cpp)
add_library(sha256 source/crypto/sha256.cpp)

add_executable(ProxyNet main.cpp)

target_link_libraries(ProxyNet socket)
target_link_libraries(ProxyNet sha256)