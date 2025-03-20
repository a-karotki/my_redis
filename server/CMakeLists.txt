add_executable(MRD_server
        MRD_server.cpp
)

# Link the shared library
target_link_libraries(MRD_server
        PRIVATE
        MRD_shared_hashtable
        MRD_shared_zset
        MRD_shared_avl
        MRD_shared_dlist
        MRD_shared_heap
        )

# Include the server's headers
target_include_directories(MRD_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})