#include <assert.h>
#include <stdlib.h>

#include <mrkcommon/array.h>
#include <mrkcommon/dict.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>
#include "diag.h"

//#ifndef NDEBUG
//#include "mrkcommon/memdebug.h"
//MEMDEBUG_DECLARE(dict);
//#endif

static int
null_init(void **v)
{
    *v = NULL;
    return 0;
}

#define DICT_SET_ITEM_BODY(malloc_fn)                          \
    uint64_t idx;                                              \
    dict_item_t **pdit, *dit;                                  \
    idx = dict->hashfn(key) % dict->sz;                        \
    if ((pdit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if ((dit = malloc_fn(sizeof(dict_item_t))) == NULL) {      \
        FAIL("malloc_fn");                                     \
    }                                                          \
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
    *pdit = dit;

void
dict_set_item(dict_t *dict, void *key, void *value)
{
    DICT_SET_ITEM_BODY(malloc);
}

void
dict_set_item_mpool(mpool_ctx_t *mpool, dict_t *dict, void *key, void *value)
{
#define _malloc(sz) mpool_malloc(mpool, (sz))
    DICT_SET_ITEM_BODY(_malloc);
#undef _malloc
}

#define DICT_SET_ITEM_UNIQ_BODY(malloc_fn)                     \
    uint64_t idx;                                              \
    assert(oldkey != NULL);                                    \
    assert(oldvalue != NULL);                                  \
    dict_item_t **pdit, *dit;                                  \
    idx = dict->hashfn(key) % dict->sz;                        \
    if ((pdit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*pdit == NULL) {                                       \
        if ((dit = malloc_fn(sizeof(dict_item_t))) == NULL) {  \
            FAIL("malloc_fn");                                 \
        }                                                      \
        dit->bucket = pdit;                                    \
        dit->prev = NULL;                                      \
        dit->next = NULL;                                      \
        dit->key = key;                                        \
        dit->value = value;                                    \
        *pdit = dit;                                           \
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
        if ((dit = malloc_fn(sizeof(dict_item_t))) == NULL) {  \
            FAIL("malloc_fn");                                 \
        }                                                      \
        dit->next = *pdit;                                     \
        (*pdit)->prev = dit;                                   \
        (*pdit)->bucket = NULL;                                \
        dit->key = key;                                        \
        dit->value = value;                                    \
        *pdit = dit;                                           \
    }                                                          \
    *oldkey = NULL;                                            \
    *oldvalue = NULL;

void
dict_set_item_uniq(dict_t *dict,
                   void *key,
                   void *value,
                   void **oldkey,
                   void **oldvalue)
{
    DICT_SET_ITEM_UNIQ_BODY(malloc);
}

void
dict_set_item_uniq_mpool(mpool_ctx_t *mpool,
                         dict_t *dict,
                         void *key,
                         void *value,
                         void **oldkey,
                         void **oldvalue)
{
#define _malloc(sz) mpool_malloc(mpool, (sz))
    DICT_SET_ITEM_UNIQ_BODY(_malloc);
#undef _malloc
}

dict_item_t *
dict_get_item(dict_t *dict, void *key)
{
    uint64_t idx;
    dict_item_t **pdit, *dit;

    idx = dict->hashfn(key) % dict->sz;

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


#define DICT_REMOVE_ITEM_BODY(free_fn)                         \
    uint64_t idx;                                              \
    dict_item_t **pdit, *dit;                                  \
    idx = dict->hashfn(key) % dict->sz;                        \
    if ((pdit = array_get(&dict->table, idx)) == NULL) {       \
        FAIL("array_get");                                     \
    }                                                          \
    if (*pdit == NULL) {                                       \
        return NULL;                                           \
    }                                                          \
    dit = *pdit;                                               \
    if (dict->cmp(key, dit->key) == 0) {                       \
        void *value;                                           \
        dit->next->prev = NULL;                                \
        *pdit = dit->next;                                     \
        value = dit->value;                                    \
        free_fn(dit);                                          \
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
            free_fn(dit);                                      \
            return value;                                      \
        }                                                      \
    }                                                          \
    return NULL

void *
dict_remove_item(dict_t *dict, void *key)
{
    DICT_REMOVE_ITEM_BODY(free);
}


void *
dict_remove_item_mpool(mpool_ctx_t *mpool, dict_t *dict, void *key)
{
#define _free(p) mpool_free(mpool, (p))
    DICT_REMOVE_ITEM_BODY(_free);
#undef _free
}


#define DICT_DELETE_PAIR_BODY(free_fn)         \
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
    free_fn(dit)



void
dict_delete_pair(dict_t *dict, dict_item_t *dit)
{
    DICT_DELETE_PAIR_BODY(free);
}


void
dict_delete_pair_mpool(mpool_ctx_t *mpool, dict_t *dict, dict_item_t *dit)
{
#define _free(p) mpool_free(mpool, (p))
    DICT_DELETE_PAIR_BODY(_free);
#undef _free
}


int
dict_traverse(dict_t *dict, dict_traverser_t cb, void *udata)
{
    int res;
    dict_item_t **pdit;
    array_iter_t it;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        dict_item_t *dit, *next;

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
dict_traverse_item(dict_t *dict, dict_traverser_item_t cb, void *udata)
{
    int res;
    dict_item_t **pdit;
    array_iter_t it;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        dict_item_t *dit, *next;

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
dict_is_empty(dict_t *dict)
{
    dict_item_t **pdit;
    array_iter_t it;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        if (*pdit != NULL) {
            return 0;
        }
    }
    return 1;
}


#define DICT_INIT_BODY(array_init_fn)                          \
    dict->sz = sz;                                             \
    assert(hashfn != NULL);                                    \
    dict->hashfn = hashfn;                                     \
    assert(cmp != NULL);                                       \
    dict->cmp = cmp;                                           \
    dict->fini = fini;                                         \
    array_init_fn(&dict->table, sizeof(dict_item_t *), sz,     \
               (array_initializer_t)null_init,                 \
               NULL)


void
dict_init(dict_t *dict,
          size_t sz,
          dict_hashfn_t hashfn,
          dict_item_comparator_t cmp,
          dict_item_finalizer_t fini)
{
    DICT_INIT_BODY(array_init);
}


void
dict_init_mpool(mpool_ctx_t *mpool,
                dict_t *dict,
                size_t sz,
                dict_hashfn_t hashfn,
                dict_item_comparator_t cmp,
                dict_item_finalizer_t fini)
{
#define _array_init(ar, elsz, elnum, init, fini) array_init_mpool(mpool, (ar), (elsz), (elnum), (init), (fini))
    DICT_INIT_BODY(_array_init);
#undef _array_init
}


#define DICT_CLEANUP_BODY(free_fn)                             \
    if (dict->fini != NULL) {                                  \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            dict_item_t **pdit, *dit, *next;                   \
            pdit = ARRAY_GET(dict_item_t *, &dict->table, i);  \
            for (dit = *pdit; dit != NULL; dit = next) {       \
                next = dit->next;                              \
                if (dict->fini(dit->key, dit->value) != 0) {   \
                    break;                                     \
                }                                              \
                free_fn(dit);                                  \
            }                                                  \
            *pdit = NULL;                                      \
        }                                                      \
    } else {                                                   \
        size_t i;                                              \
        for (i = 0; i < dict->table.elnum; ++i) {              \
            dict_item_t **pdit, *dit, *next;                   \
            pdit = ARRAY_GET(dict_item_t *, &dict->table, i);  \
            for (dit = *pdit; dit != NULL; dit = next) {       \
                next = dit->next;                              \
                free_fn(dit);                                  \
            }                                                  \
            *pdit = NULL;                                      \
        }                                                      \
    }



dict_t *
dict_new(size_t sz,
         dict_hashfn_t hashfn,
         dict_item_comparator_t cmp,
         dict_item_finalizer_t fini)
{
    dict_t *dict;

    if ((dict = malloc(sizeof(dict_t))) == NULL) {
        FAIL("malloc");
    }
    DICT_INIT_BODY(array_init);

    return dict;
}


dict_t *
dict_new_mpool(mpool_ctx_t *mpool,
               size_t sz,
               dict_hashfn_t hashfn,
               dict_item_comparator_t cmp,
               dict_item_finalizer_t fini)
{
    dict_t *dict;

    if ((dict = mpool_malloc(mpool, sizeof(dict_t))) == NULL) {
        FAIL("malloc");
    }
#define _array_init(ar, elsz, elnum, init, fini) array_init_mpool(mpool, (ar), (elsz), (elnum), (init), (fini))
    DICT_INIT_BODY(_array_init);
#undef _array_init

    return dict;
}


void
dict_cleanup(dict_t *dict)
{
    DICT_CLEANUP_BODY(free);
}


void
dict_cleanup_mpool(mpool_ctx_t *mpool, dict_t *dict)
{
#define _free(p) mpool_free(mpool, (p))
    DICT_CLEANUP_BODY(_free);
#undef _free
}


void
dict_fini(dict_t *dict)
{
    dict_cleanup(dict);
    array_fini(&dict->table);
}

void
dict_fini_mpool(mpool_ctx_t *mpool, dict_t *dict)
{
    dict_cleanup_mpool(mpool, dict);
    array_fini_mpool(mpool, &dict->table);
}


void
dict_dump_stats(dict_t *dict)
{
    array_iter_t it;
    dict_item_t **pdit;

    for (pdit = array_first(&dict->table, &it);
         pdit != NULL;
         pdit = array_next(&dict->table, &it)) {

        dict_item_t *dit;
        size_t sz;

        for (dit = *pdit, sz = 0; dit != NULL; dit = dit->next, ++sz) {
        }
        TRACE("bucket %d sz %ld", it.iter, sz);
    }
}

