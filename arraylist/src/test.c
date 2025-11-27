/**
 * @file main.c
 */

#include "arraylist.h"

ARRAYLIST(int, int)

int main(void) {


    struct arraylist_int test = {0};

    printf("test.capacity %lu\n", test.capacity);

    return 0;
}