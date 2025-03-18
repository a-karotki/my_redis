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



set(TEST_LIST hashmap_test buffer_test utility_test zset_test avl_test)

foreach (x ${TEST_LIST})
    add_executable(${x} ${x}.cpp)
    target_include_directories(${x} PRIVATE ${MRD_shared_include_dir})
    target_link_libraries(${x} PRIVATE GTest::gtest_main)
endforeach ()

target_link_libraries(hashmap_test PRIVATE  MRD_shared_hashtable)
target_link_libraries(zset_test PRIVATE MRD_shared_zset)
target_link_libraries(avl_test PRIVATE MRD_shared_avl)

include(GoogleTest)
foreach (x ${TEST_LIST})
    gtest_discover_tests(${x})
endforeach ()