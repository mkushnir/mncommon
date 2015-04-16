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
           cmh->h,
           cmh->oh);
    for (i = 0; i < cmh->w; ++i) {
        TRACEC(" %d", cmh->v[i]);
    }
    TRACEC("\n");
    //D8(cmh->o, cmh->osz);
}


/*
 * pqueue_t
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
pqueue_init(pqueue_t *pq,
            ssize_t thresh,
            dict_hashfn_t hash,
            dict_item_comparator_t cmp,
            dict_item_finalizer_t fini)
{
    pq->c = 0ul;
    pq->nleft = thresh;
    dict_init(&pq->d, wheel_2x3_find_prime(thresh), hash, cmp, fini);
    pq->fast_pop_thresh = CMMIN;
}


void
pqueue_fini(pqueue_t *pq)
{
    dict_fini(&pq->d);
}


UNUSED static int
pqueue_mingen(pqueue_item_t *it, UNUSED void *value, void *udata)
{
    struct {
        pqueue_t *pq;
        double weight;
        pqueue_item_t *it;
    } *params;
    uint64_t myage;
    double myweight;

    params = udata;
    myage = params->pq->c - it->_gen;
    /* actually weighted age */
    //myweight = 1.0 - (double)myage/(double)params->pq->c;
    myweight = 1.0 / (double)myage;

    /*
     * XXX find a good function of (myweight, myprop)
     */
    myweight *= (double)it->prop;
    //TRACE("%p myweight=%lf", it, myweight);
    /*
     * reduce to the least weighting item
     */
    if (myweight < params->weight) {
        params->weight = myweight;
        params->it = it;
    }
    return 0;
}


UNUSED static int
pqueue_mingen_fast(pqueue_item_t *it, UNUSED void *value, void *udata)
{
    struct {
        pqueue_t *pq;
        double weight;
        pqueue_item_t *it;
    } *params;

    params = udata;
    if (it->cmprop < params->it->cmprop) {
        params->it = it;
    }
    return params->it->cmprop == 1;
}


pqueue_item_t *
pqueue_peek(pqueue_t *pq, pqueue_item_t *it)
{
    pqueue_item_t *res;
    dict_item_t *dit;
    if ((dit = dict_get_item(&pq->d, it)) == NULL) {
        res = NULL;
    } else {
        res = dit->key;
    }
    return res;
}


pqueue_item_t *
pqueue_pop(pqueue_t *pq)
{
    pqueue_item_t *res, it0 = {0, NULL, 0xffffffffffffffff, CMMAX};
    struct {
        pqueue_t *pq;
        double weight;
        pqueue_item_t *it;
    } params;

    res = NULL;
    params.pq = pq;
    params.weight = 1.0;
    params.it = &it0;
    (void)dict_traverse(&pq->d, (dict_traverser_t)pqueue_mingen_fast, &params);
    if (params.it != NULL) {
        //TRACE("removing %p cmprop=%ld", params.it, params.it->cmprop);

        (void)dict_remove_item(&pq->d, params.it);
        res = params.it;
        ++pq->nleft;
        if (pq->nleft <= 0) {
            pq->fast_pop_thresh = res->cmprop;
        } else {
            pq->fast_pop_thresh = CMMIN;
        }
    }

    return res;
}


pqueue_item_t *
pqueue_push(pqueue_t *pq, pqueue_item_t *it)
{
    pqueue_item_t *res;
    dict_item_t *dit;

    it->_gen = pq->c++;

    if (it->cmprop <= pq->fast_pop_thresh) {
        res = it;
    } else {
        if ((dit = dict_get_item(&pq->d, it)) == NULL) {
            dict_set_item(&pq->d, it, NULL);
            --pq->nleft;
        } else {
            /* noop */
        }

        if (pq->nleft < 0) {
            res = pqueue_pop(pq);
        } else {
            res = NULL;
        }
    }


    return res;
}


int
pqueue_traverse(pqueue_t *pq, dict_traverser_t cb, void *udata)
{
    return dict_traverse(&pq->d, cb, udata);
}
