#ifndef SCREEN2_H
#define SCREEN2_H

#include "screens/screen_base.h"

struct screen2 {
    struct screen_base base;
    int id;
};

struct screen2 *screen2_init_ptr(int id, struct Allocator *alloc);

#endif // SCREEN2_H
