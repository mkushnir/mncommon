#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#define TRRET_DEBUG_VERBOSE
#include <mrkcommon/dumpm.h>
#include <mrkcommon/array.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>
#include "diag.h"

#ifdef DO_MEMDEBUG
#include <mrkcommon/memdebug.h>
MEMDEBUG_DECLARE(array);

#define MEMDEBUG_INIT(self)                                    \
do {                                                           \
    (self)->mdtag = (uint64_t)memdebug_get_runtime_scope();    \
} while (0)                                                    \


#define MEMDEBUG_ENTER(self)                                   \
{                                                              \
    int mdtag;                                                 \
    mdtag = memdebug_set_runtime_scope((int)(self)->mdtag);    \


#define MEMDEBUG_LEAVE(self)                   \
    (void)memdebug_set_runtime_scope(mdtag);   \
}                                              \


#else
#define MEMDEBUG_INIT(self)
#define MEMDEBUG_ENTER(self)
#define MEMDEBUG_LEAVE(self)
#endif


#define _malloc(sz) mpool_malloc(mpool, (sz))
#define _realloc(p, sz) mpool_realloc(mpool, (p), (sz))
#define _free(p) mpool_free(mpool, (p))

/*
 * Initialize array structure.
 *
 */

#define ARRAY_INIT_BODY(malloc_fn)                             \
    assert(elsz > 0);                                          \
    ar->elsz = elsz;                                           \
    ar->elnum = elnum;                                         \
    ar->init = init;                                           \
    ar->fini = fini;                                           \
    ar->datasz = elsz * elnum;                                 \
    MEMDEBUG_INIT(ar);                                         \
    MEMDEBUG_ENTER(ar);                                        \
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
    MEMDEBUG_LEAVE(ar);                                        \
    return 0;                                                  \


int
array_init(array_t *ar, size_t elsz, size_t elnum,
           array_initializer_t init,
           array_finalizer_t fini)
{
    ARRAY_INIT_BODY(malloc)
}


int
array_init_mpool(mpool_ctx_t *mpool, array_t *ar, size_t elsz, size_t elnum,
           array_initializer_t init,
           array_finalizer_t fini)
{
    ARRAY_INIT_BODY(_malloc)
}


#define ARRAY_RESET_NO_FINI_BODY(free_fn, malloc_fn)           \
    ar->elnum = newelnum;                                      \
    ar->datasz = ar->elsz * newelnum;                          \
    MEMDEBUG_ENTER(ar);                                        \
    if (ar->data != NULL) {                                    \
        free_fn(ar->data);                                     \
    }                                                          \
    if (newelnum > 0) {                                        \
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
        ar->data = NULL;                                       \
    }                                                          \
    MEMDEBUG_LEAVE(ar);                                        \
    return 0;                                                  \



int
array_reset_no_fini(array_t *ar, size_t newelnum)
{
    ARRAY_RESET_NO_FINI_BODY(free, malloc)
}


int
array_reset_no_fini_mpool(mpool_ctx_t *mpool, array_t *ar, size_t newelnum)
{
    ARRAY_RESET_NO_FINI_BODY(_free, _malloc)
}


/*
 * Make sure array is at least newelnum long.
 */

#define ARRAY_ENSURE_DIRTY_BODY(realloc_fn, free_fn, __a)                      \
    void *newdata;                                                             \
    MEMDEBUG_ENTER(ar);                                                        \
    if (!(flags & ARRAY_FLAG_SAVE)) {                                          \
        if (newelnum > 0) {                                                    \
            if (ar->datasz < ar->elsz * newelnum) {                            \
                ar->datasz = ar->elsz * newelnum;                              \
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
            if (ar->datasz < ar->elsz * newelnum) {                            \
                ar->datasz = ar->elsz * newelnum;                              \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                newdata = ar->data;                                            \
            }                                                                  \
        } else if (newelnum < ar->elnum) {                                     \
            if (newelnum > 0) {                                                \
                ar->datasz = ar->elsz * newelnum;                              \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                ar->datasz = 0;                                                \
                free_fn(ar->data);                                             \
                newdata = NULL;                                                \
            }                                                                  \
        } else {                                                               \
            newdata = ar->data;                                                \
        }                                                                      \
    }                                                                          \
    MEMDEBUG_LEAVE(ar);                                                        \
    ar->data = newdata;                                                        \
    __a;                                                                       \



int
array_ensure_len_dirty(array_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(realloc, free,
            ar->elnum = newelnum;
            return 0;
    )
}


int
array_ensure_len_dirty_mpool(mpool_ctx_t *mpool,
                             array_t *ar,
                             size_t newelnum,
                             unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(_realloc, _free,
            ar->elnum = newelnum;
            return 0;
    )
}


void
array_ensure_datasz_dirty(array_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(realloc, free,)
}


void
array_ensure_datasz_dirty_mpool(mpool_ctx_t *mpool,
                                array_t *ar,
                                size_t newelnum,
                                unsigned int flags)
{
    ARRAY_ENSURE_DIRTY_BODY(_realloc, _free,)
}


#define ARRAY_ENSURE_BODY(realloc_fn, free_fn, __a)                            \
    void *newdata;                                                             \
    unsigned i;                                                                \
    MEMDEBUG_ENTER(ar);                                                        \
    if (!(flags & ARRAY_FLAG_SAVE)) {                                          \
        if (ar->fini != NULL) {                                                \
            for (i = 0; i < ar->elnum; ++i) {                                  \
                ar->fini(ar->data + i * ar->elsz);                             \
            }                                                                  \
        }                                                                      \
        if (newelnum > 0) {                                                    \
            if (ar->datasz < ar->elsz * newelnum) {                            \
                ar->datasz = ar->elsz * newelnum;                              \
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
            if (ar->datasz < ar->elsz * newelnum) {                            \
                ar->datasz = ar->elsz * newelnum;                              \
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
                ar->datasz = ar->elsz * newelnum;                              \
                if ((newdata = realloc_fn(ar->data, ar->datasz)) == NULL) {    \
                    FAIL("realloc");                                           \
                }                                                              \
            } else {                                                           \
                ar->datasz = 0;                                                \
                free_fn(ar->data);                                             \
                newdata = NULL;                                                \
            }                                                                  \
        } else {                                                               \
            newdata = ar->data;                                                \
        }                                                                      \
    }                                                                          \
    MEMDEBUG_LEAVE(ar);                                                        \
    ar->data = newdata;                                                        \
    __a;                                                                       \




int
array_ensure_len(array_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_BODY(realloc, free,
            ar->elnum = newelnum;
            return 0;
    )
}


int
array_ensure_len_mpool(mpool_ctx_t *mpool,
                       array_t *ar,
                       size_t newelnum,
                       unsigned int flags)
{
    ARRAY_ENSURE_BODY(_realloc, _free,
            ar->elnum = newelnum;
            return 0;
    )
}


void
array_ensure_datasz(array_t *ar, size_t newelnum, unsigned int flags)
{
    ARRAY_ENSURE_BODY(realloc, free,)
}


void
array_ensure_datasz_mpool(mpool_ctx_t *mpool,
                          array_t *ar,
                          size_t newelnum,
                          unsigned int flags)
{
    ARRAY_ENSURE_BODY(_realloc, _free,)
}


int
array_clear_item(array_t *ar, unsigned idx)
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


void *
array_get(const array_t *ar, unsigned idx)
{
    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }
    return NULL;
}


void *
array_get_safe(array_t *ar, unsigned idx)
{
    size_t datasz;

    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }

    datasz = ar->elsz * (idx + 1);

    if (datasz > ar->datasz) {
        array_ensure_datasz(ar,
                            ar->elnum ? ar->elnum * 2 : 1,
                            ARRAY_FLAG_SAVE);
    }
    assert((ar->elsz * idx) < ar->datasz);
    ar->elnum = idx + 1;
    return ar->data + ar->elsz * idx;
}


void *
array_get_safe_mpool(mpool_ctx_t *mpool, array_t *ar, unsigned idx)
{
    size_t datasz;

    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }

    datasz = ar->elsz * (idx + 1);
    if (datasz > ar->datasz) {
        array_ensure_datasz_mpool(mpool,
                                  ar,
                                  ar->elnum ? ar->elnum * 2 : 1,
                                  ARRAY_FLAG_SAVE);
    }
    assert((ar->elsz * idx) < ar->datasz);
    ar->elnum = idx + 1;
    return ar->data + ar->elsz * idx;
}


int
array_index(const array_t *ar, void *item)
{
    uintptr_t n = (uintptr_t)item;
    uintptr_t s = (uintptr_t)ar->data;
    uintptr_t d = n - s;

    //TRACE("elsz=%ld s=%08lx n=%08lx e=%08lx mod=%ld", ar->elsz, s, n, s * ar->elsz, d % ar->elsz);

    if (n >= s && n < (s * ar->elsz) && d % ar->elsz == 0) {
        return d / ar->elsz;
    }
    return -1;
}


void
array_copy(array_t *dst, const array_t *src)
{
    size_t sz;
    sz = dst->elnum * dst->elsz;
    assert(sz == src->elnum * src->elsz);
    memcpy(dst->data, src->data, sz);
}


void *
array_get_iter(const array_t *ar, array_iter_t *it)
{
    if (it->iter < ar->elnum) {
        return ar->data + ar->elsz * it->iter;
    }
    return NULL;
}


int
array_fini(array_t *ar)
{
    unsigned i;
    MEMDEBUG_ENTER(ar);
    if (ar->data != NULL) {
        if (ar->fini != NULL) {
            for (i = 0; i < ar->elnum; ++i) {
                ar->fini(ar->data + (i * ar->elsz));
            }
        }
        free(ar->data);
    }
    MEMDEBUG_LEAVE(ar);
    ar->data = NULL;
    ar->init = NULL;
    ar->fini = NULL;
    ar->elnum = 0;
    return 0;
}


int
array_fini_mpool(mpool_ctx_t *mpool, array_t *ar)
{
    unsigned i;
    MEMDEBUG_ENTER(ar);
    if (ar->data != NULL) {
        if (ar->fini != NULL) {
            for (i = 0; i < ar->elnum; ++i) {
                ar->fini(ar->data + (i * ar->elsz));
            }
        }
        mpool_free(mpool, ar->data);
    }
    MEMDEBUG_LEAVE(ar);
    ar->data = NULL;
    ar->init = NULL;
    ar->fini = NULL;
    ar->elnum = 0;
    return 0;
}


void *
array_first(const array_t *ar, array_iter_t *iter)
{
    iter->iter = 0;
    if (iter->iter < ar->elnum) {
        return ar->data;
    }
    return NULL;
}


void *
array_last(const array_t *ar, array_iter_t *iter)
{
    iter->iter = ar->elnum - 1;
    if (iter->iter < ar->elnum) {
        return ar->data + iter->iter * ar->elsz;
    }
    return NULL;
}


void *
array_next(const array_t *ar, array_iter_t *iter)
{
    if (++iter->iter < ar->elnum) {
        return ar->data + iter->iter * ar->elsz;
    }
    return NULL;
}


void *
array_prev(const array_t *ar, array_iter_t *iter)
{
    --iter->iter;
    if (iter->iter < ar->elnum) {
        return ar->data + iter->iter * ar->elsz;
    }
    return NULL;
}


void *
array_incr(array_t *ar)
{
    if (array_ensure_len(ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    return array_get(ar, ar->elnum - 1);
}

void *
array_incr_mpool(mpool_ctx_t *mpool, array_t *ar)
{
    if (array_ensure_len_mpool(mpool, ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    return array_get(ar, ar->elnum - 1);
}


void *
array_incr_iter(array_t *ar, array_iter_t *it)
{
    if (array_ensure_len(ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    it->iter = ar->elnum - 1;
    return array_get(ar, ar->elnum - 1);
}


void *
array_incr_iter_mpool(mpool_ctx_t *mpool, array_t *ar, array_iter_t *it)
{
    if (array_ensure_len_mpool(mpool, ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    it->iter = ar->elnum - 1;
    return array_get(ar, ar->elnum - 1);
}


int
array_decr_fast(array_t *ar)
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
array_decr(array_t *ar)
{
    if (array_ensure_len(ar, ar->elnum - 1, ARRAY_FLAG_SAVE) != 0) {
        TRRET(ARRAY_DECR + 1);
    }
    return 0;
}


int
array_decr_mpool(mpool_ctx_t *mpool, array_t *ar)
{
    if (array_ensure_len_mpool(mpool, ar, ar->elnum - 1, ARRAY_FLAG_SAVE) != 0) {
        TRRET(ARRAY_DECR + 1);
    }
    return 0;
}


//int
//array_sort(array_t *ar)
//{
//    if (ar->compar == NULL) {
//        TRRET(ARRAY_SORT + 1);
//    }
//    if (ar->elnum == 0) {
//        TRRET(ARRAY_SORT + 2);
//    }
//    qsort(ar->data, ar->elnum, ar->elsz, ar->compar);
//    return 0;
//}
//
//void *
//array_find(const array_t *ar, const void *key)
//{
//    if (ar->compar == NULL) {
//        TRRETNULL(ARRAY_FIND + 1);
//    }
//    if (ar->elnum == 0) {
//        TRRETNULL(ARRAY_FIND + 2);
//    }
//    return bsearch(key, ar->data, ar->elnum, ar->elsz, ar->compar);
//}


int
array_traverse(array_t *ar, array_traverser_t tr, void *udata)
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
array_cmp(const array_t *ar1, const array_t *ar2, array_compar_t cmp, ssize_t sz)
{
    ssize_t res;
    ssize_t sz1, sz2;
    unsigned i;

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

