#include <assert.h>
#include <stdbool.h>

#include <mncommon/heap.h>
#include <mncommon/dumpm.h>
#include <mncommon/util.h>

/*
 * min D-heap
 */

#define HEAP_D (2)
#define HEAP_PARENT(i) (((i) - 1) / HEAP_D)

static int
siftdown(mnheap_t *heap, int i)
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
            (void)heap->swap(a, minb);
            (void)siftdown(heap, minj);
            res = 1;
        }
    }
    return res;
}


void
heap_sort(mnheap_t *heap)
{
    (void)array_sort(&heap->data, heap->cmp);
}


void
heap_sort_custom(mnheap_t *heap, array_compar_t cmp)
{
    (void)array_sort(&heap->data, cmp);
}


void
heap_ify(mnheap_t *heap)
{
    int i;

    for (i = (int)heap->data.elnum / HEAP_D - 1; i >= 0; --i) {
        (void)siftdown(heap, i);
    }
}


static void *
heap_get_at(mnheap_t *heap, int i, void *a)
{
    int j;
    void *b;

    b = NULL;
    for (j = 1; j <= HEAP_D; ++j) {

        if ((b = array_get(&heap->data, i * HEAP_D + j)) != NULL) {
            int diff;

            diff = heap->cmp(a, b);
            if (diff == 0) {
                break;

            } else if (diff > 0) {
                /* try */
                if ((b = heap_get_at(heap, i * HEAP_D + j, a)) != NULL) {
                    break;
                }
            } else {
                /* continue */
            }
        }
    }
    if (b != NULL && heap->cmp(a, b) != 0) {
        b = NULL;
    }
    return b;
}


int
heap_get(mnheap_t *heap, void *a, void **rv)
{
    int res;
    void **b;

    if ((b = array_get(&heap->data, 0)) != NULL) {
        int diff;

        diff = heap->cmp(&a, b);
        if (diff == 0) {
            *rv = *b;
            res = 0;
        } else if (diff < 0) {
            res = -1;
        } else {
            if ((b = heap_get_at(heap, 0, (void *)&a)) == NULL) {
                res = -1;
            } else {
                *rv = *b;
                res = 0;
            }
        }
    } else {
        res = -1;
    }
    return res;
}


void *
heap_head(mnheap_t *heap)
{
    return array_get(&heap->data, 0);
}


void
heap_push(mnheap_t *heap, void *v)
{
    void **pv;
    int i;

    pv = array_incr(&heap->data);
    *pv = v;
    if (heap->data.elnum > 1) {
        for (i = (int)HEAP_PARENT(heap->data.elnum - 1); i >= 0; --i) {
            (void)siftdown(heap, i);
        }
    }
}


int
heap_pop(mnheap_t *heap, void **rv)
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
            (void)heap->swap(pv, lv);
            array_decr_fast(&heap->data);
            (void)siftdown(heap, 0);
        } else {
            array_decr_fast(&heap->data);
        }
        res = 0;
    }
    return res;
}


int
heap_pushpop(mnheap_t *heap, void **rv)
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

            (void)heap->swap(pv, lv);
            *lv = tmp;
            for (i = (int)HEAP_PARENT(heap->data.elnum - 1); i >= 0; --i) {
                (void)siftdown(heap, i);
            }
        } else {
            *lv = tmp;
        }
        res = 0;
    }
    return res;
}


int
heap_traverse(mnheap_t *heap, heap_traverser_t cb, void *udata)
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


int
heap_traversea(mnheap_t *heap, array_traverser_t cb, void *udata)
{
    int res;
    size_t i;
    int j;

    res = 0;
    if ((res = cb(array_get(&heap->data, 0), udata)) != 0) {
        return res;
    }
    for (i = 0; i < heap->data.elnum; ++i) {
        for (j = 1; j <= HEAP_D; ++j) {
            if ((res = cb(array_get(&heap->data, HEAP_D * i + j), udata)) != 0) {
                return res;
            }
        }
    }
    return res;
}


static int
heap_traverse_seq_cb(void *v, void *udata)
{
    struct {
        mnheap_t *heap;
        heap_traverser_t cb;
        void *udata;
    } *params = udata;
    return params->cb(params->heap, v, params->udata);
}


int
heap_traverse_seq(mnheap_t *heap, heap_traverser_t cb, void *udata)
{
    struct {
        mnheap_t *heap;
        heap_traverser_t cb;
        void *udata;
    } params = { heap, cb, udata };
    return array_traverse(&heap->data, heap_traverse_seq_cb, &params);
}


int
heap_traversea_seq(mnheap_t *heap, array_traverser_t cb, void *udata)
{
    return array_traverse(&heap->data, cb, udata);
}


ssize_t
heap_len(const mnheap_t *heap)
{
    return heap->data.elnum;
}


void
heap_init(mnheap_t *heap,
          size_t elsz,
          size_t elnum,
          array_initializer_t init,
          array_finalizer_t fini,
          array_compar_t cmp,
          heap_swapfn_t swap)
{
    (void)array_init(&heap->data, elsz, 0, init, fini);
    array_ensure_datasz_dirty(&heap->data, elnum, 0);
    heap->cmp = cmp;
    heap->swap = swap;
}


void
heap_fini(mnheap_t *heap)
{
    array_fini(&heap->data);
    heap->cmp = NULL;
    heap->swap = NULL;
}


int
heap_pointer_swap(void *a, void *b)
{
    void **pa = (void **)a;
    void **pb = (void **)b;
    void *tmp;
    tmp = *pa;
    *pa = *pb;
    *pb = tmp;
    return 0;
}


int
heap_pointer_null(void *a)
{
    void **aa = (void **)a;
    *aa = NULL;
    return 0;
}
