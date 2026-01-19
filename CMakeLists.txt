cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add an option for the user to use MSVC or clang
option(USE_CLANG_CL "Use clang-cl as a compiler on Windows (if available)" ON)

# Detect compiler on Windows
if(WIN32)
    find_program(CLANG_CL_EXE clang-cl)
    if(CLANG_CL_EXE AND USE_CLANG_CL)
        message(STATUS "clang-cl detected and USE_CLANG_CL=ON: using clang as C compiler on Windows")
        set(CMAKE_C_COMPILER "${CLANG_CL_EXE}" CACHE STRING "C Compiler" FORCE)
    elseif(CLANG_CL_EXE AND NOT USE_CLANG_CL)
        message(STATUS "USE_CLANG_CL=OFF: using MSVC")
    elseif(NOT CLANG_CL_EXE)
        message(STATUS "clang-cl not detected, using MSVC")
    endif()
endif()

project(cdatatypes LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set sources and include directories
include_directories(include)

# Arraylist test and example sources
set(ARRAYLIST_TEST_SRC arraylist/src/test.c)
set(ARRAYLIST_EXAMPLE_FP1_SRC arraylist/src/example_fp1.c)
set(ARRAYLIST_EXAMPLE_FP2_SRC arraylist/src/example_fp2.c)
set(ARRAYLIST_EXAMPLE_FP3_SRC arraylist/src/example_fp3.c)

# Build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Sanitizer" CACHE STRING "" FORCE)

# Compiler and flags
if (MSVC AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    # MSVC/clang-cl flags
    message(STATUS "Configuring flags for clang-cl on Windows")
    set(BASE_FLAGS "/W4 /Zi")
    set(DEBUG_FLAGS "/Od /RTC1 /MDd")
    set(RELEASE_FLAGS "/O2 /DNDEBUG /MD")
    set(SAN_FLAGS "/fsanitize=address /Zi /Od /MDd")
elseif(MSVC)
    # MSVC flags
    message(STATUS "Configuring flags for MSVC on Windows")
    set(BASE_FLAGS "/W4 /Zi")
    set(DEBUG_FLAGS "/Od /RTC1 /MDd")
    set(RELEASE_FLAGS "/O2 /DNDEBUG /LTCG /GL /MD")
    set(SAN_FLAGS "/fsanitize=address /Zi /Od /MDd")
else()
    # gcc/clang flags
    message(STATUS "Configuring flags for GCC/Clang on *nix")

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
    set(RELEASE_FLAGS "-O3 -DNDEBUG -flto -fno-sanitize=all")
    set(SAN_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer -O0")
endif()

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
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

message(STATUS "Used build type: ${CMAKE_BUILD_TYPE}")

# Executables
# Arraylist unit tests executable
add_executable(test_arraylist ${ARRAYLIST_TEST_SRC})

# Arraylist examples executable
add_executable(example_arraylistfp1 ${ARRAYLIST_EXAMPLE_FP1_SRC})
add_executable(example_arraylistfp2 ${ARRAYLIST_EXAMPLE_FP2_SRC})
add_executable(example_arraylistfp3 ${ARRAYLIST_EXAMPLE_FP3_SRC})

# Output directory
set_target_properties(test_arraylist PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylistfp1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylistfp2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylistfp3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directory
target_include_directories(test_arraylist PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylistfp1 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylistfp2 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylistfp3 PRIVATE "${PROJECT_SOURCE_DIR}/include")

# The default is set on the section "# Set default build type"
# $cmake ..

# For Release build:
# $cmake -DCMAKE_BUILD_TYPE=Release ..

# For Sanitizer build (ASan/UBSan):
# $cmake -DCMAKE_BUILD_TYPE=Sanitizer ..

# For Debug build:
# $cmake -DCMAKE_BUILD_TYPE=Debug ..

# If on Windows and want to force MSVC use the option -DUSE_CLANG_CL=OFF

# $cmake --build .
# Executables will be on build/bin/*(.exe)
