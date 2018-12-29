#include "bits.h"
#include "list.h"
#include "gc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct set;
struct set *set_make(unsigned *ns, unsigned elems);
struct set *set_union(const struct set *a, const struct set *b);
struct set *set_difference(const struct set *a, const struct set *b);
struct set *set_intersection(const struct set *a, const struct set *b);
// test if set B is the subset of set A.
bool set_subset(const struct set *a, const struct set *b);

struct list *fread_templates(FILE *fp);

int main()
{
    GC_INIT();

    struct list *templates = fread_templates(stdin);
    printf("%u\n", list_length(templates));
    fflush(stdout);

    return 0;
}

struct set *fread_set(FILE *fp);

struct array;
struct array *array_make();
unsigned array_size(struct array *array);
void **array_values(struct array *array);
void array_append(struct array *array, void *value);

struct list *fread_templates(FILE *fp)
{
    struct array *array = array_make();
    struct set *set;
    while (!feof(fp) && (set = fread_set(fp)))
    {
        array_append(array, set);
    }

    return list_make(array_values(array), array_size(array));
}

struct set *fread_set(FILE *fp)
{
    char buf[256];
    if (fgets(buf, sizeof(buf), fp))
    {
        unsigned ns[9];
        sscanf(buf, "%u %u %u %u %u %u %u %u %u", ns + 0, ns + 1, ns + 2, ns + 3, ns + 4, ns + 5, ns + 6, ns + 7, ns + 8);
        return set_make(ns, 9);
    }
    else
    {
        return NULL;
    }
}

// note: use `struct bits` as `struct set`.
struct set *set_make(unsigned *ns, unsigned elems)
{
    void *p = bits_make(81);
    unsigned *const end = ns + elems;
    for (; ns != end; ns += 1)
    {
        p = bits_set(p, *ns, true);
    }
    return p;
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

struct array
{
    unsigned caps;
    unsigned n;
    void **data;
};

struct array *array_make()
{
    return GC_MALLOC(sizeof(struct array));
}

unsigned array_size(struct array *array)
{
    return array->n;
}

void **array_values(struct array *array)
{
    return array->data;
}

void array_append(struct array *array, void *value)
{
    if (array->caps == array->n)
    {
        unsigned caps = array->caps ? array->caps * 2 : 16;
        array->data = GC_REALLOC(array->data, caps * sizeof(void *));
        array->caps = caps;
    }
    array->data[array->n] = value;
    array->n += 1;
}
