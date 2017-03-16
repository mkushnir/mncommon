#ifndef MRKCOMMON_UTIL_H
#define MRKCOMMON_UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *mrkcommon_diag_str(int);

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#define GCC_VERSION 0
#endif

#ifndef MAX
#   define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#   define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define UNUSED __attribute__ ((unused))

#define PRINTFLIKE(i, l) __attribute__ ((format (printf, i, l)))

#define countof(a) (sizeof(a)/sizeof(a[0]))

#ifndef NDEBUG
#   define PASTEURIZE_ADDR(a) do {if (((uintptr_t)a) == 0x5a5a5a5a5a5a5a5a || ((uintptr_t)a) == 0xa5a5a5a5a5a5a5a5) abort();} while (0)
#else
#   define PASTEURIZE_ADDR(a)
#endif

#ifndef IN
#   define IN
#endif

#ifndef OUT
#   define OUT
#endif

#define MRKCNT16(_, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, ...) a16
#define MRKASZ(...) MRKCNT16(, ## __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define INB0(lo, x, hi) (((lo) <= (x)) && ((x) <= (hi)))
#define INB1(lo, x, hi) (((lo) <= (x)) && ((x) < (hi)))
#define INB2(lo, x, hi) (((lo) < (x)) && ((x) <= (hi)))
#define INB3(lo, x, hi) (((lo) < (x)) && ((x) < (hi)))

#define MRKLIKELY(x) __builtin_expect((x), 1)
#define MRKUNLIKELY(x) __builtin_expect((x), 0)

#ifdef __cplusplus
}
#endif

#endif
