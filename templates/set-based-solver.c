#include "list.h"
#include "set.h"
#include "pair.h"
#include "tagged.h"
#include "gc.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *fread_sudoku(FILE *fp);

static void *fput_string(void *context, void *aggregate, void *value);
static struct list *solve(const char *sudoku);

int main()
{
    GC_INIT();
    list_reduce(solve(fread_sudoku(stdin)), stdout, fput_string, NULL);
    return 0;
}

const char *fread_sudoku(FILE *fp)
{
    char *const buffer = GC_MALLOC_ATOMIC(82);
    memset(buffer, '\0', 82);

    char *p = buffer, *q;
    do
        (p = fgets(p, (82 - (p - buffer)), fp)) && (q = strchr(p, '\n')) && (*q = '\0'), p && (p += strlen(p));
    while (strlen(buffer) != 81 && !feof(fp) && !ferror(fp));

    if (strlen(buffer) == 0)
    {
        strcpy(buffer, "*6*41*83*"
                       "7**8*****"
                       "5*19*****"
                       "*******7*"
                       "6*9***5*4"
                       "*1*******"
                       "*****47*9"
                       "*****8**1"
                       "*78*39*6*");
        fprintf(stderr, "Solving sample; no sudoku read from stdin.\n");
    }
    puts(buffer);
    return buffer;
}

void *fput_string(void *context, void *file, void *string)
{
    fprintf(file, "%s\n", (char *)string);
    return file;
}

static struct list *convert_to_set(const char *sudoku);
static void *union_fn(void *context, void *aggregate, void *value);
static void *mk_candidates(void *context, void *placed);
static struct list *templates_make(void);
static void *zip_with_tag(void *context, void *aggregate, void *value);
static void *solve_aux(void *context, void *value);
static void *tagged_set_to_string(void *context, void *value);

struct list *solve(const char *sudoku)
{
    struct list *index = convert_to_set(sudoku);
    struct set *all = list_reduce(index, set_make(NULL, 0), union_fn, NULL);
    struct list *candidates = list_map(index, mk_candidates, pair_make(templates_make(), all));
    struct list *tagged_candidates = list_reduce(candidates, list_make(NULL, 0), zip_with_tag, (unsigned[1]){'0'});
    struct list *results = solve_aux(NULL, pair_make(tagged_candidates, list_make(NULL, 0)));
    return list_map(results, tagged_set_to_string, NULL);
}

struct list *convert_to_set(const char *sudoku)
{
    unsigned is[9 * 81]; // devide into 9 partition, to hold index for each number placed.
#define B(n) (is + 81 * n)
    unsigned *end[9] = {B(0), B(1), B(2), B(3), B(4), B(5), B(6), B(7), B(8)};
    unsigned n;
    const char *p;
    for (p = sudoku; p != sudoku + 81; p += 1)
        (n = *p - '1') < 9 && (*end[n]++ = p - sudoku);
#define S(n) set_make(B(n), end[n] - B(n))
    return list_make((void *[9]){S(0), S(1), S(2), S(3), S(4), S(5), S(6), S(7), S(8)}, 9);
#undef S
#undef B
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

struct list *templates_make(void)
{
    static const unsigned sudoku_template[][9] = {
#include "sudoku.template"
    };
    struct list *templates = list_make(NULL, 0);
    void *buffer[8192];
    void **p = buffer;
    const unsigned *end = sudoku_template[0] + 9 * sizeof(sudoku_template) / sizeof(sudoku_template[0]);
    const unsigned *it;
    for (it = sudoku_template[0]; it != end; it += 9)
    {
        if (p == buffer + sizeof(buffer) / sizeof(buffer[0]))
        {
            templates = list_concatenate(templates, list_make(buffer, p - buffer));
            p = buffer;
        }
        *p++ = set_make(it, 9);
    }
    return list_concatenate(templates, list_make(buffer, p - buffer));
}

static void *char_make(unsigned c);
static char as_char(void *value);

void *zip_with_tag(void *context, void *aggregate, void *value)
{
    unsigned *p = context;
    *p += 1;
    return list_append(aggregate, tagged_make(char_make(*p), value));
}

void *char_make(unsigned c)
{
    return (void *)(uintptr_t)c;
}

char as_char(void *value)
{
    return (uintptr_t)value;
}

static struct list *flatten(struct list *lists);
static void *nexts_make(void *to_be_fixed, void *fixed);

void *solve_aux(void *context, void *value)
{
    void *to_be_fixed = pair_1st(value);
    void *fixed = pair_2nd(value);
    return list_length(to_be_fixed) == 0
               ? list_make(&fixed, 1)                                                   // solved branch, or
               : flatten(list_map(nexts_make(to_be_fixed, fixed), solve_aux, context)); // search recursively.
}

static void *forge_number(void *context, void *aggregate, void *value);

void *tagged_set_to_string(void *context, void *value)
{
    char *string = GC_MALLOC_ATOMIC(82);
    string[81] = '\0';
    return list_reduce(value, string, forge_number, NULL);
}

void *forge_number(void *context, void *string, void *value)
{
    char const c = as_char(tagged_tag(value));
    struct set_decomposed *const decomp = set_decompose(tagged_value(value));
    unsigned *it = decomposed_array(decomp);
    unsigned *const end = it + decomposed_size(decomp);
    while (it != end)
        ((char *)string)[*it++] = c;
    return string;
}

static void *choose(void *context, void *pair, void *value);
static void *next_pair(void *context, void *candidate);

struct triple
{
    void *const tag, *const fixed, *const to_be_fixed;
};

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
    void *const to_be_fixed = list_map(p->to_be_fixed, strip_having_intersectoin, candidate);
    void *const fixed = list_append(p->fixed, tagged_make(p->tag, candidate));
    return pair_make(to_be_fixed, fixed);
}

void *strip_having_intersectoin(void *target, void *value)
{
    return tagged_make(tagged_tag(value), list_filter(tagged_value(value), no_intersection, target));
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
