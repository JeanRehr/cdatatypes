/**
 * @file arraylist.h
 * @brief Generic arraylist implementation using macros
 * @details The arraylist will own and free the memory, as long as the correct destructor for the
 *          object is provided, and the deinit function called correctly, once an object is inside
 *          the arraylist, do not free it, call the function of the arraylist that frees it,
 *          in the case of a container of pointers of an object, pass the correct destructor that
 *          knows how to free its data inside, and frees the struct itself
 */
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdint.h>

#include "allocator.h"

#define INITIAL_CAP 1

/**
 * @enum arraylist_error
 * @brief Error codes for the arraylist
 *
 */
enum arraylist_error {
    ARRAYLIST_OK = 0,             ///< No error
    ARRAYLIST_ERR_NULL = -1,      ///< Null pointer
    ARRAYLIST_ERR_OVERFLOW = -2,  ///< Buffer will overflow
    ARRAYLIST_ERR_ALLOC = -3,     ///< Allocation failure
};

/**
 * @def ARRAYLIST_USE_ASSERT
 * @brief Defines if the arraylist will use asserts or return error codes
 * @details If ARRAYLIST_USE_ASSERT is 1, then the lib will assert and fail early, otherwise,
 *          defensive programming and returning error codes will be used
 *
 */
#ifndef ARRAYLIST_USE_ASSERT
#define ARRAYLIST_USE_ASSERT 0
#endif // ARRAYLIST_USE_ASSERT
#include <assert.h>

#if ARRAYLIST_USE_ASSERT
#   define ARRAYLIST_ENSURE(cond, errcode)   assert(cond);
#   define ARRAYLIST_ENSURE_PTR(cond)        assert(cond);
#else
#   define ARRAYLIST_ENSURE(cond, errcode)    if (!(cond)) return (errcode);
#   define ARRAYLIST_ENSURE_PTR(cond)         if (!(cond)) return nullptr;
#endif // ARRAYLIST_USE_ASSERT if directive

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
 * - arraylist_##name##_remove_at()
 * - arraylist_##name##_remove_from_to()
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
 * @todo 
 * insert_from_to() - Inserts a number of elements at given index
 * remove_from_to(struct arraylist_##name *self, size_t from, size_t to) - Removes a number of elements at given index
 * 
 */
#define ARRAYLIST_DECLARE(T, name) \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)); \
enum arraylist_error arraylist_##name##_reserve(struct arraylist_##name *self, size_t capacity); \
enum arraylist_error arraylist_##name##_shrink_size(struct arraylist_##name *self, size_t size); \
enum arraylist_error arraylist_##name##_shrink_to_fit(struct arraylist_##name *self); \
enum arraylist_error arraylist_##name##_push_back(struct arraylist_##name *self, T value); \
T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self); \
enum arraylist_error arraylist_##name##_insert_at(struct arraylist_##name *self, T value, size_t index); \
enum arraylist_error arraylist_##name##_pop_back(struct arraylist_##name *self); \
enum arraylist_error arraylist_##name##_remove_at(struct arraylist_##name *self, size_t index); \
enum arraylist_error arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to); \
T* arraylist_##name##_at(struct arraylist_##name *self, size_t index); \
T* arraylist_##name##_begin(struct arraylist_##name *self); \
T* arraylist_##name##_back(struct arraylist_##name *self); \
T* arraylist_##name##_end(struct arraylist_##name *self); \
size_t arraylist_##name##_size(const struct arraylist_##name *self); \
bool arraylist_##name##_is_empty(const struct arraylist_##name *self); \
size_t arraylist_##name##_capacity(const struct arraylist_##name *self); \
enum arraylist_error arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other); \
enum arraylist_error arraylist_##name##_clear(struct arraylist_##name *self); \
enum arraylist_error arraylist_##name##_deinit(struct arraylist_##name *self);

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
 * @return A zero initialized arraylist structure \
 * \
 * It does not allocate \
 * \
 */ \
struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)) { \
    struct arraylist_##name arraylist = {0}; \
    if (alloc) arraylist.alloc = alloc; else arraylist.alloc = allocator_get_default(); \
    arraylist.destructor = destructor; \
    arraylist.size = 0; \
    arraylist.capacity = 0; \
    arraylist.data = nullptr; \
    return arraylist; \
} \
\
/**
 * @brief Reserves the capacity of an arraylist \
 * @param arraylist Pointer to the arraylist \
 * @param capacity New capacity of the arraylist \
 * @return 1 if arraylist is null or arraylist's capacity is lesser or equals than new capacity \
 *         0 on success, -1 on reallocation failure, or -2 if given capacity overflows the buffer \
 * \
 */ \
int arraylist_##name##_reserve(struct arraylist_##name *self, size_t capacity) { \
    if (!self || capacity <= self->capacity) return 1; \
    if (capacity > SIZE_MAX / sizeof(T)) { \
        return -2; \
    } \
    T *new_data = nullptr; \
    if (self->capacity == 0) { \
        new_data = self->alloc->malloc(capacity * sizeof(T), self->alloc->ctx); \
    } else { \
        new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), capacity * sizeof(T), self->alloc->ctx); \
    } \
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
 * @return -1 self being null or on allocation failure, -2 on buffer overflow, 0 on success \
 * \
 * Safe to call on nullptr or already deinitialed arraylists \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @note Will not construct objects in place, objects must be constructed \
 * \
 */ \
int arraylist_##name##_push_back(struct arraylist_##name *self, T value) { \
    if (!self) return -1; \
    if (self->capacity == 0) { \
        T *new_data = self->alloc->malloc(INITIAL_CAP * sizeof(T), self->alloc->ctx); \
        if (!new_data) return -1; \
        self->data = new_data; \
        self->capacity = INITIAL_CAP; \
    }\
    if (self->size >= self->capacity) { \
        size_t new_capacity = self->capacity * 2; \
        if (new_capacity > SIZE_MAX / sizeof(T)) { \
            return -2; \
        } \
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
 * @return The slot or nullptr if the arraylist isn't initialized, if the (re)allocation fails \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @warning Return should be checked for null before usage \
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
    if (self->capacity == 0) { \
        T *new_data = self->alloc->malloc(INITIAL_CAP * sizeof(T), self->alloc->ctx); \
        if (!new_data) return nullptr; \
        self->data = new_data; \
        self->capacity = INITIAL_CAP; \
        return &self->data[self->size++]; \
    }\
    if (self->size >= self->capacity) { \
        size_t new_capacity = self->capacity * 2; \
        if (new_capacity > SIZE_MAX / sizeof(T)) { \
            return nullptr; \
        } \
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
 * @return -1 on self being null, or on (re)allocation failure, -2 on data buffer overflow, 0 on success \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @warning if index < 0 size_t overflows and inserts at end \
 * \
 */ \
int arraylist_##name##_insert_at(struct arraylist_##name *self, T value, size_t index) {\
    if (!self) return -1; \
    if (index >= self->size - 1) { \
        if (arraylist_##name##_push_back(self, value) == 0) return 0; \
        else return -1; \
    } \
    if (self->capacity == 0) { \
        T *new_data = self->alloc->malloc(INITIAL_CAP * sizeof(T), self->alloc->ctx); \
        if (!new_data) return -1; \
        self->data = new_data; \
        self->capacity = INITIAL_CAP; \
        self->data[self->size++] = value; \
        return 0; \
    } \
    ++self->size; \
    if (self->size >= self->capacity) { \
        size_t new_capacity = self->capacity * 2; \
        if (new_capacity > SIZE_MAX / sizeof(T)) { \
            return -2; \
        } \
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
 * @brief Removes the element at position index \
 * @param arraylist Pointer to the arraylist \
 * @param index Position to remove \
 * \
 * Will call destructor if available \
 * \
 * @warning if index < 0 size_t overflows and removes at end \
 * \
 */ \
void arraylist_##name##_remove_at(struct arraylist_##name *self, size_t index) {\
    if (!self) return; \
    if (index >= self->size - 1) { \
        arraylist_##name##_pop_back(self); \
        return; \
    } \
    if (self->destructor) { \
        self->destructor(&self->data[index]); \
    } \
    for (size_t i = index; i <= self->size - 1; ++i) { \
        self->data[i] = self->data[i + 1]; \
    } \
    --self->size; \
    return; \
} \
\
/**
 * @brief Removes elements from index until to inclusive \
 * @param arraylist Pointer to the arraylist \
 * @param from Starting position to remove \
 * @param to Ending position inclusive \
 * \
 * Will call destructor if available \
 * If from > to does nothing \
 * \
 * @warning if from < 0 size_t overflows and removes at end \
 *          if to < 0, it will remove until size - 1 \
 * \
 */ \
void arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to) { \
    if (!self) return; \
    if (to > self->size - 1) to = self->size - 1; \
    if (from > self->size - 1) from = self->size - 1; \
    size_t num_elems_to_rem = to - from; \
    if (num_elems_to_rem == 0) { \
        arraylist_##name##_remove_at(self, from); \
        return; \
    } \
    size_t i = from; \
    if (self->destructor) { \
        for (; i <= to; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    size_t num_elems_left = (self->size - 1) - num_elems_to_rem; \
    /* \
    printf("VALUE OF from = %lu\n", from); \
    printf("VALUE OF to = %lu\n", to); \
    printf("VALUE OF num_elems_to_rem = %lu\n", num_elems_to_rem); \
    printf("VALUE OF i = %lu\n", i); \
    printf("VALUE OF num_elems_left = %lu\n", num_elems_left); \
    */ \
    for (; from <= num_elems_left; ++from) { \
        self->data[from] = self->data[to + 1]; \
        ++to; \
    } \
    self->size -= num_elems_to_rem + 1; \
    return; \
} \
\
/**
 * @brief Accesses the position of the arraylist at index \
 * @param arraylist Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is greater than arraylist size \
 * \
 * @warning Return should be checked for null before usage \
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
 * @warning Return should be checked for null before usage \
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
 * @warning Return should be checked for null before usage \
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
 * @warning Return should be checked for null before usage, dereferencing it leads to UB \
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
    } \
    self->size = 0; \
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
