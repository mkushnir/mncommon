#include <stdint.h>
#include <strings.h>

#include <mncommon/float.h>

double
mnfloord (double a, double m)
{
    return a - fmod(a, m);

#if 0
    double res = a;
    union {
        double f;
        uint64_t i;
    } ua, um;
    int ee;
    uint64_t dividend, divisor, rem;
    int implied_one_shift;


    ua.f = a;
    um.f = m;

    ee = MN_EXPONENT_DOUBLE(ua.i) - MN_EXPONENT_DOUBLE(um.i);
    dividend = MN_MANTISSA_DOUBLE(ua.i);
    divisor = MN_MANTISSA_DOUBLE(um.i) >> ee;

    rem = dividend % divisor;

    dividend -= rem;

    implied_one_shift =  DBL_MANT_DIG - flsl(dividend);
    dividend <<= implied_one_shift;

    // adjust exponent
    uint64_t iexp = _MN_EXPONENT_DOUBLE(ua.i) - implied_one_shift;

    // compose result
    ua.i = (ua.i & MN_SIGN_MASK_DOUBLE) |
           (iexp << MN_MANTISSA_WIDTH_DOUBLE) |
           (dividend & MN_MANTISSA_MASK_DOUBLE);

    res = ua.f;

    return res;
#endif
}

float
mnfloorf (float a, float m)
{
    return a - fmodf(a, m);
}


double
mnceild (double a, double m)
{
    double r = fmod(a, m);
    return a - r + ((r != 0.0) ? m : 0.0);
}


float
mnceilf (float a, float m)
{
    float r = fmodf(a, m);
    return a - r + ((r != 0.0) ? m : 0.0);
}
