#ifndef MRKCOMMON_FNVHASH_H_DEFINED
#define MRKCOMMON_FNVHASH_H_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/*
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */

#define FNV_OFFSET_BASIS64 ((uint64_t)14695981039346656037ul)
uint64_t fnvhash64(uint64_t, const unsigned char *, size_t);

#ifdef __cplusplus
}
#endif
#endif /* MRKCOMMON_FNVHASH_H_DEFINED */
