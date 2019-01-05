#include "pair.h"
#include "gc.h"

struct pair *pair_make(void *first, void *second)
{
    void **p = GC_MALLOC(sizeof(void *) * 2);
    p[0] = first;
    p[1] = second;
    return p;
}

void *pair_1st(struct pair *pair)
{
    return ((void **)pair)[0];
}

void *pair_2nd(struct pair *pair)
{
    return ((void **)pair)[1];
}
