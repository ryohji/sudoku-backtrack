#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define N 81
#define min(a, b) ((a) > (b) ? (b) : (a))
#define BITS3(x) ((unsigned[]){ \
    0,                          \
    1,                          \
    1,                          \
    2,                          \
    1,                          \
    2,                          \
    2,                          \
    3,                          \
})[(x)&07]

static const uint16_t bits[9] = {
    1,
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
};

unsigned number_of_bits(const uint16_t candidates)
{
    return BITS3(candidates) + BITS3(candidates >> 3) + BITS3(candidates >> 6);
}

void init(const char *board, uint16_t *cells)
{
    const char *it = board;
    while (it != board + N)
    {
        const unsigned index = *it - '1';
        cells[it - board] = index < 9 ? bits[index] : 511; // 511 is the sum of bits (i.e. all candidates).
        it += 1;
    }
}

void deinit(const uint16_t *cells, char *buffer)
{
    const uint16_t *it = cells;
    while (it != cells + N)
    {
        char c;
        switch (*it)
        {
        default:
            c = '*';
            break;
        case 1:
            c = '1';
            break;
        case 2:
            c = '2';
            break;
        case 4:
            c = '3';
            break;
        case 8:
            c = '4';
            break;
        case 16:
            c = '5';
            break;
        case 32:
            c = '6';
            break;
        case 64:
            c = '7';
            break;
        case 128:
            c = '8';
            break;
        case 256:
            c = '9';
            break;
        }
        buffer[it - cells] = c;
        it += 1;
    }
}

void dump(const uint16_t *cells)
{
    char buffer[N];
    unsigned n;
    deinit(cells, buffer);
    for (n = 0; n < 9; n += 1)
    {
        char line[10] = {0};
        strncpy(line, buffer + 9 * n, 9);
        puts(line);
    }
}

/**
 * search next cell, which holds some candidates.
 */
uint16_t *next(uint16_t *cells)
{
    uint16_t *it = cells;
    while (it != cells + N && number_of_bits(*it) == 1)
    {
        it += 1;
    }
    return it;
}

unsigned degree_of_freedom(uint16_t *cells)
{
    unsigned d = 1;
    uint16_t *it = cells;
    while (it != cells + N)
    {
        d *= number_of_bits(*it);
        it += 1;
    }
    return d;
}

bool ok(uint16_t *cells);

void solve(uint16_t *cells)
{
    if (ok(cells))
    {
        uint16_t *const p = next(cells);
        if (p != cells + N)
        {
            const uint16_t original = *p; // memory candidates
            const uint16_t *it = bits;
            while (it != bits + 9)
            {
                if (original & *it)
                {
                    *p = *it; // subvert cell by candidate
                    solve(cells);
                }
                it += 1;
            }
            *p = original; // revert to backtrack
        }
        else
        {
            dump(cells);
        }
    }
}

bool acceptable(const uint16_t *cells, const unsigned *index_iterator, const unsigned *offset_iterator);

bool ok(uint16_t *cells)
{
    const unsigned r_idx[] = {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
    };
    const unsigned r_off[] = {
        0,
        9,
        18,
        27,
        36,
        45,
        54,
        63,
        72,
    };
    const unsigned *c_idx = r_off;
    const unsigned *c_off = r_idx;
    const unsigned b_idx[] = {
        0,
        1,
        2,
        9,
        10,
        11,
        18,
        19,
        20,
    };
    const unsigned b_off[] = {
        0,
        3,
        6,
        27,
        30,
        33,
        54,
        57,
        60,
    };

    return acceptable(cells, r_idx, r_off) && acceptable(cells, c_idx, c_off) && acceptable(cells, b_idx, b_off);
}

bool acceptable(const uint16_t *cells, const unsigned *index_iterator, const unsigned *offset_iterator)
{
    const unsigned *it2 = offset_iterator;

    while (it2 != offset_iterator + 9)
    {
        uint16_t count[9] = { 0 };
        const unsigned *it1 = index_iterator;
        while (it1 != index_iterator + 9)
        {
            const uint16_t candidate = *(cells + *it2 + *it1);
            const uint16_t* it = bits;
            while (it != bits + 9)
            {
                if (candidate == *it)
                {
                    count[it - bits] += 1;
                }
                it += 1;
            }
            it1 += 1;
        }

        if (count[0] > 1 || count[1] > 1 || count[2] > 1 || count[3] > 1 || count[4] > 1 || count[5] > 1 || count[6] > 1 || count[7] > 1 || count[8] > 1)
        {
            break;
        }

        it2 += 1;
    }

    return it2 == offset_iterator + 9;
}

int main(int argn, const char **args)
{
    uint16_t cells[N];

    init("*6*41*83*"
         "7**8*****"
         "5*19*****"
         "*******7*"
         "6*9***5*4"
         "*1*******"
         "*****47*9"
         "*****8**1"
         "*78*39*6*",
         cells);
    solve(cells);

    return 0;
}
