#include "gc.h"
#include "list.h"
#include <stdbool.h>
#include <stdio.h>

void *Integer(int n)
{
    int *p = GC_MALLOC_ATOMIC(sizeof(int));
    *p = n;
    return p;
}

void *printInteger(void *context, void *value)
{
    printf("%d ", *(int *)value);
    return (void *)value;
}

bool even(void *context, void *value)
{
    return (*(int *)value & 1) == 0;
}

int main()
{
    GC_INIT();
    void *ns[5] = {Integer(0), Integer(1), Integer(2), Integer(3), Integer(4)};
    struct list *list = list_make(ns, 5);
    list = list_filter(list, even, NULL);
    list_map(list, printInteger, NULL);
    puts("");
    fflush(stdout);
    return 0;
}
