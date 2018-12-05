#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define N 81
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

static const unsigned *constraint[27] = {
    // constraint check index for rows
    (unsigned[9]){0, 1, 2, 3, 4, 5, 6, 7, 8},
    (unsigned[9]){9, 10, 11, 12, 13, 14, 15, 16, 17},
    (unsigned[9]){18, 19, 20, 21, 22, 23, 24, 25, 26},
    (unsigned[9]){27, 28, 29, 30, 31, 32, 33, 34, 35},
    (unsigned[9]){36, 37, 38, 39, 40, 41, 42, 43, 44},
    (unsigned[9]){45, 46, 47, 48, 49, 50, 51, 52, 53},
    (unsigned[9]){54, 55, 56, 57, 58, 59, 60, 61, 62},
    (unsigned[9]){63, 64, 65, 66, 67, 68, 69, 70, 71},
    (unsigned[9]){72, 73, 74, 75, 76, 77, 78, 79, 80},
    // constraint check index for columns
    (unsigned[9]){0, 9, 18, 27, 36, 45, 54, 63, 72},
    (unsigned[9]){1, 10, 19, 28, 37, 46, 55, 64, 73},
    (unsigned[9]){2, 11, 20, 29, 38, 47, 56, 65, 74},
    (unsigned[9]){3, 12, 21, 30, 39, 48, 57, 66, 75},
    (unsigned[9]){4, 13, 22, 31, 40, 49, 58, 67, 76},
    (unsigned[9]){5, 14, 23, 32, 41, 50, 59, 68, 77},
    (unsigned[9]){6, 15, 24, 33, 42, 51, 60, 69, 78},
    (unsigned[9]){7, 16, 25, 34, 43, 52, 61, 70, 79},
    (unsigned[9]){8, 17, 26, 35, 44, 53, 62, 71, 80},
    // constraint check index for blocks
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

bool acceptable(const uint16_t *cells, const unsigned *index);

bool ok(uint16_t *cells)
{
    const unsigned **it = constraint;
    while (it != constraint + 27 && acceptable(cells, *it))
    {
        it += 1;
    }
    return it == constraint + 27;
}

bool acceptable(const uint16_t *cells, const unsigned *index)
{
    uint16_t bitmap = 0;
    const unsigned *it1 = index;
    while (it1 != index + 9)
    {
        const uint16_t cell = *(cells + *it1);
        // check if this cell includes only one candidate
        const uint16_t *it = bits;
        while (it != bits + 9 && cell != *it)
        {
            it += 1;
        }

        if (it != bits + 9)
        {
            if (bitmap & cell)
            { // there is duplicate candidate
                break;
            }
            else
            { // memory first candidate
                bitmap |= cell;
            }
        }

        it1 += 1;
    }

    return it1 == index + 9;
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
