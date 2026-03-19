#include "components/arraylist_components.h"

/*
 * We have to undef ARRAYLIST_LINKAGE here so that there is no linkage, it has to be the normal linkage
 */

#undef ARRAYLIST_LINKAGE
#define ARRAYLIST_LINKAGE
#include "arraylist.h"

ARRAYLIST_IMPL_DYN(struct component_base *, components)

// Every macro implementation of the type used will be expanded here

// Destructor function that frees component_base pointers
// This should be used in the init of arraylist of component_base pointers
void component_base_ptr_dtor(struct component_base **base_ptr, struct Allocator *alloc) {
    if (!base_ptr || !*base_ptr) {
        return;
    }

    struct component_base *base = *base_ptr;

    // Call the component's own destructor if it has one
    if (base->component_deinit) {
        base->component_deinit(base_ptr, alloc);
    } else {
        // if no destructor, attempt to free with stored size
        // this maybe may lead to partial/incorrect frees
        alloc->free(base, base->allocated_size, alloc->ctx);
        *base_ptr = NULL;
    }
}
