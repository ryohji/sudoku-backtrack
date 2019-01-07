#include "op.h"

#include <memory.h>
#include <stdlib.h>

void bits_clear(unsigned nr_bits, unsigned *bits)
{
    memset(bits, 0, sizeof(unsigned) * nr_word_for(nr_bits));
}

void bit_set(unsigned index, bool set, unsigned *bits)
{
    div_t d = div(index, NBITS);
    if (set)
    {
        bits[d.quot] |= 1 << d.rem;
    }
    else
    {
        bits[d.quot] &= ~(1 << d.rem);
    }
}

bool bit_value(unsigned index, const unsigned *bits)
{
    div_t d = div(index, NBITS);
    return bits[d.quot] & (1 << d.rem);
}

void bits_copy(unsigned nr_bits, unsigned *dst, const unsigned *src)
{
    const unsigned *const end = src + nr_word_for(nr_bits);
    while (src != end)
    {
        *dst++ = *src++;
    }
}

void bitwise_and(unsigned nr_bits, unsigned *xs, const unsigned *ys)
{
    const unsigned *const end = xs + nr_word_for(nr_bits);
    while (xs != end)
    {
        *xs++ &= *ys++;
    }
}

void bitwise_or(unsigned nr_bits, unsigned *xs, const unsigned *ys)
{
    const unsigned *const end = xs + nr_word_for(nr_bits);
    while (xs != end)
    {
        *xs++ |= *ys++;
    }
}

static unsigned count(unsigned bits);

unsigned bits_count(unsigned nr_bits, const unsigned *bits)
{
    unsigned n = 0;
    const unsigned *const end = bits + nr_word_for(nr_bits);
    while (bits != end)
    {
        n += count(*bits++);
    }
    return n;
}

unsigned count(unsigned bits)
{
    bits = (0x55555555 & bits) + (0x55555555 & (bits >> 1));
    bits = (0x33333333 & bits) + (0x33333333 & (bits >> 2));
    bits = (0x0f0f0f0f & bits) + (0x0f0f0f0f & (bits >> 4));
    bits = (0x00ff00ff & bits) + (0x00ff00ff & (bits >> 8));
    return (0x0000ffff & bits) + (0x0000ffff & (bits >> 16));
}
