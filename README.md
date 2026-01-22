# C Data Structures

This is (WIP) a collection of generic, macro-based container data structures for the C language, with a custom allocator support and test coverage.

Currently, only an arraylist (c++ vector) is implemented, can be used with any C (custom user or not) type, including value types and pointers.

The container itself is memory-safe (at least that I know of) with clear destruction patterns and ownership semantics, but the user can write unsafe code to operate on it, like operating directly on the data buffer.

# Requirements

- C Compiler (compiled with C99)
- CMake (3.20+)

## Build & Run

### Step-by-step

1. Clone the repo
    - `$ git clone`
    - `$ cd cdatatypes`
    - `$ cd build`
2. Configure the build
    - `$ cmake -B build ..`
    - For release build do `$ cmake -B build -DCMAKE_BUILD_TYPE=Release ..`
    - For sanitizer build do `$ cmake -B build -DCMAKE_BUILD_TYPE=Sanitizer ..`
3. Build
    - `$ cmake --build .`
4. Run
    - `$ ./bin/*(.exe)`

#### Extra note for Windows

The build system will search for `clang-cl`, then fallback to MSVC. To force MSVC use the option -DUSE_CLANG_CL=OFF.

# Usage

## Using the Arraylist

### To define an arraylist for a type:

```c
#include "arraylist.h"

struct mytype { int x; float y; };
ARRAYLIST(struct mytype, mytype, mytype_macro_dtor)
```
### Initialize:

```c
struct arraylist_mytype arr = mytype_init(NULL);
// use arr
mytype_deinit(&arr);
```

Destructor functions are critical for correct memory handling of heap allocated fields inside structs, if your type doesn't allocate anything inside it, or it isn't a pointer type, no need for a destructor.

You are also free to not use a destructor at all and manage all the memory.

All arraylist functions are safe to call on NULL or deinitialized arraylist, unless assert is decided to be used.

Unit tests on [arraylist/src/test.c](arraylist/tests/test.c).

Examples are on arraylist/examples/example_*.c.

### Performance:

The `arraylist` implementation provides performance nearly identical (sometimes beating) to C++ STL `std::vector<unique_ptr<T>>`.
Overheads for the "function pointer" destructor version are very small (less than 1% for heavy pointer use).

For more details and benchmarking code, see [arraylist/src/PERFORMANCE.md](arraylist/PERFORMANCE.md).

# Custom Allocators

Allocator is a pluggable interface via the Allocator struct, by default malloc/realloc/free are used.
To use a custom allocator, you have to implement the three function pointers as described in allocator.h, then create and pass an Allocator instance to init function

# Documentation

I tried to document everything with doxygen comments, macros are very hard to document properly, but it is generating some of them.

To run it, install doxygen and also GraphViz, then run from the root project directory:
`$ doxygen` 

The [Doxyfile](Doxyfile) may be altered to not need GraphViz.
