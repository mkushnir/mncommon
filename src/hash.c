#include <assert.h>
#include <stdlib.h>

#include <mncommon/malloc.h>
#include <mncommon/array.h>
#include <mncommon/hash.h>
#include <mncommon/dumpm.h>
#include <mncommon/mpool.h>
#include <mncommon/util.h>
#include "diag.h"


#define _malloc(sz) mpool_malloc(mpool, (sz))
#define _free(p) mpool_free(mpool, (p))
#define _array_init(ar, elsz, elnum, init, fini) array_init_mpool(mpool, (ar), (elsz), (elnum), (init), (fini))
#define _array_reset_no_fini(ar, newelnum) array_reset_no_fini_mpool(mpool, (ar), (newelnum))

static int
my_rehash(UNUSED mnhash_t *dict, mnhash_item_t *it, void *udata)
{
    struct {
        mnhash_item_t **tmp;
        size_t n;
    } *params = udata;
    params->tmp[params->n++] = it;
    return 0;
}



#define HASH_REHASH_BODY(malloc_fn, free_fn, array_reset_no_fini_fn)   \
    unsigned i;                                                        \
    struct {                                                           \
        mnhash_item_t **tmp;                                           \
        size_t n;                                                      \
    } params;                                                          \
    if ((params.tmp = malloc_fn(                                       \
                    sizeof(*(params.tmp)) * dict->elnum)) == NULL) {   \
        FAIL("malloc");                                                \
    }                                                                  \
    params.n = 0;                                                      \
    if (hash_traverse_item(dict, my_rehash, &params) != 0) {           \
        FAIL("hash_traverse_item");                                    \
    }                                                                  \
    if (array_reset_no_fini_fn(&dict->table, sz) != 0) {               \
        FAIL("array_reset_no_fini");                                   \
    }                                                                  \
    assert(params.n == dict->elnum);                                   \
    for (i = 0; i < dict->elnum; ++i) {                                \
        uint64_t idx;                                                  \
        mnhash_item_t *hit, **phit;                                    \
        hit = params.tmp[i];                                           \
        idx = dict->hashfn(hit->key) % sz;                             \
        if ((phit = array_get(&dict->table, idx)) == NULL) {           \
            FAIL("array_get");                                         \
        }                                                              \
        hit->bucket = phit;                                            \
        hit->prev = NULL;                                              \
        if (*phit == NULL) {                                           \
            hit->next = NULL;                                          \
        } else {                                                       \
            /* insert before the first */                              \
            hit->next = *phit;                                         \
            (*phit)->prev = hit;                                       \
            (*phit)->bucket = NULL;                                    \
        }                                                              \
        *phit = hit;                                                   \
    }                                                                  \
    if (params.tmp != NULL) {                                          \
        free_fn(params.tmp);                                           \
        params.tmp = NULL;                                             \
    }                                                                  \


void
hash_rehash(mnhash_t *dict, size_t sz)
{
    HASH_REHASH_BODY(malloc, free, array_reset_no_fini)
}


void
hash_rehash_mpool(mpool_ctx_t *mpool, mnhash_t *dict, size_t sz)
{
    HASH_REHASH_BODY(_malloc, _free, _array_reset_no_fini)
}


static int
null_init(void *v)
{
    void **pv = (void **)v;
    *pv = NULL;
    return 0;
}


#define HASH_SET_ITEM_BODY(malloc_fn)                          \
    uint64_t idx;                                              \
    mnhash_item_t **phit, *hit;                                \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((phit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if ((hit = malloc_fn(sizeof(mnhash_item_t))) == NULL) {    \
        FAIL("malloc_fn");                                     \
    }                                                          \
    hit->bucket = phit;                                        \
    hit->prev = NULL;                                          \
    if (*phit == NULL) {                                       \
        hit->next = NULL;                                      \
    } else {                                                   \
        /* insert before the first */                          \
        hit->next = *phit;                                     \
        (*phit)->prev = hit;                                   \
        (*phit)->bucket = NULL;                                \
    }                                                          \
    hit->key = key;                                            \
    hit->value = value;                                        \
    *phit = hit;                                               \
    ++dict->elnum;                                             \


void
hash_set_item(mnhash_t *dict, void *key, void *value)
{
    HASH_SET_ITEM_BODY(malloc);
}


void
hash_set_item_mpool(mpool_ctx_t *mpool,
                    mnhash_t *dict,
                    void *key,
                    void *value)
{
    HASH_SET_ITEM_BODY(_malloc);
}


#define HASH_SET_ITEM_UNIQ_BODY(malloc_fn)                     \
    uint64_t idx;                                              \
    assert(oldkey != NULL);                                    \
    assert(oldvalue != NULL);                                  \
    mnhash_item_t **phit, *hit;                                \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((phit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*phit == NULL) {                                       \
        if ((hit = malloc_fn(sizeof(mnhash_item_t))) == NULL) {\
            FAIL("malloc_fn");                                 \
        }                                                      \
        hit->bucket = phit;                                    \
        hit->prev = NULL;                                      \
        hit->next = NULL;                                      \
        hit->key = key;                                        \
        hit->value = value;                                    \
        *phit = hit;                                           \
        ++dict->elnum;                                         \
    } else {                                                   \
        for (hit = *phit; hit != NULL; hit = hit->next) {      \
            if (dict->cmp(key, hit->key) == 0) {               \
                if (dict->fini != NULL) {                      \
                    dict->fini(hit->key, hit->value);          \
                }                                              \
                *oldkey = hit->key;                            \
                hit->key = key;                                \
                *oldvalue = hit->value;                        \
                hit->value = value;                            \
                return;                                        \
            }                                                  \
        }                                                      \
        if ((hit = malloc_fn(sizeof(mnhash_item_t))) == NULL) {\
            FAIL("malloc_fn");                                 \
        }                                                      \
        hit->next = *phit;                                     \
        (*phit)->prev = hit;                                   \
        (*phit)->bucket = NULL;                                \
        hit->key = key;                                        \
        hit->value = value;                                    \
        *phit = hit;                                           \
        ++dict->elnum;                                         \
    }                                                          \
    *oldkey = NULL;                                            \
    *oldvalue = NULL;                                          \


void
hash_set_item_uniq(mnhash_t *dict,
                   void *key,
                   void *value,
                   void **oldkey,
                   void **oldvalue)
{
    HASH_SET_ITEM_UNIQ_BODY(malloc);
}


void
hash_set_item_uniq_mpool(mpool_ctx_t *mpool,
                         mnhash_t *dict,
                         void *key,
                         void *value,
                         void **oldkey,
                         void **oldvalue)
{
    HASH_SET_ITEM_UNIQ_BODY(_malloc);
}


mnhash_item_t *
hash_get_item(mnhash_t *dict, const void *key)
{
    uint64_t idx;
    mnhash_item_t **phit, *hit;

    if (dict->elnum == 0) {
        return NULL;
    }

    idx = dict->hashfn(key) % dict->table.elnum;

    if ((phit = array_get(&dict->table, idx)) == NULL) {
        FAIL("array_get");
    }

    if (*phit == NULL) {
        return NULL;
    }

    for (hit = *phit; hit != NULL; hit = hit->next) {
        if (dict->cmp(key, hit->key) == 0) {
            return hit;
        }
    }
    return NULL;
}


#define HASH_REMOVE_ITEM_BODY(free_fn)                         \
    uint64_t idx;                                              \
    mnhash_item_t **phit, *hit;                                \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((phit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*phit == NULL) {                                       \
        return NULL;                                           \
    }                                                          \
    hit = *phit;                                               \
    if (dict->cmp(key, hit->key) == 0) {                       \
        void *value;                                           \
        if (hit->next != NULL) {                               \
            hit->next->prev = NULL;                            \
        }                                                      \
        *phit = hit->next;                                     \
        value = hit->value;                                    \
        free_fn(hit);                                          \
        --dict->elnum;                                         \
        return value;                                          \
    }                                                          \
    while (hit->next != NULL) {                                \
        hit = hit->next;                                       \
        if (dict->cmp(key, hit->key) == 0) {                   \
            void *value;                                       \
            hit->prev->next = hit->next;                       \
            if (hit->next != NULL) {                           \
                hit->next->prev = hit->prev;                   \
            }                                                  \
            value = hit->value;                                \
            free_fn(hit);                                      \
            --dict->elnum;                                     \
            return value;                                      \
        }                                                      \
    }                                                          \
    return NULL                                                \


void *
hash_remove_item(mnhash_t *dict, const void *key)
{
    HASH_REMOVE_ITEM_BODY(free);
}


void *
hash_remove_item_mpool(mpool_ctx_t *mpool, mnhash_t *dict, const void *key)
{
    HASH_REMOVE_ITEM_BODY(_free);
}


#define HASH_DELETE_PAIR_BODY(free_fn)         \
    if (hit->prev != NULL) {                   \
        hit->prev->next = hit->next;           \
    } else {                                   \
        assert(hit->bucket != NULL);           \
        *(hit->bucket) = hit->next;            \
    }                                          \
    if (hit->next != NULL) {                   \
        hit->next->prev = hit->prev;           \
        if (hit->prev == NULL) {               \
            hit->next->bucket = hit->bucket;   \
        }                                      \
    }                                          \
    if (dict->fini != NULL) {                  \
        dict->fini(hit->key, hit->value);      \
    }                                          \
    free_fn(hit);                              \
    --dict->elnum;                             \



#define HASH_DELETE_PAIR_NO_FINI_BODY(free_fn) \
    if (hit->prev != NULL) {                   \
        hit->prev->next = hit->next;           \
    } else {                                   \
        assert(hit->bucket != NULL);           \
        *(hit->bucket) = hit->next;            \
    }                                          \
    if (hit->next != NULL) {                   \
        hit->next->prev = hit->prev;           \
        if (hit->prev == NULL) {               \
            hit->next->bucket = hit->bucket;   \
        }                                      \
    }                                          \
    free_fn(hit);                              \
    --dict->elnum;                             \



void
hash_delete_pair(mnhash_t *dict, mnhash_item_t *hit)
{
    HASH_DELETE_PAIR_BODY(free);
}


void
hash_delete_pair_no_fini(mnhash_t *dict, mnhash_item_t *hit)
{
    HASH_DELETE_PAIR_NO_FINI_BODY(free);
}


void
hash_delete_pair_mpool(mpool_ctx_t *mpool, mnhash_t *dict, mnhash_item_t *hit)
{
    HASH_DELETE_PAIR_BODY(_free);
}


void
hash_delete_pair_no_fini_mpool(mpool_ctx_t *mpool, mnhash_t *dict, mnhash_item_t *hit)
{
    HASH_DELETE_PAIR_NO_FINI_BODY(_free);
}


int
hash_traverse(mnhash_t *dict, hash_traverser_t cb, void *udata)
{
    int res;
    mnhash_item_t **phit;
    mnarray_iter_t it;

    for (phit = array_first(&dict->table, &it);
         phit != NULL;
         phit = array_next(&dict->table, &it)) {

        mnhash_item_t *hit, *next;

        for (hit = *phit; hit != NULL; hit = next) {
            next = hit->next;
            if ((res = cb(hit->key, hit->value, udata)) != 0) {
                return res;
            }
        }
    }
    return 0;
}


int
hash_traverse_item(mnhash_t *dict, hash_traverser_item_t cb, void *udata)
{
    int res;
    mnhash_item_t **phit;
    mnarray_iter_t it;

    for (phit = array_first(&dict->table, &it);
         phit != NULL;
         phit = array_next(&dict->table, &it)) {

        mnhash_item_t *hit, *next;

        for (hit = *phit; hit != NULL; hit = next) {
            next = hit->next;
            if ((res = cb(dict, hit, udata)) != 0) {
                return res;
            }
        }
    }
    return 0;
}


mnhash_item_t *
hash_first(mnhash_t *hash, mnhash_iter_t *it)
{
    it->hit = NULL;
    for (it->phit = array_first(&hash->table, &it->it);
         it->phit != NULL;
         it->phit = array_next(&hash->table, &it->it)) {
        if (*it->phit != NULL) {
            it->hit = *it->phit;
            break;
        }
    }
    return it->hit;
}


mnhash_item_t *
hash_next(mnhash_t *hash, mnhash_iter_t *it)
{
    if (it->hit != NULL) {
        it->hit = it->hit->next;
    }
    if (it->hit == NULL) {
        for (it->phit = array_next(&hash->table, &it->it);
             it->phit != NULL;
             it->phit = array_next(&hash->table, &it->it)) {
            if (*it->phit != NULL) {
                it->hit = *it->phit;
                break;
            }
        }
    }
    return it->hit;
}


bool
hash_is_empty(mnhash_t *dict)
{
    return dict->elnum == 0;
}

size_t
hash_get_elnum(mnhash_t *dict)
{
    return dict->elnum;
}


#define HASH_INIT_BODY(array_init_fn)                          \
    assert(hashfn != NULL);                                    \
    dict->hashfn = hashfn;                                     \
    assert(cmp != NULL);                                       \
    dict->cmp = cmp;                                           \
    dict->fini = fini;                                         \
    array_init_fn(&dict->table, sizeof(mnhash_item_t *), sz,   \
               (array_initializer_t)null_init,                 \
               NULL);                                          \
    dict->elnum = 0;                                           \


void
hash_init(mnhash_t *dict,
          size_t sz,
          hash_hashfn_t hashfn,
          hash_item_comparator_t cmp,
          hash_item_finalizer_t fini)
{
    HASH_INIT_BODY(array_init);
}


void
hash_init_mpool(mpool_ctx_t *mpool,
                mnhash_t *dict,
                size_t sz,
                hash_hashfn_t hashfn,
                hash_item_comparator_t cmp,
                hash_item_finalizer_t fini)
{
    HASH_INIT_BODY(_array_init);
}


#define HASH_CLEANUP_BODY(free_fn)                             \
    if (dict->fini != NULL) {                                  \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            mnhash_item_t **phit, *hit, *next;                 \
            phit = ARRAY_GET(mnhash_item_t *, &dict->table, i);\
            for (hit = *phit; hit != NULL; hit = next) {       \
                next = hit->next;                              \
                if (dict->fini(hit->key, hit->value) != 0) {   \
                    break;                                     \
                }                                              \
                free_fn(hit);                                  \
                --dict->elnum;                                 \
            }                                                  \
            *phit = NULL;                                      \
        }                                                      \
    } else {                                                   \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            mnhash_item_t **phit, *hit, *next;                 \
            phit = ARRAY_GET(mnhash_item_t *, &dict->table, i);\
            for (hit = *phit; hit != NULL; hit = next) {       \
                next = hit->next;                              \
                free_fn(hit);                                  \
                --dict->elnum;                                 \
            }                                                  \
            *phit = NULL;                                      \
        }                                                      \
    }                                                          \



mnhash_t *
hash_new(size_t sz,
         hash_hashfn_t hashfn,
         hash_item_comparator_t cmp,
         hash_item_finalizer_t fini)
{
    mnhash_t *dict;

    if ((dict = malloc(sizeof(mnhash_t))) == NULL) {
        FAIL("malloc");
    }
    HASH_INIT_BODY(array_init);

    return dict;
}


mnhash_t *
hash_new_mpool(mpool_ctx_t *mpool,
               size_t sz,
               hash_hashfn_t hashfn,
               hash_item_comparator_t cmp,
               hash_item_finalizer_t fini)
{
    mnhash_t *dict;

    if ((dict = mpool_malloc(mpool, sizeof(mnhash_t))) == NULL) {
        FAIL("malloc");
    }
    HASH_INIT_BODY(_array_init);

    return dict;
}


void
hash_cleanup(mnhash_t *dict)
{
    HASH_CLEANUP_BODY(free);
}


void
hash_cleanup_mpool(mpool_ctx_t *mpool, mnhash_t *dict)
{
    HASH_CLEANUP_BODY(_free);
}


void
hash_fini(mnhash_t *dict)
{
    hash_cleanup(dict);
    array_fini(&dict->table);
}


void
hash_fini_mpool(mpool_ctx_t *mpool, mnhash_t *dict)
{
    hash_cleanup_mpool(mpool, dict);
    array_fini_mpool(mpool, &dict->table);
}


void
hash_destroy(mnhash_t **hash)
{
    if (*hash != NULL) {
        hash_fini(*hash);
        free(*hash);
        *hash = NULL;
    }
}


void
hash_destroy_mpool(mpool_ctx_t *mpool, mnhash_t **hash)
{
    if (*hash != NULL) {
        hash_fini_mpool(mpool, *hash);
        mpool_free(mpool, *hash);
        *hash = NULL;
    }
}


void
hash_dump_stats(mnhash_t *dict)
{
    mnarray_iter_t it;
    mnhash_item_t **phit;

    for (phit = array_first(&dict->table, &it);
         phit != NULL;
         phit = array_next(&dict->table, &it)) {

        mnhash_item_t *hit;
        size_t sz;

        for (hit = *phit, sz = 0; hit != NULL; hit = hit->next, ++sz) {
        }
        TRACE("bucket %d sz %ld", it.iter, sz);
    }
}


/*
 * set
 */
void
hash_set_add(mnhash_t *hash, void *key)
{
    if (hash_get_item(hash, key) == NULL) {
        hash_set_item(hash, key, NULL);
    }
}

void
hash_set_remove(mnhash_t *hash, const void *key)
{
    (void)hash_remove_item(hash, key);
}


static int
hash_set_union2_cb(UNUSED mnhash_t *b, mnhash_item_t *hit, void *udata)
{
    mnhash_t *a = udata;
    if (hash_get_item(a, hit->key) == NULL) {
        hash_set_item(a, hit->key, hit->value);
    }
    return 0;
}


void
hash_set_union2(mnhash_t *a, mnhash_t *b)
{
    (void)hash_traverse_item(b, hash_set_union2_cb, a);
}


static int
hash_set_union3_cb(UNUSED mnhash_t *a, mnhash_item_t *hit, void *udata)
{
    mnhash_t *res = udata;

    if (hash_get_item(res, hit->key) == NULL) {
        hash_set_item(res, hit->key, hit->value);
    }
    return 0;
}


void
hash_set_union3(mnhash_t *res, mnhash_t *a, mnhash_t *b)
{
    (void)hash_traverse_item(a,
                             hash_set_union3_cb,
                             res);
    (void)hash_traverse_item(b,
                             hash_set_union3_cb,
                             res);
}


static int
hash_set_diff3_cb(UNUSED mnhash_t *a, mnhash_item_t *hit, void *udata)
{
    struct {
        mnhash_t *res;
        mnhash_t *b;
    } *params = udata;
    if (hash_get_item(params->b, hit->key) == NULL) {
        hash_set_item(params->res, hit->key, hit->value);
    }
    return 0;
}


void
hash_set_diff3(mnhash_t *res, mnhash_t *a, mnhash_t *b)
{
    struct {
        mnhash_t *res;
        mnhash_t *b;
    } params = { res, b };
    (void)hash_traverse_item(a,
                             hash_set_diff3_cb,
                             &params);
}


static int
hash_set_sdiff3_cb(UNUSED mnhash_t *b, mnhash_item_t *hit, void *udata)
{
    struct {
        mnhash_t *res;
        mnhash_t *a;
    } *params = udata;
    if (hash_get_item(params->a, hit->key) == NULL) {
        void *oldkey;
        void *oldvalue;

        hash_set_item_uniq(params->res, hit->key, hit->value, &oldkey, &oldvalue);
    }
    return 0;
}


void
hash_set_sdiff3(mnhash_t *res, mnhash_t *a, mnhash_t *b)
{
    struct {
        mnhash_t *res;
        mnhash_t *b;
    } params0 = { res, b };
    (void)hash_traverse_item(a,
                             hash_set_diff3_cb,
                             &params0);
    struct {
        mnhash_t *res;
        mnhash_t *a;
    } params1 = { res, a };
    (void)hash_traverse_item(b,
                             hash_set_sdiff3_cb,
                             &params1);
}


static int
hash_set_intersect3_cb(UNUSED mnhash_t *a, mnhash_item_t *hit, void *udata)
{
    struct {
        mnhash_t *res;
        mnhash_t *b;
    } *params = udata;
    if (hash_get_item(params->b, hit->key) != NULL) {
        hash_set_item(params->res, hit->key, hit->value);
    }
    return 0;
}


void
hash_set_intersect3(mnhash_t *res, mnhash_t *a, mnhash_t *b)
{
    struct {
        mnhash_t *res;
        mnhash_t *b;
    } params = { res, b };
    (void)hash_traverse_item(a,
                             hash_set_intersect3_cb,
                             &params);
}


size_t
hash_next_prime(size_t n)
{
    /*
     * Useful primes
     */
    static size_t primes[] = {
        7,
        11,
        17,
        31,
        61,
        127,

        251,
        257,

        509,
        521,

        1021,
        1031,

        2039,
        2053,

        4093,
        4099,

        8191,
        8209,

        16381,
        16411,

        32749,
        32771,

        65213, // centered heptagonal prime
        65537, // Fermat prime
        68111, // chp
        72073, // chp
        76147, // chp
        84631, // chp
        89041, // chp
        93563, // chp
        193939, // circular prime
        1046527, // carol prime
        27644437,  // bell number prime
    };
    size_t i;

    for (i = 0; i < countof(primes); ++i) {
        if (n <= primes[i]) {
            return primes[i];
        }
    }
    return n;
}
