#ifndef TEST_CM_H_DEFINED
#define TEST_CM_H_DEFINED

#include <sys/types.h>

#include <mrkcommon/dict.h>
//#include <mrkcommon/trie.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMTY uint64_t
#define CMMAX ((CMTY)UINT64_MAX)
#define CMMIN ((CMTY)0lu)

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


typedef struct _pqueue {
    uint64_t c;
    ssize_t nleft;
    dict_t d;
    CMTY fast_pop_thresh;
} pqueue_t;


typedef struct _pqueue_item {
    uint64_t _gen;
    void *v;
    CMTY cmprop;
} pqueue_item_t;



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
 * pqueue -- dict_t based priority queue
 */
pqueue_item_t *pqueue_peek(pqueue_t *, pqueue_item_t *);
pqueue_item_t *pqueue_pop(pqueue_t *);
pqueue_item_t *pqueue_push(pqueue_t *, pqueue_item_t *);
int pqueue_traverse(pqueue_t *, dict_traverser_t, void *);

void pqueue_init(pqueue_t *,
                 ssize_t,
                 dict_hashfn_t,
                 dict_item_comparator_t,
                 dict_item_finalizer_t);

void pqueue_fini(pqueue_t *);

#ifdef __cplusplus
}
#endif
#endif /* TEST_CM_H_DEFINED */
