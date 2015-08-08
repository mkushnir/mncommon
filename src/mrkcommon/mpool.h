#ifndef MRKCOMMON_MPOOL_H
#define MRKCOMMON_MPOOL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _mpool_item {
    size_t sz;
#ifndef NDEBUG
    uint64_t flags;
#endif
    char data[];
};
typedef struct _mpool_ctx {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#endif
    size_t arenasz;
    size_t chunksz;
    int current_chunk;
    int last_chunk;
    off_t current_pos;
    void **arena;
} mpool_ctx_t;


#ifdef MPOOL_USE_STD_MALLOC
#   define mpool_malloc(mpool sz) malloc((sz))
#   define mpool_realloc(mpool ptr, sz) relloc((ptr), (sz))
#   define mpool_free(mpool, ptr) free((ptr))
#   define mpool_ctx_reset(mpool)
#   define mpool_ctx_dump_info(mpool)
#   define mpool_ctx_init(mpool, sz)
#   define mpool_ctx_fini(mpool)
#else
void *mpool_malloc(mpool_ctx_t *, size_t);
void *mpool_realloc(mpool_ctx_t *, void *, size_t);
void mpool_free(mpool_ctx_t *, void *);
void mpool_ctx_reset(mpool_ctx_t *);
void mpool_ctx_dump_info(mpool_ctx_t *);
int mpool_ctx_init(mpool_ctx_t *, size_t);
int mpool_ctx_fini(mpool_ctx_t *);
#endif


#ifdef __cplusplus
}
#endif

#endif
