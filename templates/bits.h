#pragma once
#include <stdbool.h>

struct bits;

// construct bits holder
struct bits *bits_make(unsigned nr_bits);

unsigned bits_size(const struct bits *bits);

// another constructor
struct bits *bits_restore(const char *bitstring);

// serialize bits into string
char *bits_serialize(const struct bits *bits);

// make specified bit changed object
struct bits *bits_set(const struct bits *bits, unsigned index, bool set);

// true if specified index of bit is set, otherwise false
bool bits_get(const struct bits *bits, unsigned index);

// count bits set
unsigned bits_count(const struct bits *bits);

// make bitwise inverted object
struct bits *bits_not(const struct bits *bits);

// note: apis below should be called fullfilling bits_size(a) == bits_size(b), or result is unspecified.

// make bitwise logical or'ed object
struct bits *bits_or(const struct bits *a, const struct bits *b);

// make bitwise logical and'ed object
struct bits *bits_and(const struct bits *a, const struct bits *b);

// make bitwise logical xor'ed object
struct bits *bits_xor(const struct bits *a, const struct bits *b);

// make bitwise logical eq'ed object
// note: bits_size(a) == bits_size(b) == bits_count(bits_eq(a, b)) if all bits in `a` and `b` is matched.
struct bits *bits_eq(const struct bits *a, const struct bits *b);
