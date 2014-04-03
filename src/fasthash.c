#include <stdlib.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/dumpm.h>

static inline uint64_t
rotl64(uint64_t n, int b)
{
#ifndef NDEBUG
    if (b == 0) {
        return n;
    }
#endif
    return (n << b) | (n >> (64 - b));
}

static inline uint64_t
rotr64(uint64_t n, int b)
{
#ifndef NDEBUG
    if (b == 0) {
        return n;
    }
#endif
    return (n >> b) | (n << (64 - b));
}

uint64_t
fasthash(uint64_t n, const unsigned char *s, size_t sz)
{
    size_t mod = sz % sizeof(uint64_t);
    size_t i, j, sz1;

    sz1 = sz - mod;

#define C(f) \
    for (i = 0, j = 0;                      \
         j < sz1;                           \
         ++i, j += sizeof(uint64_t)) {      \
        n += *((uint64_t *)(s + j));        \
        n = f(n, (s[j] + i) % 8);           \
    }                                       \
    if (mod) {                              \
        union {                             \
            uint64_t u64;                   \
            char  c[sizeof(uint64_t)];      \
        } ss;                               \
        ss.u64 = 0UL;                       \
        for (i = 0; i < mod; ++i) {         \
            ss.c[i] = s[sz - 1 - i];        \
        }                                   \
        n += ss.u64;                        \
        n = f(n, (ss.c[0] + i) % 8);        \
    }

    if (sz < sizeof(uint64_t) * 2) {
        C(rotl64);
    } else {
        C(rotr64);
    }
    return n;
}

