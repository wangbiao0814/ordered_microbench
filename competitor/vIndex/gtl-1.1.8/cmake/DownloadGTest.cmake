# Downloads and unpacks googletest at configure time.  Based on the instructions
# at https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project

# Download the latest googletest from Github master
configure_file(
               ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt.in
               ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt
)

set(GTL_SAVE_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(GTL_SAVE_CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Configure and build the downloaded googletest source
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

if(result)
         message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

if(result)
         message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(CMAKE_CXX_FLAGS ${GTL_SAVE_CMAKE_CXX_FLAGS})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GTL_SAVE_CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines the gtest and gtest_main
# targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)
