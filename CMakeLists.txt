cmake_minimum_required(VERSION 3.10)
project(cdatatypes LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Allocator src is needed by allocator header
set(ALLOCATOR_SRC allocator/src/allocator.c)
# Set sources and include directories
set(VECTOR_TEST_SRC arraylist/src/test.c)
include_directories(include)

if(WIN32)
    find_program(CLANG_CL_EXECUTABLE clang-cl)
    if(CLANG_CL_EXECUTABLE)
        set(CMAKE_C_COMPILER "${CLANG_CL_EXECUTABLE}")
        message(STATUS "Using clang-cl as C compiler on Windows")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /Zi")
    else()
        message(FATAL_ERROR "clang-cl not found! Install LLVM/Clang for Windows and ensure clang-cl on PATH")
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -g")
endif()

add_executable(test_vector
    ${ALLOCATOR_SRC}
    ${VECTOR_TEST_SRC}
)

set_target_properties(test_vector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_include_directories(test_vector PRIVATE "${PROJECT_SOURCE_DIR}/include")
