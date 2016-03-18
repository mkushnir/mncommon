#include <assert.h>
#include <stdlib.h>

#include <mrkcommon/array.h>
#include <mrkcommon/hash.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>
#include "diag.h"

#ifdef DO_MEMDEBUG
#include <mrkcommon/memdebug.h>
MEMDEBUG_DECLARE(dict);

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
#define _free(p) mpool_free(mpool, (p))
#define _array_init(ar, elsz, elnum, init, fini) array_init_mpool(mpool, (ar), (elsz), (elnum), (init), (fini))
#define _array_reset_no_fini(ar, newelnum) array_reset_no_fini_mpool(mpool, (ar), (newelnum))

static int
my_rehash(UNUSED hash_t *dict, hash_item_t *it, void *udata)
{
    struct {
        hash_item_t **tmp;
        size_t n;
    } *params = udata;
    params->tmp[params->n++] = it;
    return 0;
}



#define HASH_REHASH_BODY(malloc_fn, free_fn, array_reset_no_fini_fn)   \
    unsigned i;                                                        \
    struct {                                                           \
        hash_item_t **tmp;                                             \
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
        hash_item_t *hit, **phit;                                      \
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
hash_rehash(hash_t *dict, size_t sz)
{
    HASH_REHASH_BODY(malloc, free, array_reset_no_fini)
}


void
hash_rehash_mpool(mpool_ctx_t *mpool, hash_t *dict, size_t sz)
{
    HASH_REHASH_BODY(_malloc, _free, _array_reset_no_fini)
}


static int
null_init(void **v)
{
    *v = NULL;
    return 0;
}


#define HASH_SET_ITEM_BODY(malloc_fn)                          \
    uint64_t idx;                                              \
    hash_item_t **phit, *hit;                                  \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((phit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    MEMDEBUG_ENTER(dict);                                      \
    if ((hit = malloc_fn(sizeof(hash_item_t))) == NULL) {      \
        FAIL("malloc_fn");                                     \
    }                                                          \
    MEMDEBUG_LEAVE(dict);                                      \
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
hash_set_item(hash_t *dict, void *key, void *value)
{
    HASH_SET_ITEM_BODY(malloc);
}


void
hash_set_item_mpool(mpool_ctx_t *mpool, hash_t *dict, void *key, void *value)
{
    HASH_SET_ITEM_BODY(_malloc);
}


#define HASH_SET_ITEM_UNIQ_BODY(malloc_fn)                     \
    uint64_t idx;                                              \
    assert(oldkey != NULL);                                    \
    assert(oldvalue != NULL);                                  \
    hash_item_t **phit, *hit;                                  \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((phit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*phit == NULL) {                                       \
        MEMDEBUG_ENTER(dict);                                  \
        if ((hit = malloc_fn(sizeof(hash_item_t))) == NULL) {  \
            FAIL("malloc_fn");                                 \
        }                                                      \
        MEMDEBUG_LEAVE(dict);                                  \
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
        MEMDEBUG_ENTER(dict);                                  \
        if ((hit = malloc_fn(sizeof(hash_item_t))) == NULL) {  \
            FAIL("malloc_fn");                                 \
        }                                                      \
        MEMDEBUG_LEAVE(dict);                                  \
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
hash_set_item_uniq(hash_t *dict,
                   void *key,
                   void *value,
                   void **oldkey,
                   void **oldvalue)
{
    HASH_SET_ITEM_UNIQ_BODY(malloc);
}


void
hash_set_item_uniq_mpool(mpool_ctx_t *mpool,
                         hash_t *dict,
                         void *key,
                         void *value,
                         void **oldkey,
                         void **oldvalue)
{
    HASH_SET_ITEM_UNIQ_BODY(_malloc);
}


hash_item_t *
hash_get_item(hash_t *dict, void *key)
{
    uint64_t idx;
    hash_item_t **phit, *hit;

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
    hash_item_t **phit, *hit;                                  \
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
        MEMDEBUG_ENTER(dict);                                  \
        free_fn(hit);                                          \
        MEMDEBUG_LEAVE(dict);                                  \
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
            MEMDEBUG_ENTER(dict);                              \
            free_fn(hit);                                      \
            MEMDEBUG_LEAVE(dict);                              \
            --dict->elnum;                                     \
            return value;                                      \
        }                                                      \
    }                                                          \
    return NULL                                                \


void *
hash_remove_item(hash_t *dict, void *key)
{
    HASH_REMOVE_ITEM_BODY(free);
}


void *
hash_remove_item_mpool(mpool_ctx_t *mpool, hash_t *dict, void *key)
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
    MEMDEBUG_ENTER(dict);                      \
    free_fn(hit);                              \
    MEMDEBUG_LEAVE(dict);                      \
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
    MEMDEBUG_ENTER(dict);                      \
    free_fn(hit);                              \
    MEMDEBUG_LEAVE(dict);                      \
    --dict->elnum;                             \



void
hash_delete_pair(hash_t *dict, hash_item_t *hit)
{
    HASH_DELETE_PAIR_BODY(free);
}


void
hash_delete_pair_mpool(mpool_ctx_t *mpool, hash_t *dict, hash_item_t *hit)
{
    HASH_DELETE_PAIR_BODY(_free);
}


void
hash_delete_pair_no_fini_mpool(mpool_ctx_t *mpool, hash_t *dict, hash_item_t *hit)
{
    HASH_DELETE_PAIR_NO_FINI_BODY(_free);
}


int
hash_traverse(hash_t *dict, hash_traverser_t cb, void *udata)
{
    int res;
    hash_item_t **phit;
    array_iter_t it;

    for (phit = array_first(&dict->table, &it);
         phit != NULL;
         phit = array_next(&dict->table, &it)) {

        hash_item_t *hit, *next;

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
hash_traverse_item(hash_t *dict, hash_traverser_item_t cb, void *udata)
{
    int res;
    hash_item_t **phit;
    array_iter_t it;

    for (phit = array_first(&dict->table, &it);
         phit != NULL;
         phit = array_next(&dict->table, &it)) {

        hash_item_t *hit, *next;

        for (hit = *phit; hit != NULL; hit = next) {
            next = hit->next;
            if ((res = cb(dict, hit, udata)) != 0) {
                return res;
            }
        }
    }
    return 0;
}


bool
hash_is_empty(hash_t *dict)
{
    return dict->elnum == 0;
}

size_t
hash_get_elnum(hash_t *dict)
{
    return dict->elnum;
}


#define HASH_INIT_BODY(array_init_fn)                          \
    assert(hashfn != NULL);                                    \
    dict->hashfn = hashfn;                                     \
    assert(cmp != NULL);                                       \
    dict->cmp = cmp;                                           \
    dict->fini = fini;                                         \
    array_init_fn(&dict->table, sizeof(hash_item_t *), sz,     \
               (array_initializer_t)null_init,                 \
               NULL);                                          \
    dict->elnum = 0;                                           \


void
hash_init(hash_t *dict,
          size_t sz,
          hash_hashfn_t hashfn,
          hash_item_comparator_t cmp,
          hash_item_finalizer_t fini)
{
    HASH_INIT_BODY(array_init);
}


void
hash_init_mpool(mpool_ctx_t *mpool,
                hash_t *dict,
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
            hash_item_t **phit, *hit, *next;                   \
            phit = ARRAY_GET(hash_item_t *, &dict->table, i);  \
            for (hit = *phit; hit != NULL; hit = next) {       \
                next = hit->next;                              \
                if (dict->fini(hit->key, hit->value) != 0) {   \
                    break;                                     \
                }                                              \
                MEMDEBUG_ENTER(dict);                          \
                free_fn(hit);                                  \
                MEMDEBUG_LEAVE(dict);                          \
                --dict->elnum;                                 \
            }                                                  \
            *phit = NULL;                                      \
        }                                                      \
    } else {                                                   \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            hash_item_t **phit, *hit, *next;                   \
            phit = ARRAY_GET(hash_item_t *, &dict->table, i);  \
            for (hit = *phit; hit != NULL; hit = next) {       \
                next = hit->next;                              \
                MEMDEBUG_ENTER(dict);                          \
                free_fn(hit);                                  \
                MEMDEBUG_LEAVE(dict);                          \
                --dict->elnum;                                 \
            }                                                  \
            *phit = NULL;                                      \
        }                                                      \
    }                                                          \



hash_t *
hash_new(size_t sz,
         hash_hashfn_t hashfn,
         hash_item_comparator_t cmp,
         hash_item_finalizer_t fini)
{
    hash_t *dict;

    if ((dict = malloc(sizeof(hash_t))) == NULL) {
        FAIL("malloc");
    }
    MEMDEBUG_INIT(dict);
    MEMDEBUG_ENTER(dict);
    HASH_INIT_BODY(array_init);
    MEMDEBUG_LEAVE(dict);

    return dict;
}


hash_t *
hash_new_mpool(mpool_ctx_t *mpool,
               size_t sz,
               hash_hashfn_t hashfn,
               hash_item_comparator_t cmp,
               hash_item_finalizer_t fini)
{
    hash_t *dict;

    if ((dict = mpool_malloc(mpool, sizeof(hash_t))) == NULL) {
        FAIL("malloc");
    }
    MEMDEBUG_INIT(dict);
    MEMDEBUG_ENTER(dict);
    HASH_INIT_BODY(_array_init);
    MEMDEBUG_LEAVE(dict);

    return dict;
}


void
hash_cleanup(hash_t *dict)
{
    HASH_CLEANUP_BODY(free);
}


void
hash_cleanup_mpool(mpool_ctx_t *mpool, hash_t *dict)
{
    HASH_CLEANUP_BODY(_free);
}


void
hash_fini(hash_t *dict)
{
    hash_cleanup(dict);
    array_fini(&dict->table);
}


void
hash_fini_mpool(mpool_ctx_t *mpool, hash_t *dict)
{
    hash_cleanup_mpool(mpool, dict);
    array_fini_mpool(mpool, &dict->table);
}


void
hash_dump_stats(hash_t *dict)
{
    array_iter_t it;
    hash_item_t **phit;

    for (phit = array_first(&dict->table, &it);
         phit != NULL;
         phit = array_next(&dict->table, &it)) {

        hash_item_t *hit;
        size_t sz;

        for (hit = *phit, sz = 0; hit != NULL; hit = hit->next, ++sz) {
        }
        TRACE("bucket %d sz %ld", it.iter, sz);
    }
}

