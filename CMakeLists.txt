cmake_minimum_required(VERSION 3.18)
project(ProxyNet)

set(CMAKE_CXX_STANDARD 17)

add_library(socket source/socket/server_socket.cpp)
add_library(sha256 source/crypto/sha256.cpp)
add_library(server source/server/server.cpp)

add_executable(ProxyNet main.cpp)

target_link_libraries(server socket)
target_link_libraries(server sha256)

target_link_libraries(ProxyNet server)
