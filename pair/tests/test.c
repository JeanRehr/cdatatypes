/**
 * @file test.c
 * @brief Unit tests for the pair.h file
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "pair.h"

// == SIMPLE TYPE ==

#define noop_dtor(ptr, alloc) ((void)0)

PAIR(int, int, int_pair, noop_dtor, noop_dtor)

// Comparator for ints for cmp test
static int int_cmp(int *a, int *b) {
    return (*a) - (*b);
}

// Deep-clone for int: simple assignment
static void int_clone(int *dst, int *src, struct Allocator *alloc) {
    (void)alloc;
    *dst = *src;
}

void test_int_pair_init(void) {
    struct pair_int_pair p = int_pair_init(1, 2);
    assert(p.first == 1);
    assert(p.second == 2);
    puts("test_int_pair_init passed");
}

void test_int_pair_cmp(void) {
    struct pair_int_pair p1 = int_pair_init(1, 2);
    struct pair_int_pair p2 = int_pair_init(1, 2);
    struct pair_int_pair p3 = int_pair_init(2, 2);
    struct pair_int_pair p4 = int_pair_init(1, 3);
    assert(int_pair_cmp(&p1, &p2, int_cmp, int_cmp) == 0), // Equal
    assert(int_pair_cmp(&p1, &p3, int_cmp, int_cmp) < 0),  // p1 < p3
    assert(int_pair_cmp(&p3, &p1, int_cmp, int_cmp) > 0),  // p3 > p1
    assert(int_pair_cmp(&p1, &p4, int_cmp, int_cmp) < 0),  // Secondary cmp
    assert(int_pair_cmp(&p4, &p1, int_cmp, int_cmp) > 0);
    puts("test_int_pair_cmp passed");
}

void test_int_pair_swap(void) {
    struct pair_int_pair a = int_pair_init(10, 20);
    struct pair_int_pair b = int_pair_init(30, 40);
    enum pair_error err = int_pair_swap(&a, &b);
    assert(err == PAIR_OK);
    assert(a.first == 30 && a.second == 40);
    assert(b.first == 10 && b.second == 20);

    // Null input check
    assert(int_pair_swap(NULL, &b) == PAIR_ERR_NULL);
    assert(int_pair_swap(&a, NULL) == PAIR_ERR_NULL);
    puts("test_int_pair_swap passed");
}

void test_int_pair_deep_clone(void) {
    struct Allocator alloc = allocator_get_default();
    struct pair_int_pair orig = int_pair_init(9, 7);

    struct pair_int_pair clone = int_pair_deep_clone(&orig, int_clone, int_clone, &alloc);

    assert(clone.first == orig.first);
    assert(clone.second == orig.second);

    // Test NULL input (should return zero-initialized struct)
    struct pair_int_pair zero = int_pair_deep_clone(NULL, int_clone, int_clone, &alloc);
    assert(zero.first == 0 && zero.second == 0);

    puts("test_int_pair_deep_clone passed");
}

void test_int_pair_shallow_copy(void) {
    struct pair_int_pair orig = int_pair_init(53, -10);
    struct pair_int_pair copy = int_pair_shallow_copy(&orig);
    assert(copy.first == orig.first);
    assert(copy.second == orig.second);

    // Test NULL
    struct pair_int_pair zero = int_pair_shallow_copy(NULL);
    assert(zero.first == 0 && zero.second == 0);

    puts("test_int_pair_shallow_copy passed");
}

void test_int_pair_steal(void) {
    struct pair_int_pair orig = int_pair_init(99, 100);
    struct pair_int_pair moved = int_pair_steal(&orig);

    assert(moved.first == 99 && moved.second == 100);
    assert(orig.first == 0 && orig.second == 0); // Steal zeroes orig

    // NULL input returns zero
    struct pair_int_pair zero = int_pair_steal(NULL);
    assert(zero.first == 0 && zero.second == 0);
    puts("test_int_pair_steal passed");
}

void test_int_pair_deinit(void) {
    struct Allocator alloc = allocator_get_default();
    struct pair_int_pair p = int_pair_init(-1, 42);

    int_pair_deinit(&p, &alloc);

    assert(p.first == 0 && p.second == 0);

    // Should be ok to deinit NULL
    int_pair_deinit(NULL, &alloc);

    puts("test_int_pair_deinit passed");
}

// === COMPLEX TYPE ===

struct resource {
    char *data;
    int id;
};

static struct resource resource_constructor(const char *data, int id, struct Allocator *alloc) {
    struct resource res = { 0 };
    size_t size_data = strlen(data) + 1;
    res.data = alloc->malloc(size_data, alloc->ctx);
    memcpy(res.data, data, size_data);
    res.id = id;
    return res;
}

static void resource_dtor(struct resource *res, struct Allocator *alloc) {
    if (!res) {
        return;
    }
    if (!res->data) {
        return;
    }
    alloc->free(res->data, strlen(res->data), alloc->ctx);
    res->data = NULL;
    res->id = 0;
}

static void resource_clone(struct resource *dst, struct resource *src, struct Allocator *alloc) {
    if (!src || !dst)
        return;
    if (src->data) {
        size_t n = strlen(src->data) + 1;
        dst->data = alloc->malloc(n, alloc->ctx);
        memcpy(dst->data, src->data, n);
    } else {
        dst->data = NULL;
    }
    dst->id = src->id;
}

static int resource_cmp(struct resource *a, struct resource *b) {
    if (!a || !b)
        return 0;
    int str = strcmp(a->data ? a->data : "", b->data ? b->data : "");
    if (str != 0)
        return str;
    return a->id - b->id;
}

PAIR(struct resource, struct resource, resource_pair, resource_dtor, resource_dtor)

void test_resource_pair_init(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource left = resource_constructor("left-data", 99, &alloc);
    struct resource right = resource_constructor("right-data", 42, &alloc);
    struct pair_resource_pair p = resource_pair_init(left, right);
    assert(strcmp(p.first.data, "left-data") == 0);
    assert(strcmp(p.second.data, "right-data") == 0);
    assert(p.first.id == 99);
    assert(p.second.id == 42);
    // Clean up
    resource_pair_deinit(&p, &alloc);
    puts("test_resource_pair_init passed");
}

void test_resource_pair_cmp(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource l1 = resource_constructor("foo", 1, &alloc);
    struct resource l2 = resource_constructor("bar", 2, &alloc);
    struct resource r1 = resource_constructor("foo", 1, &alloc);
    struct resource r2 = resource_constructor("bar", 2, &alloc);
    struct pair_resource_pair a = resource_pair_init(l1, l2);
    struct pair_resource_pair b = resource_pair_init(r1, r2);
    // Should be 0, fields are equal
    assert(resource_pair_cmp(&a, &b, resource_cmp, resource_cmp) == 0);
    struct resource l3 = resource_constructor("bar", 2, &alloc);
    struct resource r3 = resource_constructor("bar", 2, &alloc);
    // Now change a field
    struct pair_resource_pair c = resource_pair_init(l3, r3);          // l3="bar",2 r3="bar",2
    assert(resource_pair_cmp(&a, &c, resource_cmp, resource_cmp) > 0); // "foo" > "bar"
    assert(resource_pair_cmp(&c, &a, resource_cmp, resource_cmp) < 0);
    // Clean up
    resource_pair_deinit(&a, &alloc);
    resource_pair_deinit(&b, &alloc);
    resource_pair_deinit(&c, &alloc);
    puts("test_resource_pair_cmp passed");
}

void test_resource_pair_swap(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource l1 = resource_constructor("a", 1, &alloc);
    struct resource l2 = resource_constructor("b", 2, &alloc);
    struct resource r1 = resource_constructor("c", 3, &alloc);
    struct resource r2 = resource_constructor("d", 4, &alloc);

    struct pair_resource_pair p = resource_pair_init(l1, l2);
    struct pair_resource_pair q = resource_pair_init(r1, r2);

    enum pair_error err = resource_pair_swap(&p, &q);
    assert(err == PAIR_OK);

    assert(strcmp(p.first.data, "c") == 0);
    assert(p.first.id == 3);
    assert(strcmp(p.second.data, "d") == 0);
    assert(p.second.id == 4);

    assert(strcmp(q.first.data, "a") == 0);
    assert(q.first.id == 1);
    assert(strcmp(q.second.data, "b") == 0);
    assert(q.second.id == 2);

    // Null checks
    assert(resource_pair_swap(NULL, &p) == PAIR_ERR_NULL);
    assert(resource_pair_swap(&p, NULL) == PAIR_ERR_NULL);

    resource_pair_deinit(&p, &alloc);
    resource_pair_deinit(&q, &alloc);
    puts("test_resource_pair_swap passed");
}

void test_resource_pair_deep_clone(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource l = resource_constructor("X", 8, &alloc);
    struct resource r = resource_constructor("Y", 9, &alloc);

    struct pair_resource_pair orig = resource_pair_init(l, r);

    struct pair_resource_pair clon = resource_pair_deep_clone(&orig, resource_clone, resource_clone, &alloc);

    // Data and ids must match
    assert(strcmp(clon.first.data, orig.first.data) == 0);
    assert(strcmp(clon.second.data, orig.second.data) == 0);
    assert(clon.first.id == orig.first.id);
    assert(clon.second.id == orig.second.id);

    // The data pointers must be DIFFERENT (deep copy)
    assert(clon.first.data != orig.first.data);
    assert(clon.second.data != orig.second.data);

    // Null input should return zero-initialized
    struct pair_resource_pair zero = resource_pair_deep_clone(NULL, resource_clone, resource_clone, &alloc);
    assert(zero.first.data == NULL && zero.second.data == NULL);

    resource_pair_deinit(&orig, &alloc);
    resource_pair_deinit(&clon, &alloc);
    puts("test_resource_pair_deep_clone passed");
}

void test_resource_pair_shallow_copy(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource l = resource_constructor("m", 3, &alloc);
    struct resource r = resource_constructor("n", 6, &alloc);

    struct pair_resource_pair src = resource_pair_init(l, r);
    struct pair_resource_pair copy = resource_pair_shallow_copy(&src);

    assert(strcmp(copy.first.data, src.first.data) == 0);
    assert(strcmp(copy.second.data, src.second.data) == 0);
    assert(copy.first.id == src.first.id);
    assert(copy.second.id == src.second.id);

    // The data pointers must be the SAME! (shallow copy)
    assert(copy.first.data == src.first.data);
    assert(copy.second.data == src.second.data);

    // Null input returns all-zero, data == NULL
    struct pair_resource_pair zero = resource_pair_shallow_copy(NULL);
    assert(zero.first.data == NULL && zero.second.data == NULL);

    // Clean-up both, but only free once, see notes below
    resource_pair_deinit(&src, &alloc);
    // The 'copy' fields are shallow, after src_deinit, their .data is NULL
    // But copy's data pointers are not NULL, so do not call deinit again.
    // Take extreme care with shallow copies of heap data
    // resource_pair_deinit(&copy, &alloc), ///< heap-use-after-free bug

    puts("test_resource_pair_shallow_copy passed");
}

void test_resource_pair_steal(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource l = resource_constructor("Hello", 1, &alloc);
    struct resource r = resource_constructor("World", 2, &alloc);
    struct pair_resource_pair orig = resource_pair_init(l, r);
    struct pair_resource_pair steal = resource_pair_steal(&orig);

    assert(strcmp(steal.first.data, "Hello") == 0);
    assert(strcmp(steal.second.data, "World") == 0);
    assert(orig.first.data == NULL && orig.second.data == NULL);
    // Steal puts orig to zeros

    // NULL returns zero
    struct pair_resource_pair zero = resource_pair_steal(NULL);
    assert(zero.first.data == NULL && zero.second.data == NULL);

    resource_pair_deinit(&steal, &alloc);
    puts("test_resource_pair_steal passed");
}

void test_resource_pair_deinit(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource l = resource_constructor("abc", 100, &alloc);
    struct resource r = resource_constructor("def", 200, &alloc);

    struct pair_resource_pair p = resource_pair_init(l, r);
    resource_pair_deinit(&p, &alloc);
    // Repeated deinit is safe, is no-op
    resource_pair_deinit(&p, &alloc);
    assert(p.first.data == NULL && p.second.data == NULL);
    assert(p.first.id == 0 && p.second.id == 0);
    // NULL input is safe
    resource_pair_deinit(NULL, &alloc);

    puts("test_resource_pair_deinit passed");
}

// == COMPLEX TYPE POINTER ==

static struct resource *resource_ptr_constructor(const char *data, int id, struct Allocator *alloc) {
    struct resource *res = alloc->malloc(sizeof(struct resource), alloc->ctx);
    size_t size_data = strlen(data) + 1;
    res->data = alloc->malloc(size_data, alloc->ctx);
    memcpy(res->data, data, size_data);
    res->id = id;
    return res;
}

static void resource_ptr_dtor(struct resource **res, struct Allocator *alloc) {
    if (!res || !*res) {
        return;
    }
    if ((*res)->data) {
        alloc->free((*res)->data, strlen((*res)->data), alloc->ctx);
        (*res)->data = NULL;
    }
    
    (*res)->id = 0;
    alloc->free(*res, sizeof(**res), alloc->ctx);
    *res = NULL;
}

static void resource_ptr_clone(struct resource **dst, struct resource **src, struct Allocator *alloc) {
    if (!dst || !src || !*src) {
        return;
    }

    // Allocate dst itself
    *dst = alloc->malloc(sizeof(struct resource), alloc->ctx);

    if ((*src)->data) {
        size_t n = strlen((*src)->data) + 1;
        (*dst)->data = alloc->malloc(n, alloc->ctx);
        memcpy((*dst)->data, (*src)->data, n);
    } else {
        (*dst)->data = NULL;
    }
    (*dst)->id = (*src)->id;
}

static int resource_ptr_cmp(struct resource **a, struct resource **b) {
    if (!a || !b || !*a || !*b) {
        return 0;
    }
    int str = strcmp((*a)->data ? (*a)->data : "", (*b)->data ? (*b)->data : "");
    if (str != 0)
        return str;
    return (*a)->id - (*b)->id;
}

PAIR(struct resource *, struct resource *, res_ptr_pair, resource_ptr_dtor, resource_ptr_dtor)

void test_resource_ptr_pair_init(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *left = resource_ptr_constructor("left-data", 99, &alloc);
    struct resource *right = resource_ptr_constructor("right-data", 42, &alloc);
    struct pair_res_ptr_pair p = res_ptr_pair_init(left, right);
    assert(strcmp(p.first->data, "left-data") == 0);
    assert(strcmp(p.second->data, "right-data") == 0);
    assert(p.first->id == 99);
    assert(p.second->id == 42);
    // Clean up
    res_ptr_pair_deinit(&p, &alloc);
    puts("test_resource_ptr_pair_init passed");
}

void test_resource_ptr_pair_cmp(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *l1 = resource_ptr_constructor("foo", 1, &alloc);
    struct resource *l2 = resource_ptr_constructor("bar", 2, &alloc);
    struct resource *r1 = resource_ptr_constructor("foo", 1, &alloc);
    struct resource *r2 = resource_ptr_constructor("bar", 2, &alloc);
    struct pair_res_ptr_pair a = res_ptr_pair_init(l1, l2);
    struct pair_res_ptr_pair b = res_ptr_pair_init(r1, r2);
    // Should be 0, fields are equal
    assert(res_ptr_pair_cmp(&a, &b, resource_ptr_cmp, resource_ptr_cmp) == 0);
    struct resource *l3 = resource_ptr_constructor("bar", 2, &alloc);
    struct resource *r3 = resource_ptr_constructor("bar", 2, &alloc);
    // Now change a field
    struct pair_res_ptr_pair c = res_ptr_pair_init(l3, r3);          // l3="bar",2 r3="bar",2
    assert(res_ptr_pair_cmp(&a, &c, resource_ptr_cmp, resource_ptr_cmp) > 0); // "foo" > "bar"
    assert(res_ptr_pair_cmp(&c, &a, resource_ptr_cmp, resource_ptr_cmp) < 0);
    // Clean up
    res_ptr_pair_deinit(&a, &alloc);
    res_ptr_pair_deinit(&b, &alloc);
    res_ptr_pair_deinit(&c, &alloc);
    puts("test_resource_ptr_pair_cmp passed");
}

void test_resource_ptr_pair_swap(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *l1 = resource_ptr_constructor("a", 1, &alloc);
    struct resource *l2 = resource_ptr_constructor("b", 2, &alloc);
    struct resource *r1 = resource_ptr_constructor("c", 3, &alloc);
    struct resource *r2 = resource_ptr_constructor("d", 4, &alloc);

    struct pair_res_ptr_pair p = res_ptr_pair_init(l1, l2);
    struct pair_res_ptr_pair q = res_ptr_pair_init(r1, r2);

    enum pair_error err = res_ptr_pair_swap(&p, &q);
    assert(err == PAIR_OK);

    assert(strcmp(p.first->data, "c") == 0);
    assert(p.first->id == 3);
    assert(strcmp(p.second->data, "d") == 0);
    assert(p.second->id == 4);

    assert(strcmp(q.first->data, "a") == 0);
    assert(q.first->id == 1);
    assert(strcmp(q.second->data, "b") == 0);
    assert(q.second->id == 2);

    // Null checks
    assert(res_ptr_pair_swap(NULL, &p) == PAIR_ERR_NULL);
    assert(res_ptr_pair_swap(&p, NULL) == PAIR_ERR_NULL);

    res_ptr_pair_deinit(&p, &alloc);
    res_ptr_pair_deinit(&q, &alloc);
    puts("test_resource_ptr_pair_swap passed");
}

void test_resource_ptr_pair_deep_clone(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *l = resource_ptr_constructor("X", 8, &alloc);
    struct resource *r = resource_ptr_constructor("Y", 9, &alloc);

    struct pair_res_ptr_pair orig = res_ptr_pair_init(l, r);

    struct pair_res_ptr_pair clon = res_ptr_pair_deep_clone(&orig, resource_ptr_clone, resource_ptr_clone, &alloc);

    // Data and ids must match
    assert(strcmp(clon.first->data, orig.first->data) == 0);
    assert(strcmp(clon.second->data, orig.second->data) == 0);
    assert(clon.first->id == orig.first->id);
    assert(clon.second->id == orig.second->id);

    // The data pointers must be DIFFERENT (deep copy)
    assert(clon.first->data != orig.first->data);
    assert(clon.second->data != orig.second->data);

    // Null input should return zero-initialized
    struct pair_res_ptr_pair zero = res_ptr_pair_deep_clone(NULL, resource_ptr_clone, resource_ptr_clone, &alloc);
    // the pointer itself is null for pair pointer types
    assert(zero.first == NULL && zero.second == NULL);

    res_ptr_pair_deinit(&orig, &alloc);
    res_ptr_pair_deinit(&clon, &alloc);
    puts("test_resource_ptr_pair_deep_clone passed");
}

void test_resource_ptr_pair_shallow_copy(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *l = resource_ptr_constructor("m", 3, &alloc);
    struct resource *r = resource_ptr_constructor("n", 6, &alloc);

    struct pair_res_ptr_pair src = res_ptr_pair_init(l, r);
    struct pair_res_ptr_pair copy = res_ptr_pair_shallow_copy(&src);

    assert(strcmp(copy.first->data, src.first->data) == 0);
    assert(strcmp(copy.second->data, src.second->data) == 0);
    assert(copy.first->id == src.first->id);
    assert(copy.second->id == src.second->id);

    // The data pointers must be the SAME! (shallow copy)
    assert(copy.first->data == src.first->data);
    assert(copy.second->data == src.second->data);

    // Null input returns all-zero, the pointer itself is null for pair pointer types
    struct pair_res_ptr_pair zero = res_ptr_pair_shallow_copy(NULL);
    assert(zero.first == NULL && zero.second == NULL);

    // Clean-up both, but only free once, see notes below
    res_ptr_pair_deinit(&src, &alloc);
    // The 'copy' fields are shallow, after src_deinit, their ->data is NULL
    // But copy's data pointers are not NULL, so do not call deinit again.
    // Take extreme care with shallow copies of heap data
    // res_ptr_pair_deinit(&copy, &alloc), ///< heap-use-after-free bug

    puts("test_resource_ptr_pair_shallow_copy passed");
}

void test_resource_ptr_pair_steal(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *l = resource_ptr_constructor("Hello", 1, &alloc);
    struct resource *r = resource_ptr_constructor("World", 2, &alloc);
    struct pair_res_ptr_pair orig = res_ptr_pair_init(l, r);
    struct pair_res_ptr_pair steal = res_ptr_pair_steal(&orig);

    assert(strcmp(steal.first->data, "Hello") == 0);
    assert(strcmp(steal.second->data, "World") == 0);
    assert(orig.first == NULL && orig.second == NULL);
    // Steal puts orig to zeros

    // NULL returns zero
    struct pair_res_ptr_pair zero = res_ptr_pair_steal(NULL);
    assert(zero.first == NULL && zero.second == NULL);

    res_ptr_pair_deinit(&steal, &alloc);
    puts("test_resource_ptr_pair_steal passed");
}

void test_resource_ptr_pair_deinit(void) {
    struct Allocator alloc = allocator_get_default();
    struct resource *l = resource_ptr_constructor("abc", 100, &alloc);
    struct resource *r = resource_ptr_constructor("def", 200, &alloc);

    struct pair_res_ptr_pair p = res_ptr_pair_init(l, r);
    res_ptr_pair_deinit(&p, &alloc);
    // Repeated deinit is safe, is no-op
    res_ptr_pair_deinit(&p, &alloc);
    assert(p.first == NULL && p.second == NULL);
    // NULL input is safe
    res_ptr_pair_deinit(NULL, &alloc);

    puts("test_resource_ptr_pair_deinit passed");
}

int main(void) {
    test_int_pair_init();
    test_int_pair_cmp();
    test_int_pair_swap();
    test_int_pair_deep_clone();
    test_int_pair_shallow_copy();
    test_int_pair_steal();
    test_int_pair_deinit();

    test_resource_pair_init();
    test_resource_pair_cmp();
    test_resource_pair_swap();
    test_resource_pair_deep_clone();
    test_resource_pair_shallow_copy();
    test_resource_pair_steal();
    test_resource_pair_deinit();

    test_resource_ptr_pair_init();
    test_resource_ptr_pair_cmp();
    test_resource_ptr_pair_swap();
    test_resource_ptr_pair_deep_clone();
    test_resource_ptr_pair_shallow_copy();
    test_resource_ptr_pair_steal();
    test_resource_ptr_pair_deinit();

    return 0;
}