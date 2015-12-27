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

static int
null_init(void **v)
{
    *v = NULL;
    return 0;
}

#define HASH_SET_ITEM_BODY(malloc_fn)                          \
    uint64_t idx;                                              \
    hash_item_t **pdit, *dit;                                  \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((pdit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    MEMDEBUG_ENTER(dict);                                      \
    if ((dit = malloc_fn(sizeof(hash_item_t))) == NULL) {      \
        FAIL("malloc_fn");                                     \
    }                                                          \
    MEMDEBUG_LEAVE(dict);                                      \
    dit->bucket = pdit;                                        \
    dit->prev = NULL;                                          \
    if (*pdit == NULL) {                                       \
        dit->next = NULL;                                      \
    } else {                                                   \
        /* insert before the first */                          \
        dit->next = *pdit;                                     \
        (*pdit)->prev = dit;                                   \
        (*pdit)->bucket = NULL;                                \
    }                                                          \
    dit->key = key;                                            \
    dit->value = value;                                        \
    *pdit = dit;                                               \
    ++dict->elnum;                                             \

void
hash_set_item(hash_t *dict, void *key, void *value)
{
    HASH_SET_ITEM_BODY(malloc);
}

void
hash_set_item_mpool(mpool_ctx_t *mpool, hash_t *dict, void *key, void *value)
{
#define _malloc(sz) mpool_malloc(mpool, (sz))
    HASH_SET_ITEM_BODY(_malloc);
#undef _malloc
}

#define HASH_SET_ITEM_UNIQ_BODY(malloc_fn)                     \
    uint64_t idx;                                              \
    assert(oldkey != NULL);                                    \
    assert(oldvalue != NULL);                                  \
    hash_item_t **pdit, *dit;                                  \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((pdit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*pdit == NULL) {                                       \
        MEMDEBUG_ENTER(dict);                                  \
        if ((dit = malloc_fn(sizeof(hash_item_t))) == NULL) {  \
            FAIL("malloc_fn");                                 \
        }                                                      \
        MEMDEBUG_LEAVE(dict);                                  \
        dit->bucket = pdit;                                    \
        dit->prev = NULL;                                      \
        dit->next = NULL;                                      \
        dit->key = key;                                        \
        dit->value = value;                                    \
        *pdit = dit;                                           \
        ++dict->elnum;                                         \
    } else {                                                   \
        for (dit = *pdit; dit != NULL; dit = dit->next) {      \
            if (dict->cmp(key, dit->key) == 0) {               \
                if (dict->fini != NULL) {                      \
                    dict->fini(dit->key, dit->value);          \
                }                                              \
                *oldkey = dit->key;                            \
                dit->key = key;                                \
                *oldvalue = dit->value;                        \
                dit->value = value;                            \
                return;                                        \
            }                                                  \
        }                                                      \
        MEMDEBUG_ENTER(dict);                                  \
        if ((dit = malloc_fn(sizeof(hash_item_t))) == NULL) {  \
            FAIL("malloc_fn");                                 \
        }                                                      \
        MEMDEBUG_LEAVE(dict);                                  \
        dit->next = *pdit;                                     \
        (*pdit)->prev = dit;                                   \
        (*pdit)->bucket = NULL;                                \
        dit->key = key;                                        \
        dit->value = value;                                    \
        *pdit = dit;                                           \
        ++dict->elnum;                                         \
    }                                                          \
    *oldkey = NULL;                                            \
    *oldvalue = NULL;

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
#define _malloc(sz) mpool_malloc(mpool, (sz))
    HASH_SET_ITEM_UNIQ_BODY(_malloc);
#undef _malloc
}

hash_item_t *
hash_get_item(hash_t *dict, void *key)
{
    uint64_t idx;
    hash_item_t **pdit, *dit;

    if (dict->elnum == 0) {
        return NULL;
    }

    idx = dict->hashfn(key) % dict->table.elnum;

    if ((pdit = array_get(&dict->table, idx)) == NULL) {
        FAIL("array_get");
    }

    if (*pdit == NULL) {
        return NULL;
    }

    for (dit = *pdit; dit != NULL; dit = dit->next) {
        if (dict->cmp(key, dit->key) == 0) {
            return dit;
        }
    }
    return NULL;
}


#define HASH_REMOVE_ITEM_BODY(free_fn)                         \
    uint64_t idx;                                              \
    hash_item_t **pdit, *dit;                                  \
    idx = dict->hashfn(key) % dict->table.elnum;               \
    if ((pdit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*pdit == NULL) {                                       \
        return NULL;                                           \
    }                                                          \
    dit = *pdit;                                               \
    if (dict->cmp(key, dit->key) == 0) {                       \
        void *value;                                           \
        if (dit->next != NULL) {                               \
            dit->next->prev = NULL;                            \
        }                                                      \
        *pdit = dit->next;                                     \
        value = dit->value;                                    \
        MEMDEBUG_ENTER(dict);                                  \
        free_fn(dit);                                          \
        MEMDEBUG_LEAVE(dict);                                  \
        --dict->elnum;                                         \
        return value;                                          \
    }                                                          \
    while (dit->next != NULL) {                                \
        dit = dit->next;                                       \
        if (dict->cmp(key, dit->key) == 0) {                   \
            void *value;                                       \
            dit->prev->next = dit->next;                       \
            if (dit->next != NULL) {                           \
                dit->next->prev = dit->prev;                   \
            }                                                  \
            value = dit->value;                                \
            MEMDEBUG_ENTER(dict);                              \
            free_fn(dit);                                      \
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
#define _free(p) mpool_free(mpool, (p))
    HASH_REMOVE_ITEM_BODY(_free);
#undef _free
}


#define HASH_DELETE_PAIR_BODY(free_fn)         \
    if (dit->prev != NULL) {                   \
        dit->prev->next = dit->next;           \
    } else {                                   \
        assert(dit->bucket != NULL);           \
        *(dit->bucket) = dit->next;            \
    }                                          \
    if (dit->next != NULL) {                   \
        dit->next->prev = dit->prev;           \
        if (dit->prev == NULL) {               \
            dit->next->bucket = dit->bucket;   \
        }                                      \
    }                                          \
    if (dict->fini != NULL) {                  \
        dict->fini(dit->key, dit->value);      \
    }                                          \
    MEMDEBUG_ENTER(dict);                      \
    free_fn(dit);                              \
    MEMDEBUG_LEAVE(dict);                      \
    --dict->elnum;                             \



#define HASH_DELETE_PAIR_NO_FINI_BODY(free_fn) \
    if (dit->prev != NULL) {                   \
        dit->prev->next = dit->next;           \
    } else {                                   \
        assert(dit->bucket != NULL);           \
        *(dit->bucket) = dit->next;            \
    }                                          \
    if (dit->next != NULL) {                   \
        dit->next->prev = dit->prev;           \
        if (dit->prev == NULL) {               \
            dit->next->bucket = dit->bucket;   \
        }                                      \
    }                                          \
    MEMDEBUG_ENTER(dict);                      \
    free_fn(dit);                              \
    MEMDEBUG_LEAVE(dict);                      \
    --dict->elnum;                             \



void
hash_delete_pair(hash_t *dict, hash_item_t *dit)
{
    HASH_DELETE_PAIR_BODY(free);
}


void
hash_delete_pair_mpool(mpool_ctx_t *mpool, hash_t *dict, hash_item_t *dit)
{
#define _free(p) mpool_free(mpool, (p))
    HASH_DELETE_PAIR_BODY(_free);
#undef _free
}


void
hash_delete_pair_no_fini_mpool(mpool_ctx_t *mpool, hash_t *dict, hash_item_t *dit)
{
#define _free(p) mpool_free(mpool, (p))
    HASH_DELETE_PAIR_NO_FINI_BODY(_free);
#undef _free
}


int
hash_traverse(hash_t *dict, hash_traverser_t cb, void *udata)
{
    int res;
    hash_item_t **pdit;
    array_iter_t it;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        hash_item_t *dit, *next;

        for (dit = *pdit; dit != NULL; dit = next) {
            next = dit->next;
            if ((res = cb(dit->key, dit->value, udata)) != 0) {
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
    hash_item_t **pdit;
    array_iter_t it;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        hash_item_t *dit, *next;

        for (dit = *pdit; dit != NULL; dit = next) {
            next = dit->next;
            if ((res = cb(dict, dit, udata)) != 0) {
                return res;
            }
        }
    }
    return 0;
}


int
hash_is_empty(hash_t *dict)
{
    UNUSED hash_item_t **pdit;
    UNUSED array_iter_t it;

    return dict->elnum == 0;

    //for (pdit = array_first(&dict->table, &it);
    //     pdit != NULL;
    //     pdit = array_next(&dict->table, &it)) {

    //    if (*pdit != NULL) {
    //        return 0;
    //    }
    //}
    //return 1;
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
#define _array_init(ar, elsz, elnum, init, fini) array_init_mpool(mpool, (ar), (elsz), (elnum), (init), (fini))
    HASH_INIT_BODY(_array_init);
#undef _array_init
}


#define HASH_CLEANUP_BODY(free_fn)                             \
    if (dict->fini != NULL) {                                  \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            hash_item_t **pdit, *dit, *next;                   \
            pdit = ARRAY_GET(hash_item_t *, &dict->table, i);  \
            for (dit = *pdit; dit != NULL; dit = next) {       \
                next = dit->next;                              \
                if (dict->fini(dit->key, dit->value) != 0) {   \
                    break;                                     \
                }                                              \
                MEMDEBUG_ENTER(dict);                          \
                free_fn(dit);                                  \
                MEMDEBUG_LEAVE(dict);                          \
                --dict->elnum;                                 \
            }                                                  \
            *pdit = NULL;                                      \
        }                                                      \
    } else {                                                   \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            hash_item_t **pdit, *dit, *next;                   \
            pdit = ARRAY_GET(hash_item_t *, &dict->table, i);  \
            for (dit = *pdit; dit != NULL; dit = next) {       \
                next = dit->next;                              \
                MEMDEBUG_ENTER(dict);                          \
                free_fn(dit);                                  \
                MEMDEBUG_LEAVE(dict);                          \
                --dict->elnum;                                 \
            }                                                  \
            *pdit = NULL;                                      \
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
#define _array_init(ar, elsz, elnum, init, fini) array_init_mpool(mpool, (ar), (elsz), (elnum), (init), (fini))
    MEMDEBUG_INIT(dict);
    MEMDEBUG_ENTER(dict);
    HASH_INIT_BODY(_array_init);
#undef _array_init
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
#define _free(p) mpool_free(mpool, (p))
    HASH_CLEANUP_BODY(_free);
#undef _free
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
    hash_item_t **pdit;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        hash_item_t *dit;
        size_t sz;

        for (dit = *pdit, sz = 0; dit != NULL; dit = dit->next, ++sz) {
        }
        TRACE("bucket %d sz %ld", it.iter, sz);
    }
}

