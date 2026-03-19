#ifndef SCREEN1_H
#define SCREEN1_H

#include "screens/screen_base.h"

struct screen1 {
    struct screen_base base;
    int id;
};

struct screen1 *screen1_init_ptr(int id, struct Allocator *alloc);

#endif // SCREEN1_H
