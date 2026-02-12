/**
 * @file avltree.h
 * @author Jean Rehr <jeanrehr@gmail.com>
 * @brief Generic and typesafe avltree implementation using macros
 *
 * @details
 */
#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdbool.h> // For bool, true, false

#include "allocator.h" // For a custom Allocator interface

#ifdef __cplusplus
extern "C" {
#endif // extern "C"

/**
 * @def avltree_noop_deinit
 * @brief Defines a no-op destructor macro for usage in scalar types or types
 *        that does not need a destructor
 */
#ifndef avltree_noop_deinit
    #define avltree_noop_deinit(ptr, alloc) ((void)0)
#endif // avltree_noop_deinit

/**
 * @def AVLTREE_UNUSED
 * @brief Defines a macro to supress the warning for unused function because of static inline
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define AVLTREE_UNUSED [[maybe_unused]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define AVLTREE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
    #define AVLTREE_UNUSED __attribute__((unused))
#else
    #define AVLTREE_UNUSED
#endif // AVLTREE_UNUSED definition

// clang-format off

/**
 * @def AVLTREE_USE_PREFIX
 * @brief Defines at compile-time if the functions will use the avltree_* prefix
 * This macro constructs function names for the avltree library. The naming convention depends on
 * whether @c AVLTREE_USE_PREFIX is defined.
 * Generates functions with the pattern avltree_##name##_function() instead of name##_function()
 * Must be defined before including the avltree.h header, it will be per TU/file, so once defined
 * can't be undef in the same TU for different avltree types and names.
 * Inspiration taken from the Tsoding's nob.h lib, and some other libs like jemalloc which also does
 * something similar.
 *
 * The default is without prefixes, as the "name" parameter already acts like a user defined prefix:
 * @code
 * AVLTREE(int, ints)
 * struct avltree_ints intstree = ints_init(...);
 * @endcode
 *
 * Usage:
 * @code
 * // In a header that will include avltree.h and will define some avltree types:
 * #define AVLTREE_USE_PREFIX
 * #include "avltree.h"
 * AVLTREE_TYPE(int, ints) // Creates a struct: struct avltree_ints;
 * AVLTREE_DECL(int, ints) // Declares functions named avltree_ints_init(...)
 * // In a .c file:
 * AVLTREE_IMPL(int, ints)
 * @endcode
 *
 * @warning The @c AVLTREE_FN macro is for intenal use only, I can't see any usefulness for user code
 */
#ifdef AVLTREE_USE_PREFIX
    #define AVLTREE_FN(name, func) avltree_##name##_##func
#else
    #define AVLTREE_FN(name, func) name##_##func
#endif

/**
 * @def AVLTREE_TYPE(T, name)
 * @brief Defines an avltree structure for a specific type T
 * @param T The type avltree will hold
 * @param name The name suffix for the avltree type
 *
 * This macro defines two structures:
 * - A struct named "avltree_node_##name" with the following fields:
 * - - "data": Value of type T that each node in the tree holds
 * - - "height": Current height in the tree
 * - - "left": Pointer to the left node
 * - - "right": Pointer to the right node
 * - - "parent": Pointer to the parent node
 *
 * - A struct named avltree_##name with the following fields:
 * - - "alloc": Allocator struct used to allocate nodes
 * - - "root": Pointer to the root node
 * - - "comparator_fn": Function pointer that knows how to compare two types T for balancing
 * - - "size": Size of the tree
 * @code
 * // Example: Define an avltree for integers
 * AVLTREE_TYPE(int, ints)
 * // Creates a struct named struct avltree_ints
 * @endcode
 */
#define AVLTREE_TYPE(T, name)                                                                                          \
struct avltree_node_##name {                                                                                           \
    T data;                                                                                                            \
    size_t height;                                                                                                     \
    struct avltree_node_##name *left;                                                                                  \
    struct avltree_node_##name *right;                                                                                 \
    struct avltree_node_##name *parent;                                                                                \
};                                                                                                                     \
                                                                                                                       \
struct avltree_##name {                                                                                                \
    struct Allocator alloc;                                                                                            \
    struct avltree_node_##name *root;                                                                                  \
    int (*comparator_fn)(const T *a, const T *b);                                                                      \
    size_t size;                                                                                                       \
};

/**
 * @def AVLTREE_DECL(T, name)
 * @brief Declares all functions for an avltree type
 * @param T The type avltree will hold
 * @param name The name suffix for the avltree type
 *
 * @details
 * 
 * @note All functions declared here operates on the avltree_##name struct
 * @note User code may create and operate on the node struct, but it is not part of the public api
 */
#define AVLTREE_DECL(T, name)                                                                                          \
/**                                                                                                                    \
 * @brief init: Creates a new avltree                                                                                  \
 * @param alloc Custom allocator instance, if null, default alloc will be used                                         \
 * @param comparator_fn Custom compare function that knows how to compare two types T                                  \
 *                      Must have the following prototype:                                                             \
 *                      int (*comparator_fn)(const T *a, const T *b);                                                  \
 * @return A zero initialized avltree structure                                                                        \
 *                                                                                                                     \
 * @note It does not allocate                                                                                          \
 *                                                                                                                     \
 * @warning The comparator function must not be null, otherwise this data structure will not work.                     \
 *          An assert happens regardless.                                                                              \
 * @warning Call name##deinit() when done.                                                                             \
 */                                                                                                                    \
AVLTREE_UNUSED static inline struct avltree_##name AVLTREE_FN(name, init)(                                             \
    const struct Allocator alloc,                                                                                      \
    int (*comparator_fn)(const T *a, const T *b)                                                                       \
);                                                                                                                     \

/**
 * @def AVLTREE_IMPL(T, name, deinit_fn)
 * @brief Implements all functions for an avltree type
 * @param T The type avltree will hold
 * @param name The name suffix for the avltree type
 * @param deinit_fn The function that knows how to free type T and its members (may be a macro or
 *                  a normal function), recommended to inline the function
 *
 * Implements the functions of the AVLTREE_DECL macro
 *
 * @note This macro should be used in a .c file, not in a header
 * @note All functions declared here operates on the avltree_##name struct
 * @note User code may create and operate on the node struct, but it is not part of the public api
 *
 * @warning If the type T doesn't need to have a destructor, or one doesn't want to pass it
 *          and manually free, then a noop must be passed, like the already provided
 *          avltree_noop_deinit nacro, or (void), or a macro/function that does nothing.
 */
#define AVLTREE_IMPL(T, name, deinit_fn)                                                                               \
static inline struct avltree_##name AVLTREE_FN(name, init)(                                                            \
    const struct Allocator alloc,                                                                                      \
    int (*comparator_fn)(const T *a, const T *b)                                                                       \
) {                                                                                                                    \
    struct avltree_##name avltree = { 0 };                                                                             \
    avltree.alloc = alloc;                                                                                             \
    avltree.root = NULL;                                                                                               \
    avltree.comparator_fn = comparator_fn;                                                                             \
    return avltree;                                                                                                    \
}                                                                                                                      \

// clang-format on

#ifdef __cplusplus
}
#endif // extern "C"

#endif // AVLTREE_H