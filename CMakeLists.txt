cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cdatatypes LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set sources and include directories

# Allocator src is needed by allocator header
set(ALLOCATOR_SRC allocator/src/allocator.c)
set(VECTOR_TEST_SRC arraylist/src/test.c)

include_directories(include)

# Build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Sanitizer" CACHE STRING "" FORCE)

if(WIN32)
    find_program(CLANG_CL_EXECUTABLE clang-cl)
    if(CLANG_CL_EXECUTABLE)
        set(CMAKE_C_COMPILER "${CLANG_CL_EXECUTABLE}")
        message(STATUS "Using clang-cl as C compiler on Windows")
    else()
        message(FATAL_ERROR "clang-cl not found! Install LLVM/Clang for Windows and ensure clang-cl on PATH")
    endif()

    # MSVC/clang-cl flags
    set(BASE_FLAGS "/W4 /Zi")
    set(DEBUG_FLAGS "/Od /RTC1 /MDd")
    set(RELEASE_FLAGS "/O2 /DNDEBUG /MD")
    set(SAN_FLAGS "/fsanitize=address /Zi /Od /MDd")
else()
    # gcc/clang flags

    # All possible sanitizers, as far as I searched:
    #-fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=memory -fsanitize=thread
    #-fsanitize=integer -fsanitize=null -fsanitize=return -fsanitize=signed-integer-overflow
    #-fsanitize=bounds -fsanitize=alignment -fsanitize=object-size -fsanitize=float-divide-by-zero
    #-fsanitize=float-cast-overflow -fsanitize=nonnull-attribute -fsanitize=vla-bound

    # More possible warnings for clang:
    #-ferror-limit=0 -Weverything

    # Some warnings that are not really errors:
    #-Wno-padded -Wno-reserved-macro-identifier -Wno-declaration-after-statement -Wno-unsafe-buffer-usage
    #-Wno-bad-function-cast -Wno-reserved-identifier -Wno-float-equal -Wno-vla -Wno-pre-c11-compat

    set(BASE_FLAGS "-Wall -Wextra -pedantic")
    set(DEBUG_FLAGS "-g3 -O0 -fno-sanitize=all")
    set(RELEASE_FLAGS "-O3 -DNDEBUG -fno-sanitize=all")
    set(SAN_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer -00")
endif()

# Set flags depending on build type
if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
    set(CMAKE_C_FLAGS "${BASE_FLAGS} ${DEBUG_FLAGS}")
    message(STATUS "Configuring Debug build")
elseif(CMAKE_BUILD_TYPE MATCHES "[Rr]elease")
    set(CMAKE_C_FLAGS "${BASE_FLAGS} ${RELEASE_FLAGS}")
    message(STATUS "Configuring Release build")
elseif(CMAKE_BUILD_TYPE MATCHES "[Ss]anitizer")
    set(CMAKE_C_FLAGS "${BASE_FLAGS} ${SAN_FLAGS}")
    message(STATUS "Configuring Sanitizer build")
else()
    message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

add_executable(test_vector ${ALLOCATOR_SRC} ${VECTOR_TEST_SRC})

set_target_properties(test_vector PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_include_directories(test_vector PRIVATE "${PROJECT_SOURCE_DIR}/include")

# Debug build is default
# $cmake ..

# For Release build:
# $cmake -DCMAKE_BUILD_TYPE=Release ..

# For Sanitizer build (ASan/UBSan):
# $cmake -DCMAKE_BUILD_TYPE=Sanitizer ..

# $cmake --build .
# Executable will be on build/bin/test_vector(.exe)
