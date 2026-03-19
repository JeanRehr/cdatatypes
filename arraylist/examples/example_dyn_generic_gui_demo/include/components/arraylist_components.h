#ifndef ARRAYLIST_COMPONENTS_H
#define ARRAYLIST_COMPONENTS_H

#include "components/component_base.h"

/*
 * We are defining here an arraylist of pointer base interfaces of components, the linkage has to be
 * overrided to extern to allow it to be used throughout all codebase by just including this header
 */

#define ARRAYLIST_LINKAGE extern
#include "arraylist.h"

// As there is no way to know ahead of time which component we are looking at (textbox, button, etc)
// The Arraylist must be dynamic version (destructor has to be a function pointer)

// Arraylist of component_base pointers
ARRAYLIST_TYPE_DYN(struct component_base *, components)
ARRAYLIST_DECL_DYN(struct component_base *, components)

// Every macro function declared and the struct type used will be expanded here

// Function that essentially dispatches to the correct implementation given the component
void component_base_ptr_dtor(struct component_base **base_ptr, struct Allocator *alloc);

#endif // ARRAYLIST_COMPONENTS_H
