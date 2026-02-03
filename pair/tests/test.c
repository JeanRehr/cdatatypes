#include <stdio.h>
#include "pair.h"

struct non_pod {
    int id;
    char *name;
};

bool non_pod_pair_comp1(struct non_pod *np1, struct non_pod *np2) {
    return np1->id < np2->id;
}

bool non_pod_pair_comp2(double *db1, double *db2) {
    return *db1 < *db2;
}

#define noop_dtor(ptr, alloc) ((void)0)

PAIR_TYPE(int, int, intp)

PAIR_DECL(int, int, intp)

PAIR_IMPL(int, int, intp, noop_dtor, noop_dtor)

PAIR_TYPE(struct non_pod, double, np)
PAIR_DECL(struct non_pod, double, np)
PAIR_IMPL(struct non_pod, double, np, noop_dtor, noop_dtor)

int main(void) {
    struct pair_intp p = intp_init(1, 2);

    struct pair_intp p2 = {.first = 1, .second = 4};

    struct pair_np np1 = np_init((struct non_pod) {.id = 1, .name = "test"}, 0.2);
    struct pair_np np2 = np_init((struct non_pod) {.id = 3, .name = "test1"}, 0.5);

    printf("%d\n", p.first);
    return 0;
}