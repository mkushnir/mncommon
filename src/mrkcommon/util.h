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

#ifdef __cplusplus
}
#endif

#endif
