#include "screens/arraylist_screens.h"

/*
 * We have to undef ARRAYLIST_LINKAGE here so that there is no linkage, it has to be the normal linkage
 */

#undef ARRAYLIST_LINKAGE
#define ARRAYLIST_LINKAGE
#include "arraylist.h"

ARRAYLIST_IMPL_DYN(struct screen_base *, screens)

// Every macro implementation of the type used will be expanded here

// Destructor function that frees screen_base pointers
// This should be used in the init of arraylist of screen_base pointers
void screen_base_ptr_dtor(struct screen_base **base_ptr, struct Allocator *alloc) {
    if (!base_ptr || !*base_ptr) {
        return;
    }

    struct screen_base *base = *base_ptr;

    // Call the screen's own destructor if it has one
    if (base->screen_deinit) {
        base->screen_deinit(base_ptr, alloc);
    } else {
        // if no destructor, attempt to free with stored size
        // this maybe may lead to partial/incorrect frees
        // should maybe instead abort
        alloc->free(base, base->allocated_size, alloc->ctx);
        *base_ptr = NULL;
    }
}