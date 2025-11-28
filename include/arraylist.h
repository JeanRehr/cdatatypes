/**
 * @file arraylist.h
 * @brief Generic arraylist implementation using macros
 * @details The arraylist will own and free the memory, as long as the correct destructor for the
 *          object is provided, and the deinit functions called correctly, once an object is inside
 *          the arraylist, do not free it, call the functions of the arraylist that frees it
 */
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"

/**
 * @def ARRAYLIST_DEFINE(T, name)
 * @brief Defines an arraylist structure for a specific type T
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * This macro defines a struct named "arraylist_##name" with the following fields:
 * - "data": Pointer of type T to the array of elements
 * - "size": Current number of elements in the arraylist
 * - "capacity": Current capacity of the arraylist
 *
 * @code
 * // Example: Define an arraylist for integers
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
    void (*destructor)(T *); \
};

/**
 * @def ARRAYLIST_DECLARE(T, name)
 * @brief Declares all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * Declares the following functions:
 * - arraylist_##name##_init()
 * - arraylist_##name##_reserve()
 * - arraylist_##name##_push_back()
 * - arraylist_##name##_emplace_back_slot()
 * - arraylist_##name##_pop_back()
 * - arraylist_##name##_at()
 * - arraylist_##name##_begin()
 * - arraylist_##name##_back()
 * - arraylist_##name##_end()
 * - arraylist_##name##_size()
 * - arraylist_##name##_is_empty()
 * - arraylist_##name##_capacity()
 * - arraylist_##name##_clear()
 * - arraylist_##name##_deinit()
 * 
 * @todo { WILL IMPLEMENT:
 * insert_at() - Inserts at given index
 * remove_at() - Removes at given index
 * insert_elements_at() - Inserts a number of elements at given index
 * remove_elements_at() - Removes a number of elements at given index
 * resize() - Changes the size of a vector, adding or removing elements if necessary
 * shrink_to_fit() - Reduces the reserved memory of a vector if necessary to exactly fit the number of elements
 * swap() - Swaps the contents of one vector with another with the same type T
 * }
 */
#define ARRAYLIST_DECLARE(T, name) \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)); \
int arraylist_##name##_reserve(struct arraylist_##name *arraylist, size_t new_capacity); \
int arraylist_##name##_push_back(struct arraylist_##name *arraylist, T value); \
T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *arraylist); \
void arraylist_##name##_pop_back(struct arraylist_##name *arraylist); \
T* arraylist_##name##_at(struct arraylist_##name *arraylist, size_t index); \
T* arraylist_##name##_begin(struct arraylist_##name *arraylist); \
T* arraylist_##name##_back(struct arraylist_##name *arraylist); \
T* arraylist_##name##_end(struct arraylist_##name *arraylist); \
size_t arraylist_##name##_size(const struct arraylist_##name *arraylist); \
bool arraylist_##name##_is_empty(const struct arraylist_##name *arraylist); \
size_t arraylist_##name##_capacity(const struct arraylist_##name *arraylist); \
void arraylist_##name##_clear(struct arraylist_##name *arraylist); \
void arraylist_##name##_deinit(struct arraylist_##name *arraylist);

/**
 * @def ARRAYLIST_IMPLEMENT(T, name)
 * @brief Implements all functions for an arraylist type
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
 * Creates an arraylist with initial cap of 8 elements \
 * If memory alloc fails, cap is set to 0 \
 * \
 */ \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)) { \
    struct arraylist_##name arraylist = {0}; \
    if (alloc) arraylist.alloc = alloc; else arraylist.alloc = allocator_get_default(); \
    arraylist.destructor = destructor; \
    arraylist.size = 0; \
    arraylist.capacity = 1; \
    arraylist.data = arraylist.alloc->malloc(arraylist.capacity * sizeof(T), arraylist.alloc->ctx); \
    if (!arraylist.data) { \
        fprintf(stderr, "Not able to allocate memory."); \
        arraylist.capacity = 0; \
    } \
    return arraylist; \
} \
\
/**
 * @brief Reserves the capacity of an arraylist \
 * @param arraylist Pointer to the arraylist \
 * @param capacity New capacity of the arraylist \
 * @return 1 if arraylist is null or arraylist's capacity is lesser or equals than new capacity \
 *         0 on success, -1 on reallocation failure \
 * \
 */ \
int arraylist_##name##_reserve(struct arraylist_##name *arraylist, size_t capacity) { \
    if (!arraylist || capacity <= arraylist->capacity) return 1; \
    T *new_data = arraylist->alloc->realloc(arraylist->data, arraylist->capacity * sizeof(T), capacity * sizeof(T), arraylist->alloc->ctx); \
    if (!new_data) return -1; \
    arraylist->data = new_data; \
    arraylist->capacity = capacity; \
    return 0; \
} \
\
/**
 * @brief Adds a new element to the end of the array \
 * @param arraylist Pointer to the arraylist to add a new value \
 * @param T value to be added \
 * @return -1 on failure 0 on success \
 * \
 * Safe to call on nullptr or already deinitialed arraylists \
 * \
 * @note Will not construct objects in place, objects must be constructed \
 * \
 */ \
int arraylist_##name##_push_back(struct arraylist_##name *arraylist, T value) { \
    if (!arraylist) return -1; \
    if (arraylist->size >= arraylist->capacity) { \
        size_t new_capacity = arraylist->capacity * 2; \
        T *new_data = arraylist->alloc->realloc(arraylist->data, arraylist->capacity * sizeof(T), new_capacity * sizeof(T), arraylist->alloc->ctx); \
        if (!new_data) return -1; \
        arraylist->data = new_data; \
        arraylist->capacity = new_capacity; \
    } \
    arraylist->data[arraylist->size++] = value; \
    return 0; \
} \
\
/**
 * @brief Returns a slot on the arraylist for an object to be constructed \
 * @param arraylist Pointer to the arraylist \
 * @return The slot or nullptr if the arraylist isn't initialized or if the reallocation fails\
 * \
 * @code \
 * struct Foo { int a; }; \
 * // Initialize ... \
 * struct Foo *slot = arraylist_Foo_emplace_back_slot(&mylist); \
 * // Now slot is a valid pointer for writing into a new element. For example: \
 * slot->a = 42; // or call a constructor on slot \
 * @endcode \
 */ \
T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *arraylist) { \
    if (!arraylist) return nullptr; \
    if (arraylist->size >= arraylist->capacity) { \
        size_t new_capacity = arraylist->capacity * 2; \
        T *new_data = arraylist->alloc->realloc(arraylist->data, arraylist->capacity * sizeof(T), new_capacity * sizeof(T), arraylist->alloc->ctx); \
        if (!new_data) return nullptr; \
        arraylist->data = new_data; \
        arraylist->capacity = new_capacity; \
    } \
    return &arraylist->data[arraylist->size++]; \
} \
\
/**
 * @brief Removes the last added element \
 * @param arraylist Pointer to the arraylist \
 * \
 * Does nothing on an empty arraylist \
 * \
 * @note The object will be destructed if a destructor is provided durint arraylist init \
 * \
 */ \
void arraylist_##name##_pop_back(struct arraylist_##name *arraylist) { \
    if (!arraylist || arraylist->size <= 0) return; \
    if (arraylist->destructor) arraylist->destructor(&arraylist->data[--arraylist->size]); \
    --arraylist->size; \
} \
\
/**
 * @brief Accesses the position of the arraylist at index \
 * @param arraylist Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is greater than arraylist size \
 * \
 */ \
T* arraylist_##name##_at(struct arraylist_##name *arraylist, size_t index) { \
    if (!arraylist || index >= arraylist->size) return nullptr; \
    return &arraylist->data[index]; \
} \
\
/**
 * @brief Accesses the first element of the arraylist \
 * @details It returns the block of memory allocated, can be used to iterate, \
 *          and where a function accepts a *T \
 * @param arraylist Pointer to the arraylist \
 * @return A pointer to the first value or null if !arraylist or size is less than or equal to 0 \
 * \
 * @code \
 * // Prints the contents of an arraylist of T (substitute T for your type) \
 * for (T *it = arraylist_t_begin(); it != arraylist_t_end(); ++it) { t_print(); } \
 * @endcode \
 * \
 */ \
T* arraylist_##name##_begin(struct arraylist_##name *arraylist) { \
    if (!arraylist || arraylist->size <= 0) return nullptr; \
    return &arraylist->data[0]; \
} \
\
/**
 * @brief Accesses the last position of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin, \
 *          but it will not be the end of the arraylist, just the last element, can be dereferenced \
 * @param arraylist Pointer to the arraylist \
 * @return A pointer to the last value or null if !arraylist or size is less than or equal to 0 \
 * \
 */ \
T* arraylist_##name##_back(struct arraylist_##name *arraylist) { \
    if (!arraylist || arraylist->size <= 0) return nullptr; \
    return &arraylist->data[arraylist->size - 1]; \
} \
\
/**
 * @brief Accesses the end of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin \
 * @param arraylist Pointer to the arraylist \
 * @return A pointer to the end or null if !arraylist or size is less than or equal to 0 \
 * \
 * @warning Dereferencing it leads to undefined behavior \
 * \
 */ \
T* arraylist_##name##_end(struct arraylist_##name *arraylist) { \
    if (!arraylist || arraylist->size <= 0) return nullptr; \
    return &arraylist->data[arraylist->size]; \
} \
\
/**
 * @brief Gets the size of an arraylist \
 * @param arraylist Pointer to the arraylist \
 * @return The size or 0 if arraylist is null \
 * \
 */ \
size_t arraylist_##name##_size(const struct arraylist_##name *arraylist) { \
    return arraylist ? arraylist->size : 0; \
} \
/**
 * @brief Checks if the arraylist is empty \
 * @param arraylist Pointer to the arraylist \
 * @return False if arraylist is null or size = 0, otherwise true \
 * \
 */ \
bool arraylist_##name##_is_empty(const struct arraylist_##name *arraylist) { \
    if (!arraylist) return false; \
    return arraylist->size == 0 ? true : false; \
} \
\
/**
 * @brief Gets the capacity of an arraylist \
 * @param arraylist Pointer to the arraylist \
 * @return The capacity or 0 if arraylist is null \
 * \
 */ \
size_t arraylist_##name##_capacity(const struct arraylist_##name *arraylist) { \
    return arraylist ? arraylist->capacity : 0; \
} \
\
/**
 * @brief Clears the arraylist's data \
 * @param arraylist Pointer to the arraylist \
 * \
 * It does not free the arraylist itself and does not alter the capacity, only sets it size to 0 \
 * Safe to call on nullptr, returns early \
 * \
 * @note Will call the object's destructor on objects if available \
 * \
 */ \
void arraylist_##name##_clear(struct arraylist_##name *arraylist) { \
    if (!arraylist) return; \
    if (arraylist->destructor) { \
        for (size_t i = 0; i < arraylist->size; ++i) { \
            arraylist->destructor(&arraylist->data[i]); \
        } \
    arraylist->size = 0; \
    } \
} \
\
/**
 * @brief Destroys and frees an arraylist \
 * @param arraylist Pointer to the arraylist to deinit \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on nullptr or already deinitialized arraylists, returns early \
 * \
 * @note Will call the destructor on data items if provided \
 * \
 */ \
void arraylist_##name##_deinit(struct arraylist_##name *arraylist) { \
    if (!arraylist || !arraylist->data) return; \
    if (arraylist->destructor) { \
        for (size_t i = 0; i < arraylist->size; ++i) { \
            arraylist->destructor(&arraylist->data[i]); \
        } \
    } \
    arraylist->alloc->free(arraylist->data, arraylist->capacity * sizeof(T), arraylist->alloc->ctx); \
    arraylist->data = nullptr; \
    arraylist->size = 0; \
    arraylist->capacity = 0; \
} \

/**
 * @def ARRAYLIST(T, name)
 * @brief Helper macro to define, declare and implement all in one \
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 */
#define ARRAYLIST(T, name)\
ARRAYLIST_DEFINE(T, name) \
ARRAYLIST_DECLARE(T, name) \
ARRAYLIST_IMPLEMENT(T, name)

#endif // ARRAYLIST_H
