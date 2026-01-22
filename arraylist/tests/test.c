/**
 * @file test.c
 * @brief Unit tests for the arraylist.h file
 */
#include "allocator.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// To use asserts inside arraylist functions instead of error code returns, define the following
// variable ARRAYLIST_USE_ASSERT as 1, it must be before arraylist.h is included
// Uncomment next line to use asserts
//#define ARRAYLIST_USE_ASSERT 1
// If wanted to use arraylist_* prefix to the functions:
//#define ARRAYLIST_USE_PREFIX
//#define ARRAYLIST_DYN_USE_PREFIX
#include <arraylist.h>

struct test {
    char *objname;
    int *a;
    float *b;
};

static struct test *test_alloc_ctor(int a, float b, char *objname, struct Allocator *alloc) {
    struct test *t = alloc->malloc(sizeof(struct test), alloc->ctx);
    t->a = alloc->malloc(sizeof(a), alloc->ctx);
    *t->a = a;

    t->b = alloc->malloc(sizeof(b), alloc->ctx);
    *t->b = b;

    t->objname = objname;
    return t;
}

static void test_ctor(struct test *t, int a, float b, char *objname, struct Allocator *alloc) {
    t->a = alloc->malloc(sizeof(a), alloc->ctx);
    *t->a = a;

    t->b = alloc->malloc(sizeof(b), alloc->ctx);
    *t->b = b;
    t->objname = objname;
}

static struct test test_ctor_by_val(int a, float b, char *objname, struct Allocator *alloc) {
    struct test t;
    t.a = alloc->malloc(sizeof(a), alloc->ctx);
    *t.a = a;
    t.b = alloc->malloc(sizeof(b), alloc->ctx);
    *t.b = b;
    t.objname = objname;
    return t;
}

static void test_print(struct test *t) {
    printf("t->objname = %s\n", t->objname);
}

static inline void test_dtor(struct test *t, struct Allocator *alloc) {
    if (!t) {
        return;
    }

    alloc->free(t->a, sizeof(t->a), alloc->ctx);
    alloc->free(t->b, sizeof(t->b), alloc->ctx);
}

static inline void test_ptr_dtor(struct test **t, struct Allocator *alloc) {
    if (!t || !*t) {
        return;
    }

    test_dtor(*t, alloc);
    alloc->free(*t, sizeof(*t), alloc->ctx);
    *t = NULL;
}

// static void test_move_constructor(struct test *dst, struct test *src) {
//     dst->a = src->a;
//     dst->b = src->b;
//     dst->objname = src->objname;

//     src->a = NULL;
//     src->b = NULL;
//     src->objname = NULL;
// }

// Macro based destructor
#define test_ptr_dtor_macro(dptr_test, alloc) \
    do { \
        if (!dptr_test || !*dptr_test) { \
            break; \
        } \
        (alloc)->free((*dptr_test)->a, sizeof(*(*dptr_test)->a), (alloc)->ctx); \
        (alloc)->free((*dptr_test)->b, sizeof(*(*dptr_test)->b), (alloc)->ctx); \
        (alloc)->free(*dptr_test, sizeof(struct test), (alloc)->ctx); \
        *(dptr_test) = NULL; \
    } while (0)

// C23 typeof could also be used to prevent misuses of this macro, example:
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)

    #define check_same_types(T, U) \
        { static_assert(_Generic((T), typeof(U): true, default: false), "Parameters must have the same type"); }

#else

    #define check_same_types(T, U) (void)0 // Do nothing

#endif

// now this macro, on C23, will check that dptr_test is indeed a double pointer to non_pod struct
// As this is a macro just to be used inside the arraylist, there is no way I can think of that can
// be misused as the arraylist is typesafe, unless used outside of the arraylist
#define test_ptr_dtor_macro2(dptr_test, alloc) \
    do { \
        check_same_types(dptr_test, struct test **); \
        check_same_types(alloc, struct Allocator *); \
        if (!dptr_test || !*dptr_test) { \
            break; \
        } \
        (alloc)->free((*dptr_test)->a, sizeof(*(*dptr_test)->a), (alloc)->ctx); \
        (alloc)->free((*dptr_test)->b, sizeof(*(*dptr_test)->b), (alloc)->ctx); \
        (alloc)->free(*dptr_test, sizeof(struct test), (alloc)->ctx); \
        *(dptr_test) = NULL; \
    } while (0);

ARRAYLIST(struct test *, test, test_ptr_dtor_macro2)

static void test_arraylist_init_and_deinit(void) {
    printf("Testing arraylist init and deinit functions.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);
    assert(&arrlisttest.alloc);
    assert(arrlisttest.capacity == 0);
    assert(!arrlisttest.data);
    assert(arrlisttest.size == 0);

    test_deinit(&arrlisttest);
    assert(!arrlisttest.data);

    arrlisttest = test_init(gpa);
    assert(&arrlisttest.alloc);
    assert(arrlisttest.capacity == 0);
    assert(!arrlisttest.data);
    assert(arrlisttest.size == 0);

    test_deinit(&arrlisttest);
    assert(!arrlisttest.data);
    printf("arraylist init and deinit functions passed all tests.\n");
}

static void test_arraylist_reserve(void) {
    printf("Testing arraylist reserve function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    size_t cap = 10;

    test_reserve(&arrlisttest, cap);
    assert(test_capacity(&arrlisttest) == cap);

    test_deinit(&arrlisttest);
    printf("arraylist reserve passed all tests.\n");
}

static void test_arraylist_shrink_size(void) {
    printf("Testing arraylist shrink_size function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);

    // Or use arraylist allocator if wanted
    // They must use the same allocator to not invoke UB in some custom allocator cases
    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &arrlisttest.alloc);

    *test_emplace_back_slot(&arrlisttest) = add1;
    *test_emplace_back_slot(&arrlisttest) = add2;
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(12, 0.7, "add3", &gpa);
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(13, 0.8, "add4", &gpa);
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(14, 0.9, "add5", &gpa);

    assert(arrlisttest.size == 5);

    size_t shrink = 3;

    test_shrink_size(&arrlisttest, shrink);
    assert(arrlisttest.size == 3);

    // passed size is greater than arraylist.size, noop
    test_shrink_size(&arrlisttest, 10);
    assert(arrlisttest.size == 3);

    test_deinit(&arrlisttest);
    printf("arraylist shrink_size passed all tests.\n");
}

static void test_arraylist_shrink_to_fit(void) {
    printf("Testing arraylist shrink_to_fit function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    test_reserve(&arrlisttest, 10);
    assert(arrlisttest.capacity == 10);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);

    *test_emplace_back_slot(&arrlisttest) = add1;
    *test_emplace_back_slot(&arrlisttest) = add2;
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(12, 0.7, "add3", &gpa);
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(13, 0.8, "add4", &gpa);
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(14, 0.9, "add5", &gpa);

    assert(arrlisttest.size == 5);

    test_shrink_to_fit(&arrlisttest);

    assert(arrlisttest.size == arrlisttest.capacity);

    test_deinit(&arrlisttest);
    printf("arraylist shrink_to_fit passed all tests.\n");
}

static void test_arraylist_push_back(void) {
    printf("Testing arraylist push_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = gpa.malloc(sizeof(struct test), gpa.ctx);
    test_ctor(add1, 10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    test_push_back(&arrlisttest, test_alloc_ctor(12, 0.7, "add3", &gpa));
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test *add4 = test_alloc_ctor(13, 0.8, "add4", &gpa);
    test_push_back(&arrlisttest, add4);
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);

    struct test *add5 = test_alloc_ctor(14, 0.9, "add5", &gpa);
    test_push_back(&arrlisttest, add5);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);

    test_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test *add6 = test_alloc_ctor(15, 1.0, "add6", &gpa);
    test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 11);

    struct test *add7 = test_alloc_ctor(15, 1.0, "add7", &gpa);
    test_push_back(&arrlisttest, add7);
    assert(arrlisttest.size == 7);
    assert(arrlisttest.capacity == 11);

    struct test *add8 = test_alloc_ctor(15, 1.0, "add8", &gpa);
    test_push_back(&arrlisttest, add8);
    assert(arrlisttest.size == 8);
    assert(arrlisttest.capacity == 11);

    struct test *add9 = test_alloc_ctor(15, 1.0, "add9", &gpa);
    test_push_back(&arrlisttest, add9);
    assert(arrlisttest.size == 9);
    assert(arrlisttest.capacity == 11);

    struct test *add10 = test_alloc_ctor(15, 1.0, "add10", &gpa);
    test_push_back(&arrlisttest, add10);
    assert(arrlisttest.size == 10);
    assert(arrlisttest.capacity == 11);

    struct test *add11 = test_alloc_ctor(15, 1.0, "add11", &gpa);
    test_push_back(&arrlisttest, add11);
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);

    struct test *add12 = test_alloc_ctor(15, 1.0, "add12", &gpa);
    test_push_back(&arrlisttest, add12);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));

    test_deinit(&arrlisttest);
    printf("arraylist push_back passed all tests.\n");
}

static void test_arraylist_emplace_back_slot(void) {
    printf("Testing arraylist emplace_back_slot function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    // ways that emplace_back can be used with arraylist of pointer to values:
    // Note, slot given back must always be checked in real scenarios
    struct test **slot1 = test_emplace_back_slot(&arrlisttest);
    if (!slot1) {
        assert(false && "emplace back returned null");
    }
    assert(arrlisttest.size == 1);

    // A constructor that returns an allocated and initialized struct acts like new
    *slot1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    if (!*slot1) {
        assert(false && "allocation failure");
    }

    // One liner not testing slot or allocation
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(11, 0.6, "add2", &gpa);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    // Inserting into it with emplace back slot with a constructor that does not
    // return an allocated struct itself
    struct test *add3 = gpa.malloc(sizeof(struct test), gpa.ctx);
    // Can be tested for allocation failure
    if (!add3) {
        assert(false && "allocation failure");
    }

    *add3 = test_ctor_by_val(12, 0.7, "add3", &gpa);

    // Warning: Not checking slot here
    *test_emplace_back_slot(&arrlisttest) = add3;

    struct test **add3_same = test_at(&arrlisttest, 2);
    assert(*add3_same == add3);

    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test *add4 = test_alloc_ctor(14, 0.8, "add4", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add4;
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);

    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(15, 0.9, "add5", &gpa);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);

    struct test **add6 = test_emplace_back_slot(&arrlisttest);
    *add6 = test_alloc_ctor(16, 1.0, "add6", &gpa);

    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 8);

    struct test **add6_same = test_at(&arrlisttest, 5);
    assert(*add6_same == *add6);

    struct test **add7 = test_emplace_back_slot(&arrlisttest);
    *add7 = test_alloc_ctor(14, 0.9, "add7", &gpa);
    assert(arrlisttest.size == 7);
    assert(arrlisttest.capacity == 8);
    struct test **add7_same = test_at(&arrlisttest, 6);
    assert(add7_same == add7);

    test_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test **add8 = test_emplace_back_slot(&arrlisttest);
    *add8 = test_alloc_ctor(15, 1.0, "add8", &gpa);
    assert(arrlisttest.size == 8);
    assert(arrlisttest.capacity == 11);
    struct test **add8_same = test_at(&arrlisttest, 7);
    assert(*add8_same == *add8);

    struct test **add9 = test_emplace_back_slot(&arrlisttest);
    *add9 = test_alloc_ctor(16, 1.1, "add9", &gpa);
    assert(arrlisttest.size == 9);
    struct test **add9_same = test_at(&arrlisttest, 8);
    assert(*add9_same == *add9);

    struct test **add10 = test_emplace_back_slot(&arrlisttest);
    *add10 = test_alloc_ctor(17, 1.2, "add10", &gpa);
    assert(arrlisttest.size == 10);
    struct test **add10_same = test_at(&arrlisttest, 9);
    assert(add10_same == add10);

    struct test **add11 = test_emplace_back_slot(&arrlisttest);
    *add11 = test_alloc_ctor(18, 1.3, "add11", &gpa);
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);
    struct test **add11_same = test_at(&arrlisttest, 10);
    assert(*add11_same == *add11);

    struct test **add12 = test_emplace_back_slot(&arrlisttest);
    *add12 = test_alloc_ctor(19, 1.4, "add12", &gpa);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));
    struct test **add12_same = test_at(&arrlisttest, 11);
    assert(add12_same == add12);

    struct test **add13 = test_emplace_back_slot(&arrlisttest);
    *add13 = test_alloc_ctor(20, 1.5, "add13", &gpa);
    assert(arrlisttest.size == 13);
    struct test **add13_same = test_at(&arrlisttest, 12);
    assert(add13_same == add13);

    struct test **add14 = test_emplace_back_slot(&arrlisttest);
    *add14 = test_alloc_ctor(21, 1.6, "add14", &gpa);
    assert(arrlisttest.size == 14);
    assert(arrlisttest.capacity == (11 * 2));
    struct test **add14_same = test_at(&arrlisttest, 13);
    assert(add14_same == add14);

    test_deinit(&arrlisttest);

    printf("arraylist emplace_back_slot passed all tests.\n");
}

static void test_arraylist_insert_at(void) {
    printf("Testing arraylist insert_at function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1;
    add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_insert_at(&arrlisttest, add1, 0);

    struct test *add2;
    add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_insert_at(&arrlisttest, add2, 1);

    struct test *add3;
    add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);
    test_insert_at(&arrlisttest, add3, 2);

    assert(arrlisttest.size == 3);

    struct test *add4;
    add4 = test_alloc_ctor(13, 0.2, "add4", &gpa);

    // Overflow should insert at last position
    test_insert_at(&arrlisttest, add4, -1);
    assert(arrlisttest.size == 4);
    assert(strcmp(arrlisttest.data[arrlisttest.size - 1]->objname, add4->objname) == 0);

    struct test *add5;
    add5 = test_alloc_ctor(15, 0.5, "add5", &gpa);
    test_insert_at(&arrlisttest, add5, 0);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);
    assert(strcmp(arrlisttest.data[0]->objname, add5->objname) == 0);

    size_t add6_index_pos = 2;
    struct test *add6;
    add6 = test_alloc_ctor(16, 0.6, "add6", &gpa);
    test_insert_at(&arrlisttest, add6, add6_index_pos); // Between add1 and add2
    assert(arrlisttest.size == 6);
    assert(strcmp(arrlisttest.data[2]->objname, add6->objname) == 0);

    size_t add7_index_pos = arrlisttest.size - 1;
    struct test *add7;
    add7 = test_alloc_ctor(17, 0.7, "add7", &gpa);
    test_insert_at(&arrlisttest, add7, add7_index_pos); // index 5, should be before last
    assert(strcmp(arrlisttest.data[arrlisttest.size - 2]->objname, add7->objname) == 0);
    assert(arrlisttest.size == 7);

    size_t add8_index_pos = arrlisttest.size;
    struct test *add8;
    add8 = test_alloc_ctor(17, 0.7, "add8", &gpa);
    test_insert_at(&arrlisttest, add8, add8_index_pos); // index 7, should be last
    assert(strcmp(arrlisttest.data[arrlisttest.size - 1]->objname, add8->objname) == 0);
    assert(strcmp((*test_back(&arrlisttest))->objname, add8->objname) == 0);
    assert(arrlisttest.size == 8);

    test_deinit(&arrlisttest);
    printf("arraylist insert_at passed all tests.\n");
}

static void test_arraylist_pop_back(void) {
    printf("Testing arraylist pop_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);

    test_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);
    assert(arrlisttest.capacity == 1);

    test_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 1);
    assert(arrlisttest.capacity == 2);

    test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 1);
    test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 0);

    assert(test_pop_back(&arrlisttest) == ARRAYLIST_OK);
    assert(test_pop_back(&arrlisttest) == ARRAYLIST_OK);

    test_deinit(&arrlisttest);
    printf("arraylist pop_back passed all tests.\n");
}

static void test_arraylist_remove_at(void) {
    printf("Testing arraylist remove_at function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);

    *test_emplace_back_slot(&arrlisttest) = add1;
    *test_emplace_back_slot(&arrlisttest) = add2;
    *test_emplace_back_slot(&arrlisttest) = add3;

    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test *add4 = test_alloc_ctor(12, 0.7, "add4", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add4;

    struct test *add5 = test_alloc_ctor(12, 0.7, "add6", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add5;

    struct test *add6 = test_alloc_ctor(12, 0.7, "add6", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add6;
    assert(arrlisttest.size == 6);

    /*
    for (size_t j = 0; j < arrlisttest.size; ++j) {
        printf("index = %lu | ", j);
        test_print(&arrlisttest.data[j]);
    }
    */

    test_remove_at(&arrlisttest, 3); //add4
    assert(arrlisttest.size == 5);

    test_remove_at(&arrlisttest, 3); //add5 now
    assert(arrlisttest.size == 4);

    // Should overflow and remove last element
    test_remove_at(&arrlisttest, -1); //add6
    assert(arrlisttest.size == 3);

    test_remove_at(&arrlisttest, 0); //add1
    assert(arrlisttest.size == 2);

    test_remove_at(&arrlisttest, 1); //add3
    assert(arrlisttest.size == 1);

    /*
    for (size_t i = 0; i < arrlisttest.size; ++i) {
        printf("index = %lu | ", i);
        test_print(&arrlisttest.data[i]);
    }
    */

    test_remove_at(&arrlisttest, 0);
    assert(arrlisttest.size == 0);
    assert(test_remove_at(&arrlisttest, 0) == ARRAYLIST_OK);

    test_deinit(&arrlisttest);
    printf("arraylist remove_at passed all tests.\n");
}

static void test_arraylist_remove_from_to(void) {
    printf("Testing arraylist remove_from_to function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);
    test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test *add4 = test_alloc_ctor(13, 0.8, "add4", &gpa);
    test_push_back(&arrlisttest, add4);

    struct test *add5 = test_alloc_ctor(12, 0.7, "add5", &gpa);
    test_push_back(&arrlisttest, add5);

    struct test *add6 = test_alloc_ctor(12, 0.7, "add6", &gpa);
    test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);

    /*
    for (size_t j = 0; j < arrlisttest.size; ++j) {
        printf("index = %lu | ", j);
        test_print(&arrlisttest.data[j]);
    }
    */

    // from > to should do nothing
    test_remove_from_to(&arrlisttest, 4, 3);
    assert(arrlisttest.size == 6);

    // should overflow and just pop_back
    test_remove_from_to(&arrlisttest, -1, -1); // add6
    assert(arrlisttest.size == 5);

    test_remove_from_to(&arrlisttest, 1, 3); // add2 add3 add4
    assert(arrlisttest.size == 2);

    for (size_t i = 0; i < arrlisttest.size; ++i) {
        printf("index = %lu | ", i);
        test_print(arrlisttest.data[i]);
    }

    test_remove_from_to(&arrlisttest, 0, 0); // add1
    assert(arrlisttest.size == 1);

    // Double-free, add1 must be constructed again to be instered
    // test_push_back(&arrlisttest, add1);

    add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);
    test_remove_from_to(&arrlisttest, 0, 1); // add1 add2
    assert(arrlisttest.size == 0);

    assert(test_remove_from_to(&arrlisttest, 0, 0) == ARRAYLIST_OK); // noop
    assert(arrlisttest.size == 0);

    test_deinit(&arrlisttest);
    printf("arraylist remove_from_to passed all tests.\n");
}

static void test_arraylist_at(void) {
    printf("Testing arraylist at function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add1;

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add2;

    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);
    *test_emplace_back_slot(&arrlisttest) = add3;

    struct test **add1_p = test_at(&arrlisttest, 0);
    assert(add1_p == arrlisttest.data); // A pointer to the beginning of the array

    struct test **add2_p = test_at(&arrlisttest, 1);
    assert(add2_p == arrlisttest.data + 1);

    struct test **add3_p = test_at(&arrlisttest, 2);
    assert(add3_p == arrlisttest.data + 2);

    test_deinit(&arrlisttest);
    printf("arraylist at passed all tests.\n");
}

static void test_arraylist_begin(void) {
    printf("Testing arraylist begin function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    struct test **it_begin = test_begin(&arrlisttest);
    assert(it_begin == arrlisttest.data);

    assert(it_begin + 1 == arrlisttest.data + 1);

    test_deinit(&arrlisttest);
    printf("arraylist begin passed all tests.\n");
}

static void test_arraylist_back(void) {
    printf("Testing arraylist back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test **one_ele = test_back(&arrlisttest);
    assert(one_ele == arrlisttest.data);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    struct test **last_ele = test_back(&arrlisttest);
    assert(last_ele == arrlisttest.data + 1);

    assert(last_ele - 1 == arrlisttest.data);

    test_deinit(&arrlisttest);
    printf("arraylist back passed all tests.\n");
}

static void test_arraylist_end(void) {
    printf("Testing arraylist end function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    struct test **it_end = test_end(&arrlisttest);
    assert(it_end == arrlisttest.data + 2); // it is a pointer to after the last element

    struct test **last_ele = test_back(&arrlisttest);
    assert(it_end - 1 == last_ele);

    assert(it_end - 1 == arrlisttest.data + 1);
    // The following line is a heap buffer overflow, end should not be dereferenced
    // printf("it_end data %d", *it_end->a);

    test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

bool test_pred_find_name_ptr(struct test **a, void *name) {
    if (strcmp((*a)->objname, name) == 0) {
        return true;
    }
    return false;
}

static void test_arraylist_find(void) {
    printf("Testing arraylist find function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    const char *target_name_found = "add2";

    struct test **found = test_find(&arrlisttest, test_pred_find_name_ptr, (char *)target_name_found);
    assert(found == test_at(&arrlisttest, 1));

    const char *target_name_not_found = "add3";
    struct test **not_found = test_find(&arrlisttest, test_pred_find_name_ptr, (char *)target_name_not_found);
    assert(not_found == test_end(&arrlisttest));

    test_deinit(&arrlisttest);
    printf("arraylist find passed all tests.\n");
}

static void test_arraylist_contains(void) {
    printf("Testing arraylist contains function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    const char *target_name_found = "add2";

    size_t index = 0;

    assert(test_contains(&arrlisttest, test_pred_find_name_ptr, (char *)target_name_found, &index));
    assert(index == 1);

    const char *target_name_not_found = "add3";
    assert(!test_contains(&arrlisttest, test_pred_find_name_ptr, (char *)target_name_not_found, &index));
    assert(index == 1); // Unchanged from before

    test_deinit(&arrlisttest);
    printf("arraylist contains passed all tests.\n");
}

static void test_arraylist_size(void) {
    printf("Testing arraylist size function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    assert(test_size(&arrlisttest) == 0);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);
    assert(test_size(&arrlisttest) == 1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);
    assert(test_size(&arrlisttest) == 2);

    test_deinit(&arrlisttest);
    printf("arraylist size passed all tests.\n");
}

static void test_arraylist_is_empty(void) {
    printf("Testing arraylist is empty function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);
    assert(test_is_empty(&arrlisttest));

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);
    assert(!test_is_empty(&arrlisttest));

    test_pop_back(&arrlisttest);
    assert(test_is_empty(&arrlisttest));

    test_deinit(&arrlisttest);
    printf("arraylist is empty passed all tests.\n");
}

static void test_arraylist_capacity(void) {
    printf("Testing arraylist capacity function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);
    assert(test_capacity(&arrlisttest) == 0);

    test_reserve(&arrlisttest, 2);
    assert(test_capacity(&arrlisttest) == 2);

    test_reserve(&arrlisttest, 1);
    assert(test_capacity(&arrlisttest) == 2);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);
    assert(test_capacity(&arrlisttest) == 2);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);
    assert(test_capacity(&arrlisttest) == 2);

    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);
    test_push_back(&arrlisttest, add3);

    assert(test_capacity(&arrlisttest) == 4);

    test_deinit(&arrlisttest);
    printf("arraylist capacity passed all tests.\n");
}

static void test_arraylist_swap(void) {
    printf("Testing arraylist swap function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);
    struct arraylist_test otherarr = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    // It seems that, after reallocation of data because of capacity, pointers might get invalidated
    // Spent quite a while on here because I was storing pointers right after the first push_back
    // struct test *arrlisttest_first_p = test_begin(&arrlisttest);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);
    test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);

    struct test **arrlisttest_first_p = test_begin(&arrlisttest);

    struct test *other1 = test_alloc_ctor(10, 0.5, "other1", &gpa);
    test_push_back(&otherarr, other1);

    struct test *other2 = test_alloc_ctor(11, 0.6, "other2", &gpa);
    test_push_back(&otherarr, other2);
    assert(otherarr.size == 2);

    struct test **otherarr_first_p = test_begin(&otherarr);

    assert(arrlisttest_first_p == test_begin(&arrlisttest));
    assert(otherarr_first_p == test_begin(&otherarr));

    test_swap(&arrlisttest, &otherarr);
    assert(arrlisttest.size == 2);
    assert(otherarr.size == 3);

    assert(arrlisttest_first_p == test_begin(&otherarr));
    assert(otherarr_first_p == test_begin(&arrlisttest));

    {
        size_t i = 0;
        for (struct test **it = test_begin(&arrlisttest); it < test_end(&arrlisttest); ++it) {
            test_print(arrlisttest.data[i]);
            ++i;
        }
    }

    test_deinit(&arrlisttest);
    test_deinit(&otherarr);
    printf("arraylist swap passed all tests.\n");
}

static bool comparator_test_ptr(struct test **a, struct test **b) {
    return *(*a)->a < *(*b)->a;
}

static void test_arraylist_qsort(void) {
    printf("Testing arraylist qsort function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    // Should be second after sort
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(1, 0.1, "add1", &gpa);
    // Should be fourth after sort
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(3, 0.2, "add2", &gpa);
    // Should be sixth after sort
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(5, 0.3, "add3", &gpa);
    // Should be fifth after sort
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(4, 0.4, "add4", &gpa);
    // Should be third after sort
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(2, 0.5, "add5", &gpa);
    // Should be first after sort
    *test_emplace_back_slot(&arrlisttest) = test_alloc_ctor(-1, 0.6, "add6", &gpa);

    assert(strcmp(arrlisttest.data[0]->objname, "add1") == 0);
    assert(strcmp(arrlisttest.data[1]->objname, "add2") == 0);
    assert(strcmp(arrlisttest.data[2]->objname, "add3") == 0);
    assert(strcmp(arrlisttest.data[3]->objname, "add4") == 0);
    assert(strcmp(arrlisttest.data[4]->objname, "add5") == 0);
    assert(strcmp(arrlisttest.data[5]->objname, "add6") == 0);

    test_qsort(&arrlisttest, comparator_test_ptr);

    assert(strcmp(arrlisttest.data[0]->objname, "add6") == 0);
    assert(strcmp(arrlisttest.data[1]->objname, "add1") == 0);
    assert(strcmp(arrlisttest.data[2]->objname, "add5") == 0);
    assert(strcmp(arrlisttest.data[3]->objname, "add2") == 0);
    assert(strcmp(arrlisttest.data[4]->objname, "add4") == 0);
    assert(strcmp(arrlisttest.data[5]->objname, "add3") == 0);

    test_deinit(&arrlisttest);
    printf("arraylist qsort passed all tests.\n");
}

static void test_arraylist_clear(void) {
    printf("Testing arraylist end function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_test arrlisttest = test_init(gpa);

    struct test *add1 = test_alloc_ctor(10, 0.5, "add1", &gpa);
    test_push_back(&arrlisttest, add1);

    struct test *add2 = test_alloc_ctor(11, 0.6, "add2", &gpa);
    test_push_back(&arrlisttest, add2);

    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);
    test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    test_clear(&arrlisttest);
    assert(arrlisttest.size == 0);
    assert(arrlisttest.capacity == 4);

    test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

static bool allocator_test_equality(const struct Allocator *a, const struct Allocator *b) {
    return a->malloc == b->malloc && a->realloc == b->realloc && a->free == b->free && a->ctx == b->ctx;
}

static void test_arraylist_get_allocator_default(void) {
    printf("Testing arraylist get_allocator default function.\n");
    struct arraylist_test arrlisttest = test_init(allocator_get_default()); // Init default allocator

    Allocator def = allocator_get_default();

    assert(allocator_test_equality(&def, test_get_allocator(&arrlisttest)));
    assert(allocator_test_equality(&def, &arrlisttest.alloc)); // Should be same as above

    test_deinit(&arrlisttest);
    printf("arraylist get_allocator default passed all tests.\n");
}

/*
static void test_arraylist_insert_from_to(void) {
    printf("Testing arraylist insert_from_to function.\n");
    struct arraylist_test arrlisttest = test_init(gpa);

    test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist insert_from_to passed all tests.\n");
}
*/

/* === Function Pointer version test START === */

ARRAYLIST_DYN(struct test, test)

static void test_arraylist_dyn_init_and_deinit(void) {
    printf("Testing arraylist init and deinit functions.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);
    assert(arrlisttest.destructor);
    assert(&arrlisttest.alloc);
    assert(arrlisttest.capacity == 0);
    assert(!arrlisttest.data);
    assert(arrlisttest.size == 0);

    dyn_test_deinit(&arrlisttest);
    assert(!arrlisttest.data);

    arrlisttest = dyn_test_init(gpa, test_dtor);
    assert(arrlisttest.destructor);
    assert(&arrlisttest.alloc);
    assert(arrlisttest.capacity == 0);
    assert(!arrlisttest.data);
    assert(arrlisttest.size == 0);

    dyn_test_deinit(&arrlisttest);
    assert(!arrlisttest.data);
    printf("arraylist init and deinit functions passed all tests.\n");
}

static void test_arraylist_dyn_reserve(void) {
    printf("Testing arraylist reserve function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    size_t cap = 10;

    dyn_test_reserve(&arrlisttest, cap);
    assert(dyn_test_capacity(&arrlisttest) == cap);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist reserve passed all tests.\n");
}

static void test_arraylist_dyn_shrink_size(void) {
    printf("Testing arraylist shrink_size function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;
    struct test add4;
    struct test add5;

    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    test_ctor(&add4, 13, 0.8, "add4", &gpa);
    test_ctor(&add5, 14, 0.9, "add5", &gpa);

    dyn_test_push_back(&arrlisttest, add1);
    dyn_test_push_back(&arrlisttest, add2);
    dyn_test_push_back(&arrlisttest, add3);
    dyn_test_push_back(&arrlisttest, add4);
    dyn_test_push_back(&arrlisttest, add5);

    assert(arrlisttest.size == 5);

    size_t shrink = 3;

    dyn_test_shrink_size(&arrlisttest, shrink);
    assert(arrlisttest.size == 3);

    // passed size is greater than arraylist.size, noop
    dyn_test_shrink_size(&arrlisttest, 10);
    assert(arrlisttest.size == 3);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist shrink_size passed all tests.\n");
}

static void test_arraylist_dyn_shrink_to_fit(void) {
    printf("Testing arraylist shrink_to_fit function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;
    struct test add4;
    struct test add5;

    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    test_ctor(&add4, 13, 0.8, "add4", &gpa);
    test_ctor(&add5, 14, 0.9, "add5", &gpa);

    dyn_test_reserve(&arrlisttest, 10);
    assert(arrlisttest.capacity == 10);

    dyn_test_push_back(&arrlisttest, add1);
    dyn_test_push_back(&arrlisttest, add2);
    dyn_test_push_back(&arrlisttest, add3);
    dyn_test_push_back(&arrlisttest, add4);
    dyn_test_push_back(&arrlisttest, add5);

    assert(arrlisttest.size == 5);

    dyn_test_shrink_to_fit(&arrlisttest);

    assert(arrlisttest.size == arrlisttest.capacity);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist shrink_to_fit passed all tests.\n");
}

static void test_arraylist_dyn_push_back(void) {
    printf("Testing arraylist push_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test add4;
    test_ctor(&add4, 13, 0.8, "add4", &gpa);
    dyn_test_push_back(&arrlisttest, add4);
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);

    struct test add5;
    test_ctor(&add5, 14, 0.9, "add5", &gpa);
    dyn_test_push_back(&arrlisttest, add5);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);

    dyn_test_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test add6;
    test_ctor(&add6, 15, 1.0, "add6", &gpa);
    dyn_test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 11);

    struct test add7;
    test_ctor(&add7, 15, 1.0, "add7", &gpa);
    dyn_test_push_back(&arrlisttest, add7);
    assert(arrlisttest.size == 7);
    assert(arrlisttest.capacity == 11);

    struct test add8;
    test_ctor(&add8, 15, 1.0, "add8", &gpa);
    dyn_test_push_back(&arrlisttest, add8);
    assert(arrlisttest.size == 8);
    assert(arrlisttest.capacity == 11);

    struct test add9;
    test_ctor(&add9, 15, 1.0, "add9", &gpa);
    dyn_test_push_back(&arrlisttest, add9);
    assert(arrlisttest.size == 9);
    assert(arrlisttest.capacity == 11);

    struct test add10;
    test_ctor(&add10, 15, 1.0, "add10", &gpa);
    dyn_test_push_back(&arrlisttest, add10);
    assert(arrlisttest.size == 10);
    assert(arrlisttest.capacity == 11);

    struct test add11;
    test_ctor(&add11, 15, 1.0, "add11", &gpa);
    dyn_test_push_back(&arrlisttest, add11);
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);

    struct test add12;
    test_ctor(&add12, 15, 1.0, "add12", &gpa);
    dyn_test_push_back(&arrlisttest, add12);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));

    dyn_test_deinit(&arrlisttest);
    printf("arraylist push_back passed all tests.\n");
}

static void test_arraylist_dyn_emplace_back_slot(void) {
    printf("Testing arraylist emplace_back_slot function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    // ways that emplace_back can be used with arraylist of values:

    // can be added like this, most efficient as it is constructed inside the container:
    struct test *add1 = dyn_test_emplace_back_slot(&arrlisttest);

    // emplace_back_slot may return NULL, needs to be checked in real scenarios
    if (!add1) {
        assert(false && "emplace back returned null");
    }

    test_ctor(add1, 10, 0.5, "add1", &gpa);
    assert(arrlisttest.size == 1);

    struct test *add1_same = dyn_test_at(&arrlisttest, 0);
    assert(add1_same == add1);

    // can be added like this, copy into slot:
    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    *dyn_test_emplace_back_slot(&arrlisttest) = add2;
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    // could be added by assigning a struct literal, if the struct had simple fields:
    //*dyn_test_emplace_back_slot(&arrlisttest) = (struct test){ .a = ...};

    // can be added with a constructor that returns a struct by value:
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(12, 0.7, "add3", &gpa);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    // can fill slots individually:
    struct test *add4 = dyn_test_emplace_back_slot(&arrlisttest);
    add4->a = malloc(sizeof(int));
    add4->b = malloc(sizeof(float));
    *add4->a = 13;
    *add4->b = 0.8;
    add4->objname = "add4";
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);
    struct test *add4_same = dyn_test_at(&arrlisttest, 3);
    assert(add4_same == add4);

    struct test *add5 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add5, 14, 0.9, "add5", &gpa);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);
    struct test *add5_same = dyn_test_at(&arrlisttest, 4);
    assert(add5_same == add5);

    dyn_test_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test *add6 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add6, 14, 0.9, "add6", &gpa);
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 11);
    struct test *add6_same = dyn_test_at(&arrlisttest, 5);
    assert(add6_same == add6);

    struct test *add7 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add7, 14, 0.9, "add7", &gpa);
    assert(arrlisttest.size == 7);
    struct test *add7_same = dyn_test_at(&arrlisttest, 6);
    assert(add7_same == add7);

    struct test *add8 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add8, 14, 0.9, "add8", &gpa);
    assert(arrlisttest.size == 8);
    struct test *add8_same = dyn_test_at(&arrlisttest, 7);
    assert(add8_same == add8);

    struct test *add9 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add9, 14, 0.9, "add9", &gpa);
    assert(arrlisttest.size == 9);
    struct test *add9_same = dyn_test_at(&arrlisttest, 8);
    assert(add9_same == add9);

    struct test *add10 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add10, 14, 0.9, "add10", &gpa);
    assert(arrlisttest.size == 10);
    struct test *add10_same = dyn_test_at(&arrlisttest, 9);
    assert(add10_same == add10);

    struct test *add11 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add11, 14, 0.9, "add11", &gpa);
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);
    struct test *add11_same = dyn_test_at(&arrlisttest, 10);
    assert(add11_same == add11);

    struct test *add12 = dyn_test_emplace_back_slot(&arrlisttest);
    test_ctor(add12, 14, 0.9, "add11", &gpa);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));
    struct test *add12_same = dyn_test_at(&arrlisttest, 11);
    assert(add12_same == add12);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist emplace_back_slot passed all tests.\n");
}

static void test_arraylist_dyn_insert_at(void) {
    printf("Testing arraylist insert_at function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_insert_at(&arrlisttest, add1, 0);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_insert_at(&arrlisttest, add2, 1);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_insert_at(&arrlisttest, add3, 2);

    assert(arrlisttest.size == 3);

    struct test add4;
    test_ctor(&add4, 13, 0.2, "add4", &gpa);

    // Overflow should insert at last position
    dyn_test_insert_at(&arrlisttest, add4, -1);
    assert(arrlisttest.size == 4);
    assert(strcmp(arrlisttest.data[arrlisttest.size - 1].objname, add4.objname) == 0);

    struct test add5;
    test_ctor(&add5, 15, 0.5, "add5", &gpa);
    dyn_test_insert_at(&arrlisttest, add5, 0);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);
    assert(strcmp(arrlisttest.data[0].objname, add5.objname) == 0);

    size_t add6_index_pos = 2;
    struct test add6;
    test_ctor(&add6, 16, 0.6, "add6", &gpa);
    dyn_test_insert_at(&arrlisttest, add6, add6_index_pos); // Between add1 and add2
    assert(arrlisttest.size == 6);
    assert(strcmp(arrlisttest.data[2].objname, add6.objname) == 0);

    size_t add7_index_pos = arrlisttest.size - 1;
    struct test add7;
    test_ctor(&add7, 17, 0.7, "add7", &gpa);
    dyn_test_insert_at(&arrlisttest, add7, add7_index_pos); // index 5, should be before last
    assert(strcmp(arrlisttest.data[arrlisttest.size - 2].objname, add7.objname) == 0);
    assert(arrlisttest.size == 7);

    size_t add8_index_pos = arrlisttest.size;
    struct test add8;
    test_ctor(&add8, 17, 0.7, "add8", &gpa);
    dyn_test_insert_at(&arrlisttest, add8, add8_index_pos); // index 7, should be last
    assert(strcmp(arrlisttest.data[arrlisttest.size - 1].objname, add8.objname) == 0);
    assert(strcmp(dyn_test_back(&arrlisttest)->objname, add8.objname) == 0);
    assert(arrlisttest.size == 8);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist insert_at passed all tests.\n");
}

static void test_arraylist_dyn_pop_back(void) {
    printf("Testing arraylist pop_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;

    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    test_ctor(&add3, 12, 0.7, "add3", &gpa);

    dyn_test_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);
    assert(arrlisttest.capacity == 1);

    dyn_test_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    dyn_test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 1);
    assert(arrlisttest.capacity == 2);

    dyn_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    dyn_test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 1);
    dyn_test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 0);

    assert(dyn_test_pop_back(&arrlisttest) == ARRAYLIST_OK);
    assert(dyn_test_pop_back(&arrlisttest) == ARRAYLIST_OK);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist pop_back passed all tests.\n");
}

static void test_arraylist_dyn_remove_at(void) {
    printf("Testing arraylist remove_at function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test add4;
    test_ctor(&add4, 12, 0.7, "add4", &gpa);
    dyn_test_push_back(&arrlisttest, add4);

    struct test add5;
    test_ctor(&add5, 12, 0.7, "add5", &gpa);
    dyn_test_push_back(&arrlisttest, add5);

    struct test add6;
    test_ctor(&add6, 12, 0.7, "add6", &gpa);
    dyn_test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);

    /*
    for (size_t j = 0; j < arrlisttest.size; ++j) {
        printf("index = %lu | ", j);
        test_print(&arrlisttest.data[j]);
    }
    */

    dyn_test_remove_at(&arrlisttest, 3); //add4
    assert(arrlisttest.size == 5);

    dyn_test_remove_at(&arrlisttest, 3); //add5 now
    assert(arrlisttest.size == 4);

    // Should overflow and remove last element
    dyn_test_remove_at(&arrlisttest, -1); //add6
    assert(arrlisttest.size == 3);

    dyn_test_remove_at(&arrlisttest, 0); //add1
    assert(arrlisttest.size == 2);

    dyn_test_remove_at(&arrlisttest, 1); //add3
    assert(arrlisttest.size == 1);

    /*
    for (size_t i = 0; i < arrlisttest.size; ++i) {
        printf("index = %lu | ", i);
        test_print(&arrlisttest.data[i]);
    }
    */

    dyn_test_remove_at(&arrlisttest, 0);
    assert(arrlisttest.size == 0);
    assert(dyn_test_remove_at(&arrlisttest, 0) == ARRAYLIST_OK);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist remove_at passed all tests.\n");
}

static void test_arraylist_dyn_remove_from_to(void) {
    printf("Testing arraylist remove_from_to function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test add4;
    test_ctor(&add4, 12, 0.7, "add4", &gpa);
    dyn_test_push_back(&arrlisttest, add4);

    struct test add5;
    test_ctor(&add5, 12, 0.7, "add5", &gpa);
    dyn_test_push_back(&arrlisttest, add5);

    struct test add6;
    test_ctor(&add6, 12, 0.7, "add6", &gpa);
    dyn_test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);

    /*
    for (size_t j = 0; j < arrlisttest.size; ++j) {
        printf("index = %lu | ", j);
        test_print(&arrlisttest.data[j]);
    }
    */

    // from > to should do nothing
    dyn_test_remove_from_to(&arrlisttest, 4, 3);
    assert(arrlisttest.size == 6);

    // should overflow and just pop_back
    dyn_test_remove_from_to(&arrlisttest, -1, -1); // add6
    assert(arrlisttest.size == 5);

    dyn_test_remove_from_to(&arrlisttest, 1, 3); // add2 add3 add4
    assert(arrlisttest.size == 2);

    for (size_t i = 0; i < arrlisttest.size; ++i) {
        printf("index = %lu | ", i);
        test_print(&arrlisttest.data[i]);
    }

    dyn_test_remove_from_to(&arrlisttest, 0, 0); // add1
    assert(arrlisttest.size == 1);

    // Double-free, add1 must be constructed again to be instered
    // dyn_test_push_back(&arrlisttest, add1);

    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);
    dyn_test_remove_from_to(&arrlisttest, 0, 1); // add1 add2
    assert(arrlisttest.size == 0);

    assert(dyn_test_remove_from_to(&arrlisttest, 0, 0) == ARRAYLIST_OK); // noop
    assert(arrlisttest.size == 0);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist remove_from_to passed all tests.\n");
}

static void test_arraylist_dyn_at(void) {
    printf("Testing arraylist at function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);

    struct test *add1_p = dyn_test_at(&arrlisttest, 0);
    assert(add1_p == arrlisttest.data); // A pointer to the beginning of the array

    struct test *add2_p = dyn_test_at(&arrlisttest, 1);
    assert(add2_p == arrlisttest.data + 1);

    struct test *add3_p = dyn_test_at(&arrlisttest, 2);
    assert(add3_p == arrlisttest.data + 2);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist at passed all tests.\n");
}

static void test_arraylist_dyn_begin(void) {
    printf("Testing arraylist begin function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test *it_begin = dyn_test_begin(&arrlisttest);
    assert(it_begin == arrlisttest.data);

    assert(it_begin + 1 == arrlisttest.data + 1);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist begin passed all tests.\n");
}

static void test_arraylist_dyn_back(void) {
    printf("Testing arraylist back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test *one_ele = dyn_test_back(&arrlisttest);
    assert(one_ele == arrlisttest.data);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test *last_ele = dyn_test_back(&arrlisttest);
    assert(last_ele == arrlisttest.data + 1);

    assert(last_ele - 1 == arrlisttest.data);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist back passed all tests.\n");
}

static void test_arraylist_dyn_end(void) {
    printf("Testing arraylist end function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test *it_end = dyn_test_end(&arrlisttest);
    assert(it_end == arrlisttest.data + 2); // it is a pointer to after the last element

    struct test *last_ele = dyn_test_back(&arrlisttest);
    assert(it_end - 1 == last_ele);

    assert(it_end - 1 == arrlisttest.data + 1);
    // The following line is a heap buffer overflow, end should not be dereferenced
    // printf("it_end data %d", *it_end->a);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

bool test_pred_find_name(struct test *a, void *name) {
    if (strcmp(a->objname, name) == 0) {
        return true;
    }
    return false;
}

static void test_arraylist_dyn_find(void) {
    printf("Testing arraylist find function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    const char *target_name_found = "add2";

    struct test *found = dyn_test_find(&arrlisttest, test_pred_find_name, (char *)target_name_found);
    assert(found == dyn_test_at(&arrlisttest, 1));

    const char *target_name_not_found = "add3";
    struct test *not_found = dyn_test_find(&arrlisttest, test_pred_find_name, (char *)target_name_not_found);
    assert(not_found == dyn_test_end(&arrlisttest));

    dyn_test_deinit(&arrlisttest);
    printf("arraylist find passed all tests.\n");
}

static void test_arraylist_dyn_contains(void) {
    printf("Testing arraylist contains function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    const char *target_name_found = "add2";

    size_t index = 0;

    assert(dyn_test_contains(&arrlisttest, test_pred_find_name, (char *)target_name_found, &index));
    assert(index == 1);

    const char *target_name_not_found = "add3";
    assert(!dyn_test_contains(&arrlisttest, test_pred_find_name, (char *)target_name_not_found, &index));
    assert(index == 1); // Unchanged from before

    dyn_test_deinit(&arrlisttest);
    printf("arraylist contains passed all tests.\n");
}

static void test_arraylist_dyn_size(void) {
    printf("Testing arraylist size function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);
    assert(dyn_test_size(&arrlisttest) == 0);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);
    assert(dyn_test_size(&arrlisttest) == 1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);
    assert(dyn_test_size(&arrlisttest) == 2);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist size passed all tests.\n");
}

static void test_arraylist_dyn_is_empty(void) {
    printf("Testing arraylist is empty function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);
    assert(dyn_test_is_empty(&arrlisttest));

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);
    assert(!dyn_test_is_empty(&arrlisttest));

    dyn_test_pop_back(&arrlisttest);
    assert(dyn_test_is_empty(&arrlisttest));

    dyn_test_deinit(&arrlisttest);
    printf("arraylist is empty passed all tests.\n");
}

static void test_arraylist_dyn_capacity(void) {
    printf("Testing arraylist capacity function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);
    assert(dyn_test_capacity(&arrlisttest) == 0);

    dyn_test_reserve(&arrlisttest, 2);
    assert(dyn_test_capacity(&arrlisttest) == 2);

    dyn_test_reserve(&arrlisttest, 1);
    assert(dyn_test_capacity(&arrlisttest) == 2);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);
    assert(dyn_test_capacity(&arrlisttest) == 2);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);
    assert(dyn_test_capacity(&arrlisttest) == 2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);

    assert(dyn_test_capacity(&arrlisttest) == 4);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist capacity passed all tests.\n");
}

static void test_arraylist_dyn_swap(void) {
    printf("Testing arraylist swap function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);
    struct arraylist_dyn_test otherarr = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    // It seems that, after reallocation of data because of capacity, pointers might get invalidated
    // Spent quite a while on here because I was storing pointers right after the first push_back
    // struct test *arrlisttest_first_p = dyn_test_begin(&arrlisttest);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);

    struct test *arrlisttest_first_p = dyn_test_begin(&arrlisttest);

    struct test other1;
    test_ctor(&other1, 10, 0.5, "other1", &gpa);
    dyn_test_push_back(&otherarr, other1);

    struct test other2;
    test_ctor(&other2, 11, 0.6, "other2", &gpa);
    dyn_test_push_back(&otherarr, other2);
    assert(otherarr.size == 2);

    struct test *otherarr_first_p = dyn_test_begin(&otherarr);

    assert(arrlisttest_first_p == dyn_test_begin(&arrlisttest));
    assert(otherarr_first_p == dyn_test_begin(&otherarr));

    dyn_test_swap(&arrlisttest, &otherarr);
    assert(arrlisttest.size == 2);
    assert(otherarr.size == 3);

    assert(arrlisttest_first_p == dyn_test_begin(&otherarr));
    assert(otherarr_first_p == dyn_test_begin(&arrlisttest));

    {
        size_t i = 0;
        for (struct test *it = dyn_test_begin(&arrlisttest); it < dyn_test_end(&arrlisttest); ++it) {
            test_print(&arrlisttest.data[i]);
            ++i;
        }
    }

    dyn_test_deinit(&arrlisttest);
    dyn_test_deinit(&otherarr);
    printf("arraylist swap passed all tests.\n");
}

static bool comparator_test(struct test *a, struct test *b) {
    return *a->a < *b->a;
}

static void test_arraylist_dyn_qsort(void) {
    printf("Testing arraylist qsort function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    // Should be second after sort
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(1, 0.1, "add1", &gpa);
    // Should be fourth after sort
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(3, 0.2, "add2", &gpa);
    // Should be sixth after sort
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(5, 0.3, "add3", &gpa);
    // Should be fifth after sort
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(4, 0.4, "add4", &gpa);
    // Should be third after sort
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(2, 0.5, "add5", &gpa);
    // Should be first after sort
    *dyn_test_emplace_back_slot(&arrlisttest) = test_ctor_by_val(-1, 0.6, "add6", &gpa);

    assert(strcmp(arrlisttest.data[0].objname, "add1") == 0);
    assert(strcmp(arrlisttest.data[1].objname, "add2") == 0);
    assert(strcmp(arrlisttest.data[2].objname, "add3") == 0);
    assert(strcmp(arrlisttest.data[3].objname, "add4") == 0);
    assert(strcmp(arrlisttest.data[4].objname, "add5") == 0);
    assert(strcmp(arrlisttest.data[5].objname, "add6") == 0);

    dyn_test_qsort(&arrlisttest, comparator_test);

    assert(strcmp(arrlisttest.data[0].objname, "add6") == 0);
    assert(strcmp(arrlisttest.data[1].objname, "add1") == 0);
    assert(strcmp(arrlisttest.data[2].objname, "add5") == 0);
    assert(strcmp(arrlisttest.data[3].objname, "add2") == 0);
    assert(strcmp(arrlisttest.data[4].objname, "add4") == 0);
    assert(strcmp(arrlisttest.data[5].objname, "add3") == 0);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist qsort passed all tests.\n");
}

static void test_arraylist_dyn_clear(void) {
    printf("Testing arraylist end function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test arrlisttest = dyn_test_init(gpa, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1", &gpa);
    dyn_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2", &gpa);
    dyn_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3", &gpa);
    dyn_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    dyn_test_clear(&arrlisttest);
    assert(arrlisttest.size == 0);
    assert(arrlisttest.capacity == 4);

    dyn_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

static void test_arraylist_dyn_get_allocator_default(void) {
    printf("Testing arraylist get_allocator default function.\n");
    struct arraylist_dyn_test arrlisttest = dyn_test_init(allocator_get_default(), test_dtor); // Init default allocator

    Allocator def = allocator_get_default();

    assert(allocator_test_equality(&def, dyn_test_get_allocator(&arrlisttest)));
    assert(allocator_test_equality(&def, &arrlisttest.alloc)); // Should be same as above

    dyn_test_deinit(&arrlisttest);
    printf("arraylist get_allocator default passed all tests.\n");
}

/*
static void test_arraylist_dyn_insert_from_to(void) {
    printf("Testing arraylist insert_from_to function.\n");
    struct arraylist_dyn_test arrlisttest = dyn_test_init(NULL, test_dtor);

    dyn_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist insert_from_to passed all tests.\n");
}
*/

// An example of a container of pointers of struct test
ARRAYLIST_DYN(struct test *, test_ptr)

static void test_arraylist_dyn_ptr_push_back(void) {
    printf("Testing arraylist_ptr push_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test_ptr arrlisttestptr = dyn_test_ptr_init(gpa, test_ptr_dtor);

    struct test *add1 = malloc(sizeof(struct test));
    test_ctor(add1, 10, 0.5, "add1", &gpa);

    struct test *add2 = malloc(sizeof(struct test));
    test_ctor(add2, 11, 0.6, "add2", &gpa);

    struct test *add3 = test_alloc_ctor(12, 0.7, "add3", &gpa);

    struct test *add4 = malloc(sizeof(struct test));
    test_ctor(add4, 13, 0.8, "add4", &gpa);

    struct test *add5 = malloc(sizeof(struct test));
    test_ctor(add5, 14, 0.9, "add5", &gpa);

    dyn_test_ptr_push_back(&arrlisttestptr, add1);
    assert(arrlisttestptr.size == 1);
    dyn_test_ptr_push_back(&arrlisttestptr, add2);
    assert(arrlisttestptr.size == 2);
    assert(arrlisttestptr.capacity == 2);
    dyn_test_ptr_push_back(&arrlisttestptr, add3);
    assert(arrlisttestptr.size == 3);
    assert(arrlisttestptr.capacity == 4);
    dyn_test_ptr_push_back(&arrlisttestptr, add4);
    assert(arrlisttestptr.size == 4);
    assert(arrlisttestptr.capacity == 4);
    dyn_test_ptr_push_back(&arrlisttestptr, add5);
    assert(arrlisttestptr.size == 5);
    assert(arrlisttestptr.capacity == 8);

    dyn_test_ptr_reserve(&arrlisttestptr, 11);
    assert(arrlisttestptr.capacity == 11);

    dyn_test_ptr_deinit(&arrlisttestptr);
    // free(add1); // double-free, arraylist owns the pointers to it and frees it

    // add1 is still not null, but points to freed memory, the dtor passed to arraylist sets the
    // freed pointer to null, as the arraylist shallow copies, so dtor will set only the copy
    // inside it to null
    assert(add1);
    assert(!arrlisttestptr.data);
    printf("arraylist_ptr push_back passed all tests.\n");
}

static void test_arraylist_dyn_ptr_emplace_back_slot(void) {
    printf("Testing arraylist_ptr emplace_back_slot function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test_ptr arrlisttestptr = dyn_test_ptr_init(gpa, test_ptr_dtor);

    // ways that emplace_back can be used with arraylist of pointers to values:

    // can be added like this:
    struct test *add1 = malloc(sizeof(struct test));
    test_ctor(add1, 10, 0.5, "add1", &gpa);

    // get the pointer to a slot where a struct test pointer can be stored
    struct test **slot_add1 = dyn_test_ptr_emplace_back_slot(&arrlisttestptr);

    // store it
    *slot_add1 = add1;

    assert(arrlisttestptr.size == 1);

    struct test **add1_same = dyn_test_ptr_at(&arrlisttestptr, 0);
    // at returns a struct test **, a pointer where a struct test * is stored
    assert(*add1_same == add1);

    // can be added like this as well:
    struct test *add2 = malloc(sizeof(struct test));
    test_ctor(add2, 11, 0.6, "add2", &gpa);
    *dyn_test_ptr_emplace_back_slot(&arrlisttestptr) = add2;

    struct test **add2_same = dyn_test_ptr_at(&arrlisttestptr, 1);
    assert(*add2_same == add2);

    // one-liner with an init function that returns a malloced and constructed object
    *dyn_test_ptr_emplace_back_slot(&arrlisttestptr) = test_alloc_ctor(12, 0.7, "add3", &gpa);
    assert(arrlisttestptr.size == 3);

    dyn_test_ptr_deinit(&arrlisttestptr);
    printf("arraylist_ptr emplace_back_slot passed all tests.\n");
}

struct test_simple {
    size_t id;
    int a;
    float b;
};

static void test_simple_ctor(struct test_simple *t, int a, float b, size_t id) {
    //printf("Constructor called! Creating object named %s\n", objname);
    t->a = a;
    t->b = b;
    t->id = id;
}

static struct test_simple test_simple_ctor_by_val(int a, float b, size_t id) {
    struct test_simple t;
    t.a = a;
    t.b = b;
    t.id = id;
    return t;
}

static void test_simple_ptr_dtor(struct test_simple **t, struct Allocator *alloc) {
    if (!t || !*t) {
        return;
    }

    alloc->free(*t, sizeof(*t), alloc->ctx);
    *t = NULL;
}

ARRAYLIST_DYN(struct test_simple, test_simple)

static void test_simple_arraylist_dyn_push_back(void) {
    printf("Testing arraylist push_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test_simple arrlisttest = dyn_test_simple_init(gpa, NULL);

    struct test_simple add1;
    test_simple_ctor(&add1, 10, 0.5, 1);
    dyn_test_simple_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);

    struct test_simple add2;
    test_simple_ctor(&add2, 11, 0.6, 2);
    dyn_test_simple_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    struct test_simple add3;
    test_simple_ctor(&add3, 12, 0.7, 3);
    dyn_test_simple_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test_simple add4;
    test_simple_ctor(&add4, 13, 0.8, 4);
    dyn_test_simple_push_back(&arrlisttest, add4);
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);

    struct test_simple add5;
    test_simple_ctor(&add5, 14, 0.9, 5);
    dyn_test_simple_push_back(&arrlisttest, add5);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);

    dyn_test_simple_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test_simple add6;
    test_simple_ctor(&add6, 15, 1.0, 6);
    dyn_test_simple_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 11);

    struct test_simple add7;
    test_simple_ctor(&add7, 15, 1.0, 7);
    dyn_test_simple_push_back(&arrlisttest, add7);
    assert(arrlisttest.size == 7);
    assert(arrlisttest.capacity == 11);

    struct test_simple add8;
    test_simple_ctor(&add8, 15, 1.0, 8);
    dyn_test_simple_push_back(&arrlisttest, add8);
    assert(arrlisttest.size == 8);
    assert(arrlisttest.capacity == 11);

    struct test_simple add9;
    test_simple_ctor(&add9, 15, 1.0, 9);
    dyn_test_simple_push_back(&arrlisttest, add9);
    assert(arrlisttest.size == 9);
    assert(arrlisttest.capacity == 11);

    struct test_simple add10;
    test_simple_ctor(&add10, 15, 1.0, 10);
    dyn_test_simple_push_back(&arrlisttest, add10);
    assert(arrlisttest.size == 10);
    assert(arrlisttest.capacity == 11);

    struct test_simple add11;
    test_simple_ctor(&add11, 15, 1.0, 11);
    dyn_test_simple_push_back(&arrlisttest, add11);
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);

    struct test_simple add12;
    test_simple_ctor(&add12, 15, 1.0, 12);
    dyn_test_simple_push_back(&arrlisttest, add12);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));

    dyn_test_simple_deinit(&arrlisttest);
    printf("arraylist push_back passed all tests.\n");
}

static void test_simple_arraylist_dyn_emplace_back_slot(void) {
    printf("Testing arraylist emplace_back_slot function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test_simple arrlisttest = dyn_test_simple_init(gpa, NULL);

    // ways that emplace_back can be used with arraylist of values:

    // can be added like this, most efficient as it is constructed inside the container:
    struct test_simple *add1 = dyn_test_simple_emplace_back_slot(&arrlisttest);

    // emplace_back_slot may return NULL, needs to be checked in real scenarios
    if (!add1) {
        assert(false && "emplace back returned null");
    }

    test_simple_ctor(add1, 10, 0.5, 1);
    assert(arrlisttest.size == 1);

    struct test_simple *add1_same = dyn_test_simple_at(&arrlisttest, 0);
    assert(add1_same == add1);

    // can be added like this, copy into slot:
    struct test_simple add2;
    test_simple_ctor(&add2, 11, 0.6, 2);
    *dyn_test_simple_emplace_back_slot(&arrlisttest) = add2;
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    // can be added by assigning a struct literal, if the struct has simple fields:
    *dyn_test_simple_emplace_back_slot(&arrlisttest) =
        (struct test_simple) { .a = 111, .b = 11.11, .id = 11111 };

    // can be added with a constructor that returns a struct by value:
    *dyn_test_simple_emplace_back_slot(&arrlisttest) = test_simple_ctor_by_val(12, 0.7, 3);
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);

    // can fill slots individually:
    struct test_simple *add4 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    add4->a = 13;
    add4->b = 0.8;
    add4->id = 4;
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);
    struct test_simple *add4_same = dyn_test_simple_at(&arrlisttest, 4);
    assert(add4_same == add4);

    struct test_simple *add5 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add5, 14, 0.9, 5);
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 8);
    struct test_simple *add5_same = dyn_test_simple_at(&arrlisttest, 5);
    assert(add5_same == add5);

    dyn_test_simple_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test_simple *add6 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add6, 14, 0.9, 6);
    assert(arrlisttest.size == 7);
    assert(arrlisttest.capacity == 11);
    struct test_simple *add6_same = dyn_test_simple_at(&arrlisttest, 6);
    assert(add6_same == add6);

    struct test_simple *add7 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add7, 14, 0.9, 7);
    assert(arrlisttest.size == 8);
    struct test_simple *add7_same = dyn_test_simple_at(&arrlisttest, 7);
    assert(add7_same == add7);

    struct test_simple *add8 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add8, 14, 0.9, 8);
    assert(arrlisttest.size == 9);
    struct test_simple *add8_same = dyn_test_simple_at(&arrlisttest, 8);
    assert(add8_same == add8);

    struct test_simple *add9 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add9, 14, 0.9, 9);
    assert(arrlisttest.size == 10);
    struct test_simple *add9_same = dyn_test_simple_at(&arrlisttest, 9);
    assert(add9_same == add9);

    struct test_simple *add10 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add10, 14, 0.9, 10);
    assert(arrlisttest.size == 11);
    struct test_simple *add10_same = dyn_test_simple_at(&arrlisttest, 10);
    assert(add10_same == add10);

    struct test_simple *add11 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add11, 14, 0.9, 11);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == 22);
    struct test_simple *add11_same = dyn_test_simple_at(&arrlisttest, 11);
    assert(add11_same == add11);

    struct test_simple *add12 = dyn_test_simple_emplace_back_slot(&arrlisttest);
    test_simple_ctor(add12, 14, 0.9, 12);
    assert(arrlisttest.size == 13);
    assert(arrlisttest.capacity == (11 * 2));
    struct test_simple *add12_same = dyn_test_simple_at(&arrlisttest, 12);
    assert(add12_same == add12);

    dyn_test_simple_deinit(&arrlisttest);
    printf("arraylist emplace_back_slot passed all tests.\n");
}

// An example of a container of pointers of struct test_simple
ARRAYLIST_DYN(struct test_simple *, test_simple_ptr)

static void test_simple_arraylist_dyn_ptr_push_back(void) {
    printf("Testing arraylist_ptr push_back function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test_simple_ptr arrlisttestptr = dyn_test_simple_ptr_init(gpa, test_simple_ptr_dtor);

    struct test_simple *add1 = malloc(sizeof(struct test_simple));
    test_simple_ctor(add1, 10, 0.5, 1);

    struct test_simple *add2 = malloc(sizeof(struct test_simple));
    test_simple_ctor(add2, 11, 0.6, 2);

    struct test_simple *add3 = malloc(sizeof(struct test_simple));

    *add3 = test_simple_ctor_by_val(12, 0.7, 3);

    struct test_simple *add4 = malloc(sizeof(struct test_simple));
    test_simple_ctor(add4, 13, 0.8, 4);

    struct test_simple *add5 = malloc(sizeof(struct test_simple));
    test_simple_ctor(add5, 14, 0.9, 5);

    dyn_test_simple_ptr_push_back(&arrlisttestptr, add1);
    assert(arrlisttestptr.size == 1);
    dyn_test_simple_ptr_push_back(&arrlisttestptr, add2);
    assert(arrlisttestptr.size == 2);
    assert(arrlisttestptr.capacity == 2);
    dyn_test_simple_ptr_push_back(&arrlisttestptr, add3);
    assert(arrlisttestptr.size == 3);
    assert(arrlisttestptr.capacity == 4);
    dyn_test_simple_ptr_push_back(&arrlisttestptr, add4);
    assert(arrlisttestptr.size == 4);
    assert(arrlisttestptr.capacity == 4);
    dyn_test_simple_ptr_push_back(&arrlisttestptr, add5);
    assert(arrlisttestptr.size == 5);
    assert(arrlisttestptr.capacity == 8);

    dyn_test_simple_ptr_reserve(&arrlisttestptr, 11);
    assert(arrlisttestptr.capacity == 11);

    dyn_test_simple_ptr_deinit(&arrlisttestptr);
    // free(add1); // double-free, arraylist owns the pointers to it and frees it

    // add1 is still not null, but points to freed memory, the dtor passed to arraylist sets the
    // freed pointer to null, as the arraylist shallow copies, so dtor will set only the copy
    // inside it to null
    assert(add1);
    assert(!arrlisttestptr.data);
    printf("arraylist_ptr push_back passed all tests.\n");
}

static void test_simple_arraylist_dyn_ptr_emplace_back_slot(void) {
    printf("Testing arraylist_ptr emplace_back_slot function.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_test_simple_ptr arrlisttestptr = dyn_test_simple_ptr_init(gpa, test_simple_ptr_dtor);

    // ways that emplace_back can be used with arraylist of pointers to values:

    // can be added like this:
    struct test_simple *add1 = malloc(sizeof(struct test_simple));
    test_simple_ctor(add1, 10, 0.5, 1);

    // get the pointer to a slot where a struct test_simple pointer can be stored
    struct test_simple **slot_add1 = dyn_test_simple_ptr_emplace_back_slot(&arrlisttestptr);

    // store it
    *slot_add1 = add1;

    assert(arrlisttestptr.size == 1);

    struct test_simple **add1_same = dyn_test_simple_ptr_at(&arrlisttestptr, 0);
    // at returns a struct test_simple **, a pointer where a struct test_simple * is stored
    assert(*add1_same == add1);

    // can be added like this as well:
    struct test_simple *add2 = malloc(sizeof(struct test_simple));
    test_simple_ctor(add2, 11, 0.6, 2);
    *dyn_test_simple_ptr_emplace_back_slot(&arrlisttestptr) = add2;

    struct test_simple **add2_same = dyn_test_simple_ptr_at(&arrlisttestptr, 1);
    assert(*add2_same == add2);

    // one-liner with an init function that returns a malloced and constructed object
    *dyn_test_simple_ptr_emplace_back_slot(&arrlisttestptr) =
        malloc(sizeof(test_simple_ctor_by_val(12, 0.7, 3)));
    assert(arrlisttestptr.size == 3);

    dyn_test_simple_ptr_deinit(&arrlisttestptr);
    printf("arraylist_ptr emplace_back_slot passed all tests.\n");
}

ARRAYLIST_DYN(long, long)

static void test_arraylist_dyn_bufferoverflow(void) {
    printf("arraylist size_t buffer overflow test.\n");
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_long xs = dyn_long_init(gpa, 0);
    assert(dyn_long_reserve(&xs, 0x8000000000000001) == ARRAYLIST_ERR_OVERFLOW);
    dyn_long_push_back(&xs, 0);
    dyn_long_push_back(&xs, 0);

    dyn_long_deinit(&xs);
    printf("arraylist size_t buffer overflow test passed.\n");
}

void *my_malloc(size_t n, void *p) {
    return n ? 0 : p;
}
void *my_realloc(void *p, size_t s, size_t n, void *ctx) {
    return n ? 0 : p;
    (void)ctx;
    (void)s;
}
void my_free(void *p, size_t s, void *ctx) {
    (void)p;
    (void)s;
    (void)ctx;
}
Allocator alloc = { my_malloc, my_realloc, my_free, (void *)16 };

static void test_arraylist_dyn_allocating_zero(void) {
    printf("arraylist custom alloc allocating zero.\n");
    struct arraylist_dyn_long xs_alloc = dyn_long_init(alloc, 0);

    dyn_long_push_back(&xs_alloc, 0);

    dyn_long_deinit(&xs_alloc);
    printf("arraylist custom alloc allocating zero passed.\n");
}

static void test_arraylist_dyn_get_custom_allocator(void) {
    printf("test arraylist getting custom allocator.\n");
    struct arraylist_dyn_long xs_alloc = dyn_long_init(alloc, 0);

    assert(allocator_test_equality(&alloc, dyn_long_get_allocator(&xs_alloc)));
    assert(allocator_test_equality(&alloc, &xs_alloc.alloc)); // should be same as above

    dyn_long_deinit(&xs_alloc);
    printf("arraylist getting custom allocator passed.\n");
}

#define noop_dtor(ptr, alloc) (void)0

ARRAYLIST(long, long, noop_dtor)

static void test_passing_nullptr_to_functions(void) {
    printf("test passing NULL to functions.\n");
    // If using asserts instead of return codes in the library, then the following functions
    // will fail not here, but inside the library, at the first call to reserve
    assert(long_reserve(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(long_shrink_size(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(long_shrink_to_fit(NULL) == ARRAYLIST_ERR_NULL);
    assert(long_push_back(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(long_emplace_back_slot(NULL) == NULL);
    assert(long_insert_at(NULL, 0, 0) == ARRAYLIST_ERR_NULL);
    assert(long_pop_back(NULL) == ARRAYLIST_ERR_NULL);
    assert(long_remove_at(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(long_remove_from_to(NULL, 0, 0) == ARRAYLIST_ERR_NULL);
    assert(long_at(NULL, 0) == NULL);
    assert(long_begin(NULL) == NULL);
    assert(long_back(NULL) == NULL);
    assert(long_end(NULL) == NULL);
    assert(long_find(NULL, NULL, NULL) == NULL);
    assert(long_contains(NULL, NULL, NULL, NULL) == false);
    assert(long_size(NULL) == 0);
    assert(long_is_empty(NULL) == false);
    assert(long_capacity(NULL) == 0);
    assert(long_swap(NULL, NULL) == ARRAYLIST_ERR_NULL);
    assert(long_clear(NULL) == ARRAYLIST_OK);
    assert(long_deinit(NULL) == ARRAYLIST_OK);

    assert(dyn_long_reserve(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_shrink_size(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_shrink_to_fit(NULL) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_push_back(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_emplace_back_slot(NULL) == NULL);
    assert(dyn_long_insert_at(NULL, 0, 0) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_pop_back(NULL) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_remove_at(NULL, 0) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_remove_from_to(NULL, 0, 0) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_at(NULL, 0) == NULL);
    assert(dyn_long_begin(NULL) == NULL);
    assert(dyn_long_back(NULL) == NULL);
    assert(dyn_long_end(NULL) == NULL);
    assert(dyn_long_find(NULL, NULL, NULL) == NULL);
    assert(dyn_long_contains(NULL, NULL, NULL, NULL) == false);
    assert(dyn_long_size(NULL) == 0);
    assert(dyn_long_is_empty(NULL) == false);
    assert(dyn_long_capacity(NULL) == 0);
    assert(dyn_long_swap(NULL, NULL) == ARRAYLIST_ERR_NULL);
    assert(dyn_long_clear(NULL) == ARRAYLIST_OK);
    assert(dyn_long_deinit(NULL) == ARRAYLIST_OK);
    printf("test passing NULL to functions passed.\n");
}

int main(void) {
    test_arraylist_init_and_deinit();
    test_arraylist_reserve();
    test_arraylist_shrink_size();
    test_arraylist_shrink_to_fit();
    test_arraylist_push_back();
    test_arraylist_emplace_back_slot();
    test_arraylist_insert_at();
    test_arraylist_pop_back();
    test_arraylist_remove_at();
    test_arraylist_remove_from_to();
    test_arraylist_at();
    test_arraylist_begin();
    test_arraylist_back();
    test_arraylist_end();
    test_arraylist_find();
    test_arraylist_contains();
    test_arraylist_size();
    test_arraylist_is_empty();
    test_arraylist_capacity();
    test_arraylist_get_allocator_default();
    test_arraylist_swap();
    test_arraylist_qsort();
    test_arraylist_clear();

    test_arraylist_dyn_init_and_deinit();
    test_arraylist_dyn_reserve();
    test_arraylist_dyn_shrink_size();
    test_arraylist_dyn_shrink_to_fit();
    test_arraylist_dyn_push_back();
    test_arraylist_dyn_emplace_back_slot();
    test_arraylist_dyn_insert_at();
    test_arraylist_dyn_pop_back();
    test_arraylist_dyn_remove_at();
    test_arraylist_dyn_remove_from_to();
    test_arraylist_dyn_at();
    test_arraylist_dyn_begin();
    test_arraylist_dyn_back();
    test_arraylist_dyn_end();
    test_arraylist_dyn_find();
    test_arraylist_dyn_contains();
    test_arraylist_dyn_size();
    test_arraylist_dyn_is_empty();
    test_arraylist_dyn_capacity();
    test_arraylist_dyn_get_allocator_default();
    test_arraylist_dyn_swap();
    test_arraylist_dyn_qsort();
    test_arraylist_dyn_clear();

    test_simple_arraylist_dyn_push_back();
    test_simple_arraylist_dyn_emplace_back_slot();

    test_simple_arraylist_dyn_ptr_push_back();
    test_simple_arraylist_dyn_ptr_emplace_back_slot();

    //test_arraylist_dyn_insert_from_to();

    test_arraylist_dyn_ptr_push_back();
    test_arraylist_dyn_ptr_emplace_back_slot();

    test_passing_nullptr_to_functions();

    test_arraylist_dyn_bufferoverflow();
    test_arraylist_dyn_allocating_zero();
    test_arraylist_dyn_get_custom_allocator();

    return 0;
}
