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
 * - arraylist_##name##_shrink_size()
 * - arraylist_##name##_shrink_to_fit()
 * - arraylist_##name##_push_back()
 * - arraylist_##name##_emplace_back_slot()
 * - arraylist_##name##_insert_at()
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
 * remove_at() - Removes at given index
 * insert_from_to() - Inserts a number of elements at given index
 * remove_from_to() - Removes a number of elements at given index
 * }
 */
#define ARRAYLIST_DECLARE(T, name) \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)); \
int arraylist_##name##_reserve(struct arraylist_##name *self, size_t capacity); \
void arraylist_##name##_shrink_size(struct arraylist_##name *self, size_t size); \
int arraylist_##name##_shrink_to_fit(struct arraylist_##name *self); \
int arraylist_##name##_push_back(struct arraylist_##name *self, T value); \
T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self); \
int arraylist_##name##_insert_at(struct arraylist_##name *self, T value, size_t index); \
void arraylist_##name##_pop_back(struct arraylist_##name *self); \
T* arraylist_##name##_at(struct arraylist_##name *self, size_t index); \
T* arraylist_##name##_begin(struct arraylist_##name *self); \
T* arraylist_##name##_back(struct arraylist_##name *self); \
T* arraylist_##name##_end(struct arraylist_##name *self); \
size_t arraylist_##name##_size(const struct arraylist_##name *self); \
bool arraylist_##name##_is_empty(const struct arraylist_##name *self); \
size_t arraylist_##name##_capacity(const struct arraylist_##name *self); \
void arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other); \
void arraylist_##name##_clear(struct arraylist_##name *self); \
void arraylist_##name##_deinit(struct arraylist_##name *self);

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
int arraylist_##name##_reserve(struct arraylist_##name *self, size_t capacity) { \
    if (!self || capacity <= self->capacity) return 1; \
    T *new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), capacity * sizeof(T), self->alloc->ctx); \
    if (!new_data) return -1; \
    self->data = new_data; \
    self->capacity = capacity; \
    return 0; \
} \
\
/**
 * @brief Shrinks the arraylist to size, removing elements if necessary \
 * @param arraylist Pointer to the arraylist \
 * @param size New size of the arraylist \
 * \
 * Returns early if arraylist is null or arraylist size is <= 0 or if arraylist size is <= size \
 *         or if provided size is <= 0 \
 * \
 */ \
void arraylist_##name##_shrink_size(struct arraylist_##name *self, size_t size) { \
    if (!self || self->size <= 0 || self->size <= size) return; \
    T *it_atsize = arraylist_##name##_at(self, size); \
    for (T *it = arraylist_##name##_end(self); it > it_atsize; --it) { \
        if (self->destructor) self->destructor(&self->data[self->size - 1]); \
        --self->size; \
    } \
} \
\
/**
 * @brief Shrinks the capacity of the arraylist to fit the size, may reallocate and does not remove elements \
 * @param arraylist Pointer to the arraylist \
 * @return 0 if successful, 1 on noop, and -1 on allocation failure \
 * Returns early if arraylist is null or arraylist capacity == size \
 * \
 */ \
int arraylist_##name##_shrink_to_fit(struct arraylist_##name *self) { \
    if (!self || self->capacity == self->size) return 1; \
    if (self->size == 0) { \
        self->alloc->free(self->data, self->capacity * sizeof(T), self->alloc->ctx); \
        self->data = nullptr; \
        self->capacity = 0; \
        return 0; \
    }\
    T *new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), self->size * sizeof(T), self->alloc->ctx); \
    if (!new_data) return -1; \
    self->data = new_data; \
    self->capacity = self->size; \
    return 0; \
}\
\
/**
 * @brief Adds a new element to the end of the array \
 * @param arraylist Pointer to the arraylist to add a new value \
 * @param T value to be added \
 * @return -1 on failure 0 on success \
 * \
 * Safe to call on nullptr or already deinitialed arraylists \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @note Will not construct objects in place, objects must be constructed \
 * \
 */ \
int arraylist_##name##_push_back(struct arraylist_##name *self, T value) { \
    if (!self) return -1; \
    if (self->size >= self->capacity) { \
        size_t new_capacity = self->capacity * 2; \
        T *new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), new_capacity * sizeof(T), self->alloc->ctx); \
        if (!new_data) return -1; \
        self->data = new_data; \
        self->capacity = new_capacity; \
    } \
    self->data[self->size++] = value; \
    return 0; \
} \
\
/**
 * @brief Returns a slot at the end of the arraylist for an object to be constructed \
 * @param arraylist Pointer to the arraylist \
 * @return The slot or nullptr if the arraylist isn't initialized or if the reallocation fails \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @code \
 * struct Foo { int a; }; \
 * // Initialize ... \
 * struct Foo *slot = arraylist_Foo_emplace_back_slot(&mylist); \
 * // Now slot is a valid pointer for writing into a new element. For example: \
 * slot->a = 42; // or call a constructor on slot \
 * @endcode \
 */ \
T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self) { \
    if (!self) return nullptr; \
    if (self->size >= self->capacity) { \
        size_t new_capacity = self->capacity * 2; \
        T *new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), new_capacity * sizeof(T), self->alloc->ctx); \
        if (!new_data) return nullptr; \
        self->data = new_data; \
        self->capacity = new_capacity; \
    } \
    return &self->data[self->size++]; \
} \
\
/**
 * @brief Inserts an element in the given index \
 * @param arraylist Pointer to the arraylist \
 * @param value Value of type T to be inserted \
 * @param index Index to insert \
 * @return -1 on failure 0 on success \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @warning if index < 0 size_t overflows and inserts at end \
 * \
 */ \
int arraylist_##name##_insert_at(struct arraylist_##name *self, T value, size_t index) {\
    if (!self) return -1; \
    if (index >= self->size) { \
        if (arraylist_##name##_push_back(self, value) == 0) return 0; \
        else return -1 ; \
    } \
    ++self->size; \
    if (self->size >= self->capacity) { \
        size_t new_capacity = self->capacity * 2; \
        T *new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), new_capacity * sizeof(T), self->alloc->ctx); \
        if (!new_data) return -1; \
        self->data = new_data; \
        self->capacity = new_capacity; \
    } \
    for (size_t i = self->size - 1; i >= index; --i) { \
        self->data[i + 1] = self->data[i]; \
        if (i == 0) break; \
    } \
    self->data[index] = value; \
    return 0; \
}\
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
void arraylist_##name##_pop_back(struct arraylist_##name *self) { \
    if (!self || self->size <= 0) return; \
    if (self->destructor) self->destructor(&self->data[self->size - 1]); \
    --self->size; \
} \
\
/**
 * @brief Accesses the position of the arraylist at index \
 * @param arraylist Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is greater than arraylist size \
 * \
 */ \
T* arraylist_##name##_at(struct arraylist_##name *self, size_t index) { \
    if (!self || index >= self->size) return nullptr; \
    return &self->data[index]; \
} \
\
/**
 * @brief Accesses the first element of the arraylist \
 * @details It returns the block of memory allocated, can be used to iterate, \
 *          and where a function accepts a *T \
 * @param arraylist Pointer to the arraylist \
 * @return A pointer to the first value or null if !self \
 * \
 * @code \
 * // Prints the contents of an arraylist of T (substitute T for your type) \
 * for (T *it = arraylist_t_begin(); it != arraylist_t_end(); ++it) { t_print(); } \
 * @endcode \
 * \
 */ \
T* arraylist_##name##_begin(struct arraylist_##name *self) { \
    if (!self) return nullptr; \
    return self->data; \
} \
\
/**
 * @brief Accesses the last position of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin, \
 *          but it will not be the end of the arraylist, just the last element, can be dereferenced \
 * @param arraylist Pointer to the arraylist \
 * @return A pointer to the last value or null if !self \
 * \
 */ \
T* arraylist_##name##_back(struct arraylist_##name *self) { \
    if (!self) return nullptr; \
    return self->data + (self->size - 1); \
} \
\
/**
 * @brief Accesses the end of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin \
 * @param arraylist Pointer to the arraylist \
 * @return A pointer to the end or null if !self \
 * \
 * @warning Dereferencing it leads to undefined behavior \
 * \
 */ \
T* arraylist_##name##_end(struct arraylist_##name *self) { \
    if (!self) return nullptr; \
    return self->data + self->size; \
} \
\
/**
 * @brief Gets the size of an arraylist \
 * @param arraylist Pointer to the arraylist \
 * @return The size or 0 if arraylist is null \
 * \
 */ \
size_t arraylist_##name##_size(const struct arraylist_##name *self) { \
    return self ? self->size : 0; \
} \
/**
 * @brief Checks if the arraylist is empty \
 * @param arraylist Pointer to the arraylist \
 * @return False if arraylist is null or size = 0, otherwise true \
 * \
 */ \
bool arraylist_##name##_is_empty(const struct arraylist_##name *self) { \
    if (!self) return false; \
    return self->size == 0 ? true : false; \
} \
\
/**
 * @brief Gets the capacity of an arraylist \
 * @param arraylist Pointer to the arraylist \
 * @return The capacity or 0 if arraylist is null \
 * \
 */ \
size_t arraylist_##name##_capacity(const struct arraylist_##name *self) { \
    return self ? self->capacity : 0; \
} \
\
void arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other) { \
    if (!self || !other) return; \
    struct arraylist_##name temp = *other; \
    *other = *self; \
    *self = temp; \
} \
\
/**
 * @brief Clears the arraylist's data \
 * @param arraylist Pointer to the arraylist \
 * \
 * It does not free the arraylist itself and does not alter the capacity, only sets its size to 0 \
 * Safe to call on nullptr, returns early \
 * \
 * @note Will call the object's destructor on objects if available \
 * \
 */ \
void arraylist_##name##_clear(struct arraylist_##name *self) { \
    if (!self) return; \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    self->size = 0; \
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
void arraylist_##name##_deinit(struct arraylist_##name *self) { \
    if (!self || !self->data) return; \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->alloc->free(self->data, self->capacity * sizeof(T), self->alloc->ctx); \
    self->data = nullptr; \
    self->size = 0; \
    self->capacity = 0; \
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
