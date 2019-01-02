#include "bits.h"
#include "list.h"
#include "gc.h"

#include <stdbool.h>
#include <stdint.h>
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

void solve(struct list *templates, const char *sudoku);

int main()
{
    GC_INIT();

    struct list *templates = fread_templates(stdin);

    solve(templates, "*6*41*83*"
                     "7**8*****"
                     "5*19*****"
                     "*******7*"
                     "6*9***5*4"
                     "*1*******"
                     "*****47*9"
                     "*****8**1"
                     "*78*39*6*");

    return 0;
}

static void *number_make(unsigned value);
static unsigned as_number(void *value);
static struct list *flatten(struct list *lists);

struct pair
{
    void *first;
    void *second;
};

bool pass(void *context, void *value)
{
    struct set *placed = ((struct pair *)context)->first;
    if (set_subset(value, placed))
    {
        struct set *all = ((struct pair *)context)->second;
        struct set *intersection = set_intersection(set_difference(all, placed), value);
        return set_subset(set_make(NULL, 0), intersection);
    }
    else
    {
        return false;
    }
}

void *union_fn(void *context, void *aggregate, void *value)
{
    return set_union(aggregate, value);
}

void *mk_candidates(void *context, void *value)
{
    struct list *templates = ((struct pair *)context)->first;
    struct list *all = ((struct pair *)context)->second;
    return list_filter(templates, pass, &(struct pair){.first = value, .second = all});
}

void *pair_make(void *first, void *second)
{
    struct pair *p = GC_MALLOC(sizeof(struct pair));
    p->first = first;
    p->second = second;
    return p;
}

void *pair_1st(void *pair)
{
    return ((struct pair *)pair)->first;
}

void *pair_2nd(void *pair)
{
    return ((struct pair *)pair)->second;
}

void *tagged_make(void *tag, void *value)
{
    return pair_make(tag, value);
}

void *tagged_tag(void *tagged)
{
    return pair_1st(tagged);
}

void *tagged_value(void *tagged)
{
    return pair_2nd(tagged);
}

void *next_fn(void *context, void *aggregate, void *value)
{
    struct pair *pair = aggregate;
    if (pair->first == NULL)
    {
        pair->first = value;
    }
    else
    {
        if (list_length(tagged_value(value)) < list_length(tagged_value(pair->first)))
        {
            pair->second = list_append(pair->second, pair->first);
            pair->first = value;
        }
        else
        {
            pair->second = list_append(pair->second, value);
        }
    }
    return pair;
}

bool no_intersection(void *target, void *value)
{
    return set_subset(set_make(NULL, 0), set_intersection(target, value));
}

void *strip_having_intersectoin(void *target, void *value)
{
    return tagged_make(tagged_tag(value), list_filter(tagged_value(value), no_intersection, target));
}

void *solve_aux(void *result, void *remainder)
{
    if (list_length(remainder) == 0)
    {
        return list_make(&result, 1);
    }
    else
    {
        struct pair *next = list_reduce(remainder, pair_make(NULL, list_make(NULL, 0)), next_fn, NULL);
        struct pair *tagged_candidates = pair_1st(next);
        struct list *tagged_remainder = pair_2nd(next);

        void *const tag = tagged_tag(tagged_candidates);
        struct list *const candidates = tagged_value(tagged_candidates);
        void *nexts[list_length(candidates)];
        unsigned n;
        for (n = 0; n < list_length(candidates); n += 1)
        {
            void *candidate = list_value(candidates, n);
            void *shrinked = list_map(tagged_remainder, strip_having_intersectoin, candidate);
            nexts[n] = solve_aux(list_append(result, tagged_make(tag, candidate)), shrinked);
        }
        return flatten(list_make(nexts, n));
    }
}

void *print_tagged(void *context, void *value)
{
    printf("%u %s\n", as_number(tagged_tag(value)), bits_serialize(tagged_value(value)));
    return NULL;
}

void *print_list_of_tagged(void *context, void *value)
{
    list_map(value, print_tagged, context);
    return NULL;
}

void *zip_with_tag(void *context, void *aggregate, void *value)
{
    unsigned *p = context;
    *p += 1;
    return list_append(aggregate, tagged_make(number_make(*p), value));
}

void solve(struct list *templates, const char *sudoku)
{
    unsigned is[81];
    unsigned *pis[9] = {is + 0, is + 9, is + 18, is + 27, is + 36, is + 45, is + 54, is + 63, is + 72};
    const char *p;
    for (p = sudoku; p != sudoku + 81; p += 1)
    {
        unsigned n = *p - '1';
        if (n < 9)
        {
            *pis[n] = p - sudoku;
            pis[n] += 1;
        }
    }
    void *ss[9] = {
        set_make(is + 0, pis[0] - (is + 0)),
        set_make(is + 9, pis[1] - (is + 9)),
        set_make(is + 18, pis[2] - (is + 18)),
        set_make(is + 27, pis[3] - (is + 27)),
        set_make(is + 36, pis[4] - (is + 36)),
        set_make(is + 45, pis[5] - (is + 45)),
        set_make(is + 54, pis[6] - (is + 54)),
        set_make(is + 63, pis[7] - (is + 63)),
        set_make(is + 72, pis[8] - (is + 72)),
    };
    struct list *index = list_make(ss, 9);
    struct set *all = list_reduce(index, set_make(NULL, 0), union_fn, NULL);
    struct list *candidates = list_map(index, mk_candidates, &(struct pair){.first = templates, .second = all});
    struct list *tagged_candidates = list_reduce(candidates, list_make(NULL, 0), zip_with_tag, (unsigned[1]){0});
    struct list *result = solve_aux(list_make(NULL, 0), tagged_candidates);

    list_map(result, print_list_of_tagged, NULL);

    fflush(stdout);
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

void *number_make(unsigned value)
{
    return (void *)(uintptr_t)value;
}

unsigned as_number(void *value)
{
    return (uintptr_t)value;
}

static void *concat(void *context, void *aggregate, void *list);

struct list *flatten(struct list *lists)
{
    return list_reduce(lists, list_make(NULL, 0), concat, NULL);
}

void *concat(void *context, void *aggregate, void *list)
{
    return list_concatenate(aggregate, list);
}
