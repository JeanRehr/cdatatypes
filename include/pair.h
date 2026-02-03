/**
 * @file pair.h
 * @brief Generic and typesafe pair implementation using macros
 *
 * This header provides a generic pair (2-tuple) implementation using C99 macros, similar to c++
 * std::pair or std::tuple. It allows creating type-safe pairs of any two types with configurable
 * memory management and destructors.
 *
 * @details
 * The pair struct is a simple container that holds two values that can be of different types.
 * It's stack-allocated by default and doesn't allocate memory for itself. However, it provides
 * facilities to manage heap-allocated data stored within the pair through destructor functions.
 *
 * It includes support for:
 * - Compile-time destructors (macro-based)
 * - Deep and shallow copying
 * - Move semantics with steal()
 * - Custom allocator support via allocator.h interface
 *
 * It is a zero-cost abstraction, if no destructor is provided, all code for it is eliminated.
 *
 * Regarding memory management:
 * - The pair struct itself is always stack-allocated
 * - If elements contain heap-allocated memory, destructors may be provided
 * - The deinit() function calls element destructors but doesn't free the pair itself
 * - For pointer types, the destructor should handle freeing the pointed-to memory
 *
 * Regarding the destructor functions:
 * Destructors must have the signature: `void dtor(T *element, struct Allocator *alloc)`
 * Where T is the element type (K or V). For pointer types T*, the destructor
 * receives a T** and should free the pointed-to object and set the pointer to NULL.
 *
 * Comparison to c++ std::pair:
 * - Similar to std::pair<K, V> but with manual memory management
 * - No operator overloading (==, <, etc.)
 * - Move and copy semantics must be explicitly handled
 * - Comparisons must be done manually with the type defined
 * - Destructors are explicit rather than automatic
 *
 * Equivalent operations to c++ pair:
 * | c++ std::pair         | C pair                                                                           |
 * |-----------------------|----------------------------------------------------------------------------------|
 * | pair<K,V> p = {k, v}; | PAIR(K, V, name, dtor_first, dtor_second); struct pair_name p = name_init(k, v); |
 * | p.first               | p.first                                                                          |
 * | p.second              | p.second                                                                         |
 * | std::swap(p1, p2)     | name_swap(&p1, &p2)                                                              |
 * | p2 = p1               | struct pair_name p2 = name_shallow_copy(&p1)                                     |
 * | p2 = std::move(p1)    | struct pair_name p2 = name_steal(&p1)                                            |
 * | ~pair()               | name_deinit(&p, alloc)                                                           |
 *
 * If there is a deep copy constructor, then assignment on pair is equal to:
 * `struct pair_name p2 = name_deep_clone(&p, deep_clone_first_fn, deep_clone_second_fn, alloc)`
 *
 * Usage example:
 * @code
 * // Simple value types (int, float, POD structs)
 * #define noop_dtor(ptr, alloc) (void)0
 * PAIR(int, float, int_float_pair, noop_dtor, noop_dtor)
 * struct pair_int_float_pair p = int_float_pair_init(42, 3.14f);
 * printf("First: %d, Second: %f\n", p.first, p.second);
 * int_float_pair_deinit(&p, allocator_get_default());
 * 
 * // Complex types with destructors
 * void String_dtor(char **str, struct Allocator *alloc) {
 *     if (!str || !*str) return;
 *     alloc->free(*str, strlen(*str) + 1, alloc->ctx);
 *     *str = NULL;
 * }
 * 
 * void Person_dtor(struct Person *p, struct Allocator *alloc) {
 *     if (!p) return;
 *     alloc->free(p->name, strlen(p->name) + 1, alloc->ctx);
 * }
 * 
 * PAIR(char*, struct Person, string_person_pair, String_dtor, Person_dtor)
 * 
 * struct Allocator *alloc = allocator_get_default();
 * struct pair_string_person_pair pair = string_person_pair_init(
 *     strdup("nominee"), 
 *     (struct Person){.name = strdup("name example"), .age = 69}
 * );
 * // ... use pair ...
 * string_person_pair_deinit(&pair, alloc);
 * @endcode
 *
 * The destructor functions may be macro-based, and the compiler will typically
 * optimize either function or macro equally, but the macro is more guaranteed to
 * be inlined.
 *
 * Example for a destructor macro:
 * @code
 * #define Person_dtor_macro(person_ptr, alloc) \
 * do { \
 *     if (!person_ptr) break; \
 *     if ((person_ptr)->name) { \
 *         (alloc)->free((person_ptr)->name, strlen((person_ptr)->name) + 1, (alloc)->ctx); \
 *         (person_ptr)->name = NULL; \
 *     } \
 * } while (0)
 * @endcode
 *
 * Pass noop_dtor for types that don't need cleanup and enable LTO for maximum optimization
 *
 * Thread safety:
 * - Individual pairs are not thread-safe
 * - Operations on different pair instances are independent
 * - Concurrent access to the same pair requires external synchronization
 *
 * Error Handling:
 * When PAIR_USE_ASSERT=1 (default 0):
 * - Invalid operations trigger assert() and abort
 * When PAIR_USE_ASSERT=0:
 * - Functions return error codes or zero-initialized structs
 * - User must check return values
 *
 * Requirements:
 * This pair requires a file named "allocator.h", which is a simple allocator interface that
 * defines a struct Allocator with 4 fields:
 * - void *(*malloc)(size_t size, void *ctx);
 * - void *(*realloc)(void *ptr, size_t old_size, size_t new_size, void *ctx)
 * - void (*free)(void *ptr, size_t size, void *ctx);
 * - void *ctx;
 *
 * And defines a function named allocator_get_default which returns a struct Allocator with
 * initialized fields to a default allocator of choice (std libc memory functions in this case).
 *
 * Very easy to provide your own allocator.h and/or custom allocators by
 * implementing the Allocator interface
 *
 * Compiled and tested with GCC, Clang and MSVC
 * Also supports c++ compilation (extern "C"), with some warnings
 * No platform-specific dependencies
 *
 * @warning Always call deinit() when done with pairs containing heap-allocated memory
 * @warning Use deep_clone() for independent copies of complex types
 * @warning The steal() function invalidates the source pair
 * @warning Destructors must handle null pointers
 * @warning After deinit() or steal() the pair will be left in a zeroed state
 * @warning For pairs containing only value types (no pointers), deinit() is optional
 *          but harmless if called with noop destructors
 */
#ifndef PAIR_H
#define PAIR_H

#include <stdbool.h> // For bool, true, false
#include <string.h>  // For memset()

#include "allocator.h" // For a custom Allocator interface

#ifdef __cplusplus
extern "C" {
#endif // extern "C"

/**
 * @def PAIR_UNUSED
 * @brief Defines a macro to supress the warning for unused function because of static inline
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define PAIR_UNUSED [[maybe_unused]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define PAIR_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
    #define PAIR_UNUSED __attribute__((unused))
#else
    #define PAIR_UNUSED
#endif // PAIR_UNUSED definition

/**
 * @def PAIR_NODISCARD
 * @brief Defines a macro to warn of discarded unused results when they matter
 *        (possible leak of memory is involved)
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define PAIR_NODISCARD [[nodiscard]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define PAIR_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
    #define PAIR_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
    #if defined(_SAL_VERSION_SOURCE) && _SAL_VERSION_SOURCE >= 2
        #ifndef _Check_return_
            #include <sal.h>
        #endif // _Check_return_ from sal.h
        #define PAIR_NODISCARD _Check_return_
    #else
        #define PAIR_NODISCARD
    #endif // _SAL_VERSION_SOURCE && _SAL_VERSION_SOURCE >= 2
#else
    #define PAIR_NODISCARD
#endif // PAIR_NODISCARD definition

/**
 * @def PAIR_USE_ASSERT
 * @brief Defines if the pair will use asserts or return error codes
 * @details If PAIR_USE_ASSERT is 1, then the lib will assert and fail early, otherwise,
 *          defensive programming and returning error codes will be used
 */
#ifndef PAIR_USE_ASSERT
    #define PAIR_USE_ASSERT 0
#endif // PAIR_USE_ASSERT

#include <assert.h> // For assert()

#if PAIR_USE_ASSERT
    #define PAIR_ENSURE(cond, return_val) assert(cond);
    #define PAIR_ENSURE_VOID(cond) assert(cond);
#else
    #define PAIR_ENSURE(cond, return_val)                                                                              \
        if (!(cond))                                                                                                   \
            return (return_val);
    #define PAIR_ENSURE_VOID(cond)                                                                                     \
        if (!(cond))                                                                                                   \
            return;
#endif // PAIR_USE_ASSERT if directive

/**
 * @enum pair_error
 * @brief Error codes for the pair, there are not many ways a pair data structure can fail
 */
enum pair_error {
    PAIR_OK = 0,            ///< No error
    PAIR_ERR_NULL = -1,     ///< Null pointer
};

/* ====== PAIR Macro destructor version START ====== */

// clang-format off

/**
 * @def PAIR_USE_PREFIX
 * @brief Defines at compile-time if the functions will use pair_* prefix
 * @details 
 * Generates functions with the pattern pair_##name##_function() instead of name##_function()
 * Must be defined before including the pair.h header, it will be per TU/file, so once defined
 * can't be undef in the same TU for different pair types and names.
 * The struct generated will always have the prefix pair_##name to them.
 * Inspiration taken from the Tsoding's nob.h lib, and some other libs like jemalloc which also does
 * something similar.
 *
 * The default is without prefixes, as the "name" parameter already acts like a user defined prefix:
 * @code
 * PAIR(int, int, intsp, noopdtor, noopdtor)
 * struct pair_intsp intpair = intsp_init(...);
 * @endcode
 *
 * Usage:
 * @code
 * // In a header that will include pair.h and will define some pair types:
 * #define PAIR_USE_PREFIX
 * #include "pair.h"
 * PAIR_TYPE(int, int, intsp) // Creates a struct: struct pair_intsp;
 * PAIR_DECL(int, int, intsp) // Declares functions named pair_intsp_init(...)
 * // In a .c file:
 * PAIR_IMPL(int, int, intsp, noopdtor, noopdtor)
 * @endcode
 */
#ifdef PAIR_USE_PREFIX
    #define PAIR_FN(name, func) pair_##name##_##func
#else
    #define PAIR_FN(name, func) name##_##func
#endif

/**
 * @def PAIR_TYPE(K, V, name)
 * @brief Defines a pair structure for specific types K and V
 * @param K The first type pair will hold
 * @param V The second type pair will hold
 * @param name The name suffix for the pair type
 *
 * This macro defines a struct named "pair_##name" with the following fields:
 * - "first": Value of type K
 * - "second": Value of type V
 *
 * @code
 * // Example: Define a pair of integers
 * PAIR_TYPE(int, int, intsp)
 * // Creates a struct named struct pair_intsp
 * @endcode
 */
#define PAIR_TYPE(K, V, name)                                                                                          \
struct pair_##name {                                                                                                   \
    K first;                                                                                                           \
    V second;                                                                                                          \
};

/**
 * @def PAIR_DECL(K, V, name)
 * @brief Declares all functions for a pair type
 * @param K The first type pair will hold
 * @param V The second type pair will hold
 * @param name The name suffix for the pair type
 *
 * @details
 * The following functions are declared:
 * - static inline struct pair_##name PAIR_FN(name, init)(const K first, const V second);
 * - static inline enum pair_error PAIR_FN(name, swap)(struct pair_##name *self, struct pair_##name *other);
 * - static inline struct pair_##name PAIR_FN(name, deep_clone)(struct pair_##name *self, void (*deep_clone_first_fn)(K *dst, K *src, struct Allocator *alloc), void (*deep_clone_second_fn)(V *dst, V *src, struct Allocator *alloc), struct Allocator *alloc);
 * - static inline struct pair_##name PAIR_FN(name, shallow_copy)(const struct pair_##name *self);
 * - static inline struct pair_##name PAIR_FN(name, steal)(struct pair_##name *self);
 * - static inline void PAIR_FN(name, deinit)(struct pair_##name *self, struct Allocator *alloc);
 */
#define PAIR_DECL(K, V, name)                                                                                          \
/**                                                                                                                    \
 * @brief init: Creates a new pair struct                                                                              \
 * @param first The first value of the pair                                                                            \
 * @param second The second value of the pair                                                                          \
 * @return A pair with the given values                                                                                \
 * @details it does not allocate any memory for the pair itself, all members are values                                \
 */                                                                                                                    \
PAIR_UNUSED static inline struct pair_##name PAIR_FN(name, init)(const K first, const V second);                       \
                                                                                                                       \
/**                                                                                                                    \
 * @brief swap: Swaps the contents of one pair with other                                                              \
 * @param self Pointer to the pair                                                                                     \
 * @param other Pointer to another pair to swap with                                                                   \
 * @return PAIR_ERR_NULL if any of the params are null, or PAIR_OK                                                     \
 */                                                                                                                    \
PAIR_UNUSED static inline enum pair_error PAIR_FN(name, swap)(struct pair_##name *self, struct pair_##name *other);    \
                                                                                                                       \
/**                                                                                                                    \
 * @brief deep_clone: Deeply clones a pair                                                                             \
 * @param self Pointer to the pair                                                                                     \
 * @param deep_clone_first_fn Pointer to a funtion that knows how to clone the first value of the pair                 \
 *                            Must have the prototype:                                                                 \
 *                            void (*deep_clone_first_fn)(K *dst, K *src, struct Allocator *alloc);                    \
 * @param deep_clone_second_fn Pointer to a funtion that knows how to clone the second value of the pair               \
 *                             Must have the prototype:                                                                \
 *                             void (*deep_clone_second_fn)(V *dst, V *src, struct Allocator *alloc);                  \
 * @param alloc Pointer to an allocator struct, same one used in the deep_clone function pointers                      \
 * @return A new pair struct that's a clone of self                                                                    \
 *                                                                                                                     \
 * @note The correctness of this function depends on the provided function pointer params. If the                      \
 *       pair only hold simple values, or values that do not hold heap-allocated mem or pointers,                      \
 *       then using the shallow_copy is preferred, if any of the K or V values have ptr or are ptr                     \
 *       themselves, or contain heap allocated mem, then the function pointers must allocate/copy                      \
 *       these fields as needed, if K or V are pointers, then allocate K or V itself as well                           \
 *                                                                                                                     \
 * @warning If self is NULL, or deep_clone_first_fn, or deep_clone_second_fn if NULL then it returns                   \
 *          a zero-initialized struct, if asserts are enabled then it crashes                                          \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked                     \
 */                                                                                                                    \
PAIR_NODISCARD PAIR_UNUSED static inline struct pair_##name PAIR_FN(name, deep_clone)(                                 \
    struct pair_##name *self,                                                                                          \
    void (*deep_clone_first_fn)(K *dst, K *src, struct Allocator *alloc),                                              \
    void (*deep_clone_second_fn)(V *dst, V *src, struct Allocator *alloc),                                             \
    struct Allocator *alloc                                                                                            \
);                                                                                                                     \
                                                                                                                       \
/**                                                                                                                    \
 * @brief shallow_copy: Copies a pair bit by bit                                                                       \
 * @param self Pointer to the pair                                                                                     \
 * @return A new pair struct, same as the self                                                                         \
 *                                                                                                                     \
 * @note If K or V is or contains pointer or heap-allocated data, those pointers are copied as-is,                     \
 *       both the original and copy will reference the same memory. Modifying or freeing elements                      \
 *       in one list affects the other, this is unsafe if not careful, may cause double frees or                       \
 *       incorrect assumptions. Use deep_clone function with correct function parameters                               \
 *       for truly independent copies                                                                                  \
 *                                                                                                                     \
 * @warning If self is NULL then it returns a zero-initialized struct,                                                 \
 *          if asserts are enabled then it crashes                                                                     \
 */                                                                                                                    \
PAIR_UNUSED static inline struct pair_##name PAIR_FN(name, shallow_copy)(const struct pair_##name *self);              \
                                                                                                                       \
/**                                                                                                                    \
 * @brief steal: Steals the pair in the parameter, moving it to the lhs, then zeroes self                              \
 * @param self Pointer to the pair to steal/move                                                                       \
 * @return A new pair struct                                                                                           \
 *                                                                                                                     \
 * @note The self parameter will be left in a zeroed state and should not be used, to reuse it,                        \
 *       one must reinitialize its fields again                                                                        \
 *                                                                                                                     \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked                     \
 */                                                                                                                    \
PAIR_NODISCARD PAIR_UNUSED static inline struct pair_##name PAIR_FN(name, steal)(struct pair_##name *self);            \
                                                                                                                       \
/**                                                                                                                    \
 * @brief deinit: Destroys and frees the values inside the pair                                                        \
 * @param self Pointer to the pair to deinit                                                                           \
 * @param alloc Pointer to the Allocator struct used to allocate                                                       \
 *                                                                                                                     \
 * @details This function should ever only be called if any of the pair values are pointers or have                    \
 *          heap allocated memory, this will only call the destructor set in the PAIR_IMPL macro                       \
 *          It will otherwise do nothing and should not be called, struct pairs are usually value                      \
 *          only, outside of cases like a pair that has a char * or a struct that contains pointers                    \
 *                                                                                                                     \
 * Safe to call on NULL or already deinitialized pair, returns early                                                   \
 *                                                                                                                     \
 * @note The self parameter will be left in a zeroed state and should not be used, to reuse it,                        \
 *       one must reinitialize its fields again                                                                        \
 */                                                                                                                    \
PAIR_UNUSED static inline void PAIR_FN(name, deinit)(struct pair_##name *self, struct Allocator *alloc);

/**
 * @def PAIR_IMPL(K, V, name, dtor_first, dtor_second)
 * @brief Implements all functions for a pair type
 * @param K The first type pair will hold
 * @param V The second type pair will hold
 * @param name The name sufix for the pair type
 * @param dtor_first The destructor for the first type if needed
 * @param dtor_second The destructor for the second type if needed
 *
 * Implements the functions of the PAIR_DECL macro
 *
 * @note This macro should be used in a .c file, not in a header
 * @note The two last parameters may be a macro or a function, recommended to inline the function
 *
 * @warning If the type K (dtor_first) or V (dtor_second) doesn't need to have a destructor, or one
 *          doesn't want to pass it and manually free, then a noop must be passed, like (void),
 *          or a macro/function that does nothing.
 */
#define PAIR_IMPL(K, V, name, dtor_first, dtor_second)                                                                 \
static inline struct pair_##name PAIR_FN(name, init)(const K first, const V second) {                                  \
    struct pair_##name pair = { 0 };                                                                                   \
    pair.first = first;                                                                                                \
    pair.second = second;                                                                                              \
    return pair;                                                                                                       \
}                                                                                                                      \
                                                                                                                       \
static inline enum pair_error PAIR_FN(name, swap)(struct pair_##name *self, struct pair_##name *other) {               \
    PAIR_ENSURE(self != NULL, PAIR_ERR_NULL);                                                                          \
    PAIR_ENSURE(other != NULL, PAIR_ERR_NULL);                                                                         \
    struct pair_##name temp = *other;                                                                                  \
    *other = *self;                                                                                                    \
    *self = temp;                                                                                                      \
    return PAIR_OK;                                                                                                    \
}                                                                                                                      \
                                                                                                                       \
static inline struct pair_##name PAIR_FN(name, deep_clone)(                                                            \
    struct pair_##name *self,                                                                                          \
    void (*deep_clone_first_fn)(K *dst, K *src, struct Allocator *alloc),                                              \
    void (*deep_clone_second_fn)(V *dst, V *src, struct Allocator *alloc),                                             \
    struct Allocator *alloc                                                                                            \
) {                                                                                                                    \
    struct pair_##name clone = { 0 };                                                                                  \
    PAIR_ENSURE(self != NULL, clone);                                                                                  \
    PAIR_ENSURE(deep_clone_first_fn != NULL, clone);                                                                   \
    PAIR_ENSURE(deep_clone_second_fn != NULL, clone);                                                                  \
    deep_clone_first_fn(&clone.first, &self->first, alloc);                                                            \
    deep_clone_second_fn(&clone.second, &self->second, alloc);                                                         \
    return clone;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
static inline struct pair_##name PAIR_FN(name, shallow_copy)(const struct pair_##name *self) {                         \
    struct pair_##name clone = { 0 };                                                                                  \
    PAIR_ENSURE(self != NULL, clone);                                                                                  \
    clone = *self;                                                                                                     \
    return clone;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
static inline struct pair_##name PAIR_FN(name, steal)(struct pair_##name *self) {                                      \
    struct pair_##name steal = { 0 };                                                                                  \
    PAIR_ENSURE(self != NULL, steal);                                                                                  \
    steal = *self;                                                                                                     \
    memset(self, 0, sizeof(*self));                                                                                    \
    return steal;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
static inline void PAIR_FN(name, deinit)(struct pair_##name *self, struct Allocator *alloc) {                          \
    if (!self) {                                                                                                       \
        return;                                                                                                        \
    }                                                                                                                  \
    PAIR_ENSURE_VOID(alloc != NULL);                                                                                   \
    (void)alloc;                                                                                                       \
    dtor_first(&self->first, alloc);                                                                                   \
    dtor_second(&self->second, alloc);                                                                                 \
    memset(self, 0, sizeof(*self));                                                                                    \
}

/**
 * @def PAIR(K, V, name, dtor_first, dtor_second)
 * @brief Helper macro to define, declare and implement all in one
 * @param K The first type pair will hold
 * @param V The second type pair will hold
 * @param name The name suffix for the pair type
 * @param dtor_first The destructor for the first type if needed
 * @param dtor_second The destructor for the second type if needed
 */
#define PAIR(K, V, name, dtor_first, dtor_second)                                                                      \
PAIR_TYPE(K, V, name)                                                                                                  \
PAIR_DECL(K, V, name)                                                                                                  \
PAIR_IMPL(K, V, name, dtor_first, dtor_second)                                                                         \

// clang-format on

#ifdef __cplusplus
}
#endif // extern "C"

#endif // PAIR_H
