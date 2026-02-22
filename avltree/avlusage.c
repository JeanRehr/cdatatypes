#include "allocator.h"
#include "avltree.h"
#include <assert.h>
#include <stdio.h>

AVLTREE_TYPE(int, ints_tree)
AVLTREE_DECL(int, ints_tree)
AVLTREE_IMPL(int, ints_tree, avltree_noop_deinit)

int comparator_ints(int *a, int *b) {
    if (*a == *b) return 0;
    if (*a < *b) return -1;
    return 1;
}

void inorder(struct avltree_node_ints_tree *root) {
    if (root == NULL) {
        return;
    }
    inorder(root->left);
    printf("%d\n", root->data);
    inorder(root->right);
}

int main(void) {
    struct Allocator alloc = allocator_get_default();
    struct avltree_ints_tree tree = ints_tree_init(alloc, comparator_ints);

    ints_tree_insert(&tree, 12);
    ints_tree_insert(&tree, 13);
    ints_tree_insert(&tree, 9);
    ints_tree_insert(&tree, 10);
    inorder(tree.root);
    puts("");
    ints_tree_remove(&tree, 10);
    ints_tree_remove(&tree, 9);
    inorder(tree.root);

    return 0;
}