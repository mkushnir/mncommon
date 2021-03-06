#ifndef TEST_CM_H_DEFINED
#define TEST_CM_H_DEFINED

#include <sys/types.h>

#include <mncommon/hash.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMTY int64_t
#define CMMAX ((CMTY)INT64_MAX)
#define CMMIN ((CMTY)0l)

typedef struct _cm {
    size_t w;
    size_t d;
    CMTY *v; /*count */
} cm_t;


typedef struct _cmhash {
    int *v; /* depth index */
    size_t w;
    size_t d;
    uint64_t h;
    void *o;
    size_t osz;
    uint64_t oh;
} cmhash_t;


typedef struct _pset {
    ssize_t nleft;
    mnhash_t d;
    CMTY minthresh;
    CMTY fast_pop_thresh;
} pset_t;


typedef struct _pset_item {
    void *v;
    CMTY cmprop;
} pset_item_t;



/*
 * cm_t
 */
void cm_init(cm_t *, size_t, size_t);
void cm_fini(cm_t *);
void cm_add(cm_t *, void *, size_t, CMTY);
void cm_get(cm_t *, void *, size_t, CMTY *);


/*
 * cmhash (research purposes)
 */
cmhash_t *cmhash_new(size_t, size_t);
void cmhash_destroy(cmhash_t **);
void cmhash_hash(cmhash_t *, void *, size_t);
void cmhash_dump(cmhash_t *);


/*
 * pset -- mnhash_t based priority set
 */
pset_item_t *pset_peek(pset_t *, pset_item_t *);
pset_item_t *pset_pop(pset_t *);
pset_item_t *pset_push(pset_t *, pset_item_t *);
int pset_traverse(pset_t *, hash_traverser_t, void *);

void pset_init(pset_t *,
                 ssize_t,
                 hash_hashfn_t,
                 hash_item_comparator_t,
                 hash_item_finalizer_t,
                 CMTY);

void pset_fini(pset_t *);

#ifdef __cplusplus
}
#endif
#endif /* TEST_CM_H_DEFINED */
