#ifndef MRKCOMMON_FLOAT_H
#define MRKCOMMON_FLOAT_H

#include <float.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


#define MN_EPSILON FLT_EPSILON
#define MNDBL_ISZERO(d) (fabs(d) <= MN_EPSILON)
#define MNDBL_LT(a, b) ((a) < ((b) - MN_EPSILON))
#define MNDBL_LTE(a, b) ((a) <= ((b) + MN_EPSILON))
#define MNDBL_GT(a, b) ((a) > ((b) + MN_EPSILON))
#define MNDBL_GTE(a, b) ((a) >= ((b) - MN_EPSILON))
#define MNDBL_EQ(a, b) (fabs(a - b) < MN_EPSILON)


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



#ifdef __cplusplus
}
#endif

#endif
