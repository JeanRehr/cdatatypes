/**
 * @file arraylist.h
 * @brief Generic and typesafe arraylist implementation using macros
 *
 * It is a zero-cost abstraction, if no destructor is provided to the arraylist and no manual
 * assignment of it later in the usage code, compiler eliminates all tests for destructor because 
 * it can be proved that no dtor is needed, this works due to all functions being static inlined
 * (tested on clang -O3)
 *
 * @details
 * This arraylist implementation provides similar functionality to cpp std vec, with explicit
 * control over memory management with custom allocator and destructors
 *
 * The arraylist will own and manage the memory of its internal storage. Once elements are added:
 * - Do not manually free elements, use the arraylist functions to remove.
 * - Destructor callback (if provided) will be called automatically when elements are removed.
 * - After deinit(), all contained elements are destroyed and the storage freed. 
 *
 * For value types (int, struct foo):
 * - Elements are copied into the arraylist.
 * - Destructors are called on removal/destruction if provided.
 * - If the struct/element contains heap-allocated memory, supply a destructor to avoid memory leaks
 *   or manually free them if not provided
 * - Prefer using emplace_back_slot() to construct objects in-place, avoids shallow copies
 *
 * For pointer types (struct foo*):
 * - The arraylist store the pointers, ownership of the pointed-to memory is only managed if a
 *   destructor is provided, the destructor must free the pointer and set it to null
 * - Without the destructor, the user must manage the pointed-to objects manually
 * - The typical destructor usually is: Foo_dtor_ptr(struct Foo **foo), so it can free internal
 *   resources, the obj itself, and set the pointer (in the list) to null
 *
 * Copy Semantics:
 * Unlike cpp std vec, it has no move semantics, and performs shallow copies bit by bit always
 * when adding an element or on reallocating memory, deep copy and move semantics are not automatic 
 * For structs containing heap-allocated memory, this means that the internal pointers may be
 * duplicated or invalidated after an internal realloc.
 *
 * It's recommended to always use emplace_back_slot() when possible to prevent shallow copies and
 * construct the object in-place, or do a deep-copy (moving the src to dst and invalidating src)
 * if your type needs it.
 *
 * Comparison to std vec:
 * - For value types, usage is equivalent to std vec<T>, with manual dtor/copy semantics in C
 * - For pointer types, it is like std vec<T*> by default, if a destructor is provided, it
 *   behaves like std vec<unique ptr<T>> (the container will own the pointed-to memory)
 * - Copy and move semantics must still be manually handled explicitly if needed.
 *
 * Example destructor for value types to be supplied to the arraylist:
 * @code
 * void Foo_dtor(struct Foo *t) {
 *     if (!t) return;
 *     // Free internal allocated members if needed
 *     free(t->member);
 *     // Do not free t itself, arraylist owns it
 * }
 * @endcode
 *
 * Example destructor for pointer types to be supplied to the arraylist:
 * @code
 * void Foo_dtor_ptr(struct Foo **t_ptr) {
 *     if (!t_ptr || !*t_ptr) return;
 *     // Free internal allocated members if needed
 *     free((*t_ptr)->member);
 *     // Or call the Foo_dtor:
 *     // Foo_dtor(*t_ptr);
 *     // Free the object itself
 *     free(*t_ptr);
 *     // Set it to null, which only affects the copy on the arraylist
 *     *t_ptr = NULL;
 * }
 * @endcode
 *
 * @warning Always assume that pointers are invalid after adding them to pointer containers and
 *          don't hold pointers to elements across operations that may reallocate.
 * @warning Call deinit() when done.
 *
 * @todo 
 * insert_from_to() or assign() - Inserts a number of elements at given index
 */
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

/**
 * @def ARRAYLIST_UNUSED
 * @brief Defines a macro to supress the warning for unused function because of static inline
 */
#if defined(__GNUC__) || defined(__clang__)
#   define ARRAYLIST_UNUSED __attribute__((unused))
#else
#   define ARRAYLIST_UNUSED
#endif

#include <stdbool.h>
#include <stdint.h>

#include "allocator.h"

#define initial_cap 1

/**
 * @enum arraylist_error
 * @brief Error codes for the arraylist
 */
enum arraylist_error {
    ARRAYLIST_OK = 0,             ///< No error
    ARRAYLIST_ERR_NULL = -1,      ///< Null pointer
    ARRAYLIST_ERR_OVERFLOW = -2,  ///< Buffer will overflow
    ARRAYLIST_ERR_ALLOC = -3,     ///< Allocation failure
};

#ifndef ARRAYLIST_USE_ASSERT
/**
 * @def ARRAYLIST_USE_ASSERT
 * @brief Defines if the arraylist will use asserts or return error codes
 * @details If ARRAYLIST_USE_ASSERT is 1, then the lib will assert and fail early, otherwise,
 *          defensive programming and returning error codes will be used
 */
#define ARRAYLIST_USE_ASSERT 0
#endif // ARRAYLIST_USE_ASSERT
#include <assert.h>

#if ARRAYLIST_USE_ASSERT
#   define ARRAYLIST_ENSURE(cond, errcode)   assert(cond);
#   define ARRAYLIST_ENSURE_PTR(cond)        assert(cond);
#else
#   define ARRAYLIST_ENSURE(cond, errcode)    if (!(cond)) return (errcode);
#   define ARRAYLIST_ENSURE_PTR(cond)         if (!(cond)) return NULL;
#endif // ARRAYLIST_USE_ASSERT if directive

/**
 * @def ARRAYLIST_DEF(T, name)
 * @brief Defines an arraylist structure for a specific type T
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * This macro defines a struct named "arraylist_##name" with the following fields:
 * - "data": Pointer of type T to the array of elements
 * - "size": Current number of elements in the arraylist
 * - "capacity": Current capacity of the arraylist
 * - "alloc": Pointer to custom alloc, if not provided, def alloc from allocator.h will be used
 * - "destructor": Function pointer to a destructor that knows how to free type T
 *
 * @code
 * // Example: Define an arraylist for integers
 * ARRAYLIST_DEF(int, ints)
 * // Creates a struct named struct arraylist_ints
 * @endcode
 */
#define ARRAYLIST_DEF(T, name) \
struct arraylist_##name { \
    T *data; \
    size_t size; \
    size_t capacity; \
    Allocator alloc; \
    void (*destructor)(T *); \
};

/**
 * @def ARRAYLIST_DECL(T, name)
 * @brief Declares all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * @details
 * The following functions are declared:
 * - ARRAYLIST_UNUSED static inline struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *));
 * - ARRAYLIST_UNUSED static inline struct arraylist_##name arraylist_##name##_init_with_capacity(Allocator *alloc, void (*destructor)(T *), size_t capacity);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_reserve(struct arraylist_##name *self, const size_t capacity);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_shrink_size(struct arraylist_##name *self, const size_t size);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_shrink_to_fit(struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_push_back(struct arraylist_##name *self, T value);
 * - ARRAYLIST_UNUSED static inline T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_insert_at(struct arraylist_##name *self, T value, const size_t index);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_pop_back(struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_remove_at(struct arraylist_##name *self, const size_t index);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to);
 * - ARRAYLIST_UNUSED static inline T* arraylist_##name##_at(const struct arraylist_##name *self, const size_t index);
 * - ARRAYLIST_UNUSED static inline T* arraylist_##name##_begin(const struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline T* arraylist_##name##_back(const struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline T* arraylist_##name##_end(const struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline T* arraylist_##name##_find(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx);
 * - ARRAYLIST_UNUSED static inline bool arraylist_##name##_contains(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx, size_t *out_index);
 * - ARRAYLIST_UNUSED static inline size_t arraylist_##name##_size(const struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline bool arraylist_##name##_is_empty(const struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline size_t arraylist_##name##_capacity(const struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline Allocator* arraylist_##name##_get_allocator(struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_clear(struct arraylist_##name *self);
 * - ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_deinit(struct arraylist_##name *self);
 */
#define ARRAYLIST_DECL(T, name) \
/**
 * @brief Creates a new arraylist \
 * @param alloc Custom allocator instance, if null, default alloc will be used \
 * @param destructor Custom destructor function pointer, if null, the arraylist will not know \
 *        how to free the given value if needed, user will be responsible for freeing \
 * @return A zero initialized arraylist structure \
 * \
 * @note It does not allocate \
 * \
 * @warning Call arraylist_##name##deinit() when done. \
 */ \
ARRAYLIST_UNUSED static inline struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)); \
/**
 * @brief Creates a new arraylist with a given capacity \
 * @param alloc Custom allocator instance, if null, default alloc will be used \
 * @param destructor Custom destructor function pointer, if null, the arraylist will not know \
 *        how to free the given value if needed, user will be responsible for freeing \
 * @param capacity Capacity to initialize with, if zero, it will zero initialized \
 * @return An arraylist struct with the given capacity \
 * \
 * @warning If allocation fails, or given capacity overflows the buffer, it will fail and set \
 *          everything to zero, if using asserts, it will abort, test the fields of the struct \
 *          or use the functions capacity() and the like if needed \
 * \
 * @warning Call arraylist_##name##deinit() when done. \
 */ \
ARRAYLIST_UNUSED static inline struct arraylist_##name arraylist_##name##_init_with_capacity(Allocator *alloc, void (*destructor)(T *), size_t capacity); \
/**
 * @brief Reserves the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @param capacity New capacity of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, ARRAYLIST_ERR_NULL on null being passed, \
 *         ARRAYLIST_ERR_ALLOC on allocation failure, or ARRAYLIST_ERR_OVERFLOW on buffer overflow \
 * \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_reserve(struct arraylist_##name *self, const size_t capacity); \
/**
 * @brief Shrinks the arraylist's size to size passed, removing elements if necessary \
 * @param self Pointer to the arraylist \
 * @param size New size of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_NULL on null being passed \
 * \
 * Returns early if arraylist is null or arraylist size is <= 0 or if arraylist size is >= size \
 *         or if provided size is < 0 \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_shrink_size(struct arraylist_##name *self, const size_t size); \
/**
 * @brief Shrinks the capacity of the arraylist to fit the size, may reallocate and does not remove elements \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_ALLOC on allocation failure, \
 *         or ARRAYLIST_ERR_NULL on null being passed \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_shrink_to_fit(struct arraylist_##name *self); \
/**
 * @brief Adds a new element by value to the end of the arraylist \
 * @param self Pointer to the arraylist to add a new value \
 * @param value Value of type T to be added \
 * @return ARRAYLIST_ERR_NULL if self is null, or ARRAYLIST_ERR_ALLOC on allocation failure, or \
 *         ARRAYLIST_ERR_OVERFLOW on buffer overflow, or ARRAYLIST_OK on success \
 * \
 * Safe to call on NULL or already deinitialed arraylists \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @note Will not construct objects in place, objects must be constructed \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_push_back(struct arraylist_##name *self, T value); \
/**
 * @brief Returns a slot at the end of the arraylist for an object to be constructed \
 * @param self Pointer to the arraylist \
 * @return The slot or NULL if the arraylist isn't initialized, or if the (re)allocation fails, \
 *         or if there is a buffer overflow possibility \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @code{.c} \
 * struct Foo { int a; }; \
 * // Initialize ... \
 * struct Foo *slot = arraylist_Foo_emplace_back_slot(&mylist); \
 * // Now slot is a valid pointer for writing into a new element. For example: \
 * slot->a = 42; // or call a constructor on slot \
 * @endcode \
 * \
 * For pointer types, this returns a slot for storing the pointer \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self); \
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
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_insert_at(struct arraylist_##name *self, T value, const size_t index); \
/**
 * @brief Removes the last added element \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Does nothing on an empty arraylist \
 * \
 * @note The object will be destructed if a destructor is provided durint arraylist init \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_pop_back(struct arraylist_##name *self); \
/**
 * @brief Removes the element at position index \
 * @param self Pointer to the arraylist \
 * @param index Position to remove \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Will call destructor if available \
 * \
 * @warning if index < 0 size_t overflows and removes at end \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_remove_at(struct arraylist_##name *self, const size_t index); \
/**
 * @brief Removes elements from index until to inclusive \
 * @param self Pointer to the arraylist \
 * @param from Starting position to remove \
 * @param to Ending position inclusive \
 * \
 * Will call destructor if available \
 * If from > to does nothing \
 * \
 * @warning if from < 0 size_t overflows and removes at end, if to < 0, it will also overflow and may remove everything \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to); \
/**
 * @brief Accesses the position of the arraylist at index \
 * @param self Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is OOB \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_at(const struct arraylist_##name *self, const size_t index); \
/**
 * @brief Accesses the first element of the arraylist \
 * @details It returns the block of memory allocated, can be used to iterate, \
 *          and where a function accepts a *T \
 * @param self Pointer to the arraylist \
 * @return A pointer to the first value or null if !self \
 * \
 * @code{.c} \
 * // Prints the contents of an arraylist of T \
 * for (T *it = arraylist_t_begin(); it != arraylist_t_end(); ++it) { t_print(); } \
 * @endcode \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_begin(const struct arraylist_##name *self); \
/**
 * @brief Accesses the last position of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin, \
 *          but it will not be the end of the arraylist, just the last element, can be dereferenced \
 * @param self Pointer to the arraylist \
 * @return A pointer to the last value or null if !self \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_back(const struct arraylist_##name *self); \
/**
 * @brief Accesses the end of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a T* same as begin \
 * @param self Pointer to the arraylist \
 * @return A pointer to the end or null if !self \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB, \
 *          even if NULL was not returned \
 */ \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_end(const struct arraylist_##name *self); \
/**
 * @brief Tries to finds the given value and returns it \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T* value with a void* value, \
 *                  returns a boolean \
 * @param ctx A context to be used in the function pointer, usually used for comparing with a \
 *            field of type T or a member of it \
 * @return A pointer to the value if found, a pointer to the end if not found, or null if !self \
 * \
 * @note Performs a simple linear search, if performance matters, roll your own \
 *          sort and/or find functions \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to \
            UB if value is not found \
 */ \
ARRAYLIST_UNUSED static inline T* arraylist_##name##_find(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx); \
/**
 * @brief Tries to finds the given value and returns it \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T value \
 * @param ctx A context to be used in the function pointer \
 * @param out_index The index if wanted \
 * @return True if found and out_index if provided will return the index where it was found, \
           false if not found and out_index is untouched, or self == NULL \
 * \
 * @note Performs a simple linear search, if performance matters, roll your own \
 *       sort and/or find functions \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB if \
            value is not found \
 */ \
ARRAYLIST_UNUSED static inline bool arraylist_##name##_contains(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx, size_t *out_index); \
/**
 * @brief Gets the size of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The size or 0 if arraylist is null \
 * \
 */ \
ARRAYLIST_UNUSED static inline size_t arraylist_##name##_size(const struct arraylist_##name *self); \
/**
 * @brief Checks if the arraylist is empty \
 * @param self Pointer to the arraylist \
 * @return False if arraylist is null or size = 0, otherwise true \
 */ \
ARRAYLIST_UNUSED static inline bool arraylist_##name##_is_empty(const struct arraylist_##name *self); \
/**
 * @brief Gets the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The capacity or 0 if arraylist is null \
 */ \
ARRAYLIST_UNUSED static inline size_t arraylist_##name##_capacity(const struct arraylist_##name *self); \
/**
 * @brief Gets the allocator of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The allocator \
 */ \
ARRAYLIST_UNUSED static inline Allocator* arraylist_##name##_get_allocator(struct arraylist_##name *self); \
/**
 * @brief Swaps the contents of arraylist self with other \
 * @param self Pointer to the arraylist \
 * @param other Pointer to another arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other); \
/**
 * @brief Sorts the self based on the given comp function \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL if self == null, otherwise ARRAYLIST_OK \
 * \
 * @note Performs a simple quicksort, non-stable, pivot is always the last element, \
 *       if performance matters, roll your own sort functions \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_qsort(struct arraylist_##name *self, bool (*comp)(T*, T*)); \
/**
 * @brief Clears the arraylist's data \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * It does not free the arraylist itself and does not alter the capacity, only sets its size to 0 \
 * Safe to call on NULL, returns early \
 * \
 * @note Will call the object's destructor on objects if available \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_clear(struct arraylist_##name *self); \
/**
 * @brief Destroys and frees an arraylist \
 * @param self Pointer to the arraylist to deinit \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on NULL or already deinitialized arraylists, returns early \
 * \
 * @note Will call the destructor on data items if provided \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_deinit(struct arraylist_##name *self);

/**
 * @def ARRAYLIST_IMPL(T, name)
 * @brief Implements all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * Implements the functions of the ARRAYLIST_DECL macro
 *
 * @note This macro should be used in a .c file, not in a header
 */ 
#define ARRAYLIST_IMPL(T, name) \
/* =========================== PRIVATE FUNCTIONS =========================== */ \
/**
 * @private \
 * @brief Function that deals with capacity and (re)alloc if necessary \
 * @param self Pointer to the arraylist to deinit \
 * @return ARRAYLIST_OK if successful, ARRAYLIST_ERR_OVERFLOW if buffer will overflow, \
 *         or ARRAYLIST_ERR_NULL if allocation failure \
 * \
 * This static function ensures that capacity of the arraylist is atleast min_cap \
 * If the self->capacity is 0 (first allocation) will call malloc, otherwise realloc \
 * Then set the self->capacity to min_cap \
 * \
 * @warning Assumes self is not null, as this is a static function, this is not really a problem \
 */ \
static inline enum arraylist_error arraylist_##name##_double_capacity(struct arraylist_##name *self) { \
    /* Assumes self is never null */ \
    size_t new_capacity = 0; \
    if (self->capacity != 0) { \
        new_capacity = self->capacity * 2; \
    } else { \
        new_capacity = initial_cap; \
    } \
    ARRAYLIST_ENSURE(new_capacity <= SIZE_MAX / sizeof(T), ARRAYLIST_ERR_OVERFLOW) \
    T *new_data = NULL; \
    if (self->data == NULL) { \
        new_data = self->alloc.malloc(new_capacity * sizeof(T), self->alloc.ctx); \
    } else { \
        new_data = self->alloc.realloc(self->data, self->capacity * sizeof(T), new_capacity * sizeof(T), self->alloc.ctx); \
    } \
    ARRAYLIST_ENSURE(new_data != NULL, ARRAYLIST_ERR_ALLOC) \
    self->data = new_data; \
    self->capacity = new_capacity; \
    return ARRAYLIST_OK; \
} \
\
/** \
 * @private \
 * @brief Simple swap function \
 */ \
static inline void arraylist_##name##_swap_elems(T *a, T *b) { \
    T tmp = *a; \
    *a = *b; \
    *b = tmp; \
} \
\
/** \
 * @private \
 * @brief Helper function to use in the sort algo \
 */ \
static inline size_t arraylist_##name##_partition_buffer(struct arraylist_##name *self, size_t low, size_t high, bool (*comp)(T*, T*)) { \
    T *pivot = &self->data[high]; \
    size_t i = low; \
    \
    /* Traverse buffer */ \
    for (size_t j = low; j < high; ++j) { \
        if (comp(&self->data[j], pivot)) { \
            /* Move elements to the left side */ \
            arraylist_##name##_swap_elems(&self->data[i], &self->data[j]); \
            ++i; \
        } \
    } \
    \
    /* Move pivot after smaller elements and return its position */ \
    arraylist_##name##_swap_elems(&self->data[i], &self->data[high]); \
    return i; \
} \
\
/** \
 * @private \
 * @brief Helper recursive function for the sort algo \
 */ \
static inline void arraylist_##name##_helper_qsort(struct arraylist_##name *self, size_t low, size_t high, bool (*comp)(T*, T*)) { \
    if (low < high) { \
        /* part_idx is the parition return index of pivot */ \
        size_t part_idx = arraylist_##name##_partition_buffer(self, low, high, comp); \
        /* prevent a size_t underflow */ \
        if (part_idx > 0) { \
            /* recursion calls for smaller elements */ \
            arraylist_##name##_helper_qsort(self, low, part_idx - 1, comp); \
        } \
        /* greater elements */ \
        arraylist_##name##_helper_qsort(self, part_idx + 1, high, comp); \
    } \
} \
/* =========================== PUBLIC FUNCTIONS =========================== */ \
static inline struct arraylist_##name arraylist_##name##_init(Allocator *alloc, void (*destructor)(T *)) { \
    struct arraylist_##name arraylist = {0}; \
    if (alloc) { \
        arraylist.alloc = *alloc; \
    } else { \
        arraylist.alloc = allocator_get_default(); \
    } \
    arraylist.destructor = destructor; \
    arraylist.size = 0; \
    arraylist.capacity = 0; \
    arraylist.data = NULL; \
    return arraylist; \
} \
\
static inline struct arraylist_##name arraylist_##name##_init_with_capacity(Allocator *alloc, void (*destructor)(T *), size_t capacity) { \
    struct arraylist_##name arraylist = {0}; \
    arraylist = arraylist_##name##_init(alloc, destructor); \
    if (capacity > 0) { \
        if (arraylist_##name##_reserve(&arraylist, capacity) != ARRAYLIST_OK) { \
            arraylist.size = 0; \
            arraylist.capacity = 0; \
            arraylist.data = NULL; \
        } \
    } \
    return arraylist; \
} \
\
static inline enum arraylist_error arraylist_##name##_reserve(struct arraylist_##name *self, size_t capacity) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->capacity >= capacity) { \
        return ARRAYLIST_OK; \
    } \
    ARRAYLIST_ENSURE(capacity <= SIZE_MAX / sizeof(T), ARRAYLIST_ERR_OVERFLOW); \
    T *new_data = NULL; \
    if (self->capacity == 0) { \
        new_data = self->alloc.malloc(capacity * sizeof(T), self->alloc.ctx); \
    } else { \
        new_data = self->alloc.realloc(self->data, self->capacity * sizeof(T), capacity * sizeof(T), self->alloc.ctx); \
    } \
    ARRAYLIST_ENSURE(new_data != NULL, ARRAYLIST_ERR_ALLOC) \
    self->data = new_data; \
    self->capacity = capacity; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error arraylist_##name##_shrink_size(struct arraylist_##name *self, size_t size) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (size >= self->size || self->size <= 0) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        for (size_t i = size; i < self->size; i++) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->size = size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error arraylist_##name##_shrink_to_fit(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->capacity == self->size) { \
        return ARRAYLIST_OK; \
    } \
    if (self->size == 0) { \
        self->alloc.free(self->data, self->capacity * sizeof(T), self->alloc.ctx); \
        self->data = NULL; \
        self->capacity = 0; \
        return ARRAYLIST_OK; \
    }\
    T *new_data = self->alloc.realloc(self->data, self->capacity * sizeof(T), self->size * sizeof(T), self->alloc.ctx); \
    ARRAYLIST_ENSURE(new_data != NULL, ARRAYLIST_ERR_ALLOC) \
    self->data = new_data; \
    self->capacity = self->size; \
    return ARRAYLIST_OK; \
}\
\
static inline enum arraylist_error arraylist_##name##_push_back(struct arraylist_##name *self, T value) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = arraylist_##name##_double_capacity(self); \
        if (err != ARRAYLIST_OK) { \
            return err; \
        } \
    } \
    self->data[self->size++] = value; \
    return ARRAYLIST_OK; \
} \
\
static inline T* arraylist_##name##_emplace_back_slot(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = arraylist_##name##_double_capacity(self); \
        if (err != ARRAYLIST_OK) { \
            return NULL; \
        } \
    } \
    return &self->data[self->size++]; \
} \
\
static inline enum arraylist_error arraylist_##name##_insert_at(struct arraylist_##name *self, T value, const size_t index) {\
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
        return arraylist_##name##_push_back(self, value); \
    } \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = arraylist_##name##_double_capacity(self); \
        if (err != ARRAYLIST_OK) { \
            return err; \
        } \
    } \
    for (size_t i = self->size; i > index; --i) { \
        self->data[i] = self->data[i - 1]; \
    } \
    self->data[index] = value; \
    ++self->size; \
    return ARRAYLIST_OK; \
}\
\
static inline enum arraylist_error arraylist_##name##_pop_back(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size <= 0) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        self->destructor(&self->data[self->size - 1]); \
    } \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error arraylist_##name##_remove_at(struct arraylist_##name *self, const size_t index) {\
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
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
static inline enum arraylist_error arraylist_##name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size == 0) { \
        return ARRAYLIST_OK; \
    } \
    if (to >= self->size) { \
        to = self->size - 1; \
    } \
    if (from >= self->size) { \
        from = self->size - 1; \
    } \
    size_t num_to_remove = to - from + 1; \
    if (self->destructor) { \
        for (size_t i = from; i <= to; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    \
    /* how many numbers are left after the 'to' param */ \
    size_t num_after = self->size - to - 1; \
    for (size_t i = 0; i < num_after; ++i) { \
        self->data[from + i] = self->data[to + 1 + i]; \
    } \
    self->size -= num_to_remove; \
    return ARRAYLIST_OK; \
} \
\
static inline T* arraylist_##name##_at(const struct arraylist_##name *self, const size_t index) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    ARRAYLIST_ENSURE_PTR(index < self->size) \
    return &self->data[index]; \
} \
\
static inline T* arraylist_##name##_begin(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data; \
} \
\
static inline T* arraylist_##name##_back(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data + (self->size - 1); \
} \
\
static inline T* arraylist_##name##_end(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data + self->size; \
} \
\
static inline T* arraylist_##name##_find(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    ARRAYLIST_ENSURE_PTR(predicate != NULL) \
    for (size_t i = 0; i < self->size; ++i) { \
        if (predicate(&self->data[i], ctx)) { \
           return &self->data[i]; \
        } \
    } \
    return self->data + self->size; \
} \
\
static inline bool arraylist_##name##_contains(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx, size_t *out_index) { \
    ARRAYLIST_ENSURE(self != NULL && predicate != NULL, false) \
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
static inline size_t arraylist_##name##_size(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, 0) \
    return self ? self->size : 0; \
} \
\
static inline bool arraylist_##name##_is_empty(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, false) \
    return self->size == 0 ? true : false; \
} \
\
static inline size_t arraylist_##name##_capacity(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, 0) \
    return self->capacity; \
} \
\
static inline Allocator* arraylist_##name##_get_allocator(struct arraylist_##name *self) { \
    return &self->alloc; \
} \
\
static inline enum arraylist_error arraylist_##name##_swap(struct arraylist_##name *self, struct arraylist_##name *other) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    ARRAYLIST_ENSURE(other != NULL, ARRAYLIST_ERR_NULL) \
    struct arraylist_##name temp = *other; \
    *other = *self; \
    *self = temp; \
    return ARRAYLIST_OK; \
} \
ARRAYLIST_UNUSED static inline enum arraylist_error arraylist_##name##_qsort(struct arraylist_##name *self, bool (*comp)(T*, T*)) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size > 1) { \
        arraylist_##name##_helper_qsort(self, 0, self->size - 1, comp); \
    } \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error arraylist_##name##_clear(struct arraylist_##name *self) { \
    if (!self || self->size == 0) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->size = 0; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error arraylist_##name##_deinit(struct arraylist_##name *self) { \
    if (!self || !self->data) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i]); \
        } \
    } \
    self->alloc.free(self->data, self->capacity * sizeof(T), self->alloc.ctx); \
    self->data = NULL; \
    self->size = 0; \
    self->capacity = 0; \
    return ARRAYLIST_OK; \
}

/**
 * @def ARRAYLIST(T, name)
 * @brief Helper macro to define, declare and implement all in one \
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 */
#define ARRAYLIST(T, name)\
ARRAYLIST_DEF(T, name) \
ARRAYLIST_DECL(T, name) \
ARRAYLIST_IMPL(T, name)

/**
 * @def ARRAYLIST_STRIP_PREFIX
 * @brief Creates short named wrapper functions for the arraylist operations
 *
 * Generates functions with the pattern name##_function() instead of arraylist_##name##_function()
 * Must be used in a .c file, after the ARRAYLIST_IMPL macro
 * Inspiration taken from the Tsoding's nob.h lib
 *
 * Usage:
 * @code{.c}
 * // In a header, or at the top of a .c file:
 * ARRAYLIST_DEF(int, int_list)
 * ARRAYLIST_DECL(int, int_list)
 * // In a .c file:
 * ARRAYLIST_IMPL(int, int_list)
 * ARRAYLIST_STRIP_PREFIX(int, int_list)
 * // All of them must have the same T type and name, now both naming works:
 * struct arraylist_ints list = int_list_init(&list, ...); // Short
 * arraylist_int_list_pushback(...); // Full
 * int_list_pushback(...) // Short
 * @endcode
 *
 * @warning Ensure "name" doesn't collide with existing identifiers in a codebase.
 *
 * Example:
 * @code{.c}
 * // User code:
 * void strings_init(...) {...}
 *
 * // This creates a collision
 * ARRAYLIST_STRIP_PREFIX(char *, strings) // Tries to define strings_init(...)
 * @endcode
 *
 * @warning Using the same name multiple times will cause redefinition errors
 * @code{.c}
 * // Different names for same or different types works
 * ARRAYLIST(int, int_list)
 * ARRAYLIST_STRIP_PREFIX(int, int_list)
 * 
 * ARRAYLIST(int, int_array)
 * ARRAYLIST_STRIP_PREFIX(int, int_array)
 * 
 * // Same name for different types doesn't work
 * ARRAYLIST(int, numbers)
 * ARRAYLIST_STRIP_PREFIX(int, numbers)
 * 
 * ARRAYLIST(float, numbers) // Error, confliting types with the above
 * ARRAYLIST_STRIP_PREFIX(float, numbers) // Redefinition, tries to create items_init(...), etc.
 * @endcode
 * @note The macro inline wrappers, compiler will eliminate wrapper overhead with optimizations enabled
 *
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 */
#define ARRAYLIST_STRIP_PREFIX(T, name) \
    ARRAYLIST_UNUSED static inline struct arraylist_##name name##_init(Allocator *alloc, void (*destructor)(T *)) { \
        return arraylist_##name##_init(alloc, destructor); \
    } \
    ARRAYLIST_UNUSED static inline struct arraylist_##name name##_init_with_capacity(Allocator *alloc, void (*destructor)(T *), size_t capacity) { \
        return arraylist_##name##_init_with_capacity(alloc, destructor, capacity); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_reserve(struct arraylist_##name *self, const size_t capacity) { \
        return arraylist_##name##_reserve(self, capacity); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_shrink_size(struct arraylist_##name *self, const size_t size) { \
        return arraylist_##name##_shrink_size(self, size); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_shrink_to_fit(struct arraylist_##name *self) { \
        return arraylist_##name##_shrink_to_fit(self); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_push_back(struct arraylist_##name *self, T value) { \
        return arraylist_##name##_push_back(self, value); \
    } \
    ARRAYLIST_UNUSED static inline T* name##_emplace_back_slot(struct arraylist_##name *self) { \
        return arraylist_##name##_emplace_back_slot(self); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_insert_at(struct arraylist_##name *self, T value, const size_t index) { \
        return arraylist_##name##_insert_at(self, value, index); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_pop_back(struct arraylist_##name *self) { \
        return arraylist_##name##_pop_back(self);\
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_remove_at(struct arraylist_##name *self, const size_t index) { \
        return arraylist_##name##_remove_at(self, index); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_remove_from_to(struct arraylist_##name *self, size_t from, size_t to) { \
        return arraylist_##name##_remove_from_to(self, from, to); \
    } \
    ARRAYLIST_UNUSED static inline T* name##_at(const struct arraylist_##name *self, const size_t index) { \
        return arraylist_##name##_at(self, index); \
    } \
    ARRAYLIST_UNUSED static inline T* name##_begin(const struct arraylist_##name *self) { \
        return arraylist_##name##_begin(self); \
    } \
    ARRAYLIST_UNUSED static inline T* name##_back(const struct arraylist_##name *self) { \
        return arraylist_##name##_back(self); \
    } \
    ARRAYLIST_UNUSED static inline T* name##_end(const struct arraylist_##name *self) { \
        return arraylist_##name##_end(self); \
    } \
    ARRAYLIST_UNUSED static inline T* name##_find(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx) { \
        return arraylist_##name##_find(self, predicate, ctx); \
    } \
    ARRAYLIST_UNUSED static inline bool name##_contains(const struct arraylist_##name *self, bool (*predicate)(T*, void *), void *ctx, size_t *out_index) { \
        return arraylist_##name##_contains(self, predicate, ctx, out_index); \
    } \
    ARRAYLIST_UNUSED static inline size_t name##_size(const struct arraylist_##name *self) { \
        return arraylist_##name##_size(self); \
    } \
    ARRAYLIST_UNUSED static inline bool name##_is_empty(const struct arraylist_##name *self) { \
        return arraylist_##name##_is_empty(self); \
    } \
    ARRAYLIST_UNUSED static inline size_t name##_capacity(const struct arraylist_##name *self) { \
        return arraylist_##name##_capacity(self); \
    } \
    ARRAYLIST_UNUSED static inline Allocator* name##_get_allocator(struct arraylist_##name *self) { \
        return arraylist_##name##_get_allocator(self); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_swap(struct arraylist_##name *self, struct arraylist_##name *other) { \
        return arraylist_##name##_swap(self, other); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_clear(struct arraylist_##name *self) { \
        return arraylist_##name##_clear(self); \
    } \
    ARRAYLIST_UNUSED static inline enum arraylist_error name##_deinit(struct arraylist_##name *self) { \
        return arraylist_##name##_deinit(self); \
    }

#endif // ARRAYLIST_H
