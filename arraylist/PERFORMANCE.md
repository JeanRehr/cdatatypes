# Performance Characteristics of the ARRAYLIST

## This was tested on the following:

### Hardware:

 - AMD Ryzen 5 8400F 6-Core Processor.
 - Caches (sum of all):         
 - - L1d:                       192 KiB (6 instances)
 - - L1i:                       192 KiB (6 instances)
 - - L2:                        6 MiB (6 instances)
 - - L3:                        16 MiB (1 instance)
 - RAM: 16 GiB 
 - GPU Radeon RX 7600/7600 XT/7600M XT/7600S/7700S

### Kernel:

 - 6.18.5-arch1-1

### Compiler version:
`clang version 21.1.6`

# Code:

All code was compiled with the following flags: `-O3 -DNDEBUG -flto`

## C `ARRAYLIST` Macro:
```c
#include <stdio.h>
#include "arraylist.h"

struct non_pod {
    int *_number;
    int *add;
    int *sub;
};

static inline struct non_pod non_pod_init(const int n, const int add, const int sub) {
    struct non_pod np = {0};
    np._number = malloc(sizeof(*np._number));
    *np._number = n;

    np.add = malloc(sizeof(*np.add));
    *np.add = add;

    np.sub = malloc(sizeof(*np.sub));
    *np.sub = sub;

    return np;
}

static inline struct non_pod *non_pod_init_alloc(struct Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod *np = alloc->malloc(sizeof(struct non_pod), alloc->ctx);
    np->_number = malloc(sizeof(*np->_number));
    *np->_number = n;

    np->add = malloc(sizeof(*np->add));
    *np->add = add;

    np->sub = malloc(sizeof(*np->sub));
    *np->sub = sub;

    return np;
}

static inline void non_pod_deinit(struct non_pod *self, struct Allocator *alloc) {
    if (!self)
        return;

    alloc->free(self->_number, sizeof(self->_number), NULL);
    alloc->free(self->add, sizeof(self->add), NULL);
    alloc->free(self->sub, sizeof(self->sub), NULL);
}

#define non_pod_deinit_ptr_macro(ptr, alloc)                                            \
  do {                                                                            \
    if (!(ptr) || !*(ptr)) break;                                                 \
    (alloc)->free((void*)(*ptr)->_number, sizeof(*(*ptr)->_number), (alloc)->ctx);\
    (alloc)->free((void*)(*ptr)->add,    sizeof(*(*ptr)->add),    (alloc)->ctx);  \
    (alloc)->free((void*)(*ptr)->sub,    sizeof(*(*ptr)->sub),    (alloc)->ctx);  \
    (alloc)->free(*ptr, sizeof(**ptr), (alloc)->ctx);                             \
    *(ptr) = NULL;                                                                \
  } while (0)

static inline void non_pod_deinit_ptr(struct non_pod **self, struct Allocator *alloc) {
    if (!self || !*self)
        return;

    non_pod_deinit(*self, alloc);

    alloc->free(*self, sizeof(*self), alloc->ctx);

    *self = NULL;
}

static inline int non_pod_calculate(struct non_pod *self) {
    *self->_number = *self->_number + (*self->add - *self->sub);
    return *self->_number;
}

ARRAYLIST(struct non_pod*, np, non_pod_deinit_ptr_macro)

int main(void) {
    Allocator alloc = allocator_get_default();

    struct arraylist_np vec_np = np_init(alloc);
    np_reserve(&vec_np, 1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        *np_emplace_back(&vec_np) = non_pod_init_alloc(&alloc, i, i * 3, i / 2);
    }

    for (volatile size_t i = 0; i < 1000000; i++) {
        printf("Calculate returns %d\n", non_pod_calculate(vec_np.data[i]));
    }

    np_deinit(&vec_np);
    return 0;
}
```

## C `ARRAYLIST_DYN` function pointer:

For `ARRAYLIST_DYN` macro, exactly the same, but uses a function pointer for destructor internally:

```c
#include <stdio.h>
#include "arraylist.h"

struct non_pod {
    int *_number;
    int *add;
    int *sub;
};

static inline struct non_pod non_pod_init(const int n, const int add, const int sub) {
    struct non_pod np = {0};
    np._number = malloc(sizeof(*np._number));
    *np._number = n;

    np.add = malloc(sizeof(*np.add));
    *np.add = add;

    np.sub = malloc(sizeof(*np.sub));
    *np.sub = sub;

    return np;
}

static inline struct non_pod *non_pod_init_alloc(struct Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod *np = alloc->malloc(sizeof(struct non_pod), alloc->ctx);
    np->_number = malloc(sizeof(*np->_number));
    *np->_number = n;

    np->add = malloc(sizeof(*np->add));
    *np->add = add;

    np->sub = malloc(sizeof(*np->sub));
    *np->sub = sub;

    return np;
}

static inline void non_pod_deinit(struct non_pod *self, struct Allocator *alloc) {
    if (!self)
        return;

    alloc->free(self->_number, sizeof(self->_number), NULL);
    alloc->free(self->add, sizeof(self->add), NULL);
    alloc->free(self->sub, sizeof(self->sub), NULL);
}

static inline void non_pod_deinit_ptr(struct non_pod **self, struct Allocator *alloc) {
    if (!self || !*self)
        return;

    non_pod_deinit(*self, alloc);

    alloc->free(*self, sizeof(*self), alloc->ctx);

    *self = NULL;
}

static inline int non_pod_calculate(struct non_pod *self) {
    *self->_number = *self->_number + (*self->add - *self->sub);
    return *self->_number;
}

ARRAYLIST_DYN(struct non_pod*, np)

int main(void) {
    struct Allocator alloc = allocator_get_default();

    struct arraylist_dyn_np vec_np = dyn_np_init(alloc, non_pod_deinit_ptr);
    dyn_np_reserve(&vec_np, 1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        *dyn_np_emplace_back(&vec_np) = non_pod_init_alloc(&alloc, i, i * 3, i / 2);
    }

    for (volatile size_t i = 0; i < 1000000; i++) {
        printf("Calculate returns %d\n", non_pod_calculate(vec_np.data[i]));
    }

    dyn_np_deinit(&vec_np);
    return 0;
}
```

## C++ STL `std::vector<unique_ptr<T>>`:

C++ version is exactly the same, but with STL:

```c++
#include <cstdio>
#include <string.h>
#include <vector>
#include <memory>

class non_pod {
public:
    inline non_pod(const int n, const int add, const int sub) {
        this->_number = new int(n);
        this->add = new int(add);
        this->sub = new int(sub);
    }
    inline ~non_pod() {
        delete this->_number;
        delete this->add;
        delete this->sub;
    }

    inline int calculate() {
        *this->_number = *this->_number + (*this->add - *this->sub) ;
        return *this->_number;
    }

private:
    int *_number;
    int *add;
    int *sub;
};

int main(void) {
    std::vector<std::unique_ptr<non_pod>> vec;
    vec.reserve(1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        vec.emplace_back(std::make_unique<non_pod>(i, i * 3, i / 2));
    }

    for (volatile size_t i = 0; i < 1000000; i++) {
        printf("Calculate returns %d\n", vec.at(i)->calculate());
    }

    return 0;
}
```

All of these types are custom non-pod types.

# Results of the `perf` command:

The following `perf` command was used:

`perf record perf stat -r 100`:

## C `ARRAYLIST` Macro version
### First time (already did a run before this to make it hot on the cache):
```txt
 Performance counter stats for './c' (100 runs):

     1.288.326.621      task-clock:u                     #    0,943 CPUs utilized               ( +-  0,10% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.735      page-faults:u                    #   24,633 K/sec                       ( +-  0,02% )
     2.103.190.655      instructions:u                   #    2,23  insn per cycle
                                                  #    0,19  stalled cycles per insn     ( +-  0,00% )
       941.470.235      cycles:u                         #    0,731 GHz                         ( +-  0,34% )
       405.821.817      stalled-cycles-frontend:u        #   43,11% frontend cycles idle        ( +-  0,76% )
       445.781.889      branches:u                       #  346,016 M/sec                       ( +-  0,00% )
        12.146.181      branch-misses:u                  #    2,72% of all branches             ( +-  0,18% )

           1,36552 +- 0,00140 seconds time elapsed  ( +-  0,10% )

[ perf record: Woken up 72 times to write data ]
[ perf record: Captured and wrote 19,154 MB perf.data (500371 samples) ]
```

### Second time:
```
 Performance counter stats for './c' (100 runs):

     1.289.950.797      task-clock:u                     #    0,943 CPUs utilized               ( +-  0,09% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.741      page-faults:u                    #   24,606 K/sec                       ( +-  0,02% )
     2.103.190.718      instructions:u                   #    2,23  insn per cycle
                                                  #    0,19  stalled cycles per insn     ( +-  0,00% )
       942.792.608      cycles:u                         #    0,731 GHz                         ( +-  0,33% )
       405.445.598      stalled-cycles-frontend:u        #   43,00% frontend cycles idle        ( +-  0,72% )
       445.781.928      branches:u                       #  345,581 M/sec                       ( +-  0,00% )
        12.116.270      branch-misses:u                  #    2,72% of all branches             ( +-  0,00% )

           1,36764 +- 0,00125 seconds time elapsed  ( +-  0,09% )

[ perf record: Woken up 70 times to write data ]
[ perf record: Captured and wrote 19,240 MB perf.data (502630 samples) ]
```

## C `ARRAYLIST_DYN` Function Pointer version
### First time (already did a run before this to make it hot on the cache):
```txt
 Performance counter stats for './cfp' (100 runs):

     1.304.568.221      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,10% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.730      page-faults:u                    #   24,322 K/sec                       ( +-  0,00% )
     2.118.190.864      instructions:u                   #    2,15  insn per cycle
                                                  #    0,19  stalled cycles per insn     ( +-  0,00% )
       985.598.461      cycles:u                         #    0,755 GHz                         ( +-  0,34% )
       400.238.272      stalled-cycles-frontend:u        #   40,61% frontend cycles idle        ( +-  0,74% )
       448.782.067      branches:u                       #  344,008 M/sec                       ( +-  0,00% )
        12.118.033      branch-misses:u                  #    2,70% of all branches             ( +-  0,01% )

           1,38192 +- 0,00138 seconds time elapsed  ( +-  0,10% )

[ perf record: Woken up 73 times to write data ]
[ perf record: Captured and wrote 19,481 MB perf.data (508928 samples) ]
```

### Second time:
```
 Performance counter stats for './cfp' (100 runs):

     1.310.256.949      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,11% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.730      page-faults:u                    #   24,217 K/sec                       ( +-  0,00% )
     2.118.190.909      instructions:u                   #    2,12  insn per cycle
                                                  #    0,20  stalled cycles per insn     ( +-  0,00% )
     1.000.117.181      cycles:u                         #    0,763 GHz                         ( +-  0,35% )
       414.423.539      stalled-cycles-frontend:u        #   41,44% frontend cycles idle        ( +-  0,71% )
       448.782.084      branches:u                       #  342,515 M/sec                       ( +-  0,00% )
        12.140.675      branch-misses:u                  #    2,71% of all branches             ( +-  0,11% )

           1,38811 +- 0,00162 seconds time elapsed  ( +-  0,12% )

[ perf record: Woken up 73 times to write data ]
[ perf record: Captured and wrote 19,588 MB perf.data (511733 samples) ]
```

## C++
### First run (already did a run before this to make it hot on the cache):
```txt
 Performance counter stats for './cpp' (100 runs):

     1.294.778.093      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,11% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.802      page-faults:u                    #   24,562 K/sec                       ( +-  0,00% )
     2.174.614.733      instructions:u                   #    2,25  insn per cycle
                                                  #    0,19  stalled cycles per insn     ( +-  0,00% )
       967.404.060      cycles:u                         #    0,747 GHz                         ( +-  0,36% )
       405.420.581      stalled-cycles-frontend:u        #   41,91% frontend cycles idle        ( +-  0,72% )
       466.200.804      branches:u                       #  360,062 M/sec                       ( +-  0,00% )
        12.162.513      branch-misses:u                  #    2,61% of all branches             ( +-  0,00% )

           1,37158 +- 0,00147 seconds time elapsed  ( +-  0,11% )

[ perf record: Woken up 74 times to write data ]
[ perf record: Captured and wrote 19,270 MB perf.data (502526 samples) ]
```

### Second time:
```txt
 Performance counter stats for './cpp' (100 runs):

     1.294.685.964      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,09% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.802      page-faults:u                    #   24,563 K/sec                       ( +-  0,00% )
     2.174.614.747      instructions:u                   #    2,23  insn per cycle
                                                  #    0,19  stalled cycles per insn     ( +-  0,00% )
       973.873.703      cycles:u                         #    0,752 GHz                         ( +-  0,37% )
       412.096.602      stalled-cycles-frontend:u        #   42,32% frontend cycles idle        ( +-  0,71% )
       466.200.811      branches:u                       #  360,088 M/sec                       ( +-  0,00% )
        12.172.407      branch-misses:u                  #    2,61% of all branches             ( +-  0,08% )

           1,37181 +- 0,00129 seconds time elapsed  ( +-  0,09% )

[ perf record: Woken up 71 times to write data ]
[ perf record: Captured and wrote 19,285 MB perf.data (502927 samples) ]
```

# Results:

It can be seen that the C++ `std::vector<unique_ptr<T>>` and `ARRAYLIST` macro destructor are almost exactly the same, even though C++ vector being highly tuned for modern compilers and architectures with extra logic for internal performance.

The `ARRAYLIST_DYN` function pointer is a bit slower than the macro and C++ version, which is expected, but the difference is very slight, less than 1% runtime performance speed penalty.

As for the executable size on my machine, the `ARRAYLIST` is 15.7KiB, `ARRAYLIST_DYN` function pointer is 15.9KiB, while C++ `std::vector<unique_ptr<T>>` is 16.3KiB, compiling the C++ version with `-fno-exceptions -fno-rtti` it gets to 15.8KiB, which is not bad, all of this while I did not compile for binary size performance.

Note that I did not put much thought into the performance of the ARRAYLIST, particularly in the Custom Allocator interface, which are function pointers that the compiler can't optimize. These tests where done with plain libc malloc/realloc/free through this function pointer interface, while the new and delete from C++ may pool quick and fast allocations, so may be apples to oranges comparison here, with new/delete being natively more performant (maybe).

Even then, with plain malloc and with non pod types, `ARRAYLIST` is the same, sometimes faster than `std::vector`.

A custom allocator like jemalloc/tcmalloc/mimalloc may speed runtime performance here.

# Using a custom Allocator like jemalloc:

Quick test with jemalloc on the C ARRAYLIST version, code is minimally changed to add a custom allocator to use jemalloc, rest of the code is the same:

## Code for ARRAYLIST using jemalloc
```c
#include <stdio.h>

#define JEMALLOC_NO_DEMANGLE
#include <jemalloc/jemalloc.h>

#include "arraylist.h"

static inline void *jemalloc_malloc(size_t size, void *ctx) {
    (void)ctx;
    return je_malloc(size);
}

static inline void *jemalloc_realloc(void *ptr, size_t old_size, size_t new_size, void *ctx) {
    (void)ctx;
    (void)old_size;
    return je_realloc(ptr, new_size);
}

static inline void jemalloc_free(void *ptr, size_t size, void *ctx) {
    (void)ctx;
    (void)size;
    return je_free(ptr);
}

static inline Allocator allocator_get_jemalloc(void) {
    return (Allocator) {
        .malloc = jemalloc_malloc,
        .realloc = jemalloc_realloc,
        .free = jemalloc_free,
        .ctx = NULL,
    };
}

struct non_pod {
    int *_number;
    int *add;
    int *sub;
};

// -- snip --

ARRAYLIST(struct non_pod*, np, non_pod_deinit_ptr_macro)

int main(void) {
    Allocator jemalloc_allocator = allocator_get_jemalloc();

    struct arraylist_np vec_np = np_init(jemalloc_allocator);
    np_reserve(&vec_np, 1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        *np_emplace_back(&vec_np) = non_pod_init_alloc(&jemalloc_allocator, i, i * 3, i / 2);
    }

    // -- snip --
}
```

Compiled with: `-O3 -DNDEBUG -flto -ljemalloc`

Command used: `perf record perf stat -r 100`

## C Macro using jemalloc:
### First time (already did a run before this to make it hot on the cache):
```txt
 Performance counter stats for './cje' (100 runs):

     1.256.759.464      task-clock:u                     #    0,945 CPUs utilized               ( +-  0,09% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
               199      page-faults:u                    #  158,344 /sec                        ( +-  0,06% )
     1.697.415.197      instructions:u                   #    1,85  insn per cycle
                                                  #    0,22  stalled cycles per insn     ( +-  0,01% )
       915.305.285      cycles:u                         #    0,728 GHz                         ( +-  0,21% )
       369.449.222      stalled-cycles-frontend:u        #   40,36% frontend cycles idle        ( +-  0,40% )
       295.136.780      branches:u                       #  234,840 M/sec                       ( +-  0,01% )
        12.163.435      branch-misses:u                  #    4,12% of all branches             ( +-  0,08% )

           1,32933 +- 0,00126 seconds time elapsed  ( +-  0,10% )

[ perf record: Woken up 71 times to write data ]
[ perf record: Captured and wrote 18,853 MB perf.data (491306 samples) ]
```

### Second time:
```
 Performance counter stats for './cje' (100 runs):

     1.261.191.361      task-clock:u                     #    0,943 CPUs utilized               ( +-  0,12% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
               199      page-faults:u                    #  157,787 /sec                        ( +-  0,03% )
     1.697.573.121      instructions:u                   #    1,84  insn per cycle
                                                  #    0,22  stalled cycles per insn     ( +-  0,01% )
       923.253.453      cycles:u                         #    0,732 GHz                         ( +-  0,24% )
       374.958.770      stalled-cycles-frontend:u        #   40,61% frontend cycles idle        ( +-  0,44% )
       295.160.637      branches:u                       #  234,033 M/sec                       ( +-  0,01% )
        12.153.200      branch-misses:u                  #    4,12% of all branches             ( +-  0,01% )

           1,33730 +- 0,00164 seconds time elapsed  ( +-  0,12% )

[ perf record: Woken up 70 times to write data ]
[ perf record: Captured and wrote 18,880 MB perf.data (492003 samples) ]
```

## Results of the jemalloc allocator

As can be seen, it is now significantly faster than `std::vector`, being extremely easy to add custom allocator support.
