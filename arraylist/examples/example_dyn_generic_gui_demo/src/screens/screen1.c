#include "screens/screen1.h"
#include "components/button.h"
#include "components/textbox.h"
#include "screens/screen_base.h"

#include <stdio.h>

/* Base methods/functionality */
static void screen1_render(struct screen_base *self);
static void screen1_deinit(struct screen_base **self_ptr, struct Allocator *alloc);

/* Specific function of screen1 itself */
struct screen1 *screen1_init_ptr(int id, struct Allocator *alloc) {
    struct screen1 *self = alloc->malloc(sizeof(*self), alloc->ctx);

    // Initialize base methods and fields
    screen_base_init_default(&self->base, "self", sizeof(*self));
    self->base.screen_render = screen1_render;
    self->base.screen_deinit = screen1_deinit;
    self->base.components = dyn_components_init(*alloc, component_base_ptr_dtor); // Initialize components

    // Specific struct fields
    self->id = id;

    // Inserting UI components
    struct textbox *tb1 = textbox_init_ptr("Username", 1, alloc);
    *dyn_components_emplace_back(&self->base.components) = &tb1->base;

    struct button *button1 = button_init_ptr("Login", 2, alloc);
    *dyn_components_emplace_back(&self->base.components) = &button1->base;

    // Base component of each concrete component type is initialized in their respective init functions

    return self;
}

static void screen1_render(struct screen_base *self) {
    if (!self) {
        return;
    }

    // Cast from screen_base pointer to screen1 pointer to access its elements
    struct screen1 *screen1 = (struct screen1 *)self;
    printf("rendering screen1 id %d\r\n", screen1->id);

    for (size_t i = 0; i < self->components.size; ++i) {
        // Get each component in the arraylist
        struct component_base *comp = self->components.data[i];
        // Render with their concrete implementation
        comp->component_render(comp);
    }
    printf("rendered components of screen1\r\n");
}

static void screen1_deinit(struct screen_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        return;
    }

    // Cast to the concrete type to access its elements
    struct screen1 *self = (struct screen1 *)*self_ptr;

    printf("cleaning up screen1 with ID: %d\r\n", self->id);

    // Cleanup of the arraylist of base component pointers
    dyn_components_deinit(&self->base.components);

    // Cleanup screen itself
    alloc->free(self, self->base.allocated_size, alloc->ctx);
    *self_ptr = NULL;
}
