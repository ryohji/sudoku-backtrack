#include "tagged.h"
#include "pair.h"

struct tagged *tagged_make(void *tag, void *value)
{
    return (void *)pair_make(tag, value);
}

void *tagged_tag(struct tagged *tagged)
{
    return pair_1st((void *)tagged);
}

void *tagged_value(struct tagged *tagged)
{
    return pair_2nd((void *)tagged);
}
