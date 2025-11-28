/**
 * @file main.c
 */

#include "arraylist.h"

#include <assert.h>

struct test {
    char *objname;
    int *a;
    float *b;
};

void test_ctor(struct test *t, int a, float b, char *objname);
void test_print(struct test *t);
void test_dtor(struct test *t);

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

void test_arraylist_begin(void) {
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

void test_arraylist_back(void) {
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

void test_arraylist_end(void) {
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

void test_arraylist_size(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist end passed all tests.\n");
}

void test_arraylist_is_empty(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist end passed all tests.\n");
}

void test_arraylist_capacity(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist end passed all tests.\n");
}

void test_arraylist_swap(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist end passed all tests.\n");
}

void test_arraylist_clear(void) {
    printf("Testing arraylist end function.\n");
    struct arraylist_test arrlisttest = arraylist_test_init(nullptr, test_dtor);

    arraylist_test_deinit(&arrlisttest);
    assert(false);
    printf("arraylist end passed all tests.\n");
}

int main(void) {
    test_arraylist_init_and_deinit();
    test_arraylist_reserve();
    test_arraylist_shrink_size();
    test_arraylist_shrink_to_fit();
    test_arraylist_push_back();
    test_arraylist_emplace_back_slot();
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

    struct arraylist_test arrtest = arraylist_test_init(nullptr, test_dtor);
    struct arraylist_test arrtest1 = arraylist_test_init(nullptr, test_dtor);

    struct test add1;
    struct test add2;
    struct test add3;
    struct test add4;
    struct test add5;
    struct test add6;
    struct test add7;
    struct test add8;
    struct test add9;
    struct test add10;
    struct test add11;
    struct test add12;
    struct test add13;
    struct test add14;
    struct test add15;
    struct test add16;
    struct test add17;
    test_ctor(&add1, 10, 0.5, "add1");
    test_ctor(&add2, 11, 0.6, "add2");
    test_ctor(&add3, 12, 0.7, "add3");
    test_ctor(&add4, 13, 0.8, "add4");
    test_ctor(&add5, 14, 0.9, "add5");
    test_ctor(&add6, 15, 1.0, "add6");
    test_ctor(&add7, 15, 1.0, "add7");
    test_ctor(&add8, 15, 1.0, "add8");
    test_ctor(&add9, 15, 1.0, "add9");
    test_ctor(&add10, 15, 1.0, "add10");
    test_ctor(&add11, 15, 1.0, "add11");
    test_ctor(&add12, 15, 1.0, "add12");
    test_ctor(&add13, 15, 1.0, "add13");
    test_ctor(&add14, 15, 1.0, "add14");
    test_ctor(&add15, 15, 1.0, "add15");
    test_ctor(&add16, 15, 1.0, "add16");
    test_ctor(&add17, 15, 1.0, "add17");

    arraylist_test_reserve(&arrtest, 10);
    arraylist_test_reserve(&arrtest1, 20);

    arraylist_test_push_back(&arrtest, add1);
    arraylist_test_push_back(&arrtest, add2);
    arraylist_test_push_back(&arrtest, add3);
    arraylist_test_push_back(&arrtest, add4);
    arraylist_test_push_back(&arrtest, add5);
    arraylist_test_push_back(&arrtest, add6);
    arraylist_test_push_back(&arrtest, add7);
    arraylist_test_push_back(&arrtest1, add8);
    arraylist_test_push_back(&arrtest1, add9);
    arraylist_test_push_back(&arrtest1, add10);
    arraylist_test_push_back(&arrtest1, add11);
    arraylist_test_push_back(&arrtest1, add12);
    arraylist_test_push_back(&arrtest1, add13);
    arraylist_test_push_back(&arrtest1, add14);
    arraylist_test_push_back(&arrtest1, add15);
    arraylist_test_push_back(&arrtest1, add16);
    arraylist_test_push_back(&arrtest1, add17);

    printf("size after reserve and pushback = %lu\n", arrtest.size);

    printf("Capacity after pushback 17 arrtest.cap = %lu\n", arrtest.capacity);
    printf("size after pushback 17 arrtest.size = %lu\n", arrtest.size);

    printf("print arrtest before swap\n");
    for (size_t i = 0; i < arraylist_test_size(&arrtest); ++i) {
        test_print(&arrtest.data[i]);
    }

    printf("print arrtest1 before swap\n");
    for (size_t i = 0; i < arraylist_test_size(&arrtest1); ++i) {
        test_print(&arrtest1.data[i]);
    }

    arraylist_test_swap(&arrtest, &arrtest1);

    printf("print arrtest after swap\n");
    for (size_t i = 0; i < arraylist_test_size(&arrtest); ++i) {
        test_print(&arrtest.data[i]);
    }

    printf("print arrtest1 after swap\n");
    for (size_t i = 0; i < arraylist_test_size(&arrtest1); ++i) {
        test_print(&arrtest1.data[i]);
    }

    arraylist_test_deinit(&arrtest);
    arraylist_test_deinit(&arrtest1);
    return 0;
}

void test_ctor(struct test *t, int a, float b, char *objname) {
    //printf("Constructor called! Creating object named %s\n", objname);
    t->a = malloc(sizeof(a));
    t->b = malloc(sizeof(b));
    *t->a = a;
    *t->b = b;
    t->objname = objname;
}

void test_print(struct test *t) {
    //printf("t->a = %d\n", *t->a);
    //printf("t->b = %f\n", *t->b);
    printf("t->objname = %s\n", t->objname);
}

void test_dtor(struct test *t) {
    //printf("Destructor called for obj named %s!\n", t->objname);
    free(t->a);
    free(t->b);
}