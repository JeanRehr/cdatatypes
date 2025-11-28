/**
 * @file main.c
 */

#include "arraylist.h"

struct test {
    char *objname;
    int *a;
    float *b;
};

void test_ctor(struct test *t, int a, float b, char *objname);
void test_print(struct test *t);
void test_dtor(struct test *t);

ARRAYLIST(struct test, test)

int main(void) {
    struct arraylist_test arrtest = arraylist_test_init(nullptr, test_dtor);

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

    test_print(&add1);

    arraylist_test_reserve(&arrtest, 10);
    printf("Capacity after reserve 10 arrtest.cap = %lu\n", arrtest.capacity);

    arraylist_test_push_back(&arrtest, add1);
    arraylist_test_push_back(&arrtest, add2);
    arraylist_test_push_back(&arrtest, add3);
    arraylist_test_push_back(&arrtest, add4);
    arraylist_test_push_back(&arrtest, add5);
    arraylist_test_push_back(&arrtest, add6);
    arraylist_test_push_back(&arrtest, add7);
    arraylist_test_push_back(&arrtest, add8);
    arraylist_test_push_back(&arrtest, add9);
    arraylist_test_push_back(&arrtest, add10);
    arraylist_test_push_back(&arrtest, add11);
    arraylist_test_push_back(&arrtest, add12);
    arraylist_test_push_back(&arrtest, add13);
    arraylist_test_push_back(&arrtest, add14);
    arraylist_test_push_back(&arrtest, add15);
    arraylist_test_push_back(&arrtest, add16);
    arraylist_test_push_back(&arrtest, add17);
    printf("size after reserve and pushback = %lu\n", arrtest.size);

    printf("Capacity after pushback 17 arrtest.cap = %lu\n", arrtest.capacity);
    printf("size after pushback 17 arrtest.size = %lu\n", arrtest.size);

    arraylist_test_shrink_size(&arrtest, 10);

    printf("Capacity after shrink to 10 arrtest.cap = %lu\n", arrtest.capacity);
    printf("Size after shrink to 10 arrtest.size = %lu\n", arrtest.size);

    struct test *ind0 = arraylist_test_at(&arrtest, 0);

    test_print(ind0);

    arraylist_test_deinit(&arrtest);

    return 0;
}

void test_ctor(struct test *t, int a, float b, char *objname) {
    printf("Constructor called! Creating object named %s\n", objname);
    t->a = malloc(sizeof(a));
    t->b = malloc(sizeof(b));
    *t->a = a;
    *t->b = b;
    t->objname = objname;
}

void test_print(struct test *t) {
    printf("t->a = %d\n", *t->a);
    printf("t->b = %f\n", *t->b);
    printf("t->objname = %s\n", t->objname);
}

void test_dtor(struct test *t) {
    printf("Destructor called for obj named %s!\n", t->objname);
    free(t->a);
    free(t->b);
}