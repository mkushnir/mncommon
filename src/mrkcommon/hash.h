#ifndef MRKCOMMON_HASH_H
#define MRKCOMMON_HASH_H

#include <assert.h>
#include <stdint.h>
#include <sys/types.h>

#include <mrkcommon/array.h>
#include <mrkcommon/mpool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _hash_item {
    struct _hash_item **bucket;
    struct _hash_item *prev;
    struct _hash_item *next;
    void *key;
    void *value;
} hash_item_t;

typedef uint64_t (*hash_hashfn_t)(void *);
typedef int (*hash_item_comparator_t)(void *, void *);
typedef int (*hash_item_finalizer_t)(void *, void *);
typedef int (*hash_traverser_t)(void *, void *, void *);
typedef struct _dict {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#endif
    hash_hashfn_t hashfn;
    hash_item_comparator_t cmp;
    hash_item_finalizer_t fini;
    array_t table;
    size_t elnum;
} hash_t;



void hash_set_item(hash_t *, void *, void *);
void hash_set_item_mpool(mpool_ctx_t *, hash_t *, void *, void *);
void hash_set_item_uniq(hash_t *dict,
                        void *key,
                        void *value,
                        void **oldkey,
                        void **oldvalue);
void hash_set_item_uniq_mpool(mpool_ctx_t *mpool,
                              hash_t *dict,
                              void *key,
                              void *value,
                              void **oldkey,
                              void **oldvalue);
hash_item_t *hash_get_item(hash_t *, void *);
void *hash_remove_item(hash_t *, void *);
void *hash_remove_item_mpool(mpool_ctx_t *, hash_t *, void *);
void hash_delete_pair(hash_t *, hash_item_t *);
void hash_delete_pair_mpool(mpool_ctx_t *, hash_t *, hash_item_t *);
void hash_delete_pair_no_fini_mpool(mpool_ctx_t *, hash_t *, hash_item_t *);
int hash_traverse(hash_t *, hash_traverser_t, void *);
typedef int (*hash_traverser_item_t)(hash_t *, hash_item_t *, void *);

int hash_traverse_item(hash_t *, hash_traverser_item_t, void *);
int hash_is_empty(hash_t *);
size_t hash_get_elnum(hash_t *);

hash_t *hash_new(size_t,
                 hash_hashfn_t,
                 hash_item_comparator_t,
                 hash_item_finalizer_t);
hash_t *hash_new_mpool(mpool_ctx_t *,
                       size_t,
                       hash_hashfn_t,
                       hash_item_comparator_t,
                       hash_item_finalizer_t);
void hash_init(hash_t *,
               size_t,
               hash_hashfn_t,
               hash_item_comparator_t,
               hash_item_finalizer_t);
void hash_init_mpool(mpool_ctx_t *,
                     hash_t *,
                     size_t,
                     hash_hashfn_t,
                     hash_item_comparator_t,
                     hash_item_finalizer_t);
void hash_cleanup(hash_t *);
void hash_cleanup_mpool(mpool_ctx_t *, hash_t *);
void hash_fini(hash_t *);
void hash_fini_mpool(mpool_ctx_t *, hash_t *);

void hash_dump_stats(hash_t *);

#ifdef __cplusplus
}
#endif

#endif
