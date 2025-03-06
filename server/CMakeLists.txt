add_executable(MRD_server
        server.cpp
)

# Link the shared library
target_link_libraries(MRD_server PRIVATE MRD_shared_hashtable)

# Include the server's headers
target_include_directories(MRD_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})