#include "list.h"
#include "gc.h"

#include <stdint.h>
#include <stdio.h>

/**
 * Input a template (list of numbers), returns its possible templates (list of list).
 */
static void *generate(void *context, void *list);

// make unsigned number object
static void *number_make(unsigned value);

// interpret value object as unsigned number
static unsigned as_number(void *value);

int main()
{
    GC_INIT();
    struct list *templates = generate(NULL, list_make(NULL, 0));
    printf("%u\n", list_length(templates));
    return 0;
}

static struct list *candidates(struct list *list);
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
        struct list *nexts = candidates(list);
        struct list *filtered = prune(nexts);
        struct list *lists = list_map(filtered, generate, context);
        return concat(lists);
    }
}

struct list *candidates(struct list *list)
{
    struct list *nexts = list_make(NULL, 0);
    unsigned const N = list_length(list);
    unsigned n;
    for (n = 0; n != 9; n += 1)
    {
        nexts = list_append(nexts, list_append(list, number_make(N * 9 + n)));
    }
    return nexts;
}

static bool follows_constraints(void *context, void *value);

struct list *prune(struct list *lists)
{
    return list_filter(lists, follows_constraints, NULL);
}

static bool no_duplicate(void *(*constraints)(void *context, void *value), struct list *list, void *context);
static void *column_number(void *context, void *value);
static void *block_number(void *context, void *value);

bool follows_constraints(void *context, void *list)
{
    switch (list_length(list))
    {
    case 0:
    case 1:
        return true;
    default:
        return no_duplicate(column_number, list, context) && no_duplicate(block_number, list, context);
    }
}

static bool eq(void *context, void *value);

bool no_duplicate(void *(*group)(void *context, void *value), struct list *list, void *context)
{
    struct list *number = list_map(list, group, context);
    void *the_last = list_value(number, list_length(number) - 1);
    return list_length(list_filter(number, eq, the_last)) == 1;
}

void *column_number(void *context, void *value)
{
    return number_make(as_number(value) % 9);
}

void *block_number(void *context, void *value)
{
    return number_make(as_number(value) / 3 % 3 + as_number(value) / 27 * 3);
}

bool eq(void *context, void *value)
{
    return as_number(context) == as_number(value);
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

void *number_make(unsigned value)
{
    return (void*)(uintptr_t)value;
}

unsigned as_number(void *value)
{
    return (uintptr_t)value;
}
