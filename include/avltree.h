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
 */
#ifdef AVLTREE_USE_PREFIX
    #define AVLTREE_FN(name, func) avltree_##name##_##func
#else
    #define AVLTREE_FN(name, func) name##_##func
#endif

/**
 * @def AVLTREE_TYPE
 */
#define AVLTREE_TYPE(T, name)                                                                                          \
struct avltree_node_##name {                                                                                           \
    T *data;                                                                                                           \
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
};

/**
 * @def AVLTREE_DECL
 */
#define AVLTREE_DECL(T, name)                                                                                          \
AVLTREE_UNUSED static inline struct avltree_##name AVLTREE_FN(name, init)(                                             \
    const struct Allocator alloc,                                                                                      \
    int (*comparator_fn)(const T *a, const T *b)                                                                       \
);                                                                                                                     \

/**
 * @def AVLTREE_IMPL
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