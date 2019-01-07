#include "set.h"
#include "bits.h"
#include "gc.h"

// note: use `struct bits` as `struct set`.
struct set *set_make(const unsigned *ns, unsigned elems)
{
    return (void *)bits_make(81, ns, elems);
}

struct set *set_union(const struct set *a, const struct set *b)
{
    return (void *)bits_or((const void *)a, (const void *)b);
}

struct set *set_intersection(const struct set *a, const struct set *b)
{
    return (void *)bits_and((const void *)a, (const void *)b);
}

struct set *set_difference(const struct set *a, const struct set *b)
{
    return set_intersection(a, (void *)bits_not((const void *)b));
}

// test if set B is the subset of set A.
bool set_subset(const struct set *a, const struct set *b)
{
    return bits_count((void *)set_difference(b, a)) == 0;
}

struct set_decomposed *set_decompose(const struct set *set)
{
    const void *bits = set;
    const unsigned n = bits_count(bits);
    unsigned *const p = GC_MALLOC_ATOMIC(sizeof(unsigned) * (n + 1)); // (length, array)

    unsigned *it = p;
    *it++ = n;
    unsigned i = 0;
    while (it != p + n + 1)
    {
        while (!bits_get(bits, i))
            i += 1;
        *it++ = i++;
    }
    return (void *)p;
}

unsigned decomposed_size(struct set_decomposed *decomposed)
{
    return ((unsigned *)decomposed)[0];
}

unsigned *decomposed_array(struct set_decomposed *decomposed)
{
    return ((unsigned *)decomposed) + 1;
}
