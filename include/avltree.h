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
#include <string.h>  // For memset()

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

/**
 * @def AVLTREE_NODISCARD
 * @brief Defines a macro to warn of discarded unused results when they matter
 *        (possible leak of memory is involved)
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define AVLTREE_NODISCARD [[nodiscard]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define AVLTREE_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
    #define AVLTREE_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
    #if defined(_SAL_VERSION_SOURCE) && _SAL_VERSION_SOURCE >= 2
        #ifndef _Check_return_
            #include <sal.h>
        #endif // _Check_return_ from sal.h
        #define AVLTREE_NODISCARD _Check_return_
    #else
        #define AVLTREE_NODISCARD
    #endif // _SAL_VERSION_SOURCE && _SAL_VERSION_SOURCE >= 2
#else
    #define AVLTREE_NODISCARD
#endif // AVLTREE_NODISCARD definition

/**
 * @def AVLTREE_UNLIKELY(x)
 * @brief Branch prediction hint, used in the AVLTREE_ENSURE macro, as it usually
 *        just always passes, C23 [[likely]] cannot be applied in this case
 */
#if defined(__GNUC__) || defined(__clang__)
    #define AVLTREE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define AVLTREE_UNLIKELY(x) (x)
#endif // AVLTREE_UNLIKELY definition

/**
 * @def AVLTREE_USE_ASSERT
 * @brief Defines if the avltree will use asserts or return error codes
 * @details If AVLTREE_USE_ASSERT is 1, then the lib will assert and fail early, otherwise,
 *          defensive programming and returning error codes will be used
 */
#ifndef AVLTREE_USE_ASSERT
    #define AVLTREE_USE_ASSERT 0
#endif // AVLTREE_USE_ASSERT

#if AVLTREE_USE_ASSERT
    #include <assert.h> // For assert()
    #include <stdlib.h> // For abort()
    #define AVLTREE_ENSURE(cond, ret, msg)                                                                             \
        do {                                                                                                           \
            if (AVLTREE_UNLIKELY(!(cond))) {                                                                           \
                assert(0 && (msg));                                                                                    \
                abort();                                                                                               \
            }                                                                                                          \
        } while (0)

    #define AVLTREE_ENSURE_PTR(cond, msg)                                                                              \
        do {                                                                                                           \
            if (AVLTREE_UNLIKELY(!(cond))) {                                                                           \
                assert(0 && (msg));                                                                                    \
                abort();                                                                                               \
            }                                                                                                          \
        } while (0)
#else
    #define AVLTREE_ENSURE(cond, ret, msg)                                                                             \
        do {                                                                                                           \
            if (AVLTREE_UNLIKELY(!(cond))) {                                                                           \
                return (ret);                                                                                          \
            }                                                                                                          \
        } while (0)

    #define AVLTREE_ENSURE_PTR(cond, msg)                                                                              \
        do {                                                                                                           \
            if (AVLTREE_UNLIKELY(!(cond))) {                                                                           \
                return NULL;                                                                                           \
            }                                                                                                          \
        } while (0)
#endif // AVLTREE_USE_ASSERT if directive

/**
 * @def AVLTREE_CAST
 * @brief Defines a macro that either casts type T to T* or does nothing
 * @details
 * If __cplusplus is defined (compiled with a c++ compiler) then it will cast the results of malloc
 * and realloc, if compiled with a C compiler, then it does nothing
 */
#ifdef __cplusplus
    #define AVLTREE_CAST(T) (T *)
#else
    #define AVLTREE_CAST(T)
#endif // AVLTREE_CAST(T)

/**
 * @enum avltree_error
 * @brief Error codes for the avltree
 */
enum avltree_error {
    AVLTREE_OK = 0,             ///< No error
    AVLTREE_ERR_NULL = -1,      ///< Null pointer
    AVLTREE_ERR_DUPLICATE = -2, ///< An attempt to insert a duplicate was made
    AVLTREE_ERR_ALLOC = -3,     ///< Allocation failure
};

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
    int (*comparator_fn)(T *a, T *b);                                                                                  \
    size_t size;                                                                                                       \
};

/**
 * @def AVLTREE_DECL(T, name)
 * @brief Declares all functions for an avltree type
 * @param T The type avltree will hold
 * @param name The name suffix for the avltree type
 *
 * @details
 * The following functions are declared:
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
 *                      int (*comparator_fn)(T *a, T *b);                                                              \
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
    int (*comparator_fn)(T *a, T *b)                                                                                   \
);                                                                                                                     \
                                                                                                                       \
/**                                                                                                                    \
 * @brief insert: Inserts a new value in the tree                                                                      \
 * @param self Pointer to the avltree                                                                                  \
 * @param value Value to be inserted                                                                                   \
 * @return AVLTREE_OK if insertion was okay, AVLTREE_ERR_NULL if avltree passed was null, or                           \
 *         AVLTREE_ERR_ALLOC if allocation failure happened                                                            \
 */                                                                                                                    \
AVLTREE_UNUSED static inline enum avltree_error AVLTREE_FN(name, insert)(struct avltree_##name *self, T value);        \

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
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief node_get_height: Gets the height of a node                                                                   \
 * @param node Pointer to the node                                                                                     \
 * @return The height of the node, or zero if node is null                                                             \
 * This private function is needed to ensure it returns 0 in case of null and doesn't access height directly           \
 */                                                                                                                    \
static inline int AVLTREE_FN(name, node_get_height)(struct avltree_node_##name *node) {                                \
    if (node == NULL) {                                                                                                \
        return 0;                                                                                                      \
    }                                                                                                                  \
    return node->height;                                                                                               \
}                                                                                                                      \
                                                                                                                       \
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief node_set_height: Sets the height of a node                                                                   \
 * @param node Pointer to the node                                                                                     \
 * This private function is needed to ensure it returns in case of null and doesn't access height directly             \
 */                                                                                                                    \
static inline void AVLTREE_FN(name, node_set_height)(struct avltree_node_##name *node) {                               \
    if (node == NULL) {                                                                                                \
        return;                                                                                                        \
    }                                                                                                                  \
    /* ternary operator to get the max height of left or right node */                                                 \
    size_t max_height =                                                                                                \
        (AVLTREE_FN(name, node_get_height)(node->left) > AVLTREE_FN(name, node_get_height)(node->right)) ?             \
            AVLTREE_FN(name, node_get_height)(node->left) :                                                            \
            AVLTREE_FN(name, node_get_height)(node->right);                                                            \
    node->height = max_height + 1;                                                                                     \
}                                                                                                                      \
                                                                                                                       \
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief node_get_balance_factor: Gets the balance factor of a node                                                   \
 * @param node Pointer to the node                                                                                     \
 * @return The balance factor                                                                                          \
 * This private function is needed to ensure it returns 0 in case of null and doesn't access node's fields directly    \
 */                                                                                                                    \
static inline int AVLTREE_FN(name, node_get_balance_factor)(struct avltree_node_##name *node) {                        \
    if (node == NULL) {                                                                                                \
        return 0;                                                                                                      \
    }                                                                                                                  \
    return AVLTREE_FN(name, node_get_height)(node->left) - AVLTREE_FN(name, node_get_height)(node->right);             \
}                                                                                                                      \
                                                                                                                       \
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief node_allocate: Allocates a new node with the given allocator                                                 \
 * @param alloc Pointer to the Allocator interface                                                                     \
 * @return A new zeroed allocated node                                                                                 \
 * Assumes allocator is never null, as the avltree gets it by value, it's impossible to be null                        \
 */                                                                                                                    \
static inline struct avltree_node_##name *AVLTREE_FN(name, node_allocate)(struct Allocator *alloc) {                   \
    struct avltree_node_##name *new_node = alloc->malloc(sizeof(struct avltree_node_##name), alloc->ctx);              \
    memset(new_node, 0, sizeof(*new_node));                                                                            \
    return new_node;                                                                                                   \
}                                                                                                                      \
                                                                                                                       \
static inline struct avltree_##name AVLTREE_FN(name, init)(                                                            \
    const struct Allocator alloc,                                                                                      \
    int (*comparator_fn)(T *a, T *b)                                                                                   \
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