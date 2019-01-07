#include "op.h"
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

static const unsigned *costraint_block[9] = {
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

void bitwise_print(unsigned nr_bits, const unsigned *bits)
{
    unsigned it;
    for (it = 0; it < nr_bits; it += 1)
    {
        printf("%d", bit_value(it, bits));
    }
}

void generate(unsigned *bits, const unsigned **begin, const unsigned **end, void (*output)(void *context, const unsigned *bits), void *context)
{
    if (begin != end)
    {
        const unsigned *it;
        for (it = *begin; it != *begin + 9; it += 1)
        {
            bool bit = bit_value(*it, bits);
            bit_set(*it, 1, bits);
            generate(bits, begin + 1, end, output, context);
            bit_set(*it, bit, bits);
        }
    }
    else
    {
        output(context, bits);
    }
}

int compar(const void *a, const void *b)
{
    unsigned n;
    for (n = nr_word_for(81); n != 0; n -= 1)
    {
        if (((unsigned *)a)[n - 1] == ((unsigned *)b)[n - 1])
        {
            continue;
        }
        else
        {
            return ((unsigned *)a)[n - 1] > ((unsigned *)b)[n - 1] ? 1 : -1;
        }
    }
    return 0;
}

void output(void *context, const unsigned *bits)
{
    unsigned **out = context;
    bits_copy(81, *out, bits);
    *out += 3;
}

struct filter_context
{
    const unsigned *const bits;
    const unsigned elem;
    unsigned *const out;
    unsigned n;
};

void filter_copy(void *context, const unsigned *bits)
{
    struct filter_context *fc = context;
    const unsigned width = nr_word_for(81);
    if (bsearch(bits, fc->bits, fc->elem, width * sizeof(unsigned), compar))
    {
        bits_copy(81, fc->out + fc->n * width, bits);
        fc->n += 1;
    }
}

void dump(const unsigned *bits)
{
    unsigned n;
    for (n = nr_word_for(81); n != 0; n -= 1)
    {
        printf("%08x", bits[n - 1]);
    }
}

int main()
{
    const unsigned n = nr_word_for(81);

    unsigned *const row = malloc(sizeof(unsigned) * n * 387420489);
    unsigned bits[n];
    unsigned *p;

    puts("generating row...");
    bits_clear(81, bits);
    generate(bits, constraint_row, constraint_row + 9, output, (p = row, &p));

    puts("sorting row...");
    qsort(row, 387420489, n * sizeof(unsigned), compar);

    unsigned *col = malloc(sizeof(unsigned) * n * 387420489);
    struct filter_context column_context = {
        .bits = row,
        .elem = 387420489,
        .out = col,
        .n = 0,
    };
    puts("generating column...");
    bits_clear(81, bits);
    generate(bits, constraint_column, constraint_column + 9, filter_copy, &column_context);

    free(row);
    col = realloc(col, sizeof(unsigned) * n * column_context.n);

    puts("sorting column...");
    qsort(col, column_context.n, n * sizeof(unsigned), compar);

    unsigned *blk = malloc(sizeof(unsigned) * n * column_context.n);
    struct filter_context block_context = {
        .bits = col,
        .elem = column_context.n,
        .out = blk,
        .n = 0,
    };
    puts("generating block...");
    bits_clear(81, bits);
    generate(bits, costraint_block, costraint_block + 9, filter_copy, &block_context);

    free(col);
    blk = realloc(blk, sizeof(unsigned) * n * block_context.n);

    puts("sorting block...");
    qsort(blk, block_context.n, n * sizeof(unsigned), compar);

    {
        unsigned i;
        for (i = 0; i < 20; i += 1)
        {
            dump(blk + n * i);
            puts("");
        }
    }

    printf("%d -> %d -> %d\n", 387420489, column_context.n, block_context.n);

    return 0;
}
