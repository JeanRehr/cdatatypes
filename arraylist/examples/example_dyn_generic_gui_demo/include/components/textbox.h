#ifndef TEXBOX_H
#define TEXBOX_H

#include "components/component_base.h"

struct textbox {
    struct component_base base;
    const char *title;
    int id;
};

struct textbox *textbox_init_ptr(const char *title, int id, struct Allocator *alloc);

#endif // TEXBOX_H