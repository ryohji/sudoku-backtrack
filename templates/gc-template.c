#include "list.h"
#include "gc.h"

#include <stdio.h>

/**
 * Input a template (list of numbers), returns its possible templates (list of list).
 */
static void *generate(void *context, void *list);

int main()
{
    GC_INIT();
    struct list *templates = generate(NULL, list_make(NULL, 0));
    printf("%u\n", list_length(templates));
    return 0;
}

static struct list *grow(struct list *list);
static struct list *prune(struct list *lists);
static struct list *concat(struct list *lists);

void *generate(void *context, void *list)
{
    if (list_length(list) == 9)
    {
        return list_make(&list, 1);
    }
    else
    {
        struct list *nexts = grow(list);
        struct list *filtered = prune(nexts);
        struct list *lists = list_map(filtered, generate, NULL);
        return concat(lists);
    }
}

struct list *grow(struct list *list)
{
    struct list *nexts = list_make(NULL, 0);
    unsigned const N = list_length(list);
    unsigned n;
    for (n = 0; n != 9; n += 1)
    {
        nexts = list_append(nexts, list_append(list, (void *)(N * 9L + n)));
    }
    return nexts;
}

static bool prune_violates(void *context, void *value);

struct list *prune(struct list *lists)
{
    return list_filter(lists, prune_violates, NULL);
}

static void *column_number(void *context, void *value)
{
    return (void *)(((unsigned long)value) % 9);
}

static void *block_number(void *context, void *value)
{
    unsigned long index = (unsigned long)value;
    return (void *)(index / 3 % 3 + index / 27 * 3);
}

static bool eq(void *context, void *value)
{
    return context == value;
}

bool prune_violates(void *context, void *list)
{
    unsigned const N = list_length(list);
    if (N == 0 || N == 1)
    {
        return true;
    }
    else
    {
        struct list *column = list_map(list, column_number, NULL);
        if (list_length(list_filter(column, eq, list_value(column, N - 1))) == 1)
        {
            struct list *block = list_map(list, block_number, NULL);
            return list_length(list_filter(block, eq, list_value(block, N - 1))) == 1;
        }
        else
        {
            return false;
        }
    }
}

static void *append(void *context, void *aggregate, void *list);

struct list *concat(struct list *lists)
{
    return list_reduce(lists, list_make(NULL, 0), append, NULL);
}

void *append(void *context, void *aggregate, void *list)
{
    unsigned n;
    for (n = 0; n != list_length(list); n += 1)
    {
        aggregate = list_append(aggregate, list_value(list, n));
    }
    return aggregate;
}
