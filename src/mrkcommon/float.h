#ifndef MRKCOMMON_FLOAT_H
#define MRKCOMMON_FLOAT_H

#include <float.h>

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


#ifdef __cplusplus
}
#endif

#endif
