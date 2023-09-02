#include <stdlib.h>
#include <mncommon/fasthash.h>
#include <mncommon/dumpm.h>
#include <mncommon/util.h>

/**
 * http://www.cse.yorku.ca/~oz/hash.html
 *
 */
UNUSED static inline uint64_t
rotl64(uint64_t n, int b)
{
#ifndef NDEBUG
    if (b == 0) {
        return n;
    }
#endif
    return (n << b) | (n >> (64 - b));
}

UNUSED static inline uint64_t
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
    size_t rem = sz % sizeof(uint64_t);
    size_t i, sz1;
    uint64_t *si;

    si = (uint64_t *)s;
    sz1 = sz - rem;
    n ^= sz;

    /* modified DJB2 */

#define DJB2(c) n = ((n << 5) + n) + (c);
#define DJB2XOR(c) n = ((n << 5) + n) ^ (c);

#define LOOP(A)                        \
    for (i = 0; i < (sz1 / 8); ++i) {  \
        A(si[i]);                      \
    }                                  \
    if (rem) {                         \
        union {                        \
            uint64_t u64;              \
            char  c[sizeof(uint64_t)]; \
        } ss;                          \
        ss.u64 = 0UL;                  \
        for (i = 0; i < rem; ++i) {    \
            ss.c[i] = s[sz - 1 - i];   \
        }                              \
        A(ss.u64);                     \
    }


#define C(f)                                   \
    for (i = 0; i < (sz1 / 8); ++i) {          \
        n += *((si + i));                      \
        n = f(n, (si[i] + i) %                 \
                 (sizeof(uint64_t) * 8));      \
    }                                          \
    if (rem) {                                 \
        union {                                \
            uint64_t u64;                      \
            char  c[sizeof(uint64_t)];         \
        } ss;                                  \
        ss.u64 = 0UL;                          \
        for (i = 0; i < rem; ++i) {            \
            ss.c[i] = s[sz - 1 - i];           \
        }                                      \
        n += ss.u64;                           \
        n = f(n, (ss.c[0] + i) %               \
                 (sizeof(uint64_t) * 8));      \
    }

    //C(rotr64);
    LOOP(DJB2XOR);

    //if (sz < sizeof(uint64_t) * 2) {
    //    C(rotl64);
    //} else {
    //    C(rotr64);
    //}
    return n;
}

