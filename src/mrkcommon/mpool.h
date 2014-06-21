#ifndef MRKCOMMON_MPOOL_H
#define MRKCOMMON_MPOOL_H

#include <stddef.h>
#include <mrkcommon/array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mpool_ctx {
    size_t chunksz;
    int current_chunk;
    off_t current_pos;
    array_t chunks;
} mpool_ctx_t;


void *mpool_malloc(mpool_ctx_t *, size_t);
void mpool_ctx_reset(mpool_ctx_t *);
int mpool_ctx_init(mpool_ctx_t *, size_t);
int mpool_ctx_fini(mpool_ctx_t *);


#ifdef __cplusplus
}
#endif

#endif
