#include <assert.h>

#include <mrkcommon/heap.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

/*
 * min D-heap
 */

#define HEAP_D (2)
#define HEAP_PARENT(i) (((i) - 1) / HEAP_D)

static int
siftdown(heap_t *heap, int i)
{
    int res;
    void *a, *b, *minb;
    int d, minj;
    int j;

    res = 0;
    a = array_get(&heap->data, i);
    d = 1;
    j = HEAP_D * i + d;
    minb = array_get(&heap->data, j);
    minj = j;

    if (minb != NULL) {
        for (d = 2; d <= HEAP_D; ++d) {
            j = HEAP_D * i + d;
            b = array_get(&heap->data, j);
            if (b != NULL) {
                if (heap->cmp(minb, b) > 0) {
                    minb = b;
                    minj = j;
                }
            } else {
                break;
            }
        }
        if (heap->cmp(a, minb) > 0) {
            heap->swap(a, minb);
            siftdown(heap, minj);
            res = 1;
        }
    }
    return res;
}



void
heap_ify(heap_t *heap)
{
    int i;

    for (i = (int)heap->data.elnum / HEAP_D - 1; i >= 0; --i) {
        siftdown(heap, i);
    }
}


void
heap_push(heap_t *heap, void *v)
{
    void **pv;
    int i;

    pv = array_incr(&heap->data);
    *pv = v;
    ++heap->sz;
    if (heap->data.elnum > 1) {
        for (i = (int)HEAP_PARENT(heap->data.elnum - 1); i >= 0; --i) {
            siftdown(heap, i);
        }
    }
}


int heap_pop(heap_t *heap, void **rv)
{
    int res;
    void **pv;

    assert(rv != NULL);
    pv = array_get(&heap->data, 0);
    if (pv == NULL) {
        res = -1;
    } else {
        void **lv;

        *rv = *pv;
        lv = array_get(&heap->data, heap->data.elnum - 1);
        if (lv != pv) {
            heap->swap(pv, lv);
            array_decr_fast(&heap->data);
            siftdown(heap, 0);
        } else {
            array_decr_fast(&heap->data);
        }
        res = 0;
        --heap->sz;
    }
    return res;
}


int heap_pushpop(heap_t *heap, void **rv)
{
    int res;
    void **pv;

    assert(rv != NULL);
    pv = array_get(&heap->data, 0);
    if (pv == NULL) {
        heap_push(heap, *rv);
        res = -1;
    } else {
        void **lv, *tmp;

        tmp = *rv;
        *rv = *pv;
        lv = array_get(&heap->data, heap->data.elnum - 1);
        if (lv != pv) {
            int i;

            heap->swap(pv, lv);
            *lv = tmp;
            for (i = (int)HEAP_PARENT(heap->data.elnum - 1); i >= 0; --i) {
                siftdown(heap, i);
            }
        } else {
            *lv = tmp;
        }
        res = 0;
    }
    return res;
}


int
heap_traverse(heap_t *heap, heap_traverser_t cb, void *udata)
{
    int res;
    size_t i;
    int j;

    res = 0;
    if ((res = cb(heap, array_get(&heap->data, 0), udata)) != 0) {
        return res;
    }
    for (i = 0; i < heap->data.elnum; ++i) {
        for (j = 1; j <= HEAP_D; ++j) {
            if ((res = cb(heap, array_get(&heap->data, HEAP_D * i + j), udata)) != 0) {
                return res;
            }
        }
    }
    return res;
}


ssize_t
heap_len(const heap_t *heap)
{
    return heap->sz;
}


void
heap_init(heap_t *heap,
          size_t elsz,
          size_t elnum,
          array_initializer_t init,
          array_finalizer_t fini,
          array_compar_t cmp,
          heap_swapfn_t swap)
{
    (void)array_init(&heap->data, elsz, elnum, init, fini);
    heap->cmp = cmp;
    heap->swap = swap;
    heap->sz = 0;
}

void
heap_fini(heap_t *heap)
{
    array_fini(&heap->data);
    heap->cmp = NULL;
    heap->swap = NULL;
    heap->sz = 0;
}
