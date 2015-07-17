#ifndef MRKCOMMON_LIST_H
#define MRKCOMMON_LIST_H

#include <mrkcommon/array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*heap_swapfn_t) (void *, void *);
typedef struct _heap {
    array_t data;
    array_compar_t cmp;
    heap_swapfn_t swap;
    ssize_t sz;
} heap_t;

typedef int (*heap_traverser_t) (heap_t *, void *, void *);
int heap_traverse(heap_t *, heap_traverser_t, void *);

void heap_push(heap_t *, void *);
int heap_pushpop(heap_t *, void **);
int heap_pop(heap_t *, void **);
void heap_ify(heap_t *);
ssize_t heap_len(const heap_t *);
#define heap_isempty(h) (heap_len(h) <= 0)

void heap_init(heap_t *,
               size_t,
               size_t,
               array_initializer_t,
               array_finalizer_t,
               array_compar_t,
               heap_swapfn_t);

void heap_fini(heap_t *);
#ifdef __cplusplus
}
#endif

#endif
