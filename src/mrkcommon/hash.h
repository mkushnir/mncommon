#ifndef MRKCOMMON_HASH_H
#define MRKCOMMON_HASH_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <mrkcommon/array.h>
#include <mrkcommon/mpool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Useful primes
 * 7
 * 11
 * 17
 * 31
 * 61
 * 127
 *
 * 251
 * 257
 *
 * 509
 * 521
 *
 * 1021
 * 1031
 *
 * 2039
 * 2053
 *
 * 4093
 * 4099
 *
 * 8191
 * 8209
 *
 * 16381
 * 16411
 *
 * 32749
 * 32771
 *
 * 65213 centered heptagonal prime
 * 65537 Fermat prime
 * 68111 chp
 * 72073 chp
 * 76147 chp
 * 84631 chp
 * 89041 chp
 * 93563 chp
 * 193939 circular prime
 * 1046527 carol prime
 * 27644437 bell number prime
 */

typedef struct _hash_item {
    struct _hash_item **bucket;
    struct _hash_item *prev;
    struct _hash_item *next;
    void *key;
    void *value;
} mnhash_item_t;

typedef uint64_t (*hash_hashfn_t)(void *);
typedef int (*hash_item_comparator_t)(void *, void *);
typedef int (*hash_item_finalizer_t)(void *, void *);
typedef int (*hash_traverser_t)(void *, void *, void *);
typedef struct _mnhash {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#endif
    hash_hashfn_t hashfn;
    hash_item_comparator_t cmp;
    hash_item_finalizer_t fini;
    mnarray_t table;
    size_t elnum;
} mnhash_t;


typedef struct _mnhash_iter {
    mnarray_iter_t it;
    mnhash_item_t **phit;
    mnhash_item_t *hit;
} mnhash_iter_t;


void hash_set_item(mnhash_t *, void *, void *);
void hash_set_item_mpool(mpool_ctx_t *, mnhash_t *, void *, void *);
void hash_set_item_uniq(mnhash_t *,
                        void *,
                        void *,
                        void **,
                        void **);
void hash_set_item_uniq_mpool(mpool_ctx_t *,
                              mnhash_t *,
                              void *,
                              void *,
                              void **,
                              void **);
mnhash_item_t *hash_get_item(mnhash_t *, void *);
void *hash_remove_item(mnhash_t *, void *);
void *hash_remove_item_mpool(mpool_ctx_t *, mnhash_t *, void *);
void hash_delete_pair(mnhash_t *, mnhash_item_t *);
void hash_delete_pair_no_fini(mnhash_t *, mnhash_item_t *);
void hash_delete_pair_mpool(mpool_ctx_t *, mnhash_t *, mnhash_item_t *);
void hash_delete_pair_no_fini_mpool(mpool_ctx_t *, mnhash_t *, mnhash_item_t *);
int hash_traverse(mnhash_t *, hash_traverser_t, void *);
typedef int (*hash_traverser_item_t)(mnhash_t *, mnhash_item_t *, void *);

int hash_traverse_item(mnhash_t *, hash_traverser_item_t, void *);
mnhash_item_t *hash_first(mnhash_t *, mnhash_iter_t *);
mnhash_item_t *hash_next(mnhash_t *, mnhash_iter_t *);
bool hash_is_empty(mnhash_t *);
size_t hash_get_elnum(mnhash_t *);

mnhash_t *hash_new(size_t,
                 hash_hashfn_t,
                 hash_item_comparator_t,
                 hash_item_finalizer_t);
mnhash_t *hash_new_mpool(mpool_ctx_t *,
                       size_t,
                       hash_hashfn_t,
                       hash_item_comparator_t,
                       hash_item_finalizer_t);
void hash_init(mnhash_t *,
               size_t,
               hash_hashfn_t,
               hash_item_comparator_t,
               hash_item_finalizer_t);
void hash_init_mpool(mpool_ctx_t *,
                     mnhash_t *,
                     size_t,
                     hash_hashfn_t,
                     hash_item_comparator_t,
                     hash_item_finalizer_t);
void hash_rehash(mnhash_t *, size_t);
void hash_rehash_mpool(mpool_ctx_t *, mnhash_t *, size_t);
void hash_cleanup(mnhash_t *);
void hash_cleanup_mpool(mpool_ctx_t *, mnhash_t *);
void hash_fini(mnhash_t *);
void hash_fini_mpool(mpool_ctx_t *, mnhash_t *);
void hash_destroy(mnhash_t **);
void hash_destroy_mpool(mpool_ctx_t *, mnhash_t **);

void hash_dump_stats(mnhash_t *);

/*
 * set
 */
void hash_set_add(mnhash_t *, void *);
void hash_set_remove(mnhash_t*, void *);
void hash_set_union2(mnhash_t *, mnhash_t *);
void hash_set_union3(mnhash_t *, mnhash_t *, mnhash_t *);
void hash_set_diff3(mnhash_t *, mnhash_t *, mnhash_t *);

#ifdef __cplusplus
}
#endif

#endif
