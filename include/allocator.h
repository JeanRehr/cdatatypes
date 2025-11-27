#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

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
 * @brief Initializes an Allocator to default free, realloc, and malloc from standard C library
 * 
 * @param alloc An uninitialized Allocator
 *
 * @note ctx pointer is defaulted to nullptr
 */
void allocator_init_default(Allocator *alloc);

/**
 * @brief function that returns a default allocator
 *
 * @return A global Allocator defaulted to malloc, realloc, and free
 */
Allocator *allocator_get_default(void);

#endif // ALLOCATOR_H
