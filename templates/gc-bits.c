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

struct bits *bits_make(unsigned nr_bits, const unsigned *index, unsigned elems)
{
    unsigned width = sizeof(struct bits) + WSIZE * nr_word_for(nr_bits);
    struct bits *bits = GC_MALLOC_ATOMIC(width);
    memset(bits, 0, width);
    bits->n = nr_bits;
    const unsigned *it;
    for (it = index; it != index + elems; it += 1)
        bits->data[*it / NBITS] |= 1 << (*it % NBITS);
    return bits;
}

unsigned bits_size(const struct bits *bits)
{
    return bits->n;
}

struct bits *bits_restore(const char *bitstring)
{
    const unsigned length = strlen(bitstring);
    unsigned index[length];
    unsigned *end = index;
    const char *p;
    for (p = bitstring; p != bitstring + length; p += 1)
        if (*p != '0')
            *end++ = p - bitstring;
    return bits_make(length, index, end - index);
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
    struct bits *copy = bits_make(bits->n, NULL, 0);
    memcpy(copy->data, bits->data, WSIZE * nr_word_for(bits->n));
    if (set)
    {
        copy->data[index / NBITS] |= 1 << (index % NBITS);
    }
    else
    {
        copy->data[index / NBITS] &= ~(1 << (index % NBITS));
    }
    return copy;
}

bool bits_get(const struct bits *bits, unsigned index)
{
    return bits->data[index / NBITS] & (1 << (index % NBITS));
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

struct bits *bits_eq(const struct bits *a, const struct bits *b)
{
    return bits_not(bits_xor(a, b));
}

#define FOR_EACH_WORD_OF(bits, value)                                           \
    do                                                                          \
    {                                                                           \
        unsigned *it;                                                           \
        for (it = bits->data; it != bits->data + nr_word_for(bits->n); it += 1) \
        {                                                                       \
            const unsigned distance = it - bits->data;                          \
            *it = value;                                                        \
        }                                                                       \
        bits->data[bits->n / NBITS] &= (1 << (bits->n % NBITS)) - 1;            \
    } while (0)

struct bits *bits_not(const struct bits *original)
{
    struct bits *bits = bits_make(original->n, NULL, 0);
    FOR_EACH_WORD_OF(bits, (~original->data[distance]));
    return bits;
}

struct bits *bits_or(const struct bits *a, const struct bits *b)
{
    struct bits *bits = bits_make(a->n, NULL, 0);
    FOR_EACH_WORD_OF(bits, (a->data[distance] | b->data[distance]));
    return bits;
}

struct bits *bits_and(const struct bits *a, const struct bits *b)
{
    struct bits *bits = bits_make(a->n, NULL, 0);
    FOR_EACH_WORD_OF(bits, (a->data[distance] & b->data[distance]));
    return bits;
}

struct bits *bits_xor(const struct bits *a, const struct bits *b)
{
    struct bits *bits = bits_make(a->n, NULL, 0);
    FOR_EACH_WORD_OF(bits, (a->data[distance] ^ b->data[distance]));
    return bits;
}
