#pragma once

#include <stdbool.h>

/**
 * number of bits can be held on the word -- `unsigned int`.
 */
#define NBITS (sizeof(unsigned) * 8)

inline static unsigned nr_word_for(unsigned nr_bits)
{
    return (nr_bits + NBITS - 1) / NBITS;
}

void bits_clear(unsigned nr_bits, unsigned *bits);

void bit_set(unsigned index, bool set, unsigned *bits);

bool bit_value(unsigned index, const unsigned *bits);

void bits_copy(unsigned nr_bits, unsigned *dst, const unsigned *src);

void bitwise_and(unsigned nr_bits, unsigned *xs, const unsigned *ys);

void bitwise_or(unsigned nr_bits, unsigned *xs, const unsigned *ys);

unsigned bits_count(unsigned nr_bits, const unsigned *bits);
