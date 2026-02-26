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
 * @warning Call name##deinit() when done.                                                                             \
 */                                                                                                                    \
AVLTREE_UNUSED static inline struct avltree_##name AVLTREE_FN(name, init)(                                             \
    const struct Allocator alloc,                                                                                      \
    int (*comparator_fn)(T *a, T *b)                                                                                   \
);                                                                                                                     \
                                                                                                                       \
/**                                                                                                                    \
 * @brief deep_clone: Deeply clones an avltree                                                                         \
 * @param self Pointer to the avltree to copy from                                                                     \
 * @param deep_clone_fn Function that knows how to clone a single element of type T                                    \
 *                      Must have the following prototype:                                                             \
 *                      void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc);                                \
 * @return A new avltree struct that is independent of self                                                            \
 *                                                                                                                     \
 * @note The correctness of this function depends on the provided deep_clone_fn parameter                              \
 *       for value types, int or pod structs, deep_clone_fn can simply assign *dst = *src;                             \
 *       if T contains pointers or heap allocations, then deep_clone_fn must allocate/copy                             \
 *       these fields as needed, if using vltree of pointers to T, deep_clone_fn must allocate                         \
 *       a new T and, if it contains member to pointers, allocate/copy as needed                                       \
 *                                                                                                                     \
 * @warning If self is NULL or deep_clone_fn is NULL then it returns a zero-initialized struct,                        \
 *          if asserts are enabled then it crashes                                                                     \
 * @warning If an error happens during reserve capacity, then a zero-initialized struct is returned,                   \
            asserts will crash on reserve                                                                              \
 * @warning The return of this function should not be discarded, if doing so, memory may be leaked                     \
 */                                                                                                                    \
AVLTREE_NODISCARD AVLTREE_UNUSED static inline struct avltree_##name AVLTREE_FN(name, deep_clone)(                     \
    const struct avltree_##name *self,                                                                                 \
    void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc)                                                     \
);                                                                                                                     \
                                                                                                                       \
/**                                                                                                                    \
 * @brief deinit: Destroys and frees the avltree                                                                       \
 * @param self Pointer to the avltree to deinitialize                                                                  \
 *                                                                                                                     \
 * Frees the internal nodes and the data inside the nodes if a deinit_fn function is provided                          \
 * Safe to call on NULL or already deinitialized avltrees, returns early                                               \
 *                                                                                                                     \
 * @note Will call the destructor on data items if provided                                                            \
 * @note The self parameter will be left in an unusable, NULL/uninitialized state and should not be                    \
 *       used, to reuse it, one must call init again and reinitialize it                                               \
 */                                                                                                                    \
AVLTREE_UNUSED static inline void AVLTREE_FN(name, deinit)(struct avltree_##name *self);                               \
                                                                                                                       \
/**                                                                                                                    \
 * @brief clear: Cleats the tree, leaving it in an empty but reusable state                                            \
 * @param self Pointer to the avltree                                                                                  \
 */                                                                                                                    \
AVLTREE_UNUSED static inline void AVLTREE_FN(name, clear)(struct avltree_##name *self);                                \
                                                                                                                       \
/**                                                                                                                    \
 * @brief insert: Inserts a new value in the tree                                                                      \
 * @param self Pointer to the avltree                                                                                  \
 * @param value Value to be inserted                                                                                   \
 * @return AVLTREE_OK if insertion was okay, AVLTREE_ERR_NULL if avltree passed was null, or                           \
 *         AVLTREE_ERR_ALLOC if allocation failure happened                                                            \
 */                                                                                                                    \
AVLTREE_UNUSED static inline enum avltree_error AVLTREE_FN(name, insert)(struct avltree_##name *self, T value);        \
                                                                                                                       \
/**                                                                                                                    \
 * @brief remove: Removes a value from the tree                                                                        \
 * @param self Pointer to the avltree                                                                                  \
 * @param value Value to be removed                                                                                    \
 * @return                                                                                                             \
 */                                                                                                                    \
AVLTREE_UNUSED static inline enum avltree_error AVLTREE_FN(name, remove)(struct avltree_##name *self, T value);        \
                                                                                                                       \
/**                                                                                                                    \
 * @brief emplace: Inserts in-place a new value in the tree                                                            \
 * @param self Pointer to the avltree                                                                                  \
 * @param construct_fn Function that knows how to construct type T                                                     \
 *                     Must have the following prototype:                                                              \
 *                     int (*construct_fn)(T *location, void *args, struct Allocator *alloc);                          \
 *                     It must return 0 for success, < 0 or > 0 is treated as a failure.                               \
 *                     The constructor used will and must be the same as the tree allocator                            \
 * @param args Pointer to the arguments used in the constructor function                                               \
 * @return Pointer of type T to the already constructed node value inplace, or NULL on failure                         \
 *         (self is null, constructor_fn is null, duplicate, allocation failure or constructor failure)                \
 *                                                                                                                     \
 * @warning construct_fn function must return 0 for success, < 0 or > 0 for failure,                                   \
 *          and the allocator must be the same as the tree allocator                                                   \
 */                                                                                                                    \
AVLTREE_UNUSED static inline T *AVLTREE_FN(name, emplace)(                                                             \
    struct avltree_##name *self,                                                                                       \
    int (*construct_fn)(T *location, void *args, struct Allocator *alloc),                                             \
    void *args                                                                                                         \
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
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief node_allocate: Allocates a new node with the given allocator                                                 \
 * @param alloc Pointer to the Allocator interface                                                                     \
 * @return A new zeroed allocated node                                                                                 \
 * Assumes allocator is never null, as the avltree gets it by value, it's impossible to be null                        \
 */                                                                                                                    \
static inline struct avltree_node_##name *AVLTREE_FN(name, node_allocate)(struct Allocator *alloc) {                   \
    struct avltree_node_##name *new_node = AVLTREE_CAST(T)alloc->malloc(sizeof(*new_node), alloc->ctx);                \
    if (new_node) {                                                                                                    \
        memset(new_node, 0, sizeof(*new_node));                                                                        \
    }                                                                                                                  \
    return new_node;                                                                                                   \
}                                                                                                                      \
                                                                                                                       \
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
 * @brief right_rotation: Rotates a node to the right                                                                  \
 * @param node Pointer to the node                                                                                     \
 * @return The node that got into the place of the node parameter, the new root of the subtree                         \
 * Demonstration of the rotation:                                                                                      \
 *             4                            2                                                                          \
 *            / \                          / \                                                                         \
 *           2   6                        1   4                                                                        \
 *          / \                 =>       /   / \                                                                       \
 *         1   3                       -1   3   6                                                                      \
 *        /                                                                                                            \
 *      -1                                                                                                             \
 * 4 = node                     | 2 = left_of_node                                                                     \
 * 6 = node.right (unchanged)   | 4 = left_of_node.right (node)                                                        \
 * 2 = node.left (left_of_node) | 1 = leftOfNode.left                                                                  \
 * 1 = left_of_node.left        | 3 = node.left                                                                        \
 * 3 = left_of_node.right       | 6 = node.right                                                                       \
 */                                                                                                                    \
static inline struct avltree_node_##name *AVLTREE_FN(name, right_rotation)(struct avltree_node_##name *node) {         \
    struct avltree_node_##name *left_of_node = node->left; /* 2 the new root of subtree */                             \
    struct avltree_node_##name *right_of_left_node  = left_of_node->right; /* 3 right of the left of node */           \
    left_of_node->right = node; /* 4 goes to the right of 2 */                                                         \
    node->left = right_of_left_node; /* 3 goes to the left of 4 */                                                     \
    /* Update parents */                                                                                               \
    if (right_of_left_node != NULL) {                                                                                  \
        right_of_left_node->parent = node; /* 4 is now parent of 3 */                                                  \
    }                                                                                                                  \
    left_of_node->parent = node->parent; /* the parent of 4 is now the parent of 2 */                                  \
    node->parent = left_of_node; /* 2 is now parent of 4 */                                                            \
    /* Set heights */                                                                                                  \
    AVLTREE_FN(name, node_set_height)(node);                                                                           \
    AVLTREE_FN(name, node_set_height)(left_of_node);                                                                   \
    return left_of_node; /* return the new root */                                                                     \
}                                                                                                                      \
                                                                                                                       \
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief left_rotation: Rotates a node to the left                                                                    \
 * @param node Pointer to the node                                                                                     \
 * @return The node that got into the place of the node parameter, the new root of the subtree                         \
 * Demonstration of the rotation:                                                                                      \
 *             4                          6                                                                            \
 *            / \                        / \                                                                           \
 *           2   6                      4   8                                                                          \
 *              / \           =>       / \   \                                                                         \
 *             5   8                  2   5   9                                                                        \
 *                  \                                                                                                  \
 *                   9                                                                                                 \
 * 4 = node                     | 6 = rightOfNode                                                                      \
 * 6 = node.right (rightOfNode) | 4 = leftOfNode.right (node)                                                          \
 * 2 = node.left (unchanged)    | 8 = leftOfNode.left                                                                  \
 * 5 = rightOfNode.left         | 2 = node.left                                                                        \
 * 8 = rightOfNode.right        | 5 = node.right                                                                       \
 */                                                                                                                    \
static inline struct avltree_node_##name *AVLTREE_FN(name, left_rotation)(struct avltree_node_##name *node) {          \
    struct avltree_node_##name *right_of_node = node->right; /* 6 the new root of subtree */                           \
    struct avltree_node_##name *left_of_right_node  = right_of_node->left; /* 5 left of the right of node */           \
    right_of_node->left = node; /* 4 goes to the right of 6 */                                                         \
    node->right = left_of_right_node; /* 5 goes to the right of 4 */                                                   \
    /* Update parents */                                                                                               \
    if (left_of_right_node != NULL) {                                                                                  \
        left_of_right_node->parent = node; /* 4 is now parent of 5 */                                                  \
    }                                                                                                                  \
    right_of_node->parent = node->parent; /* the parent of 4 is now the parent of 6 */                                 \
    node->parent = right_of_node; /* 6 is now parent of 4 */                                                           \
    /* Set heights */                                                                                                  \
    AVLTREE_FN(name, node_set_height)(node);                                                                           \
    AVLTREE_FN(name, node_set_height)(right_of_node);                                                                  \
    return right_of_node; /* return the new root */                                                                    \
}                                                                                                                      \
                                                                                                                       \
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief rebalance: balances a node                                                                                   \
 * @param node Pointer to the node                                                                                     \
 * @return The balanced node, may be different than the original parameter                                             \
 */                                                                                                                    \
static inline struct avltree_node_##name *AVLTREE_FN(name, rebalance)(struct avltree_node_##name *node) {              \
    int balance_factor = AVLTREE_FN(name, node_get_balance_factor)(node);                                              \
    /* Left Left case */                                                                                               \
    if (balance_factor > 1 && AVLTREE_FN(name, node_get_balance_factor)(node->left) >= 0) {                            \
        /* Perform a right rotation on node */                                                                         \
        return AVLTREE_FN(name, right_rotation)(node);                                                                 \
    }                                                                                                                  \
    /* Right Right case */                                                                                             \
    if (balance_factor < -1 && AVLTREE_FN(name, node_get_balance_factor)(node->right) <= 0) {                          \
        /* Perform a left rotation on node */                                                                          \
        return AVLTREE_FN(name, left_rotation)(node);                                                                  \
    }                                                                                                                  \
    /* Left Right case */                                                                                              \
    if (balance_factor > 1 && AVLTREE_FN(name, node_get_balance_factor)(node->left) < 0) {                             \
        /* Perform a left rotation on node->left and then right rotation on node */                                    \
        node->left = AVLTREE_FN(name, left_rotation)(node->left);                                                      \
        return AVLTREE_FN(name, right_rotation)(node);                                                                 \
    }                                                                                                                  \
    /* Right Left case */                                                                                              \
    if (balance_factor < -1 && AVLTREE_FN(name, node_get_balance_factor)(node->right) > 0) {                           \
        /* Perform a right rotation on node->right and then left rotation on node */                                   \
        node->right = AVLTREE_FN(name, right_rotation)(node->right);                                                   \
        return AVLTREE_FN(name, left_rotation)(node);                                                                  \
    }                                                                                                                  \
    return node;                                                                                                       \
}                                                                                                                      \
                                                                                                                       \
/**                                                                                                                    \
 * @private                                                                                                            \
 * @brief minimum: gets the minimum of a subtree from the given node                                                   \
 * @param node Pointer to the node                                                                                     \
 * @return The minimum of the subtree or NULL if node is null                                                          \
 */                                                                                                                    \
static inline struct avltree_node_##name *AVLTREE_FN(name, minimum)(struct avltree_node_##name *node) {                \
    if (node == NULL) {                                                                                                \
        return NULL;                                                                                                   \
    }                                                                                                                  \
    while (node->left != NULL) {                                                                                       \
        node = node->left;                                                                                             \
    }                                                                                                                  \
    return node;                                                                                                       \
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
                                                                                                                       \
static inline struct avltree_##name AVLTREE_FN(name, deep_clone)(                                                      \
    const struct avltree_##name *self,                                                                                 \
    void (*deep_clone_fn)(T *dst, T *src, struct Allocator *alloc)                                                     \
) {                                                                                                                    \
    struct avltree_##name clone = { 0 };                                                                               \
    AVLTREE_ENSURE(self != NULL, clone, "deep_clone(): avltree is null.");                                             \
    AVLTREE_ENSURE(deep_clone_fn != NULL, clone, "deep_clone(): deep_clone_fn function is null.");                     \
    return clone;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
static inline void AVLTREE_FN(name, deinit)(struct avltree_##name *self) {                                             \
    if (!self) {                                                                                                       \
        return;                                                                                                        \
    }                                                                                                                  \
    struct avltree_node_##name *curr = self->root;                                                                     \
    struct avltree_node_##name *last = NULL;                                                                           \
    while (curr) {                                                                                                     \
        if (last == curr->parent) {                                                                                    \
            /* descending from parent */                                                                               \
            if (curr->left) {                                                                                          \
                last = curr;                                                                                           \
                curr = curr->left;                                                                                     \
                continue;                                                                                              \
            }                                                                                                          \
            if (curr->right) {                                                                                         \
                last = curr;                                                                                           \
                curr = curr->right;                                                                                    \
                continue;                                                                                              \
            }                                                                                                          \
            /* arrived at a leaf, destroy and climb up */                                                              \
            struct avltree_node_##name *parent = curr->parent;                                                         \
            deinit_fn(&curr->data, &self->alloc);                                                                      \
            self->alloc.free(curr, sizeof(*curr), self->alloc.ctx);                                                    \
            last = curr;                                                                                               \
            curr = parent;                                                                                             \
        } else if (last == curr->left) {                                                                               \
            /* came up from left, go right if exists, else deinitilize */                                              \
            if (curr->right) {                                                                                         \
                last = curr;                                                                                           \
                curr = curr->right;                                                                                    \
            } else {                                                                                                   \
                struct avltree_node_##name *parent = curr->parent;                                                     \
                deinit_fn(&curr->data, &self->alloc);                                                                  \
                self->alloc.free(curr, sizeof(*curr), self->alloc.ctx);                                                \
                last = curr;                                                                                           \
                curr = parent;                                                                                         \
            }                                                                                                          \
        } else {                                                                                                       \
            /* came up from right (last == curr->right) */                                                             \
            /* just finished the right subtree, we always handle left before right */                                  \
            /* coming from right means both children have been processed, just delete it */                            \
            struct avltree_node_##name *parent = curr->parent;                                                         \
            deinit_fn(&curr->data, &self->alloc);                                                                      \
            self->alloc.free(curr, sizeof(*curr), self->alloc.ctx);                                                    \
            last = curr;                                                                                               \
            curr = parent;                                                                                             \
        }                                                                                                              \
    }                                                                                                                  \
    self->root = NULL;                                                                                                 \
    self->size = 0;                                                                                                    \
    self->comparator_fn = NULL;                                                                                        \
    memset(&self->alloc, 0, sizeof(self->alloc));                                                                      \
}                                                                                                                      \
                                                                                                                       \
static inline void AVLTREE_FN(name, clear)(struct avltree_##name *self) {                                              \
    if (!self || self->size == 0) {                                                                                    \
        return;                                                                                                        \
    }                                                                                                                  \
    struct avltree_node_##name *curr = self->root;                                                                     \
    struct avltree_node_##name *last = NULL;                                                                           \
    while (curr) {                                                                                                     \
        if (last == curr->parent) {                                                                                    \
            /* descending from parent */                                                                               \
            if (curr->left) {                                                                                          \
                last = curr;                                                                                           \
                curr = curr->left;                                                                                     \
                continue;                                                                                              \
            }                                                                                                          \
            if (curr->right) {                                                                                         \
                last = curr;                                                                                           \
                curr = curr->right;                                                                                    \
                continue;                                                                                              \
            }                                                                                                          \
            /* arrived at a leaf, destroy and climb up */                                                              \
            struct avltree_node_##name *parent = curr->parent;                                                         \
            deinit_fn(&curr->data, &self->alloc);                                                                      \
            self->alloc.free(curr, sizeof(*curr), self->alloc.ctx);                                                    \
            last = curr;                                                                                               \
            curr = parent;                                                                                             \
        } else if (last == curr->left) {                                                                               \
            /* came up from left, go right if exists, else deinitilize */                                              \
            if (curr->right) {                                                                                         \
                last = curr;                                                                                           \
                curr = curr->right;                                                                                    \
            } else {                                                                                                   \
                struct avltree_node_##name *parent = curr->parent;                                                     \
                deinit_fn(&curr->data, &self->alloc);                                                                  \
                self->alloc.free(curr, sizeof(*curr), self->alloc.ctx);                                                \
                last = curr;                                                                                           \
                curr = parent;                                                                                         \
            }                                                                                                          \
        } else {                                                                                                       \
            /* came up from right (last == curr->right) */                                                             \
            /* just finished the right subtree, we always handle left before right */                                  \
            /* coming from right means both children have been processed, just delete it */                            \
            struct avltree_node_##name *parent = curr->parent;                                                         \
            deinit_fn(&curr->data, &self->alloc);                                                                      \
            self->alloc.free(curr, sizeof(*curr), self->alloc.ctx);                                                    \
            last = curr;                                                                                               \
            curr = parent;                                                                                             \
        }                                                                                                              \
    }                                                                                                                  \
    self->root = NULL;                                                                                                 \
    self->size = 0;                                                                                                    \
}                                                                                                                      \
                                                                                                                       \
static inline enum avltree_error AVLTREE_FN(name, insert)(struct avltree_##name *self, T value) {                      \
    AVLTREE_ENSURE(self != NULL, AVLTREE_ERR_NULL, "insert(): self is null.");                                         \
    /* Search valid position */                                                                                        \
    struct avltree_node_##name *current = self->root;                                                                  \
    struct avltree_node_##name *insert_pos = NULL;                                                                     \
    while (current != NULL) {                                                                                          \
        int cmp = self->comparator_fn(&value, &current->data);                                                         \
        insert_pos = current;                                                                                          \
        if (cmp < 0) {                                                                                                 \
            current = current->left;                                                                                   \
        } else if (cmp > 0) {                                                                                          \
            current = current->right;                                                                                  \
        } else {                                                                                                       \
            return AVLTREE_ERR_DUPLICATE;                                                                              \
        }                                                                                                              \
    }                                                                                                                  \
    /* Allocate new node */                                                                                            \
    struct avltree_node_##name *new_node = AVLTREE_FN(name, node_allocate)(&self->alloc);                              \
    AVLTREE_ENSURE(new_node != NULL, AVLTREE_ERR_ALLOC, "insert(): allocation of new node failed.");                   \
    new_node->data = value;                                                                                            \
    /* Insert into position */                                                                                         \
    if (insert_pos == NULL) {                                                                                          \
        self->root = new_node;                                                                                         \
    } else {                                                                                                           \
        if (self->comparator_fn(&value, &insert_pos->data) < 0) {                                                      \
            insert_pos->left = new_node;                                                                               \
        } else {                                                                                                       \
            insert_pos->right = new_node;                                                                              \
        }                                                                                                              \
        new_node->parent = insert_pos;                                                                                 \
    }                                                                                                                  \
    /* Update height and rebalance, going up through parent pointer */                                                 \
    struct avltree_node_##name *current_insert_pos = new_node;                                                         \
    while (current_insert_pos != NULL) {                                                                               \
        struct avltree_node_##name *old_parent = current_insert_pos->parent;                                           \
        struct avltree_node_##name *new_subroot = AVLTREE_FN(name, rebalance)(current_insert_pos);                     \
        /* If subtree root changed, update the parent pointer or the tree root */                                      \
        if (new_subroot != current_insert_pos) {                                                                       \
            if (old_parent == NULL) {                                                                                  \
                /* current_insert_pos was the tree root, new root of the tree is then new_subroot */                   \
                self->root = new_subroot;                                                                              \
            } else if (old_parent->left == current_insert_pos) {                                                       \
                /* the old node was the left child of its parent, set old_parent->left to the new root */              \
                old_parent->left = new_subroot;                                                                        \
            } else {                                                                                                   \
                /* the old node was the right child of its parent, set old_parent->right to the new root */            \
                old_parent->right = new_subroot;                                                                       \
            }                                                                                                          \
        }                                                                                                              \
        /* move up */                                                                                                  \
        current_insert_pos = old_parent;                                                                               \
    }                                                                                                                  \
    self->size += 1;                                                                                                   \
    return AVLTREE_OK;                                                                                                 \
}                                                                                                                      \
                                                                                                                       \
static inline enum avltree_error AVLTREE_FN(name, remove)(struct avltree_##name *self, T value) {                      \
    AVLTREE_ENSURE(self != NULL, AVLTREE_ERR_NULL, "remove(): self is null.");                                         \
    /* Search value to remove position */                                                                              \
    struct avltree_node_##name *del_pos = self->root;                                                                  \
    while (del_pos != NULL) {                                                                                          \
        int cmp = self->comparator_fn(&value, &del_pos->data);                                                         \
        if (cmp < 0) {                                                                                                 \
            del_pos = del_pos->left;                                                                                   \
        } else if (cmp > 0) {                                                                                          \
            del_pos = del_pos->right;                                                                                  \
        } else {                                                                                                       \
            break;                                                                                                     \
        }                                                                                                              \
    }                                                                                                                  \
    /* Not found */                                                                                                    \
    if (del_pos == NULL) {                                                                                             \
        return AVLTREE_OK;                                                                                             \
    }                                                                                                                  \
    struct avltree_node_##name *start = NULL; /* ancestor to begin rebalancing from */                                 \
    if (del_pos->left == NULL || del_pos->right == NULL) { /* node with only 1 or no child */                          \
        struct avltree_node_##name *child = (del_pos->left != NULL) ? del_pos->left : del_pos->right;                  \
        /* relink parent or root to child */                                                                           \
        if (del_pos->parent == NULL) {                                                                                 \
            /* deletion position is root */                                                                            \
            self->root = child;                                                                                        \
        } else if (del_pos->parent->left == del_pos) {                                                                 \
            /* if del_pos->parent->left is equal to deletion position, move child to it */                             \
            del_pos->parent->left = child;                                                                             \
        } else {                                                                                                       \
            del_pos->parent->right = child;                                                                            \
        }                                                                                                              \
        if (child != NULL) {                                                                                           \
            child->parent = del_pos->parent;                                                                           \
        }                                                                                                              \
        start = del_pos->parent;                                                                                       \
        deinit_fn(&del_pos->data, &self->alloc);                                                                       \
        self->alloc.free(del_pos, sizeof(*del_pos), self->alloc.ctx);                                                  \
    } else { /* two children node, remove successor */                                                                 \
        struct avltree_node_##name *successor = AVLTREE_FN(name, minimum)(del_pos->right);                             \
        /* just swap the data, do not need to free the del_pos node itself */                                          \
        T tmp = del_pos->data;                                                                                         \
        del_pos->data = successor->data;                                                                               \
        successor->data = tmp;                                                                                         \
        /* successor has no left child, but may have a right child */                                                  \
        struct avltree_node_##name *child = successor->right;                                                          \
        /* relink successor parents to child */                                                                        \
        if (successor->parent->left == successor) {                                                                    \
            successor->parent->left = child;                                                                           \
        } else {                                                                                                       \
            successor->parent->right = child;                                                                          \
        }                                                                                                              \
        start = successor->parent;                                                                                     \
        deinit_fn(&successor->data, &self->alloc);                                                                     \
        self->alloc.free(successor, sizeof(*successor), self->alloc.ctx);                                              \
    }                                                                                                                  \
    /* rebalance, going up through the first ancestor */                                                               \
    struct avltree_node_##name *node = start;                                                                          \
    while (node != NULL) {                                                                                             \
        struct avltree_node_##name *old_parent = node->parent;                                                         \
        struct avltree_node_##name *new_subroot = AVLTREE_FN(name, rebalance)(node);                                   \
        /* If subtree root changed, update the parent pointer or the tree root */                                      \
        if (new_subroot != node) {                                                                                     \
            if (old_parent == NULL) {                                                                                  \
                new_subroot->parent = old_parent; /* ensure parent pointer on the new root */                          \
                /* node was the tree root, new root of the tree is then new_subroot */                                 \
                self->root = new_subroot;                                                                              \
            } else if (old_parent->left == node) {                                                                     \
                /* the old node was the left child of its parent, set old_parent->left to the new root */              \
                old_parent->left = new_subroot;                                                                        \
            } else {                                                                                                   \
                /* the old node was the right child of its parent, set old_parent->right to the new root */            \
                old_parent->right = new_subroot;                                                                       \
            }                                                                                                          \
        }                                                                                                              \
        /* move up */                                                                                                  \
        node = old_parent;                                                                                             \
    }                                                                                                                  \
    self->size -= 1;                                                                                                   \
    return AVLTREE_OK;                                                                                                 \
}                                                                                                                      \
                                                                                                                       \
static inline T *AVLTREE_FN(name, emplace)(                                                                            \
    struct avltree_##name *self,                                                                                       \
    int (*construct_fn)(T *location, void *args, struct Allocator *alloc),                                             \
    void *args                                                                                                         \
) {                                                                                                                    \
    AVLTREE_ENSURE_PTR(self != NULL, "emplace(): self is null.");                                                      \
    AVLTREE_ENSURE_PTR(construct_fn != NULL, "emplace(): constructor function is null.");                              \
    /* Allocate new node */                                                                                            \
    struct avltree_node_##name *new_node = AVLTREE_FN(name, node_allocate)(&self->alloc);                              \
    AVLTREE_ENSURE_PTR(new_node != NULL, "emplace(): allocation of new node failed.");                                 \
    /* Construct new node inplace */                                                                                   \
    if (construct_fn(&new_node->data, args, &self->alloc) != 0) {                                                      \
        return NULL;                                                                                                   \
    }                                                                                                                  \
    /* Search valid position using the constructed value */                                                            \
    struct avltree_node_##name *current = self->root;                                                                  \
    struct avltree_node_##name *insert_pos = NULL;                                                                     \
    while (current != NULL) {                                                                                          \
        int cmp = self->comparator_fn(&new_node->data, &current->data);                                                \
        insert_pos = current;                                                                                          \
        if (cmp < 0) {                                                                                                 \
            current = current->left;                                                                                   \
        } else if (cmp > 0) {                                                                                          \
            current = current->right;                                                                                  \
        } else {                                                                                                       \
            deinit_fn(&new_node->data, &self->alloc);                                                                  \
            self->alloc.free(new_node, sizeof(*new_node), self->alloc.ctx);                                            \
            return NULL;                                                                                               \
        }                                                                                                              \
    }                                                                                                                  \
    /* Insert node into position */                                                                                    \
    if (insert_pos == NULL) {                                                                                          \
        self->root = new_node;                                                                                         \
    } else {                                                                                                           \
        if (self->comparator_fn(&new_node->data, &insert_pos->data) < 0) {                                             \
            insert_pos->left = new_node;                                                                               \
        } else {                                                                                                       \
            insert_pos->right = new_node;                                                                              \
        }                                                                                                              \
        new_node->parent = insert_pos;                                                                                 \
    }                                                                                                                  \
    /* Update height and rebalance, going up through parent pointer */                                                 \
    struct avltree_node_##name *current_insert_pos = new_node;                                                         \
    while (current_insert_pos != NULL) {                                                                               \
        struct avltree_node_##name *old_parent = current_insert_pos->parent;                                           \
        struct avltree_node_##name *new_subroot = AVLTREE_FN(name, rebalance)(current_insert_pos);                     \
        /* If subtree root changed, update the parent pointer or the tree root */                                      \
        if (new_subroot != current_insert_pos) {                                                                       \
            if (old_parent == NULL) {                                                                                  \
                /* current_insert_pos was the tree root, new root of the tree is then new_subroot */                   \
                self->root = new_subroot;                                                                              \
            } else if (old_parent->left == current_insert_pos) {                                                       \
                /* the old node was the left child of its parent, set old_parent->left to the new root */              \
                old_parent->left = new_subroot;                                                                        \
            } else {                                                                                                   \
                /* the old node was the right child of its parent, set old_parent->right to the new root */            \
                old_parent->right = new_subroot;                                                                       \
            }                                                                                                          \
        }                                                                                                              \
        /* move up */                                                                                                  \
        current_insert_pos = old_parent;                                                                               \
    }                                                                                                                  \
    self->size += 1;                                                                                                   \
    return &new_node->data;                                                                                            \
}                                                                                                                      \

// clang-format on

#ifdef __cplusplus
}
#endif // extern "C"

#endif // AVLTREE_H