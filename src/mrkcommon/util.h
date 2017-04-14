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
#   if __STDC_VERSION__ >= 201112
#       define _MAX(a, b) a > b ? a : b
#       define _DMAX(ty, name) \
    static inline ty name(const ty a, const ty b) {return _MAX(a,b);}

_DMAX(char, mnmax_char)
_DMAX(unsigned char, mnmax_uchar)
_DMAX(short, mnmax_short)
_DMAX(unsigned short, mnmax_ushort)
_DMAX(int, mnmax_int)
_DMAX(unsigned int, mnmax_uint)
_DMAX(long, mnmax_long)
_DMAX(unsigned long, mnmax_ulong)
_DMAX(long long, mnmax_llong)
_DMAX(unsigned long long, mnmax_ullong)
_DMAX(float, mnmax_float)
_DMAX(double, mnmax_double)
_DMAX(long double, mnmax_ldouble)
_DMAX(intmax_t, mnmax_intmax)

#       define MAX(a, b)                               \
    _Generic(a,                                        \
             char: mnmax_char,                         \
             unsigned char: mnmax_uchar,               \
             short: mnmax_short,                       \
             unsigned short: mnmax_ushort,             \
             int: mnmax_int,                           \
             unsigned int: mnmax_uint,                 \
             long: mnmax_long,                         \
             unsigned long: mnmax_ulong,               \
             long long: mnmax_llong,                   \
             unsigned long long: mnmax_ullong,         \
             float: mnmax_float,                       \
             double: mnmax_double,                     \
             long double: mnmax_ldouble,               \
             const char: mnmax_char,                   \
             const unsigned char: mnmax_uchar,         \
             const short: mnmax_short,                 \
             const unsigned short: mnmax_ushort,       \
             const int: mnmax_int,                     \
             const unsigned int: mnmax_uint,           \
             const long: mnmax_long,                   \
             const unsigned long: mnmax_ulong,         \
             const long long: mnmax_llong,             \
             const unsigned long long: mnmax_ullong,   \
             const float: mnmax_float,                 \
             const double: mnmax_double,               \
             const long double: mnmax_ldouble,         \
             default: mnmax_intmax)((a),(b))           \


#   else
#       define MAX(a,b) ((a)>(b)?(a):(b))
#   endif
#endif


#ifndef MIN
#   if __STDC_VERSION__ >= 201112
#       define _MIN(a, b) a < b ? a : b
#       define _DMIN(ty, name) \
    static inline ty name(const ty a, const ty b) {return a < b ? a : b;}

_DMIN(char, mnmin_char)
_DMIN(unsigned char, mnmin_uchar)
_DMIN(short, mnmin_short)
_DMIN(unsigned short, mnmin_ushort)
_DMIN(int, mnmin_int)
_DMIN(unsigned int, mnmin_uint)
_DMIN(long, mnmin_long)
_DMIN(unsigned long, mnmin_ulong)
_DMIN(long long, mnmin_llong)
_DMIN(unsigned long long, mnmin_ullong)
_DMIN(float, mnmin_float)
_DMIN(double, mnmin_double)
_DMIN(long double, mnmin_ldouble)
_DMIN(intmax_t, mnmin_intmin)

#       define MIN(a, b)                               \
    _Generic(a,                                        \
             char: mnmin_char,                         \
             unsigned char: mnmin_uchar,               \
             short: mnmin_short,                       \
             unsigned short: mnmin_ushort,             \
             int: mnmin_int,                           \
             unsigned int: mnmin_uint,                 \
             long: mnmin_long,                         \
             unsigned long: mnmin_ulong,               \
             long long: mnmin_llong,                   \
             unsigned long long: mnmin_ullong,         \
             float: mnmin_float,                       \
             double: mnmin_double,                     \
             long double: mnmin_ldouble,               \
             const char: mnmin_char,                   \
             const unsigned char: mnmin_uchar,         \
             const short: mnmin_short,                 \
             const unsigned short: mnmin_ushort,       \
             const int: mnmin_int,                     \
             const unsigned int: mnmin_uint,           \
             const long: mnmin_long,                   \
             const unsigned long: mnmin_ulong,         \
             const long long: mnmin_llong,             \
             const unsigned long long: mnmin_ullong,   \
             const float: mnmin_float,                 \
             const double: mnmin_double,               \
             const long double: mnmin_ldouble,         \
             default: mnmin_intmin)((a),(b))           \


#   else
#       define MIN(a,b) ((a)<(b)?(a):(b))
#   endif
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
