#ifndef SCREEN_BASE_H
#define SCREEN_BASE_H

#include "allocator.h"
#include "components/arraylist_components.h"

// All common behavior that a screen could have
struct screen_base {
    void (*screen_render)(struct screen_base *self);
    void (*screen_deinit)(struct screen_base **self_ptr, struct Allocator *alloc);
    struct arraylist_dyn_components components; // As every screen will have components, storing it on the abstract base class makes sense
    const char *screen_name;
    size_t allocated_size;
};

// Some say that interfaces/abstract base classes may only have methods (function pointers in this case), but there are trade-offs

// Prevent runtime crash due to uninitialized function pointers
// Implements only stubs
void screen_base_init_default(struct screen_base *self, const char *screen_name, size_t allocated_size);

#endif // SCREEN_BASE_H