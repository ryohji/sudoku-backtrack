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
