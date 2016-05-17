#include <mrkcommon/dumpm.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/util.h>
#include <mrkcommon/cm.h>

#include "diag.h"


/*
 * cm_t
 */
void
cm_init(cm_t *cm, size_t w, size_t d)
{
    cm->w = w;
    cm->d = d;
    if ((cm->v = calloc(w * d, sizeof(CMTY))) == NULL) {
        FAIL("calloc");
    }
}


void
cm_fini(cm_t *cm)
{
    if (cm->v != NULL) {
        free(cm->v);
        cm->v = NULL;
    }
}


void
cm_add(cm_t *cm, void *key, size_t ksz, CMTY val)
{
    size_t i;

    for (i = 0; i < cm->w; ++i) {
        uint64_t h;
        size_t j;

        h = fasthash(i, (unsigned char *)key, ksz);
        j = (size_t)(h % cm->d);
        //TRACE("Adding %d+%d", cm->v[cm->d * i + j], val);
        cm->v[cm->d * i + j] += val;
    }
}


void
cm_get(cm_t *cm, void *key, size_t ksz, CMTY *val)
{
    size_t i;

    for (i = 0; i < cm->w; ++i) {
        uint64_t h;
        size_t j;
        CMTY v;

        h = fasthash(i, (unsigned char *)key, ksz);
        j = (size_t)(h % cm->d);
        v = cm->v[cm->d * i + j];
        //TRACE("v=%d *val=%d", v, *val);
        *val = MIN(*val, v);
        //if (v < *val) {
        //    *val = v;
        //}
    }
}



/*
 * cmhash_t
 */
cmhash_t *
cmhash_new(size_t w, size_t d)
{
    cmhash_t *h;

    if ((h = malloc(sizeof(cmhash_t))) == NULL) {
        FAIL("malloc");
    }
    h->w = w;
    h->d = d;
    if ((h->v = malloc(w * sizeof(*h->v))) == NULL) {
        FAIL("malloc");
    }

    return h;
}


void
cmhash_destroy(cmhash_t **ph)
{
    if (*ph != NULL) {
        if ((*ph)->v != NULL) {
            free((*ph)->v);
        }
        free(*ph);
        *ph = NULL;
    }
}



void
cmhash_hash(cmhash_t *cmh, void *o, size_t osz)
{
    size_t i;

    cmh->o = o;
    cmh->osz = osz;
    cmh->oh = fasthash(0, (unsigned char *)o, osz);
    for (i = 0; i < cmh->w; ++i) {
        cmh->v[i] = fasthash(i, (unsigned char *)o, osz) % cmh->d;
    }
}


void
cmhash_dump(cmhash_t *cmh)
{
    size_t i;

    TRACEN("%ld/%ld/%016lx/%016lx",
           cmh->w,
           cmh->d,
           (long)cmh->h,
           (long)cmh->oh);
    for (i = 0; i < cmh->w; ++i) {
        TRACEC(" %d", cmh->v[i]);
    }
    TRACEC("\n");
    //D8(cmh->o, cmh->osz);
}


/*
 * pset_t
 */


/*
 * a 2x3 wheel prime finder
 */
static size_t
wheel_2x3_find_prime(size_t n)
{
    int i;
    size_t p;

    for (i = 1, p = 5; p < n; ++i) {
        if (i % 5) {
            p = 6 * i + 5;
        }
        //TRACE("p=%ld", p);
    }
    return p;
}


void
pset_init(pset_t *pset,
            ssize_t thresh,
            hash_hashfn_t hash,
            hash_item_comparator_t cmp,
            hash_item_finalizer_t fini,
            CMTY minthresh)
{
    pset->nleft = thresh;
    hash_init(&pset->d, wheel_2x3_find_prime(thresh), hash, cmp, fini);
    pset->minthresh = minthresh;
    pset->fast_pop_thresh = minthresh;
}


void
pset_fini(pset_t *pset)
{
    hash_fini(&pset->d);
}


static int
pset_mingen_fast(pset_item_t *it, UNUSED void *value, void *udata)
{
    struct {
        pset_t *pset;
        double weight;
        pset_item_t *it;
    } *params;

    params = udata;
    if (it->cmprop < params->it->cmprop) {
        params->it = it;
    }
    return params->it->cmprop == 1;
}


pset_item_t *
pset_peek(pset_t *pset, pset_item_t *it)
{
    pset_item_t *res;
    hash_item_t *dit;
    if ((dit = hash_get_item(&pset->d, it)) == NULL) {
        res = NULL;
    } else {
        res = dit->key;
    }
    return res;
}


pset_item_t *
pset_pop(pset_t *pset)
{
    pset_item_t *res, it0 = {NULL, CMMAX};
    struct {
        pset_t *pset;
        double weight;
        pset_item_t *it;
    } params;

    res = NULL;
    params.pset = pset;
    params.weight = 1.0;
    params.it = &it0;
    (void)hash_traverse(&pset->d, (hash_traverser_t)pset_mingen_fast, &params);
    if (params.it != NULL) {
        //TRACE("removing %p cmprop=%ld", params.it, params.it->cmprop);

        (void)hash_remove_item(&pset->d, params.it);
        res = params.it;
        ++pset->nleft;
        if (pset->nleft <= 0) {
            pset->fast_pop_thresh = MAX(res->cmprop, pset->minthresh);
        } else {
            pset->fast_pop_thresh = pset->minthresh;
        }
    }

    return res;
}


pset_item_t *
pset_push(pset_t *pset, pset_item_t *it)
{
    pset_item_t *res;
    hash_item_t *dit;

    if (it->cmprop <= pset->fast_pop_thresh) {
        res = it;
    } else {
        if ((dit = hash_get_item(&pset->d, it)) == NULL) {
            hash_set_item(&pset->d, it, NULL);
            --pset->nleft;
        } else {
            /* noop */
        }

        if (pset->nleft < 0) {
            res = pset_pop(pset);
        } else {
            res = NULL;
        }
    }


    return res;
}


int
pset_traverse(pset_t *pset, hash_traverser_t cb, void *udata)
{
    return hash_traverse(&pset->d, cb, udata);
}
