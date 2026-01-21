/**
 * @file example.c
 * @brief Example usage on how to use the arraylist.h
 * @details This example will show the function pointer version usage for a non_pod
 *          type to pointer type (like struct T*)
 */

#include <stdio.h>

#include "arraylist.h"

struct non_pod {
    int *_number;
    int *add;
    int *sub;
};

// Returns a stack allocated struct itself, with allocated members on the heap
struct non_pod non_pod_init(Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod np = { 0 };
    np._number = alloc->malloc(sizeof(*np._number), alloc->ctx);
    *np._number = n;

    np.add = alloc->malloc(sizeof(*np.add), alloc->ctx);
    *np.add = add;

    np.sub = alloc->malloc(sizeof(*np.sub), alloc->ctx);
    *np.sub = sub;

    return np;
}

// Returns a heap allocated struct, with heap allocated members
struct non_pod *non_pod_init_alloc(Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod *np = alloc->malloc(sizeof(struct non_pod), alloc->ctx);
    np->_number = alloc->malloc(sizeof(*np->_number), alloc->ctx);
    *np->_number = n;

    np->add = alloc->malloc(sizeof(*np->add), alloc->ctx);
    *np->add = add;

    np->sub = alloc->malloc(sizeof(*np->sub), alloc->ctx);
    *np->sub = sub;

    return np;
}

// Frees members of the struct
void non_pod_deinit(struct non_pod *self, Allocator *alloc) {
    if (!self)
        return;

    alloc->free(self->_number, sizeof(self->_number), alloc->ctx);
    alloc->free(self->add, sizeof(self->add), alloc->ctx);
    alloc->free(self->sub, sizeof(self->sub), alloc->ctx);
}

// Frees the members of the struct and then the heap allocated struct itself
void non_pod_deinit_ptr(struct non_pod **self, Allocator *alloc) {
    if (!self || !*self)
        return;

    non_pod_deinit(*self, alloc);

    alloc->free(*self, sizeof(*self), alloc->ctx);

    *self = NULL;
}

#define non_pod_ptr_deinit_macro(non_pod_double_ptr, allocptr) \
do { \
    if (!non_pod_double_ptr || !*non_pod_double_ptr) { \
        break; \
    } \
    (allocptr)->free((*non_pod_double_ptr)->_number, sizeof(*(*non_pod_double_ptr)->_number), (allocptr)->ctx); \
    (allocptr)->free((*non_pod_double_ptr)->add, sizeof(*(*non_pod_double_ptr)->add), (allocptr)->ctx); \
    (allocptr)->free((*non_pod_double_ptr)->sub, sizeof(*(*non_pod_double_ptr)->sub), (allocptr)->ctx); \
    \
    (allocptr)->free(*non_pod_double_ptr, sizeof(*non_pod_double_ptr), (allocptr)->ctx); \
    *non_pod_double_ptr = NULL; \
} while(0)

ARRAYLIST(struct non_pod *, np, non_pod_ptr_deinit_macro)

// Example for sorting
bool non_pod_sort(struct non_pod **np1, struct non_pod **np2) {
    return *(*np1)->_number < *(*np2)->_number;
}

// Example for finding
bool non_pod_find(struct non_pod **self, void *find) {
    if (*(*self)->_number == (int)(intptr_t)find) {
        return true;
    }
    return false;
}

int main(void) {
    Allocator gpa = allocator_get_default();
    struct arraylist_np vec_np = arraylist_np_init(&gpa);
    arraylist_np_reserve(&vec_np, 40);

    // Inserting into it with emplace back slot
    // Note, slot given back must always be checked in real scenarios
    struct non_pod **slot1 = arraylist_np_emplace_back_slot(&vec_np);
    if (!slot1) {
        fprintf(stderr, "Failed to get the slot\n");
        return -1;
    }
    // A constructor that returns an allocated and initialized struct acts like new
    *slot1 = non_pod_init_alloc(&gpa, 90, 80, 70);
    if (!*slot1) {
        fprintf(stderr, "Allocation failure\n");
        return -1;
    }

    // One liner not testing slot or allocation
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, 940, 820, 710);

    // Inserting into it with emplace back slot with a constructor that does not
    // return an allocated struct itself
    struct non_pod *non_pod1 = gpa.malloc(sizeof(struct non_pod), NULL);
    // Can be tested for allocation failure
    if (!non_pod1) {
        fprintf(stderr, "allocation failure\n");
        return -1;
    }

    *non_pod1 = non_pod_init(&gpa, 999, 987, 781);

    // Warning: Not checking slot here
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod1;

    // Inserting into it with push_back one liner, not checking for allocation failure here
    arraylist_np_push_back(&vec_np, non_pod_init_alloc(&gpa, 464, 422, 180));

    // Inserting into it with push_back constructing first and then inserting
    // Warning: Not testing the malloc failure here
    struct non_pod *add1 = gpa.malloc(sizeof(struct non_pod), NULL);
    *add1 = non_pod_init(&gpa, 228, 421, 244);
    // Testing the result of push_back
    if (arraylist_np_push_back(&vec_np, add1) != ARRAYLIST_OK) {
        fprintf(stderr, "error occurred at line %d, file %s\n", __LINE__, __FILE__);
        return -1;
    }

    // Inserting into it with insert_at is basically the same as push_back

    // inserting some values
    for (size_t i = 0; i < 100; ++i) {
        *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, i, i * 3, i / 2);
    }

    // Rest of the functions are essentially the same thing

    // Reading/Acessing values:
    for (size_t i = 0; i < arraylist_np_size(&vec_np); i++) {
        printf("index %lu value %d\n", i, *vec_np.data[i]->_number);
    }

    printf("\n");

    // May access arraylist fields directly
    for (size_t i = 0; i < vec_np.size; i++) {
        printf("index %lu value %d\n", i, *vec_np.data[i]->_number);
    }

    printf("\n");

    // at() function returns a pointer, so to get the value of pointers to compound type,
    // it must be dereferenced at the top level, then dereference it again, and access its members
    printf("vec_np _number value at first position = %d\n", *(*arraylist_np_at(&vec_np, 0))->_number);
    printf("vec_np _number value at last position = %d\n", *(*arraylist_np_at(&vec_np, vec_np.size - 1))->_number);

    // May access the data directly if wanted
    // It is basically the same as the above, as the [] operator is just syntactic sugar
    // to dereference double pointers
    printf("vec_np _number value at position 2 = %d\n", *vec_np.data[2]->_number);

    printf("\n");

    // May also use iterators and pointer arithmetic to read values
    for (struct non_pod **it = arraylist_np_begin(&vec_np); it < arraylist_np_end(&vec_np); ++it) {
        printf("value %d\n", *(*it)->_number);
    }

    printf("\n");

    for (size_t i = 0; i < vec_np.size; i++) {
        printf("index %lu value %d\n", i, *vec_np.data[i]->_number);
    }

    /* == REMOVING VALUES == */

    // Removes the last element
    arraylist_np_pop_back(&vec_np);

    // If you want to get the last element before removing:
    struct non_pod **last_elem = arraylist_np_back(&vec_np);
    printf("last elem before removing: %d\n", *(*last_elem)->_number);
    arraylist_np_pop_back(&vec_np);

    // Removes a single value at an index
    arraylist_np_remove_at(&vec_np, 0);

    // Will Remove values from position 1 to last index, only index 0 will remain
    arraylist_np_remove_from_to(&vec_np, 1, vec_np.size - 1);
    printf("arraylist size after removing from 1 to (size - 1): %lu\n", vec_np.size);
    printf("last value remaining: %d\n", *(*arraylist_np_begin(&vec_np))->_number);

    /* == OTHER FUNCTIONS == */

    // shrink to fit will reallocate capacity to fit the size, freeing up memory and not removing elements
    printf("Capacity reserved before shrink_to_fit(): %lu\n", arraylist_np_capacity(&vec_np));
    arraylist_np_shrink_to_fit(&vec_np);
    printf("Capacity reserved after shrink_to_fit(): %lu\n", vec_np.capacity);

    // Inserting elements again
    for (size_t i = 0; i < 10; ++i) {
        arraylist_np_insert_at(&vec_np, 0, i);
    }
    // Capacity and size after reinserting some elements:
    printf("Capacity again after reinstering elements: %lu\n", arraylist_np_capacity(&vec_np));
    printf("Size again after reinstering elements: %lu\n", arraylist_np_size(&vec_np));

    // shrink_size will shrink size to fit the size passed, removing elements, does not reallocate
    arraylist_np_shrink_size(&vec_np, 5);
    printf("Capacity after shrink_size(): %lu\n", arraylist_np_capacity(&vec_np));
    printf("Size after after shrink_size(): %lu\n", arraylist_np_size(&vec_np));

    // Tests if the arraylist is empty
    if (arraylist_np_is_empty(&vec_np) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Clear the arraylist of all elements, does not reallocate, only remove elements
    arraylist_np_clear(&vec_np);
    if (arraylist_np_is_empty(&vec_np) == true) {
        printf("is empty\n");
    } else {
        printf("is NOT empty\n");
    }

    // Gets the allocator if needed, it will be the default allocator in this case
    arraylist_np_get_allocator(&vec_np);

    // Swaps arraylists

    struct arraylist_np other = arraylist_np_init(&gpa);
    *arraylist_np_emplace_back_slot(&other) = non_pod_init_alloc(&gpa, 10000, -80, -70);
    *arraylist_np_emplace_back_slot(&other) = non_pod_init_alloc(&gpa, 11992, -1000, 101010);

    // Other size before swap:
    printf("Other arraylist size before swap(): %lu\n", arraylist_np_size(&other));
    printf("Original arraylist size before swap(): %lu\n", arraylist_np_size(&vec_np));

    arraylist_np_swap(&vec_np, &other);

    printf("Other arraylist size after swap(): %lu\n", arraylist_np_size(&other));
    printf("Original arraylist size after swap(): %lu\n", arraylist_np_size(&vec_np));

    arraylist_np_deinit(&other);
    /* == FIND AND CONTAINS == */

    // Find returns the reference if found, end if not
    // A function must be provided so that it knows how and what to find
    struct non_pod **not_foundref = arraylist_np_find(&vec_np, non_pod_find, (void *)10);

    // If value is not found, it will return the end of the arraylist, dereferencing it is UB
    if (not_foundref == arraylist_np_end(&vec_np)) {
        printf("not found\n");
    }

    struct non_pod **foundref = arraylist_np_find(&vec_np, non_pod_find, (void *)1000);
    if (foundref != arraylist_np_end(&vec_np)) {
        printf("found, value is: %d\n", *(*foundref)->_number);
    }

    // Contains returns false or true, and an optional index where it was found
    size_t index = 0;
    bool found = arraylist_np_contains(&vec_np, non_pod_find, (void *)2000, &index);

    if (found) {
        printf("value found at index %lu\n", index);
    }

    /* == SORT FUNCTION == */
    arraylist_np_clear(&vec_np);
    arraylist_np_reserve(&vec_np, 5);

    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, 1, 80, 70);
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, 2, 80, 70);
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, -0, 80, 70);
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, -10, 80, 70);
    *arraylist_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, 10, 80, 70);

    for (size_t i = 0; i < vec_np.size; i++) {
        printf("index %lu value %d\n", i, *vec_np.data[i]->_number);
    }

    arraylist_np_qsort(&vec_np, non_pod_sort);

    printf("\n");

    for (size_t i = 0; i < vec_np.size; i++) {
        printf("index %lu value %d\n", i, *vec_np.data[i]->_number);
    }

    printf("\n");

    for (size_t i = 0; i < vec_np.size; i++) {
        printf("index %lu value %d\n", i, *vec_np.data[i]->_number);
    }

    arraylist_np_deinit(&vec_np);

    return 0;
}
