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
    struct Allocator gpa = allocator_get_default();

    struct arraylist_np vec_np = arraylist_np_init(&gpa);
    arraylist_np_reserve(&vec_np, 1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, i, i * 3, i / 2);
    }

    for (volatile size_t i = 0; i < 1000000; i++) {
        printf("Calculate returns %d\n", non_pod_calculate(vec_np.data[i]));
    }

    arraylist_np_deinit(&vec_np);
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
    struct Allocator gpa = allocator_get_default();

    struct arraylistfp_np vec_np = arraylistfp_np_init(&gpa, non_pod_deinit_ptr);
    arraylistfp_np_reserve(&vec_np, 1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        *arraylistfp_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, i, i * 3, i / 2);
    }

    for (volatile size_t i = 0; i < 1000000; i++) {
        printf("Calculate returns %d\n", non_pod_calculate(vec_np.data[i]));
    }

    arraylistfp_np_deinit(&vec_np);
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

     1.290.042.196      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,09% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.730      page-faults:u                    #   24,596 K/sec                       ( +-  0,00% )
     2.086.190.560      instructions:u                   #    2,22  insn per cycle
                                                  #    0,18  stalled cycles per insn     ( +-  0,00% )
       940.407.072      cycles:u                         #    0,729 GHz                         ( +-  0,20% )
       374.427.871      stalled-cycles-frontend:u        #   39,82% frontend cycles idle        ( +-  0,38% )
       442.781.894      branches:u                       #  343,231 M/sec                       ( +-  0,00% )
        12.116.514      branch-misses:u                  #    2,74% of all branches             ( +-  0,00% )

           1,36616 +- 0,00119 seconds time elapsed  ( +-  0,09% )

[ perf record: Woken up 71 times to write data ]
[ perf record: Captured and wrote 19,184 MB perf.data (501162 samples) ]
```

### Second time:
```
 Performance counter stats for './c' (100 runs):

     1.292.967.989      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,10% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.730      page-faults:u                    #   24,540 K/sec                       ( +-  0,00% )
     2.086.190.579      instructions:u                   #    2,21  insn per cycle
                                                  #    0,18  stalled cycles per insn     ( +-  0,00% )
       945.687.916      cycles:u                         #    0,731 GHz                         ( +-  0,22% )
       377.158.829      stalled-cycles-frontend:u        #   39,88% frontend cycles idle        ( +-  0,41% )
       442.781.900      branches:u                       #  342,454 M/sec                       ( +-  0,00% )
        12.136.578      branch-misses:u                  #    2,74% of all branches             ( +-  0,12% )

           1,36940 +- 0,00140 seconds time elapsed  ( +-  0,10% )

[ perf record: Woken up 72 times to write data ]
[ perf record: Captured and wrote 19,239 MB perf.data (502586 samples) ]
```

## C `ARRAYLIST_DYN` Function Pointer version
### First time (already did a run before this to make it hot on the cache):
```txt
 Performance counter stats for './cfp' (100 runs):

     1.305.094.905      task-clock:u                     #    0,944 CPUs utilized               ( +-  0,11% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.730      page-faults:u                    #   24,312 K/sec                       ( +-  0,00% )
     2.108.190.532      instructions:u                   #    2,14  insn per cycle
                                                  #    0,18  stalled cycles per insn     ( +-  0,00% )
       984.217.394      cycles:u                         #    0,754 GHz                         ( +-  0,22% )
       384.144.910      stalled-cycles-frontend:u        #   39,03% frontend cycles idle        ( +-  0,40% )
       447.781.953      branches:u                       #  343,103 M/sec                       ( +-  0,00% )
        12.118.143      branch-misses:u                  #    2,71% of all branches             ( +-  0,01% )

           1,38229 +- 0,00153 seconds time elapsed  ( +-  0,11% )

[ perf record: Woken up 73 times to write data ]
[ perf record: Captured and wrote 19,380 MB perf.data (506267 samples) ]
```

### Second time:
```
 Performance counter stats for './cfp' (100 runs):

     1.302.618.214      task-clock:u                     #    0,943 CPUs utilized               ( +-  0,09% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.730      page-faults:u                    #   24,359 K/sec                       ( +-  0,00% )
     2.108.190.562      instructions:u                   #    2,14  insn per cycle
                                                  #    0,18  stalled cycles per insn     ( +-  0,00% )
       983.477.040      cycles:u                         #    0,755 GHz                         ( +-  0,22% )
       381.931.151      stalled-cycles-frontend:u        #   38,83% frontend cycles idle        ( +-  0,39% )
       447.781.971      branches:u                       #  343,755 M/sec                       ( +-  0,00% )
        12.127.169      branch-misses:u                  #    2,71% of all branches             ( +-  0,08% )

           1,38099 +- 0,00143 seconds time elapsed  ( +-  0,10% )

[ perf record: Woken up 74 times to write data ]
[ perf record: Captured and wrote 19,425 MB perf.data (507443 samples) ]
```

## C++
### First run (already did a run before this to make it hot on the cache):
```txt
 Performance counter stats for './cpp' (100 runs):

     1.286.880.232      task-clock:u                     #    0,938 CPUs utilized               ( +-  0,10% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.802      page-faults:u                    #   24,712 K/sec                       ( +-  0,00% )
     2.174.614.877      instructions:u                   #    2,33  insn per cycle
                                                  #    0,18  stalled cycles per insn     ( +-  0,00% )
       933.337.106      cycles:u                         #    0,725 GHz                         ( +-  0,18% )
       383.611.949      stalled-cycles-frontend:u        #   41,10% frontend cycles idle        ( +-  0,33% )
       466.200.816      branches:u                       #  362,272 M/sec                       ( +-  0,00% )
        12.163.199      branch-misses:u                  #    2,61% of all branches             ( +-  0,00% )

           1,37206 +- 0,00152 seconds time elapsed  ( +-  0,11% )

[ perf record: Woken up 70 times to write data ]
[ perf record: Captured and wrote 19,251 MB perf.data (502037 samples) ]
```

### Second time:
```txt
 Performance counter stats for './cpp' (100 runs):

     1.284.196.414      task-clock:u                     #    0,943 CPUs utilized               ( +-  0,09% )
                 0      context-switches:u               #    0,000 /sec
                 0      cpu-migrations:u                 #    0,000 /sec
            31.802      page-faults:u                    #   24,764 K/sec                       ( +-  0,00% )
     2.174.614.864      instructions:u                   #    2,33  insn per cycle
                                                  #    0,18  stalled cycles per insn     ( +-  0,00% )
       933.468.796      cycles:u                         #    0,727 GHz                         ( +-  0,19% )
       385.314.822      stalled-cycles-frontend:u        #   41,28% frontend cycles idle        ( +-  0,33% )
       466.200.811      branches:u                       #  363,029 M/sec                       ( +-  0,00% )
        12.162.517      branch-misses:u                  #    2,61% of all branches             ( +-  0,00% )

           1,36208 +- 0,00129 seconds time elapsed  ( +-  0,09% )

[ perf record: Woken up 72 times to write data ]
[ perf record: Captured and wrote 19,223 MB perf.data (501310 samples) ]
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

ARRAYLIST(struct non_pod*, np, nptrdeinit_ptr_macro)

int main(void) {
    Allocator jemalloc_allocator = allocator_get_jemalloc();

    struct arraylist_np vec_np = arraylist_np_init(&jemalloc_allocator);
    arraylist_np_reserve(&vec_np, 1000000);
    for (volatile size_t i = 0; i < 1000000; i++) {
        *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&jemalloc_allocator, i, i * 3, i / 2);
    }

    // -- snip --
}
```

Compiled with: `-O3 -DNDEBUG -flto -ljemalloc`

Command used: `perf record perf stat -r 100`

## C Macro using jemalloc:
### First time (already did a run before this to make it hot on the cache):
```txt

 Performance counter stats for './c' (100 runs):

     1.247.097.316      task-clock:u                     #    0,936 CPUs utilized               ( +-  0,13% )
                 0      context-switches:u               #    0,000 /sec                      
                 0      cpu-migrations:u                 #    0,000 /sec                      
               199      page-faults:u                    #  159,571 /sec                        ( +-  0,05% )
     1.680.869.863      instructions:u                   #    1,87  insn per cycle            
                                                  #    0,22  stalled cycles per insn     ( +-  0,01% )
       898.335.034      cycles:u                         #    0,720 GHz                         ( +-  0,32% )
       373.551.599      stalled-cycles-frontend:u        #   41,58% frontend cycles idle        ( +-  0,35% )
       292.200.526      branches:u                       #  234,305 M/sec                       ( +-  0,01% )
        12.155.504      branch-misses:u                  #    4,16% of all branches             ( +-  0,05% )

           1,33166 +- 0,00182 seconds time elapsed  ( +-  0,14% )

[ perf record: Woken up 69 times to write data ]
[ perf record: Captured and wrote 18,729 MB perf.data (488078 samples) ]
```

### Second time:
```

 Performance counter stats for './c' (100 runs):

     1.242.906.156      task-clock:u                     #    0,937 CPUs utilized               ( +-  0,14% )
                 0      context-switches:u               #    0,000 /sec                      
                 0      cpu-migrations:u                 #    0,000 /sec                      
               198      page-faults:u                    #  159,304 /sec                        ( +-  0,04% )
     1.680.678.592      instructions:u                   #    1,88  insn per cycle            
                                                  #    0,22  stalled cycles per insn     ( +-  0,01% )
       894.793.298      cycles:u                         #    0,720 GHz                         ( +-  0,38% )
       371.860.118      stalled-cycles-frontend:u        #   41,56% frontend cycles idle        ( +-  0,43% )
       292.178.183      branches:u                       #  235,077 M/sec                       ( +-  0,01% )
        12.149.868      branch-misses:u                  #    4,16% of all branches             ( +-  0,01% )

           1,32663 +- 0,00201 seconds time elapsed  ( +-  0,15% )

[ perf record: Woken up 70 times to write data ]
[ perf record: Captured and wrote 18,601 MB perf.data (484715 samples) ]
```

## Results of the jemalloc allocator

As can be seen, it is now significantly faster than `std::vector`, being extremely easy to add custom allocator support.
