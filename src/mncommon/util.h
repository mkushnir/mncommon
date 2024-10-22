#ifndef MNCOMMON_UTIL_H
#define MNCOMMON_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *mncommon_diag_str(int);

#if defined(__GNUC__) && \
        !defined(__clang__) && \
        !defined(__llvm__) && \
        !defined(__INTEL_COMPILER)
#   define GCC_VERSION (__GNUC__ * 10000 + \
                        __GNUC_MINOR__ * 100 + \
                        __GNUC_PATCHLEVEL__)
#   define __GCC__
#else
#   define GCC_VERSION 0
#endif

#ifdef __GCC__
#   if GCC_VERSION >= 40900
#       define MNCOMMON_GENERIC_SUPPORT
#   endif
#else
#   define MNCOMMON_GENERIC_SUPPORT
#endif

// clang compatibility
#define MNCOMMON_FEATURE_CHECK_STUB(x) 0
#ifndef __has_feature
#   define __has_feature MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_builtin
#   define __has_builtin MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_extension
#   define __has_extension MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_c_attribute
#   define __has_c_attribute MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_cpp_attribute
#   define __has_cpp_attribute MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_attribute
#   define __has_attribute MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_declspec_attribute
#   define __has_declspec_attribute MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __is_identifier
#   define __is_identifier MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_include
#   define __has_include MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_include_next
#   define __has_include_next MNCOMMON_FEATURE_CHECK_STUB
#endif
#ifndef __has_warning
#   define __has_warning MNCOMMON_FEATURE_CHECK_STUB
#endif

#ifdef MAX
#undef MAX
#endif
#define _MAX(a, b) a > b ? a : b
#if __STDC_VERSION__ >= 201112 && defined(MNCOMMON_GENERIC_SUPPORT)
#    define _DMAX(ty, name) \
static inline ty name(const ty a, const ty b) {return _MAX(a, b);}

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

#   define MAX(a, b)                                   \
_Generic(a,                                            \
         char: mnmax_char,                             \
         unsigned char: mnmax_uchar,                   \
         short: mnmax_short,                           \
         unsigned short: mnmax_ushort,                 \
         int: mnmax_int,                               \
         unsigned int: mnmax_uint,                     \
         long: mnmax_long,                             \
         unsigned long: mnmax_ulong,                   \
         long long: mnmax_llong,                       \
         unsigned long long: mnmax_ullong,             \
         float: mnmax_float,                           \
         double: mnmax_double,                         \
         long double: mnmax_ldouble,                   \
         default: mnmax_intmax)((a),(b))               \


#else
#   define MAX(a, b) _MAX((a), (b))
#endif


#ifdef MIN
#undef MIN
#endif
#define _MIN(a, b) a < b ? a : b
#if __STDC_VERSION__ >= 201112 && defined(MNCOMMON_GENERIC_SUPPORT)
#   define _DMIN(ty, name) \
static inline ty name(const ty a, const ty b) {return _MIN(a, b);}

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
_DMIN(intmax_t, mnmin_intmax)

#   define MIN(a, b)                                   \
_Generic(a,                                            \
         char: mnmin_char,                             \
         unsigned char: mnmin_uchar,                   \
         short: mnmin_short,                           \
         unsigned short: mnmin_ushort,                 \
         int: mnmin_int,                               \
         unsigned int: mnmin_uint,                     \
         long: mnmin_long,                             \
         unsigned long: mnmin_ulong,                   \
         long long: mnmin_llong,                       \
         unsigned long long: mnmin_ullong,             \
         float: mnmin_float,                           \
         double: mnmin_double,                         \
         long double: mnmin_ldouble,                   \
         default: mnmin_intmax)((a),(b))               \


#else
#   define MIN(a, b) _MIN((a), (b))
#endif

#if !defined(UNUSED)
#define UNUSED __attribute__ ((__unused__))
#else
//#error UNUSED is already defined. Please un-define UNUSED before including mncommon/util.h
#endif

#if !defined(RESERVED)
#define RESERVED __attribute__ ((__unused__))
#else
#error RESERVED is already defined. Please un-define RESERVED before including mncommon/util.h
#endif

#if !defined(DEPRECATED)
#define DEPRECATED(d) __attribute__ ((__deprecated__(d)))
#else
#error DEPRECATED is already defined. Please un-define DEPRECATED before including mncommon/util.h
#endif

#if !defined(PRINTFLIKE)
#define PRINTFLIKE(i, l) __attribute__ ((format (printf, i, l)))
#else
#error PRINTFLIKE is already defined. Please un-define PRINTFLIKE before including mncommon/util.h
#endif

#if defined(NORETURN)
#error NORETURN is already defined. Please un-define NORETURN before including mncommon/util.h
#endif

#if defined(__dead2)
#   define NORETURN __dead2
#else
#   if __STDC_VERSION__ >= 201112
#       define NORETURN _Noreturn
#   else
#       define NORETURN __attribute__ ((__noreturn__))
#   endif
#endif

#if !defined(PACKED)
#define PACKED __attribute__ ((__packed__))
#else
#error PACKED is already defined. Please un-define PACKED before including mncommon/util.h
#endif

#ifndef countof
#define countof(a) (sizeof(a)/sizeof(a[0]))
#else
#error countof is already defined. Please un-define countof before including mncommon/util.h
#endif

#ifndef NDEBUG
#   define PASTEURIZE_ADDR(a)                  \
do {                                           \
    if (((uintptr_t)a) == 0x5a5a5a5a5a5a5a5a ||\
        ((uintptr_t)a) == 0xa5a5a5a5a5a5a5a5)  \
            abort();                           \
} while (0)                                    \

#else
#   define PASTEURIZE_ADDR(a)
#endif

#ifndef IN
#   define IN
#endif

#ifndef OUT
#   define OUT
#endif

#define MNMAXASZ (32)
#define MNCNT32(_, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, ...) a32
#define MNASZ(...) MNCNT32(, ## __VA_ARGS__, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define INB0(lo, x, hi) (((lo) <= (x)) && ((x) <= (hi)))
#define INB1(lo, x, hi) (((lo) <= (x)) && ((x) < (hi)))
#define INB2(lo, x, hi) (((lo) < (x)) && ((x) <= (hi)))
#define INB3(lo, x, hi) (((lo) < (x)) && ((x) < (hi)))

#define MNLIKELY(x) __builtin_expect((x), 1)
#define MNUNLIKELY(x) __builtin_expect((x), 0)


#define _MNCMP(a, b) (((a) > (b)) - ((a) < (b)))
//#define _MNCMP(a, b) ((a) < (b) ? -1 : (a) > (b) ? 1 : 0)
#if __STDC_VERSION__ >= 201112 && defined(MNCOMMON_GENERIC_SUPPORT)
#   define _DMNCMP(ty, name)   \
static inline int name(const ty a, const ty b) {return _MNCMP(a, b);}

_DMNCMP(char, mncmp_char)
_DMNCMP(unsigned char, mncmp_uchar)
_DMNCMP(short, mncmp_short)
_DMNCMP(unsigned short, mncmp_ushort)
_DMNCMP(int, mncmp_int)
_DMNCMP(unsigned int, mncmp_uint)
_DMNCMP(long, mncmp_long)
_DMNCMP(unsigned long, mncmp_ulong)
_DMNCMP(long long, mncmp_llong)
_DMNCMP(unsigned long long, mncmp_ullong)
_DMNCMP(float, mncmp_float)
_DMNCMP(double, mncmp_double)
_DMNCMP(long double, mncmp_ldouble)
_DMNCMP(void *, mncmp_vptr)

#       define MNCMP(a, b)                             \
_Generic(a,                                            \
         char: mncmp_char,                             \
         _Bool: mncmp_char,                            \
         unsigned char: mncmp_uchar,                   \
         short: mncmp_short,                           \
         unsigned short: mncmp_ushort,                 \
         int: mncmp_int,                               \
         unsigned int: mncmp_uint,                     \
         long: mncmp_long,                             \
         unsigned long: mncmp_ulong,                   \
         long long: mncmp_llong,                       \
         unsigned long long: mncmp_ullong,             \
         float: mncmp_float,                           \
         double: mncmp_double,                         \
         long double: mncmp_ldouble,                   \
         default: mncmp_vptr)((a),(b))                 \


#else
#   define MNCMP(a, b) _MNCMP((a), (b))
#endif


#define _MNCMPR(a, b) ((a) > (b) ? -1 : (a) < (b) ? 1 : 0)
#if __STDC_VERSION__ >= 201112 && defined(MNCOMMON_GENERIC_SUPPORT)
#   define _DMNCMPR(ty, name)   \
static inline int name(const ty a, const ty b) {return _MNCMPR(a, b);}

_DMNCMPR(char, mncmpr_char)
_DMNCMPR(unsigned char, mncmpr_uchar)
_DMNCMPR(short, mncmpr_short)
_DMNCMPR(unsigned short, mncmpr_ushort)
_DMNCMPR(int, mncmpr_int)
_DMNCMPR(unsigned int, mncmpr_uint)
_DMNCMPR(long, mncmpr_long)
_DMNCMPR(unsigned long, mncmpr_ulong)
_DMNCMPR(long long, mncmpr_llong)
_DMNCMPR(unsigned long long, mncmpr_ullong)
_DMNCMPR(float, mncmpr_float)
_DMNCMPR(double, mncmpr_double)
_DMNCMPR(long double, mncmpr_ldouble)
_DMNCMP(void *, mncmpr_vptr)

#       define MNCMPR(a, b)                            \
_Generic(a,                                            \
         char: mncmpr_char,                            \
         _Bool: mncmpr_char,                           \
         unsigned char: mncmpr_uchar,                  \
         short: mncmpr_short,                          \
         unsigned short: mncmpr_ushort,                \
         int: mncmpr_int,                              \
         unsigned int: mncmpr_uint,                    \
         long: mncmpr_long,                            \
         unsigned long: mncmpr_ulong,                  \
         long long: mncmpr_llong,                      \
         unsigned long long: mncmpr_ullong,            \
         float: mncmpr_float,                          \
         double: mncmpr_double,                        \
         long double: mncmpr_ldouble,                  \
         default: mncmpr_vptr)((a),(b))                \


#else
#   define MNCMPR(a, b) _MNCMPR((a), (b))
#endif


#if __STDC_VERSION__ >= 201112 && defined(MNCOMMON_GENERIC_SUPPORT)
#define MNTYPECHK(ty, v) _Generic((v), ty: (v), default: (ty)mn_check_type_failure(#ty))
#else
#define MNTYPECHK(ty, v) (v)
#endif

intptr_t mn_check_type_failure(const char *);


#if __STDC_VERSION__ >= 201112 && defined(MNCOMMON_GENERIC_SUPPORT)
#define MNIDIV(num, denom)                     \
_Generic(num,                                  \
         char: div,                            \
         unsigned char: div,                   \
         short: div,                           \
         unsigned short: div,                  \
         int: div,                             \
         unsigned int: div,                    \
         long: ldiv,                           \
         unsigned long: ldiv,                  \
         long long: lldiv,                     \
         unsigned long long: lldiv,            \
         default: imaxdiv)(num, denom)         \

#else
#define MNIDIV(num, denom) imaxdiv(num, denom)
#endif



#ifdef __cplusplus
}
#endif

#endif
