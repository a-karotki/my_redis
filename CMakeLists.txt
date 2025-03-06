cmake_minimum_required(VERSION 3.24)
project(MRD)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(MRD_shared_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/shared/include)
set(MRD_server_dir ${CMAKE_CURRENT_SOURCE_DIR}/server)

add_subdirectory(client)
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(test)
