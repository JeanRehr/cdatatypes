/**
 * @file allocator.h
 * @brief Single-header allocator interface to support custom allocators. 
 */
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

/**
 * @struct Allocator
 * @brief Allocator interface for different Allocators 
 *
 */
typedef struct Allocator {
    void *(*malloc)(size_t size, void *ctx); ///< Function poitner that allocates memory, takes a ctx if needed
    void *(*realloc)(void *ptr, size_t old_size, size_t new_size, void *ctx); ///< Realocates the given pointer
    void (*free)(void *ptr, size_t size, void *ctx); ///< Frees the given pointer
    void *ctx; ///< Context in case it is needed
} Allocator;

/**
 * @brief Default malloc, uses the global malloc standard C function
 */
static inline void *default_malloc(size_t size, void *ctx) {
    (void)ctx;
    return malloc(size);
}

/**
 * @brief Default realloc, uses the global realloc standard C function
 */
static inline void *default_realloc(void *ptr, size_t old_size, size_t new_size, void *ctx) {
    (void)old_size;
    (void)ctx;
    return realloc(ptr, new_size);
}

/**
 * @brief Default free, uses the global free standard C function
 */
static inline void default_free(void *ptr, size_t size, void *ctx) {
    (void)size;
    (void)ctx;
    free(ptr);
}

/**
 * @brief function that returns a default allocator
 *
 * @return An Allocator defaulted to malloc, realloc, and free
 */
static inline Allocator allocator_get_default(void) {
    return (Allocator) {
        .malloc = default_malloc,
        .realloc = default_realloc,
        .free = default_free,
        .ctx = nullptr,
    };
}

#endif // ALLOCATOR_H
