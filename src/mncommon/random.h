#ifndef MNCOMMON_RANDOM_H
#define MNCOMMON_RANDOM_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

long randint_weighted (long a, long b, size_t wsz, float const weights[static wsz]);

long randint_biased (float a, float b, float bias);

#ifdef __cplusplus
}
#endif

#endif
