#ifndef MNCOMMON_FASTHASH_H_DEFINED
#define MNCOMMON_FASTHASH_H_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

uint64_t fasthash(uint64_t, const unsigned char *, size_t);

#ifdef __cplusplus
}
#endif
#endif /* MNCOMMON_FASTHASH_H_DEFINED */

