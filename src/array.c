#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#define TRRET_DEBUG_VERBOSE
#include <mncommon/assert.h>
#include <mncommon/malloc.h>
#include <mncommon/dumpm.h>
#include <mncommon/array.h>
#include <mncommon/mpool.h>
#include <mncommon/util.h>
#include "diag.h"

#define _malloc(sz) mpool_malloc(mpool, (sz))
#define _realloc(p, sz) mpool_realloc(mpool, (p), (sz))
#define _free(p) mpool_free(mpool, (p))

#define _array_init(ar,        \
                    elsz,      \
                    elnum,     \
                    init,      \
                    fini)      \
array_init_mpool(mpool,        \
                 (ar),         \
                 (elsz),       \
                 (elnum),      \
                 (init),       \
                 (fini))       \


/*
 * Initialize array structure.
 *
 */

#define ARRAY_INIT_BODY(malloc_fn)                             \
    size_t datasz;                                             \
    assert(elsz > 0);                                          \
    ar->elsz = elsz;                                           \
    ar->elnum = elnum;                                         \
    ar->init = init;                                           \
    ar->fini = fini;                                           \
    datasz = elsz * elnum;                                     \
    if (datasz >= MNARRAY_SMALL_DATASZ) {                      \
        ar->datasz = datasz;                                   \
    } else if (datasz > 0) {                                   \
        ar->datasz = 1;                                        \
        while (ar->datasz < datasz) {                          \
            ar->datasz <<= 1;                                  \
        }                                                      \
    } else {                                                   \
        assert(datasz == 0);                                   \
        ar->datasz = 0;                                        \
    }                                                          \
    if (elnum > 0) {                                           \
        if ((ar->data = malloc_fn(ar->datasz)) == NULL) {      \
            TRRET(ARRAY_INIT + 1);                             \
        }                                                      \
        if (ar->init != NULL) {                                \
            unsigned i;                                        \
            for (i = 0; i < elnum; ++i) {                      \
                if (ar->init(ar->data + (i * ar->elsz)) != 0) {\
                    TRRET(ARRAY_INIT + 2);                     \
                }                                              \
            }                                                  \
        }                                                      \
    } else {                                                   \
        ar->data = NULL;                                       \
    }                                                          \
    return 0                                                   \


int
array_init(mnarray_t *ar, size_t elsz, size_t elnum,
           array_initializer_t init,
           array_finalizer_t fini)
{
    ARRAY_INIT_BODY(malloc);
}


int
array_init_mpool(mpool_ctx_t *mpool, mnarray_t *ar, size_t elsz, size_t elnum,
           array_initializer_t init,
           array_finalizer_t fini)
{
    ARRAY_INIT_BODY(_malloc);
}


int
array_init_ref(mnarray_t *ar, void *data, size_t elsz, size_t elnum,
           array_initializer_t init,
           array_finalizer_t fini)
{
    assert(elsz > 0);
    ar->elsz = elsz;
    ar->elnum = elnum;
    ar->data = data;
    ar->datasz = elsz * elnum;
    ar->init = init;

    if (ar->init != NULL) {
        unsigned i;

        for (i = 0; i < elnum; ++i) {
            if (ar->init(ar->data + i * ar->elsz) != 0) {
                TRRET(ARRAY_INIT + 2);
            }
        }
    }
    ar->fini = fini;
    return 0;
}


int
array_fini_ref(mnarray_t *ar)
{
    unsigned i;

    if (ar->fini != NULL) {
        for (i = 0; i < ar->elnum; ++i) {
            ar->fini(ar->data + (i * ar->elsz));
        }
    }

    ar->data = NULL;
    ar->init = NULL;
    ar->fini = NULL;
    ar->elnum = 0;
    return 0;
}


#define ARRAY_INIT_FROM_BODY(malloc_fn)                        \
    size_t datasz;                                             \
    ar->elsz = src->elsz;                                      \
    ar->elnum = elnum;                                         \
    ar->init = src->init;                                      \
    ar->fini = src->fini;                                      \
    datasz = ar->elsz * elnum;                                 \
    if (datasz >= MNARRAY_SMALL_DATASZ) {                      \
        ar->datasz = datasz;                                   \
    } else if (datasz > 0) {                                   \
        ar->datasz = 1;                                        \
        while (ar->datasz < datasz) {                          \
            ar->datasz <<= 1;                                  \
        }                                                      \
    } else {                                                   \
        assert(datasz == 0);                                   \
        ar->datasz = 0;                                        \
    }                                                          \
    if (elnum > 0) {                                           \
        if ((ar->data = malloc_fn(ar->datasz)) == NULL) {      \
            TRRET(ARRAY_INIT + 1);                             \
        }                                                      \
        if (ar->init != NULL) {                                \
            unsigned i;                                        \
            for (i = 0; i < elnum; ++i) {                      \
                if (ar->init(ar->data + (i * ar->elsz)) != 0) {\
                    TRRET(ARRAY_INIT + 2);                     \
                }                                              \
            }                                                  \
        }                                                      \
    } else {                                                   \
        ar->data = NULL;                                       \
    }                                                          \
    return 0                                                   \


int
array_init_from(mnarray_t * restrict ar,
                const mnarray_t * restrict src,
                size_t elnum)
{
    ARRAY_INIT_FROM_BODY(malloc);
}


#define ARRAY_RESET_NO_FINI_BODY(free_fn, malloc_fn)           \
    ar->elnum = newelnum;                                      \
    if (ar->data != NULL) {                                    \
        free_fn(ar->data);                                     \
    }                                                          \
    if (newelnum > 0) {                                        \
        size_t newdatasz;                                      \
        newdatasz = ar->elsz * newelnum;                       \
        if (newdatasz >= MNARRAY_SMALL_DATASZ) {               \
            ar->datasz = newdatasz;                            \
        } else {                                               \
            ar->datasz = 1;                                    \
            while (ar->datasz < newdatasz) {                   \
                ar->datasz <<= 1;                              \
            }                                                  \
        }                                                      \
                                                               \
        if ((ar->data = malloc_fn(ar->datasz)) == NULL) {      \
            TRRET(ARRAY_RESET_NO_FINI + 1);                    \
        }                                                      \
        if (ar->init != NULL) {                                \
            unsigned i;                                        \
            for (i = 0; i < newelnum; ++i) {                   \
                if (ar->init(ar->data + (i * ar->elsz)) != 0) {\
                    TRRET(ARRAY_RESET_NO_FINI + 2);            \
                }                                              \
            }                                                  \
        }                                                      \
    } else {                                                   \
        ar->datasz = 0;                                        \
        ar->data = NULL;                                       \
    }                                                          \
    return 0                                                   \




#define ARRAY_NEW_BODY(malloc_fn, array_init_fn)               \
    mnarray_t *res;                                            \
    if ((res = malloc_fn(sizeof(mnarray_t))) == NULL) {        \
        FAIL("malloc");                                        \
    }                                                          \
    if (array_init_fn(res, elsz, elnum, init, fini) != 0) {    \
        FAIL("array_init");                                    \
    }                                                          \
    return res                                                 \


mnarray_t *
array_new(size_t elsz,
          size_t elnum,
          array_initializer_t init,
          array_finalizer_t fini)
{
    ARRAY_NEW_BODY(malloc, array_init);
}


mnarray_t *
array_new_mpool(mpool_ctx_t *mpool,
                size_t elsz,
                size_t elnum,
                array_initializer_t init,
                array_finalizer_t fini)
{
    ARRAY_NEW_BODY(_malloc, _array_init);
}


const mnarray_t *
array_ref_from(mnarray_t *ref, const mnarray_t *src)
{
    ref->elsz = src->elsz;
    ref->elnum = src->elnum;
    ref->init = NULL;
    ref->fini = NULL;
    ref->data = src->data;
    return (const mnarray_t *)ref;
}


int
array_reset_no_fini(mnarray_t *ar, size_t newelnum)
{
    ARRAY_RESET_NO_FINI_BODY(free, malloc);
}


int
array_reset_no_fini_mpool(mpool_ctx_t *mpool, mnarray_t *ar, size_t newelnum)
{
    ARRAY_RESET_NO_FINI_BODY(_free, _malloc);
}


void
array_destroy(mnarray_t **ar)
{
    if (*ar != NULL) {
        (void)array_fini(*ar);
        free(*ar);
        *ar = NULL;
    }
}


void
array_destroy_mpool(mpool_ctx_t *mpool, mnarray_t **ar)
{
    if (*ar != NULL) {
        (void)array_fini_mpool(mpool, *ar);
        mpool_free(mpool, *ar);
        *ar = NULL;
    }
}

#define ARRAY_ENSURE_SET_DATASZ(m, v)          \
    if (v >= MNARRAY_SMALL_DATASZ) {           \
        m = v;                                 \
    } else {                                   \
        if ((m == 0) ||                        \
            (m > MNARRAY_SMALL_DATASZ)) {      \
            m = 1;                             \
        }                                      \
        while (m < v) {                        \
            m <<= 1;                           \
        }                                      \
    }                                          \

/*
 * Make sure array is at least newelnum long.
 */

#define ARRAY_ENSURE_DIRTY_BODY(realloc_fn, free_fn, __a)                      \
    void *newdata;                                                             \
    if (!(flags & ARRAY_FLAG_SAVE)) {                                          \
        if (newelnum > 0) {                                                    \
            size_t newdatasz;                                                  \
            newdatasz = ar->elsz * newelnum;                                   \
            if (ar->datasz < newdatasz) {                                      \
                ARRAY_ENSURE_SET_DATASZ(ar->datasz, newdatasz);                \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                newdata = ar->data;                                            \
            }                                                                  \
        } else {                                                               \
            ar->datasz = 0;                                                    \
            free_fn(ar->data);                                                 \
            newdata = NULL;                                                    \
        }                                                                      \
    } else {                                                                   \
        if (newelnum > ar->elnum) {                                            \
            size_t newdatasz;                                                  \
            newdatasz = ar->elsz * newelnum;                                   \
            if (ar->datasz < newdatasz) {                                      \
                ARRAY_ENSURE_SET_DATASZ(ar->datasz, newdatasz);                \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                newdata = ar->data;                                            \
            }                                                                  \
        } else {                                                               \
            if (newelnum > 0) {                                                \
                /*                                                             \
                if (ar->datasz == 0 || ar->datasz > MNARRAY_SMALL_DATASZ) {    \
                    ar->datasz = 1;                                            \
                }                                                              \
                while (ar->datasz < ar->elsz * newelnum) {                     \
                    ar->datasz <<= 1;                                          \
                }                                                              \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
                */                                                             \
                /*ar->datasz = ar->elsz * newelnum;*/                          \
                newdata = ar->data;                                            \
            } else {                                                           \
                ar->datasz = 0;                                                \
                free_fn(ar->data);                                             \
                newdata = NULL;                                                \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    ar->data = newdata;                                                        \
    __a                                                                        \



int
array_ensure_len_dirty(mnarray_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(realloc, free,
            ar->elnum = newelnum;
            return 0
    );
}


int
array_ensure_len_dirty_mpool(mpool_ctx_t *mpool,
                             mnarray_t *ar,
                             size_t newelnum,
                             unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(_realloc, _free,
            ar->elnum = newelnum;
            return 0
    );
}


void
array_ensure_datasz_dirty(mnarray_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(realloc, free,);
}


void
array_ensure_datasz_dirty_mpool(mpool_ctx_t *mpool,
                                mnarray_t *ar,
                                size_t newelnum,
                                unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(_realloc, _free,);
}


#define ARRAY_ENSURE_BODY(realloc_fn, free_fn, __a)                            \
    void *newdata;                                                             \
    unsigned i;                                                                \
    if (!(flags & ARRAY_FLAG_SAVE)) {                                          \
        if (ar->fini != NULL) {                                                \
            for (i = 0; i < ar->elnum; ++i) {                                  \
                ar->fini(ar->data + i * ar->elsz);                             \
            }                                                                  \
        }                                                                      \
        if (newelnum > 0) {                                                    \
            size_t newdatasz;                                                  \
            newdatasz = ar->elsz * newelnum;                                   \
            if (ar->datasz < newdatasz) {                                      \
                ARRAY_ENSURE_SET_DATASZ(ar->datasz, newdatasz);                \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                newdata = ar->data;                                            \
            }                                                                  \
        } else {                                                               \
            ar->datasz = 0;                                                    \
            free_fn(ar->data);                                                 \
            newdata = NULL;                                                    \
        }                                                                      \
        if (ar->init != NULL) {                                                \
            for (i = 0; i < newelnum; ++i) {                                   \
                ar->init(newdata + i * ar->elsz);                              \
            }                                                                  \
        }                                                                      \
    } else {                                                                   \
        if (newelnum > ar->elnum) {                                            \
            size_t newdatasz;                                                  \
            newdatasz = ar->elsz * newelnum;                                   \
            if (ar->datasz < newdatasz) {                                      \
                ARRAY_ENSURE_SET_DATASZ(ar->datasz, newdatasz);                \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                newdata = ar->data;                                            \
            }                                                                  \
            if (ar->init != NULL) {                                            \
                for (i = ar->elnum; i < newelnum; ++i) {                       \
                    ar->init(newdata + i * ar->elsz);                          \
                }                                                              \
            }                                                                  \
        } else if (newelnum < ar->elnum) {                                     \
            if (ar->fini != NULL) {                                            \
                for (i = newelnum; i < ar->elnum; ++i) {                       \
                    ar->fini(ar->data + i * ar->elsz);                         \
                }                                                              \
            }                                                                  \
            if (newelnum > 0) {                                                \
                /*                                                             \
                ar->datasz = ar->elsz * newelnum;                              \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
                */                                                             \
                newdata = ar->data;                                            \
            } else {                                                           \
                ar->datasz = 0;                                                \
                free_fn(ar->data);                                             \
                newdata = NULL;                                                \
            }                                                                  \
        } else {                                                               \
            newdata = ar->data;                                                \
        }                                                                      \
    }                                                                          \
    ar->data = newdata;                                                        \
    __a                                                                        \




int
array_ensure_len(mnarray_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_BODY(realloc, free,
            ar->elnum = newelnum;
            return 0
    );
}


int
array_ensure_len_mpool(mpool_ctx_t *mpool,
                       mnarray_t *ar,
                       size_t newelnum,
                       unsigned int flags)
{
    ARRAY_ENSURE_BODY(_realloc, _free,
            ar->elnum = newelnum;
            return 0
    );
}


void
array_ensure_datasz(mnarray_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_BODY(realloc, free,);
}


void
array_ensure_datasz_mpool(mpool_ctx_t *mpool,
                          mnarray_t *ar,
                          size_t newelnum,
                          unsigned int flags)
{
    ARRAY_ENSURE_BODY(_realloc, _free,);
}


int
array_clear_item(mnarray_t *ar, unsigned idx)
{
    //TRACE("idx=%d elnum=%ld", idx, ar->elnum);
    if (idx >= ar->elnum) {
        TRRET(ARRAY_CLEAR_ITEM + 1);
    }
    if (ar->fini != NULL) {
        return ar->fini(ar->data + ar->elsz * idx);
    }
    return 0;
}


int
array_clear(mnarray_t *ar)
{
    if (ar->fini != NULL) {
        int res;
        unsigned i;
        for (i = 0; i < ar->elnum; ++i) {
            if ((res = ar->fini(ar->data + i * ar->elsz)) != 0) {
                return res;
            }
        }
    }
    ar->elnum = 0;
    return 0;
}


int
array_init_item(mnarray_t *ar, unsigned idx)
{
    //TRACE("idx=%d elnum=%ld", idx, ar->elnum);
    if (idx >= ar->elnum) {
        TRRET(ARRAY_INIT_ITEM + 1);
    }
    if (ar->init != NULL) {
        return ar->init(ar->data + ar->elsz * idx);
    }
    return 0;
}


void *
array_get(const mnarray_t *ar, unsigned idx)
{
    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }
    return NULL;
}


void *
array_get_safe(mnarray_t *ar, unsigned idx)
{
    size_t datasz;

    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }

    datasz = ar->elsz * (idx + 1);

    if (datasz > ar->datasz) {
        array_ensure_datasz(ar,
                            (idx + 1) * 2,
                            ARRAY_FLAG_SAVE);
    }
    assert((ar->elsz * idx) < ar->datasz);
    ar->elnum = idx + 1;
    return ar->data + ar->elsz * idx;
}


void *
array_get_safe_mpool(mpool_ctx_t *mpool, mnarray_t *ar, unsigned idx)
{
    size_t datasz;

    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }

    datasz = ar->elsz * (idx + 1);
    if (datasz > ar->datasz) {
        array_ensure_datasz_mpool(mpool,
                                  ar,
                                  (idx + 1) * 2,
                                  ARRAY_FLAG_SAVE);
    }
    assert((ar->elsz * idx) < ar->datasz);
    ar->elnum = idx + 1;
    return ar->data + ar->elsz * idx;
}


int
array_index(const mnarray_t *ar, void *item)
{
    uintptr_t n = (uintptr_t)item;
    uintptr_t s = (uintptr_t)ar->data;
    uintptr_t d = n - s;

    if (n >= s && n < (s * ar->elsz) && d % ar->elsz == 0) {
        return d / ar->elsz;
    }
    return -1;
}


void
array_copy(mnarray_t * restrict dst, const mnarray_t * restrict src)
{
    size_t sz;
    sz = dst->elnum * dst->elsz;
    assert(sz == src->elnum * src->elsz);
    memcpy(dst->data, src->data, sz);
}


void
arary_copy_slice(mnarray_t * restrict dst,
                 const mnarray_t * restrict src,
                 unsigned idx)
{
    size_t sz;
    const char *p;

    assert(dst->elnum <= (src->elnum - idx));
    assert(dst->elsz == src->elsz);

    sz = dst->elnum * dst->elsz;
    p = src->data;
    memcpy(dst->data, p + idx * src->elsz, sz);
}


void *
array_get_iter(const mnarray_t *ar, mnarray_iter_t *it)
{
    if (it->iter < ar->elnum) {
        return ar->data + ar->elsz * it->iter;
    }
    return NULL;
}


int
array_fini(mnarray_t *ar)
{
    unsigned i;
    if (ar->data != NULL) {
        if (ar->fini != NULL) {
            for (i = 0; i < ar->elnum; ++i) {
                ar->fini(ar->data + (i * ar->elsz));
            }
        }
        free(ar->data);
    }
    ar->data = NULL;
    ar->init = NULL;
    ar->fini = NULL;
    ar->elnum = 0;
    return 0;
}


int
array_fini_mpool(mpool_ctx_t *mpool, mnarray_t *ar)
{
    unsigned i;
    if (ar->data != NULL) {
        if (ar->fini != NULL) {
            for (i = 0; i < ar->elnum; ++i) {
                ar->fini(ar->data + (i * ar->elsz));
            }
        }
        mpool_free(mpool, ar->data);
    }
    ar->data = NULL;
    ar->init = NULL;
    ar->fini = NULL;
    ar->elnum = 0;
    return 0;
}


void *
array_first(const mnarray_t *ar, mnarray_iter_t *it)
{
    it->iter = 0;
    if (it->iter < ar->elnum) {
        return ar->data;
    }
    return NULL;
}


void *
array_last(const mnarray_t *ar, mnarray_iter_t *it)
{
    it->iter = ar->elnum - 1;
    if (it->iter < ar->elnum) {
        return ar->data + it->iter * ar->elsz;
    }
    return NULL;
}


void *
array_next(const mnarray_t *ar, mnarray_iter_t *it)
{
    if (++it->iter < ar->elnum) {
        return ar->data + it->iter * ar->elsz;
    }
    return NULL;
}


void *
array_prev(const mnarray_t *ar, mnarray_iter_t *it)
{
    --it->iter;
    if (it->iter < ar->elnum) {
        return ar->data + it->iter * ar->elsz;
    }
    return NULL;
}


void *
array_incr(mnarray_t *ar)
{
    if (array_ensure_len(ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    return array_get(ar, ar->elnum - 1);
}

void *
array_incr_mpool(mpool_ctx_t *mpool, mnarray_t *ar)
{
    if (array_ensure_len_mpool(mpool, ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    return array_get(ar, ar->elnum - 1);
}


void *
array_incr_iter(mnarray_t *ar, mnarray_iter_t *it)
{
    if (array_ensure_len(ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    it->iter = ar->elnum - 1;
    return array_get(ar, ar->elnum - 1);
}


void *
array_incr_iter_mpool(mpool_ctx_t *mpool, mnarray_t *ar, mnarray_iter_t *it)
{
    if (array_ensure_len_mpool(mpool, ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    it->iter = ar->elnum - 1;
    return array_get(ar, ar->elnum - 1);
}


int
array_decr_fast(mnarray_t *ar)
{
    if (!ar->elnum) {
        TRRET(ARRAY_DECR_FAST + 1);
    }
    --ar->elnum;
    if (ar->fini != NULL) {
        (void)ar->fini(ar->data + ar->elnum * ar->elsz);
    }
    return 0;
}


int
array_decr(mnarray_t *ar)
{
    if (!ar->elnum) {
        TRRET(ARRAY_DECR + 1);
    }
    if (array_ensure_len(ar, ar->elnum - 1, ARRAY_FLAG_SAVE) != 0) {
        TRRET(ARRAY_DECR + 2);
    }
    return 0;
}


int
array_decr_mpool(mpool_ctx_t *mpool, mnarray_t *ar)
{
    if (!ar->elnum) {
        TRRET(ARRAY_DECR + 1);
    }
    if (array_ensure_len_mpool(mpool, ar, ar->elnum - 1, ARRAY_FLAG_SAVE) != 0) {
        TRRET(ARRAY_DECR + 2);
    }
    return 0;
}


int
array_sort(mnarray_t *ar, array_compar_t compar)
{
    if (ar->elnum == 0) {
        TRRET(ARRAY_SORT + 1);
    }
    qsort(ar->data, ar->elnum, ar->elsz, compar);
    return 0;
}

void *
array_find(const mnarray_t *ar, const void *key, array_compar_t compar)
{
    if (ar->elnum == 0) {
        TRRETNULL(ARRAY_FIND + 1);
    }
    return bsearch(key, ar->data, ar->elnum, ar->elsz, compar);
}


void *
array_find_linear(const mnarray_t *ar, const void *key, array_compar_t compar)
{
    void *item;
    mnarray_iter_t it;

    if (ar->elnum == 0) {
        TRRETNULL(ARRAY_FIND_LINEAR + 1);
    }
    for (item = array_first(ar, &it);
         item != NULL;
         item = array_next(ar, &it)) {
        if (compar(item, &key) == 0) {
            break;
        }
    }
    return item;
}


int
array_traverse(mnarray_t *ar, array_traverser_t tr, void *udata)
{
    unsigned i;
    int res;
    for (i = 0; i < ar->elnum; ++i) {
        if ((res = tr(ar->data + i * ar->elsz, udata)) != 0) {
            return res;
        }
    }
    return 0;
}


int
array_cmp(const mnarray_t * restrict ar1,
          const mnarray_t * restrict ar2,
          array_compar_t cmp,
          ssize_t sz)
{
    ssize_t res;
    ssize_t sz1, sz2;
    unsigned i;

    assert(ar1->elnum <= ar2->elnum);

    if (sz > 0) {
        sz1 = MIN(sz, (ssize_t)(ar1->elnum));
        sz2 = MIN(sz, (ssize_t)(ar2->elnum));
    } else {
        sz1 = (ssize_t)(ar1->elnum);
        sz2 = (ssize_t)(ar2->elnum);
    }
    res = sz1 - sz2;


    for (i = 0; res == 0 && i < sz1; ++i) {
        res = cmp(array_get(ar1, i), array_get(ar2, i));
    }

    return (int)res;
}
