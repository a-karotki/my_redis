cmake_minimum_required(VERSION 3.24)
project(MRD)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
add_executable(server server/server.cpp
        server/Buffer.h
        server/MRD_server.h)
add_executable(client client/client.cpp)

target_include_directories(server PUBLIC shared)
target_include_directories(client PUBLIC shared)

add_test(NAME ServerClientTest COMMAND ${CMAKE_SOURCE_DIR}/test.sh)