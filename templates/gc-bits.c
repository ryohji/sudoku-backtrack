#include "bits.h"
#include "gc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define WSIZE sizeof(unsigned)
#define NBITS (WSIZE * 8)

inline static unsigned nr_word_for(unsigned nr_bits)
{
    return (nr_bits + NBITS - 1) / NBITS;
}

struct bits
{
    unsigned n;
    unsigned data[0];
};

struct bits *bits_make(unsigned nr_bits)
{
    struct bits *bits = GC_MALLOC(sizeof(struct bits) + WSIZE * nr_word_for(nr_bits));
    bits->n = nr_bits;
    return bits;
}

unsigned bits_size(const struct bits *bits)
{
    return bits->n;
}

static void bits_set_(unsigned *bits, unsigned index, bool set);

struct bits *bits_restore(const char *bitstring)
{
    struct bits *bits = bits_make(strlen(bitstring));
    const char *it;
    for (it = bitstring; it != bitstring + bits_size(bits); it += 1)
    {
        bits_set_(bits->data, it - bitstring, *it != '0');
    }
    return bits;
}

void bits_set_(unsigned *bits, unsigned index, bool set)
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

char *bits_serialize(const struct bits *bits)
{
    char *p = GC_MALLOC_ATOMIC(bits->n + 1);
    unsigned n;
    for (n = 0; n != bits->n; n += 1)
    {
        *(p + bits->n - 1 - n) = bits_get(bits, n) ? '1' : '0';
    }
    p[n] = '\0';
    return p;
}

struct bits *bits_set(const struct bits *bits, unsigned index, bool set)
{
    struct bits *copy = bits_make(bits->n);
    memcpy(copy->data, bits->data, WSIZE * nr_word_for(bits->n));
    bits_set_(copy->data, index, set);
    return copy;
}

bool bits_get(const struct bits *bits, unsigned index)
{
    div_t d = div(index, NBITS);
    return bits->data[d.quot] & (1 << d.rem);
}

static unsigned count(unsigned bits);

unsigned bits_count(const struct bits *bits)
{
    unsigned n = 0;
    const unsigned *const end = bits->data + nr_word_for(bits->n);
    const unsigned *it = bits->data;
    while (it != end)
    {
        n += count(*it++);
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

static void bits_mask_(unsigned *bits, unsigned nr_bits)
{
    div_t d = div(nr_bits, NBITS);
    unsigned mask = (1 << d.rem) - 1;
    bits[d.quot] &= mask;
}

struct bits *bits_not(const struct bits *original)
{
    struct bits *bits = bits_make(original->n);
    unsigned *it;
    for (it = bits->data; it != bits->data + nr_word_for(bits->n); it += 1)
    {
        unsigned distance = it - bits->data;
        *it = ~original->data[distance];
    }
    bits_mask_(bits->data, bits->n);
    return bits;
}

struct bits *bits_or(const struct bits *a, const struct bits *b)
{
    struct bits *bits = bits_make(a->n);
    unsigned *it;
    for (it = bits->data; it != bits->data + nr_word_for(bits->n); it += 1)
    {
        unsigned distance = it - bits->data;
        *it = a->data[distance] | b->data[distance];
    }
    bits_mask_(bits->data, bits->n);
    return bits;
}

struct bits *bits_and(const struct bits *a, const struct bits *b)
{
    struct bits *bits = bits_make(a->n);
    unsigned *it;
    for (it = bits->data; it != bits->data + nr_word_for(bits->n); it += 1)
    {
        unsigned distance = it - bits->data;
        *it = a->data[distance] & b->data[distance];
    }
    bits_mask_(bits->data, bits->n);
    return bits;
}

struct bits *bits_xor(const struct bits *a, const struct bits *b)
{
    struct bits *bits = bits_make(a->n);
    unsigned *it;
    for (it = bits->data; it != bits->data + nr_word_for(bits->n); it += 1)
    {
        unsigned distance = it - bits->data;
        *it = a->data[distance] ^ b->data[distance];
    }
    bits_mask_(bits->data, bits->n);
    return bits;
}

struct bits *bits_eq(const struct bits *a, const struct bits *b)
{
    return bits_not(bits_xor(a, b));
}
