#include "list.h"
#include "gc.h"

#include <string.h>

struct list
{
    unsigned n;
    void *vs[0];
};

static struct list *create(unsigned elems)
{
    struct list *const list = GC_MALLOC(sizeof(struct list) + elems * sizeof(void *));
    list->n = elems;
    return list;
}

static void **head_of(struct list *list)
{
    return list->vs;
}

struct list *list_make(void *const *values, unsigned elems)
{
    struct list *const list = create(elems);
    memcpy(head_of(list), values, elems * sizeof(void *));
    return list;
}

struct list *list_append(const struct list *list, void *value)
{
    struct list *new = create(list->n + 1);
    memcpy(head_of(new), list->vs, list->n * sizeof(void *));
    memcpy(head_of(new) + list->n, &value, sizeof(void *));
    return new;
}

struct list *list_concatenate(const struct list *a, const struct list *b)
{
    struct list *list = create(a->n + b->n);
    memcpy(head_of(list), a->vs, a->n * sizeof(void *));
    memcpy(head_of(list) + a->n, b->vs, b->n * sizeof(void *));
    return list;
}

void *list_reduce(const struct list *list, void *aggregate, void *(*f)(void *context, void *aggregate, void *value), void *context)
{
    void *const *it;
    for (it = list->vs; it != list->vs + list->n; it += 1)
    {
        aggregate = f(context, aggregate, *it);
    }
    return aggregate;
}

unsigned list_length(const struct list *list)
{
    return list->n;
}

void *list_value(const struct list *list, unsigned index)
{
    return list->vs[index];
}

struct pair
{
    void *first;
    void *second;
};

static struct list *reduce_aux(const struct list *list, void *(*reduce_fn)(void *, void *, void*), struct pair pair);
static void *map_fn(void *pair, void *place, void *value);
static void *filter_fn(void *pair, void *place, void *value);

struct list *list_map(const struct list *list, void *(*f)(void *context, void *value), void *context)
{
    return reduce_aux(list, map_fn, (struct pair){.first = context, .second = f});
}

struct list *list_filter(const struct list *list, bool (*f)(void *context, void *value), void *context)
{
    return reduce_aux(list, filter_fn, (struct pair){.first = context, .second = f});
}

struct list *reduce_aux(const struct list *list, void *(*reduce_fn)(void *, void *, void*), struct pair pair)
{
    void **begin = GC_MALLOC(list_length(list) * sizeof(void *));
    void **end = list_reduce(list, begin, reduce_fn, &pair);
    return list_make(begin, end - begin);
}

void *map_fn(void *pair, void *place, void *value)
{
    void *context = ((struct pair *)pair)->first;
    void *(*f)(void *, void *) = ((struct pair *)pair)->second;
    *((void **)place) = f(context, value);
    return ((void **)place) + 1;
}

void *filter_fn(void *pair, void *place, void *value)
{
    void *context = ((struct pair *)pair)->first;
    bool (*f)(void *, void *) = ((struct pair *)pair)->second;
    *((void **)place) = value;
    return ((void **)place) + f(context, value);
}
