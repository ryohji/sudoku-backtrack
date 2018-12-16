#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const unsigned *constraint_row[9] = {
    (unsigned[9]){0, 1, 2, 3, 4, 5, 6, 7, 8},
    (unsigned[9]){9, 10, 11, 12, 13, 14, 15, 16, 17},
    (unsigned[9]){18, 19, 20, 21, 22, 23, 24, 25, 26},
    (unsigned[9]){27, 28, 29, 30, 31, 32, 33, 34, 35},
    (unsigned[9]){36, 37, 38, 39, 40, 41, 42, 43, 44},
    (unsigned[9]){45, 46, 47, 48, 49, 50, 51, 52, 53},
    (unsigned[9]){54, 55, 56, 57, 58, 59, 60, 61, 62},
    (unsigned[9]){63, 64, 65, 66, 67, 68, 69, 70, 71},
    (unsigned[9]){72, 73, 74, 75, 76, 77, 78, 79, 80},
};

static const unsigned *constraint_column[9] = {
    (unsigned[9]){0, 9, 18, 27, 36, 45, 54, 63, 72},
    (unsigned[9]){1, 10, 19, 28, 37, 46, 55, 64, 73},
    (unsigned[9]){2, 11, 20, 29, 38, 47, 56, 65, 74},
    (unsigned[9]){3, 12, 21, 30, 39, 48, 57, 66, 75},
    (unsigned[9]){4, 13, 22, 31, 40, 49, 58, 67, 76},
    (unsigned[9]){5, 14, 23, 32, 41, 50, 59, 68, 77},
    (unsigned[9]){6, 15, 24, 33, 42, 51, 60, 69, 78},
    (unsigned[9]){7, 16, 25, 34, 43, 52, 61, 70, 79},
    (unsigned[9]){8, 17, 26, 35, 44, 53, 62, 71, 80},
};

static const unsigned *constraint_block[9] = {
    (unsigned[9]){0, 1, 2, 9, 10, 11, 18, 19, 20},
    (unsigned[9]){3, 4, 5, 12, 13, 14, 21, 22, 23},
    (unsigned[9]){6, 7, 8, 15, 16, 17, 24, 25, 26},
    (unsigned[9]){27, 28, 29, 36, 37, 38, 45, 46, 47},
    (unsigned[9]){30, 31, 32, 39, 40, 41, 48, 49, 50},
    (unsigned[9]){33, 34, 35, 42, 43, 44, 51, 52, 53},
    (unsigned[9]){54, 55, 56, 63, 64, 65, 72, 73, 74},
    (unsigned[9]){57, 58, 59, 66, 67, 68, 75, 76, 77},
    (unsigned[9]){60, 61, 62, 69, 70, 71, 78, 79, 80},
};

struct list;
struct map
{
    unsigned (*const call)(void *context, unsigned value);
    void *context;
};

struct list *list_make();
void list_free(struct list *list);
void list_append(struct list *list, unsigned value);
void list_set(struct list *list, unsigned index, unsigned value);
void list_map(struct list *list, struct map f);
unsigned list_get(const struct list *list, unsigned index);
unsigned list_length(const struct list *list);
void list_grow(struct list *list);
void list_shrink(struct list *list);

struct ok
{
    bool (*const call)(void *context, const struct list *list);
    void *context;
};

struct out
{
    void (*const call)(void *context, const struct list *list);
    void *context;
};

struct next
{
    struct list *(*const call)(void *context, const struct list *list);
    void *context;
};

unsigned map_fn(void *context, unsigned value);
struct arguments
{
    struct list *list;
    struct ok ok;
    struct next next;
    struct out out;
};

void generate(struct arguments *args)
{
    if (args->ok.call(args->ok.context, args->list))
    {
        if (list_length(args->list) == 9)
        {
            args->out.call(args->out.context, args->list);
        }
        else
        {
            struct list *ns = args->next.call(args->next.context, args->list);
            list_grow(args->list);
            list_map(ns, (struct map){.call = map_fn, .context = args});
            list_shrink(args->list);
            list_free(ns);
        }
    }
}

// context points FILE*.
void out_fn(void *context, const struct list *list)
{
    const unsigned N = list_length(list);
    unsigned n;
    for (n = 0; n < N; n += 1)
    {
        fprintf(context, "%2d ", list_get(list, n));
    }
    fprintf(context, "\n");
}

int compar(const void *a, const void *b)
{
    return *(unsigned *)a - *(unsigned *)b;
}

const unsigned *includes(unsigned value, const unsigned **begin, const unsigned **end)
{
    const unsigned **it;
    for (it = begin; it != end && !bsearch(&value, *it, 9, sizeof(unsigned), compar); it += 1)
    {
        continue;
    }
    return *it;
}

bool elem(unsigned value, const unsigned *begin, const unsigned *end)
{
    return bsearch(&value, begin, end - begin, sizeof(unsigned), compar);
}

bool ok_fn(void *context, const struct list *list)
{
    const unsigned n = list_length(list);
    if (n == 0 || n == 1)
    {
        return true;
    }
    else
    {
        unsigned i;
        const unsigned x = list_get(list, n - 1);
        const unsigned *column = includes(x, constraint_column, constraint_column + 9);
        for (i = 0; i != n - 1 && !elem(list_get(list, i), column, column + 9); i += 1)
        {
            continue;
        }
        if (i == n - 1)
        {
            const unsigned *block = includes(x, constraint_block, constraint_block + 9);
            for (i = 0; i != n - 1 && !elem(list_get(list, i), block, block + 9); i += 1)
            {
                continue;
            }
            return i == n - 1;
        }
        else
        {
            return false;
        }
    }
}

struct list *next_fn(void *context, const struct list *list)
{
    struct list *const next = list_make();
    const unsigned *ns = constraint_row[list_length(list)];
    const unsigned *it;
    for (it = ns; it != ns + 9; it += 1)
    {
        list_append(next, *it);
    }
    return next;
}

unsigned map_fn(void *context, unsigned value)
{
    struct arguments *args = context;
    list_set(args->list, list_length(args->list) - 1, value);
    generate(args);
    return value;
}

int main()
{
    struct list *list = list_make();
    generate(&(struct arguments){
        .list = list,
        .ok = (struct ok){.call = ok_fn},
        .next = (struct next){.call = next_fn},
        .out = (struct out){.call = out_fn, .context = stdout},
    });
    list_free(list);
    return 0;
}

struct list
{
    unsigned n;
    unsigned *vs;
};

struct list *list_make()
{
    struct list *const list = malloc(sizeof(struct list));
    list->n = 0;
    list->vs = malloc(0);
    return list;
}

void list_free(struct list *list)
{
    free(list->vs);
    free(list);
}

unsigned list_length(const struct list *list)
{
    return list->n;
}

void list_append(struct list *list, unsigned value)
{
    list_grow(list);
    list->vs[list->n - 1] = value;
}

void list_grow(struct list *list)
{
    list->vs = realloc(list->vs, sizeof(unsigned) * ++list->n);
}

void list_shrink(struct list *list)
{
    list->vs = realloc(list->vs, sizeof(unsigned) * --list->n);
}

unsigned list_get(const struct list *list, unsigned index)
{
    return list->vs[index];
}

void list_set(struct list *list, unsigned index, unsigned value)
{
    list->vs[index] = value;
}

void list_map(struct list *list, struct map f)
{
    unsigned *it;
    for (it = list->vs; it != list->vs + list->n; it += 1)
    {
        *it = f.call(f.context, *it);
    }
}
