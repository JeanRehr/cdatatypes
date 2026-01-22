/**
 * @file example_fp1.c
 * @brief Example usage on how to use the arraylist.h
 * @details This example will show the function pointer version usage for simple types like ints
 */

#include <stdio.h>

// If wanted to use asserts:
// #define ARRAYLIST_USE_ASSERT 1
#include "arraylist.h"

// The following ARRAYLIST_DEF and ARRAYLIST_DECL may be declared on the header
ARRAYLIST_DEF_DYN(int, ints)
ARRAYLIST_DECL_DYN(int, ints)

// ARRAYLIST_DEF just defined a struct that hold the type int with "ints" appended on the name
// struct dyn_ints

// This must always be on a .c file
ARRAYLIST_IMPL_DYN(int, ints)

// Or with the all in one macro, must be in a .c file
// ARRAYLIST_DYN(int, ints)

// All three macros must have the same type T and name (int and ints in this case)

// The struct defined may be typedefed if wanted
// typedef struct dyn_ints arraylist_ints;

// To be used to find an int inside a container of ints
bool find_arraylist_int(int *t, void *find) {
    // Cast void pointer to intptr_t, cast to int
    if (*t == (int)(intptr_t)find) {
        return true;
    }
    return false;
}

// To test the sorting algorithm
bool comp_ascend(int *i, int *j) {
    return *i < *j;
}

// To test the sorting algorithm
bool comp_descend(int *i, int *j) {
    return *i > *j;
}

int main(void) {
    struct Allocator gpa = allocator_get_default();

    /* === EXAMPLE USAGE OF THE FUNCTION POINTER VERSION === */

    /* == Example with a simple type like int == */

    // Creates a zero initialized arraylist that holds ints named int_vec with default allocator and no destructor
    struct arraylist_dyn_ints int_vec = dyn_ints_init(&gpa, NULL);

    // May be created with capacity if wanted, but can't be tested for failure
    // if reserving fails due to buffer overflow or allocation failure, it will zero initialize
    // struct arraylist_dyn_ints int_vec = dyn_ints_init_with_capacity(NULL, NULL, 10);

    // Alternatively capacity may be reserved to minimize allocations
    dyn_ints_reserve(&int_vec, 16);

    // This way, reserve can be tested for allocation failure or buffer overflow if needed
    enum arraylist_error ret = dyn_ints_reserve(&int_vec, SIZE_MAX - 1); // buffer overflow
    if (ret != ARRAYLIST_OK) {
        printf("buffer overflow, capacity not reserved, ret value: %d\n", ret);
        printf("capacity is still %lu\n", int_vec.capacity);
    }

    /* == INSERTING VALUES == */
    dyn_ints_push_back(&int_vec, 1);
    dyn_ints_push_back(&int_vec, 2);
    dyn_ints_push_back(&int_vec, 3);

    // Prefer using emplace_back_slot to construct the object directly into the arraylist
    // It returns a pointer slot to the end of it, must be dereferenced to assign a value
    *dyn_ints_emplace_back_slot(&int_vec) = 10;
    *dyn_ints_emplace_back_slot(&int_vec) = 20;

    // Insert at an index, this will insert 1 to 10 from index 0 to 9
    for (size_t i = 0; i < 10; ++i) {
        dyn_ints_insert_at(&int_vec, i + 1, i);
    }

    *dyn_ints_emplace_back_slot(&int_vec) = 20;

    // may use an additional variable for testing before usage
    int *slot = dyn_ints_emplace_back_slot(&int_vec);
    assert(slot != NULL);

    *slot = 30;

    /* == READING/ACCESSING/ITERATING VALUES == */
    for (size_t i = 0; i < dyn_ints_size(&int_vec); i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    printf("\n");

    // May access arraylist fields directly
    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    printf("\n");

    // at() function returns a pointer, so to get the value int, it must be dereferenced
    printf("int_vec value at first position = %d\n", *dyn_ints_at(&int_vec, 0));
    printf("int_vec value at last position = %d\n", *dyn_ints_at(&int_vec, int_vec.size - 1));

    // May access the data directly if wanted
    printf("int_vec value at position 2 = %d\n", int_vec.data[2]);

    printf("\n");

    // May also use iterators and pointer arithmetic to read values
    for (int *it = dyn_ints_begin(&int_vec); it < dyn_ints_end(&int_vec); ++it) {
        printf("value %d\n", *it);
    }

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    /* == REMOVING VALUES == */

    // Removes the last element
    dyn_ints_pop_back(&int_vec);

    // If you want to get the last element before removing:
    int *last_elem = dyn_ints_back(&int_vec);
    printf("last elem before removing: %d\n", *last_elem);
    dyn_ints_pop_back(&int_vec);

    // Removes a single value at an index
    dyn_ints_remove_at(&int_vec, 0);

    // Will Remove values from position 1 to last index, only index 0 will remain
    dyn_ints_remove_from_to(&int_vec, 1, int_vec.size - 1);
    printf("arraylist size after removing from 1 to (size - 1): %lu\n", int_vec.size);
    printf("last value remaining: %d\n", *dyn_ints_begin(&int_vec));

    /* == OTHER FUNCTIONS == */

    // shrink to fit will reallocate capacity to fit the size, freeing up memory and not removing elements
    printf("Capacity reserved before shrink_to_fit(): %lu\n", dyn_ints_capacity(&int_vec));
    dyn_ints_shrink_to_fit(&int_vec);
    printf("Capacity reserved after shrink_to_fit(): %lu\n", int_vec.capacity);

    // Inserting elements again
    for (size_t i = 0; i < 10; ++i) {
        dyn_ints_insert_at(&int_vec, 0, i);
    }
    // Capacity and size after reinserting some elements:
    printf("Capacity again after reinstering elements: %lu\n", dyn_ints_capacity(&int_vec));
    printf("Size again after reinstering elements: %lu\n", dyn_ints_size(&int_vec));

    // shrink_size will shrink size to fit the size passed, removing elements, does not reallocate
    dyn_ints_shrink_size(&int_vec, 5);
    printf("Capacity after shrink_size(): %lu\n", dyn_ints_capacity(&int_vec));
    printf("Size after after shrink_size(): %lu\n", dyn_ints_size(&int_vec));

    // Tests if the arraylist is empty
    if (dyn_ints_is_empty(&int_vec) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Clear the arraylist of all elements, does not reallocate, only remove elements
    dyn_ints_clear(&int_vec);
    if (dyn_ints_is_empty(&int_vec) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Gets the allocator if needed, it will be the default allocator in this case
    dyn_ints_get_allocator(&int_vec);

    // Swaps arraylists

    struct arraylist_dyn_ints other = dyn_ints_init(&gpa, NULL);
    *dyn_ints_emplace_back_slot(&other) = 1000;
    *dyn_ints_emplace_back_slot(&other) = 2000;

    // Other size before swap:
    printf("Other arraylist size before swap(): %lu\n", dyn_ints_size(&other));
    printf("Original arraylist size before swap(): %lu\n", dyn_ints_size(&int_vec));

    dyn_ints_swap(&int_vec, &other);

    printf("Other arraylist size after swap(): %lu\n", dyn_ints_size(&other));
    printf("Original arraylist size after swap(): %lu\n", dyn_ints_size(&int_vec));

    dyn_ints_deinit(&other);
    /* == FIND AND CONTAINS == */

    // Find returns the reference if found, end if not
    // A function must be provided so that it knows how and what to find
    int *not_foundref = dyn_ints_find(&int_vec, find_arraylist_int, (void *)10);

    // If value is not found, it will return the end of the arraylist, dereferencing it is UB
    if (not_foundref == dyn_ints_end(&int_vec)) {
        printf("not found\n");
    }

    int *foundref = dyn_ints_find(&int_vec, find_arraylist_int, (void *)1000);
    if (foundref != dyn_ints_end(&int_vec)) {
        printf("found, value is: %d\n", *foundref);
    }

    // Contains returns false or true, and an optional index where it was found
    size_t index = 0;
    bool found = dyn_ints_contains(&int_vec, find_arraylist_int, (void *)2000, &index);

    if (found) {
        printf("value found at index %lu\n", index);
    }

    /* == SORT FUNCTION == */
    dyn_ints_clear(&int_vec);
    dyn_ints_reserve(&int_vec, 5);

    *dyn_ints_emplace_back_slot(&int_vec) = 3;
    *dyn_ints_emplace_back_slot(&int_vec) = 5;
    *dyn_ints_emplace_back_slot(&int_vec) = 1;
    *dyn_ints_emplace_back_slot(&int_vec) = -2;
    *dyn_ints_emplace_back_slot(&int_vec) = 6;

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    dyn_ints_qsort(&int_vec, comp_ascend);

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    dyn_ints_qsort(&int_vec, comp_descend);

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    // Must be called when done
    dyn_ints_deinit(&int_vec);
}