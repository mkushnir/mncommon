#include <assert.h>
#include <stdlib.h>

#include "mrkcommon/array.h"
#include "mrkcommon/dict.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "diag.h"

#ifndef NDEBUG
#include "mrkcommon/memdebug.h"
MEMDEBUG_DECLARE(dict);
#endif

static int
null_init(void **v)
{
    *v = NULL;
    return 0;
}

static int
dict_item_fini(dict_item_t **pdit)
{

    if (*pdit != NULL) {
        dict_item_t *dit = *pdit;
        if (dit->next != NULL) {
            dict_item_fini(&dit->next);
        }

        if (dit->dict->fini != NULL) {
            dit->dict->fini(dit->key, dit->value);
        }

        free(dit);
        *pdit = NULL;
    }
    return 0;
}

void
dict_set_item(dict_t *dict, void *key, void *value)
{
    uint64_t idx;
    dict_item_t **pdit, *dit;

    idx = dict->hashfn(key) % dict->sz;
    if ((pdit = array_get(&dict->table, idx)) == NULL) {
        FAIL("array_get");
    }

    if (*pdit == NULL) {
        if ((dit = malloc(sizeof(dict_item_t))) == NULL) {
            FAIL("malloc");
        }
        dit->dict = dict;
        dit->bucket = pdit;
        dit->prev = NULL;
        dit->next = NULL;
        dit->key = key;
        dit->value = value;
        *pdit = dit;

    } else {
        dit = *pdit;
        /* move to last */
        while (dit->next != NULL) {
            dit = dit->next;
        }
        if ((dit->next = malloc(sizeof(dict_item_t))) == NULL) {
            FAIL("malloc");
        }
        dit->next->dict = dict;
        dit->bucket = NULL;
        dit->next->prev = dit;
        dit->next->next = NULL;
        dit->next->key = key;
        dit->next->value = value;
    }
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

void *
dict_remove_key(dict_t *dict, void *key)
{
    uint64_t idx;
    dict_item_t **pdit, *dit;
    void *value = NULL;

    idx = dict->hashfn(key) % dict->sz;

    if ((pdit = array_get(&dict->table, idx)) == NULL) {
        FAIL("array_get");
    }

    if (*pdit == NULL) {
        return NULL;
    }

    dit = *pdit;

    if (dict->cmp(key, dit->key) == 0) {
        dit->next->prev = NULL;
        *pdit = dit->next;
        value = dit->value;
        free(dit);
        return value;
    }

    while (dit->next != NULL) {
        dit = dit->next;
        if (dict->cmp(key, dit->key) == 0) {
            dit->prev->next = dit->next;
            if (dit->next != NULL) {
                dit->next->prev = dit->prev;
            }
            value = dit->value;
            free(dit);
            return value;
        }
    }

    return value;
}


void
dict_remove_item(dict_item_t *dit)
{
    if (dit->prev != NULL) {
        dit->prev->next = dit->next;
    } else {
        assert(dit->bucket != NULL);
        *(dit->bucket) = dit->next;
    }
    if (dit->next != NULL) {
        dit->next->prev = dit->prev;
    }
    free(dit);
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
            if ((res = cb(dit, udata)) != 0) {
                return res;
            }
        }
    }
    return 0;
}


void
dict_init(dict_t *dict,
          size_t sz,
          dict_hashfn_t hashfn,
          dict_item_comparator_t cmp,
          dict_item_finalizer_t fini)
{
    dict->sz = sz;
    assert(hashfn != NULL);
    dict->hashfn = hashfn;
    assert(cmp != NULL);
    dict->cmp = cmp;
    dict->fini = fini;
    array_init(&dict->table, sizeof(dict_item_t *), sz,
               (array_initializer_t)null_init,
               (array_finalizer_t)dict_item_fini);
}

void
dict_fini(dict_t *dict)
{
    array_fini(&dict->table);
}
