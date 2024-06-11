cmake_minimum_required(VERSION 3.18)
project(ProxyNet)

set(CMAKE_CXX_STANDARD 17)

add_library(socket source/socket/socket.cpp)
add_library(socketClient source/socket/socketClient.cpp)
add_library(sha256 source/crypto/sha.cpp)

add_executable(ProxyNet main.cpp)

target_link_libraries(socket sha256)
target_link_libraries(socketClient sha256)
target_link_libraries(socket socketClient)

target_link_libraries(ProxyNet socket)