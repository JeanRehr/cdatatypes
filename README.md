# C Data Structures

This is (WIP) a collection of generic, macro-based container data structures for the C language, with a custom allocator support and test coverage.

Currently, only an arraylist (c++ vector) is implemented, can be used with any C (custom user or not) type, including value types and pointers.

The container itself is memory-safe (at least that I know of) with clear destruction patterns and ownership semantics, but the user can write unsafe code to operate on it, like operating directly on the data buffer.

Compiled with C23, but should work with C99 with minimal changes, changing nullptr, bool, false, true, and constexpr to their counterparts in C99 should do the trick

# Requirements

- C Compiler supporting C23 (or C99 with minimal changes)
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

# Extra note for Windows

The build system will search for `clang-cl`. CMake can be changed to use MSVC.

# Usage

## Using the Arraylist

### To define an arraylist for a type:

```c
#include "arraylist.h"

struct mytype { int x; float y; };
ARRAYLIST(struct mytype, mytype)
```
### Initialize:

```c
struct arraylist_mytype arr = arraylist_mytype_init(nullptr, mytype_dtor);
// use arr
arraylist_mytyoe_deinit(&arr);
```

Destructor functions are critical for correct memory handling of heap allocated fields inside structs, if your type doesn't allocate anything inside it, no need for a destructor.

All arraylist functions are safe to call on nullptr or deinitialized arraylist, unless assert is decided to be used.

Full example on arraylist/src/test.c

# Custom Allocators

Allocator is a pluggable interface via the Allocator struct, by default malloc/realloc/free are used.
To use a custom allocator, you have to implement the three function pointers as described in allocator.h, then create and pass an Allocator instance to init function
