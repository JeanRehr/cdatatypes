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

/**
 * @def ARRAYLIST_UNUSED
 * @brief Defines a macro to supress the warning for unused function because of static inline
 *
 */
#if defined(__GNUC__) || defined(__clang__)
#   define ARRAYLIST_UNUSED __attribute__((unused))
#else
#   define ARRAYLIST_UNUSED
#endif

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
 * @todo 
 * insert_from_to() - Inserts a number of elements at given index
 * remove_from_to(struct arraylist_##name *self, size_t from, size_t to) - Removes a number of elements at given index
 */

/**
 * @def ARRAYLIST_DECLARE(T, name)
 * @brief Declares all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 * 
 */
#define ARRAYLIST_DECLARE(T, name) \
ARRAYLIST_UNUSED static inline struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_reserve(struct arraylist_##name *self, const size_t capacity); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_shrink_size(struct arraylist_##name *self, const size_t size); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_shrink_to_fit(struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_push_back(struct arraylist_##name *self, T value); \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_insert_at(struct arraylist_##name *self, T value, const size_t index); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_pop_back(struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_remove_at(struct arraylist_##name *self, const size_t index); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to); \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_at(const struct arraylist_##name *self, const size_t index); \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_begin(const struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_back(const struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_end(const struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_find(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx); \
ARRAYLIST_UNUSED static inline bool arraylist_##name##_contains(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx, size_t *out_index); \
ARRAYLIST_UNUSED static inline size_t arraylist_##name##_size(const struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline bool arraylist_##name##_is_empty(const struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline size_t arraylist_##name##_capacity(const struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_clear(struct arraylist_##name *self); \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_deinit(struct arraylist_##name *self);

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
/* === PRIVATE FUNCTIONS === */ \
/**
 * @brief Static (private) function that deals with capacity and (re)alloc if necessary \
 * @param self Pointer to the arraylist to deinit \
 * @return ARRAYLIST_OK if successful, ARRAYLIST_ERR_OVERFLOW if buffer will overflow, \
 *         or ARRAYLIST_ERR_NULL if allocation failure \
 * \
 * This static function ensures that capacity of the arraylist is atleast min_cap \
 * If the self->capacity is 0 (first allocation) will call malloc, otherwise realloc \
 * Then set the self->capacity to min_cap \
 * \
 * @warning Assumes self is not null, as this is a static function, this is not really a problem \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_double_capacity(struct arraylist_##name *self) { \
    /* Assumes self is never null */ \
    size_t new_capacity = 0; \
    if (self->capacity != 0) { \
        new_capacity = self->capacity * 2; \
    } else { \
        new_capacity = INITIAL_CAP; \
    } \
    ARRAYLIST_ENSURE(new_capacity <= SIZE_MAX / sizeof(T), ARRAYLIST_ERR_OVERFLOW) \
    T *new_data = nullptr; \
    if (self->capacity == 0) { \
        new_data = self->alloc->malloc(new_capacity * sizeof(T), self->alloc->ctx); \
    } else { \
        new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), new_capacity * sizeof(T), self->alloc->ctx); \
    } \
    ARRAYLIST_ENSURE(new_data != nullptr, ARRAYLIST_ERR_ALLOC) \
    self->data = new_data; \
    self->capacity = new_capacity; \
    return ARRAYLIST_OK; \
} \
\
/* === PUBLIC FUNCTIONS === */ \
/**
 * @brief Creates a new arraylist \
 * @param alloc Custom allocator instance, if null, default alloc will be used \
 * @return A zero initialized arraylist structure \
 * \
 * It does not allocate \
 * \
 */ \
static inline struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)) { \
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
 * @param self Pointer to the arraylist \
 * @param capacity New capacity of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, ARRAYLIST_ERR_NULL on null being passed, \
 *         ARRAYLIST_ERR_ALLOC on allocation failure, or ARRAYLIST_ERR_OVERFLOW on buffer overflow \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_reserve(struct arraylist_##name *self, size_t capacity) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (self->capacity >= capacity) return ARRAYLIST_OK; \
    ARRAYLIST_ENSURE(capacity <= SIZE_MAX / sizeof(T), ARRAYLIST_ERR_OVERFLOW); \
    T *new_data = nullptr; \
    if (self->capacity == 0) { \
        new_data = self->alloc->malloc(capacity * sizeof(T), self->alloc->ctx); \
    } else { \
        new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), capacity * sizeof(T), self->alloc->ctx); \
    } \
    ARRAYLIST_ENSURE(new_data != nullptr, ARRAYLIST_ERR_ALLOC) \
    self->data = new_data; \
    self->capacity = capacity; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Shrinks the arraylist's capacity to size, removing elements if necessary \
 * @param self Pointer to the arraylist \
 * @param size New size of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_NULL on null being passed \
 * \
 * Returns early if arraylist is null or arraylist size is <= 0 or if arraylist size is <= size \
 *         or if provided size is <= 0 \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_shrink_size(struct arraylist_##name *self, size_t size) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (self->size <= size || self->size <= 0) return ARRAYLIST_OK; \
    if (self->destructor) { \
        for (size_t i = size; i < self->size; i++) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->size = size; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Shrinks the capacity of the arraylist to fit the size, may reallocate and does not remove elements \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_ALLOC on allocation failure, \
 *         or ARRAYLIST_ERR_NULL on null being passed \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_shrink_to_fit(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (self->capacity == self->size) return ARRAYLIST_OK; \
    if (self->size == 0) { \
        self->alloc->free(self->data, self->capacity * sizeof(T), self->alloc->ctx); \
        self->data = nullptr; \
        self->capacity = 0; \
        return ARRAYLIST_OK; \
    }\
    T *new_data = self->alloc->realloc(self->data, self->capacity * sizeof(T), self->size * sizeof(T), self->alloc->ctx); \
    ARRAYLIST_ENSURE(new_data != nullptr, ARRAYLIST_ERR_ALLOC) \
    self->data = new_data; \
    self->capacity = self->size; \
    return ARRAYLIST_OK; \
}\
\
/**
 * @brief Adds a new element to the end of the array \
 * @param self Pointer to the arraylist to add a new value \
 * @param value Value of type T to be added \
 * @return ARRAYLIST_ERR_NULL if self is null, or ARRAYLIST_ERR_ALLOC on allocation failure, or \
 *         ARRAYLIST_ERR_OVERFLOW on buffer overflow, or ARRAYLIST_OK on success \
 * \
 * Safe to call on nullptr or already deinitialed arraylists \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @note Will not construct objects in place, objects must be constructed \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_push_back(struct arraylist_##name *self, T value) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = arraylist_##name##_double_capacity(self); \
        if (err != ARRAYLIST_OK) return err; \
    } \
    self->data[self->size++] = value; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Returns a slot at the end of the arraylist for an object to be constructed \
 * @param self Pointer to the arraylist \
 * @return The slot or nullptr if the arraylist isn't initialized, or if the (re)allocation fails, \
 *         or if there is a buffer overflow possibility \
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
static inline T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != nullptr) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = arraylist_##name##_double_capacity(self); \
        if (err != ARRAYLIST_OK) return nullptr; \
    } \
    return &self->data[self->size++]; \
} \
\
/**
 * @brief Inserts an element in the given index \
 * @param self Pointer to the arraylist \
 * @param value Value of type T to be inserted \
 * @param index Index to insert \
 * @return ARRAYLIST_ERR_NULL if self is null, or ARRAYLIST_ERR_ALLOC on allocation failure, or \
 *         ARRAYLIST_ERR_OVERFLOW on buffer overflow, or ARRAYLIST_OK on success \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @warning if index < 0 size_t overflows and inserts at end \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_insert_at(struct arraylist_##name *self, T value, const size_t index) {\
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
        return arraylist_##name##_push_back(self, value); \
    } \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = arraylist_##name##_double_capacity(self); \
        if (err != ARRAYLIST_OK) return err; \
    } \
    ++self->size; \
    for (size_t i = self->size; i > index; --i) { \
        self->data[i] = self->data[i - 1]; \
    } \
    self->data[index] = value; \
    return ARRAYLIST_OK; \
}\
\
/**
 * @brief Removes the last added element \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of nullptr being passed, or ARRAYLIST_OK \
 * \
 * Does nothing on an empty arraylist \
 * \
 * @note The object will be destructed if a destructor is provided durint arraylist init \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_pop_back(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (self->size <= 0) return ARRAYLIST_OK; \
    if (self->destructor) self->destructor(&self->data[self->size - 1]); \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Removes the element at position index \
 * @param self Pointer to the arraylist \
 * @param index Position to remove \
 * @return ARRAYLIST_ERR_NULL in case of nullptr being passed, or ARRAYLIST_OK \
 * \
 * Will call destructor if available \
 * \
 * @warning if index < 0 size_t overflows and removes at end \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_remove_at(struct arraylist_##name *self, const size_t index) {\
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (index >= self->size - 1) { \
        return arraylist_##name##_pop_back(self); \
    } \
    if (self->destructor) { \
        self->destructor(&self->data[index]); \
    } \
    for (size_t i = index; i <= self->size - 1; ++i) { \
        self->data[i] = self->data[i + 1]; \
    } \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Removes elements from index until to inclusive \
 * @param self Pointer to the arraylist \
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
static inline enum arraylist_error arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (to > self->size - 1) { \
        to = self->size - 1; \
    } \
    if (from > self->size - 1) { \
        from = self->size - 1; \
    } \
    size_t num_to_remove = to - from + 1; \
    if (self->destructor) { \
        for (size_t i = from; i <= to; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    \
    /* how many numbers are left after the to param */ \
    size_t num_after = self->size - to - 1; \
    for (size_t i = 0; i < num_after; ++i) { \
        self->data[from + i] = self->data[to + 1 + i]; \
    } \
    self->size -= num_to_remove; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Accesses the position of the arraylist at index \
 * @param self Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is OOB \
 * \
 * @warning Return should be checked for null before usage \
 * \
 */ \
static inline T* arraylist_##name##_at(const struct arraylist_##name *self, const size_t index) { \
    ARRAYLIST_ENSURE_PTR(self != nullptr) \
    ARRAYLIST_ENSURE_PTR(index < self->size) \
    return &self->data[index]; \
} \
\
/**
 * @brief Accesses the first element of the arraylist \
 * @details It returns the block of memory allocated, can be used to iterate, \
 *          and where a function accepts a *T \
 * @param self Pointer to the arraylist \
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
static inline T* arraylist_##name##_begin(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != nullptr) \
    return self->data; \
} \
\
/**
 * @brief Accesses the last position of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin, \
 *          but it will not be the end of the arraylist, just the last element, can be dereferenced \
 * @param self Pointer to the arraylist \
 * @return A pointer to the last value or null if !self \
 * \
 * @warning Return should be checked for null before usage \
 * \
 */ \
static inline T* arraylist_##name##_back(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != nullptr) \
    return self->data + (self->size - 1); \
} \
\
/**
 * @brief Accesses the end of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin \
 * @param self Pointer to the arraylist \
 * @return A pointer to the end or null if !self \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB, \
 *          even if nullptr was not returned \
 * \
 */ \
static inline T* arraylist_##name##_end(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != nullptr) \
    return self->data + self->size; \
} \
\
/**
 * @brief Tries to finds the given value and returns it \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T value \
 * @param ctx A context to be used in the function pointer \
 * @return A pointer to the value if found, a pointer to the end if not found, or null if !self \
 * \
 * @details Performs a simple linear search, if performance matters, roll your own \
 *          sort and/or find functions \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB if \
            value is not found \
 * \
 */ \
static inline T* arraylist_##name##_find(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx) { \
    ARRAYLIST_ENSURE_PTR(self != nullptr) \
    ARRAYLIST_ENSURE_PTR(predicate != nullptr) \
    for (size_t i = 0; i < self->size; ++i) { \
        if (predicate(&self->data[i], ctx)) { \
           return &self->data[i]; \
        } \
    } \
    return self->data + self->size; \
} \
\
/**
 * @brief Tries to finds the given value and returns it \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T value \
 * @param ctx A context to be used in the function pointer \
 * @param out_index The index if wanted \
 * @return True if found, false if not found or self == nullptr \
 * \
 * @details Performs a simple linear search, if performance matters, roll your own \
 *          sort and/or find functions \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB if \
            value is not found \
 * \
 */ \
static inline bool arraylist_##name##_contains(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx, size_t *out_index) { \
    ARRAYLIST_ENSURE(self != nullptr && predicate != nullptr, false) \
    for (size_t i = 0; i < self->size; ++i) { \
        if (predicate(&self->data[i], ctx)) { \
            if (out_index) { \
                *out_index = i; \
            } \
           return true; \
        } \
    } \
    return false; \
} \
\
/**
 * @brief Gets the size of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The size or 0 if arraylist is null \
 * \
 */ \
static inline size_t arraylist_##name##_size(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, 0) \
    return self ? self->size : 0; \
} \
/**
 * @brief Checks if the arraylist is empty \
 * @param self Pointer to the arraylist \
 * @return False if arraylist is null or size = 0, otherwise true \
 * \
 */ \
static inline bool arraylist_##name##_is_empty(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, false) \
    return self->size == 0 ? true : false; \
} \
\
/**
 * @brief Gets the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The capacity or 0 if arraylist is null \
 * \
 */ \
static inline size_t arraylist_##name##_capacity(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, 0) \
    return self->capacity; \
} \
\
/**
 * @brief Swaps the contents of arraylist self with other \
 * @param self Pointer to the arraylist \
 * @param other Pointer to another arraylist \
 * @return ARRAYLIST_ERR_NULL in case of nullptr being passed, or ARRAYLIST_OK \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    ARRAYLIST_ENSURE(other != nullptr, ARRAYLIST_ERR_NULL) \
    struct arraylist_##name temp = *other; \
    *other = *self; \
    *self = temp; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Clears the arraylist's data \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of nullptr being passed, or ARRAYLIST_OK \
 * \
 * It does not free the arraylist itself and does not alter the capacity, only sets its size to 0 \
 * Safe to call on nullptr, returns early \
 * \
 * @note Will call the object's destructor on objects if available \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_clear(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->size = 0; \
    return ARRAYLIST_OK; \
} \
\
/**
 * @brief Destroys and frees an arraylist \
 * @param self Pointer to the arraylist to deinit \
 * @return ARRAYLIST_ERR_NULL in case of nullptr being passed, or ARRAYLIST_OK \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on nullptr or already deinitialized arraylists, returns early \
 * \
 * @note Will call the destructor on data items if provided \
 * \
 */ \
static inline enum arraylist_error arraylist_##name##_deinit(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != nullptr, ARRAYLIST_ERR_NULL) \
    if (!self->data) return ARRAYLIST_OK; \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->alloc->free(self->data, self->capacity * sizeof(T), self->alloc->ctx); \
    self->data = nullptr; \
    self->size = 0; \
    self->capacity = 0; \
    return ARRAYLIST_OK; \
}

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
