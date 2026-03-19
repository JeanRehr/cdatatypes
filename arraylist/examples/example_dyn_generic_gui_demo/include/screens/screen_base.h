#ifndef SCREEN_BASE_H
#define SCREEN_BASE_H

#include "allocator.h"

// All common behavior that a screen could have
struct screen_base {
    void (*screen_render)(struct screen_base *self);
    void (*screen_deinit)(struct screen_base **self_ptr, struct Allocator *alloc);
    const char *screen_name;
    size_t allocated_size;
};

// Prevent runtime crash due to uninitialized function pointers
void screen_base_init_default(struct screen_base *self, const char *screen_name, size_t allocated_size);

#endif // SCREEN_BASE_H