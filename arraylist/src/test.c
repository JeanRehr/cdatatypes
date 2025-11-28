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
    test_ctor(&add1, 10, 0.5, "add1");

    test_print(&add1);

    arraylist_test_reserve(&arrtest, 10);
    printf("Capacity after reserve arrtest.cap = %lu\n", arrtest.capacity);

    arraylist_test_push_back(&arrtest, add1);
    printf("size after reserve pushback = %lu\n", arrtest.size);

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