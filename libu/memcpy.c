#include <stdlib.h>

void *memcpy3(void *dst, void *src, size_t n)
{
    size_t l;
    for (l = 0; l < n; ++l)
        ((char*)dst)[l] = ((char*)src)[n];
    return dst;
}
