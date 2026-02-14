cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

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
set(ARRAYLIST_TEST_SRC arraylist/tests/test.c)
set(ARRAYLIST_DYN_TEST_SRC arraylist/tests/test_dyn.c)
set(ARRAYLIST_EXAMPLE_1_SRC arraylist/examples/example_1.c)
set(ARRAYLIST_EXAMPLE_2_SRC arraylist/examples/example_2.c)
set(ARRAYLIST_EXAMPLE_3_SRC arraylist/examples/example_3.c)
set(ARRAYLIST_EXAMPLE_DYN1_SRC arraylist/examples/example_dyn1.c)
set(ARRAYLIST_EXAMPLE_DYN2_SRC arraylist/examples/example_dyn2.c)
set(ARRAYLIST_EXAMPLE_DYN3_SRC arraylist/examples/example_dyn3.c)

# Pair test and example sources
set(PAIR_TEST_SRC pair/tests/test.c)
set(PAIR_EXAMPLE_1_SRC pair/examples/example1.c)

# AVLTree test and example sources
set(AVL_TEST_USE avltree/avlusage.c)

# Build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Sanitizer" CACHE STRING "" FORCE)

# Compiler and flags
if (MSVC AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    # MSVC/clang-cl flags
    message(STATUS "Configuring flags for clang-cl on Windows")
    set(BASE_FLAGS "/W4 /Zi")
    set(DEBUG_FLAGS "/Od /RTC1 /MDd")
    set(RELEASE_FLAGS "/O2 /DNDEBUG /LTCG /GL /MD /MD")
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
# Arraylist executables
add_executable(test_arraylist ${ARRAYLIST_TEST_SRC})
add_executable(test_arraylist_dyn ${ARRAYLIST_DYN_TEST_SRC})
add_executable(example_arraylist1 ${ARRAYLIST_EXAMPLE_1_SRC})
add_executable(example_arraylist2 ${ARRAYLIST_EXAMPLE_2_SRC})
add_executable(example_arraylist3 ${ARRAYLIST_EXAMPLE_3_SRC})
add_executable(example_arraylistdyn1 ${ARRAYLIST_EXAMPLE_DYN1_SRC})
add_executable(example_arraylistdyn2 ${ARRAYLIST_EXAMPLE_DYN2_SRC})
add_executable(example_arraylistdyn3 ${ARRAYLIST_EXAMPLE_DYN3_SRC})

# Arraylist Output directory
set_target_properties(test_arraylist PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(test_arraylist_dyn PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylist1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylist2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylist3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylistdyn1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylistdyn2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_arraylistdyn3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Arraylist Include directory
target_include_directories(test_arraylist PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(test_arraylist_dyn PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylist1 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylist2 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylist3 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylistdyn1 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylistdyn2 PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_arraylistdyn3 PRIVATE "${PROJECT_SOURCE_DIR}/include")

# Pair executables
add_executable(test_pair ${PAIR_TEST_SRC})
add_executable(example_pair1 ${PAIR_EXAMPLE_1_SRC})

# Pair Output directory
set_target_properties(test_pair PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(example_pair1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Pair Include directory
target_include_directories(test_pair PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example_pair1 PRIVATE "${PROJECT_SOURCE_DIR}/include")

# AVLTree executables
add_executable(avl_test_usage ${AVL_TEST_USE})

# AVLTree Output directory
set_target_properties(avl_test_usage PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# AVLTree Include directory
target_include_directories(avl_test_usage PRIVATE "${PROJECT_SOURCE_DIR}/include")

# ctest
add_test(NAME unit_test_arraylist COMMAND test_arraylist)
add_test(NAME unit_test_arraylist_dyn COMMAND test_arraylist_dyn)
add_test(NAME unit_test_pair COMMAND test_pair)

add_custom_target(
    run_all_binaries
    COMMAND $<TARGET_FILE:test_arraylist>
    COMMAND $<TARGET_FILE:test_arraylist_dyn>
    COMMAND $<TARGET_FILE:example_arraylist1>
    COMMAND $<TARGET_FILE:example_arraylist2>
    COMMAND $<TARGET_FILE:example_arraylist3>
    COMMAND $<TARGET_FILE:example_arraylistdyn1>
    COMMAND $<TARGET_FILE:example_arraylistdyn2>
    COMMAND $<TARGET_FILE:example_arraylistdyn3>
    COMMAND $<TARGET_FILE:test_pair>
    COMMAND $<TARGET_FILE:example_pair1>
    COMMAND $<TARGET_FILE:avl_test_usage>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all project executables..."
    # Add a dependency so 'run_all_binaries' is built (though it doesn't create a file)
    # DEPENDS MyApp1 MyApp2
)

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

# To run tests:
# $ctest

# To run all binaries (including the test)
# $cmake --build . --target run_all_binaries
