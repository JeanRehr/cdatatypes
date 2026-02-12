# C Data Structures

This is (WIP) a collection of generic, macro-based container data structures for the C language, with a custom allocator support and test coverage.

Can be used with any C (custom user or scalar) type, including value types and pointers.

The container itself is memory-safe (at least that I know of) with clear destruction patterns and ownership semantics, but the user can write unsafe code to operate on it, like operating directly on the data buffer/nodes.

# Requirements

- C Compiler (tested on C99)
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
// myfile.h:
#include "arraylist.h"

struct mytype { int x; float y; };
// -- mytype functions --

ARRAYLIST_TYPE(struct mytype, mytype)
ARRAYLIST_DECL(struct mytype, mytype)
/* ============= */
// myfile.c:
#include "myfile.h"
ARRAYLIST_IMPL(struct mytype, mytype, mytype_macro_dtor)

struct Allocator alloc = get_default_allocator();
struct arraylist_mytype arr = mytype_init(alloc);
// use arr
mytype_deinit(&arr);
```

Destructor functions are critical for correct memory handling of heap allocated fields inside structs, if your type doesn't allocate anything inside it, or it isn't a pointer type, no need for a destructor.

You are also free to not use a destructor at all and manage all the memory.

Once an arraylist is deinitialized (e.g. on deinit or steal call), they are not safe to be used again, and must be initialized again.

Unit tests on [arraylist/tests](arraylist/tests) directory.

Examples are on [arraylist/examples](arraylist/examples) directory.

### Performance:

The `arraylist` implementation provides performance nearly identical (sometimes beating) to C++ STL `std::vector<unique_ptr<T>>`.
Overheads for the "function pointer" destructor version are very small (less than 1% for heavy pointer use).

For more details and benchmarking code, see [arraylist/PERFORMANCE.md](arraylist/PERFORMANCE.md).

## Using the Pair

### To define a pair type:

Simple usage for double int:

```c
// myfile.h:
#include "pair.h"

PAIR_TYPE(int, int, int_pair)
PAIR_DECL(int, int, int_pair)
/* ============= */
// myfile.c:
#include "myfile.h"
// Scalar types do not need a destructor function 
PAIR_IMPL(int, int, int_pair, pair_noop_deinit, pair_noop_deinit)

// But if your type allocates memory, or is a pointer type, a destructor can be passed to each one of them.

struct pair_int_pair points = int_pair_init(1, 1);
// Or:
points.first = 10;
points.second = 10;

// Use points...

// Not needed, as the type is scalar:
//points_deinit(&points, &alloc);
```

Unit tests on [pair/tests/test.c](pair/tests/test.c).

Example on using the pair for student grades on [pair/examples/example1.c](pair/examples/example1.c).

# Custom Allocators

Allocator is a pluggable interface via the Allocator struct, by default malloc/realloc/free are used.
To use a custom allocator, you have to implement the three function pointers as described in allocator.h, then create and pass an Allocator instance to init function.

# Documentation

I tried to document everything with doxygen comments, macros are very hard to document properly, but it is generating some of them.

To run it, install doxygen and GraphViz, then run from the root project directory:

`$ doxygen`

The [Doxyfile](Doxyfile) may be altered to not need GraphViz.
