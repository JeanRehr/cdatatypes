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

ARRAYLIST_DYN(struct non_pod *, np)

// Returns a stack allocated struct itself, with allocated members on the heap
struct non_pod non_pod_init(struct Allocator *alloc, const int n, const int add, const int sub) {
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
struct non_pod *non_pod_init_alloc(struct Allocator *alloc, const int n, const int add, const int sub) {
    struct non_pod *np = alloc->malloc(sizeof(struct non_pod), alloc->ctx);
    np->_number = malloc(sizeof(*np->_number));
    *np->_number = n;

    np->add = malloc(sizeof(*np->add));
    *np->add = add;

    np->sub = malloc(sizeof(*np->sub));
    *np->sub = sub;

    return np;
}

// Frees members of the struct
void non_pod_deinit(struct non_pod *self, struct Allocator *alloc) {
    if (!self)
        return;

    alloc->free(self->_number, sizeof(self->_number), alloc->ctx);
    alloc->free(self->add, sizeof(self->add), alloc->ctx);
    alloc->free(self->sub, sizeof(self->sub), alloc->ctx);
}

// Frees the members of the struct and then the heap allocated struct itself
void non_pod_deinit_ptr(struct non_pod **self, struct Allocator *alloc) {
    if (!self || !*self)
        return;

    non_pod_deinit(*self, alloc);

    alloc->free(*self, sizeof(*self), alloc->ctx);

    *self = NULL;
}

// Example for sorting
bool non_pod_sort(const struct non_pod *const np1, const struct non_pod *const np2) {
    return np1->_number < np2->_number;
}

// Example for finding
bool non_pod_find(const struct non_pod *const self, void *find) {
    if (*self->_number == (int)(intptr_t)find) {
        return true;
    }
    return false;
}

int main(void) {
    struct Allocator gpa = allocator_get_default();
    struct arraylist_dyn_np vec_np = dyn_np_init(gpa, non_pod_deinit_ptr);
    dyn_np_reserve(&vec_np, 40);

    // Inserting into it with emplace back slot
    // Note, slot given back must always be checked in real scenarios
    struct non_pod **slot1 = dyn_np_emplace_back_slot(&vec_np);
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
    *dyn_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, 940, 820, 710);

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
    *dyn_np_emplace_back_slot(&vec_np) = non_pod1;

    // Inserting into it with push_back one liner, not checking for allocation failure here
    dyn_np_push_back(&vec_np, non_pod_init_alloc(&gpa, 464, 422, 180));

    // Inserting into it with push_back constructing first and then inserting
    // Warning: Not testing the malloc failure here
    struct non_pod *add1 = gpa.malloc(sizeof(struct non_pod), NULL);
    *add1 = non_pod_init(&gpa, 228, 421, 244);
    // Testing the result of push_back
    if (dyn_np_push_back(&vec_np, add1) != ARRAYLIST_OK) {
        fprintf(stderr, "error occurred at line %d, file %s\n", __LINE__, __FILE__);
        return -1;
    }

    // Inserting into it with insert_at is basically the same as push_back

    // inserting some values
    for (size_t i = 0; i < 100; ++i) {
        *dyn_np_emplace_back_slot(&vec_np) = non_pod_init_alloc(&gpa, i, i * 3, i / 2);
    }

    // Rest of the functions are essentially the same thing

    dyn_np_deinit(&vec_np);

    return 0;
}
