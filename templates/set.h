#pragma once
#include <stdbool.h>

struct set;

struct set *set_make(const unsigned *ns, unsigned elems);

struct set *set_union(const struct set *a, const struct set *b);

struct set *set_difference(const struct set *a, const struct set *b);

struct set *set_intersection(const struct set *a, const struct set *b);

// test if set B is the subset of set A.
bool set_subset(const struct set *a, const struct set *b);

// return the array object which having numbers in the specified set.
struct set_decomposed;
struct set_decomposed *set_decompose(const struct set *set);

unsigned decomposed_size(struct set_decomposed *decomposed);

unsigned *decomposed_array(struct set_decomposed *decomposed);