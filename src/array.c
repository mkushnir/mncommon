#include <assert.h>
#include <stdlib.h>

#include "mrkcommon/dumpm.h"
#include "mrkcommon/array.h"
#include "diag.h"


/*
 * Initialize array structure.
 *
 */
int
array_init (array_t *ar, size_t elsz, size_t elnum,
            array_initializer_t init,
            array_finalizer_t fini)
{
    unsigned i;

    assert(elsz > 0);
    ar->elsz = elsz;
    ar->elnum = elnum;
    ar->init = init;
    ar->fini = fini;
    ar->compar = NULL;
    if (elnum > 0) {
        if ((ar->data = malloc(elsz * elnum)) == NULL) {
            TRRET(ARRAY_INIT + 1);
        }
        if (ar->init != NULL) {
            for (i = 0; i < elnum; ++i) {
                if (ar->init(ar->data + (i * ar->elsz)) != 0) {
                    TRRET(ARRAY_INIT + 2);
                }
            }
        }
    } else {
        ar->data = NULL;
    }
    return 0;
}

/*
 * Make sure array is at least newelnum long.
 */
int
array_ensure_len(array_t *ar, size_t newelnum, unsigned int flags)
{
    void *newdata;
    unsigned i;

    if (!(flags & ARRAY_FLAG_SAVE)) {
        if (ar->fini != NULL) {
            for (i = 0; i < ar->elnum; ++i) {
                ar->fini(ar->data + i * ar->elsz);
            }
        }
        if (newelnum > 0) {
            if ((newdata = realloc(ar->data, ar->elsz * newelnum)) == NULL) {
                TRRET(ARRAY_ENSURE_LEN + 1);
            }
        } else {
            free(ar->data);
            ar->data = NULL;
            newdata = NULL;
        }
        if (ar->init != NULL) {
            for (i = 0; i < newelnum; ++i) {
                ar->init(newdata + i * ar->elsz);
            }
        }

    } else {
        if (newelnum > ar->elnum) {
            if ((newdata = realloc(ar->data, ar->elsz * newelnum)) == NULL) {
                TRRET(ARRAY_ENSURE_LEN + 2);
            }
            if (ar->init != NULL) {
                for (i = ar->elnum; i < newelnum; ++i) {
                    ar->init(newdata + i * ar->elsz);
                }
            }
        } else if (newelnum < ar->elnum) {
            if (ar->fini != NULL) {
                for (i = newelnum; i < ar->elnum; ++i) {
                    ar->fini(ar->data + i * ar->elsz);
                }
            }
            if (newelnum > 0) {
                if ((newdata = realloc(ar->data, ar->elsz * newelnum)) == NULL) {
                    TRRET(ARRAY_ENSURE_LEN + 3);
                }
            } else {
                free(ar->data);
                ar->data = NULL;
                newdata = NULL;
            }
        } else {
            newdata = ar->data;
        }
    }
    ar->data = newdata;
    ar->elnum = newelnum;
    return 0;
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
array_get(array_t *ar, unsigned idx)
{
    if (idx < ar->elnum) {
        return ar->data + ar->elsz * idx;
    }
    return NULL;
}

int
array_index(array_t *ar, void *item)
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

void *
array_get_iter(array_t *ar, array_iter_t *it)
{
    if (it->iter < ar->elnum) {
        return ar->data + ar->elsz * it->iter;
    }
    return NULL;
}

int
array_fini (array_t *ar)
{
    unsigned i;
    if (ar->fini != NULL) {
        for (i = 0; i < ar->elnum; ++i) {
            ar->fini(ar->data + (i * ar->elsz));
        }
    }
    free(ar->data);
    ar->data = NULL;
    ar->init = NULL;
    ar->fini = NULL;
    ar->compar = NULL;
    ar->elnum = 0;
    return 0;
}

void *
array_first(array_t *ar, array_iter_t *iter)
{
    iter->iter = 0;
    if (iter->iter < ar->elnum) {
        return ar->data;
    }
    return NULL;
}

void *
array_last(array_t *ar, array_iter_t *iter)
{
    iter->iter = ar->elnum - 1;
    if (iter->iter < ar->elnum) {
        return ar->data + iter->iter * ar->elsz;
    }
    return NULL;
}

void *
array_next(array_t *ar, array_iter_t *iter)
{
    if (++iter->iter < ar->elnum) {
        return ar->data + iter->iter * ar->elsz;
    }
    return NULL;
}

void *
array_prev(array_t *ar, array_iter_t *iter)
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
array_incr_iter(array_t *ar, array_iter_t *it)
{
    if (array_ensure_len(ar, ar->elnum + 1, ARRAY_FLAG_SAVE) != 0) {
        TRRETNULL(ARRAY_INCR + 1);
    }
    it->iter = ar->elnum - 1;
    return array_get(ar, ar->elnum - 1);
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
array_sort(array_t *ar)
{
    if (ar->compar == NULL) {
        TRRET(ARRAY_SORT + 1);
    }
    if (ar->elnum == 0) {
        TRRET(ARRAY_SORT + 2);
    }
    qsort(ar->data, ar->elnum, ar->elsz, ar->compar);
    return 0;
}

void *
array_find(array_t *ar, const void *key)
{
    if (ar->compar == NULL) {
        TRRETNULL(ARRAY_FIND + 1);
    }
    if (ar->elnum == 0) {
        TRRETNULL(ARRAY_FIND + 2);
    }
    return bsearch(key, ar->data, ar->elnum, ar->elsz, ar->compar);
}

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

