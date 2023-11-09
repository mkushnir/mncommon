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

#define MN_MANTISSA_WIDTH_FLOAT (FLT_MANT_DIG - 1)
#define MN_MANTISSA_MASK_FLOAT ((((uint32_t)1) << MN_MANTISSA_WIDTH_FLOAT) - 1)

#define MN_EXPONENT_WIDTH_FLOAT ((int)(sizeof(float) * 8 - MN_MANTISSA_WIDTH_FLOAT - 1))
#define _MN_EXPONENT_MASK_FLOAT ((((uint32_t)1) << MN_EXPONENT_WIDTH_FLOAT) - 1)
#define MN_EXPONENT_MASK_FLOAT (_MN_EXPONENT_MASK_FLOAT << MN_MANTISSA_WIDTH_FLOAT)

#define MN_MANTISSA_WIDTH_DOUBLE (DBL_MANT_DIG - 1)
#define MN_MANTISSA_MASK_DOUBLE ((((uint64_t)1) << MN_MANTISSA_WIDTH_DOUBLE) - 1)
#define MN_MANTISSA_DOUBLE(i) ((i) & MN_MANTISSA_MASK_DOUBLE | ((1ul) << MN_MANTISSA_WIDTH_DOUBLE))


#define MN_EXPONENT_WIDTH_DOUBLE ((int)(sizeof(double) * 8 - MN_MANTISSA_WIDTH_DOUBLE - 1))
#define _MN_EXPONENT_MASK_DOUBLE ((((uint64_t)1) << MN_EXPONENT_WIDTH_DOUBLE) - 1)
#define MN_EXPONENT_MASK_DOUBLE (_MN_EXPONENT_MASK_DOUBLE << MN_MANTISSA_WIDTH_DOUBLE)
#define _MN_EXPONENT_DOUBLE(i) ((int)(((i) & MN_EXPONENT_MASK_DOUBLE) >> MN_MANTISSA_WIDTH_DOUBLE))
#define MN_EXPONENT_DOUBLE(i) (_MN_EXPONENT_DOUBLE(i) - (DBL_MAX_EXP - 1))

#define MN_SIGN_MASK_DOUBLE ((1ul) << (sizeof(double) * 8 - 1))


double mnfloord (double, double);
float mnfloorf (float, float);

#define MNFLOOR(a, m)                          \
_Generic(a,                                    \
    float: mnfloorf(a, m),                     \
    double: mnfloord(a, m),                    \
    default: mnfloorf((float)a, (float)m))     \


double mnceild (double, double);
float mnceilf (float, float);

#define MNCEIL(a, m)                           \
_Generic(a,                                    \
    float: mnceilf(a, m),                      \
    double: mnceild(a, m),                     \
    default: mnceilf((float)a, (float)m))      \



#ifdef __cplusplus
}
#endif

#endif
