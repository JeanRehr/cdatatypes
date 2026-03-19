#ifndef BUTTON_H
#define BUTTON_H

#include "components/component_base.h"

struct button {
    struct component_base base;
    const char *title;
    int id;
};

struct button *button_init_ptr(const char *title, int id, struct Allocator *alloc);

#endif // BUTTON_H