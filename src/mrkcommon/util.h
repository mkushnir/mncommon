#ifndef MRKCOMMON_UTIL_H
#define MRKCOMMON_UTIL_H

#ifdef __cplusplus
extern "C" {
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

#ifdef __cplusplus
}
#endif

#endif
