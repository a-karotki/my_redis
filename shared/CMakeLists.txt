set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(MRD_shared_hashtable STATIC ${source_dir}/MRD_hashtable.cpp)

target_include_directories(MRD_shared_hashtable PUBLIC
        ${MRD_shared_include_dir}
)

