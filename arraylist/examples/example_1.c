/**
 * @file example_fp1.c
 * @brief Example usage on how to use the arraylist.h
 * @details This example will show the function pointer version usage for simple types like ints
 */

#include <stdio.h>

// If wanted to use asserts:
// #define ARRAYLIST_USE_ASSERT 1
// If wanted to use the arraylist_ prefix:
// #define ARRAYLIST_USE_PREFIX
#include "arraylist.h"

// The following ARRAYLIST_DEF and ARRAYLIST_DECL may be declared on the header
ARRAYLIST_DEF(int, ints)
ARRAYLIST_DECL(int, ints)

// ARRAYLIST_DEF just defined a struct that hold the type int with "ints" appended on the name
// struct arraylist_ints

// For simple and/or non pointer types, a destructor is not needed
// if using (void) as a parameter to the ARRAYLIST_IMPL macro, the compiler will warn about unused
// values inside the macro if compiling in Debug or Sanitizer mode, Release mode the compiler
// completely removes the code for these calls and will not complain, however, if the warnings
// bothers, defining a noop macro also works, like:
#define dummy_dtor(ptr, alloc) (void)0

// This must always be on a .c file
ARRAYLIST_IMPL(int, ints, dummy_dtor)

// Or with the all in one macro, must be in a .c file
// ARRAYLIST(int, ints, macro_dtor)

// All three macros must have the same type T and name (int and ints in this case)

// The struct defined may be typedefed if wanted
// typedef struct arraylist_ints arraylist_ints;

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

    // Creates a zero initialized arraylist that holds ints named int_vec with default allocator and no destructor
    struct arraylist_ints int_vec = ints_init(gpa);

    // Can create directly with allocator_get_default()
    // struct arraylist_ints int_vec = ints_init(allocator_get_default());

    // Capacity may be reserved to minimize allocations
    ints_reserve(&int_vec, 16);

    // This way, reserve can be tested for allocation failure or buffer overflow if needed
    enum arraylist_error ret = ints_reserve(&int_vec, SIZE_MAX - 1); // buffer overflow
    if (ret != ARRAYLIST_OK) {
        printf("buffer overflow, capacity not reserved, ret value: %d\n", ret);
        printf("capacity is still %lu\n", int_vec.capacity);
    }

    /* == INSERTING VALUES == */
    ints_push_back(&int_vec, 1);
    ints_push_back(&int_vec, 2);
    ints_push_back(&int_vec, 3);

    // Prefer using emplace_back_slot to construct the object directly into the arraylist
    // It returns a pointer slot to the end of it, must be dereferenced to assign a value
    *ints_emplace_back_slot(&int_vec) = 10;
    *ints_emplace_back_slot(&int_vec) = 20;

    // Insert at an index, this will insert 1 to 10 from index 0 to 9
    for (size_t i = 0; i < 10; ++i) {
        ints_insert_at(&int_vec, i + 1, i);
    }

    *ints_emplace_back_slot(&int_vec) = 20;

    // may use an additional variable for testing before usage
    int *slot = ints_emplace_back_slot(&int_vec);
    assert(slot != NULL);

    *slot = 30;

    /* == READING/ACCESSING/ITERATING VALUES == */
    for (size_t i = 0; i < ints_size(&int_vec); i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    printf("\n");

    // May access arraylist fields directly
    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    printf("\n");

    // at() function returns a pointer, so to get the value int, it must be dereferenced
    printf("int_vec value at first position = %d\n", *ints_at(&int_vec, 0));
    printf("int_vec value at last position = %d\n", *ints_at(&int_vec, int_vec.size - 1));

    // May access the data directly if wanted
    printf("int_vec value at position 2 = %d\n", int_vec.data[2]);

    printf("\n");

    // May also use iterators and pointer arithmetic to read values
    for (int *it = ints_begin(&int_vec); it < ints_end(&int_vec); ++it) {
        printf("value %d\n", *it);
    }

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    /* == REMOVING VALUES == */

    // Removes the last element
    ints_pop_back(&int_vec);

    // If you want to get the last element before removing:
    int *last_elem = ints_back(&int_vec);
    printf("last elem before removing: %d\n", *last_elem);
    ints_pop_back(&int_vec);

    // Removes a single value at an index
    ints_remove_at(&int_vec, 0);

    // Will Remove values from position 1 to last index, only index 0 will remain
    ints_remove_from_to(&int_vec, 1, int_vec.size - 1);
    printf("arraylist size after removing from 1 to (size - 1): %lu\n", int_vec.size);
    printf("last value remaining: %d\n", *ints_begin(&int_vec));

    /* == OTHER FUNCTIONS == */

    // shrink to fit will reallocate capacity to fit the size, freeing up memory and not removing elements
    printf("Capacity reserved before shrink_to_fit(): %lu\n", ints_capacity(&int_vec));
    ints_shrink_to_fit(&int_vec);
    printf("Capacity reserved after shrink_to_fit(): %lu\n", int_vec.capacity);

    // Inserting elements again
    for (size_t i = 0; i < 10; ++i) {
        ints_insert_at(&int_vec, 0, i);
    }
    // Capacity and size after reinserting some elements:
    printf("Capacity again after reinstering elements: %lu\n", ints_capacity(&int_vec));
    printf("Size again after reinstering elements: %lu\n", ints_size(&int_vec));

    // shrink_size will shrink size to fit the size passed, removing elements, does not reallocate
    ints_shrink_size(&int_vec, 5);
    printf("Capacity after shrink_size(): %lu\n", ints_capacity(&int_vec));
    printf("Size after after shrink_size(): %lu\n", ints_size(&int_vec));

    // Tests if the arraylist is empty
    if (ints_is_empty(&int_vec) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Clear the arraylist of all elements, does not reallocate, only remove elements
    ints_clear(&int_vec);
    if (ints_is_empty(&int_vec) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Gets the allocator if needed, it will be the default allocator in this case
    ints_get_allocator(&int_vec);

    // Swaps arraylists

    struct arraylist_ints other = ints_init(gpa);
    *ints_emplace_back_slot(&other) = 1000;
    *ints_emplace_back_slot(&other) = 2000;

    // Other size before swap:
    printf("Other arraylist size before swap(): %lu\n", ints_size(&other));
    printf("Original arraylist size before swap(): %lu\n", ints_size(&int_vec));

    ints_swap(&int_vec, &other);

    printf("Other arraylist size after swap(): %lu\n", ints_size(&other));
    printf("Original arraylist size after swap(): %lu\n", ints_size(&int_vec));

    ints_deinit(&other);
    /* == FIND AND CONTAINS == */

    // Find returns the reference if found, end if not
    // A function must be provided so that it knows how and what to find
    int *not_foundref = ints_find(&int_vec, find_arraylist_int, (void *)10);

    // If value is not found, it will return the end of the arraylist, dereferencing it is UB
    if (not_foundref == ints_end(&int_vec)) {
        printf("not found\n");
    }

    int *foundref = ints_find(&int_vec, find_arraylist_int, (void *)1000);
    if (foundref != ints_end(&int_vec)) {
        printf("found, value is: %d\n", *foundref);
    }

    // Contains returns false or true, and an optional index where it was found
    size_t index = 0;
    bool found = ints_contains(&int_vec, find_arraylist_int, (void *)2000, &index);

    if (found) {
        printf("value found at index %lu\n", index);
    }

    /* == SORT FUNCTION == */
    ints_clear(&int_vec);
    ints_reserve(&int_vec, 5);

    *ints_emplace_back_slot(&int_vec) = 3;
    *ints_emplace_back_slot(&int_vec) = 5;
    *ints_emplace_back_slot(&int_vec) = 1;
    *ints_emplace_back_slot(&int_vec) = -2;
    *ints_emplace_back_slot(&int_vec) = 6;

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    ints_qsort(&int_vec, comp_ascend);

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    ints_qsort(&int_vec, comp_descend);

    printf("\n");

    for (size_t i = 0; i < int_vec.size; i++) {
        printf("index %lu value %d\n", i, int_vec.data[i]);
    }

    // deep_clone/shallow_copy are the same thing for scalar/pod types
    struct arraylist_ints copied = ints_shallow_copy(&int_vec);
    printf("Size of the original list %lu\n", int_vec.size);
    printf("Size of the copied list %lu\n", copied.size);

    // Copied is an independent copy and may be changed independently
    *ints_emplace_back_slot(&copied) = 10000;
    printf("Size of the copied list after inserting 1 element %lu\n", copied.size);
    printf("Size of the original list again %lu\n", int_vec.size);

    // If using deep_clone on a scalar type, then the deep clone fn should be a simple assignment

    // Steal function will move the rhs to the lhs and leave lhs in an invalid state
    struct arraylist_ints stolen_list = ints_steal(&copied);

    // Must be called when done
    ints_deinit(&int_vec);
    ints_deinit(&stolen_list);
    // arraylists that were stealed do not need to be deinitialized
    //ints_deinit(&copied);
}