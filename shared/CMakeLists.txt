set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(MRD_shared_hashtable STATIC ${source_dir}/MRD_hashtable.cpp)
add_library(MRD_shared_avl STATIC ${source_dir}/MRD_avl.cpp)
add_library(MRD_shared_zset STATIC ${source_dir}/MRD_zset.cpp)
add_library(MRD_shared_dlist STATIC ${source_dir}/MRD_DList.cpp)
add_library(MRD_shared_heap STATIC ${source_dir}/MRD_heap.cpp)
target_include_directories(MRD_shared_hashtable PUBLIC
        ${MRD_shared_include_dir}
)
target_include_directories(MRD_shared_avl PUBLIC
        ${MRD_shared_include_dir}
)
target_include_directories(
        MRD_shared_dlist PUBLIC
        ${MRD_shared_include_dir}
)
target_include_directories(
        MRD_shared_heap PUBLIC
        ${MRD_shared_include_dir}
)

target_link_libraries(MRD_shared_zset
        PUBLIC
        MRD_shared_avl
        MRD_shared_hashtable
)

