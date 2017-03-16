#ifndef MRKCOMMON_LIST_H
#define MRKCOMMON_LIST_H

#include <mrkcommon/array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*heap_swapfn_t) (void *, void *);
typedef struct _heap {
    mnarray_t data;
    array_compar_t cmp;
    heap_swapfn_t swap;
    ssize_t sz;
} mnheap_t;

typedef int (*heap_traverser_t) (mnheap_t *, void *, void *);
int heap_traverse(mnheap_t *, heap_traverser_t, void *);

void heap_push(mnheap_t *, void *);
int heap_pushpop(mnheap_t *, void **);
int heap_pop(mnheap_t *, void **);
void heap_ify(mnheap_t *);
ssize_t heap_len(const mnheap_t *);
#define heap_isempty(h) (heap_len(h) <= 0)

void heap_init(mnheap_t *,
               size_t,
               size_t,
               array_initializer_t,
               array_finalizer_t,
               array_compar_t,
               heap_swapfn_t);

void heap_fini(mnheap_t *);
int heap_pointer_swap(void *, void *);
int heap_pointer_null(void *);
#ifdef __cplusplus
}
#endif

#endif
