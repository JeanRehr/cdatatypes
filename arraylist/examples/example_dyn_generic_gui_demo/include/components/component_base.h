#ifndef COMPONENT_BASE_H
#define COMPONENT_BASE_H

#include "allocator.h"

// All common behavior that a component could have
struct component_base {
    void (*component_render)(struct component_base *self);
    void (*component_deinit)(struct component_base **self_ptr, struct Allocator *alloc);
    const char *component_name;
    size_t allocated_size;
};

// Prevent runtime crash due to uninitialized function pointers
void component_base_init_default(struct component_base *self, const char *component_name, size_t allocated_size);

#endif // COMPONENT_BASE_H