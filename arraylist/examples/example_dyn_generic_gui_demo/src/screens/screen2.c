#include "screens/screen2.h"
#include "components/button.h"
#include "components/textbox.h"
#include "screens/screen_base.h"

#include <stdio.h>

/* Base methods/functionality */
static void screen2_render(struct screen_base *self);
static void screen2_deinit(struct screen_base **self_ptr, struct Allocator *alloc);

/* Specific function of screen2 itself */
struct screen2 *screen2_init_ptr(int id, struct Allocator *alloc) {
    struct screen2 *self = alloc->malloc(sizeof(*self), alloc->ctx);

    // Initialize base methods and fields
    screen_base_init_default(&self->base, "screen2", sizeof(*self));
    self->base.screen_render = screen2_render;
    self->base.screen_deinit = screen2_deinit;
    self->base.components = dyn_components_init(*alloc, component_base_ptr_dtor); // Initialize components

    // Specific struct fields
    self->id = id;

    // Inserting UI components
    struct textbox *tb1 = textbox_init_ptr("First Name", 1, alloc);
    *dyn_components_emplace_back(&self->base.components) = &tb1->base;

    struct textbox *tb2 = textbox_init_ptr("Last Name", 2, alloc);
    *dyn_components_emplace_back(&self->base.components) = &tb2->base;

    struct button *button1 = button_init_ptr("Confirm", 3, alloc);
    *dyn_components_emplace_back(&self->base.components) = &button1->base;

    // Base component of each concrete component type is initialized in their respective init functions

    return self;
}

static void screen2_render(struct screen_base *self) {
    if (!self) {
        return;
    }

    // Cast from screen_base pointer to screen2 pointer to access its elements
    struct screen2 *screen2 = (struct screen2 *)self;
    printf("rendering screen2 id %d\r\n", screen2->id);

    for (size_t i = 0; i < self->components.size; ++i) {
        // Get each component in the arraylist
        struct component_base *comp = self->components.data[i];
        // Render with their concrete implementation
        comp->component_render(comp);
    }
    printf("rendered components of screen2\r\n");
}

static void screen2_deinit(struct screen_base **self_ptr, struct Allocator *alloc) {
    if (!self_ptr || !*self_ptr) {
        return;
    }

    // Cast to the concrete type to access its elements
    struct screen2 *self = (struct screen2 *)*self_ptr;

    printf("cleaning up screen2 with ID: %d\r\n", self->id);

    // Cleanup of the arraylist of base component pointers
    dyn_components_deinit(&self->base.components);

    // Cleanup screen itself
    alloc->free(self, self->base.allocated_size, alloc->ctx);
    *self_ptr = NULL;
}
