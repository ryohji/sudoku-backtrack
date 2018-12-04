#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define N 81

void init(const char *board, uint16_t *cells)
{
    const char *it = board;
    while (it != board + N)
    {
        uint16_t c;
        switch (*it)
        {
        default:
            c = 511;
            break;
        case '1':
            c = 1;
            break;
        case '2':
            c = 2;
            break;
        case '3':
            c = 4;
            break;
        case '4':
            c = 8;
            break;
        case '5':
            c = 16;
            break;
        case '6':
            c = 32;
            break;
        case '7':
            c = 64;
            break;
        case '8':
            c = 128;
            break;
        case '9':
            c = 256;
            break;
        }
        cells[it - board] = c;
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
    fflush(stdout);
}

uint16_t *candidates(const uint16_t cell, uint16_t *buffer)
{
    const uint16_t bit[] = {
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
    const uint16_t *it = bit;
    while (it != bit + 9)
    {
        if (*it & cell)
        {
            *buffer = *it;
            buffer += 1;
        }
        it += 1;
    }
    return buffer;
}

/**
 * search next cell, which holds some candidates.
 */
struct next
{
    uint16_t *p;
    unsigned n; // number of valid candidates
    uint16_t candidates[9];
};

struct next next(uint16_t *cells)
{
    struct next ret = {.p = cells};
    while (ret.p != cells + N)
    {
        uint16_t *p = candidates(*ret.p, ret.candidates);
        if ((ret.n = p - ret.candidates) > 1)
        {
            break;
        }
        ret.p += 1;
    }
    return ret;
}

unsigned degree_of_freedom(uint16_t *cells)
{
    unsigned d = 1;
    uint16_t *it = cells;
    while (it != cells + N)
    {
        uint16_t buffer[9];
        uint16_t *p = candidates(*it, buffer);
        d *= p - buffer;
        it += 1;
    }
    return d;
}

bool ok(uint16_t *cells);

void solve(uint16_t *cells)
{
    if (ok(cells))
    {
        struct next n = next(cells);
        if (n.p != cells + N)
        {
            uint16_t cell = *n.p; // memory
            uint16_t *it = n.candidates;
            while (it != n.candidates + n.n)
            {
                *n.p = *it; // subvert cells by candidate
                solve(cells);
                it += 1;
            }
            *n.p = cell; // revert to backtrack
        }
        else
        {
            dump(cells);
        }
    }
}

bool acceptable(const char *board, const unsigned *index_iterator, const unsigned *offset_iterator);

bool ok(uint16_t *cells)
{
    char board[N];
    deinit(cells, board);

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

    return acceptable(board, r_idx, r_off) && acceptable(board, c_idx, c_off) && acceptable(board, b_idx, b_off);
}

#define min(a, b) ((a) > (b) ? (b) : (a))

bool acceptable(const char *board, const unsigned *index_iterator, const unsigned *offset_iterator)
{
    const unsigned *it2 = offset_iterator;

    while (it2 != offset_iterator + 9)
    {
        const char *p = board + *it2;

        unsigned bits[10] = {0};
        const unsigned *it1 = index_iterator;
        while (it1 != index_iterator + 9)
        {
            unsigned i = p[*it1] - '1';
            bits[min(i, 9)] += 1;
            it1 += 1;
        }

        if (bits[0] > 1 || bits[1] > 1 || bits[2] > 1 || bits[3] > 1 || bits[4] > 1 || bits[5] > 1 || bits[6] > 1 || bits[7] > 1 || bits[8] > 1)
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
