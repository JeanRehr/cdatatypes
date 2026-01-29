/**
 * @file arraylist.h
 * @brief Generic and typesafe arraylist implementation using macros
 *
 * It provides two versions ARRAYLIST and ARRAYLIST_DYN, DYN has a flexible function pointer for
 * destructor inside the struct.
 * As a cost for the flexibility, there is a runtime performance penalty of ~0.3% (compared to
 * c++ standard vector and the normal version ARRAYLIST without function pointer).
 *
 * One of the use-cases of the function pointer version is to change the destructor implementation
 * at runtime, where, for example, you have an arraylist of pointer to a base interface, and then
 * during runtime, the base pointer is casted to derived structs and the destructor is changed
 * dynamically.
 *
 * It is a zero-cost abstraction for both versions, if no destructor is provided to the arraylist
 * and no manual assignment of it (in the case of the function pointer version) later in the usage
 * code, compiler eliminates all tests for destructor because it can be proved that no dtor is
 * needed, this works due to all functions being static inlined (tested on clang with -O3 -flto)
 *
 * Prefer the ARRAYLIST version if the runtime flexibility of a destructor function pointer is not
 * needed.
 *
 * This arraylist requires a file named "allocator.h", which is a simple allocator interface that
 * defines a struct Allocator with 4 fields:
 * - void *(*malloc)(size_t size, void *ctx);
 * - void *(*realloc)(void *ptr, size_t old_size, size_t new_size, void *ctx)
 * - void (*free)(void *ptr, size_t size, void *ctx);
 * - void *ctx;
 *
 * And defines a function named allocator_get_default which returns a struct Allocator with
 * initialized fields to a default allocator of choice (std libc memory functions in this case).
 *
 * Very easy to provide your own arraylist.h and/or custom allocators by
 * implementing the Allocator interface
 *
 * @details
 * This arraylist implementation provides similar functionality to cpp standard vec, with explicit
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
 * - The typical destructor usually is: Foo_dtor_ptr(struct Foo **foo, Alloc *alloc), so it can
 *   free internal resources, the obj itself, and set the pointer (in the list) to null
 *
 * Copy/Move Semantics:
 * Unlike cpp standard lib vec, it has no builtin move semantics, and performs shallow copies bit by bit always
 * when adding an element or on reallocating memory, deep copy and move semantics are not automatic. 
 * For structs containing heap-allocated memory, this means that the internal pointers may be
 * duplicated or invalidated after an internal realloc.
 * A steal and deep_clone functions are provided for move and clone semantics. 
 *
 * It's recommended to always use emplace_back_slot() when possible to prevent shallow copies and
 * construct the object in-place, or do a deep-copy (moving the src to dst and invalidating src)
 * if your type needs it.
 *
 * Comparison to cpp standard lib vec:
 * - For value types, usage is equivalent to vector of type <T>, with manual dtor/copy semantics in C
 * - For pointer types, it is like vector of type <T*> by default, if a destructor is provided, it
 *   behaves like vector <unique pointer<T>> (the container will own the pointed-to memory)
 * - If the value type is non-pod (it allocates memory for field members), and a destructor is
 *   provided, it behaves just like vector <unique pointer<T>> as well
 * - Copy and move semantics must still be manually handled explicitly if needed.
 *
 * Example destructor for value types to be supplied to the arraylist:
 * @code
 * inline void Foo_dtor(struct Foo *t, Alloc *alloc) {
 *     if (!t) return;
 *     // Free internal allocated members if needed
 *     alloc->free(t->member, sizeof(t->member), alloc->ctx);
 *     // Do not free t itself, arraylist owns it
 *     // Optionally set members to null
 *     t->member = NULL;
 * }
 * @endcode
 *
 * Example destructor for pointer types to be supplied to the arraylist:
 * @code
 * inline void Foo_dtor_ptr(struct Foo **t_ptr, Alloc *alloc) {
 *     if (!t_ptr || !*t_ptr) return;
 *     // Free internal allocated members if needed
 *     alloc->free((*t_ptr)->member, sizeof((*t_ptr)->member), alloc->ctx);
 *     // Or call the Foo_dtor:
 *     // Foo_dtor(*t_ptr);
 *     // Free the object itself
 *     alloc->free(*t_ptr, sizeof(*self), alloc->ctx);
 *     // Set it to null, which only affects the copy on the arraylist
 *     *t_ptr = NULL;
 * }
 * @endcode
 *
 * The destructor function may be macro based for the non function pointer arraylist type, most
 * probably the compiler will optimize either function or macro equally, but the macro is more
 * guaranteed to be inlined.
 *
 * Exmaple for a destructor macro:
 * @code
 * #define Foo_ptr_dtor_macro(foo_dptr, alloc) \
 * do { \
 *     if (!foo_dptr || !*foo_dptr) { \
 *         break; \
 *     } \
 *     (alloc)->free(*(*foo_dptr)->member, sizeof(*(*foo_dptr)->member), (alloc)->ctx); \
 *     (alloc)->free(*foo_dptr, sizeof(*foo_dptr), (alloc)->ctx); \
 *     (foo_dptr) = NULL; \
 * } while (0)
 * @endcode
 *
 * C23 typeof with _Generic and static_assert/assert may make the code safer by checking if
 * foo_dptr is indeed of type Foo **.
 *
 * @warning Always assume that pointers are invalid after adding them to pointer containers and
 *          don't hold pointers to elements across operations that may reallocate.
 *
 * @warning Call deinit() when done, if reusing after deinit(), initialize it again with init().
 *
 * @warning After deinit() or steal() the arraylist will be left in an invalid and zeroed out state,
 *          to reuse it again, init() it first, if using without init() a segfault crash will most
 *          likely (hopefully) happen
 */
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdbool.h> // For bool, true, false
#include <stdint.h>  // For SIZE_MAX
#include <string.h>  // For memset()

#include "allocator.h" // For a custom Allocator interface

#define initial_cap 1

/**
 * @def ARRAYLIST_UNUSED
 * @brief Defines a macro to supress the warning for unused function because of static inline
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define ARRAYLIST_UNUSED [[maybe_unused]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define ARRAYLIST_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
    #define ARRAYLIST_UNUSED __attribute__((unused))
#else
    #define ARRAYLIST_UNUSED
#endif // ARRAYLIST_UNUSED definition

/**
 * @def ARRAYLIST_NODISCARD
 * @brief Defines a macro to warn of discarded unused results when they matter
 *        (possible leak of memory is involved)
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define ARRAYLIST_NODISCARD [[nodiscard]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define ARRAYLIST_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
    #define ARRAYLIST_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
    #if defined(_SAL_VERSION_SOURCE) && _SAL_VERSION_SOURCE >= 2
        #ifndef _Check_return_
            #include <sal.h>
        #endif // _Check_return_ from sal.h
        #define ARRAYLIST_NODISCARD _Check_return_
    #else
        #define ARRAYLIST_NODISCARD
    #endif // _SAL_VERSION_SOURCE && _SAL_VERSION_SOURCE >= 2
#else
    #define ARRAYLIST_NODISCARD
#endif // ARRAYLIST_NODISCARD definition

/**
 * @def ARRAYLIST_USE_ASSERT
 * @brief Defines if the arraylist will use asserts or return error codes
 * @details If ARRAYLIST_USE_ASSERT is 1, then the lib will assert and fail early, otherwise,
 *          defensive programming and returning error codes will be used
 */
#ifndef ARRAYLIST_USE_ASSERT
    #define ARRAYLIST_USE_ASSERT 0
#endif // ARRAYLIST_USE_ASSERT

#include <assert.h> // For assert()

#if ARRAYLIST_USE_ASSERT
    #define ARRAYLIST_ENSURE(cond, return_val) assert(cond);
    #define ARRAYLIST_ENSURE_PTR(cond) assert(cond);
#else
    #define ARRAYLIST_ENSURE(cond, return_val) \
        if (!(cond)) \
            return (return_val);
    #define ARRAYLIST_ENSURE_PTR(cond) \
        if (!(cond)) \
            return NULL;
#endif // ARRAYLIST_USE_ASSERT if directive

/**
 * @enum arraylist_error
 * @brief Error codes for the arraylist
 */
enum arraylist_error {
    ARRAYLIST_OK = 0,            ///< No error
    ARRAYLIST_ERR_NULL = -1,     ///< Null pointer
    ARRAYLIST_ERR_OVERFLOW = -2, ///< Buffer will overflow
    ARRAYLIST_ERR_ALLOC = -3,    ///< Allocation failure
};

/* ====== ARRAYLIST Macro destructor version START ====== */

/**
 * @def ARRAYLIST_USE_PREFIX
 * @brief Defines at compile-time if the functions will use the arraylist_* prefix
 * @details 
 * Generates functions with the pattern arraylist_##name##_function() instead of name##_function()
 * Must be defined before including the arraylist.h header, it will be per TU/file, so once defined
 * can't be undef in the same TU for different arraylist types and names.
 * The struct generated will always have the prefix arraylist_##name to them.
 * Inspiration taken from the Tsoding's nob.h lib, and some other libs like jemalloc which also does
 * something similar.
 *
 * The default is without prefixes, as the "name" parameter already acts like a user defined prefix:
 * @code
 * ARRAYLIST(int, int_list)
 * struct arraylist_int_list int_list = int_list_init(...);
 * @endcode
 *
 * Usage:
 * @code
 * // In a header that will include arraylist.h and will define some arraylist types:
 * #define ARRAYLIST_USE_PREFIX
 * #include "arraylist.h"
 * ARRAYLIST_TYPE(int, int_list) // Creates a struct: struct arraylist_int_list;
 * ARRAYLIST_DECL(int, int_list) // Declares functions named arraylist_int_list_init(...)
 * // In a .c file:
 * ARRAYLIST_IMPL(int, int_list)
 * @endcode
 */
#ifdef ARRAYLIST_USE_PREFIX
    #define ARRAYLIST_FN(name, func) arraylist_##name##_##func
#else
    #define ARRAYLIST_FN(name, func) name##_##func
#endif

// clang-format off

/**
 * @def ARRAYLIST_TYPE(T, name)
 * @brief Defines an arraylist structure for a specific type T
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * This macro defines a struct named "arraylist_##name" with the following fields:
 * - "data": Pointer of type T to the array of elements
 * - "size": Current number of elements in the arraylist
 * - "capacity": Current capacity of the arraylist
 * - "alloc": Pointer to custom alloc, if not provided, def alloc from allocator.h will be used
 *
 * @code
 * // Example: Define an arraylist for integers
 * ARRAYLIST_TYPE(int, ints)
 * // Creates a struct named struct arraylist_ints
 * @endcode
 */
#define ARRAYLIST_TYPE(T, name) \
struct arraylist_##name { \
    T *data; \
    size_t size; \
    size_t capacity; \
    struct Allocator alloc; \
};

/**
 * @def ARRAYLIST_DECL(T, name)
 * @brief Declares all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * @details
 * The following functions are declared:
 * - struct arraylist_##name ARRAYLIST_FN(name, init)(const struct Allocator alloc);
 * - enum arraylist_error ARRAYLIST_FN(name, reserve)(struct arraylist_##name *self, const size_t capacity);
 * - enum arraylist_error ARRAYLIST_FN(name, shrink_size)(struct arraylist_##name *self, const size_t size);
 * - enum arraylist_error ARRAYLIST_FN(name, shrink_to_fit)(struct arraylist_##name *self);
 * - enum arraylist_error ARRAYLIST_FN(name, push_back)(struct arraylist_##name *self, T value);
 * - T* ARRAYLIST_FN(name, emplace_back_slot)(struct arraylist_##name *self);
 * - enum arraylist_error ARRAYLIST_FN(name, insert_at)(struct arraylist_##name *self, T value, const size_t index);
 * - enum arraylist_error ARRAYLIST_FN(name, pop_back)(struct arraylist_##name *self);
 * - enum arraylist_error ARRAYLIST_FN(name, remove_at)(struct arraylist_##name *self, const size_t index);
 * - enum arraylist_error ARRAYLIST_FN(name, remove_from_to)(struct arraylist_##name *self, size_t from, size_t to);
 * - T* ARRAYLIST_FN(name, at(const struct arraylist_##name *self, const size_t index);
 * - T* ARRAYLIST_FN(name, begin)(const struct arraylist_##name *self);
 * - T* ARRAYLIST_FN(name, back)(const struct arraylist_##name *self);
 * - T* ARRAYLIST_FN(name, end)(const struct arraylist_##name *self);
 * - T* ARRAYLIST_FN(name, find)(const struct arraylist_##name *self, bool (*predicate)(T *elem, void *target), void *ctx);
 * - bool ARRAYLIST_FN(name, contains)(const struct arraylist_##name *self, bool (*predicate)(T *elem, void *target), void *ctx, size_t *out_index);
 * - size_t ARRAYLIST_FN(name, size)(const struct arraylist_##name *self);
 * - bool ARRAYLIST_FN(name, is_empty)(const struct arraylist_##name *self);
 * - size_t ARRAYLIST_FN(name, capacity)(const struct arraylist_##name *self);
 * - struct Allocator* ARRAYLIST_FN(name, get_allocator)(struct arraylist_##name *self);
 * - enum arraylist_error ARRAYLIST_FN(name, swap)(struct arraylist_##name *self, struct arraylist_##name *other);
 * - enum arraylist_error ARRAYLIST_FN(name, qsort)(struct arraylist_##name *self, bool (*comp)(T *n1, T *n2));
 * - struct arraylist_##name ARRAYLIST_FN(name, deep_clone)(const struct arraylist_##name *self, void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc));
 * - struct arraylist_##name ARRAYLIST_FN(name, shallow_copy)(const struct arraylist_##name *self);
 * - struct arraylist_##name ARRAYLIST_FN(name, steal)(struct arraylist_##name *self);
 * - enum arraylist_error ARRAYLIST_FN(name, clear)(struct arraylist_##name *self);
 * - void ARRAYLIST_FN(name, deinit)(struct arraylist_##name *self);
 */
#define ARRAYLIST_DECL(T, name) \
\
/** \
 * @brief init: Creates a new arraylist \
 * @param alloc Custom allocator instance, if null, default alloc will be used \
 * @return A zero initialized arraylist structure \
 * \
 * @note It does not allocate \
 * \
 * For pre-allocation (which is better for performance): \
 * @code \
 * struct arraylist_ints list = ints_init(alloc); \
 * if (ints_reserve(&list, expected_size) != ARRAYLIST_OK) { \
 *       // Handle or continue with default growth \
 * } \
 * @endcode \
 * \
 * @warning Call arraylist_##name##deinit() when done. \
 */ \
ARRAYLIST_UNUSED static inline struct arraylist_##name ARRAYLIST_FN(name, init)(const struct Allocator alloc); \
\
/** \
 * @brief reserve: Reserves the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @param capacity New capacity of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, ARRAYLIST_ERR_NULL on null being passed, \
 *         ARRAYLIST_ERR_ALLOC on allocation failure, or ARRAYLIST_ERR_OVERFLOW on buffer overflow \
 * \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, reserve)( \
    struct arraylist_##name *self, \
    const size_t capacity \
); \
\
/** \
 * @brief shrink_size: Shrinks the arraylist's size to size passed, removing elements if necessary \
 * @param self Pointer to the arraylist \
 * @param size New size of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_NULL on null being passed \
 * \
 * Returns early if arraylist is null or arraylist size is <= 0 or if arraylist size is >= size \
 *         or if provided size is < 0 \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, shrink_size)( \
    struct arraylist_##name *self, \
    const size_t size \
); \
\
/** \
 * @brief shrink_to_fit: Shrinks the capacity of the arraylist to fit the size \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_ALLOC on allocation failure, \
 *         or ARRAYLIST_ERR_NULL on null being passed \
 * \
 * @note May reallocate and does not remove elements \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, shrink_to_fit)(struct arraylist_##name *self); \
\
/** \
 * @brief push_back: Adds a new element by value to the end of the arraylist \
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
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, push_back)( \
    struct arraylist_##name *self, \
    T value \
); \
\
/** \
 * @brief emplace_back_slot: Returns a slot at the end of the arraylist for an object to be constructed \
 * @param self Pointer to the arraylist \
 * @return Pointer of type T to the slot for a type T to be constructed in place, NULL if self is \
 *         null or on any re/alloc failure or buffer overflow possibility \
 * \
 * Will automatically resize and realocate capacity, doubling it \
 * \
 * @code{.c} \
 * struct Foo { int a; }; \
 * // Initialize ... \
 * struct Foo *slot = Foo_emplace_back_slot(&mylist); \
 * // Now slot is a valid pointer for writing into a new element. For example: \
 * slot->a = 42; // or call a constructor on slot \
 * @endcode \
 * \
 * For pointer types (T*), returns T** pointing to uninitialized pointer. \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_FN(name, emplace_back_slot)(struct arraylist_##name *self); \
\
/** \
 * @brief insert_at: Inserts an element in the given index \
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
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, insert_at)( \
    struct arraylist_##name *self, \
    T value, \
    const size_t index \
); \
\
/** \
 * @brief pop_back: Removes the last added element \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Does nothing on an empty arraylist \
 * \
 * @note The object will be destructed if a destructor is provided in the ARRAYLIST_IMPL macro \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, pop_back)(struct arraylist_##name *self); \
\
/** \
 * @brief remove_at: Removes the element at position index \
 * @param self Pointer to the arraylist \
 * @param index Position to remove \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Will call destructor if available (if passed in the ARRAYLIST_IMPL macro) \
 * \
 * @warning if index < 0 size_t overflows and removes at end \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, remove_at)( \
    struct arraylist_##name *self, \
    const size_t index \
); \
\
/** \
 * @brief remove_from_to: Removes elements from index until to inclusive \
 * @param self Pointer to the arraylist \
 * @param from Starting position to remove \
 * @param to Ending position inclusive \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Will call destructor if available (if passed in the ARRAYLIST_IMPL macro) \
 * If from > to does nothing \
 * \
 * @warning if from < 0 size_t overflows and removes at end, if to < 0, \
 *          it will also overflow and may remove everything \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, remove_from_to)( \
    struct arraylist_##name *self, \
    size_t from, \
    size_t to \
); \
\
/** \
 * @brief at: Accesses the position of the arraylist at index \
 * @param self Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is OOB \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_FN(name, at)(const struct arraylist_##name *self, const size_t index); \
\
/** \
 * @brief begin: Accesses the first element of the arraylist \
 * @details It returns the block of memory allocated, can be used to iterate, \
 *          and where a function accepts a *T \
 * @param self Pointer to the arraylist \
 * @return A pointer to the first value or null if !self \
 * \
 * @code{.c} \
 * // Prints the contents of an arraylist of T \
 * for (T *it = t_begin(); it != t_end(); ++it) { t_print(); } \
 * @endcode \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_FN(name, begin)(const struct arraylist_##name *self); \
\
/** \
 * @brief back: Accesses the last position of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin, \
 *          but it will not be the end of the arraylist, just the last element, can be dereferenced \
 * @param self Pointer to the arraylist \
 * @return A pointer to the last value or null if !self or list is empty \
 * \
 * @warning Return should be checked for null before usage, calling back on an empty list and then \
 *          dereferencing it is UB \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_FN(name, back)(const struct arraylist_##name *self); \
\
/** \
 * @brief end: Accesses the end of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a T* same as begin \
 * @param self Pointer to the arraylist \
 * @return A pointer to the end or null if !self \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB, \
 *          even if NULL was not returned \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_FN(name, end)(const struct arraylist_##name *self); \
\
/** \
 * @brief find: Tries to finds the given value and returns it \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T *element with a void *target \
 *                  Must have the prototype: \
 *                  bool predicate(T *elem, void *target); \
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
ARRAYLIST_UNUSED static inline T* ARRAYLIST_FN(name, find)( \
    const struct arraylist_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx \
); \
\
/** \
 * @brief contains: Tries to finds the given value \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T *element with a void *target \
 *                  Must have the prototype: \
 *                  bool predicate(T *elem, void *target); \
 * @param ctx A context to be used in the function pointer, usually used for comparing with a \
 *            field of type T or a member of it \
 * @param out_index The index if wanted \
 * @return True if found and out_index if provided will return the index where it was found, \
           false if not found and out_index is untouched, or self == NULL \
 * \
 * @note Performs a simple linear search, if performance matters, roll your own \
 *       sort and/or find functions \
 */ \
ARRAYLIST_UNUSED static inline bool ARRAYLIST_FN(name, contains)( \
    const struct arraylist_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx, \
    size_t *out_index \
); \
\
/** \
 * @brief size: Gets the size of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The size or 0 if arraylist is null \
 * \
 */ \
ARRAYLIST_UNUSED static inline size_t ARRAYLIST_FN(name, size)(const struct arraylist_##name *self); \
\
/** \
 * @brief is_empty: Checks if the arraylist is empty \
 * @param self Pointer to the arraylist \
 * @return False if arraylist is null or size = 0, otherwise true \
 */ \
ARRAYLIST_UNUSED static inline bool ARRAYLIST_FN(name, is_empty)(const struct arraylist_##name *self); \
\
/** \
 * @brief capacity: Gets the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The capacity or 0 if arraylist is null \
 */ \
ARRAYLIST_UNUSED static inline size_t ARRAYLIST_FN(name, capacity)(const struct arraylist_##name *self); \
\
/** \
 * @brief get_allocator: Gets the allocator of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The allocator or NULL if self == null \
 */ \
ARRAYLIST_UNUSED static inline struct Allocator* ARRAYLIST_FN(name, get_allocator)(struct arraylist_##name *self); \
\
/** \
 * @brief swap: Swaps the contents of arraylist self with other \
 * @param self Pointer to the arraylist \
 * @param other Pointer to another arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, swap)( \
    struct arraylist_##name *self, \
    struct arraylist_##name *other \
); \
\
/** \
 * @brief qsort: Sorts the self based on the given comp function \
 * @param self Pointer to the arraylist \
 * @param comp Function that knows how to compare two T types \
 * @return ARRAYLIST_ERR_NULL if self == null or if fn comp == null, otherwise ARRAYLIST_OK \
 * \
 * @note Performs a simple quicksort, non-stable, pivot is always the last element, \
 *       if performance matters, roll your own sort functions \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, qsort)( \
    struct arraylist_##name *self, \
    bool (*comp)(T *n1, T *n2) \
); \
\
/** \
 * @brief deep_clone: Deeply clones an arraylist \
 * @param self Pointer to the arraylist to copy \
 * @param deep_clone_fn Function that knows how to clone a single element \
 *                      Must have the prototype: \
 *                      void deep_clone_fn(T *dst, const T *src, struct Allocator *alloc); \
 * @return A new arraylist struct that is independent of the self \
 * \
 * @note The correctness of this function depends on the provided deep_clone_fn parameter \
 *       for value types, int or pod structs, deep_clone_fn can simply assign *dst = *src; \
 *       if T contains pointers or heap allocations, then deep_clone_fn must allocate/copy \
 *       these fields as needed, if using arraylist of pointers to T, deep_clone_fn must allocate \
 *       a new T and, if it contains member to pointers, allocate/copy as needed \
 * \
 * @warning If self is NULL or deep_clone_fn if NULL then it returns a zero-initialized struct, \
 *          if asserts are enabled then it crashes \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked \
 */ \
ARRAYLIST_NODISCARD ARRAYLIST_UNUSED static inline struct arraylist_##name ARRAYLIST_FN(name, deep_clone)( \
    const struct arraylist_##name *self, \
    void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc) \
); \
\
/** \
 * @brief shallow_copy: Copies an arraylist bit by bit \
 * @param self Pointer to the arraylist to copy \
 * @return A new arraylist struct \
 * \
 * @note If T is or contains pointer or heap-allocated data, those pointers are copied as-is, \
 *       both the original and copy will reference the same memory. Modifying or freeing elements \
 *       in one list affects the other, this is unsafe if not careful, may cause double frees or incorrect assumptions. \
 *       Use deep_clone function with a correct function parameter for truly independent copies \
 * \
 * @warning If self is NULL then it returns a zero-initialized struct, if asserts are enabled then it crashes \
 */ \
ARRAYLIST_UNUSED static inline struct arraylist_##name ARRAYLIST_FN(name, shallow_copy)( \
    const struct arraylist_##name *self \
); \
\
/** \
 * @brief move: Moves the arraylist in the parameter to the lhs, leaving self as NULL \
 * @param self Pointer to the arraylist to move \
 * @return A new arraylist struct \
 * \
 * @note The self parameter will be left in a used, but NULL/uninitialized state and should not be used \
 *       to reuse it, one must call init again and reinitialize it \
 * \
 * @note The self parameter will be left in an unusable, NULL/uninitialized state and should not be used \
 *       to reuse it, one must call init again and reinitialize it \
 * \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked \
 */ \
ARRAYLIST_NODISCARD ARRAYLIST_UNUSED static inline struct arraylist_##name ARRAYLIST_FN(name, steal)( \
    struct arraylist_##name *self \
); \
\
/** \
 * @brief clear: Clears the arraylist's data \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * It does not free the arraylist itself and does not alter the capacity, only sets its size to 0 \
 * Safe to call on NULL, returns early \
 * \
 * @note Will call the object's destructor on objects if available \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_FN(name, clear)(struct arraylist_##name *self); \
\
/** \
 * @brief deinit: Destroys and frees an arraylist \
 * @param self Pointer to the arraylist to deinit \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on NULL or already deinitialized arraylists, returns early \
 * \
 * @note Will call the destructor on data items if provided \
 * @note The self parameter will be left in an unusable, NULL/uninitialized state and should not be used \
 *       to reuse it, one must call init again and reinitialize it \
 */ \
ARRAYLIST_UNUSED static inline void ARRAYLIST_FN(name, deinit)(struct arraylist_##name *self);

/** \
 * @def ARRAYLIST_IMPL(T, name, fn_dtor)
 * @brief Implements all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 * @param fn_dtor The function that knows how to free type T and its members (may be a macro or
 *                a normal function), recommended to inline the function
 *
 * Implements the functions of the ARRAYLIST_DECL macro
 *
 * @note This macro should be used in a .c file, not in a header
 *
 * @warning If the type T doesn't need to have a destructor, or one doesn't want to pass it and
 *          manually free, then a noop must be passed, like (void), or a macro/function
 *          that does nothing.
 */ 
#define ARRAYLIST_IMPL(T, name, fn_dtor) \
\
/* =========================== PRIVATE FUNCTIONS =========================== */ \
\
/** \
 * @private \
 * @brief double_capacity: Function that deals with capacity and (re)alloc if necessary \
 * @param self Pointer to the arraylist to deinit \
 * @return ARRAYLIST_OK if successful, ARRAYLIST_ERR_OVERFLOW if buffer will overflow, \
 *         or ARRAYLIST_ERR_NULL if allocation failure \
 * \
 * This private function ensures that capacity of the arraylist is atleast min_cap \
 * If the self->capacity is 0 (first allocation) will call malloc, otherwise realloc \
 * Then set the self->capacity to min_cap \
 * \
 * @warning Assumes self is not null, as this is a private function, this is not really a problem \
 */ \
static inline enum arraylist_error ARRAYLIST_FN(name, double_capacity)(struct arraylist_##name *self) { \
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
 * @brief swap_elems: Simple swap function \
 */ \
static inline void ARRAYLIST_FN(name, swap_elems)(T *a, T *b) { \
    T tmp = *a; \
    *a = *b; \
    *b = tmp; \
} \
\
/** \
 * @private \
 * @brief partition_buffer: Helper function to use in the sort algo \
 */ \
static inline size_t ARRAYLIST_FN(name, partition_buffer)( \
    struct arraylist_##name *self, \
    size_t low, \
    size_t high, \
    bool (*comp)(T *n1, T *n2) \
) { \
    T *pivot = &self->data[high]; \
    size_t i = low; \
    \
    /* Traverse buffer */ \
    for (size_t j = low; j < high; ++j) { \
        if (comp(&self->data[j], pivot)) { \
            /* Move elements to the left side */ \
            ARRAYLIST_FN(name, swap_elems)(&self->data[i], &self->data[j]); \
            ++i; \
        } \
    } \
    \
    /* Move pivot after smaller elements and return its position */ \
    ARRAYLIST_FN(name, swap_elems)(&self->data[i], &self->data[high]); \
    return i; \
} \
\
/** \
 * @private \
 * @brief helper_qsort: Helper recursive function for the sort algo \
 */ \
static inline void ARRAYLIST_FN(name, helper_qsort)( \
    struct arraylist_##name *self, \
    size_t low, \
    size_t high, \
    bool (*comp)(T *n1, T *n2) \
) { \
    if (low < high) { \
        /* part_idx is the parition return index of pivot */ \
        size_t part_idx = ARRAYLIST_FN(name, partition_buffer)(self, low, high, comp); \
        /* prevent a size_t underflow */ \
        if (part_idx > 0) { \
            /* recursion calls for smaller elements */ \
            ARRAYLIST_FN(name, helper_qsort)(self, low, part_idx - 1, comp); \
        } \
        /* greater elements */ \
        ARRAYLIST_FN(name, helper_qsort)(self, part_idx + 1, high, comp); \
    } \
} \
\
/* =========================== PUBLIC FUNCTIONS =========================== */ \
\
static inline struct arraylist_##name ARRAYLIST_FN(name, init)(const struct Allocator alloc) { \
    struct arraylist_##name arraylist = {0}; \
    arraylist.alloc = alloc; \
    arraylist.size = 0; \
    arraylist.capacity = 0; \
    arraylist.data = NULL; \
    return arraylist; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, reserve)(struct arraylist_##name *self, const size_t capacity) { \
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
static inline enum arraylist_error ARRAYLIST_FN(name, shrink_size)(struct arraylist_##name *self, const size_t size) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (size >= self->size || self->size <= 0) { \
        return ARRAYLIST_OK; \
    } \
    for (size_t i = size; i < self->size; i++) { \
        fn_dtor(&self->data[i], &self->alloc); \
    } \
    self->size = size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, shrink_to_fit)(struct arraylist_##name *self) { \
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
static inline enum arraylist_error ARRAYLIST_FN(name, push_back)(struct arraylist_##name *self, T value) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = ARRAYLIST_FN(name, double_capacity)(self); \
        if (err != ARRAYLIST_OK) { \
            return err; \
        } \
    } \
    self->data[self->size++] = value; \
    return ARRAYLIST_OK; \
} \
\
static inline T* ARRAYLIST_FN(name, emplace_back_slot)(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = ARRAYLIST_FN(name, double_capacity)(self); \
        if (err != ARRAYLIST_OK) { \
            return NULL; \
        } \
    } \
    return &self->data[self->size++]; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, insert_at)( \
    struct arraylist_##name *self, \
    T value, \
    const size_t index \
) {\
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
        return ARRAYLIST_FN(name, push_back)(self, value); \
    } \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = ARRAYLIST_FN(name, double_capacity)(self); \
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
static inline enum arraylist_error ARRAYLIST_FN(name, pop_back)(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size <= 0) { \
        return ARRAYLIST_OK; \
    } \
    fn_dtor(&self->data[self->size - 1], &self->alloc); \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, remove_at)(struct arraylist_##name *self, const size_t index) {\
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
        return ARRAYLIST_FN(name, pop_back)(self); \
    } \
    fn_dtor(&self->data[index], &self->alloc); \
    for (size_t i = index; i < self->size - 1; ++i) { \
        self->data[i] = self->data[i + 1]; \
    } \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, remove_from_to)( \
    struct arraylist_##name *self, \
    size_t from, \
    size_t to \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size == 0) { \
        return ARRAYLIST_OK; \
    } \
    if (from > to) { \
        return ARRAYLIST_OK; \
    } \
    if (to >= self->size) { \
        to = self->size - 1; \
    } \
    if (from >= self->size) { \
        from = self->size - 1; \
    } \
    size_t num_to_remove = to - from + 1; \
    for (size_t i = from; i <= to; ++i) { \
        fn_dtor(&self->data[i], &self->alloc); \
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
static inline T* ARRAYLIST_FN(name, at)(const struct arraylist_##name *self, const size_t index) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    ARRAYLIST_ENSURE_PTR(index < self->size) \
    return &self->data[index]; \
} \
\
static inline T* ARRAYLIST_FN(name, begin)(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data; \
} \
\
static inline T* ARRAYLIST_FN(name, back)(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    ARRAYLIST_ENSURE_PTR(self->size != 0) \
    return self->data + (self->size - 1); \
} \
\
static inline T* ARRAYLIST_FN(name, end)(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data + self->size; \
} \
\
static inline T* ARRAYLIST_FN(name, find)( \
    const struct arraylist_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx \
) { \
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
static inline bool ARRAYLIST_FN(name, contains)( \
    const struct arraylist_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx, \
    size_t *out_index \
) { \
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
static inline size_t ARRAYLIST_FN(name, size)(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, 0) \
    return self ? self->size : 0; \
} \
\
static inline bool ARRAYLIST_FN(name, is_empty)(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, false) \
    return self->size == 0 ? true : false; \
} \
\
static inline size_t ARRAYLIST_FN(name, capacity)(const struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, 0) \
    return self->capacity; \
} \
\
static inline struct Allocator* ARRAYLIST_FN(name, get_allocator)(struct arraylist_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return &self->alloc; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, swap)( \
    struct arraylist_##name *self, \
    struct arraylist_##name *other \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    ARRAYLIST_ENSURE(other != NULL, ARRAYLIST_ERR_NULL) \
    struct arraylist_##name temp = *other; \
    *other = *self; \
    *self = temp; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, qsort)( \
    struct arraylist_##name *self, \
    bool (*comp)(T *n1, T *n2) \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    ARRAYLIST_ENSURE(comp != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size > 1) { \
        ARRAYLIST_FN(name, helper_qsort)(self, 0, self->size - 1, comp); \
    } \
    return ARRAYLIST_OK; \
} \
\
static inline struct arraylist_##name ARRAYLIST_FN(name, deep_clone)( \
    const struct arraylist_##name *self, \
    void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc) \
) { \
    struct arraylist_##name clone = { 0 }; \
    ARRAYLIST_ENSURE(self != NULL, clone); \
    ARRAYLIST_ENSURE(deep_clone_fn != NULL, clone); \
    clone = ARRAYLIST_FN(name, init)(self->alloc); \
    ARRAYLIST_FN(name, reserve)(&clone, self->capacity); \
    clone.size = self->size; \
    for (size_t i = 0; i < self->size; ++i) { \
        deep_clone_fn(&clone.data[i], &self->data[i], &clone.alloc); \
    } \
    return clone; \
} \
\
static inline struct arraylist_##name ARRAYLIST_FN(name, shallow_copy)(const struct arraylist_##name *self) { \
    struct arraylist_##name clone = { 0 }; \
    ARRAYLIST_ENSURE(self != NULL, clone); \
    clone = ARRAYLIST_FN(name, init)(self->alloc); \
    ARRAYLIST_FN(name, reserve)(&clone, self->capacity); \
    clone.size = self->size; \
    for (size_t i = 0; i < self->size; ++i) { \
        clone.data[i] = self->data[i]; \
    } \
    return clone; \
} \
\
static inline struct arraylist_##name ARRAYLIST_FN(name, steal)(struct arraylist_##name *self) { \
    struct arraylist_##name steal = { 0 }; \
    ARRAYLIST_ENSURE(self != NULL, steal); \
    steal.data = self->data; \
    steal.size = self->size; \
    steal.capacity = self->capacity; \
    steal.alloc = self->alloc; \
    self->data = NULL; \
    self->size = 0; \
    self->capacity = 0; \
    memset(&self->alloc, 0, sizeof(self->alloc)); \
    return steal; \
} \
\
static inline enum arraylist_error ARRAYLIST_FN(name, clear)(struct arraylist_##name *self) { \
    if (!self || self->size == 0) { \
        return ARRAYLIST_OK; \
    } \
    for (size_t i = 0; i < self->size; ++i) { \
        fn_dtor(&self->data[i], &self->alloc); \
    } \
    self->size = 0; \
    return ARRAYLIST_OK; \
} \
\
static inline void ARRAYLIST_FN(name, deinit)(struct arraylist_##name *self) { \
    if (!self) { \
        return; \
    } \
    if (self->data) { \
        for (size_t i = 0; i < self->size; ++i) { \
            fn_dtor(&self->data[i], &self->alloc); \
        } \
        self->alloc.free(self->data, self->capacity * sizeof(T), self->alloc.ctx); \
        self->data = NULL; \
    } \
    self->size = 0; \
    self->capacity = 0; \
    memset(&self->alloc, 0, sizeof(self->alloc)); \
    return; \
}

/**
 * @def ARRAYLIST(T, name)
 * @brief Helper macro to define, declare and implement all in one
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 */
#define ARRAYLIST(T, name, fn_dtor) \
ARRAYLIST_TYPE(T, name) \
ARRAYLIST_DECL(T, name) \
ARRAYLIST_IMPL(T, name, fn_dtor)

// clang-format on

/* ====== ARRAYLIST_DYN Function Pointer destructor version START ====== */

/**
 * @def ARRAYLIST_DYN_USE_PREFIX
 * @brief Defines at compile-time if the functions will use the arraylist_dyn_* prefix
 * @details 
 * Generates functions with the pattern arraylist_dyn_##name##_function() instead of dyn_name##_function()
 * Must be defined before including the arraylist.h header, it will be per TU/file, so once defined
 * can't be undef in the same TU for different arraylist types and names.
 * The struct generated will always have the prefix arraylist_dyn_##name to them.
 * Inspiration taken from the Tsoding's nob.h lib.
 *
 * The default is without prefixes, as the "name" parameter already acts like a user defined prefix:
 * @code
 * ARRAYLIST(int, int_list)
 * struct arraylist_dyn_int_list int_list = dyn_int_list_init(...);
 * @endcode
 *
 * Usage:
 * @code
 * // In a header that will include arraylist.h and will define some arraylist types:
 * #define ARRAYLIST_USE_PREFIX
 * #include "arraylist.h"
 * ARRAYLIST_TYPE(int, int_list) // Creates a struct: struct arraylist_dyn_int_list;
 * ARRAYLIST_DECL(int, int_list) // Declares functions named arraylist_dyn_int_list_init(...)
 * // In a .c file:
 * ARRAYLIST_IMPL(int, int_list)
 * @endcode
 */
#ifdef ARRAYLIST_DYN_USE_PREFIX
    #define ARRAYLIST_DYN_FN(name, func) arraylist_dyn_##name##_##func
#else
    #define ARRAYLIST_DYN_FN(name, func) dyn_##name##_##func
#endif

// clang-format off

/**
 * @def ARRAYLIST_TYPE_DYN(T, name)
 * @brief Defines an arraylist structure for a specific type T
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * @details
 * This macro defines a struct named "arraylist_dyn_##name", fp stands for function pointer.
 * This struct is more flexible, at the cost of runtime performance, one can change the destructor
 * during runtime if needed.
 * It has the following fields:
 * - "data": Pointer of type T to the array of elements
 * - "size": Current number of elements in the arraylist
 * - "capacity": Current capacity of the arraylist
 * - "alloc": Pointer to custom alloc, if not provided, def alloc from allocator.h will be used
 * - "destructor": Function pointer to a destructor that knows how to free type T
 *
 * @note Allocator passed to destructor function must be the same as the Allocator in the init
 *       function
 *
 * @code
 * // Example: Define an arraylist for integers
 * ARRAYLIST_TYPE_DYN(int, ints)
 * // Creates a struct named struct arraylist_dyn_ints
 * @endcode
 */
#define ARRAYLIST_TYPE_DYN(T, name) \
struct arraylist_dyn_##name { \
    T *data; \
    size_t size; \
    size_t capacity; \
    struct Allocator alloc; \
    void (*destructor)(T *type, struct Allocator *alloc); \
};

/**
 * @def ARRAYLIST_DECL_DYN(T, name)
 * @brief Declares all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * @details
 * The following functions are declared:
 * - struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, init)(const struct Allocator alloc, void (*destructor)(T *type));
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, reserve)(struct arraylist_dyn_##name *self, const size_t capacity);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, shrink_size)(struct arraylist_dyn_##name *self, const size_t size);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, shrink_to_fit)(struct arraylist_dyn_##name *self);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, push_back)(struct arraylist_dyn_##name *self, T value);
 * - T* ARRAYLIST_DYN_FN(name, emplace_back_slot)(struct arraylist_dyn_##name *self);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, insert_at)(struct arraylist_dyn_##name *self, T value, const size_t index);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, pop_back)(struct arraylist_dyn_##name *self);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, remove_at)(struct arraylist_dyn_##name *self, const size_t index);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, remove_from_to)(struct arraylist_dyn_##name *self, size_t from, size_t to);
 * - T* ARRAYLIST_DYN_FN(name, at)(const struct arraylist_dyn_##name *self, const size_t index);
 * - T* ARRAYLIST_DYN_FN(name, begin)(const struct arraylist_dyn_##name *self);
 * - T* ARRAYLIST_DYN_FN(name, back)(const struct arraylist_dyn_##name *self);
 * - T* ARRAYLIST_DYN_FN(name, end)(const struct arraylist_dyn_##name *self);
 * - T* ARRAYLIST_DYN_FN(name, find)(const struct arraylist_dyn_##name *self, bool (*predicate)(T *elem, void *target), void *ctx);
 * - bool ARRAYLIST_DYN_FN(name, contains)(const struct arraylist_dyn_##name *self, bool (*predicate)(T *elem, void *target), void *ctx, size_t *out_index);
 * - size_t ARRAYLIST_DYN_FN(name, size)(const struct arraylist_dyn_##name *self);
 * - bool ARRAYLIST_DYN_FN(name, is_empty)(const struct arraylist_dyn_##name *self);
 * - size_t ARRAYLIST_DYN_FN(name, capacity)(const struct arraylist_dyn_##name *self);
 * - struct Allocator* ARRAYLIST_DYN_FN(name, get_allocator)(struct arraylist_dyn_##name *self);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, swap)(struct arraylist_dyn_##name *self, struct arraylist_dyn_##name *other);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, qsort)(struct arraylist_dyn_##name *self, bool (*comp)(T *n1, T *n2));
 * - struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, deep_clone)(const struct arraylist_dyn_##name *self, void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc));
 * - struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, shallow_copy)(const struct arraylist_dyn_##name *self);
 * - struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, steal)(struct arraylist_dyn_##name *self);
 * - enum arraylist_error ARRAYLIST_DYN_FN(name, clear)(struct arraylist_dyn_##name *self);
 * - void ARRAYLIST_DYN_FN(name, deinit)(struct arraylist_dyn_##name *self);
 */
#define ARRAYLIST_DECL_DYN(T, name) \
\
/** \
 * @brief init: Creates a new arraylist \
 * @param alloc Custom allocator instance, if null, default alloc will be used \
 * @param destructor Custom destructor function pointer, if null, the arraylist will not know \
 *        how to free the given value if needed, user will be responsible for freeing \
 * @return A zero initialized arraylist structure \
 * \
 * @note It does not allocate \
 * \
 * For pre-allocation (which is better for performance): \
 * @code \
 * struct arraylist_ints list = ints_init(alloc); \
 * if (ints_reserve(&list, expected_size) != ARRAYLIST_OK) { \
 *       // Handle or continue with default growth \
 * } \
 * @endcode \
 * \
 * @warning Call arraylist_dyn_##name##deinit() when done. \
 */ \
\
ARRAYLIST_UNUSED static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, init)( \
    const struct Allocator alloc, \
    void (*destructor)(T *type, struct Allocator *alloc) \
); \
\
/** \
 * @brief reserve: Reserves the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @param capacity New capacity of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, ARRAYLIST_ERR_NULL on null being passed, \
 *         ARRAYLIST_ERR_ALLOC on allocation failure, or ARRAYLIST_ERR_OVERFLOW on buffer overflow \
 * \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, reserve)( \
    struct arraylist_dyn_##name *self, \
    const size_t capacity \
); \
\
/** \
 * @brief shrink_size: Shrinks the arraylist's size to size passed, removing elements if necessary \
 * @param self Pointer to the arraylist \
 * @param size New size of the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_NULL on null being passed \
 * \
 * Returns early if arraylist is null or arraylist size is <= 0 or if arraylist size is >= size \
 *         or if provided size is < 0 \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, shrink_size)( \
    struct arraylist_dyn_##name *self, \
    const size_t size \
); \
\
/** \
 * @brief shrink_to_fit: Shrinks the capacity of the arraylist to fit the size, may reallocate and does not remove elements \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_OK if successful or on noop, or ARRAYLIST_ERR_ALLOC on allocation failure, \
 *         or ARRAYLIST_ERR_NULL on null being passed \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, shrink_to_fit)( \
    struct arraylist_dyn_##name *self \
); \
\
/** \
 * @brief push_back: Adds a new element by value to the end of the arraylist \
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
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, push_back)( \
    struct arraylist_dyn_##name *self, \
    T value \
); \
\
/** \
 * @brief emplace_back_slot: Returns a slot at the end of the arraylist for an object to be constructed \
 * @param self Pointer to the arraylist \
 * @return Pointer of type T to the slot for a type T to be constructed in place, NULL if self is \
 *         null or on any re/alloc failure or buffer overflow possibility \
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
 * For pointer types (T*), returns T** pointing to uninitialized pointer. \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_DYN_FN(name, emplace_back_slot)(struct arraylist_dyn_##name *self); \
\
/** \
 * @brief insert_at: Inserts an element in the given index \
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
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, insert_at)( \
    struct arraylist_dyn_##name *self, \
    T value, \
    const size_t index \
); \
\
/** \
 * @brief pop_back: Removes the last added element \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Does nothing on an empty arraylist \
 * \
 * @note The object will be destructed if a destructor is provided during arraylist init \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, pop_back)(struct arraylist_dyn_##name *self); \
\
/** \
 * @brief remove_at: Removes the element at position index \
 * @param self Pointer to the arraylist \
 * @param index Position to remove \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Will call destructor if available \
 * \
 * @warning if index < 0 size_t overflows and removes at end \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, remove_at)( \
    struct arraylist_dyn_##name *self, \
    const size_t index \
); \
\
/** \
 * @brief remove_from_to: Removes elements from index until to inclusive \
 * @param self Pointer to the arraylist \
 * @param from Starting position to remove \
 * @param to Ending position inclusive \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * Will call destructor if available \
 * If from > to does nothing \
 * \
 * @warning if from < 0 size_t overflows and removes at end, if to < 0, it will also overflow and may remove everything \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, remove_from_to)( \
    struct arraylist_dyn_##name *self, \
    size_t from, \
    size_t to \
); \
\
/** \
 * @brief at: Accesses the position of the arraylist at index \
 * @param self Pointer to the arraylist \
 * @param index Position to access \
 * @return A pointer to the value accessed or null if arraylist=null or index is OOB \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_DYN_FN(name, at)( \
    const struct arraylist_dyn_##name *self, \
    const size_t index \
); \
\
/** \
 * @brief begin: Accesses the first element of the arraylist \
 * @details It returns the block of memory allocated, can be used to iterate, \
 *          and where a function accepts a *T \
 * @param self Pointer to the arraylist \
 * @return A pointer to the first value or null if !self \
 * \
 * @code{.c} \
 * // Prints the contents of an arraylist of T \
 * for (T *it = t_begin(); it != t_end(); ++it) { t_print(); } \
 * @endcode \
 * \
 * @warning Return should be checked for null before usage \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_DYN_FN(name, begin)(const struct arraylist_dyn_##name *self); \
\
/** \
 * @brief back: Accesses the last position of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a *T same as begin, \
 *          but it will not be the end of the arraylist, just the last element, can be dereferenced \
 * @param self Pointer to the arraylist \
 * @return A pointer to the last value or null if !self or list is empty \
 * \
 * @warning Return should be checked for null before usage, calling back on an empty list and then \
 *          dereferencing it is UB \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_DYN_FN(name, back)(const struct arraylist_dyn_##name *self); \
\
/** \
 * @brief end: Accesses the end of the arraylist \
 * @details Can be used as an Iterator, where a function accepts a T* same as begin \
 * @param self Pointer to the arraylist \
 * @return A pointer to the end or null if !self \
 * \
 * @warning Return should be checked for null before usage, dereferencing it leads to UB, \
 *          even if NULL was not returned \
 */ \
ARRAYLIST_UNUSED static inline T* ARRAYLIST_DYN_FN(name, end)(const struct arraylist_dyn_##name *self); \
\
/** \
 * @brief find: Tries to finds the given value and returns it \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T *element with a void *target \
 *                  Must have the prototype: \
 *                  bool predicate(T *elem, void *target); \
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
ARRAYLIST_UNUSED static inline T* ARRAYLIST_DYN_FN(name, find)( \
    const struct arraylist_dyn_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx \
); \
\
/** \
 * @brief contains: Tries to finds the given value \
 * @param self Pointer to the arraylist \
 * @param predicate Function pointer responsible for comparing a T *element with a void *target \
 *                  Must have the prototype: \
 *                  bool predicate(T *elem, void *target); \
 * @param ctx A context to be used in the function pointer, usually used for comparing with a \
 *            field of type T or a member of it \
 * @param out_index The index if wanted \
 * @return True if found and out_index if provided will return the index where it was found, \
           false if not found and out_index is untouched, or self == NULL \
 * \
 * @note Performs a simple linear search, if performance matters, roll your own \
 *       sort and/or find functions \
 */ \
ARRAYLIST_UNUSED static inline bool ARRAYLIST_DYN_FN(name, contains)( \
    const struct arraylist_dyn_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx, \
    size_t *out_index \
); \
\
/** \
 * @brief size: Gets the size of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The size or 0 if arraylist is null \
 * \
 */ \
ARRAYLIST_UNUSED static inline size_t ARRAYLIST_DYN_FN(name, size)(const struct arraylist_dyn_##name *self); \
/** \
 * @brief is_empty: Checks if the arraylist is empty \
 * @param self Pointer to the arraylist \
 * @return False if arraylist is null or size = 0, otherwise true \
 */ \
ARRAYLIST_UNUSED static inline bool ARRAYLIST_DYN_FN(name, is_empty)(const struct arraylist_dyn_##name *self); \
\
/** \
 * @brief capacity: Gets the capacity of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The capacity or 0 if arraylist is null \
 */ \
ARRAYLIST_UNUSED static inline size_t ARRAYLIST_DYN_FN(name, capacity)(const struct arraylist_dyn_##name *self); \
\
/** \
 * @brief get_allocator: Gets the allocator of an arraylist \
 * @param self Pointer to the arraylist \
 * @return The allocator or NULL if self == null \
 */ \
ARRAYLIST_UNUSED static inline struct Allocator* ARRAYLIST_DYN_FN(name, get_allocator)( \
    struct arraylist_dyn_##name *self \
); \
\
/** \
 * @brief swap: Swaps the contents of arraylist self with other \
 * @param self Pointer to the arraylist \
 * @param other Pointer to another arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, swap)( \
    struct arraylist_dyn_##name *self, \
    struct arraylist_dyn_##name *other \
); \
\
/** \
 * @brief qsort: Sorts the self based on the given comp function \
 * @param self Pointer to the arraylist \
 * @param comp Function that knows how to compare two T types \
 * @return ARRAYLIST_ERR_NULL if self == null or if fn comp == null, otherwise ARRAYLIST_OK \
 * \
 * @note Performs a simple quicksort, non-stable, pivot is always the last element, \
 *       if performance matters, roll your own sort functions \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, qsort)( \
    struct arraylist_dyn_##name *self, \
    bool (*comp)(T *n1, T *n2) \
); \
\
/** \
 * @brief deep_clone: Deeply clones an arraylist \
 * @param self Pointer to the arraylist to copy \
 * @param deep_clone_fn Function that knows how to clone a single element \
 *                      Must have the prototype: \
 *                      void deep_clone_fn(T *dst, const T *src, struct Allocator *alloc); \
 * @return A new arraylist struct that is independent of the self \
 * \
 * @note The correctness of this function depends on the provided deep_clone_fn parameter \
 *       for value types, int or pod structs, deep_clone_fn can simply assign *dst = *src; \
 *       if T contains pointers or heap allocations, then deep_clone_fn must allocate/copy \
 *       these fields as needed, if using arraylist of pointers to T, deep_clone_fn must allocate \
 *       a new T and, if it contains member to pointers, allocate/copy as needed \
 * \
 * @warning If self is NULL or deep_clone_fn if NULL then it returns a zero-initialized struct, \
 *          if asserts are enabled then it crashes \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked \
 */ \
ARRAYLIST_NODISCARD ARRAYLIST_UNUSED static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, deep_clone)( \
    const struct arraylist_dyn_##name *self, \
    void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc) \
); \
\
/** \
 * @brief shallow_copy: Copies an arraylist bit by bit \
 * @param self Pointer to the arraylist to copy \
 * @return A new arraylist struct \
 * \
 * @note If T is or contains pointer or heap-allocated data, those pointers are copied as-is, \
 *       both the original and copy will reference the same memory. Modifying or freeing elements \
 *       in one list affects the other, this is unsafe if not careful, may cause double frees or incorrect assumptions. \
 *       Use deep_clone function with a correct function parameter for truly independent copies \
 * \
 * @warning If self is NULL then it returns a zero-initialized struct, if asserts are enabled then it crashes \
 */ \
ARRAYLIST_UNUSED static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, shallow_copy)( \
    const struct arraylist_dyn_##name *self \
); \
\
/** \
 * @brief move: Moves the arraylist in the parameter to the lhs, leaving self as NULL \
 * @param self Pointer to the arraylist to move \
 * @return A new arraylist struct \
 * \
 * @note The self parameter will be left in a used, but NULL/uninitialized state and should not be used \
 *       to reuse it, one must call init again and reinitialize it \
 * \
 * @note The self parameter will be left in an unusable, NULL/uninitialized state and should not be used \
 *       to reuse it, one must call init again and reinitialize it \
 * \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked \
 */ \
ARRAYLIST_NODISCARD ARRAYLIST_UNUSED static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, steal)( \
    struct arraylist_dyn_##name *self \
); \
\
/** \
 * @brief clear: Clears the arraylist's data \
 * @param self Pointer to the arraylist \
 * @return ARRAYLIST_ERR_NULL in case of NULL being passed, or ARRAYLIST_OK \
 * \
 * It does not free the arraylist itself and does not alter the capacity, only sets its size to 0 \
 * Safe to call on NULL, returns early \
 * \
 * @note Will call the object's destructor on objects if available \
 */ \
ARRAYLIST_UNUSED static inline enum arraylist_error ARRAYLIST_DYN_FN(name, clear)(struct arraylist_dyn_##name *self); \
\
/** \
 * @brief deinit: Destroys and frees an arraylist \
 * @param self Pointer to the arraylist to deinit \
 * \
 * Frees the internal data array and resets the fields \
 * Safe to call on NULL or already deinitialized arraylists, returns early \
 * \
 * @note Will call the destructor on data items if provided \
 * @note The self parameter will be left in an unusable, NULL/uninitialized state and should not be used \
 *       to reuse it, one must call init again and reinitialize it \
 */ \
ARRAYLIST_UNUSED static inline void ARRAYLIST_DYN_FN(name, deinit)(struct arraylist_dyn_##name *self);

/** \
 * @def ARRAYLIST_IMPL_DYN(T, name)
 * @brief Implements all functions for an arraylist type
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 *
 * Implements the functions of the ARRAYLIST_DECL macro
 *
 * @note This macro should be used in a .c file, not in a header
 */ 
#define ARRAYLIST_IMPL_DYN(T, name) \
\
/* =========================== PRIVATE FUNCTIONS =========================== */ \
\
/** \
 * @private \
 * @brief double_capacity: Function that deals with capacity and (re)alloc if necessary \
 * @param self Pointer to the arraylist to deinit \
 * @return ARRAYLIST_OK if successful, ARRAYLIST_ERR_OVERFLOW if buffer will overflow, \
 *         or ARRAYLIST_ERR_NULL if allocation failure \
 * \
 * This private function ensures that capacity of the arraylist is atleast min_cap \
 * If the self->capacity is 0 (first allocation) will call malloc, otherwise realloc \
 * Then set the self->capacity to min_cap \
 * \
 * @warning Assumes self is not null, as this is a private function, this is not really a problem \
 */ \
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, double_capacity)(struct arraylist_dyn_##name *self) { \
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
 * @brief swap_elems: Simple swap function \
 */ \
static inline void ARRAYLIST_DYN_FN(name, swap_elems)(T *a, T *b) { \
    T tmp = *a; \
    *a = *b; \
    *b = tmp; \
} \
\
/** \
 * @private \
 * @brief partition_buffer: Helper function to use in the sort algo \
 */ \
static inline size_t ARRAYLIST_DYN_FN(name, partition_buffer)( \
    struct arraylist_dyn_##name *self, \
    size_t low, \
    size_t high, \
    bool (*comp)(T *n1, T *n2) \
) { \
    T *pivot = &self->data[high]; \
    size_t i = low; \
    \
    /* Traverse buffer */ \
    for (size_t j = low; j < high; ++j) { \
        if (comp(&self->data[j], pivot)) { \
            /* Move elements to the left side */ \
            ARRAYLIST_DYN_FN(name, swap_elems)(&self->data[i], &self->data[j]); \
            ++i; \
        } \
    } \
    \
    /* Move pivot after smaller elements and return its position */ \
    ARRAYLIST_DYN_FN(name, swap_elems)(&self->data[i], &self->data[high]); \
    return i; \
} \
\
/** \
 * @private \
 * @brief helper_qsort: Helper recursive function for the sort algo \
 */ \
static inline void ARRAYLIST_DYN_FN(name, helper_qsort)( \
    struct arraylist_dyn_##name *self, \
    size_t low, \
    size_t high, \
    bool (*comp)(T *n1, T *n2) \
) { \
    if (low < high) { \
        /* part_idx is the parition return index of pivot */ \
        size_t part_idx = ARRAYLIST_DYN_FN(name, partition_buffer)(self, low, high, comp); \
        /* prevent a size_t underflow */ \
        if (part_idx > 0) { \
            /* recursion calls for smaller elements */ \
            ARRAYLIST_DYN_FN(name, helper_qsort)(self, low, part_idx - 1, comp); \
        } \
        /* greater elements */ \
        ARRAYLIST_DYN_FN(name, helper_qsort)(self, part_idx + 1, high, comp); \
    } \
} \
\
/* =========================== PUBLIC FUNCTIONS =========================== */ \
\
static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, init)( \
    const struct Allocator alloc, \
    void (*destructor)(T *type, struct Allocator *alloc) \
) { \
    struct arraylist_dyn_##name arraylist = {0}; \
    arraylist.alloc = alloc; \
    arraylist.destructor = destructor; \
    arraylist.size = 0; \
    arraylist.capacity = 0; \
    arraylist.data = NULL; \
    return arraylist; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, reserve)( \
    struct arraylist_dyn_##name *self, \
    const size_t capacity \
) { \
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
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, shrink_size)( \
    struct arraylist_dyn_##name *self, \
    const size_t size \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (size >= self->size || self->size <= 0) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        for (size_t i = size; i < self->size; i++) { \
            self->destructor(&self->data[i], &self->alloc); \
        } \
    } \
    self->size = size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, shrink_to_fit)(struct arraylist_dyn_##name *self) { \
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
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, push_back)(struct arraylist_dyn_##name *self, T value) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = ARRAYLIST_DYN_FN(name, double_capacity)(self); \
        if (err != ARRAYLIST_OK) { \
            return err; \
        } \
    } \
    self->data[self->size++] = value; \
    return ARRAYLIST_OK; \
} \
\
static inline T* ARRAYLIST_DYN_FN(name, emplace_back_slot)(struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = ARRAYLIST_DYN_FN(name, double_capacity)(self); \
        if (err != ARRAYLIST_OK) { \
            return NULL; \
        } \
    } \
    return &self->data[self->size++]; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, insert_at)( \
    struct arraylist_dyn_##name *self, \
    T value, \
    const size_t index \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
        return ARRAYLIST_DYN_FN(name, push_back)(self, value); \
    } \
    if (self->size >= self->capacity) { \
        enum arraylist_error err = ARRAYLIST_DYN_FN(name, double_capacity)(self); \
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
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, pop_back)(struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size <= 0) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        self->destructor(&self->data[self->size - 1], &self->alloc); \
    } \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, remove_at)( \
    struct arraylist_dyn_##name *self, \
    const size_t index \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (index >= self->size) { \
        return ARRAYLIST_DYN_FN(name, pop_back)(self); \
    } \
    if (self->destructor) { \
        self->destructor(&self->data[index], &self->alloc); \
    } \
    for (size_t i = index; i < self->size - 1; ++i) { \
        self->data[i] = self->data[i + 1]; \
    } \
    --self->size; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, remove_from_to)( \
    struct arraylist_dyn_##name *self, \
    size_t from, \
    size_t to \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size == 0) { \
        return ARRAYLIST_OK; \
    } \
    if (from > to) { \
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
            self->destructor(&self->data[i], &self->alloc); \
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
static inline T* ARRAYLIST_DYN_FN(name, at)( \
    const struct arraylist_dyn_##name *self, \
    const size_t index \
) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    ARRAYLIST_ENSURE_PTR(index < self->size) \
    return &self->data[index]; \
} \
\
static inline T* ARRAYLIST_DYN_FN(name, begin)(const struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data; \
} \
\
static inline T* ARRAYLIST_DYN_FN(name, back)(const struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    ARRAYLIST_ENSURE_PTR(self->size != 0) \
    return self->data + (self->size - 1); \
} \
\
static inline T* ARRAYLIST_DYN_FN(name, end)(const struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return self->data + self->size; \
} \
\
static inline T* ARRAYLIST_DYN_FN(name, find)( \
    const struct arraylist_dyn_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx \
) { \
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
static inline bool ARRAYLIST_DYN_FN(name, contains)( \
    const struct arraylist_dyn_##name *self, \
    bool (*predicate)(T *elem, void *target), \
    void *ctx, \
    size_t *out_index \
) { \
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
static inline size_t ARRAYLIST_DYN_FN(name, size)(const struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, 0) \
    return self ? self->size : 0; \
} \
\
static inline bool ARRAYLIST_DYN_FN(name, is_empty)(const struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, false) \
    return self->size == 0 ? true : false; \
} \
\
static inline size_t ARRAYLIST_DYN_FN(name, capacity)(const struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE(self != NULL, 0) \
    return self->capacity; \
} \
\
static inline struct Allocator* ARRAYLIST_DYN_FN(name, get_allocator)(struct arraylist_dyn_##name *self) { \
    ARRAYLIST_ENSURE_PTR(self != NULL) \
    return &self->alloc; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, swap)( \
    struct arraylist_dyn_##name *self, \
    struct arraylist_dyn_##name *other \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    ARRAYLIST_ENSURE(other != NULL, ARRAYLIST_ERR_NULL) \
    struct arraylist_dyn_##name temp = *other; \
    *other = *self; \
    *self = temp; \
    return ARRAYLIST_OK; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, qsort)( \
    struct arraylist_dyn_##name *self, \
    bool (*comp)(T *n1, T *n2) \
) { \
    ARRAYLIST_ENSURE(self != NULL, ARRAYLIST_ERR_NULL) \
    ARRAYLIST_ENSURE(comp != NULL, ARRAYLIST_ERR_NULL) \
    if (self->size > 1) { \
        ARRAYLIST_DYN_FN(name, helper_qsort)(self, 0, self->size - 1, comp); \
    } \
    return ARRAYLIST_OK; \
} \
\
static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, deep_clone)( \
    const struct arraylist_dyn_##name *self, \
    void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc) \
) { \
    struct arraylist_dyn_##name clone = { 0 }; \
    ARRAYLIST_ENSURE(self != NULL, clone); \
    ARRAYLIST_ENSURE(deep_clone_fn != NULL, clone); \
    clone = ARRAYLIST_DYN_FN(name, init)(self->alloc, self->destructor); \
    ARRAYLIST_DYN_FN(name, reserve)(&clone, self->capacity); \
    clone.size = self->size; \
    for (size_t i = 0; i < self->size; ++i) { \
        deep_clone_fn(&clone.data[i], &self->data[i], &clone.alloc); \
    } \
    return clone; \
} \
\
static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, shallow_copy)( \
    const struct arraylist_dyn_##name *self \
) { \
    struct arraylist_dyn_##name clone = { 0 }; \
    ARRAYLIST_ENSURE(self != NULL, clone); \
    clone = ARRAYLIST_DYN_FN(name, init)(self->alloc, self->destructor); \
    ARRAYLIST_DYN_FN(name, reserve)(&clone, self->capacity); \
    clone.size = self->size; \
    for (size_t i = 0; i < self->size; ++i) { \
        clone.data[i] = self->data[i]; \
    } \
    return clone; \
} \
\
static inline struct arraylist_dyn_##name ARRAYLIST_DYN_FN(name, steal)(struct arraylist_dyn_##name *self) { \
    struct arraylist_dyn_##name steal = { 0 }; \
    ARRAYLIST_ENSURE(self != NULL, steal); \
    steal.data = self->data; \
    steal.size = self->size; \
    steal.capacity = self->capacity; \
    steal.alloc = self->alloc; \
    steal.destructor = self->destructor; \
    self->data = NULL; \
    self->size = 0; \
    self->capacity = 0; \
    memset(&self->alloc, 0, sizeof(self->alloc)); \
    return steal; \
} \
\
static inline enum arraylist_error ARRAYLIST_DYN_FN(name, clear)(struct arraylist_dyn_##name *self) { \
    if (!self || self->size == 0) { \
        return ARRAYLIST_OK; \
    } \
    if (self->destructor) { \
        for (size_t i = 0; i < self->size; ++i) { \
            self->destructor(&self->data[i], &self->alloc); \
        } \
    } \
    self->size = 0; \
    return ARRAYLIST_OK; \
} \
\
static inline void ARRAYLIST_DYN_FN(name, deinit)(struct arraylist_dyn_##name *self) { \
    if (!self) { \
        return; \
    } \
    if (self->data) { \
        if (self->destructor) { \
            for (size_t i = 0; i < self->size; ++i) { \
                self->destructor(&self->data[i], &self->alloc); \
            } \
        } \
        self->alloc.free(self->data, self->capacity * sizeof(T), self->alloc.ctx); \
        self->data = NULL; \
    } \
    self->size = 0; \
    self->capacity = 0; \
    memset(&self->alloc, 0, sizeof(self->alloc)); \
    self->destructor = NULL; \
    return; \
}

/**
 * @def ARRAYLIST(T, name)
 * @brief Helper macro to define, declare and implement all in one
 * @param T The type arraylist will hold
 * @param name The name suffix for the arraylist type
 */
#define ARRAYLIST_DYN(T, name) \
ARRAYLIST_TYPE_DYN(T, name) \
ARRAYLIST_DECL_DYN(T, name) \
ARRAYLIST_IMPL_DYN(T, name)

// clang-format on

#endif // ARRAYLIST_H
