#ifndef MRKCOMMON_ARRAY_H
#define MRKCOMMON_ARRAY_H

#include <sys/types.h>

#include <mrkcommon/mpool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*array_initializer_t) (void *);
typedef int (*array_finalizer_t) (void *);
typedef int (*array_traverser_t) (void *, void *);
typedef int (*array_compar_t) (const void *, const void *);

/**/
typedef struct _array {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#endif
    size_t elsz;
    size_t elnum;
    void *data;
    size_t datasz;
    array_initializer_t init;
    array_finalizer_t fini;
} array_t;

typedef struct _array_iter {
    unsigned iter;
    void *data;
} array_iter_t;

int array_init(array_t *, size_t, size_t,
               array_initializer_t,
               array_finalizer_t);

int array_init_mpool(mpool_ctx_t *,
                     array_t *, size_t, size_t,
                     array_initializer_t,
                     array_finalizer_t);

int array_reset_no_fini(array_t *, size_t);
int array_reset_no_fini_mpool(mpool_ctx_t *, array_t *, size_t);

#define ARRAY_FLAG_SAVE 0x01
int array_ensure_len(array_t *, size_t, unsigned int);
int array_ensure_len_dirty(array_t *, size_t, unsigned int);
int array_ensure_len_mpool(mpool_ctx_t *, array_t *, size_t, unsigned int);
int array_ensure_len_dirty_mpool(mpool_ctx_t *, array_t *, size_t, unsigned int);
void array_ensure_datasz(array_t *, size_t, unsigned int);
void array_ensure_datasz_mpool(mpool_ctx_t *, array_t *, size_t, unsigned int);
void array_ensure_datasz_dirty(array_t *, size_t, unsigned int);
void array_ensure_datasz_dirty_mpool(mpool_ctx_t *, array_t *, size_t, unsigned int);
void *array_get(const array_t *, unsigned);
#define ARRAY_GET(ty, a, i) (((ty *)((a)->data)) + i)
void *array_get_safe(array_t *, unsigned);
void *array_get_safe_mpool(mpool_ctx_t *, array_t *, unsigned);
int array_index(const array_t *, void *);
void array_copy(array_t *, const array_t *);
void *array_get_iter(const array_t *, array_iter_t *);
int array_clear_item(array_t *, unsigned);
void *array_incr(array_t *);
void *array_incr_mpool(mpool_ctx_t *, array_t *);
void *array_incr_iter(array_t *, array_iter_t *);
void *array_incr_iter_mpool(mpool_ctx_t *, array_t *, array_iter_t *);
int array_decr(array_t *);
int array_decr_fast(array_t *);
int array_decr_mpool(mpool_ctx_t *, array_t *);
int array_fini(array_t *);
int array_fini_mpool(mpool_ctx_t *mpool, array_t *);
void *array_first(const array_t *, array_iter_t *);
#define ARRAY_FIRST(ty, a) (((ty *)((a)->data))[0])
void *array_last(const array_t *, array_iter_t *);
#define ARRAY_LAST(ty, a) (((ty *)((a)->data))[(a)->elnum - 1])
void *array_next(const array_t *, array_iter_t *);
void *array_prev(const array_t *, array_iter_t *);
//int array_sort(array_t *);
//void *array_find(const array_t *, const void *);
int array_traverse(array_t *, array_traverser_t, void *);
int array_cmp(const array_t *, const array_t *, array_compar_t, ssize_t);

#ifdef __cplusplus
}
#endif

#endif
