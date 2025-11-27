
#include "allocator.h"

#include <stdlib.h>

/**
 * @brief Default malloc, uses the global malloc standard C function
 */
static void *default_malloc(size_t size, void *ctx) {
    (void)ctx;
    return malloc(size);
}

/**
 * @brief Default realloc, uses the global realloc standard C function
 */
static void *default_realloc(void *ptr, size_t old_size, size_t new_size, void *ctx) {
    (void)old_size;
    (void)ctx;
    return realloc(ptr, new_size);
}

/**
 * @brief Default free, uses the global free standard C function
 */
static void default_free(void *ptr, size_t size, void *ctx) {
    (void)size;
    (void)ctx;
    free(ptr);
}

void allocator_init_default(Allocator *alloc) {
    *alloc = (Allocator) {
        .malloc = default_malloc,
        .realloc = default_realloc,
        .free = default_free,
        .ctx = nullptr,
    };
}
 
/**
 * Global default allocator using the default malloc, realloc, and free from standard c library
 */
static Allocator default_alloc = {
    .malloc = default_malloc,
    .realloc = default_realloc,
    .free = default_free,
    .ctx = NULL
};

Allocator *allocator_get_default(void) {
    return &default_alloc;
}
