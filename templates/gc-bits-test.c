#include "bits.h"
#include "gc.h"

#include <stdio.h>

int main()
{
    struct bits *bits = bits_restore("01010101");
    printf("%s\n", bits_serialize(bits));
    printf("%s (%d)\n", bits_serialize(bits_or(bits, bits_not(bits))), bits_count(bits_or(bits, bits_not(bits))));
    printf("%s xor %s = %s\n", bits_serialize(bits), bits_serialize(bits), bits_serialize(bits_xor(bits, bits)));
    printf("%s eq  %s = %s\n", bits_serialize(bits), bits_serialize(bits), bits_serialize(bits_eq(bits, bits)));
    printf("%s and %s = %s (%d)\n", bits_serialize(bits), "00001111", bits_serialize(bits_and(bits, bits_restore("00001111"))), bits_count(bits_and(bits, bits_restore("00001111"))));
    printf("%s or  %s = %s (%d)\n", bits_serialize(bits), "00001111", bits_serialize(bits_or(bits, bits_restore("00001111"))), bits_count(bits_or(bits, bits_restore("00001111"))));

    bits = bits_set(bits_make(33), 32, true);
    printf("%s / %s\n", bits_serialize(bits), bits_serialize(bits_not(bits)));
    fflush(stdout);
    return 0;
}