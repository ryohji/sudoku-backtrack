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

// return the array object which having numbers in the specified set.
void *set_decompose(const struct set *set);
unsigned decomposed_size(void *decomposed);
unsigned *decomposed_array(void *decomposed);

struct list *fread_templates(FILE *fp);

void *fput_string(void *context, void *value);
struct list *solve(struct list *templates, const char *sudoku);

int main()
{
    GC_INIT();

    list_map(solve(fread_templates(stdin),
                   "*6*41*83*"
                   "7**8*****"
                   "5*19*****"
                   "*******7*"
                   "6*9***5*4"
                   "*1*******"
                   "*****47*9"
                   "*****8**1"
                   "*78*39*6*"),
             fput_string, stdout);
    fflush(stdout);

    return 0;
}

void *fput_string(void *file, void *string)
{
    fputs(string, file);
    return NULL;
}

static struct list *convert_to_set(const char *sudoku);
static void *union_fn(void *context, void *aggregate, void *value);
static void *mk_candidates(void *context, void *placed);
static void *zip_with_tag(void *context, void *aggregate, void *value);
static void *solve_aux(void *context, void *value);
static void *tagged_set_to_string(void *context, void *value);

static void *pair_make(void *first, void *second);
static void *pair_1st(void *pair);
static void *pair_2nd(void *pair);

struct list *solve(struct list *templates, const char *sudoku)
{
    struct list *index = convert_to_set(sudoku);
    struct set *all = list_reduce(index, set_make(NULL, 0), union_fn, NULL);
    struct list *candidates = list_map(index, mk_candidates, pair_make(templates, all));
    struct list *tagged_candidates = list_reduce(candidates, list_make(NULL, 0), zip_with_tag, (unsigned[1]){0});
    struct list *results = solve_aux(NULL, pair_make(tagged_candidates, list_make(NULL, 0)));
    return list_map(results, tagged_set_to_string, NULL);
}

struct list *convert_to_set(const char *sudoku)
{
    unsigned is[81]; // devide into 9 partition, to hold index for each number placed.
    unsigned *end[9] = {is + 0, is + 9, is + 18, is + 27, is + 36, is + 45, is + 54, is + 63, is + 72};
    const char *p;
    for (p = sudoku; p != sudoku + 81; p += 1)
    {
        const unsigned n = *p - '1';
        if (n < 9)
            *end[n]++ = p - sudoku;
    }
#define S(n) set_make(is + 9 * n, end[n] - (is + 9 * n))
    return list_make((void *[9]){S(0), S(1), S(2), S(3), S(4), S(5), S(6), S(7), S(8)}, 9);
#undef S
}

void *union_fn(void *context, void *aggregate, void *value)
{
    return set_union(aggregate, value);
}

static bool subset(void *placed, void *template);
static bool no_intersection(void *target, void *value);

void *mk_candidates(void *context, void *placed)
{
    struct list *templates = pair_1st(context);
    struct set *all = pair_2nd(context);
    struct list *subsets = list_filter(templates, subset, placed);
    return list_filter(subsets, no_intersection, set_difference(all, placed));
}

bool subset(void *placed, void *template)
{
    return set_subset(template, placed);
}

bool no_intersection(void *target, void *value)
{
    return set_subset(set_make(NULL, 0), set_intersection(target, value));
}

static void *tagged_make(void *tag, void *value);
static void *tagged_tag(void *tagged);
static void *tagged_value(void *tagged);

static void *number_make(unsigned value);
static unsigned as_number(void *value);

void *zip_with_tag(void *context, void *aggregate, void *value)
{
    unsigned *p = context;
    *p += 1;
    return list_append(aggregate, tagged_make(number_make(*p), value));
}

static struct list *flatten(struct list *lists);
static void *nexts_make(void *to_be_fixed, void *fixed);

void *solve_aux(void *context, void *value)
{
    void *to_be_fixed = pair_1st(value);
    void *fixed = pair_2nd(value);
    return list_length(to_be_fixed) == 0
               ? list_make(&fixed, 1)                                                   // solved branch
               : flatten(list_map(nexts_make(to_be_fixed, fixed), solve_aux, context)); // recursive search
}

static void *forge_number(void *context, void *value);

void *tagged_set_to_string(void *context, void *value)
{
    char *string = GC_MALLOC_ATOMIC(82);
    string[81] = '\0';
    list_map(value, forge_number, string);
    return string;
}

void *forge_number(void *context, void *value)
{
    char *const string = context;
    unsigned const n = as_number(tagged_tag(value));
    void *const decomp = set_decompose(tagged_value(value));
    unsigned *it = decomposed_array(decomp);
    unsigned *const end = it + decomposed_size(decomp);
    while (it != end)
        string[*it++] = n + '0';
    return NULL;
}

static void *next_pair(void *context, void *candidate);

struct triple
{
    void *const tag, *const fixed, *const to_be_fixed;
};

static void *choose(void *context, void *pair, void *value);

void *nexts_make(void *to_be_fixed, void *fixed)
{
    void *const choosen = list_reduce(to_be_fixed, NULL, choose, NULL); // choose minimum list
    void *const fixing = pair_1st(choosen);
    void *const delayed = pair_2nd(choosen);
    void *const tag = tagged_tag(fixing);
    void *const candidates = tagged_value(fixing);
    return list_map(candidates, next_pair, &(struct triple){.tag = tag, .fixed = fixed, .to_be_fixed = delayed});
}

/**
 * reduce function: to choose the tagged list which length is minimum.
 * 
 * at first, create a pair of (tagged-list, empty-list).
 * 2nd time and later, compare the length of pair's first list and passing value's, and return a pair (lesser, cons(list, major)).
 */
void *choose(void *context, void *pair, void *value)
{
    if (pair == NULL)
    {
        return pair_make(value, list_make(NULL, 0));
    }
    else
    {
        void *fewer = pair_1st(pair);
        void *mores = pair_2nd(pair);
        return list_length(tagged_value(value)) < list_length(tagged_value(fewer))
                   ? pair_make(value, list_append(mores, fewer))
                   : pair_make(fewer, list_append(mores, value));
    }
}

static void *strip_having_intersectoin(void *target, void *value);

void *next_pair(void *context, void *candidate)
{
    struct triple *const p = context;
    void *const fixed = list_append(p->fixed, tagged_make(p->tag, candidate));
    void *const to_be_fixed = list_map(p->to_be_fixed, strip_having_intersectoin, candidate);
    return pair_make(to_be_fixed, fixed);
}

void *strip_having_intersectoin(void *target, void *value)
{
    return tagged_make(tagged_tag(value), list_filter(tagged_value(value), no_intersection, target));
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

struct decomposed
{
    unsigned n;
    unsigned vs[0];
};

void *set_decompose(const struct set *set)
{
    const void *bits = set;
    const unsigned n = bits_count(bits);
    struct decomposed *p = GC_MALLOC_ATOMIC(sizeof(struct decomposed) + sizeof(unsigned) * n);
    p->n = n;
    unsigned i = 0;
    unsigned *it = p->vs;
    while (it != p->vs + n)
    {
        while (!bits_get(bits, i))
            i += 1;
        *it++ = i++;
    }
    return p;
}

unsigned decomposed_size(void *decomposed)
{
    return ((struct decomposed *)decomposed)->n;
}

unsigned *decomposed_array(void *decomposed)
{
    return ((struct decomposed *)decomposed)->vs;
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

struct pair
{
    void *first;
    void *second;
};

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
