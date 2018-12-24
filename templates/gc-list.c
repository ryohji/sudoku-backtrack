#include "list.h"
#include "gc.h"

#include <string.h>

struct list
{
    unsigned n;
    void *const vs[0];
};

struct list *list_make(void *const *values, unsigned elems)
{
    struct list *const list = GC_MALLOC(sizeof(struct list) + elems * sizeof(void *));
    list->n = elems;
    memcpy((void **)list->vs, values, elems * sizeof(void *));
    return list;
}

struct list *list_append(const struct list *list, void *value)
{
    struct list *new = GC_MALLOC(sizeof(struct list) + (list->n + 1) * sizeof(void *));
    new->n = list->n + 1;
    memcpy((void **)new->vs, list->vs, list->n * sizeof(void *));
    ((void **)new->vs)[list->n] = value;
    return new;
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

static void *map_fn(void *context, void *aggregate, void *value)
{
    void *(*map)(void *context, void *value) = ((struct pair *)context)->second;
    return list_append(aggregate, map(((struct pair *)context)->first, value));
}

struct list *list_map(const struct list *list, void *(*f)(void *context, void *value), void *context)
{
    return list_reduce(list, list_make(0, 0), map_fn, &(struct pair){.first = context, .second = f});
}

static void *filter_fn(void *context, void *aggregate, void *value)
{
    bool (*filter)(void *context, void *value) = ((struct pair *)context)->second;
    return filter(((struct pair *)context)->first, value) ? list_append(aggregate, value) : aggregate;
}

struct list *list_filter(const struct list *list, bool (*f)(void *context, void *value), void *context)
{
    return list_reduce(list, list_make(0, 0), filter_fn, &(struct pair){.first = context, .second = f});
}
