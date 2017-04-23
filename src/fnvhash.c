#include <mrkcommon/fnvhash.h>

#define FNV_PRIME64 (1099511628211ul)

uint64_t
fnvhash64(uint64_t n, const unsigned char *s, size_t sz)
{
    unsigned i;

    /*
     * fnv-1a
     */
    for (i = 0; i < sz; ++i) {
        n ^= (uint64_t)s[i];
        n *= FNV_PRIME64;
    }
    return n;
}

