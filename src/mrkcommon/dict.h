#ifndef MRKCOMMON_DICT_H
#define MRKCOMMON_DICT_H

#include <assert.h>
#include <stdint.h>
#include <sys/types.h>

#include "mrkcommon/array.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _dict_item {
    struct _dict *dict;
    struct _dict_item **bucket;
    struct _dict_item *prev;
    struct _dict_item *next;
    void *key;
    void *value;
} dict_item_t;

typedef uint64_t (*dict_hashfn_t)(void *);
typedef int (*dict_item_comparator_t)(void *, void *);
typedef int (*dict_item_finalizer_t)(void *, void *);
typedef int (*dict_traverser_t)(void *, void *, void *);
typedef int (*dict_traverser_item_t)(dict_item_t *, void *);

typedef struct _dict {
    size_t sz;
    dict_hashfn_t hashfn;
    dict_item_comparator_t cmp;
    dict_item_finalizer_t fini;
    array_t table;
} dict_t;



void dict_set_item(dict_t *, void *, void *);
dict_item_t *dict_get_item(dict_t *, void *);
void *dict_remove_key(dict_t *, void *);
void dict_remove_item(dict_item_t *);
int dict_traverse(dict_t *, dict_traverser_t, void *);
int dict_traverse_item(dict_t *, dict_traverser_item_t, void *);

void dict_init(dict_t *,
               size_t,
               dict_hashfn_t,
               dict_item_comparator_t,
               dict_item_finalizer_t);
void dict_fini(dict_t *);

#ifdef __cplusplus
}
#endif

#endif
