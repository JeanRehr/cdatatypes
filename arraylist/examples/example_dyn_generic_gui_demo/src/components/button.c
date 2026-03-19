#include "components/button.h"

#include <stdio.h>

/* Base methods/functionality */
static void button_render(struct component_base *self);
static void button_deinit(struct component_base **self_ptr, struct Allocator *alloc);

/* Specific function of button itself */
struct button *button_init_ptr(const char *title, int id, struct Allocator *alloc) {
    struct button *self = alloc->malloc(sizeof(*self), alloc->ctx);

    // Initialize methods and base fields
    self->base.component_render = button_render;
    self->base.component_deinit = button_deinit;
    self->base.allocated_size = sizeof(*self);
    self->base.component_name = "Button";

    // Specific button fields
    self->id = id;
    self->title = title;

    return self;
}

// Private implementation details 
static void button_render(struct component_base *self) {
    if (!self) {
        return;
    }
    
    // Cast to button to access its fields and render it
    struct button *button = (struct button *)self;
    printf("rendering button with ID: %d and title: %s\n", button->id, button->title);
}

static void button_deinit(struct component_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        return;
    }

    printf("button deinit ptr called\n");

    struct component_base *self = *self_ptr;

    // We could cast this back to a struct button, for logging maybe:
    struct button *button = (struct button *)self;
    printf("deinit button with ID: %d and title: %s\n", button->id, button->title);

    alloc->free(self, self->allocated_size, alloc->ctx);
    *self_ptr = NULL;
}
