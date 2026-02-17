#include "allocator.h"
#include "avltree.h"
#include <stdio.h>

AVLTREE_TYPE(int, ints_tree)
AVLTREE_DECL(int, ints_tree)
AVLTREE_IMPL(int, ints_tree, avltree_noop_deinit)

int comparator_ints(int *a, int *b) {
    if (*a == *b) return 0;
    if (*a < *b) return -1;
    return 1;
}

int main(void) {
    struct Allocator alloc = allocator_get_default();
    struct avltree_ints_tree tree = ints_tree_init(alloc, comparator_ints);

    printf("%p\n", tree.alloc.malloc);

    printf("%s\n", __FILE__);
    return 0;
}