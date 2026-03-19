#include "components/component_base.h"

#include <stdio.h>

// Aborting on these functions would be better
static void component_default_render(struct component_base *self) {
    if (!self) {
        fprintf(stderr, "component_base is null on default render call\n");
        return;
    }
    if (self->component_name) {
        fprintf(stderr, "Render not implemented for ui component [%s]\n", self->component_name);
    } else {
        // This should never happen, how would this function be initialized to a function pointer without a name?
        // unless manually assigning it to a component_base field 
        fprintf(stderr, "Render not implemented for ui component [undefined]\n");
    }
}

static void component_default_deinit(struct component_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        fprintf(stderr, "component_base is null on default deinit call\n");
        return;
    }

    (void)alloc;

    if ((*self_ptr)->component_name) {
        fprintf(stderr, "Deinit not implemented for ui component [%s]\n", (*self_ptr)->component_name);
    } else {
        // This should never happen, how would this function be initialized to a function pointer without a name?
        // unless manually assigning it to a component_base field 
        fprintf(stderr, "Deinit not implemented for ui component [undefined]\n");
    }
}

void component_base_init_default(struct component_base *self, const char *component_name, size_t allocated_size) {
    self->component_render = component_default_render;
    self->component_deinit = component_default_deinit;
    self->component_name = component_name;
    self->allocated_size = allocated_size;
}
