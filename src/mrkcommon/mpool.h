#ifndef MRKCOMMON_MPOOL_H
#define MRKCOMMON_MPOOL_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _mpool_item {
    size_t sz;
    char data[];
};
typedef struct _mpool_ctx {
    size_t arenasz;
    size_t chunksz;
    int current_chunk;
    int last_chunk;
    off_t current_pos;
    void **arena;
} mpool_ctx_t;


void *mpool_malloc(mpool_ctx_t *, size_t);
void *mpool_realloc(mpool_ctx_t *, void *, size_t);
void mpool_free(mpool_ctx_t *, void *);
void mpool_ctx_reset(mpool_ctx_t *);
int mpool_ctx_init(mpool_ctx_t *, size_t);
int mpool_ctx_fini(mpool_ctx_t *);


#ifdef __cplusplus
}
#endif

#endif
