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

void inorder_ints(struct avltree_node_ints_tree *root) {
    if (root == NULL) {
        return;
    }
    inorder_ints(root->left);
    printf("%d\n", root->data);
    inorder_ints(root->right);
}

struct non_pod {
    int *data;
};

static inline struct non_pod non_pod_init(int data, struct Allocator *alloc) {
    struct non_pod np = { 0 };

    np.data = alloc->malloc(sizeof(int), alloc->ctx);
    if (np.data) {
        *np.data = data;
    }

    return np;
}

static inline int non_pod_construct(struct non_pod *self, void *args, struct Allocator *alloc) {
    self->data = alloc->malloc(sizeof(*self->data), alloc->ctx);
    if (self->data) {
        *self->data = *(int*)args;
        return 0;
    }
    return -1;
}

static inline void non_pod_deinit(struct non_pod *np, struct Allocator *alloc) {
    if (!np) {
        return;
    }

    if (np->data) {
        alloc->free(np->data, sizeof(*np->data), alloc->ctx);
        np->data = NULL;
    }
}

int comparator_non_pod(struct non_pod *a, struct non_pod *b) {
    if (*a->data == *b->data) return 0;
    if (*a->data < *b->data) return -1;
    return 1;
}

AVLTREE_TYPE(struct non_pod, np)
AVLTREE_DECL(struct non_pod, np)
AVLTREE_IMPL(struct non_pod, np, non_pod_deinit)

void inorder_np(struct avltree_node_np *root) {
    if (root == NULL) {
        return;
    }
    inorder_np(root->left);
    printf("%d\n", *root->data.data);
    inorder_np(root->right);
}

int main(void) {
    struct Allocator alloc = allocator_get_default();
    struct avltree_ints_tree tree = ints_tree_init(alloc, comparator_ints);

    ints_tree_insert(&tree, 12);
    ints_tree_insert(&tree, 13);
    ints_tree_insert(&tree, 9);
    ints_tree_insert(&tree, 10);
    inorder_ints(tree.root);
    puts("");
    ints_tree_remove(&tree, 10);
    ints_tree_remove(&tree, 9);
    ints_tree_remove(&tree, 13);
    ints_tree_remove(&tree, 12);
    inorder_ints(tree.root);

    struct avltree_np nptree = np_init(alloc, comparator_non_pod);

    np_insert(&nptree, non_pod_init(10, &nptree.alloc));

    struct non_pod tmp2 = non_pod_init(11, &nptree.alloc);
    np_insert(&nptree, tmp2);

    np_insert(&nptree, non_pod_init(12, &nptree.alloc));
    np_insert(&nptree, non_pod_init(13, &nptree.alloc));

    inorder_np(nptree.root);

    // Temporary key to remove here, as there was no intermediate variable to insert there
    int k = 10;
    struct non_pod key = { .data = &k };
    np_remove(&nptree, key);

    // most of the times in real world usage, tmp2 here would be out of scope, so using a temp struct to remove is better
    np_remove(&nptree, tmp2);

    k = 12;
    key.data = &k;
    np_remove(&nptree, key);

    k = 13;
    key.data = &k;
    np_remove(&nptree, key);

    puts("");

    int value = 10;
    np_emplace(&nptree, non_pod_construct, &value);
    value = 20;
    np_emplace(&nptree, non_pod_construct, &value);
    value = 50;
    np_emplace(&nptree, non_pod_construct, &value);

    inorder_np(nptree.root);

    k = 10;
    key.data = &k;
    np_remove(&nptree, key);
    k = 20;
    key.data = &k;
    np_remove(&nptree, key);
    k = 50;
    key.data = &k;
    np_remove(&nptree, key);

    return 0;
}