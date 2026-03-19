#include "textbox.h"
#include "component_base.h"
#include <stdio.h>

/* Base methods/functionality */
static void textbox_render(struct component_base *self);
static void textbox_deinit_ptr(struct component_base **self_ptr, struct Allocator *alloc);

/* Specific function of button itself */
struct textbox *textbox_init_ptr(const char *title, int id, struct Allocator *alloc) {
    struct textbox *self = alloc->malloc(sizeof(*self), alloc->ctx);

    // initialize base methods and fields
    self->base.component_render = textbox_render;
    self->base.component_deinit = textbox_deinit_ptr;
    self->base.component_name = "Textbox";
    self->base.allocated_size = sizeof(*self);

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
    printf("rendering textbox with ID: %d and title: %s\n", tb->id, tb->title);

}

static void textbox_deinit_ptr(struct component_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        return;
    }

    printf("textbox deinit ptr called\n");

    struct component_base *self = *self_ptr;

    // We could cast this back to a struct textbox, for logging maybe:
    struct textbox *tb = (struct textbox *)self;
    printf("deinit textbox with ID: %d and title: %s\n", tb->id, tb->title);

    alloc->free(self, self->allocated_size, alloc->ctx);

    *self_ptr = NULL;
}
