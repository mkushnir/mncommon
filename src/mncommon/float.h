#ifndef MNCOMMON_FLOAT_H
#define MNCOMMON_FLOAT_H

#include <float.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __clang__
#pragma clang diagnostic ignored "-Wabsolute-value"
#endif

#define MNPOW(x, y)            \
_Generic(x,                    \
    float: powf(x, y),         \
    double: pow(x, y),         \
    long double: powl(x, y))   \


#define MNFABS(x)              \
_Generic(x,                    \
    float: fabsf(x),           \
    double: fabs(x),           \
    long double: fabsl(x))     \


#define MNEXP(x)               \
_Generic(x,                    \
    float: expf(x),            \
    double: exp(x),            \
    long double: expl(x))      \


#define MN_EPSILON FLT_EPSILON
#define MNDBL_ISZERO(d) (MNFABS(d) <= MN_EPSILON)
#define MNDBL_EQ(a, b) (MNFABS(a - b) < MN_EPSILON)
#define MNDBL_LT(a, b) ((a) < ((b) - MN_EPSILON))
#define MNDBL_LTE(a, b) ((a) <= ((b) + MN_EPSILON))
#define MNDBL_GT(a, b) ((a) > ((b) + MN_EPSILON))
#define MNDBL_GTE(a, b) ((a) >= ((b) - MN_EPSILON))



#ifdef __cplusplus
}
#endif

#endif
