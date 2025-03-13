cmake_minimum_required(VERSION 3.24)
project(MRD)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        hashmap_test
        hashmap_test.cpp
)
add_executable(
        buffer_test
        buffer_test.cpp
)
add_executable(
        utility_test
        utility_test.cpp
)
add_executable(
        avl_test
        avl_test.cpp
)
add_executable(
        zset_test
        zset_test.cpp
)
target_link_libraries(
        hashmap_test
        PRIVATE
        MRD_shared_hashtable
        GTest::gtest_main
)
target_link_libraries(
        buffer_test
        PRIVATE
        GTest::gtest_main
)
target_link_libraries(
        utility_test
        PRIVATE
        GTest::gtest_main
)
target_link_libraries(
        avl_test
        PRIVATE
        GTest::gtest_main
        MRD_shared_avl
)
target_link_libraries(
        zset_test
        PRIVATE
        GTest::gtest_main
        MRD_shared_zset
)
target_include_directories(
        utility_test
        PRIVATE
        ${MRD_shared_include_dir}
)
target_include_directories(
        buffer_test
        PRIVATE
        ${MRD_server_dir}
        ${MRD_shared_include_dir}
)
target_include_directories(
        avl_test
        PRIVATE
        ${MRD_shared_include_dir}
)
target_include_directories(
        zset_test
        PRIVATE
        ${MRD_shared_include_dir}
)
include(GoogleTest)
gtest_discover_tests(hashmap_test)
gtest_discover_tests(buffer_test)
gtest_discover_tests(utility_test)
gtest_discover_tests(avl_test)
gtest_discover_tests(zset_test)