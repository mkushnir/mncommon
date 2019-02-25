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
typedef struct _mnarray {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#endif
    size_t elsz;
    size_t elnum;
    void *data;
#ifndef MNARRAY_SMALL_DATASZ
#   define MNARRAY_SMALL_DATASZ (0x100000ul)
#endif
    size_t datasz;
    array_initializer_t init;
    array_finalizer_t fini;
} mnarray_t;

typedef struct _mnarray_iter {
    unsigned iter;
    void *data;
} mnarray_iter_t;

int array_init(mnarray_t *, size_t, size_t,
               array_initializer_t,
               array_finalizer_t);

int array_init_mpool(mpool_ctx_t *,
                     mnarray_t *, size_t, size_t,
                     array_initializer_t,
                     array_finalizer_t);

int array_init_ref(mnarray_t *, void *, size_t, size_t,
                   array_initializer_t,
                   array_finalizer_t);

int array_reset_no_fini(mnarray_t *, size_t);
int array_reset_no_fini_mpool(mpool_ctx_t *, mnarray_t *, size_t);

mnarray_t *array_new(size_t, size_t,
                   array_initializer_t,
                   array_finalizer_t);

mnarray_t *array_new_mpool(mpool_ctx_t *,
                         size_t, size_t,
                         array_initializer_t,
                         array_finalizer_t);

#define ARRAY_FLAG_SAVE 0x01
int array_ensure_len(mnarray_t *, size_t, unsigned int);
int array_ensure_len_dirty(mnarray_t *, size_t, unsigned int);
int array_ensure_len_mpool(mpool_ctx_t *, mnarray_t *, size_t, unsigned int);
int array_ensure_len_dirty_mpool(mpool_ctx_t *, mnarray_t *, size_t, unsigned int);
void array_ensure_datasz(mnarray_t *, size_t, unsigned int);
void array_ensure_datasz_mpool(mpool_ctx_t *, mnarray_t *, size_t, unsigned int);
void array_ensure_datasz_dirty(mnarray_t *, size_t, unsigned int);
void array_ensure_datasz_dirty_mpool(mpool_ctx_t *, mnarray_t *, size_t, unsigned int);
void *array_get(const mnarray_t *, unsigned);
#define ARRAY_GET(ty, a, i) (((ty *)((a)->data)) + i)
void *array_get_safe(mnarray_t *, unsigned);
void *array_get_safe_mpool(mpool_ctx_t *, mnarray_t *, unsigned);
int array_index(const mnarray_t *, void *);
void array_copy(mnarray_t *, const mnarray_t *);
void *array_get_iter(const mnarray_t *, mnarray_iter_t *);
int array_clear_item(mnarray_t *, unsigned);
int array_clear(mnarray_t *);
int array_init_item(mnarray_t *, unsigned);
void *array_incr(mnarray_t *);
void *array_incr_mpool(mpool_ctx_t *, mnarray_t *);
void *array_incr_iter(mnarray_t *, mnarray_iter_t *);
void *array_incr_iter_mpool(mpool_ctx_t *, mnarray_t *, mnarray_iter_t *);
int array_decr(mnarray_t *);
int array_decr_fast(mnarray_t *);
int array_decr_mpool(mpool_ctx_t *, mnarray_t *);
int array_fini(mnarray_t *);
int array_fini_mpool(mpool_ctx_t *, mnarray_t *);
int array_fini_ref(mnarray_t *);
void array_destroy(mnarray_t **);
void array_destroy_mpool(mpool_ctx_t *, mnarray_t **);
void *array_first(const mnarray_t *, mnarray_iter_t *);
#define ARRAY_FIRST(ty, a) (((ty *)((a)->data))[0])
void *array_last(const mnarray_t *, mnarray_iter_t *);
#define ARRAY_LAST(ty, a) (((ty *)((a)->data))[(a)->elnum - 1])
void *array_next(const mnarray_t *, mnarray_iter_t *);
void *array_prev(const mnarray_t *, mnarray_iter_t *);

int array_sort(mnarray_t *, array_compar_t);
void *array_find(const mnarray_t *, const void *, array_compar_t);
void * array_find_linear(const mnarray_t *, const void *, array_compar_t);

int array_traverse(mnarray_t *, array_traverser_t, void *);
int array_cmp(const mnarray_t *, const mnarray_t *, array_compar_t, ssize_t);

#ifdef __cplusplus
}
#endif

#endif
