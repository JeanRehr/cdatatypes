#ifndef ARRAYLIST_SCREENS_H
#define ARRAYLIST_SCREENS_H

#include "screens/screen_base.h"

/*
 * We are defining here an arraylist of pointer base interfaces of screens, the linkage has to be
 * overrided to extern to allow it to be used throughout all codebase by just including this header
 */

#define ARRAYLIST_LINKAGE extern
#include "arraylist.h"

// As there is no way to know ahead of time which screen we are looking at the Arraylist
// must be dynamic version (destructor has to be a function pointer)

// Arraylist of screen_base pointers
ARRAYLIST_TYPE_DYN(struct screen_base *, screens)
ARRAYLIST_DECL_DYN(struct screen_base *, screens)

// Every macro declared and the type used will be expanded here

// Function that essentially dispatches to the correct implementation given the screen
void screen_base_ptr_dtor(struct screen_base **base_ptr, struct Allocator *alloc);

#endif // ARRAYLIST_SCREENS_H