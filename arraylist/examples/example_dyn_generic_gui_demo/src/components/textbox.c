#include "components/textbox.h"

#include <stdio.h>

/* Base methods/functionality */
static void textbox_render(struct component_base *self);
static void textbox_deinit_ptr(struct component_base **self_ptr, struct Allocator *alloc);

/* Specific function of button itself */
struct textbox *textbox_init_ptr(const char *title, int id, struct Allocator *alloc) {
    struct textbox *self = alloc->malloc(sizeof(*self), alloc->ctx);

    // initialize base methods and fields
    component_base_init_default(&self->base, "Textbox", sizeof(*self));
    self->base.component_render = textbox_render;
    self->base.component_deinit = textbox_deinit_ptr;

    // Specific struct fields
    self->title = title;
    self->id = id;

    return self;
}

// Private implementation details
static void textbox_render(struct component_base *self) {
    if (!self) {
        return;
    }

    // Cast to textbox to access its fields and render it
    struct textbox *tb = (struct textbox *)self;
    printf("rendering textbox with ID: %d and title: %s\r\n", tb->id, tb->title);
}

static void textbox_deinit_ptr(struct component_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        return;
    }

    printf("textbox deinit ptr called\r\n");

    struct component_base *self = *self_ptr;

    // We could cast this back to a struct textbox, for logging maybe:
    struct textbox *tb = (struct textbox *)self;
    printf("deinit textbox with ID: %d and title: %s\r\n", tb->id, tb->title);

    alloc->free(self, self->allocated_size, alloc->ctx);

    *self_ptr = NULL;
}
