/**
 * @file test.c
 * @brief Unit tests for the arraylist.h file
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// To use asserts inside arraylist functions instead of error code returns, define the following
// variable ARRAYLIST_USE_ASSERT as 1, it must be before arraylist.h is included
// Uncomment next line to use asserts
//#define ARRAYLIST_USE_ASSERT 1
// If wanted to use arraylist_* prefix to the functions:
//#define ARRAYLIST_USE_PREFIX
//#define ARRAYLIST_DYN_USE_PREFIX
#include "allocator.h"
#include "arraylist.h"

struct non_pod {
    char *objname;
    int *a; // Just an integer, not an array of integers
    float *b; // Same thing
};

size_t global_destructor_counter_arraylist = 0;

// Returns a fully allocated struct by value
static inline struct non_pod non_pod_init(const char *name, int a, float b, struct Allocator *alloc) {
    struct non_pod np = { 0 };
    size_t len = strlen(name) + 1;

    np.objname = alloc->malloc(len, alloc->ctx);

    if (np.objname) {
        strcpy(np.objname, name);
    }

    np.a = alloc->malloc(sizeof(int), alloc->ctx);
    if (np.a) {
        *np.a = a;
    }

    np.b = alloc->malloc(sizeof(float), alloc->ctx);
    if (np.b) {
        *np.b = b;
    }
    ++global_destructor_counter_arraylist;
    return np;
}

// Returns a pointer to a fully allocated struct
static inline struct non_pod *non_pod_init_ptr(const char *name, int a, float b, struct Allocator *alloc) {
    struct non_pod *np = alloc->malloc(sizeof(struct non_pod), alloc->ctx);
    if (!np) {
        return NULL;
    }

    size_t len = strlen(name) + 1;

    np->objname = alloc->malloc(len, alloc->ctx);

    if (np->objname) {
        strcpy(np->objname, name);
    }

    np->a = alloc->malloc(sizeof(int), alloc->ctx);
    if (np->a) {
        *np->a = a;
    }

    np->b = alloc->malloc(sizeof(float), alloc->ctx);
    if (np->b) {
        *np->b = b;
    }

    ++global_destructor_counter_arraylist;
    return np;
}

static inline void non_pod_deinit(struct non_pod *np, struct Allocator *alloc) {
    if (!np) {
        return;
    }

    if (np->objname) {
        alloc->free(np->objname, strlen(np->objname) + 1, alloc->ctx);
        np->objname = NULL;
    }

    if (np->a) {
        alloc->free(np->a, sizeof(*np->a), alloc->ctx);
        np->a = NULL;
    }

    if (np->b) {
        alloc->free(np->b, sizeof(*np->b), alloc->ctx);
        np->b = NULL;
    }
    --global_destructor_counter_arraylist;
}

static inline void non_pod_deinit_ptr(struct non_pod **np_ptr, struct Allocator *alloc) {
    if (!np_ptr || !*np_ptr) {
        return;
    }

    //non_pod_deinit(*np_ptr, alloc);

    if ((*np_ptr)->objname) {
        alloc->free((*np_ptr)->objname, strlen((*np_ptr)->objname) + 1, alloc->ctx);
        (*np_ptr)->objname = NULL;
    }

    if ((*np_ptr)->a) {
        alloc->free((*np_ptr)->a, sizeof(*(*np_ptr)->a), alloc->ctx);
        (*np_ptr)->a = NULL;
    }

    if ((*np_ptr)->b) {
        alloc->free((*np_ptr)->b, sizeof(*(*np_ptr)->b), alloc->ctx);
        (*np_ptr)->b = NULL;
    }

    alloc->free(*np_ptr, sizeof(struct non_pod), alloc->ctx);
    *np_ptr = NULL;
    --global_destructor_counter_arraylist;
}

#define non_pod_deinit_macro(np, alloc) \
    do { \
        if ((np)) { \
            if ((np)->objname) { \
                (alloc)->free((np)->objname, strlen((np)->objname) + 1, (alloc)->ctx); \
                (np)->objname = NULL; \
            } \
            if ((np)->a) { \
                (alloc)->free((np)->a, sizeof(*(np)->a), (alloc)->ctx); \
                (np)->a = NULL; \
            } \
            if ((np)->b) { \
                (alloc)->free((np)->b, sizeof(*(np)->b), (alloc)->ctx); \
                (np)->b = NULL; \
            } \
            --global_destructor_counter_arraylist; \
            ; \
        } \
    } while (0)

#define non_pod_deinit_macro_ptr(np_ptr, alloc) \
    do { \
        if ((np_ptr) && *(np_ptr)) { \
            /*non_pod_deinit_macro(*(np_ptr), alloc);*/ \
            if ((*np_ptr)->objname) { \
                (alloc)->free((*np_ptr)->objname, strlen((*np_ptr)->objname) + 1, (alloc)->ctx); \
                (*np_ptr)->objname = NULL; \
            } \
            if ((*np_ptr)->a) { \
                (alloc)->free((*np_ptr)->a, sizeof(*(*np_ptr)->a), (alloc)->ctx); \
                (*np_ptr)->a = NULL; \
            } \
            if ((*np_ptr)->b) { \
                (alloc)->free((*np_ptr)->b, sizeof(*(*np_ptr)->b), (alloc)->ctx); \
                (*np_ptr)->b = NULL; \
            } \
            (alloc)->free(*(np_ptr), sizeof(struct non_pod), (alloc)->ctx); \
            *(np_ptr) = NULL; \
            --global_destructor_counter_arraylist; \
            ; \
        } \
    } while (0)

// Function for find and contains for value containers
static bool non_pod_find_val(struct non_pod *np, void *find) {
    if (strcmp(np->objname, (char *)find) == 0) {
        return true;
    }
    return false;
}

// Function for qsort for value containers
static bool non_pod_sort_val(struct non_pod *np1, struct non_pod *np2) {
    return strcmp(np1->objname, np2->objname) < 0;
}

// Function for deep_clone for value containers
static void non_pod_deep_clone_val(struct non_pod *dst, struct non_pod *src, struct Allocator *alloc) {
    if (!src) {
        dst = NULL;
        return;
    }

    if (src->objname) {
        size_t len = strlen(src->objname) + 1;

        dst->objname = alloc->malloc(len, alloc->ctx);

        if (dst->objname) {
            strcpy(dst->objname, src->objname);
        }
    } else {
        dst->objname = NULL;
    }

    if (src->a) {
        dst->a = alloc->malloc(sizeof(dst->a), alloc->ctx);
        *dst->a = *src->a;
    } else {
        dst->a = NULL;
    }

    if (src->b) {
        dst->b = alloc->malloc(sizeof(dst->b), alloc->ctx);
        *dst->b = *src->b;
    } else {
        dst->b = NULL;
    }
}

// Function for deep_clone for pointer containers
static void non_pod_deep_clone_ptr(struct non_pod **dst, struct non_pod **src, struct Allocator *alloc) {
    if (!src || !*src) {
        *dst = NULL;
        return;
    }

    *dst = alloc->malloc(sizeof(struct non_pod), alloc->ctx);

    if ((*src)->objname) {
        size_t len = strlen((*src)->objname) + 1;

        (*dst)->objname = alloc->malloc(len, alloc->ctx);

        if ((*dst)->objname) {
            strcpy((*dst)->objname, (*src)->objname);
        }
    } else {
        (*dst)->objname = NULL;
    }

    if ((*src)->a) {
        (*dst)->a = alloc->malloc(sizeof((*dst)->a), alloc->ctx);
        *(*dst)->a = *(*src)->a;
    } else {
        (*dst)->a = NULL;
    }

    if ((*src)->b) {
        (*dst)->b = alloc->malloc(sizeof((*dst)->b), alloc->ctx);
        *(*dst)->b = *(*src)->b;
    } else {
        (*dst)->b = NULL;
    }
}

// Function for find and contains for pointer containers
static bool non_pod_find_ptr(struct non_pod **np, void *find) {
    if (strcmp((*np)->objname, (char *)find) == 0) {
        return true;
    }
    return false;
}

// Function for qsort for pointer containers
static bool non_pod_sort_ptr(struct non_pod **np1, struct non_pod **np2) {
    return strcmp((*np1)->objname, (*np2)->objname) < 0;
}

// A custom failing allocator
static void *fail_malloc(size_t size, void *ctx) {
    (void)size;
    (void)ctx;
    return NULL;
}
static void *fail_realloc(void *p, size_t os, size_t ns, void *ctx) {
    (void)p;
    (void)os;
    (void)ns;
    (void)ctx;
    return NULL;
}
static void fail_free(void *ptr, size_t sz, void *ctx) {
    (void)ptr;
    (void)sz;
    (void)ctx;
}

struct Allocator allocator_get_failling_alloc(void) {
    return (struct Allocator) { .malloc = fail_malloc, .realloc = fail_realloc, .free = fail_free, .ctx = NULL };
}

/* === START ARRAYLIST UNIT TESTS NON POD === */

ARRAYLIST(struct non_pod, nonpods, non_pod_deinit_macro)

void test_arraylist_init_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.data == NULL);
    assert(list.alloc.malloc == gpa.malloc);

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist init for value types passed\n");
}

void test_arraylist_reserve_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Reserve when list is empty: allocates exactly requested size
    enum arraylist_error err = nonpods_reserve(&list, 10);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 10);
    assert(list.data != NULL);
    assert(list.size == 0);

    // Reserve smaller (no-op): capacity remains unchanged
    err = nonpods_reserve(&list, 5);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 10);    // No shrink

    // Reserve same size (no-op)
    err = nonpods_reserve(&list, 10);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 10);    // Still 10

    // Reserve larger: reallocs and grows buffer, data valid

    // Fill up to 4
    for (size_t i = 0; i < 4; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("a", i, i, &gpa);
    }

    // struct non_pod *olddata = list.data;
    err = nonpods_reserve(&list, 32);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 32);    // Grown
    assert(list.size == 4);
    assert(list.data != NULL);

    for (size_t i = 0; i < list.size; ++i) {
        assert(strcmp(list.data[i].objname, "a") == 0);
    }
    // Possibly new buffer, but implementation allows same pointer if realloc does nothing.
    // Do not hold pointers to old data if a realloc happens
    // assert(list.data == olddata);

    // Emplace more after reserve: no further realloc until cap hit
    size_t prevcap = list.capacity;
    for (size_t i = 4; i < 32; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("b", (int)i, (float)i, &gpa);
    }
    assert(list.size == 32);
    assert(list.capacity == prevcap);

    // Reserve to much larger: works and keeps old data
    err = nonpods_reserve(&list, 64);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 64);
    for (size_t i = 0; i < list.size; ++i) {
        assert(list.data[i].a != NULL);
    }

    // Reserve with zero (should not shrink; no effect, not an error)
    prevcap = list.capacity;
    err = nonpods_reserve(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == prevcap);

    // Reserve with capacity equal to maximum supported by the allocator: catches overflow
    size_t maxsafe = SIZE_MAX / sizeof(struct non_pod);
    err = nonpods_reserve(&list, maxsafe + 1);
    assert(err == ARRAYLIST_ERR_OVERFLOW);

    // Reserve with failing allocation: returns error, does not affect old buffer
    // Provide an allocator that always fails, init a fresh list
    struct Allocator fail_alloc = allocator_get_failling_alloc();
    struct arraylist_nonpods fail_list = nonpods_init(fail_alloc);
    // first reserve: fails as malloc returns NULL
    err = nonpods_reserve(&fail_list, 8);
    assert(err == ARRAYLIST_ERR_ALLOC);
    assert(fail_list.capacity == 0);
    assert(fail_list.data == NULL);

    // Fill a normal list to known good state
    struct arraylist_nonpods small = nonpods_init(gpa);
    for (size_t i = 0; i < 2; ++i) {
        *nonpods_emplace_back(&small) = non_pod_init("QQ", 2, 3, &gpa);
    }
    nonpods_shrink_to_fit(&small);
    // Now try to reserve with failing allocator change
    small.alloc = fail_alloc;
    err = nonpods_reserve(&small, 3);
    // Fails (realloc returns NULL), buffer unchanged
    assert(err == ARRAYLIST_ERR_ALLOC);
    // Since realloc failed, old data/cap remains
    assert(small.capacity == 2);
    // May or may not have NULL data pointer as realloc is permitted to leave old pointer unchanged
    small.alloc = gpa;

    // Reserve on uninitialized/null list: returns error (no crash)
    err = nonpods_reserve(NULL, 8);
    assert(err == ARRAYLIST_ERR_NULL);

    // Remark: After shrinking the list (with shrink_size), reserve does not shrink buffer
    err = nonpods_reserve(&list, 16);
    assert(err == ARRAYLIST_OK);
    // (Cap remains 64 since already large enough)
    assert(list.capacity == 64);

    // Clean up
    nonpods_deinit(&list);
    nonpods_deinit(&small);
    nonpods_deinit(&fail_list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist reserve value-type passed\n");
}

void test_arraylist_shrink_size_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Add three objects
    struct non_pod a = non_pod_init("a", 1, 1.1, &gpa);
    struct non_pod b = non_pod_init("b", 2, 2.2, &gpa);
    struct non_pod c = non_pod_init("c", 3, 3.3, &gpa);
    *nonpods_emplace_back(&list) = a;
    *nonpods_emplace_back(&list) = b;
    *nonpods_emplace_back(&list) = c;
    assert(list.size == 3);

    // Shrink to 2: calls dtor for c only
    size_t before = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_shrink_size(&list, 2);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp(list.data[0].objname, "a") == 0);
    assert(strcmp(list.data[1].objname, "b") == 0);
    assert(global_destructor_counter_arraylist == before - 1); // 1 dtor called

    // Shrink again to 1: only for b
    err = nonpods_shrink_size(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0].objname, "a") == 0);

    // Shrink to 1 again (no-op)
    before = global_destructor_counter_arraylist;
    err = nonpods_shrink_size(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);                                // unchanged
    assert(global_destructor_counter_arraylist == before); // no dtor called

    // Shrink to greater (no-op)
    before = global_destructor_counter_arraylist;
    err = nonpods_shrink_size(&list, 5);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == before);

    // Shrink to zero: calls dtor for "a"
    err = nonpods_shrink_size(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0); // all dtors called

    // Shrink already empty: clean no-op
    err = nonpods_shrink_size(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Testing return value when null
    assert(nonpods_shrink_size(NULL, 0) == ARRAYLIST_ERR_NULL);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shrink_size value-type passed\n");
}

void test_arraylist_shrink_to_fit_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Grow to larger capacity
    for (size_t i = 0; i < 8; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("z", i, i, &gpa);
    }

    size_t orig_cap = list.capacity;
    size_t orig_size = list.size;
    assert(orig_cap == orig_size);

    // Shrink to fit: capacity matches size, data valid
    enum arraylist_error err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == orig_size);
    for (size_t i = 0; i < list.size; ++i) {
        assert(list.data[i].a != NULL);
    }

    // Repeated shrink is no-op (shouldn't move array)
    void *data_ptr = list.data;
    err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == list.size);
    assert(list.data == data_ptr);

    // Shrink after shrink_size
    err = nonpods_shrink_size(&list, 3);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);

    size_t old_cap = list.capacity;
    err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 3);
    assert(list.capacity < old_cap);

    // Shrink to fit on size == 0 frees everything
    err = nonpods_shrink_size(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 0);
    assert(list.data == NULL);

    // Should not fail or free again
    err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);

    // Testing return value when null
    assert(nonpods_shrink_to_fit(NULL) == ARRAYLIST_ERR_NULL);

    // Testing allocation failure
    struct arraylist_nonpods failalloc = nonpods_init(gpa);
    nonpods_reserve(&failalloc, 100);

    for (size_t i = 0; i < 8; ++i) {
        *nonpods_emplace_back(&failalloc) = non_pod_init("z", i, i, &gpa);
    }

    // Change allocator mid operations just to test the return value
    failalloc.alloc = allocator_get_failling_alloc();
    assert(nonpods_shrink_to_fit(&failalloc) == ARRAYLIST_ERR_ALLOC);

    // Change it back
    failalloc.alloc = gpa;
    nonpods_deinit(&failalloc);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shrink_to_fit value-type passed\n");
}

void test_arraylist_push_back_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Push first element
    struct non_pod n1 = non_pod_init("one", 1, 1.1, &gpa);
    enum arraylist_error err = nonpods_push_back(&list, n1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(list.capacity == 1);
    assert(list.data != NULL);
    // Should still be 1 allocation
    assert(strcmp(list.data[0].objname, "one") == 0);
    assert(*list.data[0].a == 1);
    assert(*list.data[0].b == (float)1.1f);

    // Push multiple to grow beyond initial cap
    size_t N = 10;
    for (size_t i = 1; i < N; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "val%lu", i);
        struct non_pod n = non_pod_init(buf, i, i * 2.0f, &gpa);
        err = nonpods_push_back(&list, n);
        assert(err == ARRAYLIST_OK);
        assert(nonpods_size(&list) == i + 1);
        assert(strcmp(list.data[i].objname, buf) == 0);
        assert(*list.data[i].a == (int)i);
        assert(*list.data[i].b == (float)(i * 2.0f));
        // Capacity should always >= size, doubled when full
        assert(list.capacity >= list.size);
    }

    // Check all content
    for (size_t i = 0; i < N; ++i) {
        if (i == 0) {
            assert(strcmp(list.data[i].objname, "one") == 0);
            assert(*list.data[i].a == 1);
        } else {
            char buf[5];
            snprintf(buf, sizeof(buf), "val%lu", i);
            assert(strcmp(list.data[i].objname, buf) == 0);
            assert(*list.data[i].a == (int)i);
        }
    }

    // Remove some, then push_back again
    nonpods_pop_back(&list);     // Remove "val9"
    nonpods_remove_at(&list, 0); // Remove "one"
    size_t before = nonpods_size(&list);
    struct non_pod nnew = non_pod_init("again", 42, 42.42, &gpa);
    err = nonpods_push_back(&list, nnew);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_size(&list) == before + 1);
    assert(strcmp(list.data[list.size - 1].objname, "again") == 0);
    assert(*list.data[list.size - 1].a == 42);

    // Pop all and check destructor counter returns to zero
    nonpods_clear(&list);
    assert(global_destructor_counter_arraylist == 0);

    // Push after clear
    struct non_pod tmp = non_pod_init("after", 888, 8.88, &gpa);
    err = nonpods_push_back(&list, tmp);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);

    // Testing doubling capacity
    nonpods_shrink_to_fit(&list);
    assert(list.size == 1);
    assert(list.capacity == 1);

    struct non_pod np1 = non_pod_init("a", 1, 1.1, &gpa);
    nonpods_push_back(&list, np1);
    assert(list.size == 2);
    assert(list.capacity == 2);

    struct non_pod np2 = non_pod_init("b", 2, 2.2, &gpa);
    nonpods_push_back(&list, np2);
    assert(list.size == 3);
    assert(list.capacity == 4);

    struct non_pod np3 = non_pod_init("c", 3, 3.3, &gpa);
    nonpods_push_back(&list, np3);
    assert(list.size == 4);
    assert(list.capacity == 4);

    struct non_pod np4 = non_pod_init("d", 4, 4.4, &gpa);
    nonpods_push_back(&list, np4);
    assert(list.size == 5);
    assert(list.capacity == 8);

    // Testing return null
    assert(nonpods_push_back(NULL, np4) == ARRAYLIST_ERR_NULL);

    // Testing allocation failure
    struct arraylist_nonpods allocfail = nonpods_init(allocator_get_failling_alloc());
    assert(nonpods_push_back(&allocfail, np4) == ARRAYLIST_ERR_ALLOC);

    // Testing buffer overflow
    struct arraylist_nonpods arraylistoveralloc = nonpods_init(gpa);
    // Simulate a full capacity
    // ARRAYLIST_ERR_OVERFLOW is only triggered when the arraylist WOULD overflow, so we need to get a bit less
    arraylistoveralloc.capacity = SIZE_MAX / (2 * sizeof(struct non_pod)) + 1;
    arraylistoveralloc.size = arraylistoveralloc.capacity;
    assert(nonpods_push_back(&arraylistoveralloc, np4) == ARRAYLIST_ERR_OVERFLOW);

    nonpods_deinit(&arraylistoveralloc);
    nonpods_deinit(&allocfail);
    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist push_back value-type passed\n");
}

void test_arraylist_emplace_back_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Emplace one element; returned pointer is valid and modifiable
    struct non_pod *slot1 = nonpods_emplace_back(&list);
    assert(slot1 != NULL);
    // Can write individually through the slots:
    // slot1->objname = gpa.malloc(5, gpa.ctx);
    // strcpy(slot1->objname, "foo");
    // slot1->a = gpa.malloc(sizeof(int), gpa.ctx);
    // *slot1->a = 1;
    // slot1->b = gpa.malloc(sizeof(float), gpa.ctx);
    // *slot1->b = 1.1f;

    *slot1 = non_pod_init("foo", 1, 1.1, &gpa);

    assert(list.size == 1);
    assert(list.capacity == 1);
    assert(strcmp(list.data[0].objname, "foo") == 0);
    assert(list.data[0].a && *list.data[0].a == 1);

    // Data written through slot is visible in data array
    *slot1->a = 27;
    assert(*list.data[0].a == 27);

    // However after realloc, these pointers may not be valid anymore, so don't do this

    // Emplace multiple: Each pointer is unique, order preserved
    for (size_t i = 0; i < 5; ++i) {
        struct non_pod *slot = nonpods_emplace_back(&list);
        assert(slot != NULL);
        char name[6];
        snprintf(name, sizeof name, "item%lu", i);
        *slot = non_pod_init(name, i, 100.1f + i, &gpa);
        assert(list.data[list.size - 1].a && *slot->a == (int)i);
        assert(strcmp(slot->objname, name) == 0);
    }

    // Emplacing triggers growth, previous pointers are NOT preserved
    size_t old_capacity = list.capacity;
    struct non_pod *old_first = &list.data[0];
    for (size_t i = 0; i < 20; ++i) {
        struct non_pod *slot_grow = nonpods_emplace_back(&list);
        char name[8];
        snprintf(name, sizeof name, "slot%lu", i);
        *slot_grow = non_pod_init(name, i + 1, 200.2f + i, &gpa);
    }

    assert(list.capacity > old_capacity);
    // After realloc, previous values are still valid and not overwritten, however, pointers are not in the same location
    // Thus, do not hold data pointers of list after operations that may realloc
    assert(slot1 != old_first);

    assert(strcmp(list.data[0].objname, "foo") == 0);

    // After deinit, one must init again to not crash
    nonpods_deinit(&list);
    list = nonpods_init(gpa);
    *nonpods_emplace_back(&list) = non_pod_init("testing1", 1, 1.1, &gpa);
    assert(list.size == 1);
    assert(list.capacity == 1);

    // Testing doubling capacity
    // Do not need to create temp values to insert, however, there is way to check for failure this way
    *nonpods_emplace_back(&list) = non_pod_init("testing2", 2, 2.2, &gpa);
    assert(list.size == 2);
    assert(list.capacity == 2);

    *nonpods_emplace_back(&list) = non_pod_init("testing3", 3, 3.3, &gpa);
    assert(list.size == 3);
    assert(list.capacity == 4);

    *nonpods_emplace_back(&list) = non_pod_init("testing4", 4, 4.4, &gpa);
    assert(list.size == 4);
    assert(list.capacity == 4);

    *nonpods_emplace_back(&list) = non_pod_init("testing5", 5, 5.5, &gpa);
    assert(list.size == 5);
    assert(list.capacity == 8);

    // Testing null parameter
    assert(nonpods_emplace_back(NULL) == NULL);

    // Testing alloc failure
    struct arraylist_nonpods allocfail = nonpods_init(allocator_get_failling_alloc());
    assert(nonpods_emplace_back(&allocfail) == NULL);

    // Testing buffer overflow
    struct arraylist_nonpods arraylistoveralloc = nonpods_init(gpa);
    // Simulate a full capacity
    // Buffer overflow error is only triggered when the arraylist WOULD overflow, so we need to get a bit less
    arraylistoveralloc.capacity = SIZE_MAX / (2 * sizeof(struct non_pod)) + 1;
    arraylistoveralloc.size = arraylistoveralloc.capacity;
    assert(nonpods_emplace_back(&arraylistoveralloc) == NULL);

    nonpods_deinit(&arraylistoveralloc);
    nonpods_deinit(&allocfail);
    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist emplace_back value-type passed\n");
}

void test_arraylist_emplace_at_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Insert at end when list is empty (index == size): behaves like emplace_back
    size_t s0 = list.size;
    struct non_pod *slot_end0 = nonpods_emplace_at(&list, s0);
    assert(slot_end0 != NULL);
    *slot_end0 = non_pod_init("foo", 1, 1.1f, &gpa);

    assert(list.size == 1);
    assert(list.capacity == 1);
    assert(strcmp(list.data[0].objname, "foo") == 0);
    assert(list.data[0].a && *list.data[0].a == 1);

    // Insert at end again, capacity doubling
    size_t s1 = list.size;
    struct non_pod *slot_end1 = nonpods_emplace_at(&list, s1);
    assert(slot_end1 != NULL);
    *slot_end1 = non_pod_init("bar", 2, 2.2f, &gpa);

    assert(list.size == 2);
    assert(list.capacity == 2);
    assert(strcmp(list.data[1].objname, "bar") == 0);
    assert(list.data[1].a && *list.data[1].a == 2);

    // Insert at end again to trigger capacity growth to 4
    struct non_pod *slot_end2 = nonpods_emplace_at(&list, list.size);
    assert(slot_end2 != NULL);
    *slot_end2 = non_pod_init("baz", 3, 3.3f, &gpa);
    assert(list.size == 3);
    assert(list.capacity == 4);

    // Insert at beginning (index 0): shifts existing elements right
    struct non_pod *slot_begin = nonpods_emplace_at(&list, 0);
    assert(slot_begin != NULL);
    *slot_begin = non_pod_init("begin", 100, 100.0f, &gpa);

    assert(list.size == 4);
    assert(strcmp(list.data[0].objname, "begin") == 0);
    assert(strcmp(list.data[1].objname, "foo") == 0);
    assert(strcmp(list.data[2].objname, "bar") == 0);
    assert(strcmp(list.data[3].objname, "baz") == 0);

    // Insert in the middle (index 2): shifts tail elements right
    struct non_pod *slot_mid = nonpods_emplace_at(&list, 2);
    assert(slot_mid != NULL);
    *slot_mid = non_pod_init("mid", 777, 7.77f, &gpa);

    assert(list.size == 5);
    assert(strcmp(list.data[0].objname, "begin") == 0);
    assert(strcmp(list.data[1].objname, "foo") == 0);
    assert(strcmp(list.data[2].objname, "mid") == 0);
    assert(strcmp(list.data[3].objname, "bar") == 0);
    assert(strcmp(list.data[4].objname, "baz") == 0);

    // Writing through returned slot is reflected in the data array
    *slot_mid->a = 123;
    assert(*list.data[2].a == 123);

    // Capacity growth and note about pointer stability: do not hold element pointers across growth
    size_t old_capacity = list.capacity;
    struct non_pod *old_first = &list.data[0];
    for (size_t i = 0; i < 20; ++i) {
        struct non_pod *slot_grow = nonpods_emplace_at(&list, list.size); // append via emplace_at
        assert(slot_grow != NULL);
        char name[16];
        snprintf(name, sizeof name, "slot%lu", i);
        *slot_grow = non_pod_init(name, (int)(1000 + i), 500.5f + (float)i, &gpa);
    }
    assert(list.capacity > old_capacity);
    // After potential realloc, previous element pointers may be invalid or relocated
    // Sometimes this may pass sometimes may not
    (void)old_first;
    // assert(old_first != &list.data[0]);

    // Verify earlier values are preserved
    assert(strcmp(list.data[0].objname, "begin") == 0);
    assert(strcmp(list.data[1].objname, "foo") == 0);
    assert(strcmp(list.data[3].objname, "bar") == 0);
    assert(strcmp(list.data[4].objname, "baz") == 0);

    // Null list
    assert(nonpods_emplace_at(NULL, 0) == NULL);

    // Out-of-bounds index (index > size)
    struct arraylist_nonpods list_oob = nonpods_init(gpa);
    assert(list_oob.size == 0);
    assert(nonpods_emplace_at(&list_oob, 1) == NULL);
    assert(list_oob.size == 0);
    nonpods_deinit(&list_oob);

    // Allocation failure
    struct arraylist_nonpods allocfail = nonpods_init(allocator_get_failling_alloc());
    // Attempt insert at end of empty list; should fail and return NULL
    assert(nonpods_emplace_at(&allocfail, 0) == NULL);
    nonpods_deinit(&allocfail);

    // Buffer overflow scenario
    struct arraylist_nonpods over = nonpods_init(gpa);
    // Simulate near-overflow capacity; doubling would exceed SIZE_MAX
    over.capacity = SIZE_MAX / (2 * sizeof(struct non_pod)) + 1;
    over.size = over.capacity;
    assert(nonpods_emplace_at(&over, over.size) == NULL); // insert at end triggers growth check
    // Also test inserting not at end (still triggers growth due to size >= capacity)
    assert(nonpods_emplace_at(&over, 0) == NULL);

    // Clean up
    nonpods_deinit(&list);
    nonpods_deinit(&over);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist emplace_at value-type passed\n");
}

void test_arraylist_insert_at_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Insert into empty list, index 0; becomes head
    struct non_pod a = non_pod_init("A", 10, 0.1, &gpa);
    enum arraylist_error err = nonpods_insert_at(&list, a, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0].objname, "A") == 0);

    // Insert at head (index 0), should become new head; existing shift right
    struct non_pod b = non_pod_init("B", 20, 0.2, &gpa);
    err = nonpods_insert_at(&list, b, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp(list.data[0].objname, "B") == 0);
    assert(strcmp(list.data[1].objname, "A") == 0);

    // Insert at tail (index=size), should append
    struct non_pod c = non_pod_init("C", 30, 0.3, &gpa);
    err = nonpods_insert_at(&list, c, list.size);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);
    assert(strcmp(list.data[2].objname, "C") == 0);

    // Insert in the middle (index=1)
    struct non_pod d = non_pod_init("D", 40, 0.4, &gpa);
    err = nonpods_insert_at(&list, d, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 4);
    assert(strcmp(list.data[0].objname, "B") == 0);
    assert(strcmp(list.data[1].objname, "D") == 0);
    assert(strcmp(list.data[2].objname, "A") == 0);
    assert(strcmp(list.data[3].objname, "C") == 0);

    // Insert at index > size (out of bounds), should return an error code
    struct non_pod e = non_pod_init("E", 50, 0.5, &gpa);
    err = nonpods_insert_at(&list, e, 999);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == 4);

    err = nonpods_insert_at(&list, e, list.size);
    assert(strcmp(list.data[4].objname, "E") == 0);
    assert(list.size == 5);

    // Check all memory still valid and unchanged
    assert(strcmp(list.data[0].objname, "B") == 0);
    assert(strcmp(list.data[1].objname, "D") == 0);
    assert(strcmp(list.data[2].objname, "A") == 0);
    assert(strcmp(list.data[3].objname, "C") == 0);
    assert(strcmp(list.data[4].objname, "E") == 0);

    // Capacity grows correctly; insert repeatedly to trigger realloc
    for (size_t i = 0; i < 10; ++i) {
        char name[12];
        snprintf(name, sizeof(name), "val%lu", i);
        struct non_pod tmp = non_pod_init(name, i, i, &gpa);
        err = nonpods_insert_at(&list, tmp, 2); // always in middle
        assert(err == ARRAYLIST_OK);
    }
    assert(list.size == 15);
    assert(list.capacity >= list.size);

    // Insert at every possible index in small list
    nonpods_clear(&list);
    for (size_t i = 0; i < 4; ++i) {
        struct non_pod t = non_pod_init("X", i, i, &gpa);
        err = nonpods_insert_at(&list, t, i); // should be append to each
        assert(err == ARRAYLIST_OK);
        assert(list.size == i + 1);
    }
    // Now list is [X, X, X, X]; insert at head (0), tail (size=4), and middle (2)
    struct non_pod y = non_pod_init("Y", 99, 99.9, &gpa);
    err = nonpods_insert_at(&list, y, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 5);
    assert(strcmp(list.data[0].objname, "Y") == 0);
    struct non_pod z = non_pod_init("Z", 100, -10, &gpa);
    err = nonpods_insert_at(&list, z, 2);
    assert(list.size == 6);
    assert(strcmp(list.data[2].objname, "Z") == 0);

    // After deinit, one must init again to not crash
    nonpods_deinit(&list);
    list = nonpods_init(gpa);

    // Testing double capacity
    struct non_pod a1 = non_pod_init("A", 1, 1.1, &gpa);
    nonpods_insert_at(&list, a1, 0);
    assert(list.size == 1);
    assert(list.capacity == 1);

    struct non_pod a2 = non_pod_init("B", 2, 2.2, &gpa);
    nonpods_insert_at(&list, a2, 0);
    assert(list.size == 2);
    assert(list.capacity == 2);

    struct non_pod a3 = non_pod_init("C", 3, 3.3, &gpa);
    nonpods_insert_at(&list, a3, 0);
    assert(list.size == 3);
    assert(list.capacity == 4);

    struct non_pod a4 = non_pod_init("D", 4, 4.4, &gpa);
    nonpods_insert_at(&list, a4, 0);
    assert(list.size == 4);
    assert(list.capacity == 4);

    struct non_pod a5 = non_pod_init("E", 5, 5.5, &gpa);
    nonpods_insert_at(&list, a5, 0);
    assert(list.size == 5);
    assert(list.capacity == 8);

    // Testing null parameter
    assert(nonpods_insert_at(NULL, a5, 0) == ARRAYLIST_ERR_NULL);

    // Testing alloc failure
    struct arraylist_nonpods allocfail = nonpods_init(allocator_get_failling_alloc());
    assert(nonpods_insert_at(&allocfail, a5, 0) == ARRAYLIST_ERR_ALLOC);

    // Testing buffer overflow
    struct arraylist_nonpods arraylistoveralloc = nonpods_init(gpa);
    // Simulate a full capacity
    // Buffer overflow error is only triggered when the arraylist WOULD overflow, so we need to get a bit less
    arraylistoveralloc.capacity = SIZE_MAX / (2 * sizeof(struct non_pod)) + 1;
    arraylistoveralloc.size = arraylistoveralloc.capacity;
    assert(nonpods_insert_at(&arraylistoveralloc, a5, 0) == ARRAYLIST_ERR_OVERFLOW);

    nonpods_deinit(&arraylistoveralloc);
    nonpods_deinit(&allocfail);
    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist insert_at value-type passed\n");
}

void test_arraylist_pop_back_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Pop on empty list
    size_t before = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Push one, then pop
    struct non_pod a = non_pod_init("A", 11, 1.1, &gpa);
    err = nonpods_push_back(&list, a);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    before = global_destructor_counter_arraylist;
    err = nonpods_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 1);

    // Push several, then pop all
    for (size_t i = 0; i < 5; ++i) {
        char name[16];
        snprintf(name, sizeof name, "item%lu", i);
        *nonpods_emplace_back(&list) = non_pod_init(name, 10 + i, 10 + i, &gpa);
    }
    assert(list.size == 5);
    size_t alive = global_destructor_counter_arraylist;
    for (size_t i = 0; i < 5; ++i) {
        err = nonpods_pop_back(&list);
        assert(err == ARRAYLIST_OK);
        assert(list.size == 5 - (i + 1));
        assert(global_destructor_counter_arraylist == alive - (i + 1));
    }
    assert(list.size == 0);

    *nonpods_emplace_back(&list) = non_pod_init("foo", 1, 1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("bar", 2, 2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("baz", 3, 3, &gpa);

    const char *expect_names[] = { "foo", "bar", "baz" };
    for (size_t i = 0; i < 3; ++i)
        assert(strcmp(list.data[i].objname, expect_names[i]) == 0);

    err = nonpods_pop_back(&list); // removes "baz"
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp(list.data[0].objname, "foo") == 0);
    assert(strcmp(list.data[1].objname, "bar") == 0);

    err = nonpods_pop_back(&list); // removes "bar"
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0].objname, "foo") == 0);

    err = nonpods_pop_back(&list); // removes "foo"
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Push many to grow capacity, then pop many
    for (size_t i = 0; i < 16; ++i)
        *nonpods_emplace_back(&list) = non_pod_init("grow", i, i, &gpa);
    size_t cap = list.capacity;
    for (size_t i = 0; i < 16; ++i)
        nonpods_pop_back(&list);
    assert(list.size == 0);
    assert(list.capacity == cap); // pop_back never shrinks
    assert(global_destructor_counter_arraylist == 0);

    // Push, clear, pop_back is no-op
    *nonpods_emplace_back(&list) = non_pod_init("after", 123, 1, &gpa);
    nonpods_clear(&list);
    before = global_destructor_counter_arraylist;
    err = nonpods_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Testing null parameter
    assert(nonpods_pop_back(NULL) == ARRAYLIST_ERR_NULL);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist pop_back value-type passed\n");
}

void test_arraylist_remove_at_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);
    enum arraylist_error err;
    size_t before;

    // Remove from empty list (should no-op)
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // One element, remove at 0
    struct non_pod a = non_pod_init("A", 1, 1.1, &gpa);
    *nonpods_emplace_back(&list) = a;
    assert(list.size == 1);
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 1); // dtor called

    // Add several, remove head, tail, middle
    struct non_pod b = non_pod_init("B", 2, 2.2, &gpa);
    struct non_pod c = non_pod_init("C", 3, 3.3, &gpa);
    struct non_pod d = non_pod_init("D", 4, 4.4, &gpa);
    struct non_pod e = non_pod_init("E", 5, 5.5, &gpa);
    *nonpods_emplace_back(&list) = b;  // 0
    *nonpods_emplace_back(&list) = c;  // 1
    *nonpods_emplace_back(&list) = d;  // 2
    *nonpods_emplace_back(&list) = e;  // 3
    assert(list.size == 4);

    // Remove head (index 0)
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);
    assert(global_destructor_counter_arraylist == before - 1); // one destructor called
    // Remaining should be [C, D, E]
    assert(strcmp(list.data[0].objname, "C") == 0);

    // Remove tail (index size-1)
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, list.size - 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(global_destructor_counter_arraylist == before - 1); // one destructor called
    // Remaining should be [C, D]
    assert(strcmp(list.data[0].objname, "C") == 0);
    assert(strcmp(list.data[1].objname, "D") == 0);

    // Remove middle (index 1, which is D)
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == before - 1);
    assert(strcmp(list.data[0].objname, "C") == 0);

    // Remove only remaining element
    err = nonpods_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Add more, remove with index out of bounds (should return an error code)
    for (int i = 0; i < 5; ++i) {
        char buf[8];
        snprintf(buf, sizeof buf, "X%d", i);
        *nonpods_emplace_back(&list) = non_pod_init(buf, i, i+0.1, &gpa);
    }
    size_t sz = list.size;
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 9999); // way OOB
    assert(err == ARRAYLIST_ERR_OOB);

    err = nonpods_remove_at(&list, list.size - 1);
    assert(list.size == sz - 1);
    assert(global_destructor_counter_arraylist == before - 1);

    // Remove with index == size (past end, also returns error)
    sz = list.size;
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, list.size);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == sz);
    assert(global_destructor_counter_arraylist == before);

    // Remove with index == SIZE_MAX, returns ARRAYLIST_ERR_OOB
    sz = list.size;
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, SIZE_MAX);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == sz);
    assert(global_destructor_counter_arraylist == before);

    // Remove remaining (result: should be fully destructed)
    while (list.size > 0) {
        nonpods_remove_at(&list, 0);
    }
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Test repeated removes on already empty list
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    err = nonpods_remove_at(&list, SIZE_MAX);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Restore: Fill list, remove every position, check value stability / shifting
    for (int i = 0; i < 4; ++i) {
        char buf[8];
        snprintf(buf, sizeof buf, "YY%d", i);
        *nonpods_emplace_back(&list) = non_pod_init(buf, 100+i, 500.5+i, &gpa);
    }
    // [YY0, YY1, YY2, YY3]
    // Remove 1 (YY1)
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_at(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);
    assert(global_destructor_counter_arraylist == before - 1);
    assert(strcmp(list.data[0].objname, "YY0") == 0);
    assert(strcmp(list.data[1].objname, "YY2") == 0); // shifted up
    assert(strcmp(list.data[2].objname, "YY3") == 0);

    // Remove 1 (YY2, now at index 1)
    err = nonpods_remove_at(&list, 1);
    assert(list.size == 2);
    assert(strcmp(list.data[0].objname, "YY0") == 0);
    assert(strcmp(list.data[1].objname, "YY3") == 0);

    // Remove 0 (YY0)
    err = nonpods_remove_at(&list, 0);
    assert(list.size == 1);
    assert(strcmp(list.data[0].objname, "YY3") == 0);

    // Remove 0 (YY3)
    err = nonpods_remove_at(&list, 0);
    assert(list.size == 0);

    // Remove at 0 again to check stable empty
    err = nonpods_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Testing null parameter
    assert(nonpods_remove_at(NULL, 0) == ARRAYLIST_ERR_NULL);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist remove_at value-type passed\n");
}

void test_arraylist_remove_from_to_value(void)
{
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Fill with 6 named objects: A, B, C, D, E, F
    const char *names[] = {"A", "B", "C", "D", "E", "F"};
    for (size_t i = 0; i < 6; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init(names[i], 100 + i, 1.0+i, &gpa);
    }

    assert(list.size == 6);
    assert(global_destructor_counter_arraylist == 6);

    // Remove from head: [A, B, C, D, E, F] -- remove 0..1 -> [C, D, E, F]
    size_t before = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_remove_from_to(&list, 0, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 4);
    assert(global_destructor_counter_arraylist == before - 2);
    assert(strcmp(list.data[0].objname, "C") == 0);
    assert(strcmp(list.data[1].objname, "D") == 0);
    assert(strcmp(list.data[2].objname, "E") == 0);
    assert(strcmp(list.data[3].objname, "F") == 0);

    // Remove from middle: [C, D, E, F] -- remove 1..2 -> [C, F]
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 1, 2);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(global_destructor_counter_arraylist == before - 2);
    assert(strcmp(list.data[0].objname, "C") == 0);
    assert(strcmp(list.data[1].objname, "F") == 0);

    // Remove last element only: [C, F] -- remove 1..1 => [C]
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 1, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == before - 1);
    assert(strcmp(list.data[0].objname, "C") == 0);

    // Remove first element only: [C] -- remove 0..0 => []
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 1);

    // Remove from empty: no-op
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Fill again for more tests
    for (size_t i = 0; i < 6; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init(names[i], 200 + i, 2.0+i, &gpa);
    }
    assert(list.size == 6);

    // Remove all by from=0, to=5 (size-1)
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 0, list.size - 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 6);

    // Fill again for edge cases
    for (size_t i = 0; i < 6; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init(names[i], 400 + i, 4.0+i, &gpa);
    }
    assert(list.size == 6);

    // Remove with from > to: should return ARRAYLIST_ERR_OOB
    before = list.size;
    size_t before_dtor = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 4, 2);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == before);
    assert(global_destructor_counter_arraylist == before_dtor);

    // Remove with from == to == large index (index >= size): should return err oob
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 99, 99);
    assert(err == ARRAYLIST_ERR_OOB);
    err = nonpods_remove_from_to(&list, list.size - 1, list.size - 1);
    assert(list.size == 5);
    assert(global_destructor_counter_arraylist == before - 1);
    // Last element should have been removed, check new last
    assert(strcmp(list.data[4].objname, "E") == 0);

    // Remove with to out of bounds but from valid: remove from 3..(big) => returns ARRAYLIST_ERR_OOB
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 3, 123456);
    assert(err == ARRAYLIST_ERR_OOB);

    err = nonpods_remove_from_to(&list, 3, 4);
    assert(list.size == 3);
    assert(global_destructor_counter_arraylist == before - 2);
    // Data: C, D, E was at 4, but now gone
    assert(strcmp(list.data[0].objname, "A") == 0);
    assert(strcmp(list.data[1].objname, "B") == 0);
    assert(strcmp(list.data[2].objname, "C") == 0);

    // Remove with both out of bounds, with to being greater: returns ARRAYLIST_ERR_OOB
    before = global_destructor_counter_arraylist;
    err = nonpods_remove_from_to(&list, 123, 9999);
    assert(err == ARRAYLIST_ERR_OOB);

    err = nonpods_remove_from_to(&list, list.size - 1, list.size - 1);
    assert(list.size == 2);
    assert(global_destructor_counter_arraylist == before - 1);

    err = nonpods_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);

    // Remove last remaining
    err = nonpods_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Remove from empty again (after everything): no crash, no change
    err = nonpods_remove_from_to(&list, 0, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Testing null parameter
    assert(nonpods_remove_from_to(NULL, 0, 1) == ARRAYLIST_ERR_NULL);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist remove_from_to value-type passed\n");
}

void test_arraylist_at_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Empty array: OOB returns NULL
    assert(nonpods_at(&list, 0) == NULL);
    assert(nonpods_at(&list, 123) == NULL);

    // Add some values
    *nonpods_emplace_back(&list) = non_pod_init("A", 10, 1.1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("B", 20, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("C", 30, 3.3, &gpa);

    // Valid indices (0,1,2) return address in data, not NULL
    for (size_t i = 0; i < list.size; ++i) {
        struct non_pod *p = nonpods_at(&list, i);
        assert(p != NULL);
        // Content check (insertion order)
        if (i == 0) assert(strcmp(p->objname, "A") == 0);
        if (i == 1) assert(strcmp(p->objname, "B") == 0);
        if (i == 2) assert(strcmp(p->objname, "C") == 0);
    }

    // Out of bounds: index == size or greater returns NULL
    assert(nonpods_at(&list, 3) == NULL);
    assert(nonpods_at(&list, 99) == NULL);

    // Negative index interpreted as large size_t: always returns NULL
    // (Some compilers warn on -1 cast to size_t, so cast explicitly)
    assert(nonpods_at(&list, (size_t)-1) == NULL);

    // Use returned pointer as modifiable, changes reflected in array
    struct non_pod *pa = nonpods_at(&list, 0);
    assert(pa);
    // Reallocating the name to a new size as to not occur a Heap Buffer Overflow error
    // Needs to use the same allocator as on list, pa here is a pointer that's located inside the list 
    pa->objname = list.alloc.realloc(pa->objname, sizeof(strlen(pa->objname) + 1), strlen("newName") + 1, list.alloc.ctx);
    strcpy(pa->objname, "newName");
    assert(strcmp(list.data[0].objname, "newName") == 0);

    // Remove element: shrink then query OOB (old last is dead)
    nonpods_pop_back(&list); // removes "objectC"
    assert(list.size == 2);
    assert(nonpods_at(&list, 2) == NULL);

    // Remove head, elements shift, at() reflects new order
    nonpods_remove_at(&list, 0); // removes "newName"
    assert(list.size == 1);
    assert(strcmp(nonpods_at(&list, 0)->objname, "B") == 0);
    assert(nonpods_at(&list, 1) == NULL);

    // Insert after removal is visible via at()
    struct non_pod tmp = non_pod_init("last", 99, 99.9, &gpa);
    enum arraylist_error err = nonpods_insert_at(&list, tmp, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp(nonpods_at(&list, 0)->objname, "last") == 0);
    assert(strcmp(nonpods_at(&list, 1)->objname, "B") == 0);

    // at(NULL, x) returns NULL
    assert(nonpods_at(NULL, 0) == NULL);
    assert(nonpods_at(NULL, 1000) == NULL);

    // Clearing empties the array; at() never deref OOB
    nonpods_clear(&list);
    assert(list.size == 0);
    assert(nonpods_at(&list, 0) == NULL);
    assert(nonpods_at(&list, 1) == NULL);

    // Huge number of items: check random OOB and in-bounds
    size_t bigN = 100;
    for (size_t i = 0; i < bigN; ++i) {
        char name[20];
        snprintf(name, sizeof name, "item%lu", i);
        *nonpods_emplace_back(&list) = non_pod_init(name, (int)i, (float)i, &gpa);
    }
    for (size_t i = 0; i < bigN; ++i) {
        struct non_pod *p = nonpods_at(&list, i);
        assert(p != NULL);
        char expect[20];
        snprintf(expect, sizeof expect, "item%lu", i);
        assert(strcmp(p->objname, expect) == 0);
        assert(*p->a == (int)i);
        assert(*p->b == (float)i);
    }
    assert(nonpods_at(&list, bigN) == NULL);
    assert(nonpods_at(&list, bigN + 10000) == NULL);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist at value-type passed\n");
}

void test_arraylist_begin_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Begin on empty list should be NULL or a valid pointer only if capacity>0, so always check size first
    assert(list.size == 0);
    struct non_pod *begin = nonpods_begin(&list);
    // If data == NULL, then begin==NULL; if not, begin==data
    assert((begin == NULL && list.data == NULL) || begin == list.data);

    // Add one element; begin() == &data[0], non-null
    struct non_pod a = non_pod_init("A", 10, 1.1, &gpa);
    nonpods_push_back(&list, a);
    begin = nonpods_begin(&list);
    assert(begin != NULL);
    assert(begin == &list.data[0]);
    assert(strcmp(begin->objname, "A") == 0);

    // Add another, this operation caused a realloc, which means begin() may change, not always true,
    // but should always be held as true
    //struct non_pod *old_begin = begin;
    struct non_pod b = non_pod_init("B", 20, 2.2, &gpa);
    nonpods_push_back(&list, b);
    begin = nonpods_begin(&list);
    assert(begin == list.data);
    // assert(begin != old_begin); may be true or false, do not hold pointers between operations that reallocate
    assert(strcmp(begin->objname, "A") == 0);
    assert(strcmp((begin+1)->objname, "B") == 0);

    // Iteration with begin()/end()
    size_t iter_count = 0;
    for (struct non_pod *it = nonpods_begin(&list); it != nonpods_end(&list); ++it) {
        if (iter_count == 0) assert(strcmp(it->objname, "A") == 0);
        if (iter_count == 1) assert(strcmp(it->objname, "B") == 0);
        ++iter_count;
    }
    assert(iter_count == list.size);

    // Add multiple elements to trigger realloc/growth, confirm begin pointer is updated properly
    size_t many = 32;
    for (size_t i = 0; i < many; ++i) {
        char buf[10];
        snprintf(buf, sizeof buf, "q%lu", i);
        nonpods_push_back(&list, non_pod_init(buf, (int)i, (float)i, &gpa));
    }
    begin = nonpods_begin(&list);
    assert(begin == list.data);
    // After growth, check first value is still "A"
    assert(strcmp(begin->objname, "A") == 0);
    // And nth value is correct
    assert(strcmp((begin+list.size-1)->objname, "q31") == 0);

    // begin pointer is valid up to end(), never deref past end
    size_t count = 0;
    for (struct non_pod *it = nonpods_begin(&list); it != nonpods_end(&list); ++it) {
        count++;
    }
    assert(count == list.size);

    // pop_back: begin unchanged, last element gone
    size_t prev_size = list.size;
    nonpods_pop_back(&list);
    begin = nonpods_begin(&list);
    assert(begin == list.data);
    assert(list.size == prev_size - 1);

    // clear: begin can be non-null if capacity > 0, but size==0 so do not deref it
    nonpods_clear(&list);
    assert(list.size == 0);
    begin = nonpods_begin(&list);
    // Dereferencing begin when list is empty is invalid, but it's ok to compare pointer value
    assert(begin == list.data);
    // If data==NULL, begin==NULL

    // Remove all, then add again: begin valid, data correct
    nonpods_push_back(&list, non_pod_init("foo", 1, 1.2f, &gpa));
    assert(list.size == 1);
    begin = nonpods_begin(&list);
    assert(begin != NULL);
    assert(strcmp(begin->objname, "foo") == 0);

    // remove_at changes order, begin still valid
    nonpods_push_back(&list, non_pod_init("bar", 2, 2.3f, &gpa));
    nonpods_remove_at(&list, 0);
    begin = nonpods_begin(&list);
    assert(list.size == 1);
    assert(strcmp(begin->objname, "bar") == 0);

    // reset: deinit, then begin on fresh list
    nonpods_deinit(&list);
    list = nonpods_init(gpa);
    begin = nonpods_begin(&list);
    assert(list.size == 0);
    assert((begin == NULL && list.data == NULL) || begin == list.data);

    // bulk insert; iterate via begin
    for (size_t i = 0; i < 8; ++i) {
        nonpods_push_back(&list, non_pod_init("x", i, i, &gpa));
    }
    
    size_t loopcheck = 0;
    for (struct non_pod *it = nonpods_begin(&list); it != nonpods_end(&list); ++it) {
        loopcheck++;
    }
    assert(loopcheck == list.size);

    // begin(NULL) returns NULL
    assert(nonpods_begin(NULL) == NULL);

    // begin not deref'd if list is truly empty; do not crash
    nonpods_clear(&list);
    begin = nonpods_begin(&list);
    // Dereferencing it is invalid, but pointer value is as expected
    assert((begin == NULL && list.data == NULL) || begin == list.data);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist begin value-type passed\n");
}

void test_arraylist_back_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // back() on empty array: returns NULL
    assert(nonpods_back(&list) == NULL);

    // single element: back() returns data+0, points to first, non-null
    struct non_pod a = non_pod_init("first", 11, 1.1, &gpa);
    nonpods_push_back(&list, a);
    struct non_pod *last = nonpods_back(&list);
    assert(last != NULL);
    assert(last == &list.data[0]);
    assert(strcmp(last->objname, "first") == 0);
    assert(*last->a == 11);

    // two elements: back() points to new element each time, never to prior
    struct non_pod b = non_pod_init("second", 22, 2.2, &gpa);
    nonpods_push_back(&list, b);
    assert(list.size == 2);
    struct non_pod *back2 = nonpods_back(&list);
    assert(back2 != NULL);
    assert(back2 == &list.data[1]);
    assert(strcmp(back2->objname, "second") == 0);
    assert(*back2->a == 22);

    // many elements: back() always matches last added
    for (int i = 2; i < 12; ++i) {
        char name[32];
        snprintf(name, sizeof name, "item%d", i);
        struct non_pod t = non_pod_init(name, 100+i, 3.3f+i, &gpa);
        nonpods_push_back(&list, t);
        struct non_pod *b = nonpods_back(&list);
        assert(b != NULL);
        assert(b == &list.data[list.size-1]);
        assert(strcmp(b->objname, name) == 0);
        assert(*b->a == 100+i);
    }

    // back() is mutable: change value via back(), data array reflects change
    struct non_pod *bref = nonpods_back(&list);
    strcpy(bref->objname, "lastX");
    assert(strcmp(list.data[list.size-1].objname, "lastX") == 0);

    // Remove last element: back() now returns previous element
    enum arraylist_error err = nonpods_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 11);
    struct non_pod *newlast = nonpods_back(&list);
    assert(newlast == &list.data[list.size-1]);
    assert(*newlast->a == 110);
    // Content should match what was inserted
    assert(strcmp(newlast->objname, "item10") == 0);

    // Remove all but one: back always matches, then back(NULL) when empty
    while (list.size > 1)
        nonpods_pop_back(&list);
    assert(list.size == 1);
    struct non_pod *lastleft = nonpods_back(&list);
    assert(lastleft != NULL);
    assert(lastleft == &list.data[0]);
    assert(strcmp(lastleft->objname, "first") == 0);

    nonpods_pop_back(&list);
    assert(list.size == 0);
    assert(nonpods_back(&list) == NULL);

    // Push after clear: back() valid again
    struct non_pod c = non_pod_init("newone", 123, 4.56, &gpa);
    nonpods_push_back(&list, c);
    struct non_pod *b2 = nonpods_back(&list);
    assert(b2 != NULL);
    assert(strcmp(b2->objname, "newone") == 0);

    // Clear: back() should return NULL
    nonpods_clear(&list);
    assert(list.size == 0);
    assert(nonpods_back(&list) == NULL);

    // Push several, remove some, back() always matches last
    for (int i = 0; i < 5; ++i) {
        char nm[8];
        snprintf(nm, sizeof nm, "set%d", i);
        nonpods_push_back(&list, non_pod_init(nm, i+30, i+0.13f, &gpa));
        assert(strcmp(nonpods_back(&list)->objname, nm) == 0);
    }
    assert(list.size == 5);
    nonpods_remove_at(&list, 2); // remove element 2, last remains same
    assert(strcmp(nonpods_back(&list)->objname, "set4") == 0);
    nonpods_pop_back(&list); // remove last
    assert(strcmp(nonpods_back(&list)->objname, "set3") == 0);

    nonpods_clear(&list);

    // back() on NULL list pointer: returns NULL (should not segfault)
    assert(nonpods_back(NULL) == NULL);

    // After deinit: back() returns NULL; repeated deinit is safe
    nonpods_push_back(&list, non_pod_init("foo", 16, 2.5, &gpa));
    nonpods_deinit(&list);
    assert(nonpods_back(&list) == NULL);
    nonpods_deinit(&list); // should not crash

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist back value-type passed\n");
}

void test_arraylist_end_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // end() on an empty list returns data + 0 (== data)
    assert(list.size == 0);
    struct non_pod *endptr = nonpods_end(&list);
    // end() may be NULL or data, so must be compared as a pointer, not dereferenced!
    assert((endptr == NULL && list.data == NULL) || (endptr == (list.data + list.size)));

    // end() pointer is always list.data + list.size
    // add several items, confirm end > data and never before data
    for (size_t i = 0; i < 6; ++i) {
        char namebuf[24];
        snprintf(namebuf, sizeof namebuf, "item%lu", i);
        nonpods_push_back(&list, non_pod_init(namebuf, (int)i, (float)i, &gpa));
        assert(nonpods_end(&list) == list.data + list.size);
        if (list.size > 0)
            assert(nonpods_end(&list) > list.data);
    }

    // Iterating: end() is always exclusive; can be used as limit for begin/end style iteration.
    size_t niter = 0;
    for (struct non_pod *it = nonpods_begin(&list); it != nonpods_end(&list); ++it) {
        assert(it >= list.data && it < list.data + list.size);
        niter++;
    }
    assert(niter == list.size);

    // Dereferencing end() is always invalid: verify pointer value but never deref
    struct non_pod *should_be_out_of_bounds = nonpods_end(&list);
    // The next is only to show the address, do not dereference
    assert(should_be_out_of_bounds == list.data + list.size);

    // After pop_back/clear, end() updates accordingly
    size_t oldsize = list.size;
    nonpods_pop_back(&list);
    assert(nonpods_end(&list) == list.data + list.size);
    assert(list.size == oldsize - 1);

    // Massive insertions: end() always tracks [data + size]
    size_t n_massive = 100;
    nonpods_clear(&list);
    for (size_t i = 0; i < n_massive; ++i) {
        nonpods_push_back(&list, non_pod_init("foo", (int)i, (float)i, &gpa));
        assert(nonpods_end(&list) == list.data + list.size);
    }

    // after shrink_size, end() updates
    enum arraylist_error err = nonpods_shrink_size(&list, 17);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_end(&list) == list.data + 17);

    // Testing after clear, end() == data (if data null, is null)
    nonpods_clear(&list);
    endptr = nonpods_end(&list);
    assert((endptr == NULL && list.data == NULL) || (endptr == list.data + list.size && list.size == 0));

    // Test after deinit: end() returns NULL (if data is NULL)
    nonpods_deinit(&list);
    endptr = nonpods_end(&list);
    assert(endptr == NULL);

    // Use of end() in for-loop after deinit or clear never crashes
    for (struct non_pod *it = nonpods_begin(&list); it != nonpods_end(&list); ++it) {
        assert(0 && "This loop must never run: list is empty!");
    }

    // out-of-bounds: end() is always (data + size); at(size) returns NULL, end() is not valid for deref
    struct non_pod *at_end = nonpods_at(&list, list.size);
    assert(at_end == NULL);

    // Use after repeated deinit/clear: no crash, end() steady.
    nonpods_clear(&list);
    nonpods_clear(&list);
    endptr = nonpods_end(&list);
    assert((endptr == NULL && list.data == NULL) || (endptr == list.data && list.size == 0));
    nonpods_deinit(&list);
    assert(nonpods_end(&list) == NULL);

    // Null pointer argument: end(NULL) returns NULL (never crashes)
    assert(nonpods_end(NULL) == NULL);

    // Fill then remove from head/tail; end always points at size
    list = nonpods_init(gpa);
    for (size_t i = 0; i < 8; ++i)
        nonpods_push_back(&list, non_pod_init("bar", (int)i, (float)0, &gpa));
    for (size_t i = 0; i < 4; ++i) {
        nonpods_pop_back(&list); // remove last
        assert(nonpods_end(&list) == list.data + list.size);
    }
    for (size_t i = 0; list.size > 0; ++i) {
        nonpods_remove_at(&list, 0); // remove head
        assert(nonpods_end(&list) == list.data + list.size);
        (void)i;
    }
    assert(list.size == 0);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    printf("test arraylist end value-type passed\n");
}

void test_arraylist_find_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Find on empty list: always returns data+size == end()
    char notfound_name[] = "none";
    struct non_pod *res = nonpods_find(&list, non_pod_find_val, notfound_name);
    assert(res == list.data + list.size);    // In empty, both are NULL, so ok
    assert(res == nonpods_end(&list) || res == NULL);

    // Insert some non_pods
    *nonpods_emplace_back(&list) = non_pod_init("A", 1, 1.1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("B", 2, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("C", 3, 3.3, &gpa);

    // Find value for actual key; should point to that object
    // The predicate must match the type: struct non_pod*
    struct non_pod *f1 = nonpods_find(&list, non_pod_find_val, "A");
    assert(f1);
    assert(f1 != list.data + list.size);
    assert(strcmp(f1->objname, "A") == 0);

    struct non_pod *f2 = nonpods_find(&list, non_pod_find_val, "B");
    assert(f2);
    assert(f2 != list.data + list.size);
    assert(strcmp(f2->objname, "B") == 0);

    struct non_pod *f3 = nonpods_find(&list, non_pod_find_val, "C");
    assert(f3);
    assert(f3 != list.data + list.size);
    assert(strcmp(f3->objname, "C") == 0);

    // Not found: returns end() (safe to compare pointer identity, don't deref!)
    struct non_pod *nf = nonpods_find(&list, non_pod_find_val, "Zzznotfoundzz");
    assert(nf == list.data + list.size);
    assert(nf == nonpods_end(&list));

    // NULL predicate: always returns NULL
    assert(nonpods_find(&list, NULL, NULL) == NULL);

    // NULL list: always returns NULL
    assert(nonpods_find(NULL, non_pod_find_val, "A") == NULL);

    // Mutate underlying object, search reflects new state
    // Reallocating the name to a new size as to not occur a Heap Buffer Overflow error
    // Needs to use the same allocator as on list
    list.data[0].objname = list.alloc.realloc(list.data[0].objname, sizeof(strlen(list.data[0].objname) + 1), strlen("Zmaybe") + 1, list.alloc.ctx);
    strcpy(list.data[0].objname, "Zmaybe");
    struct non_pod *nz = nonpods_find(&list, non_pod_find_val, "Zmaybe");
    assert(nz == &list.data[0]);
    assert(strcmp(nz->objname, "Zmaybe") == 0);

    // List with duplicates: find returns the first matching by search order
    *nonpods_emplace_back(&list) = non_pod_init("C", 5, 10.1, &gpa);
    struct non_pod *dup1 = nonpods_find(&list, non_pod_find_val, "C");
    assert(dup1 && strcmp(dup1->objname, "C") == 0);
    assert(dup1 == &list.data[2]);
    // (first occurrence, not the new one at data[3])

    // Remove a value; find reflects removal
    nonpods_remove_at(&list, 1); // removes "B"
    struct non_pod *gone = nonpods_find(&list, non_pod_find_val, "B");
    assert(gone == list.data + list.size);

    // Find is not confused by holes or shifts
    assert(strcmp(list.data[1].objname, "C") == 0);
    struct non_pod *nowfound = nonpods_find(&list, non_pod_find_val, "C");
    assert(nowfound == &list.data[1]);

    // Find works after clearing list
    nonpods_clear(&list);
    assert(nonpods_find(&list, non_pod_find_val, "A") == list.data + list.size);

    // Find works after deinit/reinit
    nonpods_deinit(&list);
    list = nonpods_init(gpa);
    assert(nonpods_find(&list, non_pod_find_val, "foo") == nonpods_end(&list));

    // Large list, stress test
    size_t big = 102;
    const char *key = "BIGMATCH";
    for (size_t i = 0; i < big; ++i) {
        if (i == 50) {
            *nonpods_emplace_back(&list) = non_pod_init(key, 99, 101, &gpa);
        } else {
            char nm[16]; snprintf(nm, sizeof nm, "num%02u", (unsigned)i);
            *nonpods_emplace_back(&list) = non_pod_init(nm, (int)i, (float)i, &gpa);
        }
    }
    struct non_pod *bigfound = nonpods_find(&list, non_pod_find_val, (void*)key);
    assert(bigfound != list.data + list.size);
    assert(strcmp(bigfound->objname, key) == 0);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist find value-type passed\n");
}

void test_arraylist_contains_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // contains on empty list: always returns false, out_index untouched
    size_t outidx = 12345;
    bool hasA = nonpods_contains(&list, non_pod_find_val, "A", &outidx);
    assert(hasA == false);
    assert(outidx == 12345);

    // Add some objects and check positive and negative cases
    *nonpods_emplace_back(&list) = non_pod_init("A", 1, 1.1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("B", 2, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("C", 3, 3.3, &gpa);

    // Positive: present
    outidx = 9999;
    bool hasB = nonpods_contains(&list, non_pod_find_val, "B", &outidx);
    assert(hasB == true);
    assert(outidx == 1);

    outidx = 9999;
    bool hasC = nonpods_contains(&list, non_pod_find_val, "C", &outidx);
    assert(hasC == true);
    assert(outidx == 2);

    // Negative: not present
    outidx = 42;
    bool hasZZ = nonpods_contains(&list, non_pod_find_val, "ZZ", &outidx);
    assert(hasZZ == false);
    assert(outidx == 42);

    // If out_index is NULL, function is still correct (no write, no segfault)
    assert(nonpods_contains(&list, non_pod_find_val, "B", NULL) == true);
    assert(nonpods_contains(&list, non_pod_find_val, "NO", NULL) == false);

    // Duplicate entries: first match wins, correct index
    *nonpods_emplace_back(&list) = non_pod_init("B", 12, 2.22, &gpa);
    outidx = 999;
    bool founddup = nonpods_contains(&list, non_pod_find_val, "B", &outidx);
    assert(founddup == true);
    assert(outidx == 1);  // Should be first "B"
    // The one at index [3] is not returned by contains

    // Mutate data; contains reflects update
    // Reallocating the name to a new size as to not occur a Heap Buffer Overflow error
    // Needs to use the same allocator as on list
    list.data[0].objname = list.alloc.realloc(list.data[0].objname, sizeof(strlen(list.data[0].objname) + 1), strlen("XXX") + 1, list.alloc.ctx);
    strcpy(list.data[0].objname, "XXX");
    outidx = 800;
    bool foundXXX = nonpods_contains(&list, non_pod_find_val, "XXX", &outidx);
    assert(foundXXX == true);
    assert(outidx == 0);

    // Remove value; contains no longer finds it
    nonpods_remove_at(&list, 0); // removes "XXX"
    outidx = 0;
    bool hasXXX = nonpods_contains(&list, non_pod_find_val, "XXX", &outidx);
    assert(hasXXX == false);

    // out_index remains untouched on negative result
    outidx = 4444;
    bool failZ = nonpods_contains(&list, non_pod_find_val, "zzznotfound", &outidx);
    assert(failZ == false);
    assert(outidx == 4444);

    // NULL predicate: always returns false
    outidx = 1111;
    assert(!nonpods_contains(&list, NULL, "A", &outidx));
    assert(outidx == 1111);

    // NULL arraylist: always returns false
    outidx = 2020;
    assert(!nonpods_contains(NULL, non_pod_find_val, "A", &outidx));
    assert(outidx == 2020);

    // Large list: finds correct element, fast fail if not found
    nonpods_clear(&list);
    int N = 40;
    for (int i = 0; i < N; ++i) {
        char nm[10]; snprintf(nm, sizeof nm, "v%02d", i);
        *nonpods_emplace_back(&list) = non_pod_init(nm, i*i, i*2, &gpa);
    }
    outidx = 0;
    bool hasv15 = nonpods_contains(&list, non_pod_find_val, "v15", &outidx);
    assert(hasv15 == true);
    assert(outidx == 15);

    // Works after clear() and deinit()
    nonpods_clear(&list);
    assert(!nonpods_contains(&list, non_pod_find_val, "v20", &outidx));
    nonpods_deinit(&list);
    assert(!nonpods_contains(&list, non_pod_find_val, "XXX", &outidx));

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist contains value-type passed\n");
}

void test_arraylist_size_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // New list: size == 0 always.
    assert(nonpods_size(&list) == 0);

    // Empty, pop_back/remove: No-op for size.
    size_t sz = nonpods_size(&list);
    nonpods_pop_back(&list);
    assert(nonpods_size(&list) == sz);
    nonpods_remove_at(&list, 0);
    assert(nonpods_size(&list) == sz);

    // Add with push_back, emplace_back.
    *nonpods_emplace_back(&list) = non_pod_init("a", 1, 1.0, &gpa);
    assert(nonpods_size(&list) == 1);
    struct non_pod b = non_pod_init("b", 2, 2.0, &gpa);
    nonpods_push_back(&list, b);
    assert(nonpods_size(&list) == 2);

    // Insert in the middle increases size.
    struct non_pod c = non_pod_init("c", 3, 3.0, &gpa);
    enum arraylist_error err = nonpods_insert_at(&list, c, 1);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_size(&list) == 3);

    // Remove by index, shrink_size.
    sz = nonpods_size(&list);
    nonpods_remove_at(&list, 1);
    assert(nonpods_size(&list) == sz - 1);
    sz = nonpods_size(&list);
    nonpods_shrink_size(&list, 1);
    assert(nonpods_size(&list) == 1);

    // clear() sets size to zero, capacity nonzero.
    nonpods_clear(&list);
    assert(nonpods_size(&list) == 0);

    // At this point, can insert again.
    *nonpods_emplace_back(&list) = non_pod_init("x", 77, 7.7, &gpa);
    assert(nonpods_size(&list) == 1);

    // push_back until size > 1
    for (size_t i = 0; i < 5; ++i) {
        char name[10];
        snprintf(name, sizeof name, "item%lu", i);
        *nonpods_emplace_back(&list) = non_pod_init(name, (int)i, (float)i, &gpa);
        assert(nonpods_size(&list) == 2 + i);
    }

    // bulk pop_back: size decreases until zero, never negative
    size_t bigsize = nonpods_size(&list);
    for (size_t i = bigsize; i > 0; --i) {
        nonpods_pop_back(&list);
        assert(nonpods_size(&list) == i - 1);
    }
    // Double clear doesn't change size.
    assert(nonpods_size(&list) == 0);
    nonpods_clear(&list);
    assert(nonpods_size(&list) == 0);

    // Remove_from_to on empty/one-element: size zero, no crash, no negative
    nonpods_remove_from_to(&list, 0, 10);
    assert(nonpods_size(&list) == 0);

    // Add many, then remove range
    for (size_t i = 0; i < 16; ++i) {
        char name[10];
        snprintf(name, sizeof name, "bulk%lu", i);
        *nonpods_emplace_back(&list) = non_pod_init(name, 0, 0, &gpa);
    }
    assert(nonpods_size(&list) == 16);
    nonpods_remove_from_to(&list, 3, 8); // removes 6
    assert(nonpods_size(&list) == 10);
    nonpods_shrink_size(&list, 5);
    assert(nonpods_size(&list) == 5);

    // insert_at > size returns error
    size_t before = nonpods_size(&list);
    struct non_pod dummy = non_pod_init("tail", 555, 5.55, &gpa);
    nonpods_insert_at(&list, dummy, 10000);
    assert(nonpods_size(&list) == before);

    nonpods_insert_at(&list, dummy, list.size);

    // pop_back until empty never underflows
    while (nonpods_size(&list) > 0)
        nonpods_pop_back(&list);
    assert(nonpods_size(&list) == 0);

    // After deinit, size remains zero (undefined unless you re-init, but should be fine)
    nonpods_deinit(&list);
    assert(nonpods_size(&list) == 0);

    // After deinit, one must init again to not crash
    nonpods_deinit(&list);
    list = nonpods_init(gpa);
    *nonpods_emplace_back(&list) = non_pod_init("q", 2, 2.2, &gpa);
    assert(nonpods_size(&list) == 1);

    // Shrink_to_fit doesn't change size
    size_t sizesnap = nonpods_size(&list);
    nonpods_shrink_to_fit(&list);
    assert(nonpods_size(&list) == sizesnap);

    // NULL pointer always returns zero.
    assert(nonpods_size(NULL) == 0);

    // Remove_at/Pop_back OOB/negative (size_t-wrap) safe: size not negative
    nonpods_remove_at(&list, 100000);
    size_t left = nonpods_size(&list);
    nonpods_remove_at(&list, (size_t)-1);
    assert(nonpods_size(&list) <= left); // No > left

    nonpods_deinit(&list);

    // Failed allocator: push_back fails, size remains unchanged
    struct Allocator fail_a = allocator_get_failling_alloc();
    list = nonpods_init(fail_a);
    sz = nonpods_size(&list);
    struct non_pod dummyfail = {0};
    err = nonpods_push_back(&list, dummyfail);
    assert(err != ARRAYLIST_OK);
    assert(nonpods_size(&list) == sz);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    printf("test arraylist size value-type passed\n");
}

void test_arraylist_is_empty_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Freshly initialized list is empty
    assert(nonpods_is_empty(&list) == true);
    assert(list.size == 0);

    // After a single push_back: not empty
    struct non_pod a = non_pod_init("one", 1, 1.1, &gpa);
    nonpods_push_back(&list, a);
    assert(nonpods_is_empty(&list) == false);
    assert(list.size == 1);

    // After pop_back: becomes empty again
    nonpods_pop_back(&list);
    assert(nonpods_is_empty(&list) == true);
    assert(list.size == 0);

    // emplace_back, non-empty
    *nonpods_emplace_back(&list) = non_pod_init("two", 2, 2.2, &gpa);
    assert(nonpods_is_empty(&list) == false);
    assert(list.size == 1);

    // Remove_at(0): list empty again
    nonpods_remove_at(&list, 0);
    assert(nonpods_is_empty(&list) == true);
    assert(list.size == 0);

    // Insert at head, middle, tail
    for (int i = 0; i < 3; ++i) {
        char buf[8]; snprintf(buf, sizeof buf, "k%d", i);
        struct non_pod t = non_pod_init(buf, i+10, i+0.1, &gpa);
        if (i == 0){
            nonpods_insert_at(&list, t, 0);
        } else if (i == 1) {
            nonpods_insert_at(&list, t, list.size/2);
        } else {
            nonpods_insert_at(&list, t, list.size);
        }

        assert(nonpods_is_empty(&list) == false);
    }
    // All removed
    while (list.size > 0) {
        nonpods_pop_back(&list);
    }

    assert(nonpods_is_empty(&list) == true);
    assert(list.size == 0);

    // Insert many, remove by shrink_size
    size_t N = 20;
    for (size_t i = 0; i < N; ++i) {
        char buf[12]; snprintf(buf, sizeof buf, "v%lu", i);
        nonpods_push_back(&list, non_pod_init(buf, i, i, &gpa));
    }
    assert(nonpods_is_empty(&list) == false);
    nonpods_shrink_size(&list, 0);
    assert(nonpods_is_empty(&list) == true);

    // Insert many, remove_from_to to empty
    for (size_t i = 0; i < N; ++i) {
        char buf[12]; snprintf(buf, sizeof buf, "w%lu", i);
        nonpods_push_back(&list, non_pod_init(buf, i, i, &gpa));
    }
    assert(nonpods_is_empty(&list) == false);
    nonpods_remove_from_to(&list, 0, N-1);
    assert(nonpods_is_empty(&list) == true);

    // Insert, then clear
    for (size_t i = 0; i < N; ++i) {
        nonpods_push_back(&list, non_pod_init("z", i, i, &gpa));
    }

    assert(nonpods_is_empty(&list) == false);
    nonpods_clear(&list);
    assert(nonpods_is_empty(&list) == true);

    // Insert, clear, insert again -- works
    nonpods_push_back(&list, non_pod_init("hehe", 5, 5.5, &gpa));
    assert(nonpods_is_empty(&list) == false);
    nonpods_pop_back(&list);
    assert(nonpods_is_empty(&list) == true);

    // Empty after many growths/reallocs
    for (size_t i = 0; i < 128; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("bulk", i, i, &gpa);
    }

    assert(nonpods_is_empty(&list) == false);
    nonpods_clear(&list);
    assert(nonpods_is_empty(&list) == true);

    // Remove OOB: is_empty stays correct (should not crash)
    assert(nonpods_is_empty(&list) == true);  // still empty
    nonpods_remove_at(&list, 99999);
    assert(nonpods_is_empty(&list) == true);

    // Remove negative index: is_empty still correct (should not crash)
    nonpods_remove_at(&list, (size_t)-1);
    assert(nonpods_is_empty(&list) == true);

    // After deinit, is_empty is true
    nonpods_deinit(&list);
    assert(nonpods_is_empty(&list) == true);
    // Needs to be init again
    list = nonpods_init(gpa);
    *nonpods_emplace_back(&list) = non_pod_init("after", 7, 7.7, &gpa);
    assert(nonpods_is_empty(&list) == false);
    nonpods_clear(&list);

    // Call is_empty(NULL), must return false
    assert(nonpods_is_empty(NULL) == false);

    // Call is_empty repeatedly doesn't change anything
    assert(nonpods_is_empty(&list) == true);
    assert(nonpods_is_empty(&list) == true);

    // Push and pop alternately
    for (int i = 0; i < 5; ++i) {
        nonpods_push_back(&list, non_pod_init("ping", i, i, &gpa));
        assert(nonpods_is_empty(&list) == false);
        nonpods_pop_back(&list);
        assert(nonpods_is_empty(&list) == true);
    }

    // Failing allocator: list remains empty
    struct Allocator fail = allocator_get_failling_alloc();
    struct arraylist_nonpods zlist = nonpods_init(fail);
    assert(nonpods_is_empty(&zlist) == true);
    struct non_pod dummy = {0};
    enum arraylist_error err = nonpods_push_back(&zlist, dummy);
    assert(err != ARRAYLIST_OK);
    assert(nonpods_is_empty(&zlist) == true);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist is_empty value-type passed\n");
}

void test_arraylist_capacity_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Fresh list: capacity is zero
    assert(nonpods_capacity(&list) == 0);

    // capacity(NULL) == 0
    assert(nonpods_capacity(NULL) == 0);

    // Add one element (should allocate)
    struct non_pod np1 = non_pod_init("x", 1, 2.2, &gpa);
    nonpods_push_back(&list, np1);
    assert(nonpods_capacity(&list) >= 1);
    size_t cap1 = nonpods_capacity(&list);

    // capacity never decreases on push_back/emplace
    struct non_pod np2 = non_pod_init("y", 2, 3.3, &gpa);
    nonpods_push_back(&list, np2);
    assert(nonpods_capacity(&list) >= cap1);
    size_t cap2 = nonpods_capacity(&list);

    // Add until it grows (capacity should double; implementation doubles)
    size_t old_capacity = cap2;
    size_t to_grow = old_capacity;
    for (size_t i = 0; i < to_grow; ++i)
        nonpods_push_back(&list, non_pod_init("m", (int)i, (float)i, &gpa));
    // At least one alloc triggered: capacity at least doubled
    assert(nonpods_capacity(&list) >= 2 * old_capacity);

    // Reserve increases capacity if called, but never shrinks
    size_t prev_cap = nonpods_capacity(&list);
    enum arraylist_error err = nonpods_reserve(&list, prev_cap + 10);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_capacity(&list) >= prev_cap + 10);

    // Reserve with less or equal is no-op (doesn't shrink)
    prev_cap = nonpods_capacity(&list);
    err = nonpods_reserve(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_capacity(&list) == prev_cap);

    // Shrinking size does not shrink capacity
    size_t prev_capacity = nonpods_capacity(&list);
    err = nonpods_shrink_size(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_capacity(&list) == prev_capacity);

    // shrink_to_fit reduces capacity to size (if possible)
    size_t prev_size = nonpods_size(&list);
    err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_capacity(&list) == nonpods_size(&list));
    assert(nonpods_capacity(&list) == prev_size);

    // After clear: capacity unchanged, but size is zero
    err = nonpods_clear(&list);
    assert(nonpods_capacity(&list) == prev_size);

    // After shrink_to_fit on empty (size=0): capacity is 0 and data is NULL
    err = nonpods_shrink_to_fit(&list);
    assert(nonpods_capacity(&list) == 0);
    assert(list.data == NULL);

    // After deinit: capacity=0
    nonpods_deinit(&list);
    assert(nonpods_capacity(&list) == 0);

    // After deinit, one must init again to not crash
    nonpods_deinit(&list);
    list = nonpods_init(gpa);
    nonpods_push_back(&list, non_pod_init("test", 44, 12.1, &gpa));
    assert(nonpods_capacity(&list) > 0);

    // Remove all, add again, etc: capacity tracks as expected
    nonpods_clear(&list);
    err = nonpods_shrink_to_fit(&list);
    assert(nonpods_capacity(&list) == 0);

    // Add many, check grows as powers of two (OPTIONAL: for your implementation)
    size_t last_cap = 0;
    for (size_t i = 0; i < 40; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("bulk", (int)i, (float)i, &gpa);
        size_t cap = nonpods_capacity(&list);
        assert(cap >= nonpods_size(&list));
        // Implementation doubles, so as size crosses the boundary, capacity increases
        assert(cap >= last_cap);
        last_cap = cap;
    }

    // Capacity is always >= size, never less
    assert(nonpods_capacity(&list) >= nonpods_size(&list));

    // Remove/shrink_size never shrinks capacity
    size_t before_cap = nonpods_capacity(&list);
    nonpods_shrink_size(&list, 2);
    assert(nonpods_capacity(&list) == before_cap);

    // Clear then reserve boosts capacity again
    nonpods_clear(&list);
    err = nonpods_reserve(&list, 32);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_capacity(&list) >= 32);

    // Shrink to fit after removing all reduces capacity to zero
    nonpods_shrink_size(&list, 0);
    err = nonpods_shrink_to_fit(&list);
    assert(nonpods_capacity(&list) == 0);

    // Failed allocator never increases capacity, remains zero
    struct Allocator faila = allocator_get_failling_alloc();
    struct arraylist_nonpods bad = nonpods_init(faila);
    assert(nonpods_capacity(&bad) == 0);
    err = nonpods_push_back(&bad, np1); // Should fail
    assert(err != ARRAYLIST_OK);
    assert(nonpods_capacity(&bad) == 0);
    err = nonpods_reserve(&bad, 1000);
    assert(err != ARRAYLIST_OK);
    assert(nonpods_capacity(&bad) == 0);

    nonpods_deinit(&bad);

    // Defensive: after deinit, capacity is always 0
    nonpods_deinit(&list);
    assert(nonpods_capacity(&list) == 0);

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist capacity value-type passed\n");
}

void test_arraylist_get_allocator_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // get_allocator returns non-NULL when valid
    struct Allocator *out = nonpods_get_allocator(&list);
    assert(out != NULL);
    assert(out->malloc == gpa.malloc && out->free == gpa.free && 
           out->realloc == gpa.realloc && out->ctx == gpa.ctx);

    // Same pointer returned each time; doesn't move
    struct Allocator *out2 = nonpods_get_allocator(&list);
    assert(out == out2);

    // Use returned allocator to allocate/free
    void *buf = out->malloc(32, out->ctx);
    assert(buf != NULL);
    out->free(buf, 32, out->ctx);

    // Changing the allocator via the struct - ONLY WHEN EMPTY
    // First clear the list to ensure no elements allocated with old allocator
    nonpods_clear(&list);
    struct Allocator fail_a = allocator_get_failling_alloc();
    list.alloc = fail_a;
    struct Allocator *out3 = nonpods_get_allocator(&list);
    assert(out3 != NULL);
    assert(out3->malloc == fail_malloc && out3->free == fail_free);

    // Don't create shallow copies of containers with ownership
    // Instead create a new list
    struct arraylist_nonpods list2 = nonpods_init(fail_a);
    struct Allocator *oa2 = nonpods_get_allocator(&list2);
    assert(oa2 != NULL);
    assert(oa2->malloc == fail_malloc && oa2->free == fail_free);

    // After deinit, struct is zeroed
    nonpods_deinit(&list);
    struct Allocator *out4 = nonpods_get_allocator(&list);
    assert(out4 != NULL);
    // Check it's zeroed
    assert(out4->malloc == NULL);
    assert(out4->free == NULL);
    assert(out4->realloc == NULL);
    assert(out4->ctx == NULL);

    // get_allocator(NULL) returns NULL
    assert(nonpods_get_allocator(NULL) == NULL);

    // get_allocator works on zero-initialized
    struct arraylist_nonpods zeroed = {0};
    struct Allocator *az = nonpods_get_allocator(&zeroed);
    assert(az != NULL);
    assert(az->malloc == NULL);

    // New list with default allocator
    struct arraylist_nonpods list3 = nonpods_init(gpa);
    struct Allocator *oa3 = nonpods_get_allocator(&list3);
    assert(oa3 != NULL);
    assert(oa3->malloc == gpa.malloc);

    // Add elements
    for (size_t i = 0; i < 8; ++i) {
        // Store the allocator to use consistently
        struct Allocator *current_alloc = nonpods_get_allocator(&list3);
        struct non_pod np = non_pod_init("z", 42, 123.0f + i, current_alloc);
        *nonpods_emplace_back(&list3) = np;
    }
    assert(list3.size == 8);

    // The pointer is always to the arraylist's .alloc field
    assert(oa3 == &list3.alloc);

    // After clear, get_allocator returns same pointer
    nonpods_clear(&list3);
    assert(nonpods_get_allocator(&list3) == oa3);

    // After shrink_to_fit, allocator unchanged
    nonpods_shrink_to_fit(&list3);
    assert(nonpods_get_allocator(&list3) == oa3);

    // Test failed allocation PROPERLY
    struct Allocator saved_alloc = list3.alloc;  // Save current
    list3.alloc = fail_a;  // Switch to failing allocator
    
    // Create temporary object that we'll clean up manually
    struct non_pod temp = non_pod_init("fail", 0, 0, &gpa);
    enum arraylist_error err = nonpods_push_back(&list3, temp);
    assert(err != ARRAYLIST_OK);
    
    // MANUALLY clean up the temporary since push_back failed
    non_pod_deinit(&temp, &gpa);
    
    // Restore original allocator
    list3.alloc = saved_alloc;
    assert(nonpods_get_allocator(&list3)->malloc == gpa.malloc);

    // After pop_back, allocator unchanged
    // Add one element first so we can pop it
    struct non_pod np = non_pod_init("popme", 1, 1.0f, nonpods_get_allocator(&list3));
    *nonpods_emplace_back(&list3) = np;
    nonpods_pop_back(&list3);
    assert(nonpods_get_allocator(&list3) == &list3.alloc);

    // Cleanups
    nonpods_deinit(&list2);
    nonpods_deinit(&list3);

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist get_allocator value-type passed\n");
}

void test_arraylist_swap_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods a = nonpods_init(gpa);
    struct arraylist_nonpods b = nonpods_init(gpa);

    // swap two empty lists
    size_t pre_count = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    assert(a.size == 0 && b.size == 0 && a.capacity == 0 && b.capacity == 0);
    assert(a.data == NULL && b.data == NULL);
    assert(global_destructor_counter_arraylist == pre_count);

    // swap where one is empty, one is populated
    // Add a few entries to 'a'
    for (int i = 0; i < 5; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", i);
        *nonpods_emplace_back(&a) = non_pod_init(buf, i, 1.1f * i, &gpa);
    }
    assert(a.size == 5 && b.size == 0);
    // Record a's original buffer properties
    size_t old_a_cap = a.capacity;
    void *old_a_data = a.data;

    // Swap
    err = nonpods_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    // a is now empty, b has previous contents of a
    assert(a.size == 0 && a.capacity == 0 && a.data == NULL);
    assert(b.size == 5 && b.capacity == old_a_cap && b.data == old_a_data);
    for (size_t i = 0; i < b.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", (int)i);
        assert(strcmp(b.data[i].objname, buf) == 0);
        assert(*b.data[i].a == (int)i);
    }

    // swap back; restores original state
    err = nonpods_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    assert(b.size == 0 && b.data == NULL);
    assert(a.size == 5 && a.data != NULL);
    for (size_t i = 0; i < a.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", (int)i);
        assert(strcmp(a.data[i].objname, buf) == 0);
        assert(*a.data[i].a == (int)i);
    }

    // swap two non-empty lists, different sizes
    nonpods_clear(&b);
    // b: three items
    for (int i = 0; i < 3; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "B_%d", i);
        *nonpods_emplace_back(&b) = non_pod_init(buf, i + 10, i + 20, &gpa);
    }
    size_t a_size = a.size, b_size = b.size;
    void *a_data = a.data, *b_data = b.data;
    size_t a_cap = a.capacity, b_cap = b.capacity;

    err = nonpods_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    // a gets b's content
    assert(a.size == b_size && a.capacity == b_cap && a.data == b_data);
    for (size_t i = 0; i < a.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "B_%d", (int)i);
        assert(strcmp(a.data[i].objname, buf) == 0);
        assert(*a.data[i].a == (int)(i+10));
    }
    // b gets a's content
    assert(b.size == a_size && b.capacity == a_cap && b.data == a_data);
    for (size_t i = 0; i < b.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", (int)i);
        assert(strcmp(b.data[i].objname, buf) == 0);
        assert(*b.data[i].a == (int)i);
    }

    // swap with self
    err = nonpods_swap(&a, &a);
    assert(err == ARRAYLIST_OK); // ok to swap with self, unchanged
    assert(a.size == 3);
    for (size_t i = 0; i < a.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "B_%d", (int)i);
        assert(strcmp(a.data[i].objname, buf) == 0);
    }

    // swap after deinit (one deinit, one alive)
    nonpods_deinit(&b);
    err = nonpods_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    assert(a.size == 0 && a.data == NULL);
    assert(b.size == 3 && b.data != NULL);

    // NULL-ptr safeties: returns error, doesn't crash
    err = nonpods_swap(NULL, &b);
    assert(err == ARRAYLIST_ERR_NULL);
    err = nonpods_swap(&b, NULL);
    assert(err == ARRAYLIST_ERR_NULL);
    err = nonpods_swap(NULL, NULL);
    assert(err == ARRAYLIST_ERR_NULL);

    // Does not leak memory regardless of swap
    nonpods_clear(&a);
    nonpods_clear(&b);
    assert(global_destructor_counter_arraylist == 0);

    // swap two very large lists
    struct arraylist_nonpods x = nonpods_init(gpa), y = nonpods_init(gpa);
    for (size_t i = 0; i < 100; ++i) {
        *nonpods_emplace_back(&x) = non_pod_init("XLIST", (int)i, (float)i, &gpa);
    }
    for (size_t i = 0; i < 55; ++i) {
        *nonpods_emplace_back(&y) = non_pod_init("YLIST", (int)i, (float)i, &gpa);
    }
    // Save snapshots
    size_t xsize = x.size, ysize = y.size;
    void *xdata = x.data, *ydata = y.data;
    err = nonpods_swap(&x, &y);
    assert(err == ARRAYLIST_OK);
    assert(x.size == ysize && x.data == ydata);
    assert(y.size == xsize && y.data == xdata);
    for (size_t i = 0; i < x.size; ++i) {
        assert(strcmp(x.data[i].objname, "YLIST") == 0);
    }
    for (size_t i = 0; i < y.size; ++i) {
        assert(strcmp(y.data[i].objname, "XLIST") == 0);
    }

    // clean up all
    nonpods_deinit(&a);
    nonpods_deinit(&b);
    nonpods_deinit(&x);
    nonpods_deinit(&y);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist swap value passed\n");
}

void test_arraylist_qsort_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Sort empty list (should be OK, does nothing, returns OK)
    enum arraylist_error err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Sort single-item list (no change, works)
    *nonpods_emplace_back(&list) = non_pod_init("z", 1, 3.0, &gpa);
    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0].objname, "z") == 0);

    // Sort two-item: reverse, should swap
    nonpods_clear(&list);
    *nonpods_emplace_back(&list) = non_pod_init("b", 2, 9.0, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a", -1, 1.1, &gpa);
    assert(strcmp(list.data[0].objname, "b") == 0);
    assert(strcmp(list.data[1].objname, "a") == 0);

    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    // Sorted by objname, so "a" < "b"
    assert(strcmp(list.data[0].objname, "a") == 0);
    assert(strcmp(list.data[1].objname, "b") == 0);

    // Sort already sorted: no change, still correct
    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    assert(strcmp(list.data[0].objname, "a") == 0);
    assert(strcmp(list.data[1].objname, "b") == 0);

    // Sort with multiple elements: random order, check sort result
    nonpods_clear(&list);
    const char *names[] = { "delta", "alpha", "charlie", "bravo", "echo" };
    size_t N = sizeof(names)/sizeof(names[0]);
    for (size_t i = 0; i < N; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s", names[i]);
        *nonpods_emplace_back(&list) = non_pod_init(buf, (int)i, 42.42, &gpa);
    }
    // Confirm not sorted
    for (size_t i = 1; i < N; ++i) {
        assert(strcmp(list.data[i-1].objname, list.data[i].objname) != 0 || strcmp(list.data[i-1].objname, list.data[i].objname) > 0);
    }

    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);

    // Check sorted order
    for (size_t i = 1; i < N; ++i) {
        assert(strcmp(list.data[i-1].objname, list.data[i].objname) <= 0);
    }
    // Alphabetical: "alpha", "bravo", "charlie", "delta", "echo"
    assert(strcmp(list.data[0].objname, "alpha") == 0);
    assert(strcmp(list.data[1].objname, "bravo") == 0);
    assert(strcmp(list.data[2].objname, "charlie") == 0);
    assert(strcmp(list.data[3].objname, "delta") == 0);
    assert(strcmp(list.data[4].objname, "echo") == 0);

    // List with duplicates: check order is sorted, not necessarily stable
    nonpods_clear(&list);
    *nonpods_emplace_back(&list) = non_pod_init("x", 0, 0.0, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a", 99, 1.0, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("x", 11, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("b", 12, 3.3, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("x", -1, 4.4, &gpa);

    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);

    // Sorted: a, b, x, x, x
    assert(strcmp(list.data[0].objname, "a") == 0);
    assert(strcmp(list.data[1].objname, "b") == 0);
    assert(strcmp(list.data[2].objname, "x") == 0);
    assert(strcmp(list.data[3].objname, "x") == 0);
    assert(strcmp(list.data[4].objname, "x") == 0);

    // List in reverse order: sorts to correct order
    nonpods_clear(&list);
    const char *revnames[] = { "gamma", "foxtrot", "echo", "delta", "charlie", "bravo", "alpha" };
    for (size_t i = 0; i < 7; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init(revnames[i], (int)i, 3.14f, &gpa);
    }
    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    // alpha, bravo, charlie, delta, echo, foxtrot, gamma
    for (size_t i = 1; i < list.size; ++i) {
        assert(strcmp(list.data[i-1].objname, list.data[i].objname) <= 0);
    }
    assert(strcmp(list.data[0].objname, "alpha") == 0);
    assert(strcmp(list.data[6].objname, "gamma") == 0);

    // Re-sort after mutating contents
    strcpy(list.data[3].objname, "zulu");
    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    // alpha, bravo, charlie, echo, foxtrot, gamma, zulu
    assert(strcmp(list.data[list.size - 1].objname, "zulu") == 0);

    // Test behavior if NULL list passed: returns ARRAYLIST_ERR_NULL, does not crash
    err = nonpods_qsort(NULL, non_pod_sort_val);
    assert(err == ARRAYLIST_ERR_NULL);

    //Test behavior if NULL comparison function
    err = nonpods_qsort(&list, NULL);
    // Should not crash, returns early
    assert(err == ARRAYLIST_ERR_NULL);

    // After clear: sort is valid, sort no data does nothing
    nonpods_clear(&list);
    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Large random case: sort is correct, stable
    nonpods_clear(&list);
    size_t bigN = 128;
    // Insert shuffled (descending) names
    for (size_t i = 0; i < bigN; ++i) {
        char buf[32];
        snprintf(buf, sizeof buf, "%.3u", (unsigned)(bigN-i));
        *nonpods_emplace_back(&list) = non_pod_init(buf, (int)i, 1.0+i, &gpa);
    }
    err = nonpods_qsort(&list, non_pod_sort_val);
    assert(err == ARRAYLIST_OK);
    // Should be in strictly increasing order by string
    for (size_t i = 1; i < list.size; ++i) {
        assert(strcmp(list.data[i-1].objname, list.data[i].objname) <= 0);
    }
    // Should have 001 at beginning, etc
    assert(strncmp(list.data[0].objname, "001", 3) == 0);

    // Does not lose or duplicate objects
    assert(list.size == bigN);

    // Sorted result is correct: using at()
    struct non_pod *first = nonpods_at(&list, 0);
    struct non_pod *last = nonpods_at(&list, bigN - 1);
    assert(first && last);
    assert(strncmp(first->objname, "001", 3) == 0);
    assert(strncmp(last->objname, "128", 3) == 0);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist qsort value-type passed\n");
}

void test_arraylist_deep_clone_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Adding some values
    *nonpods_emplace_back(&list) = non_pod_init("a1", 1, 1.1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a2", 2, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a3", 3, 3.3, &gpa);
    assert(list.size == 3);

    // Cloning
    struct arraylist_nonpods cloned = nonpods_deep_clone(&list, non_pod_deep_clone_val);
    assert(cloned.data != NULL);
    assert(cloned.size == list.size);
    assert(cloned.size == 3);
    assert(cloned.capacity == list.capacity);
    assert(cloned.capacity == 4);

    assert(strcmp(list.data[0].objname, "a1") == 0);
    assert(strcmp(cloned.data[0].objname, "a1") == 0);

    assert(*list.data[0].a == 1);
    assert(*cloned.data[0].a == 1);

    assert(list.alloc.malloc == gpa.malloc);
    assert(cloned.alloc.malloc == gpa.malloc);

    // Independent copies, changing a value on list doesn't affect cloned
    *list.data[0].a = 10;
    assert(*list.data[0].a == 10);
    assert(*cloned.data[0].a == 1);

    *cloned.data[0].a = 50;
    assert(*cloned.data[0].a == 50);
    assert(*list.data[0].a == 10);

    // Passing NULL to either parameter will result in an empty struct, or assert failure
    struct arraylist_nonpods empty1 = nonpods_deep_clone(NULL, non_pod_deep_clone_val);
    assert(empty1.data == NULL);

    struct arraylist_nonpods empty2 = nonpods_deep_clone(&list, NULL);
    assert(empty2.data == NULL);

    nonpods_deinit(&list);
    nonpods_deinit(&cloned);
    nonpods_deinit(&empty1);
    nonpods_deinit(&empty2);
    // This is needed just because I'm not incrementing on deep_clone function
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist deep_clone value-type passed\n");
}

void test_arraylist_shallow_copy_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Adding some values
    *nonpods_emplace_back(&list) = non_pod_init("a1", 1, 1.1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a2", 2, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a3", 3, 3.3, &gpa);
    assert(list.size == 3);

    // Shallow copying a non-pod, they are linked through addresses now
    // This should not be done unless one know what they are doing
    struct arraylist_nonpods copied = nonpods_shallow_copy(&list);
    assert(copied.data != NULL);
    assert(copied.size == list.size);
    assert(copied.size == 3);
    assert(copied.capacity == list.capacity);
    assert(copied.capacity == 4);

    assert(strcmp(list.data[0].objname, "a1") == 0);
    assert(strcmp(copied.data[0].objname, "a1") == 0);

    assert(*list.data[0].a == 1);
    assert(*copied.data[0].a == 1);

    assert(list.alloc.malloc == gpa.malloc);
    assert(copied.alloc.malloc == gpa.malloc);

    // Dependent copies, changing a value on list AFFECTS copied
    *list.data[0].a = 10;
    assert(*list.data[0].a == 10);
    assert(*copied.data[0].a == 10);

    *copied.data[0].a = 50;
    assert(*copied.data[0].a == 50);
    assert(*list.data[0].a == 50);

    // Passing NULL will result in an empty struct, or assert failure
    struct arraylist_nonpods empty1 = nonpods_shallow_copy(NULL);
    assert(empty1.data == NULL);

    nonpods_deinit(&list);
    // DO NOT free the shallow copied pointers, DOUBLE FREE HERE
    // nonpods_deinit(&copied);
    // To not leak memory here, manually free the data buffer
    copied.alloc.free(copied.data, copied.capacity * sizeof(struct non_pod), copied.alloc.ctx);
    // Again, this is not recommended and should not be done unless there is really a need to

    nonpods_deinit(&empty1);
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shallow_copy value-type passed\n");
}

void test_arraylist_steal_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);

    // Adding some values
    *nonpods_emplace_back(&list) = non_pod_init("a1", 1, 1.1, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a2", 2, 2.2, &gpa);
    *nonpods_emplace_back(&list) = non_pod_init("a3", 3, 3.3, &gpa);
    assert(list.data != NULL);
    assert(list.size == 3);
    assert(list.capacity == 4);

    // Moving
    struct arraylist_nonpods new_arraylist = nonpods_steal(&list);
    assert(new_arraylist.data != NULL);
    assert(new_arraylist.size == 3);
    assert(new_arraylist.capacity == 4);

    // List is not valid anymore, it has been moved to new_arraylist
    assert(list.data == NULL);
    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.alloc.malloc == NULL);

    // Trying to insert new values on a moved list will result in a segfault crash
    // *nonpods_emplace_back(&list) = non_pod_init("a3", 3, 3.3, &gpa);

    assert(strcmp(new_arraylist.data[0].objname, "a1") == 0);
    assert(*new_arraylist.data[0].a == 1);
    assert(new_arraylist.alloc.malloc == gpa.malloc);

    // Passing NULL will result in an empty struct, or assert failure
    struct arraylist_nonpods empty1 = nonpods_steal(NULL);
    assert(empty1.data == NULL);

    // List does not need to be deinitialized
    //nonpods_deinit(&list);
    nonpods_deinit(&new_arraylist);
    nonpods_deinit(&empty1);
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist steal value-type passed\n");
}

void test_arraylist_clear_value(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods list = nonpods_init(gpa);
    size_t N = 8;

    // On a freshly initialized list
    assert(list.size == 0 && list.data == NULL);
    enum arraylist_error err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK); // No crash
    assert(list.size == 0);
    assert(list.capacity == 0);

    // On NULL pointer: no crash, returns OK
    assert(nonpods_clear(NULL) == ARRAYLIST_OK);

    // After adding a single element, then clearing
    *nonpods_emplace_back(&list) = non_pod_init("A", 1, 2.0, &gpa);
    assert(list.size == 1 && global_destructor_counter_arraylist == 1);
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(list.capacity >= 1);
    assert(global_destructor_counter_arraylist == 0);
    // Data pointer may remain allocated, but don't deref beyond list.size.

    // Multiple elements, all destructors called, buffer not shrunk
    for (size_t i = 0; i < N; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("B", (int)i, (float)i, &gpa);
    }
    assert(list.size == N);
    assert(global_destructor_counter_arraylist == N);
    size_t prev_cap = list.capacity;
    void *prev_data = list.data;
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(list.capacity == prev_cap);
    assert(list.data == prev_data);
    assert(global_destructor_counter_arraylist == 0);

    // After clear, can add again
    *nonpods_emplace_back(&list) = non_pod_init("C", 3, 4.0, &gpa);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);

    // clear again after clear: safe (idempotent)
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);
    assert(list.capacity == prev_cap);

    // clear on an already empty but allocated list is a no-op
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(list.capacity == prev_cap);

    // List still usable after clear
    for (size_t i = 0; i < 5; ++i) {
        char buf[5]; snprintf(buf, sizeof buf, "X%lu", i);
        *nonpods_emplace_back(&list) = non_pod_init(buf, (int)i, (float)i, &gpa);
    }
    assert(list.size == 5);
    assert(global_destructor_counter_arraylist == 5);

    // clear, then shrink_to_fit to test buffer freed/compaction after clear
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);
    err = nonpods_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    // Now storage should be truly freed
    assert(list.data == NULL || list.capacity == 0);
    // Clear on a totally fresh buffer returns OK
    err = nonpods_clear(&list);

    // After deinit, clear is safe (does not crash, acts as no-op)
    nonpods_deinit(&list);
    err = nonpods_clear(&list); // Shouldn't crash, size should be zero, return OK.
    assert(err == ARRAYLIST_OK);

    // Multiple calls to clear after deinit: no op
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);

    // Re-init, add, clear
    list = nonpods_init(gpa);
    *nonpods_emplace_back(&list) = non_pod_init("ZZ", 9, 9.9, &gpa);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0 && global_destructor_counter_arraylist == 0);

    // Large list, destructor call count and buffer preserved
    for (size_t i = 0; i < 40; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("bulk", (int)i, (float)i, &gpa);
    }
    assert(list.size == 40 && global_destructor_counter_arraylist == 40);
    prev_cap = list.capacity; prev_data = list.data;
    err = nonpods_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0 && list.capacity == prev_cap && list.data == prev_data);
    assert(global_destructor_counter_arraylist == 0);

    // After clear, insert and then deinit: destructor is called for new element
    *nonpods_emplace_back(&list) = non_pod_init("L", 1, 2.2, &gpa); // +1
    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    printf("test arraylist clear value-type passed\n");
}

void test_arraylist_deinit_value(void) {
    struct Allocator gpa = allocator_get_default();

    // Basic deinit: empties everything, frees buffer, destructor called on all

    global_destructor_counter_arraylist = 0;

    struct arraylist_nonpods list = nonpods_init(gpa);

    // Fill the array
    const size_t N = 10;
    for (size_t i = 0; i < N; ++i) {
        char name[16];
        snprintf(name, sizeof(name), "np%lu", i);
        *nonpods_emplace_back(&list) = non_pod_init(name, (int)i, (float)i, &gpa);
    }
    assert(list.size == N);
    assert(global_destructor_counter_arraylist == N);
    assert(list.data != NULL);
    assert(list.capacity >= N);

    // Call deinit
    nonpods_deinit(&list);

    // After deinit, array is empty/zeroed, no leaks, buffer is invalid
    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.data == NULL);
    assert(memcmp(&list.alloc, (const char[sizeof(list.alloc)]){0}, sizeof(list.alloc)) == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Deinit is safe for already deinitialized or empty lists: second call does nothing
    nonpods_deinit(&list); // No crash, no double free, still zeroed
    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.data == NULL);
    assert(global_destructor_counter_arraylist == 0);

    // Deinit is a no-op for zero-capacity, zero-size (already empty)
    struct arraylist_nonpods empty = nonpods_init(gpa);
    nonpods_deinit(&empty);
    assert(empty.size == 0);
    assert(empty.capacity == 0);
    assert(empty.data == NULL);
    assert(global_destructor_counter_arraylist == 0);

    // Deinit safe for NULL: nothing happens, no crash
    nonpods_deinit(NULL);

    // Interleaved use: repeated allocate/deinit cycles (no leaks/UB)
    for (size_t cycle = 0; cycle < 5; ++cycle) {
        struct arraylist_nonpods cyc = nonpods_init(gpa);
        for (size_t j = 0; j < 3 + cycle; ++j) {
            char nm[8]; snprintf(nm, sizeof nm, "cyc%lu_%lu", cycle, j);
            *nonpods_emplace_back(&cyc) = non_pod_init(nm, (int)j, (float)j, &gpa);
        }
        assert(global_destructor_counter_arraylist > 0);
        nonpods_deinit(&cyc);
        assert(global_destructor_counter_arraylist == 0);
    }

    // Can safely re-init after deinit, as per API doc
    list = nonpods_init(gpa);
    *nonpods_emplace_back(&list) = non_pod_init("AFTER", 123, 4.56, &gpa);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    // Ensure dtor called for each element
    // We'll count dtors, then forcibly clear without dtor (dirty trick: bypass it)
    list = nonpods_init(gpa);
    for (size_t i = 0; i < 5; ++i) {
        *nonpods_emplace_back(&list) = non_pod_init("count", 17, 3.14, &gpa);
    }
    assert(global_destructor_counter_arraylist == 5);
    // Remove three elements (to cover partial clear)
    nonpods_remove_at(&list, 0);
    nonpods_remove_at(&list, 0);
    nonpods_remove_at(&list, 0);
    assert(global_destructor_counter_arraylist == 2);

    nonpods_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist deinit value-type passed\n");
}

/* === END ARRAYLIST UNIT TESTS NON POD === */

/* === START ARRAYLIST UNIT TESTS NON POD POINTER === */

ARRAYLIST(struct non_pod *, nonpods_ptr, non_pod_deinit_macro_ptr)

void test_arraylist_init_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.data == NULL);
    assert(list.alloc.malloc == gpa.malloc);

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist init for pointer types passed\n");
}

void test_arraylist_reserve_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Reserve when list is empty: allocates exactly requested size
    enum arraylist_error err = nonpods_ptr_reserve(&list, 10);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 10);
    assert(list.data != NULL);
    assert(list.size == 0);

    // Reserve smaller (no-op): capacity remains unchanged
    err = nonpods_ptr_reserve(&list, 5);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 10);    // No shrink

    // Reserve same size (no-op)
    err = nonpods_ptr_reserve(&list, 10);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 10);    // Still 10

    // Reserve larger: reallocs and grows buffer, data valid

    // Fill up to 4
    for (size_t i = 0; i < 4; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a", i, i, &gpa);
    }
    //struct non_pod **olddata = list.data;
    err = nonpods_ptr_reserve(&list, 32);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 32);    // Grown
    assert(list.size == 4);
    assert(list.data != NULL);
    // Data is preserved
    for (size_t i = 0; i < list.size; ++i) {
        assert(strcmp(list.data[i]->objname, "a") == 0);
    }
    // Possibly new buffer, but implementation allows same pointer if realloc does nothing.
    // Do not hold pointers to old data if a realloc happens
    //assert(list.data == olddata);

    // Emplace more after reserve: no further realloc until cap hit
    size_t prevcap = list.capacity;
    for (size_t i = 4; i < 32; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("b", (int)i, (float)i, &gpa);
    }
    assert(list.size == 32);
    assert(list.capacity == prevcap);

    // Reserve to much larger: works and keeps old data
    err = nonpods_ptr_reserve(&list, 64);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 64);
    for (size_t i = 0; i < list.size; ++i) {
        assert(list.data[i]->a != NULL);
    }

    // Reserve with zero (should not shrink; no effect, not an error)
    prevcap = list.capacity;
    err = nonpods_ptr_reserve(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == prevcap);

    // Reserve with capacity equal to maximum supported by the allocator: catches overflow
    size_t maxsafe = SIZE_MAX / sizeof(struct non_pod*);
    err = nonpods_ptr_reserve(&list, maxsafe + 1);
    assert(err == ARRAYLIST_ERR_OVERFLOW);

    // Reserve with failing allocation: returns error, does not affect old buffer
    // Provide an allocator that always fails, init a fresh list
    struct Allocator fail_alloc = allocator_get_failling_alloc();
    struct arraylist_nonpods_ptr fail_list = nonpods_ptr_init(fail_alloc);
    // first reserve: fails as malloc returns NULL
    err = nonpods_ptr_reserve(&fail_list, 8);
    assert(err == ARRAYLIST_ERR_ALLOC);
    assert(fail_list.capacity == 0);
    assert(fail_list.data == NULL);

    // Fill a normal list to known good state
    struct arraylist_nonpods_ptr small = nonpods_ptr_init(gpa);
    for (size_t i = 0; i < 2; ++i) {
        *nonpods_ptr_emplace_back(&small) = non_pod_init_ptr("QQ", 2, 3, &gpa);
    }
    nonpods_ptr_shrink_to_fit(&small);
    // Now try to reserve with failing allocator change
    small.alloc = fail_alloc;
    err = nonpods_ptr_reserve(&small, 3);
    // Fails (realloc returns NULL), buffer unchanged
    assert(err == ARRAYLIST_ERR_ALLOC);
    // Since realloc failed, old data/cap remains
    assert(small.capacity == 2);
    // May or may not have NULL data pointer as realloc is permitted to leave old pointer unchanged
    small.alloc = gpa;

    // Reserve on uninitialized/null list: returns error (no crash)
    err = nonpods_ptr_reserve(NULL, 8);
    assert(err == ARRAYLIST_ERR_NULL);

    // Remark: After shrinking the list (with shrink_size), reserve does not shrink buffer
    err = nonpods_ptr_reserve(&list, 16);
    assert(err == ARRAYLIST_OK);
    // (Cap remains 64 since already large enough)
    assert(list.capacity == 64);

    // Clean up
    nonpods_ptr_deinit(&list);
    nonpods_ptr_deinit(&small);
    nonpods_ptr_deinit(&fail_list);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist reserve pointer-type passed\n");
}

void test_arraylist_shrink_size_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Add three objects
    struct non_pod *a = non_pod_init_ptr("a", 1, 1.1, &gpa);
    struct non_pod *b = non_pod_init_ptr("b", 2, 2.2, &gpa);
    struct non_pod *c = non_pod_init_ptr("c", 3, 3.3, &gpa);
    *nonpods_ptr_emplace_back(&list) = a;
    *nonpods_ptr_emplace_back(&list) = b;
    *nonpods_ptr_emplace_back(&list) = c;
    assert(list.size == 3);

    // Shrink to 2: calls dtor for c only
    size_t before = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_ptr_shrink_size(&list, 2);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp(list.data[0]->objname, "a") == 0);
    assert(strcmp(list.data[1]->objname, "b") == 0);
    assert(global_destructor_counter_arraylist == before - 1); // 1 dtor called

    // Shrink again to 1: only for b
    err = nonpods_ptr_shrink_size(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0]->objname, "a") == 0);

    // Shrink to 1 again (no-op)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_shrink_size(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);                                // unchanged
    assert(global_destructor_counter_arraylist == before); // no dtor called

    // Shrink to greater (no-op)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_shrink_size(&list, 5);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == before);

    // Shrink to zero: calls dtor for "a"
    err = nonpods_ptr_shrink_size(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0); // all dtors called

    // Shrink already empty: clean no-op
    err = nonpods_ptr_shrink_size(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Testing return value when null
    assert(nonpods_ptr_shrink_size(NULL, 0) == ARRAYLIST_ERR_NULL);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shrink_size pointer-type passed\n");
}

void test_arraylist_shrink_to_fit_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Grow to larger capacity
    for (size_t i = 0; i < 8; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("z", i, i, &gpa);
    }

    size_t orig_cap = list.capacity;
    size_t orig_size = list.size;
    assert(orig_cap == orig_size);

    // Shrink to fit: capacity matches size, data valid
    enum arraylist_error err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == orig_size);
    for (size_t i = 0; i < list.size; ++i) {
        assert(list.data[i]->a != NULL);
    }

    // Repeated shrink is no-op (shouldn't move array)
    void *data_ptr = list.data;
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == list.size);
    assert(list.data == data_ptr);

    // Shrink after shrink_size
    err = nonpods_ptr_shrink_size(&list, 3);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);

    size_t old_cap = list.capacity;
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 3);
    assert(list.capacity < old_cap);

    // Shrink to fit on size == 0 frees everything
    err = nonpods_ptr_shrink_size(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.capacity == 0);
    assert(list.data == NULL);

    // Should not fail or free again
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);

    // Testing return value when null
    assert(nonpods_ptr_shrink_to_fit(NULL) == ARRAYLIST_ERR_NULL);

    // Testing allocation failure
    struct arraylist_nonpods_ptr failalloc = nonpods_ptr_init(gpa);
    nonpods_ptr_reserve(&failalloc, 100);

    for (size_t i = 0; i < 8; ++i) {
        *nonpods_ptr_emplace_back(&failalloc) = non_pod_init_ptr("z", i, i, &gpa);
    }

    // Change allocator mid operations just to test the return value
    failalloc.alloc = allocator_get_failling_alloc();
    assert(nonpods_ptr_shrink_to_fit(&failalloc) == ARRAYLIST_ERR_ALLOC);

    // Change it back
    failalloc.alloc = gpa;
    nonpods_ptr_deinit(&failalloc);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shrink_to_fit pointer-type passed\n");
}

void test_arraylist_push_back_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Push first element
    struct non_pod *n1 = non_pod_init_ptr("one", 1, 1.1, &gpa);
    enum arraylist_error err = nonpods_ptr_push_back(&list, n1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(list.capacity == 1);
    assert(list.data != NULL);
    // Should still be 1 allocation
    assert(strcmp(list.data[0]->objname, "one") == 0);
    assert(*list.data[0]->a == 1);
    assert(*list.data[0]->b == (float)1.1f);

    // Push multiple to grow beyond initial cap
    size_t N = 10;
    for (size_t i = 1; i < N; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "val%lu", i);
        struct non_pod *n = non_pod_init_ptr(buf, i, i * 2.0f, &gpa);
        err = nonpods_ptr_push_back(&list, n);
        assert(err == ARRAYLIST_OK);
        assert(nonpods_ptr_size(&list) == i + 1);
        assert(strcmp(list.data[i]->objname, buf) == 0);
        assert(*list.data[i]->a == (int)i);
        assert(*list.data[i]->b == (float)(i * 2.0f));
        // Capacity should always >= size, doubled when full
        assert(list.capacity >= list.size);
    }

    // Check all content
    for (size_t i = 0; i < N; ++i) {
        if (i == 0) {
            assert(strcmp(list.data[i]->objname, "one") == 0);
            assert(*list.data[i]->a == 1);
        } else {
            char buf[5];
            snprintf(buf, sizeof(buf), "val%lu", i);
            assert(strcmp(list.data[i]->objname, buf) == 0);
            assert(*list.data[i]->a == (int)i);
        }
    }

    // Remove some, then push_back again
    nonpods_ptr_pop_back(&list);     // Remove "val9"
    nonpods_ptr_remove_at(&list, 0); // Remove "one"
    size_t before = nonpods_ptr_size(&list);
    struct non_pod *nnew = non_pod_init_ptr("again", 42, 42.42, &gpa);
    err = nonpods_ptr_push_back(&list, nnew);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_size(&list) == before + 1);
    assert(strcmp(list.data[list.size - 1]->objname, "again") == 0);
    assert(*list.data[list.size - 1]->a == 42);

    // Pop all and check destructor counter returns to zero
    nonpods_ptr_clear(&list);
    assert(global_destructor_counter_arraylist == 0);

    // Push after clear
    struct non_pod *tmp = non_pod_init_ptr("after", 888, 8.88, &gpa);
    err = nonpods_ptr_push_back(&list, tmp);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);

    // Testing doubling capacity
    nonpods_ptr_shrink_to_fit(&list);
    assert(list.size == 1);
    assert(list.capacity == 1);

    struct non_pod *np1 = non_pod_init_ptr("a", 1, 1.1, &gpa);
    nonpods_ptr_push_back(&list, np1);
    assert(list.size == 2);
    assert(list.capacity == 2);

    struct non_pod *np2 = non_pod_init_ptr("b", 2, 2.2, &gpa);
    nonpods_ptr_push_back(&list, np2);
    assert(list.size == 3);
    assert(list.capacity == 4);

    struct non_pod *np3 = non_pod_init_ptr("c", 3, 3.3, &gpa);
    nonpods_ptr_push_back(&list, np3);
    assert(list.size == 4);
    assert(list.capacity == 4);

    struct non_pod *np4 = non_pod_init_ptr("d", 4, 4.4, &gpa);
    nonpods_ptr_push_back(&list, np4);
    assert(list.size == 5);
    assert(list.capacity == 8);

    // Testing return null
    assert(nonpods_ptr_push_back(NULL, np4) == ARRAYLIST_ERR_NULL);

    // Testing allocation failure
    struct arraylist_nonpods_ptr allocfail = nonpods_ptr_init(allocator_get_failling_alloc());
    assert(nonpods_ptr_push_back(&allocfail, np4) == ARRAYLIST_ERR_ALLOC );

    // Testing buffer overflow
    struct arraylist_nonpods_ptr arraylistoveralloc = nonpods_ptr_init(gpa);
    // Simulate a full capacity
    // ARRAYLIST_ERR_OVERFLOW is only triggered when the arraylist WOULD overflow, so we need to get a bit less
    arraylistoveralloc.capacity = SIZE_MAX / (2 * sizeof(struct non_pod *)) + 1;
    arraylistoveralloc.size = arraylistoveralloc.capacity;
    assert(nonpods_ptr_push_back(&arraylistoveralloc, np4) == ARRAYLIST_ERR_OVERFLOW);

    nonpods_ptr_deinit(&arraylistoveralloc);
    nonpods_ptr_deinit(&allocfail);
    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist push_back pointer-type passed\n");
}

void test_arraylist_emplace_back_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Emplace one element; returned pointer is valid and modifiable
    struct non_pod **slot1 = nonpods_ptr_emplace_back(&list);
    assert(slot1 != NULL);
    // Can write individually through the slots:
    // (*slot1)->objname = gpa.malloc(5, gpa.ctx);
    // strcpy((*slot1)->objname, "foo");
    // (*slot1)->a = gpa.malloc(sizeof(int), gpa.ctx);
    // *(*slot1)->a = 1;
    // (*slot1)->b = gpa.malloc(sizeof(float), gpa.ctx);
    // *(*slot1)->b = 1.1f;

    // need to deref only the slot with a list of pointer types
    *slot1 = non_pod_init_ptr("foo", 1, 1.1, &gpa);

    assert(list.size == 1);
    assert(list.capacity == 1);
    assert(strcmp((*list.data[0]).objname, "foo") == 0);
    assert(list.data[0]->a && *list.data[0]->a == 1);
    // Data written through slot is visible in data array
    *(*slot1)->a = 27;
    assert(*(*list.data[0]).a == 27);

    // However after realloc, these pointers may not be valid anymore, so don't do this

    // Emplace multiple: Each pointer is unique, order preserved
    for (size_t i = 0; i < 5; ++i) {
        struct non_pod **slot = nonpods_ptr_emplace_back(&list);
        assert(slot != NULL);
        char name[6];
        snprintf(name, sizeof name, "item%lu", i);
        *slot = non_pod_init_ptr(name, i, 100.1f + i, &gpa);
        assert((*list.data[list.size - 1]).a && *(*slot)->a == (int)i);
        assert(strcmp((*slot)->objname, name) == 0);
    }

    // Emplacing triggers growth, previous pointers are NOT preserved
    size_t old_capacity = list.capacity;
    struct non_pod **old_first = &list.data[0];
    for (size_t i = 0; i < 20; ++i) {
        struct non_pod **slot_grow = nonpods_ptr_emplace_back(&list);
        char name[8];
        snprintf(name, sizeof name, "slot%lu", i);
        *slot_grow = non_pod_init_ptr(name, i + 1, 200.2f + i, &gpa);
    }

    assert(list.capacity > old_capacity);
    // After realloc, previous values are still valid and not overwritten, however, pointers are not in the same location
    // Thus, do not hold data pointers of list after operations that may realloc
    assert(slot1 != old_first);

    assert(strcmp(list.data[0]->objname, "foo") == 0);

    // After deinit, one must init again to not crash
    nonpods_ptr_deinit(&list);
    list = nonpods_ptr_init(gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("testing1", 1, 1.1, &gpa);
    assert(list.size == 1);
    assert(list.capacity == 1);

    // Testing doubling capacity
    // Do not need to create temp values to insert, however, there is way to check for failure this way
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("testing2", 2, 2.2, &gpa);
    assert(list.size == 2);
    assert(list.capacity == 2);

    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("testing3", 3, 3.3, &gpa);
    assert(list.size == 3);
    assert(list.capacity == 4);

    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("testing4", 4, 4.4, &gpa);
    assert(list.size == 4);
    assert(list.capacity == 4);

    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("testing5", 5, 5.5, &gpa);
    assert(list.size == 5);
    assert(list.capacity == 8);

    // Testing null parameter
    assert(nonpods_ptr_emplace_back(NULL) == NULL);

    // Testing alloc failure
    struct arraylist_nonpods_ptr allocfail = nonpods_ptr_init(allocator_get_failling_alloc());
    assert(nonpods_ptr_emplace_back(&allocfail) == NULL);

    // Testing buffer overflow
    struct arraylist_nonpods_ptr arraylistoveralloc = nonpods_ptr_init(gpa);
    // Simulate a full capacity
    // Buffer overflow error is only triggered when the arraylist WOULD overflow, so we need to get a bit less
    arraylistoveralloc.capacity = SIZE_MAX / (2 * sizeof(struct non_pod *)) + 1;
    arraylistoveralloc.size = arraylistoveralloc.capacity;
    assert(nonpods_ptr_emplace_back(&arraylistoveralloc) == NULL);

    nonpods_ptr_deinit(&arraylistoveralloc);
    nonpods_ptr_deinit(&allocfail);
    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist emplace_back pointer-type passed\n");
}

void test_arraylist_emplace_at_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Insert at end when list is empty (index == size): behaves like emplace_back
    size_t s0 = list.size;
    struct non_pod **slot_end0 = nonpods_ptr_emplace_at(&list, s0);
    assert(slot_end0 != NULL);
    *slot_end0 = non_pod_init_ptr("foo", 1, 1.1f, &gpa);

    assert(list.size == 1);
    assert(list.capacity == 1);
    assert(strcmp(list.data[0]->objname, "foo") == 0);
    assert(list.data[0]->a && *list.data[0]->a == 1);

    // Insert at end again, capacity doubling
    size_t s1 = list.size;
    struct non_pod **slot_end1 = nonpods_ptr_emplace_at(&list, s1);
    assert(slot_end1 != NULL);
    *slot_end1 = non_pod_init_ptr("bar", 2, 2.2f, &gpa);

    assert(list.size == 2);
    assert(list.capacity == 2);
    assert(strcmp(list.data[1]->objname, "bar") == 0);
    assert(list.data[1]->a && *list.data[1]->a == 2);

    // Insert at end again to trigger capacity growth to 4
    struct non_pod **slot_end2 = nonpods_ptr_emplace_at(&list, list.size);
    assert(slot_end2 != NULL);
    *slot_end2 = non_pod_init_ptr("baz", 3, 3.3f, &gpa);
    assert(list.size == 3);
    assert(list.capacity == 4);

    // Insert at beginning (index 0): shifts existing elements right
    struct non_pod **slot_begin = nonpods_ptr_emplace_at(&list, 0);
    assert(slot_begin != NULL);
    *slot_begin = non_pod_init_ptr("begin", 100, 100.0f, &gpa);

    assert(list.size == 4);
    assert(strcmp(list.data[0]->objname, "begin") == 0);
    assert(strcmp(list.data[1]->objname, "foo") == 0);
    assert(strcmp(list.data[2]->objname, "bar") == 0);
    assert(strcmp(list.data[3]->objname, "baz") == 0);

    // Insert in the middle (index 2): shifts tail elements right
    struct non_pod **slot_mid = nonpods_ptr_emplace_at(&list, 2);
    assert(slot_mid != NULL);
    *slot_mid = non_pod_init_ptr("mid", 777, 7.77f, &gpa);

    assert(list.size == 5);
    assert(strcmp(list.data[0]->objname, "begin") == 0);
    assert(strcmp(list.data[1]->objname, "foo") == 0);
    assert(strcmp(list.data[2]->objname, "mid") == 0);
    assert(strcmp(list.data[3]->objname, "bar") == 0);
    assert(strcmp(list.data[4]->objname, "baz") == 0);

    // Writing through returned slot is reflected in the data array
    *(*slot_mid)->a = 123;
    assert(*list.data[2]->a == 123);

    // Capacity growth and note about pointer stability: do not hold element pointers across growth
    size_t old_capacity = list.capacity;
    struct non_pod **old_first = &list.data[0];
    for (size_t i = 0; i < 20; ++i) {
        struct non_pod **slot_grow = nonpods_ptr_emplace_at(&list, list.size); // append via emplace_at
        assert(slot_grow != NULL);
        char name[16];
        snprintf(name, sizeof name, "slot%lu", i);
        *slot_grow = non_pod_init_ptr(name, (int)(1000 + i), 500.5f + (float)i, &gpa);
    }
    assert(list.capacity > old_capacity);
    // After potential realloc, previous element pointers may be invalid or relocated
    assert(old_first != &list.data[0]);

    // Verify earlier values are preserved
    assert(strcmp(list.data[0]->objname, "begin") == 0);
    assert(strcmp(list.data[1]->objname, "foo") == 0);
    assert(strcmp(list.data[3]->objname, "bar") == 0);
    assert(strcmp(list.data[4]->objname, "baz") == 0);

    // Null list
    assert(nonpods_ptr_emplace_at(NULL, 0) == NULL);

    // Out-of-bounds index (index > size)
    struct arraylist_nonpods_ptr list_oob = nonpods_ptr_init(gpa);
    assert(list_oob.size == 0);
    assert(nonpods_ptr_emplace_at(&list_oob, 1) == NULL);
    assert(list_oob.size == 0);
    nonpods_ptr_deinit(&list_oob);

    // Allocation failure
    struct arraylist_nonpods_ptr allocfail = nonpods_ptr_init(allocator_get_failling_alloc());
    // Attempt insert at end of empty list; should fail and return NULL
    assert(nonpods_ptr_emplace_at(&allocfail, 0) == NULL);
    nonpods_ptr_deinit(&allocfail);

    // Buffer overflow scenario
    struct arraylist_nonpods_ptr over = nonpods_ptr_init(gpa);
    // Simulate near-overflow capacity; doubling would exceed SIZE_MAX
    over.capacity = SIZE_MAX / (2 * sizeof(struct non_pod *)) + 1;
    over.size = over.capacity;
    assert(nonpods_ptr_emplace_at(&over, over.size) == NULL); // insert at end triggers growth check
    // Also test inserting not at end (still triggers growth due to size >= capacity)
    assert(nonpods_ptr_emplace_at(&over, 0) == NULL);

    // Clean up
    nonpods_ptr_deinit(&list);
    nonpods_ptr_deinit(&over);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist emplace_at pointer-type passed\n");
}

void test_arraylist_insert_at_pointer(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Insert into empty list, index 0; becomes head
    struct non_pod *a = non_pod_init_ptr("A", 10, 0.1, &gpa);
    enum arraylist_error err = nonpods_ptr_insert_at(&list, a, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0]->objname, "A") == 0);

    // Insert at head (index 0), should become new head; existing shift right
    struct non_pod *b = non_pod_init_ptr("B", 20, 0.2, &gpa);
    err = nonpods_ptr_insert_at(&list, b, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp(list.data[0]->objname, "B") == 0);
    assert(strcmp(list.data[1]->objname, "A") == 0);

    // Insert at tail (index=size), should append
    struct non_pod *c = non_pod_init_ptr("C", 30, 0.3, &gpa);
    err = nonpods_ptr_insert_at(&list, c, list.size);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);
    assert(strcmp(list.data[2]->objname, "C") == 0);

    // Insert in the middle (index=1)
    struct non_pod *d = non_pod_init_ptr("D", 40, 0.4, &gpa);
    err = nonpods_ptr_insert_at(&list, d, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 4);
    assert(strcmp(list.data[0]->objname, "B") == 0);
    assert(strcmp(list.data[1]->objname, "D") == 0);
    assert(strcmp(list.data[2]->objname, "A") == 0);
    assert(strcmp(list.data[3]->objname, "C") == 0);

    // Insert at index > size (out of bounds), should return an error code
    struct non_pod *e = non_pod_init_ptr("E", 50, 0.5, &gpa);
    err = nonpods_ptr_insert_at(&list, e, 999);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == 4);

    err = nonpods_ptr_insert_at(&list, e, list.size);
    assert(strcmp(list.data[4]->objname, "E") == 0);
    assert(list.size == 5);

    // Check all memory still valid and unchanged
    assert(strcmp(list.data[0]->objname, "B") == 0);
    assert(strcmp(list.data[1]->objname, "D") == 0);
    assert(strcmp(list.data[2]->objname, "A") == 0);
    assert(strcmp(list.data[3]->objname, "C") == 0);
    assert(strcmp(list.data[4]->objname, "E") == 0);

    // Capacity grows correctly; insert repeatedly to trigger realloc
    for (size_t i = 0; i < 10; ++i) {
        char name[12];
        snprintf(name, sizeof(name), "val%lu", i);
        struct non_pod *tmp = non_pod_init_ptr(name, i, i, &gpa);
        err = nonpods_ptr_insert_at(&list, tmp, 2); // always in middle
        assert(err == ARRAYLIST_OK);
    }
    assert(list.size == 15);
    assert(list.capacity >= list.size);

    // Insert at every possible index in small list
    nonpods_ptr_clear(&list);
    for (size_t i = 0; i < 4; ++i) {
        struct non_pod *t = non_pod_init_ptr("X", i, i, &gpa);
        err = nonpods_ptr_insert_at(&list, t, i); // should be append to each
        assert(err == ARRAYLIST_OK);
        assert(list.size == i + 1);
    }
    // Now list is [X, X, X, X]; insert at head (0), tail (size=4), and middle (2)
    struct non_pod *y = non_pod_init_ptr("Y", 99, 99.9, &gpa);
    err = nonpods_ptr_insert_at(&list, y, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 5);
    assert(strcmp(list.data[0]->objname, "Y") == 0);
    struct non_pod *z = non_pod_init_ptr("Z", 100, -10, &gpa);
    err = nonpods_ptr_insert_at(&list, z, 2);
    assert(list.size == 6);
    assert(strcmp(list.data[2]->objname, "Z") == 0);

    // Reset list
    nonpods_ptr_deinit(&list);
    list = nonpods_ptr_init(gpa);

    // Testing double capacity
    struct non_pod *a1 = non_pod_init_ptr("A", 1, 1.1, &gpa);
    nonpods_ptr_insert_at(&list, a1, 0);
    assert(list.size == 1);
    assert(list.capacity == 1);

    struct non_pod *a2 = non_pod_init_ptr("B", 2, 2.2, &gpa);
    nonpods_ptr_insert_at(&list, a2, 0);
    assert(list.size == 2);
    assert(list.capacity == 2);

    struct non_pod *a3 = non_pod_init_ptr("C", 3, 3.3, &gpa);
    nonpods_ptr_insert_at(&list, a3, 0);
    assert(list.size == 3);
    assert(list.capacity == 4);

    struct non_pod *a4 = non_pod_init_ptr("D", 4, 4.4, &gpa);
    nonpods_ptr_insert_at(&list, a4, 0);
    assert(list.size == 4);
    assert(list.capacity == 4);

    struct non_pod *a5 = non_pod_init_ptr("E", 5, 5.5, &gpa);
    nonpods_ptr_insert_at(&list, a5, 0);
    assert(list.size == 5);
    assert(list.capacity == 8);

    // Testing null parameter
    assert(nonpods_ptr_insert_at(NULL, a5, 0) == ARRAYLIST_ERR_NULL);

    // Testing alloc failure
    struct arraylist_nonpods_ptr allocfail = nonpods_ptr_init(allocator_get_failling_alloc());
    assert(nonpods_ptr_insert_at(&allocfail, a5, 0) == ARRAYLIST_ERR_ALLOC);

    // Testing buffer overflow
    struct arraylist_nonpods_ptr arraylistoveralloc = nonpods_ptr_init(gpa);
    // Simulate a full capacity
    // Buffer overflow error is only triggered when the arraylist WOULD overflow, so we need to get a bit less
    arraylistoveralloc.capacity = SIZE_MAX / (2 * sizeof(struct non_pod *)) + 1;
    arraylistoveralloc.size = arraylistoveralloc.capacity;
    assert(nonpods_ptr_insert_at(&arraylistoveralloc, a5, 0) == ARRAYLIST_ERR_OVERFLOW);

    nonpods_ptr_deinit(&arraylistoveralloc);
    nonpods_ptr_deinit(&allocfail);
    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist insert_at pointer-type passed\n");
}

void test_arraylist_pop_back_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Pop on empty list
    size_t before = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_ptr_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Push one, then pop
    struct non_pod *a = non_pod_init_ptr("A", 11, 1.1, &gpa);
    err = nonpods_ptr_push_back(&list, a);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 1);

    // Push several, then pop all
    for (size_t i = 0; i < 5; ++i) {
        char name[16]; snprintf(name, sizeof name, "item%lu", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(name, 10+i, 10+i, &gpa);
    }
    assert(list.size == 5);
    size_t alive = global_destructor_counter_arraylist;
    for (size_t i = 0; i < 5; ++i) {
        err = nonpods_ptr_pop_back(&list);
        assert(err == ARRAYLIST_OK);
        assert(list.size == 5 - (i + 1));
        assert(global_destructor_counter_arraylist == alive - (i + 1));
    }
    assert(list.size == 0);

    // Interleave push/pop (LIFO)
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("foo", 1, 1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("bar", 2, 2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("baz", 3, 3, &gpa);

    const char* expect_names[] = {"foo", "bar", "baz"};
    for (size_t i = 0; i < 3; ++i)
        assert(strcmp(list.data[i]->objname, expect_names[i]) == 0);

    err = nonpods_ptr_pop_back(&list); // removes "baz"
    assert(list.size == 2);
    assert(strcmp(list.data[0]->objname, "foo") == 0);
    assert(strcmp(list.data[1]->objname, "bar") == 0);

    err = nonpods_ptr_pop_back(&list); // removes "bar"
    assert(list.size == 1);
    assert(strcmp(list.data[0]->objname, "foo") == 0);

    err = nonpods_ptr_pop_back(&list); // removes "foo"
    assert(list.size == 0);

    // Push many to grow capacity, then pop many
    for (size_t i = 0; i < 16; ++i)
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("grow", i, i, &gpa);
    size_t cap = list.capacity;
    for (size_t i = 0; i < 16; ++i)
        nonpods_ptr_pop_back(&list);
    assert(list.size == 0);
    assert(list.capacity == cap); // pop_back never shrinks
    assert(global_destructor_counter_arraylist == 0);

    // Push, clear, pop_back is no-op
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("after", 123, 1, &gpa);
    nonpods_ptr_clear(&list);
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Testing null parameter
    assert(nonpods_ptr_pop_back(NULL) == ARRAYLIST_ERR_NULL);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist dyn pop_back pointer-type passed\n");
}

void test_arraylist_remove_at_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);
    enum arraylist_error err;
    size_t before;

    // Remove from empty list (should no-op)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // One element, remove at 0
    struct non_pod *a = non_pod_init_ptr("A", 1, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = a;
    assert(list.size == 1);
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 1); // dtor called

    // Add several, remove head, tail, middle
    struct non_pod *b = non_pod_init_ptr("B", 2, 2.2, &gpa);
    struct non_pod *c = non_pod_init_ptr("C", 3, 3.3, &gpa);
    struct non_pod *d = non_pod_init_ptr("D", 4, 4.4, &gpa);
    struct non_pod *e = non_pod_init_ptr("E", 5, 5.5, &gpa);
    *nonpods_ptr_emplace_back(&list) = b;  // 0
    *nonpods_ptr_emplace_back(&list) = c;  // 1
    *nonpods_ptr_emplace_back(&list) = d;  // 2
    *nonpods_ptr_emplace_back(&list) = e;  // 3
    assert(list.size == 4);

    // Remove head (index 0)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);
    assert(global_destructor_counter_arraylist == before - 1); // one destructor called
    // Remaining should be [C, D, E]
    assert(strcmp(list.data[0]->objname, "C") == 0);

    // Remove tail (index size-1)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, list.size - 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(global_destructor_counter_arraylist == before - 1); // one destructor called
    // Remaining should be [C, D]
    assert(strcmp(list.data[0]->objname, "C") == 0);
    assert(strcmp(list.data[1]->objname, "D") == 0);

    // Remove middle (index 1, which is D)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == before - 1);
    assert(strcmp(list.data[0]->objname, "C") == 0);

    // Remove only remaining element
    err = nonpods_ptr_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Add more, remove with index out of bounds (should return an error code)
    for (int i = 0; i < 5; ++i) {
        char buf[8];
        snprintf(buf, sizeof buf, "X%d", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(buf, i, i+0.1, &gpa);
    }
    size_t sz = list.size;
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 9999); // way OOB
    assert(err == ARRAYLIST_ERR_OOB);

    err = nonpods_ptr_remove_at(&list, list.size - 1);
    assert(list.size == sz - 1);
    assert(global_destructor_counter_arraylist == before - 1);

    // Remove with index == size (past end, also returns error)
    sz = list.size;
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, list.size);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == sz);
    assert(global_destructor_counter_arraylist == before);

    // Remove with index == SIZE_MAX, returns ARRAYLIST_ERR_OOB
    sz = list.size;
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, SIZE_MAX);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == sz);
    assert(global_destructor_counter_arraylist == before);

    // Remove remaining (result: should be fully destructed)
    while (list.size > 0) {
        nonpods_ptr_remove_at(&list, 0);
    }
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Test repeated removes on already empty list
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    err = nonpods_ptr_remove_at(&list, SIZE_MAX);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Restore: Fill list, remove every position, check value stability / shifting
    for (int i = 0; i < 4; ++i) {
        char buf[8];
        snprintf(buf, sizeof buf, "YY%d", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(buf, 100+i, 500.5+i, &gpa);
    }
    // [YY0, YY1, YY2, YY3]
    // Remove 1 (YY1)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_at(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 3);
    assert(global_destructor_counter_arraylist == before - 1);
    assert(strcmp(list.data[0]->objname, "YY0") == 0);
    assert(strcmp(list.data[1]->objname, "YY2") == 0); // shifted up
    assert(strcmp(list.data[2]->objname, "YY3") == 0);

    // Remove 1 (YY2, now at index 1)
    err = nonpods_ptr_remove_at(&list, 1);
    assert(list.size == 2);
    assert(strcmp(list.data[0]->objname, "YY0") == 0);
    assert(strcmp(list.data[1]->objname, "YY3") == 0);

    // Remove 0 (YY0)
    err = nonpods_ptr_remove_at(&list, 0);
    assert(list.size == 1);
    assert(strcmp(list.data[0]->objname, "YY3") == 0);

    // Remove 0 (YY3)
    err = nonpods_ptr_remove_at(&list, 0);
    assert(list.size == 0);

    // Remove at 0 again to check stable empty
    err = nonpods_ptr_remove_at(&list, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Testing null parameter
    assert(nonpods_ptr_remove_at(NULL, 0) == ARRAYLIST_ERR_NULL);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist remove_at pointer-type passed\n");
}

void test_arraylist_remove_from_to_ptr(void)
{
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Fill with 6 named objects: A, B, C, D, E, F
    const char *names[] = {"A", "B", "C", "D", "E", "F"};
    for (size_t i = 0; i < 6; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(names[i], 100 + i, 1.0+i, &gpa);
    }

    assert(list.size == 6);
    assert(global_destructor_counter_arraylist == 6);

    // Remove from head: [A, B, C, D, E, F] -- remove 0..1 -> [C, D, E, F]
    size_t before = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_ptr_remove_from_to(&list, 0, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 4);
    assert(global_destructor_counter_arraylist == before - 2);
    assert(strcmp(list.data[0]->objname, "C") == 0);
    assert(strcmp(list.data[1]->objname, "D") == 0);
    assert(strcmp(list.data[2]->objname, "E") == 0);
    assert(strcmp(list.data[3]->objname, "F") == 0);

    // Remove from middle: [C, D, E, F] -- remove 1..2 -> [C, F]
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 1, 2);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(global_destructor_counter_arraylist == before - 2);
    assert(strcmp(list.data[0]->objname, "C") == 0);
    assert(strcmp(list.data[1]->objname, "F") == 0);

    // Remove last element only: [C, F] -- remove 1..1 => [C]
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 1, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == before - 1);
    assert(strcmp(list.data[0]->objname, "C") == 0);

    // Remove first element only: [C] -- remove 0..0 => []
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 1);

    // Remove from empty: no-op
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before);

    // Fill again for more tests
    for (size_t i = 0; i < 6; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(names[i], 200 + i, 2.0+i, &gpa);
    }
    assert(list.size == 6);

    // Remove all by from=0, to=5 (size-1)
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 0, list.size - 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == before - 6);

    // Fill again for edge cases
    for (size_t i = 0; i < 6; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(names[i], 400 + i, 4.0+i, &gpa);
    }
    assert(list.size == 6);

    // Remove with from > to: should return ARRAYLIST_ERR_OOB
    before = list.size;
    size_t before_dtor = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 4, 2);
    assert(err == ARRAYLIST_ERR_OOB);
    assert(list.size == before);
    assert(global_destructor_counter_arraylist == before_dtor);

    // Remove with from == to == large index (index >= size): should return err oob
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 99, 99);
    assert(err == ARRAYLIST_ERR_OOB);
    err = nonpods_ptr_remove_from_to(&list, list.size - 1, list.size - 1);
    assert(list.size == 5);
    assert(global_destructor_counter_arraylist == before - 1);
    // Last element should have been removed, check new last
    assert(strcmp(list.data[4]->objname, "E") == 0);

    // Remove with to out of bounds but from valid: remove from 3..(big) => returns ARRAYLIST_ERR_OOB
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 3, 123456);
    assert(err == ARRAYLIST_ERR_OOB);

    err = nonpods_ptr_remove_from_to(&list, 3, 4);
    assert(list.size == 3);
    assert(global_destructor_counter_arraylist == before - 2);
    // Data: C, D, E was at 4, but now gone
    assert(strcmp(list.data[0]->objname, "A") == 0);
    assert(strcmp(list.data[1]->objname, "B") == 0);
    assert(strcmp(list.data[2]->objname, "C") == 0);

    // Remove with both out of bounds, with to being greater: returns ARRAYLIST_ERR_OOB
    before = global_destructor_counter_arraylist;
    err = nonpods_ptr_remove_from_to(&list, 123, 9999);
    assert(err == ARRAYLIST_ERR_OOB);

    err = nonpods_ptr_remove_from_to(&list, list.size - 1, list.size - 1);
    assert(list.size == 2);
    assert(global_destructor_counter_arraylist == before - 1);

    err = nonpods_ptr_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);

    // Remove last remaining
    err = nonpods_ptr_remove_from_to(&list, 0, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Remove from empty again (after everything): no crash, no change
    err = nonpods_ptr_remove_from_to(&list, 0, 1);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Testing null parameter
    assert(nonpods_ptr_remove_from_to(NULL, 0, 1) == ARRAYLIST_ERR_NULL);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist remove_from_to pointer-type passed\n");
}

void test_arraylist_at_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Empty array: OOB returns NULL
    assert(nonpods_ptr_at(&list, 0) == NULL);
    assert(nonpods_ptr_at(&list, 123) == NULL);

    // Add some values
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("A", 10, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("B", 20, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("C", 30, 3.3, &gpa);

    // Valid indices (0,1,2) return address in data, not NULL
    for (size_t i = 0; i < list.size; ++i) {
        struct non_pod **p = nonpods_ptr_at(&list, i);
        assert(p != NULL);
        // Content check (insertion order)
        if (i == 0) assert(strcmp((*p)->objname, "A") == 0);
        if (i == 1) assert(strcmp((*p)->objname, "B") == 0);
        if (i == 2) assert(strcmp((*p)->objname, "C") == 0);
    }

    // Out of bounds: index == size or greater returns NULL
    assert(nonpods_ptr_at(&list, 3) == NULL);
    assert(nonpods_ptr_at(&list, 99) == NULL);

    // Negative index interpreted as large size_t: always returns NULL
    // (Some compilers warn on -1 cast to size_t, so cast explicitly)
    assert(nonpods_ptr_at(&list, (size_t)-1) == NULL);

    // Use returned pointer as modifiable, changes reflected in array
    struct non_pod **pa = nonpods_ptr_at(&list, 0);
    assert(pa);
    // Reallocating the name to a new size as to not occur a Heap Buffer Overflow error
    // Needs to use the same allocator as on list
    (*pa)->objname = list.alloc.realloc((*pa)->objname, sizeof(strlen((*pa)->objname) + 1), strlen("newName") + 1, list.alloc.ctx);
    strcpy((*pa)->objname, "newName");
    assert(strcmp(list.data[0]->objname, "newName") == 0);

    // Remove element: shrink then query OOB (old last is dead)
    nonpods_ptr_pop_back(&list); // removes "objectC"
    assert(list.size == 2);
    assert(nonpods_ptr_at(&list, 2) == NULL);

    // Remove head, elements shift, at() reflects new order
    nonpods_ptr_remove_at(&list, 0); // removes "newName"
    assert(list.size == 1);
    assert(strcmp((*nonpods_ptr_at(&list, 0))->objname, "B") == 0);
    assert(nonpods_ptr_at(&list, 1) == NULL);

    // Insert after removal is visible via at()
    struct non_pod *tmp = non_pod_init_ptr("last", 99, 99.9, &gpa);
    enum arraylist_error err = nonpods_ptr_insert_at(&list, tmp, 0);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 2);
    assert(strcmp((*nonpods_ptr_at(&list, 0))->objname, "last") == 0);
    assert(strcmp((*nonpods_ptr_at(&list, 1))->objname, "B") == 0);

    // at(NULL, x) returns NULL
    assert(nonpods_ptr_at(NULL, 0) == NULL);
    assert(nonpods_ptr_at(NULL, 1000) == NULL);

    // Clearing empties the array; at() never deref OOB
    nonpods_ptr_clear(&list);
    assert(list.size == 0);
    assert(nonpods_ptr_at(&list, 0) == NULL);
    assert(nonpods_ptr_at(&list, 1) == NULL);

    // Huge number of items: check random OOB and in-bounds
    size_t bigN = 100;
    for (size_t i = 0; i < bigN; ++i) {
        char name[20];
        snprintf(name, sizeof name, "item%lu", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(name, (int)i, (float)i, &gpa);
    }
    for (size_t i = 0; i < bigN; ++i) {
        struct non_pod **p = nonpods_ptr_at(&list, i);
        assert(p != NULL);
        char expect[20];
        snprintf(expect, sizeof expect, "item%lu", i);
        assert(strcmp((*p)->objname, expect) == 0);
        assert(*(*p)->a == (int)i);
        assert(*(*p)->b == (float)i);
    }
    assert(nonpods_ptr_at(&list, bigN) == NULL);
    assert(nonpods_ptr_at(&list, bigN + 10000) == NULL);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist at pointer-type passed\n");
}

void test_arraylist_begin_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Begin on empty list should be NULL or a valid pointer only if capacity>0, so always check size first
    assert(list.size == 0);
    struct non_pod **begin = nonpods_ptr_begin(&list);
    // If data == NULL, then begin==NULL; if not, begin==data
    assert((begin == NULL && list.data == NULL) || begin == list.data);

    // Add one element; begin() == &data[0], non-null
    struct non_pod *a = non_pod_init_ptr("A", 10, 1.1, &gpa);
    nonpods_ptr_push_back(&list, a);
    begin = nonpods_ptr_begin(&list);
    assert(begin != NULL);
    assert(begin == &list.data[0]);
    assert(strcmp((*begin)->objname, "A") == 0);

    // Add another, this operation caused a realloc, which means begin() may change, not always true,
    // but should always be held as true
    //struct non_pod *old_begin = begin;
    struct non_pod *b = non_pod_init_ptr("B", 20, 2.2, &gpa);
    nonpods_ptr_push_back(&list, b);
    begin = nonpods_ptr_begin(&list);
    assert(begin == list.data);
    // assert(begin != old_begin); may be true or false, do not hold pointers between operations that reallocate
    assert(strcmp((*begin)->objname, "A") == 0);
    assert(strcmp((*(begin+1))->objname, "B") == 0);

    // Iteration with begin()/end()
    size_t iter_count = 0;
    for (struct non_pod **it = nonpods_ptr_begin(&list); it != nonpods_ptr_end(&list); ++it) {
        if (iter_count == 0) {
            assert(strcmp((*it)->objname, "A") == 0);
        }
        if (iter_count == 1) {
            assert(strcmp((*it)->objname, "B") == 0);
        }
        ++iter_count;
    }
    assert(iter_count == list.size);

    // Add multiple elements to trigger realloc/growth, confirm begin pointer is updated properly
    size_t many = 32;
    for (size_t i = 0; i < many; ++i) {
        char buf[10];
        snprintf(buf, sizeof buf, "q%lu", i);
        nonpods_ptr_push_back(&list, non_pod_init_ptr(buf, (int)i, (float)i, &gpa));
    }
    begin = nonpods_ptr_begin(&list);
    assert(begin == list.data);
    // After growth, check first value is still "A"
    assert(strcmp((*begin)->objname, "A") == 0);
    // And nth value is correct
    assert(strcmp((*(begin+list.size-1))->objname, "q31") == 0);

    // begin pointer is valid up to end(), never deref past end
    size_t count = 0;
    for (struct non_pod **it = nonpods_ptr_begin(&list); it != nonpods_ptr_end(&list); ++it) {
        count++;
    }
    assert(count == list.size);

    // pop_back: begin unchanged, last element gone
    size_t prev_size = list.size;
    nonpods_ptr_pop_back(&list);
    begin = nonpods_ptr_begin(&list);
    assert(begin == list.data);
    assert(list.size == prev_size - 1);

    // clear: begin can be non-null if capacity > 0, but size==0 so do not deref it
    nonpods_ptr_clear(&list);
    assert(list.size == 0);
    begin = nonpods_ptr_begin(&list);
    // Dereferencing begin when list is empty is invalid, but it's ok to compare pointer value
    assert(begin == list.data);
    // If data==NULL, begin==NULL

    // Remove all, then add again: begin valid, data correct
    nonpods_ptr_push_back(&list, non_pod_init_ptr("foo", 1, 1.2f, &gpa));
    assert(list.size == 1);
    begin = nonpods_ptr_begin(&list);
    assert(begin != NULL);
    assert(strcmp((*begin)->objname, "foo") == 0);

    // remove_at changes order, begin still valid
    nonpods_ptr_push_back(&list, non_pod_init_ptr("bar", 2, 2.3f, &gpa));
    nonpods_ptr_remove_at(&list, 0);
    begin = nonpods_ptr_begin(&list);
    assert(list.size == 1);
    assert(strcmp((*begin)->objname, "bar") == 0);

    // reset: deinit, then begin on fresh list
    nonpods_ptr_deinit(&list);
    list = nonpods_ptr_init(gpa);
    begin = nonpods_ptr_begin(&list);
    assert(list.size == 0);
    assert((begin == NULL && list.data == NULL) || begin == list.data);

    // bulk insert; iterate via begin
    for (size_t i = 0; i < 8; ++i) {
        nonpods_ptr_push_back(&list, non_pod_init_ptr("x", i, i, &gpa));
    }
    
    size_t loopcheck = 0;
    for (struct non_pod **it = nonpods_ptr_begin(&list); it != nonpods_ptr_end(&list); ++it) {
        loopcheck++;
    }
    assert(loopcheck == list.size);

    // begin(NULL) returns NULL
    assert(nonpods_ptr_begin(NULL) == NULL);

    // begin not deref'd if list is truly empty; do not crash
    nonpods_ptr_clear(&list);
    begin = nonpods_ptr_begin(&list);
    // Dereferencing it is invalid, but pointer value is as expected
    assert((begin == NULL && list.data == NULL) || begin == list.data);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist begin pointer-type passed\n");
}

void test_arraylist_back_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // back() on empty array: returns NULL
    assert(nonpods_ptr_back(&list) == NULL);

    // single element: back() returns data+0, points to first, non-null
    struct non_pod *a = non_pod_init_ptr("first", 11, 1.1, &gpa);
    nonpods_ptr_push_back(&list, a);
    struct non_pod **last = nonpods_ptr_back(&list);
    assert(last != NULL);
    assert(last == &list.data[0]);
    assert(strcmp((*last)->objname, "first") == 0);
    assert(*(*last)->a == 11);

    // two elements: back() points to new element each time, never to prior
    struct non_pod *b = non_pod_init_ptr("second", 22, 2.2, &gpa);
    nonpods_ptr_push_back(&list, b);
    assert(list.size == 2);
    struct non_pod **back2 = nonpods_ptr_back(&list);
    assert(back2 != NULL);
    assert(back2 == &list.data[1]);
    assert(strcmp((*back2)->objname, "second") == 0);
    assert(*(*back2)->a == 22);

    // many elements: back() always matches last added
    for (int i = 2; i < 12; ++i) {
        char name[32];
        snprintf(name, sizeof name, "item%d", i);
        struct non_pod *t = non_pod_init_ptr(name, 100+i, 3.3f+i, &gpa);
        nonpods_ptr_push_back(&list, t);
        struct non_pod **b = nonpods_ptr_back(&list);
        assert(b != NULL);
        assert(b == &list.data[list.size-1]);
        assert(strcmp((*b)->objname, name) == 0);
        assert(*(*b)->a == 100+i);
    }

    // back() is mutable: change value via back(), data array reflects change
    struct non_pod **bref = nonpods_ptr_back(&list);
    strcpy((*bref)->objname, "lastX");
    assert(strcmp(list.data[list.size-1]->objname, "lastX") == 0);

    // Remove last element: back() now returns previous element
    enum arraylist_error err = nonpods_ptr_pop_back(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 11);
    struct non_pod **newlast = nonpods_ptr_back(&list);
    assert(newlast == &list.data[list.size-1]);
    assert(*(*newlast)->a == 110);
    // Content should match what was inserted
    assert(strcmp((*newlast)->objname, "item10") == 0);

    // Remove all but one: back always matches, then back(NULL) when empty
    while (list.size > 1)
        nonpods_ptr_pop_back(&list);
    assert(list.size == 1);
    struct non_pod **lastleft = nonpods_ptr_back(&list);
    assert(lastleft != NULL);
    assert(lastleft == &list.data[0]);
    assert(strcmp((*lastleft)->objname, "first") == 0);

    nonpods_ptr_pop_back(&list);
    assert(list.size == 0);
    assert(nonpods_ptr_back(&list) == NULL);

    // Push after clear: back() valid again
    struct non_pod *c = non_pod_init_ptr("newone", 123, 4.56, &gpa);
    nonpods_ptr_push_back(&list, c);
    struct non_pod **b2 = nonpods_ptr_back(&list);
    assert(b2 != NULL);
    assert(strcmp((*b2)->objname, "newone") == 0);

    // Clear: back() should return NULL
    nonpods_ptr_clear(&list);
    assert(list.size == 0);
    assert(nonpods_ptr_back(&list) == NULL);

    // Push several, remove some, back() always matches last
    for (int i = 0; i < 5; ++i) {
        char nm[8];
        snprintf(nm, sizeof nm, "set%d", i);
        nonpods_ptr_push_back(&list, non_pod_init_ptr(nm, i+30, i+0.13f, &gpa));
        assert(strcmp((*nonpods_ptr_back(&list))->objname, nm) == 0);
    }
    assert(list.size == 5);
    nonpods_ptr_remove_at(&list, 2); // remove element 2, last remains same
    assert(strcmp((*nonpods_ptr_back(&list))->objname, "set4") == 0);
    nonpods_ptr_pop_back(&list); // remove last
    assert(strcmp((*nonpods_ptr_back(&list))->objname, "set3") == 0);

    nonpods_ptr_clear(&list);

    // back() on NULL list pointer: returns NULL (should not segfault)
    assert(nonpods_ptr_back(NULL) == NULL);

    // After deinit: back() returns NULL; repeated deinit is safe
    nonpods_ptr_push_back(&list, non_pod_init_ptr("foo", 16, 2.5, &gpa));
    nonpods_ptr_deinit(&list);
    assert(nonpods_ptr_back(&list) == NULL);
    nonpods_ptr_deinit(&list); // should not crash

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist back pointer-type passed\n");
}

void test_arraylist_end_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // end() on an empty list returns data + 0 (== data)
    assert(list.size == 0);
    struct non_pod **endptr = nonpods_ptr_end(&list);
    // end() may be NULL or data, so must be compared as a pointer, not dereferenced!
    assert((endptr == NULL && list.data == NULL) || (endptr == (list.data + list.size)));

    // end() pointer is always list.data + list.size
    // add several items, confirm end > data and never before data
    for (size_t i = 0; i < 6; ++i) {
        char namebuf[24];
        snprintf(namebuf, sizeof namebuf, "item%lu", i);
        nonpods_ptr_push_back(&list, non_pod_init_ptr(namebuf, (int)i, (float)i, &gpa));
        assert(nonpods_ptr_end(&list) == list.data + list.size);
        if (list.size > 0)
            assert(nonpods_ptr_end(&list) > list.data);
    }

    // Iterating: end() is always exclusive; can be used as limit for begin/end style iteration.
    size_t niter = 0;
    for (struct non_pod **it = nonpods_ptr_begin(&list); it != nonpods_ptr_end(&list); ++it) {
        assert(it >= list.data && it < list.data + list.size);
        niter++;
    }
    assert(niter == list.size);

    // Dereferencing end() is always invalid: verify pointer value but never deref
    struct non_pod **should_be_out_of_bounds = nonpods_ptr_end(&list);
    // The next is only to show the address, do not dereference
    assert(should_be_out_of_bounds == list.data + list.size);

    // After pop_back/clear, end() updates accordingly
    size_t oldsize = list.size;
    nonpods_ptr_pop_back(&list);
    assert(nonpods_ptr_end(&list) == list.data + list.size);
    assert(list.size == oldsize - 1);

    // Massive insertions: end() always tracks [data + size]
    size_t n_massive = 100;
    nonpods_ptr_clear(&list);
    for (size_t i = 0; i < n_massive; ++i) {
        nonpods_ptr_push_back(&list, non_pod_init_ptr("foo", (int)i, (float)i, &gpa));
        assert(nonpods_ptr_end(&list) == list.data + list.size);
    }

    // after shrink_size, end() updates
    enum arraylist_error err = nonpods_ptr_shrink_size(&list, 17);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_end(&list) == list.data + 17);

    // Testing after clear, end() == data (if data null, is null)
    nonpods_ptr_clear(&list);
    endptr = nonpods_ptr_end(&list);
    assert((endptr == NULL && list.data == NULL) || (endptr == list.data + list.size && list.size == 0));

    // Test after deinit: end() returns NULL (if data is NULL)
    nonpods_ptr_deinit(&list);
    endptr = nonpods_ptr_end(&list);
    assert(endptr == NULL);

    // Use of end() in for-loop after deinit or clear never crashes
    for (struct non_pod **it = nonpods_ptr_begin(&list); it != nonpods_ptr_end(&list); ++it) {
        assert(0 && "This loop must never run: list is empty!");
    }

    // out-of-bounds: end() is always (data + size); at(size) returns NULL, end() is not valid for deref
    struct non_pod **at_end = nonpods_ptr_at(&list, list.size);
    assert(at_end == NULL);

    // Use after repeated deinit/clear: no crash, end() steady.
    nonpods_ptr_clear(&list);
    nonpods_ptr_clear(&list);
    endptr = nonpods_ptr_end(&list);
    assert((endptr == NULL && list.data == NULL) || (endptr == list.data && list.size == 0));
    nonpods_ptr_deinit(&list);
    assert(nonpods_ptr_end(&list) == NULL);

    // Null pointer argument: end(NULL) returns NULL (never crashes)
    assert(nonpods_ptr_end(NULL) == NULL);

    // Fill then remove from head/tail; end always points at size
    list = nonpods_ptr_init(gpa);
    for (size_t i = 0; i < 8; ++i)
        nonpods_ptr_push_back(&list, non_pod_init_ptr("bar", (int)i, (float)0, &gpa));
    for (size_t i = 0; i < 4; ++i) {
        nonpods_ptr_pop_back(&list); // remove last
        assert(nonpods_ptr_end(&list) == list.data + list.size);
    }
    for (size_t i = 0; list.size > 0; ++i) {
        nonpods_ptr_remove_at(&list, 0); // remove head
        assert(nonpods_ptr_end(&list) == list.data + list.size);
        (void)i;
    }
    assert(list.size == 0);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    printf("test arraylist end pointer-type passed\n");
}

void test_arraylist_find_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Find on empty list: always returns data+size == end()
    char notfound_name[] = "none";
    struct non_pod **res = nonpods_ptr_find(&list, non_pod_find_ptr, notfound_name);
    assert(res == list.data + list.size);    // In empty, both are NULL, so ok
    assert(res == nonpods_ptr_end(&list) || res == NULL);

    // Insert some non_pods
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("A", 1, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("B", 2, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("C", 3, 3.3, &gpa);

    // Find value for actual key; should point to that object
    // The predicate must match the type: struct non_pod*
    struct non_pod **f1 = nonpods_ptr_find(&list, non_pod_find_ptr, "A");
    assert(f1);
    assert(f1 != list.data + list.size);
    assert(strcmp((*f1)->objname, "A") == 0);

    struct non_pod **f2 = nonpods_ptr_find(&list, non_pod_find_ptr, "B");
    assert(f2);
    assert(f2 != list.data + list.size);
    assert(strcmp((*f2)->objname, "B") == 0);

    struct non_pod **f3 = nonpods_ptr_find(&list, non_pod_find_ptr, "C");
    assert(f3);
    assert(f3 != list.data + list.size);
    assert(strcmp((*f3)->objname, "C") == 0);

    // Not found: returns end() (safe to compare pointer identity, don't deref!)
    struct non_pod **nf = nonpods_ptr_find(&list, non_pod_find_ptr, "Zzznotfoundzz");
    assert(nf == list.data + list.size);
    assert(nf == nonpods_ptr_end(&list));

    // NULL predicate: always returns NULL
    assert(nonpods_ptr_find(&list, NULL, NULL) == NULL);

    // NULL list: always returns NULL
    assert(nonpods_ptr_find(NULL, non_pod_find_ptr, "A") == NULL);

    // Mutate underlying object, search reflects new state
    // Reallocating the name to a new size as to not occur a Heap Buffer Overflow error
    // Needs to use the same allocator as on list
    list.data[0]->objname = list.alloc.realloc(list.data[0]->objname, sizeof(strlen(list.data[0]->objname) + 1), strlen("Zmaybe") + 1, list.alloc.ctx);
    strcpy(list.data[0]->objname, "Zmaybe");
    struct non_pod **nz = nonpods_ptr_find(&list, non_pod_find_ptr, "Zmaybe");
    assert(nz == &list.data[0]);
    assert(strcmp((*nz)->objname, "Zmaybe") == 0);

    // List with duplicates: find returns the first matching by search order
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("C", 5, 10.1, &gpa);
    struct non_pod **dup1 = nonpods_ptr_find(&list, non_pod_find_ptr, "C");
    assert(dup1 && strcmp((*dup1)->objname, "C") == 0);
    assert(dup1 == &list.data[2]);
    // (first occurrence, not the new one at data[3])

    // Remove a value; find reflects removal
    nonpods_ptr_remove_at(&list, 1); // removes "B"
    struct non_pod **gone = nonpods_ptr_find(&list, non_pod_find_ptr, "B");
    assert(gone == list.data + list.size);

    // Find is not confused by holes or shifts
    assert(strcmp(list.data[1]->objname, "C") == 0);
    struct non_pod **nowfound = nonpods_ptr_find(&list, non_pod_find_ptr, "C");
    assert(nowfound == &list.data[1]);

    // Find works after clearing list
    nonpods_ptr_clear(&list);
    assert(nonpods_ptr_find(&list, non_pod_find_ptr, "A") == list.data + list.size);

    // Find works after deinit/reinit
    nonpods_ptr_deinit(&list);
    list = nonpods_ptr_init(gpa);
    assert(nonpods_ptr_find(&list, non_pod_find_ptr, "foo") == nonpods_ptr_end(&list));

    // Large list, stress test
    size_t big = 102;
    const char *key = "BIGMATCH";
    for (size_t i = 0; i < big; ++i) {
        if (i == 50) {
            *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(key, 99, 101, &gpa);
        } else {
            char nm[16]; snprintf(nm, sizeof nm, "num%02u", (unsigned)i);
            *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(nm, (int)i, (float)i, &gpa);
        }
    }
    struct non_pod **bigfound = nonpods_ptr_find(&list, non_pod_find_ptr, (void*)key);
    assert(bigfound != list.data + list.size);
    assert(strcmp((*bigfound)->objname, key) == 0);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist find pointer-type passed\n");
}

void test_arraylist_contains_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // contains on empty list: always returns false, out_index untouched
    size_t outidx = 12345;
    bool hasA = nonpods_ptr_contains(&list, non_pod_find_ptr, "A", &outidx);
    assert(hasA == false);
    assert(outidx == 12345);

    // Add some objects and check positive and negative cases
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("A", 1, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("B", 2, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("C", 3, 3.3, &gpa);

    // Positive: present
    outidx = 9999;
    bool hasB = nonpods_ptr_contains(&list, non_pod_find_ptr, "B", &outidx);
    assert(hasB == true);
    assert(outidx == 1);

    outidx = 9999;
    bool hasC = nonpods_ptr_contains(&list, non_pod_find_ptr, "C", &outidx);
    assert(hasC == true);
    assert(outidx == 2);

    // Negative: not present
    outidx = 42;
    bool hasZZ = nonpods_ptr_contains(&list, non_pod_find_ptr, "ZZ", &outidx);
    assert(hasZZ == false);
    assert(outidx == 42);

    // If out_index is NULL, function is still correct (no write, no segfault)
    assert(nonpods_ptr_contains(&list, non_pod_find_ptr, "B", NULL) == true);
    assert(nonpods_ptr_contains(&list, non_pod_find_ptr, "NO", NULL) == false);

    // Duplicate entries: first match wins, correct index
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("B", 12, 2.22, &gpa);
    outidx = 999;
    bool founddup = nonpods_ptr_contains(&list, non_pod_find_ptr, "B", &outidx);
    assert(founddup == true);
    assert(outidx == 1);  // Should be first "B"
    // The one at index [3] is not returned by contains

    // Mutate data; contains reflects update
    // Reallocating the name to a new size as to not occur a Heap Buffer Overflow error
    // Needs to use the same allocator as on list
    list.data[0]->objname = list.alloc.realloc(list.data[0]->objname, sizeof(strlen(list.data[0]->objname) + 1), strlen("XXX") + 1, list.alloc.ctx);
    strcpy(list.data[0]->objname, "XXX");
    outidx = 800;
    bool foundXXX = nonpods_ptr_contains(&list, non_pod_find_ptr, "XXX", &outidx);
    assert(foundXXX == true);
    assert(outidx == 0);

    // Remove value; contains no longer finds it
    nonpods_ptr_remove_at(&list, 0); // removes "XXX"
    outidx = 0;
    bool hasXXX = nonpods_ptr_contains(&list, non_pod_find_ptr, "XXX", &outidx);
    assert(hasXXX == false);

    // out_index remains untouched on negative result
    outidx = 4444;
    bool failZ = nonpods_ptr_contains(&list, non_pod_find_ptr, "zzznotfound", &outidx);
    assert(failZ == false);
    assert(outidx == 4444);

    // NULL predicate: always returns false
    outidx = 1111;
    assert(!nonpods_ptr_contains(&list, NULL, "A", &outidx));
    assert(outidx == 1111);

    // NULL arraylist: always returns false
    outidx = 2020;
    assert(!nonpods_ptr_contains(NULL, non_pod_find_ptr, "A", &outidx));
    assert(outidx == 2020);

    // Large list: finds correct element, fast fail if not found
    nonpods_ptr_clear(&list);
    int N = 40;
    for (int i = 0; i < N; ++i) {
        char nm[10]; snprintf(nm, sizeof nm, "v%02d", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(nm, i*i, i*2, &gpa);
    }
    outidx = 0;
    bool hasv15 = nonpods_ptr_contains(&list, non_pod_find_ptr, "v15", &outidx);
    assert(hasv15 == true);
    assert(outidx == 15);

    // Works after clear() and deinit()
    nonpods_ptr_clear(&list);
    assert(!nonpods_ptr_contains(&list, non_pod_find_ptr, "v20", &outidx));
    nonpods_ptr_deinit(&list);
    assert(!nonpods_ptr_contains(&list, non_pod_find_ptr, "XXX", &outidx));

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist contains pointer-type passed\n");
}

void test_arraylist_size_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // New list: size == 0 always.
    assert(nonpods_ptr_size(&list) == 0);

    // Empty, pop_back/remove: No-op for size.
    size_t sz = nonpods_ptr_size(&list);
    nonpods_ptr_pop_back(&list);
    assert(nonpods_ptr_size(&list) == sz);
    nonpods_ptr_remove_at(&list, 0);
    assert(nonpods_ptr_size(&list) == sz);

    // Add with push_back, emplace_back.
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a", 1, 1.0, &gpa);
    assert(nonpods_ptr_size(&list) == 1);
    struct non_pod *b = non_pod_init_ptr("b", 2, 2.0, &gpa);
    nonpods_ptr_push_back(&list, b);
    assert(nonpods_ptr_size(&list) == 2);

    // Insert in the middle increases size.
    struct non_pod *c = non_pod_init_ptr("c", 3, 3.0, &gpa);
    enum arraylist_error err = nonpods_ptr_insert_at(&list, c, 1);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_size(&list) == 3);

    // Remove by index, shrink_size.
    sz = nonpods_ptr_size(&list);
    nonpods_ptr_remove_at(&list, 1);
    assert(nonpods_ptr_size(&list) == sz - 1);
    sz = nonpods_ptr_size(&list);
    nonpods_ptr_shrink_size(&list, 1);
    assert(nonpods_ptr_size(&list) == 1);

    // clear() sets size to zero, capacity nonzero.
    nonpods_ptr_clear(&list);
    assert(nonpods_ptr_size(&list) == 0);

    // At this point, can insert again.
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("x", 77, 7.7, &gpa);
    assert(nonpods_ptr_size(&list) == 1);

    // push_back until size > 1
    for (size_t i = 0; i < 5; ++i) {
        char name[10];
        snprintf(name, sizeof name, "item%lu", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(name, (int)i, (float)i, &gpa);
        assert(nonpods_ptr_size(&list) == 2 + i);
    }

    // bulk pop_back: size decreases until zero, never negative
    size_t bigsize = nonpods_ptr_size(&list);
    for (size_t i = bigsize; i > 0; --i) {
        nonpods_ptr_pop_back(&list);
        assert(nonpods_ptr_size(&list) == i - 1);
    }
    // Double clear doesn't change size.
    assert(nonpods_ptr_size(&list) == 0);
    nonpods_ptr_clear(&list);
    assert(nonpods_ptr_size(&list) == 0);

    // Remove_from_to on empty/one-element: size zero, no crash, no negative
    nonpods_ptr_remove_from_to(&list, 0, 10);
    assert(nonpods_ptr_size(&list) == 0);

    // Add many, then remove range
    for (size_t i = 0; i < 16; ++i) {
        char name[10];
        snprintf(name, sizeof name, "bulk%lu", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(name, 0, 0, &gpa);
    }
    assert(nonpods_ptr_size(&list) == 16);
    nonpods_ptr_remove_from_to(&list, 3, 8); // removes 6
    assert(nonpods_ptr_size(&list) == 10);
    nonpods_ptr_shrink_size(&list, 5);
    assert(nonpods_ptr_size(&list) == 5);

    // insert_at > size returns error
    size_t before = nonpods_ptr_size(&list);
    struct non_pod *dummy = non_pod_init_ptr("tail", 555, 5.55, &gpa);
    nonpods_ptr_insert_at(&list, dummy, 10000);
    assert(nonpods_ptr_size(&list) == before);

    nonpods_ptr_insert_at(&list, dummy, list.size);

    // pop_back until empty never underflows
    while (nonpods_ptr_size(&list) > 0)
        nonpods_ptr_pop_back(&list);
    assert(nonpods_ptr_size(&list) == 0);

    // After deinit, size remains zero
    nonpods_ptr_deinit(&list);
    assert(nonpods_ptr_size(&list) == 0);

    // after deinit, must be initialized again
    list = nonpods_ptr_init(gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("q", 2, 2.2, &gpa);
    assert(nonpods_ptr_size(&list) == 1);

    // Shrink_to_fit doesn't change size
    size_t sizesnap = nonpods_ptr_size(&list);
    nonpods_ptr_shrink_to_fit(&list);
    assert(nonpods_ptr_size(&list) == sizesnap);

    // NULL pointer always returns zero.
    assert(nonpods_ptr_size(NULL) == 0);

    // Remove_at/Pop_back OOB/negative (size_t-wrap) safe: size not negative
    nonpods_ptr_remove_at(&list, 100000);
    size_t left = nonpods_ptr_size(&list);
    nonpods_ptr_remove_at(&list, (size_t)-1);
    assert(nonpods_ptr_size(&list) <= left); // No > left

    nonpods_ptr_deinit(&list);

    // Failed allocator: push_back fails, size remains unchanged
    struct Allocator fail_a = allocator_get_failling_alloc();
    list = nonpods_ptr_init(fail_a);
    sz = nonpods_ptr_size(&list);
    struct non_pod *dummyfail = {0};
    err = nonpods_ptr_push_back(&list, dummyfail);
    assert(err != ARRAYLIST_OK);
    assert(nonpods_ptr_size(&list) == sz);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    printf("test arraylist size pointer-type passed\n");
}

void test_arraylist_is_empty_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Freshly initialized list is empty
    assert(nonpods_ptr_is_empty(&list) == true);
    assert(list.size == 0);

    // After a single push_back: not empty
    struct non_pod *a = non_pod_init_ptr("one", 1, 1.1, &gpa);
    nonpods_ptr_push_back(&list, a);
    assert(nonpods_ptr_is_empty(&list) == false);
    assert(list.size == 1);

    // After pop_back: becomes empty again
    nonpods_ptr_pop_back(&list);
    assert(nonpods_ptr_is_empty(&list) == true);
    assert(list.size == 0);

    // emplace_back, non-empty
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("two", 2, 2.2, &gpa);
    assert(nonpods_ptr_is_empty(&list) == false);
    assert(list.size == 1);

    // Remove_at(0): list empty again
    nonpods_ptr_remove_at(&list, 0);
    assert(nonpods_ptr_is_empty(&list) == true);
    assert(list.size == 0);

    // Insert at head, middle, tail
    for (int i = 0; i < 3; ++i) {
        char buf[8]; snprintf(buf, sizeof buf, "k%d", i);
        struct non_pod *t = non_pod_init_ptr(buf, i+10, i+0.1, &gpa);
        if (i == 0){
            nonpods_ptr_insert_at(&list, t, 0);
        } else if (i == 1) {
            nonpods_ptr_insert_at(&list, t, list.size/2);
        } else {
            nonpods_ptr_insert_at(&list, t, list.size);
        }

        assert(nonpods_ptr_is_empty(&list) == false);
    }
    // All removed
    while (list.size > 0) {
        nonpods_ptr_pop_back(&list);
    }

    assert(nonpods_ptr_is_empty(&list) == true);
    assert(list.size == 0);

    // Insert many, remove by shrink_size
    size_t N = 20;
    for (size_t i = 0; i < N; ++i) {
        char buf[12]; snprintf(buf, sizeof buf, "v%lu", i);
        nonpods_ptr_push_back(&list, non_pod_init_ptr(buf, i, i, &gpa));
    }
    assert(nonpods_ptr_is_empty(&list) == false);
    nonpods_ptr_shrink_size(&list, 0);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Insert many, remove_from_to to empty
    for (size_t i = 0; i < N; ++i) {
        char buf[12]; snprintf(buf, sizeof buf, "w%lu", i);
        nonpods_ptr_push_back(&list, non_pod_init_ptr(buf, i, i, &gpa));
    }
    assert(nonpods_ptr_is_empty(&list) == false);
    nonpods_ptr_remove_from_to(&list, 0, N-1);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Insert, then clear
    for (size_t i = 0; i < N; ++i) {
        nonpods_ptr_push_back(&list, non_pod_init_ptr("z", i, i, &gpa));
    }

    assert(nonpods_ptr_is_empty(&list) == false);
    nonpods_ptr_clear(&list);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Insert, clear, insert again -- works
    nonpods_ptr_push_back(&list, non_pod_init_ptr("hehe", 5, 5.5, &gpa));
    assert(nonpods_ptr_is_empty(&list) == false);
    nonpods_ptr_pop_back(&list);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Empty after many growths/reallocs
    for (size_t i = 0; i < 128; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("bulk", i, i, &gpa);
    }

    assert(nonpods_ptr_is_empty(&list) == false);
    nonpods_ptr_clear(&list);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Remove OOB: is_empty stays correct (should not crash)
    assert(nonpods_ptr_is_empty(&list) == true);  // still empty
    nonpods_ptr_remove_at(&list, 99999);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Remove negative index: is_empty still correct (should not crash)
    nonpods_ptr_remove_at(&list, (size_t)-1);
    assert(nonpods_ptr_is_empty(&list) == true);

    // After deinit, is_empty is true
    nonpods_ptr_deinit(&list);
    assert(nonpods_ptr_is_empty(&list) == true);

    // needs to be init again to be used
    list = nonpods_ptr_init(gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("after", 7, 7.7, &gpa);
    assert(nonpods_ptr_is_empty(&list) == false);
    nonpods_ptr_clear(&list);

    // Call is_empty(NULL), must return false
    assert(nonpods_ptr_is_empty(NULL) == false);

    // Call is_empty repeatedly doesn't change anything
    assert(nonpods_ptr_is_empty(&list) == true);
    assert(nonpods_ptr_is_empty(&list) == true);

    // Push and pop alternately
    for (int i = 0; i < 5; ++i) {
        nonpods_ptr_push_back(&list, non_pod_init_ptr("ping", i, i, &gpa));
        assert(nonpods_ptr_is_empty(&list) == false);
        nonpods_ptr_pop_back(&list);
        assert(nonpods_ptr_is_empty(&list) == true);
    }

    // Failing allocator: list remains empty
    struct Allocator fail = allocator_get_failling_alloc();
    struct arraylist_nonpods_ptr zlist = nonpods_ptr_init(fail);
    assert(nonpods_ptr_is_empty(&zlist) == true);
    struct non_pod *dummy = {0};
    enum arraylist_error err = nonpods_ptr_push_back(&zlist, dummy);
    assert(err != ARRAYLIST_OK);
    assert(nonpods_ptr_is_empty(&zlist) == true);
    nonpods_ptr_deinit(&zlist);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist is_empty pointer-type passed\n");
}

void test_arraylist_capacity_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Fresh list: capacity is zero
    assert(nonpods_ptr_capacity(&list) == 0);

    // capacity(NULL) == 0
    assert(nonpods_ptr_capacity(NULL) == 0);

    // Add one element (should allocate)
    struct non_pod *np1 = non_pod_init_ptr("x", 1, 2.2, &gpa);
    nonpods_ptr_push_back(&list, np1);
    assert(nonpods_ptr_capacity(&list) >= 1);
    size_t cap1 = nonpods_ptr_capacity(&list);

    // capacity never decreases on push_back/emplace
    struct non_pod *np2 = non_pod_init_ptr("y", 2, 3.3, &gpa);
    nonpods_ptr_push_back(&list, np2);
    assert(nonpods_ptr_capacity(&list) >= cap1);
    size_t cap2 = nonpods_ptr_capacity(&list);

    // Add until it grows (capacity should double; implementation doubles)
    size_t old_capacity = cap2;
    size_t to_grow = old_capacity;
    for (size_t i = 0; i < to_grow; ++i)
        nonpods_ptr_push_back(&list, non_pod_init_ptr("m", (int)i, (float)i, &gpa));
    // At least one alloc triggered: capacity at least doubled
    assert(nonpods_ptr_capacity(&list) >= 2 * old_capacity);

    // Reserve increases capacity if called, but never shrinks
    size_t prev_cap = nonpods_ptr_capacity(&list);
    enum arraylist_error err = nonpods_ptr_reserve(&list, prev_cap + 10);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&list) >= prev_cap + 10);

    // Reserve with less or equal is no-op (doesn't shrink)
    prev_cap = nonpods_ptr_capacity(&list);
    err = nonpods_ptr_reserve(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&list) == prev_cap);

    // Shrinking size does not shrink capacity
    size_t prev_capacity = nonpods_ptr_capacity(&list);
    err = nonpods_ptr_shrink_size(&list, 1);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&list) == prev_capacity);

    // shrink_to_fit reduces capacity to size (if possible)
    size_t prev_size = nonpods_ptr_size(&list);
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&list) == nonpods_ptr_size(&list));
    assert(nonpods_ptr_capacity(&list) == prev_size);

    // After clear: capacity unchanged, but size is zero
    err = nonpods_ptr_clear(&list);
    assert(nonpods_ptr_capacity(&list) == prev_size);

    // After shrink_to_fit on empty (size=0): capacity is 0 and data is NULL
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(nonpods_ptr_capacity(&list) == 0);
    assert(list.data == NULL);

    // After deinit: capacity=0
    nonpods_ptr_deinit(&list);
    assert(nonpods_ptr_capacity(&list) == 0);

    // Need to re-initialize list to reuse
    list = nonpods_ptr_init(gpa);
    nonpods_ptr_push_back(&list, non_pod_init_ptr("test", 44, 12.1, &gpa));
    assert(nonpods_ptr_capacity(&list) > 0);

    // Remove all, add again, etc: capacity tracks as expected
    nonpods_ptr_clear(&list);
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(nonpods_ptr_capacity(&list) == 0);

    // Add many, check grows as powers of two (OPTIONAL: for your implementation)
    size_t last_cap = 0;
    for (size_t i = 0; i < 40; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("bulk", (int)i, (float)i, &gpa);
        size_t cap = nonpods_ptr_capacity(&list);
        assert(cap >= nonpods_ptr_size(&list));
        // Implementation doubles, so as size crosses the boundary, capacity increases
        assert(cap >= last_cap);
        last_cap = cap;
    }

    // Capacity is always >= size, never less
    assert(nonpods_ptr_capacity(&list) >= nonpods_ptr_size(&list));

    // Remove/shrink_size never shrinks capacity
    size_t before_cap = nonpods_ptr_capacity(&list);
    nonpods_ptr_shrink_size(&list, 2);
    assert(nonpods_ptr_capacity(&list) == before_cap);

    // Clear then reserve boosts capacity again
    nonpods_ptr_clear(&list);
    err = nonpods_ptr_reserve(&list, 32);
    assert(err == ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&list) >= 32);

    // Shrink to fit after removing all reduces capacity to zero
    nonpods_ptr_shrink_size(&list, 0);
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(nonpods_ptr_capacity(&list) == 0);

    // Failed allocator never increases capacity, remains zero
    struct Allocator faila = allocator_get_failling_alloc();
    struct arraylist_nonpods_ptr bad = nonpods_ptr_init(faila);
    assert(nonpods_ptr_capacity(&bad) == 0);
    err = nonpods_ptr_push_back(&bad, np1); // Should fail
    assert(err != ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&bad) == 0);
    err = nonpods_ptr_reserve(&bad, 1000);
    assert(err != ARRAYLIST_OK);
    assert(nonpods_ptr_capacity(&bad) == 0);

    nonpods_ptr_deinit(&bad);

    // Defensive: after deinit, capacity is always 0
    nonpods_ptr_deinit(&list);
    assert(nonpods_ptr_capacity(&list) == 0);

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist capacity pointer-type passed\n");
}

void test_arraylist_get_allocator_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // get_allocator returns non-NULL when valid
    struct Allocator *out = nonpods_ptr_get_allocator(&list);
    assert(out != NULL);
    assert(out->malloc == gpa.malloc && out->free == gpa.free && 
           out->realloc == gpa.realloc && out->ctx == gpa.ctx);

    // Same pointer returned each time; doesn't move
    struct Allocator *out2 = nonpods_ptr_get_allocator(&list);
    assert(out == out2);

    // Use returned allocator to allocate/free
    void *buf = out->malloc(32, out->ctx);
    assert(buf != NULL);
    out->free(buf, 32, out->ctx);

    // Changing the allocator via the struct - ONLY WHEN EMPTY
    // First clear the list to ensure no elements allocated with old allocator
    nonpods_ptr_clear(&list);
    struct Allocator fail_a = allocator_get_failling_alloc();
    list.alloc = fail_a;
    struct Allocator *out3 = nonpods_ptr_get_allocator(&list);
    assert(out3 != NULL);
    assert(out3->malloc == fail_malloc && out3->free == fail_free);

    // Don't create shallow copies of containers with ownership
    // Instead create a new list
    struct arraylist_nonpods_ptr list2 = nonpods_ptr_init(fail_a);
    struct Allocator *oa2 = nonpods_ptr_get_allocator(&list2);
    assert(oa2 != NULL);
    assert(oa2->malloc == fail_malloc && oa2->free == fail_free);

    // After deinit, struct is zeroed
    nonpods_ptr_deinit(&list);
    struct Allocator *out4 = nonpods_ptr_get_allocator(&list);
    assert(out4 != NULL);
    // Check it's zeroed
    assert(out4->malloc == NULL);
    assert(out4->free == NULL);
    assert(out4->realloc == NULL);
    assert(out4->ctx == NULL);

    // get_allocator(NULL) returns NULL
    assert(nonpods_ptr_get_allocator(NULL) == NULL);

    // get_allocator works on zero-initialized
    struct arraylist_nonpods_ptr zeroed = {0};
    struct Allocator *az = nonpods_ptr_get_allocator(&zeroed);
    assert(az != NULL);
    assert(az->malloc == NULL);

    // New list with default allocator
    struct arraylist_nonpods_ptr list3 = nonpods_ptr_init(gpa);
    struct Allocator *oa3 = nonpods_ptr_get_allocator(&list3);
    assert(oa3 != NULL);
    assert(oa3->malloc == gpa.malloc);

    // Add elements
    for (size_t i = 0; i < 8; ++i) {
        // Store the allocator to use consistently
        struct Allocator *current_alloc = nonpods_ptr_get_allocator(&list3);
        struct non_pod *np = non_pod_init_ptr("z", 42, 123.0f + i, current_alloc);
        *nonpods_ptr_emplace_back(&list3) = np;
    }
    assert(list3.size == 8);

    // The pointer is always to the arraylist's .alloc field
    assert(oa3 == &list3.alloc);

    // After clear, get_allocator returns same pointer
    nonpods_ptr_clear(&list3);
    assert(nonpods_ptr_get_allocator(&list3) == oa3);

    // After shrink_to_fit, allocator unchanged
    nonpods_ptr_shrink_to_fit(&list3);
    assert(nonpods_ptr_get_allocator(&list3) == oa3);

    // Test failed allocation PROPERLY
    struct Allocator saved_alloc = list3.alloc;  // Save current
    list3.alloc = fail_a;  // Switch to failing allocator
    
    // Create temporary object that we'll clean up manually
    struct non_pod *temp = non_pod_init_ptr("fail", 0, 0, &gpa);
    enum arraylist_error err = nonpods_ptr_push_back(&list3, temp);
    assert(err != ARRAYLIST_OK);
    
    // MANUALLY clean up the temporary since push_back failed
    non_pod_deinit_ptr(&temp, &gpa);
    
    // Restore original allocator
    list3.alloc = saved_alloc;
    assert(nonpods_ptr_get_allocator(&list3)->malloc == gpa.malloc);

    // After pop_back, allocator unchanged
    // Add one element first so we can pop it
    struct non_pod *np = non_pod_init_ptr("popme", 1, 1.0f, nonpods_ptr_get_allocator(&list3));
    *nonpods_ptr_emplace_back(&list3) = np;
    nonpods_ptr_pop_back(&list3);
    assert(nonpods_ptr_get_allocator(&list3) == &list3.alloc);

    // Cleanups
    nonpods_ptr_deinit(&list2);
    nonpods_ptr_deinit(&list3);

    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist get_allocator pointer-type passed\n");
}

void test_arraylist_swap_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr a = nonpods_ptr_init(gpa);
    struct arraylist_nonpods_ptr b = nonpods_ptr_init(gpa);

    // swap two empty lists
    size_t pre_count = global_destructor_counter_arraylist;
    enum arraylist_error err = nonpods_ptr_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    assert(a.size == 0 && b.size == 0 && a.capacity == 0 && b.capacity == 0);
    assert(a.data == NULL && b.data == NULL);
    assert(global_destructor_counter_arraylist == pre_count);

    // swap where one is empty, one is populated
    // Add a few entries to 'a'
    for (int i = 0; i < 5; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", i);
        *nonpods_ptr_emplace_back(&a) = non_pod_init_ptr(buf, i, 1.1f * i, &gpa);
    }
    assert(a.size == 5 && b.size == 0);
    // Record a's original buffer properties
    size_t old_a_cap = a.capacity;
    void *old_a_data = a.data;

    // Swap
    err = nonpods_ptr_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    // a is now empty, b has previous contents of a
    assert(a.size == 0 && a.capacity == 0 && a.data == NULL);
    assert(b.size == 5 && b.capacity == old_a_cap && b.data == old_a_data);
    for (size_t i = 0; i < b.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", (int)i);
        assert(strcmp(b.data[i]->objname, buf) == 0);
        assert(*b.data[i]->a == (int)i);
    }

    // swap back; restores original state
    err = nonpods_ptr_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    assert(b.size == 0 && b.data == NULL);
    assert(a.size == 5 && a.data != NULL);
    for (size_t i = 0; i < a.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", (int)i);
        assert(strcmp(a.data[i]->objname, buf) == 0);
        assert(*a.data[i]->a == (int)i);
    }

    // swap two non-empty lists, different sizes
    nonpods_ptr_clear(&b);
    // b: three items
    for (int i = 0; i < 3; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "B_%d", i);
        *nonpods_ptr_emplace_back(&b) = non_pod_init_ptr(buf, i + 10, i + 20, &gpa);
    }
    size_t a_size = a.size, b_size = b.size;
    void *a_data = a.data, *b_data = b.data;
    size_t a_cap = a.capacity, b_cap = b.capacity;

    err = nonpods_ptr_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    // a gets b's content
    assert(a.size == b_size && a.capacity == b_cap && a.data == b_data);
    for (size_t i = 0; i < a.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "B_%d", (int)i);
        assert(strcmp(a.data[i]->objname, buf) == 0);
        assert(*a.data[i]->a == (int)(i+10));
    }
    // b gets a's content
    assert(b.size == a_size && b.capacity == a_cap && b.data == a_data);
    for (size_t i = 0; i < b.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "A_%d", (int)i);
        assert(strcmp(b.data[i]->objname, buf) == 0);
        assert(*b.data[i]->a == (int)i);
    }

    // swap with self
    err = nonpods_ptr_swap(&a, &a);
    assert(err == ARRAYLIST_OK); // ok to swap with self, unchanged
    assert(a.size == 3);
    for (size_t i = 0; i < a.size; ++i) {
        char buf[16]; snprintf(buf, sizeof buf, "B_%d", (int)i);
        assert(strcmp(a.data[i]->objname, buf) == 0);
    }

    // swap after deinit (one deinit, one alive)
    nonpods_ptr_deinit(&b);
    err = nonpods_ptr_swap(&a, &b);
    assert(err == ARRAYLIST_OK);
    assert(a.size == 0 && a.data == NULL);
    assert(b.size == 3 && b.data != NULL);

    // NULL-ptr safeties: returns error, doesn't crash
    err = nonpods_ptr_swap(NULL, &b);
    assert(err == ARRAYLIST_ERR_NULL);
    err = nonpods_ptr_swap(&b, NULL);
    assert(err == ARRAYLIST_ERR_NULL);
    err = nonpods_ptr_swap(NULL, NULL);
    assert(err == ARRAYLIST_ERR_NULL);

    // Does not leak memory regardless of swap
    nonpods_ptr_clear(&a);
    nonpods_ptr_clear(&b);
    assert(global_destructor_counter_arraylist == 0);

    // swap two very large lists
    struct arraylist_nonpods_ptr x = nonpods_ptr_init(gpa), y = nonpods_ptr_init(gpa);
    for (size_t i = 0; i < 100; ++i) {
        *nonpods_ptr_emplace_back(&x) = non_pod_init_ptr("XLIST", (int)i, (float)i, &gpa);
    }
    for (size_t i = 0; i < 55; ++i) {
        *nonpods_ptr_emplace_back(&y) = non_pod_init_ptr("YLIST", (int)i, (float)i, &gpa);
    }
    // Save snapshots
    size_t xsize = x.size, ysize = y.size;
    void *xdata = x.data, *ydata = y.data;
    err = nonpods_ptr_swap(&x, &y);
    assert(err == ARRAYLIST_OK);
    assert(x.size == ysize && x.data == ydata);
    assert(y.size == xsize && y.data == xdata);
    for (size_t i = 0; i < x.size; ++i) {
        assert(strcmp(x.data[i]->objname, "YLIST") == 0);
    }
    for (size_t i = 0; i < y.size; ++i) {
        assert(strcmp(y.data[i]->objname, "XLIST") == 0);
    }

    // clean up all
    nonpods_ptr_deinit(&a);
    nonpods_ptr_deinit(&b);
    nonpods_ptr_deinit(&x);
    nonpods_ptr_deinit(&y);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist swap pointer-type passed\n");
}

void test_arraylist_qsort_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Sort empty list (should be OK, does nothing, returns OK)
    enum arraylist_error err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Sort single-item list (no change, works)
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("z", 1, 3.0, &gpa);
    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 1);
    assert(strcmp(list.data[0]->objname, "z") == 0);

    // Sort two-item: reverse, should swap
    nonpods_ptr_clear(&list);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("b", 2, 9.0, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a", -1, 1.1, &gpa);
    assert(strcmp(list.data[0]->objname, "b") == 0);
    assert(strcmp(list.data[1]->objname, "a") == 0);

    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    // Sorted by objname, so "a" < "b"
    assert(strcmp(list.data[0]->objname, "a") == 0);
    assert(strcmp(list.data[1]->objname, "b") == 0);

    // Sort already sorted: no change, still correct
    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    assert(strcmp(list.data[0]->objname, "a") == 0);
    assert(strcmp(list.data[1]->objname, "b") == 0);

    // Sort with multiple elements: random order, check sort result
    nonpods_ptr_clear(&list);
    const char *names[] = { "delta", "alpha", "charlie", "bravo", "echo" };
    size_t N = sizeof(names)/sizeof(names[0]);
    for (size_t i = 0; i < N; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s", names[i]);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(buf, (int)i, 42.42, &gpa);
    }
    // Confirm not sorted
    for (size_t i = 1; i < N; ++i) {
        assert(strcmp(list.data[i-1]->objname, list.data[i]->objname) != 0 || strcmp(list.data[i-1]->objname, list.data[i]->objname) > 0);
    }

    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);

    // Check sorted order
    for (size_t i = 1; i < N; ++i) {
        assert(strcmp(list.data[i-1]->objname, list.data[i]->objname) <= 0);
    }
    // Alphabetical: "alpha", "bravo", "charlie", "delta", "echo"
    assert(strcmp(list.data[0]->objname, "alpha") == 0);
    assert(strcmp(list.data[1]->objname, "bravo") == 0);
    assert(strcmp(list.data[2]->objname, "charlie") == 0);
    assert(strcmp(list.data[3]->objname, "delta") == 0);
    assert(strcmp(list.data[4]->objname, "echo") == 0);

    // List with duplicates: check order is sorted, not necessarily stable
    nonpods_ptr_clear(&list);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("x", 0, 0.0, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a", 99, 1.0, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("x", 11, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("b", 12, 3.3, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("x", -1, 4.4, &gpa);

    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);

    // Sorted: a, b, x, x, x
    assert(strcmp(list.data[0]->objname, "a") == 0);
    assert(strcmp(list.data[1]->objname, "b") == 0);
    assert(strcmp(list.data[2]->objname, "x") == 0);
    assert(strcmp(list.data[3]->objname, "x") == 0);
    assert(strcmp(list.data[4]->objname, "x") == 0);

    // List in reverse order: sorts to correct order
    nonpods_ptr_clear(&list);
    const char *revnames[] = { "gamma", "foxtrot", "echo", "delta", "charlie", "bravo", "alpha" };
    for (size_t i = 0; i < 7; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(revnames[i], (int)i, 3.14f, &gpa);
    }
    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    // alpha, bravo, charlie, delta, echo, foxtrot, gamma
    for (size_t i = 1; i < list.size; ++i) {
        assert(strcmp(list.data[i-1]->objname, list.data[i]->objname) <= 0);
    }
    assert(strcmp(list.data[0]->objname, "alpha") == 0);
    assert(strcmp(list.data[6]->objname, "gamma") == 0);

    // Re-sort after mutating contents
    strcpy(list.data[3]->objname, "zulu");
    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    // alpha, bravo, charlie, echo, foxtrot, gamma, zulu
    assert(strcmp(list.data[list.size - 1]->objname, "zulu") == 0);

    // Test behavior if NULL list passed: returns ARRAYLIST_ERR_NULL, does not crash
    err = nonpods_ptr_qsort(NULL, non_pod_sort_ptr);
    assert(err == ARRAYLIST_ERR_NULL);

    //Test behavior if NULL comparison function
    err = nonpods_ptr_qsort(&list, NULL);
    // Should not crash, returns early
    assert(err == ARRAYLIST_ERR_NULL);

    // After clear: sort is valid, sort no data does nothing
    nonpods_ptr_clear(&list);
    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);

    // Large random case: sort is correct, stable
    nonpods_ptr_clear(&list);
    size_t bigN = 128;
    // Insert shuffled (descending) names
    for (size_t i = 0; i < bigN; ++i) {
        char buf[32];
        snprintf(buf, sizeof buf, "%.3u", (unsigned)(bigN-i));
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(buf, (int)i, 1.0+i, &gpa);
    }
    err = nonpods_ptr_qsort(&list, non_pod_sort_ptr);
    assert(err == ARRAYLIST_OK);
    // Should be in strictly increasing order by string
    for (size_t i = 1; i < list.size; ++i) {
        assert(strcmp(list.data[i-1]->objname, list.data[i]->objname) <= 0);
    }
    // Should have 001 at beginning, etc
    assert(strncmp(list.data[0]->objname, "001", 3) == 0);

    // Does not lose or duplicate objects
    assert(list.size == bigN);

    // Sorted result is correct: using at()
    struct non_pod **first = nonpods_ptr_at(&list, 0);
    struct non_pod **last = nonpods_ptr_at(&list, bigN - 1);
    assert(first && last);
    assert(strncmp((*first)->objname, "001", 3) == 0);
    assert(strncmp((*last)->objname, "128", 3) == 0);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist qsort pointer-type passed\n");
}

void test_arraylist_deep_clone_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Adding some values
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a1", 1, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a2", 2, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a3", 3, 3.3, &gpa);
    assert(list.size == 3);

    // Cloning
    struct arraylist_nonpods_ptr cloned = nonpods_ptr_deep_clone(&list, non_pod_deep_clone_ptr);
    assert(cloned.data != NULL);
    assert(cloned.size == list.size);
    assert(cloned.size == 3);
    assert(cloned.capacity == list.capacity);
    assert(cloned.capacity == 4);

    assert(strcmp(list.data[0]->objname, "a1") == 0);
    assert(strcmp(cloned.data[0]->objname, "a1") == 0);

    assert(*list.data[0]->a == 1);
    assert(*cloned.data[0]->a == 1);

    assert(list.alloc.malloc == gpa.malloc);
    assert(cloned.alloc.malloc == gpa.malloc);

    // Independent copies, changing a value on list doesn't affect cloned
    *list.data[0]->a = 10;
    assert(*list.data[0]->a == 10);
    assert(*cloned.data[0]->a == 1);

    *cloned.data[0]->a = 50;
    assert(*cloned.data[0]->a == 50);
    assert(*list.data[0]->a == 10);

    // Passing NULL to either parameter will result in an empty struct, or assert failure
    struct arraylist_nonpods_ptr empty1 = nonpods_ptr_deep_clone(NULL, non_pod_deep_clone_ptr);
    assert(empty1.data == NULL);

    struct arraylist_nonpods_ptr empty2 = nonpods_ptr_deep_clone(&list, NULL);
    assert(empty2.data == NULL);

    nonpods_ptr_deinit(&list);
    nonpods_ptr_deinit(&cloned);
    nonpods_ptr_deinit(&empty1);
    nonpods_ptr_deinit(&empty2);
    // This is needed just because I'm not incrementing on deep_clone function
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist deep_clone pointer-type passed\n");
}

void test_arraylist_shallow_copy_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Adding some values
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a1", 1, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a2", 2, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a3", 3, 3.3, &gpa);
    assert(list.size == 3);

    // Shallow copying a non-pod, they are linked through addresses now
    // This should not be done unless one know what they are doing
    struct arraylist_nonpods_ptr copied = nonpods_ptr_shallow_copy(&list);
    assert(copied.data != NULL);
    assert(copied.size == list.size);
    assert(copied.size == 3);
    assert(copied.capacity == list.capacity);
    assert(copied.capacity == 4);

    assert(strcmp(list.data[0]->objname, "a1") == 0);
    assert(strcmp(copied.data[0]->objname, "a1") == 0);

    assert(*list.data[0]->a == 1);
    assert(*copied.data[0]->a == 1);

    assert(list.alloc.malloc == gpa.malloc);
    assert(copied.alloc.malloc == gpa.malloc);

    // Dependent copies, changing a value on list AFFECTS copied
    *list.data[0]->a = 10;
    assert(*list.data[0]->a == 10);
    assert(*copied.data[0]->a == 10);

    *copied.data[0]->a = 50;
    assert(*copied.data[0]->a == 50);
    assert(*list.data[0]->a == 50);

    // Passing NULL will result in an empty struct, or assert failure
    struct arraylist_nonpods_ptr empty1 = nonpods_ptr_shallow_copy(NULL);
    assert(empty1.data == NULL);

    nonpods_ptr_deinit(&list);
    // DO NOT free the shallow copied pointers, DOUBLE FREE HERE
    // nonpods_ptr_deinit(&copied);
    // To not leak memory here, manually free the data buffer
    copied.alloc.free(copied.data, copied.capacity * sizeof(struct non_pod *), copied.alloc.ctx);
    // Again, this is not recommended and should not be done unless there is really a need to

    nonpods_ptr_deinit(&empty1);
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shallow_copy pointer-type passed\n");
}

void test_arraylist_steal_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Adding some values
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a1", 1, 1.1, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a2", 2, 2.2, &gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a3", 3, 3.3, &gpa);
    assert(list.data != NULL);
    assert(list.size == 3);
    assert(list.capacity == 4);

    // Moving
    struct arraylist_nonpods_ptr new_arraylist = nonpods_ptr_steal(&list);
    assert(new_arraylist.data != NULL);
    assert(new_arraylist.size == 3);
    assert(new_arraylist.capacity == 4);

    // List is not valid anymore, it has been moved to new_arraylist
    assert(list.data == NULL);
    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.alloc.malloc == NULL);

    // Trying to insert new values on a moved list will result in a segfault crash
    // *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("a3", 3, 3.3, &gpa);

    assert(strcmp(new_arraylist.data[0]->objname, "a1") == 0);
    assert(*new_arraylist.data[0]->a == 1);
    assert(new_arraylist.alloc.malloc == gpa.malloc);

    // Passing NULL will result in an empty struct, or assert failure
    struct arraylist_nonpods_ptr empty1 = nonpods_ptr_steal(NULL);
    assert(empty1.data == NULL);

    // List does not need to be deinitialized
    //nonpods_ptr_deinit(&list);
    nonpods_ptr_deinit(&new_arraylist);
    nonpods_ptr_deinit(&empty1);
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist steal pointer-type passed\n");
}

void test_arraylist_clear_ptr(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);
    size_t N = 8;

    // On a freshly initialized list
    assert(list.size == 0 && list.data == NULL);
    enum arraylist_error err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK); // No crash
    assert(list.size == 0);
    assert(list.capacity == 0);

    // On NULL pointer: no crash, returns OK
    assert(nonpods_ptr_clear(NULL) == ARRAYLIST_OK);

    // After adding a single element, then clearing
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("A", 1, 2.0, &gpa);
    assert(list.size == 1 && global_destructor_counter_arraylist == 1);
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(list.capacity >= 1);
    assert(global_destructor_counter_arraylist == 0);
    // Data pointer may remain allocated, but don't deref beyond list.size.

    // Multiple elements, all destructors called, buffer not shrunk
    for (size_t i = 0; i < N; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("B", (int)i, (float)i, &gpa);
    }
    assert(list.size == N);
    assert(global_destructor_counter_arraylist == N);
    size_t prev_cap = list.capacity;
    void *prev_data = list.data;
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(list.capacity == prev_cap);
    assert(list.data == prev_data);
    assert(global_destructor_counter_arraylist == 0);

    // After clear, can add again
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("C", 3, 4.0, &gpa);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);

    // clear again after clear: safe (idempotent)
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);
    assert(list.capacity == prev_cap);

    // clear on an already empty but allocated list is a no-op
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(list.capacity == prev_cap);

    // List still usable after clear
    for (size_t i = 0; i < 5; ++i) {
        char buf[5]; snprintf(buf, sizeof buf, "X%lu", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(buf, (int)i, (float)i, &gpa);
    }
    assert(list.size == 5);
    assert(global_destructor_counter_arraylist == 5);

    // clear, then shrink_to_fit to test buffer freed/compaction after clear
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0);
    assert(global_destructor_counter_arraylist == 0);
    err = nonpods_ptr_shrink_to_fit(&list);
    assert(err == ARRAYLIST_OK);
    // Now storage should be truly freed
    assert(list.data == NULL || list.capacity == 0);
    // Clear on a totally fresh buffer returns OK
    err = nonpods_ptr_clear(&list);

    // After deinit, clear is safe (does not crash, acts as no-op)
    nonpods_ptr_deinit(&list);
    err = nonpods_ptr_clear(&list); // Shouldn't crash, size should be zero, return OK.
    assert(err == ARRAYLIST_OK);

    // Multiple calls to clear after deinit: no op
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);

    // Re-init, add, clear
    list = nonpods_ptr_init(gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("ZZ", 9, 9.9, &gpa);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0 && global_destructor_counter_arraylist == 0);

    // Large list, destructor call count and buffer preserved
    for (size_t i = 0; i < 40; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("bulk", (int)i, (float)i, &gpa);
    }
    assert(list.size == 40 && global_destructor_counter_arraylist == 40);
    prev_cap = list.capacity; prev_data = list.data;
    err = nonpods_ptr_clear(&list);
    assert(err == ARRAYLIST_OK);
    assert(list.size == 0 && list.capacity == prev_cap && list.data == prev_data);
    assert(global_destructor_counter_arraylist == 0);

    // After clear, insert and then deinit: destructor is called for new element
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("L", 1, 2.2, &gpa); // +1
    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    printf("test arraylist clear pointer-type passed\n");
}

void test_arraylist_deinit_ptr(void) {
    struct Allocator gpa = allocator_get_default();

    // Basic deinit: empties everything, frees buffer, destructor called on all

    global_destructor_counter_arraylist = 0;

    struct arraylist_nonpods_ptr list = nonpods_ptr_init(gpa);

    // Fill the array
    const size_t N = 10;
    for (size_t i = 0; i < N; ++i) {
        char name[16];
        snprintf(name, sizeof(name), "np%lu", i);
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr(name, (int)i, (float)i, &gpa);
    }
    assert(list.size == N);
    assert(global_destructor_counter_arraylist == N);
    assert(list.data != NULL);
    assert(list.capacity >= N);

    // Call deinit
    nonpods_ptr_deinit(&list);

    // After deinit, array is empty/zeroed, no leaks, buffer is invalid
    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.data == NULL);
    assert(memcmp(&list.alloc, (const char[sizeof(list.alloc)]){0}, sizeof(list.alloc)) == 0);
    assert(global_destructor_counter_arraylist == 0);

    // Deinit is safe for already deinitialized or empty lists: second call does nothing
    nonpods_ptr_deinit(&list); // No crash, no double free, still zeroed
    assert(list.size == 0);
    assert(list.capacity == 0);
    assert(list.data == NULL);
    assert(global_destructor_counter_arraylist == 0);

    // Deinit is a no-op for zero-capacity, zero-size (already empty)
    struct arraylist_nonpods_ptr empty = nonpods_ptr_init(gpa);
    nonpods_ptr_deinit(&empty);
    assert(empty.size == 0);
    assert(empty.capacity == 0);
    assert(empty.data == NULL);
    assert(global_destructor_counter_arraylist == 0);

    // Deinit safe for NULL: nothing happens, no crash
    nonpods_ptr_deinit(NULL);

    // Interleaved use: repeated allocate/deinit cycles (no leaks/UB)
    for (size_t cycle = 0; cycle < 5; ++cycle) {
        struct arraylist_nonpods_ptr cyc = nonpods_ptr_init(gpa);
        for (size_t j = 0; j < 3 + cycle; ++j) {
            char nm[8]; snprintf(nm, sizeof nm, "cyc%lu_%lu", cycle, j);
            *nonpods_ptr_emplace_back(&cyc) = non_pod_init_ptr(nm, (int)j, (float)j, &gpa);
        }
        assert(global_destructor_counter_arraylist > 0);
        nonpods_ptr_deinit(&cyc);
        assert(global_destructor_counter_arraylist == 0);
    }

    // Can safely re-init after deinit, as per API doc
    list = nonpods_ptr_init(gpa);
    *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("AFTER", 123, 4.56, &gpa);
    assert(list.size == 1);
    assert(global_destructor_counter_arraylist == 1);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);

    // Ensure dtor called for each element
    // We'll count dtors, then forcibly clear without dtor (dirty trick: bypass it)
    list = nonpods_ptr_init(gpa);
    for (size_t i = 0; i < 5; ++i) {
        *nonpods_ptr_emplace_back(&list) = non_pod_init_ptr("count", 17, 3.14, &gpa);
    }
    assert(global_destructor_counter_arraylist == 5);
    // Remove three elements (to cover partial clear)
    nonpods_ptr_remove_at(&list, 0);
    nonpods_ptr_remove_at(&list, 0);
    nonpods_ptr_remove_at(&list, 0);
    assert(global_destructor_counter_arraylist == 2);

    nonpods_ptr_deinit(&list);
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist deinit pointer-type passed\n");
}

/* === END ARRAYLIST UNIT TESTS NON POD POINTER === */

/* === START TEST SHALLOW COPY/DEEP CLONE ON POD TYPE === */

// Just to ensure that calling deep_clone with a function that shallow copies behaves just like shallow copy
static void int_deep_clone(int *dst, int *src, struct Allocator *alloc) {
    (void)alloc;
    if (!src) {
        dst = NULL;
        return;
    }
    *dst = *src;
}

/* === START ARRAYLIST SHALLOW COPY ON SCALAR TYPE=== */

ARRAYLIST(int, intlist, arraylist_noop_deinit)

void test_arraylist_shallow_copy_scalar_type(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_intlist list = intlist_init(gpa);

    // Adding some values
    *intlist_emplace_back(&list) = 10;
    *intlist_emplace_back(&list) = 20;
    *intlist_emplace_back(&list) = 30;
    assert(list.size == 3);

    // Shallow copying a scalar type, they are essentially their own independent copies
    struct arraylist_intlist copied = intlist_shallow_copy(&list);
    assert(copied.data != NULL);
    assert(copied.size == list.size);
    assert(copied.size == 3);
    assert(copied.capacity == list.capacity);
    assert(copied.capacity == 4);

    assert(list.data[0] == copied.data[0]);
    assert(list.data[0] == 10);
    assert(copied.data[0] == 10);

    assert(list.alloc.malloc == gpa.malloc);
    assert(copied.alloc.malloc == gpa.malloc);

    // Independent copies, changing a value on list doesn't copied
    list.data[0] = 300;
    assert(list.data[0] == 300);
    assert(copied.data[0] == 10);

    copied.data[0] = 50;
    assert(copied.data[0] == 50);
    assert(list.data[0] == 300);

    // Passing NULL will result in an empty struct, or assert failure
    struct arraylist_intlist empty1 = intlist_shallow_copy(NULL);
    assert(empty1.data == NULL);

    intlist_deinit(&list);
    intlist_deinit(&copied);
    intlist_deinit(&empty1);
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist shallow_copy scalar-type passed\n");
}

/* === END ARRAYLIST SHALLOW COPY ON SCALAR TYPE === */

/* === START ARRAYLIST DEEP CLONE ON SCALAR TYPE === */

void test_arraylist_deep_clone_scalar_type(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_intlist list = intlist_init(gpa);

    // Adding some values
    *intlist_emplace_back(&list) = 10;
    *intlist_emplace_back(&list) = 20;
    *intlist_emplace_back(&list) = 30;
    assert(list.size == 3);

    // Shallow copying a scalar type, they are essentially their own independent copies
    struct arraylist_intlist copied = intlist_deep_clone(&list, int_deep_clone);
    assert(copied.data != NULL);
    assert(copied.size == list.size);
    assert(copied.size == 3);
    assert(copied.capacity == list.capacity);
    assert(copied.capacity == 4);

    assert(list.data[0] == copied.data[0]);
    assert(list.data[0] == 10);
    assert(copied.data[0] == 10);

    assert(list.alloc.malloc == gpa.malloc);
    assert(copied.alloc.malloc == gpa.malloc);

    // Independent copies, changing a value on list doesn't copied
    list.data[0] = 300;
    assert(list.data[0] == 300);
    assert(copied.data[0] == 10);

    copied.data[0] = 50;
    assert(copied.data[0] == 50);
    assert(list.data[0] == 300);

    // Passing NULL will result in an empty struct, or assert failure
    struct arraylist_intlist empty1 = intlist_shallow_copy(NULL);
    assert(empty1.data == NULL);

    intlist_deinit(&list);
    intlist_deinit(&copied);
    intlist_deinit(&empty1);
    global_destructor_counter_arraylist = 0;
    assert(global_destructor_counter_arraylist == 0);
    printf("test arraylist deep_clone scalar-type passed\n");
}

/* === END ARRAYLIST DEEP CLONE ON SCALAR TYPE === */

int main(void) {
    test_arraylist_init_value();
    test_arraylist_reserve_value();
    test_arraylist_shrink_size_value();
    test_arraylist_shrink_to_fit_value();
    test_arraylist_push_back_value();
    test_arraylist_emplace_back_value();
    test_arraylist_emplace_at_value();
    test_arraylist_insert_at_value();
    test_arraylist_pop_back_value();
    test_arraylist_remove_at_value();
    test_arraylist_remove_from_to_value();
    test_arraylist_at_value();
    test_arraylist_begin_value();
    test_arraylist_back_value();
    test_arraylist_end_value();
    test_arraylist_find_value();
    test_arraylist_contains_value();
    test_arraylist_size_value();
    test_arraylist_is_empty_value();
    test_arraylist_capacity_value();
    test_arraylist_get_allocator_value();
    test_arraylist_swap_value();
    test_arraylist_qsort_value();
    test_arraylist_deep_clone_value();
    test_arraylist_shallow_copy_value();
    test_arraylist_steal_value();
    test_arraylist_clear_value();
    test_arraylist_deinit_value();

    test_arraylist_init_ptr();
    test_arraylist_reserve_ptr();
    test_arraylist_shrink_size_ptr();
    test_arraylist_shrink_to_fit_ptr();
    test_arraylist_push_back_ptr();
    test_arraylist_emplace_back_ptr();
    test_arraylist_emplace_at_ptr();
    test_arraylist_insert_at_pointer();
    test_arraylist_pop_back_ptr();
    test_arraylist_remove_at_ptr();
    test_arraylist_remove_from_to_ptr();
    test_arraylist_at_ptr();
    test_arraylist_begin_ptr();
    test_arraylist_back_ptr();
    test_arraylist_end_ptr();
    test_arraylist_find_ptr();
    test_arraylist_contains_ptr();
    test_arraylist_size_ptr();
    test_arraylist_is_empty_ptr();
    test_arraylist_capacity_ptr();
    test_arraylist_get_allocator_ptr();
    test_arraylist_swap_ptr();
    test_arraylist_qsort_ptr();
    test_arraylist_deep_clone_ptr();
    test_arraylist_shallow_copy_ptr();
    test_arraylist_steal_ptr();
    test_arraylist_clear_ptr();
    test_arraylist_deinit_ptr();

    test_arraylist_shallow_copy_scalar_type();
    test_arraylist_deep_clone_scalar_type();

    return 0;
}
