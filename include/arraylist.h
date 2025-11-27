/**
 * @file arraylist.h
 * @brief Generic arraylist implementation using macros
 */
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "allocator.h"

/**
 * @def ARRAYLIST_DEFINE(T, name)
 * @brief Defines a arraylist structure for a specific type T
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * This macro defines a struct named "arraylist_##name" with the following fields:
 * - "data": Pointer of type T to the array of elements
 * - "size": Current number of elements in the arraylist
 * - "capacity": Current capacity of the arraylist
 *
 * @code
 * // Example: Define a arraylist for integers
 * ARRAYLIST_DEFINE(int, int)
 * // Creates a struct named struct arraylist_int
 * @endcode
 *
 */
#define ARRAYLIST_DEFINE(T, name) \
struct arraylist_##name { \
    T *data; \
    size_t size; \
    size_t capacity; \
    Allocator *alloc; \
};

/**
 * @def ARRAYLIST_DECLARE(T, name)
 * @brief Declares all functions for a arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * Declares the following functions:
 * - arraylist_##name##_init()
 * - arraylist_##name##_deinit()
 * - arraylist_##name##_push_back()
 * - arraylist_##name##_pop_back()
 * - arraylist_##name##_at()
 * - arraylist_##name##_size()
 * - arraylist_##name##_capacity()
 * - arraylist_##name##_reserve()
 * - arraylist_##name##_clear()
 * 
 */
#define ARRAYLIST_DECLARE(T, name) \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc); \
void arraylist_##name##_deinit(struct arraylist_##name *vec); \
int arraylist_##name##_push_back(struct arraylist_##name *vec, T value); \
T arraylist_##name##_pop_back(struct arraylist_##name *vec); \
T* arraylist_##name##_at(struct arraylist_##name *vec, size_t index); \
size_t arraylist_##name##_size(const struct arraylist_##name *vec); \
size_t arraylist_##name##_capacity(const struct arraylist_##name *vec); \
int arraylist_##name##_reserve(struct arraylist_##name *vec, size_t new_capacity); \
void arraylist_##name##_clear(struct arraylist_##name *vec);

/**
 * @def ARRAYLIST_IMPLEMENT(T, name)
 * @brief Implements all functions for a arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * Implements the functions of the ARRAYLIST_DECLARE macro
 *
 * @note This macro should be used in a .c file, not in a header
 *
 */ 
#define ARRAYLIST_IMPLEMENT(T, name) \
/**
 * @brief Creates a new arraylist \
 * @param alloc Custom allocator instance, if null, default alloc will be used \
 * @return An initialized arraylist structure \
 * \
 * Creates a arraylist with initial cap of 8 elements \
 * If memory alloc fails, cap is set to 0 \
 * \
 */ \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc) { \
    struct arraylist_##name vec = {0}; \
    if (alloc) vec.alloc = alloc; else vec.alloc = allocator_get_default(); \
    vec.size = 0; \
    vec.capacity = 8; \
    vec.data = vec.alloc->malloc(vec.capacity * sizeof(T), vec.alloc->ctx); \
    if (!vec.data) { \
        fprintf(stderr, "Not able to allocate memory."); \
        vec.capacity = 0; \
    } \
    return vec; \
} \
\
/**
 * @brief Destroys and frees a arraylist \
 * @param vec Pointer to the arraylist to deinit \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on nullptr or already deinitialized arraylists \
 * \
 */ \
void arraylist_##name##_deinit(struct arraylist_##name *vec) { \
    if (vec && vec->data) { \
        vec->alloc->free(vec->data, vec->capacity * sizeof(T), vec->alloc->ctx); \
        vec->data = nullptr; \
        vec->size = 0; \
        vec->capacity = 0; \
    } \
} \
\
/**
 * @brief Adds a new element to the end of the array \
 * @param vec Pointer to the arraylist to add a new value \
 * @param T value to be added \
 * @return -1 on failure 0 on success \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on nullptr or already deinitialed arraylists \
 * \
 */ \
int arraylist_##name##_push_back(struct arraylist_##name *vec, T value) { \
    if (!vec) return -1; \
    if (vec->size >= vec->capacity) { \
        size_t new_capacity = vec->capacity * 2; \
        T *new_data = vec->alloc->realloc(vec->data, vec->capacity * sizeof(T), new_capacity * sizeof(T), vec->alloc->ctx); \
        if (!new_data) return -1; \
        vec->data = new_data; \
        vec->capacity = new_capacity; \
    } \
    vec->data[vec->size++] = value; \
    return 0; \
} \
\
/**
 * @brief Removes the last added element \
 * @param vec Pointer to the arraylist \
 * @return The value removed \
 * \
 * The function asserts if the vec isn't null and if the size is greater than 0 \
 * \
 */ \
T arraylist_##name##_pop_back(struct arraylist_##name *vec) { \
    assert(vec && vec->size > 0); \
    return vec->data[--vec->size]; \
} \
\
/**
 * @brief Accesses the position of the arraylist at index \
 * @param vec Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accesses or null if vec=null or index is greater than vec size \
 * \
 */ \
T* arraylist_##name##_at(struct arraylist_##name *vec, size_t index) { \
    if (!vec || index >= vec->size) return nullptr; \
    return &vec->data[index]; \
} \
\
/**
 * @brief Gets the size of a arraylist \
 * @param vec Pointer to the arraylist \
 * @return The size or 0 if vec is null \
 * \
 */ \
size_t arraylist_##name##_size(const struct arraylist_##name *vec) { \
    return vec ? vec->size : 0; \
} \
\
/**
 * @brief Gets the capacity of a arraylist \
 * @param vec Pointer to the arraylist \
 * @return The capacity or 0 if vec is null \
 * \
 */ \
size_t arraylist_##name##_capacity(const struct arraylist_##name *vec) { \
    return vec ? vec->capacity : 0; \
} \
\
/**
 * @brief Reserves the capacity of a arraylist \
 * @param vec Pointer to the arraylist \
 * @param new_capacity New capacity of the arraylist \
 * @return 1 if vec is null or vec's capacity is lesser or equals than new capacity \
 *         0 on success, -1 on reallocation failure \
 * \
 */ \
int arraylist_##name##_reserve(struct arraylist_##name *vec, size_t new_capacity) { \
    if (!vec || new_capacity <= vec->capacity) return 1; \
    T *new_data = vec->alloc->realloc(vec->data, vec->capacity * sizeof(T), new_capacity * sizeof(T), vec->alloc->ctx); \
    if (!new_data) return -1; \
    vec->data = new_data; \
    vec->capacity = new_capacity; \
    return 0; \
} \
\
/**
 * @brief Clears the arraylist's data \
 * @param vec Pointer to the arraylist \
 * \
 * It does not free anything \
 * \
 */ \
void arraylist_##name##_clear(struct arraylist_##name *vec) { \
    if (vec) vec->size = 0; \
} \

#define ARRAYLIST(T, name)\
ARRAYLIST_DEFINE(T, name) \
ARRAYLIST_DECLARE(T, name) \
ARRAYLIST_IMPLEMENT(T, name)

#endif // ARRAYLIST_H
