#include <stdio.h>

void pairs(unsigned *begin, unsigned *end)
{
    if (begin + 1 != end)
    {
        unsigned *it;
        for (it = begin + 1; it != end; it += 1)
        {
            printf("(%d,%d) ", *begin, *it);
        }
        pairs(begin + 1, end);
    }
}

int main()
{
    unsigned xs[] = {1, 2, 3, 4, 5};
    pairs(xs, xs + 5);
    return 0;
}