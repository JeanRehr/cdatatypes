#include "screens/screen_base.h"

#include <stdio.h>

// Aborting on these functions would be better
static void screen_default_render(struct screen_base *self) {
    if (!self) {
        fprintf(stderr, "screen_base is null on default render call\n");
        return;
    }
    if (self->screen_name) {
        fprintf(stderr, "Render not implemented for ui screen [%s]\n", self->screen_name);
    } else {
        // This should never happen, how would this function be initialized to a function pointer without a name?
        // unless manually assigning it to a screen_base field
        fprintf(stderr, "Render not implemented for ui screen [undefined]\n");
    }
}

static void screen_default_deinit(struct screen_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        fprintf(stderr, "screen_base is null on default deinit call\n");
        return;
    }

    (void)alloc;

    if ((*self_ptr)->screen_name) {
        fprintf(stderr, "Deinit not implemented for ui screen [%s]\n", (*self_ptr)->screen_name);
    } else {
        // This should never happen, how would this function be initialized to a function pointer without a name?
        // unless manually assigning it to a screen_base field
        fprintf(stderr, "Deinit not implemented for ui screen [undefined]\n");
    }
}

void screen_base_init_default(struct screen_base *self, const char *component_name, size_t allocated_size) {
    self->screen_render = screen_default_render;
    self->screen_deinit = screen_default_deinit;
    // Should self->components be initialized here? I don't think so as there is no Allocator struct to be used
    self->screen_name = component_name;
    self->allocated_size = allocated_size;
}
