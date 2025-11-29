/**
 * @file test.c
 */
#include "arraylist.h"

#include <assert.h>
#include <string.h>

struct test {
    char *objname;
    int *a;
    float *b;
};

static void test_ctor(struct test *t, int a, float b, char *objname);
static void test_print(struct test *t);
static void test_dtor(struct test *t);

ARRAYLIST(struct test, test)

static void test_arraylist_init_and_deinit(void) {
    printf("Testing arraylist init and deinit functions.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);
    assert(arrlisttest.destructor);
    assert(arrlisttest.alloc);
    assert(arrlisttest.capacity == 1);
    assert(arrlisttest.data);
    assert(arrlisttest.size == 0);

    arraylist_test_deinit(&arrlisttest);
    assert(!arrlisttest.data);

    arrlisttest = arraylist_test_init(nullptr, test_dtor);
    assert(arrlisttest.destructor);
    assert(arrlisttest.alloc);
    assert(arrlisttest.capacity == 1);
    assert(arrlisttest.data);
    assert(arrlisttest.size == 0);

    arraylist_test_deinit(&arrlisttest);
    assert(!arrlisttest.data);
    printf("arraylist init and deinit functions passed all tests.\n");
}

static void test_arraylist_reserve(void) {
    printf("Testing arraylist reserve function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    size_t cap = 10;

    arraylist_test_reserve(&arrlisttest, cap);
    assert(arraylist_test_capacity(&arrlisttest) == cap);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist reserve passed all tests.\n");
}

static void test_arraylist_shrink_size(void) {
    printf("Testing arraylist shrink_size function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;
    struct test add4;
    struct test add5;

    test_ctor(&add1, 10, 0.5, "add1");
    test_ctor(&add2, 11, 0.6, "add2");
    test_ctor(&add3, 12, 0.7, "add3");
    test_ctor(&add4, 13, 0.8, "add4");
    test_ctor(&add5, 14, 0.9, "add5");

    arraylist_test_push_back(&arrlisttest, add1);
    arraylist_test_push_back(&arrlisttest, add2);
    arraylist_test_push_back(&arrlisttest, add3);
    arraylist_test_push_back(&arrlisttest, add4);
    arraylist_test_push_back(&arrlisttest, add5);

    assert(arrlisttest.size == 5);

    size_t shrink = 3;

    arraylist_test_shrink_size(&arrlisttest, shrink);
    assert(arrlisttest.size == 3);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist shrink_size passed all tests.\n");
}

static void test_arraylist_shrink_to_fit(void) {
    printf("Testing arraylist shrink_to_fit function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;
    struct test add4;
    struct test add5;

    test_ctor(&add1, 10, 0.5, "add1");
    test_ctor(&add2, 11, 0.6, "add2");
    test_ctor(&add3, 12, 0.7, "add3");
    test_ctor(&add4, 13, 0.8, "add4");
    test_ctor(&add5, 14, 0.9, "add5");

    arraylist_test_reserve(&arrlisttest, 10);
    assert(arrlisttest.capacity == 10);

    arraylist_test_push_back(&arrlisttest, add1);
    arraylist_test_push_back(&arrlisttest, add2);
    arraylist_test_push_back(&arrlisttest, add3);
    arraylist_test_push_back(&arrlisttest, add4);
    arraylist_test_push_back(&arrlisttest, add5);

    assert(arrlisttest.size == 5);

    arraylist_test_shrink_to_fit(&arrlisttest);

    assert(arrlisttest.size == arrlisttest.capacity);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist shrink_to_fit passed all tests.\n");
}

static void test_arraylist_push_back(void) {
    printf("Testing arraylist push_back function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;
    struct test add4;
    struct test add5;

    test_ctor(&add1, 10, 0.5, "add1");
    test_ctor(&add2, 11, 0.6, "add2");
    test_ctor(&add3, 12, 0.7, "add3");
    test_ctor(&add4, 13, 0.8, "add4");
    test_ctor(&add5, 14, 0.9, "add5");

    arraylist_test_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);
    arraylist_test_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);
    arraylist_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);
    arraylist_test_push_back(&arrlisttest, add4);
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);
    arraylist_test_push_back(&arrlisttest, add5);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);

    arraylist_test_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test add6;
    struct test add7;
    struct test add8;
    struct test add9;
    struct test add10;
    struct test add11;
    struct test add12;

    test_ctor(&add6, 15, 1.0, "add6");
    test_ctor(&add7, 15, 1.0, "add7");
    test_ctor(&add8, 15, 1.0, "add8");
    test_ctor(&add9, 15, 1.0, "add9");
    test_ctor(&add10, 15, 1.0, "add10");
    test_ctor(&add11, 15, 1.0, "add11");
    test_ctor(&add12, 15, 1.0, "add12");

    arraylist_test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 11);
    arraylist_test_push_back(&arrlisttest, add7);
    assert(arrlisttest.size == 7);
    assert(arrlisttest.capacity == 11);
    arraylist_test_push_back(&arrlisttest, add8);
    assert(arrlisttest.size == 8);
    assert(arrlisttest.capacity == 11);
    arraylist_test_push_back(&arrlisttest, add9);
    assert(arrlisttest.size == 9);
    assert(arrlisttest.capacity == 11);
    arraylist_test_push_back(&arrlisttest, add10);
    assert(arrlisttest.size == 10);
    assert(arrlisttest.capacity == 11);
    arraylist_test_push_back(&arrlisttest, add11);
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);
    arraylist_test_push_back(&arrlisttest, add12);
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist push_back passed all tests.\n");
}

static void test_arraylist_emplace_back_slot(void) {
    printf("Testing arraylist emplace_back_slot function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test *add1 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add1, 10, 0.5, "add1");
    assert(arrlisttest.size == 1);
    struct test *add1_same = arraylist_test_at(&arrlisttest, 0);
    assert(add1_same == add1);

    struct test *add2 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add2, 11, 0.6, "add2");
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);
    struct test *add2_same = arraylist_test_at(&arrlisttest, 1);
    assert(add2_same == add2);

    struct test *add3 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add3, 12, 0.7, "add3");
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);
    struct test *add3_same = arraylist_test_at(&arrlisttest, 2);
    assert(add3_same == add3);

    struct test *add4 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add4, 13, 0.8, "add4");
    assert(arrlisttest.size == 4);
    assert(arrlisttest.capacity == 4);
    struct test *add4_same = arraylist_test_at(&arrlisttest, 3);
    assert(add4_same == add4);

    struct test *add5 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add5, 14, 0.9, "add5");
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);
    struct test *add5_same = arraylist_test_at(&arrlisttest, 4);
    assert(add5_same == add5);

    arraylist_test_reserve(&arrlisttest, 11);
    assert(arrlisttest.capacity == 11);

    struct test *add6 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add6, 14, 0.9, "add6");
    assert(arrlisttest.size == 6);
    assert(arrlisttest.capacity == 11);
    struct test *add6_same = arraylist_test_at(&arrlisttest, 5);
    assert(add6_same == add6);

    struct test *add7 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add7, 14, 0.9, "add7");
    assert(arrlisttest.size == 7);
    struct test *add7_same = arraylist_test_at(&arrlisttest, 6);
    assert(add7_same == add7);

    struct test *add8 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add8, 14, 0.9, "add8");
    assert(arrlisttest.size == 8);
    struct test *add8_same = arraylist_test_at(&arrlisttest, 7);
    assert(add8_same == add8);

    struct test *add9 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add9, 14, 0.9, "add9");
    assert(arrlisttest.size == 9);
    struct test *add9_same = arraylist_test_at(&arrlisttest, 8);
    assert(add9_same == add9);

    struct test *add10 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add10, 14, 0.9, "add10");
    assert(arrlisttest.size == 10);
    struct test *add10_same = arraylist_test_at(&arrlisttest, 9);
    assert(add10_same == add10);

    struct test *add11 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add11, 14, 0.9, "add11");
    assert(arrlisttest.size == 11);
    assert(arrlisttest.capacity == 11);
    struct test *add11_same = arraylist_test_at(&arrlisttest, 10);
    assert(add11_same == add11);

    struct test *add12 = arraylist_test_emplace_back_slot(&arrlisttest);
    test_ctor(add12, 14, 0.9, "add11");
    assert(arrlisttest.size == 12);
    assert(arrlisttest.capacity == (11 * 2));
    struct test *add12_same = arraylist_test_at(&arrlisttest, 11);
    assert(add12_same == add12);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist emplace_back_slot passed all tests.\n");
}

static void test_arraylist_insert_at(void) {
    printf("Testing arraylist insert_at function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3");
    arraylist_test_push_back(&arrlisttest, add3);

    assert(arrlisttest.size == 3);

    struct test add4;
    test_ctor(&add4, 13, 0.2, "add4");
        
    // Overflow should insert at last position
    arraylist_test_insert_at(&arrlisttest, add4, -1);
    assert(arrlisttest.size == 4);
    assert(strcmp(arrlisttest.data[arrlisttest.size - 1].objname, add4.objname) == 0);

    struct test add5;
    test_ctor(&add5, 15, 0.5, "add5");
    
    arraylist_test_insert_at(&arrlisttest, add5, 0);
    assert(arrlisttest.size == 5);
    assert(arrlisttest.capacity == 8);
    assert(strcmp(arrlisttest.data[0].objname, add5.objname) == 0);

    size_t add6_index_pos = 2;
    struct test add6;
    test_ctor(&add6, 16, 0.6, "add6");
    arraylist_test_insert_at(&arrlisttest, add6, add6_index_pos); // Between add1 and add2
    assert(arrlisttest.size == 6);
    assert(strcmp(arrlisttest.data[2].objname, add6.objname) == 0);


    size_t add7_index_pos = arrlisttest.size - 1;
    struct test add7;
    test_ctor(&add7, 17, 0.7, "add7");
    arraylist_test_insert_at(&arrlisttest, add7, add7_index_pos);
    assert(strcmp(arrlisttest.data[arrlisttest.size - 1].objname, add7.objname) == 0);
    assert(arrlisttest.size == 7);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist insert_at passed all tests.\n");
}

static void test_arraylist_pop_back(void) {
    printf("Testing arraylist pop_back function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;

    test_ctor(&add1, 10, 0.5, "add1");
    test_ctor(&add2, 11, 0.6, "add2");
    test_ctor(&add3, 12, 0.7, "add3");

    arraylist_test_push_back(&arrlisttest, add1);
    assert(arrlisttest.size == 1);
    assert(arrlisttest.capacity == 1);
    
    arraylist_test_push_back(&arrlisttest, add2);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);
    
    arraylist_test_pop_back(&arrlisttest);
    assert(arrlisttest.size == 1);
    assert(arrlisttest.capacity == 2);

    arraylist_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 2);
    assert(arrlisttest.capacity == 2);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist pop_back passed all tests.\n");
}

static void test_arraylist_at(void) {
    printf("Testing arraylist at function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3");
    arraylist_test_push_back(&arrlisttest, add3);

    struct test *add1_p = arraylist_test_at(&arrlisttest, 0);
    assert(add1_p == arrlisttest.data); // A pointer to the beginning of the array

    struct test *add2_p = arraylist_test_at(&arrlisttest, 1);
    assert(add2_p == arrlisttest.data + 1);

    struct test *add3_p = arraylist_test_at(&arrlisttest, 2);
    assert(add3_p == arrlisttest.data + 2);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist at passed all tests.\n");
}

static void test_arraylist_begin(void) {
    printf("Testing arraylist begin function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test *it_begin = arraylist_test_begin(&arrlisttest);
    assert(it_begin == arrlisttest.data);

    assert(it_begin + 1 == arrlisttest.data + 1);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist begin passed all tests.\n");
}

static void test_arraylist_back(void) {
    printf("Testing arraylist back function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);
    
    struct test *one_ele = arraylist_test_back(&arrlisttest);
    assert(one_ele == arrlisttest.data);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test *last_ele = arraylist_test_back(&arrlisttest);
    assert(last_ele == arrlisttest.data + 1);

    assert(last_ele - 1 == arrlisttest.data);
    
    arraylist_test_deinit(&arrlisttest);
    printf("arraylist back passed all tests.\n");
}

static void test_arraylist_end(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test *it_end = arraylist_test_end(&arrlisttest);
    assert(it_end == arrlisttest.data + 2); // it is a pointer to after the last element

    struct test *last_ele = arraylist_test_back(&arrlisttest);
    assert(it_end - 1 == last_ele);

    assert(it_end - 1 == arrlisttest.data + 1);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

static void test_arraylist_size(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);
    assert(arraylist_test_size(&arrlisttest) == 0);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);
    assert(arraylist_test_size(&arrlisttest) == 1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);
    assert(arraylist_test_size(&arrlisttest) == 2);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

static void test_arraylist_is_empty(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);
    assert(arraylist_test_is_empty(&arrlisttest));

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);
    assert(!arraylist_test_is_empty(&arrlisttest));

    arraylist_test_pop_back(&arrlisttest);
    assert(arraylist_test_is_empty(&arrlisttest));

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

static void test_arraylist_capacity(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);
    assert(arraylist_test_capacity(&arrlisttest) == 1);

    arraylist_test_reserve(&arrlisttest, 2);
    assert(arraylist_test_capacity(&arrlisttest) == 2);

    arraylist_test_reserve(&arrlisttest, 1);
    assert(arraylist_test_capacity(&arrlisttest) == 2);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);
    assert(arraylist_test_capacity(&arrlisttest) == 2);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);
    assert(arraylist_test_capacity(&arrlisttest) == 2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3");
    arraylist_test_push_back(&arrlisttest, add3);

    assert(arraylist_test_capacity(&arrlisttest) == 4);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
}

static void test_arraylist_swap(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);
    struct arraylist_test otherarr = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    // It seems that, after reallocation of data because of capacity, pointers might get invalidated
    // Spent quite a while on here because I was storing pointers right after the first push_back
    // struct test *arrlisttest_first_p = arraylist_test_begin(&arrlisttest);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3");
    arraylist_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);

    struct test *arrlisttest_first_p = arraylist_test_begin(&arrlisttest);

    struct test other1;
    test_ctor(&other1, 10, 0.5, "other1");
    arraylist_test_push_back(&otherarr, other1);

    struct test other2;
    test_ctor(&other2, 11, 0.6, "other2");
    arraylist_test_push_back(&otherarr, other2);
    assert(otherarr.size == 2);

    struct test *otherarr_first_p = arraylist_test_begin(&otherarr);

    assert(arrlisttest_first_p == arraylist_test_begin(&arrlisttest));
    assert(otherarr_first_p == arraylist_test_begin(&otherarr));

    arraylist_test_swap(&arrlisttest, &otherarr);
    assert(arrlisttest.size == 2);
    assert(otherarr.size == 3);

    assert(arrlisttest_first_p == arraylist_test_begin(&otherarr));
    assert(otherarr_first_p == arraylist_test_begin(&arrlisttest));

    size_t i = 0;
    for (struct test *it = arraylist_test_begin(&arrlisttest); it < arraylist_test_end(&arrlisttest); ++it) {
        test_print(&arrlisttest.data[i]);
        ++i;
    }

    arraylist_test_deinit(&arrlisttest);
    arraylist_test_deinit(&otherarr);
    printf("arraylist end passed all tests.\n");
}

static void test_arraylist_remove_at(void) {
    printf("Testing arraylist remove_at function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3");
    arraylist_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    struct test add4;
    test_ctor(&add4, 12, 0.7, "add4");
    arraylist_test_push_back(&arrlisttest, add4);

    struct test add5;
    test_ctor(&add5, 12, 0.7, "add5");
    arraylist_test_push_back(&arrlisttest, add5);

    struct test add6;
    test_ctor(&add6, 12, 0.7, "add6");
    arraylist_test_push_back(&arrlisttest, add6);
    assert(arrlisttest.size == 6);

    /*
    for (size_t j = 0; j < arrlisttest.size; ++j) {
        printf("index = %lu | ", j);
        test_print(&arrlisttest.data[j]);
    }
    */

    arraylist_test_remove_at(&arrlisttest, 3); //add4
    assert(arrlisttest.size == 5);

    arraylist_test_remove_at(&arrlisttest, 3); //add5 now
    assert(arrlisttest.size == 4);

    // Should overflow and remove last element
    arraylist_test_remove_at(&arrlisttest, -1); //add6
    assert(arrlisttest.size == 3);

    arraylist_test_remove_at(&arrlisttest, 0); //add1
    assert(arrlisttest.size == 2);
    
    arraylist_test_remove_at(&arrlisttest, 1); //add3
    assert(arrlisttest.size == 1);

    /*
    for (size_t i = 0; i < arrlisttest.size; ++i) {
        printf("index = %lu | ", i);
        test_print(&arrlisttest.data[i]);
    }
    */

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist remove_at passed all tests.\n");
}

static void test_arraylist_insert_from_to(void) {
    printf("Testing arraylist insert_from_to function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist insert_from_to passed all tests.\n");
}

static void test_arraylist_remove_from_to(void) {
    printf("Testing arraylist remove_from_to function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist remove_from_to passed all tests.\n");
}

static void test_arraylist_clear(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    test_ctor(&add1, 10, 0.5, "add1");
    arraylist_test_push_back(&arrlisttest, add1);

    struct test add2;
    test_ctor(&add2, 11, 0.6, "add2");
    arraylist_test_push_back(&arrlisttest, add2);

    struct test add3;
    test_ctor(&add3, 12, 0.7, "add3");
    arraylist_test_push_back(&arrlisttest, add3);
    assert(arrlisttest.size == 3);
    assert(arrlisttest.capacity == 4);

    arraylist_test_clear(&arrlisttest);
    assert(arrlisttest.size == 0);
    assert(arrlisttest.capacity == 4);

    arraylist_test_deinit(&arrlisttest);
    printf("arraylist end passed all tests.\n");
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
    test_arraylist_at();
    test_arraylist_begin();
    test_arraylist_back();
    test_arraylist_end();
    test_arraylist_size();
    test_arraylist_is_empty();
    test_arraylist_capacity();
    test_arraylist_swap();
    test_arraylist_clear();

    test_arraylist_remove_at();
    test_arraylist_insert_from_to();
    test_arraylist_remove_from_to();
    return 0;
}

static void test_ctor(struct test *t, int a, float b, char *objname) {
    //printf("Constructor called! Creating object named %s\n", objname);
    t->a = malloc(sizeof(a));
    t->b = malloc(sizeof(b));
    *t->a = a;
    *t->b = b;
    t->objname = objname;
}

static void test_print(struct test *t) {
    //printf("t->a = %d\n", *t->a);
    //printf("t->b = %f\n", *t->b);
    printf("t->objname = %s\n", t->objname);
}

static void test_dtor(struct test *t) {
    //printf("Destructor called for obj named %s!\n", t->objname);
    free(t->a);
    free(t->b);
}