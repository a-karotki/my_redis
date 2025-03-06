add_executable(MRD_client
        client.cpp
)

# Optionally link the shared library (if client uses shared utilities)
target_include_directories(MRD_client PRIVATE ${MRD_shared_include_dir})