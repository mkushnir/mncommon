#include <assert.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <string.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/hash.h>
#include <mrkcommon/util.h>

#include <mrkcommon/heap.h>

#include "diag.h"
#include "unittest.h"

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static int
mycmp(uint64_t *a, uint64_t *b)
{
    return *a < *b ? -1 : *a > *b ? 1 : 0;
}

static void
myswap(void **a, void **b)
{
    void *tmp;

    //TRACE("%p <-> %p", *a, *b);
    tmp = *a;
    *a = *b;
    *b = tmp;
}


static int
mycb(UNUSED mnheap_t *heap, void *pval, UNUSED void *udata)
{
    uint64_t *v;

    if (pval == NULL) {
        return -1;
    }
    v = pval;
    TRACE("val=%ld", *v);
    return 0;
}


static void
test0(void)
{
    mnheap_t heap;
    int res;
    struct {
        long rnd;
        uint64_t i;
    } data[] = {
        {0, 5},
        {0, 5},
        {0, 11},
        {0, 12},
        {0, 51},
        {0, 45},
        {0, 26},
        {0, 14},
        {0, 25},
        {0, 87},
        {0, 79},
        {0, 56},
        {0, 59},
        {0, 41},
        {0, 30},
        {0, 96},
        {0, 89},
        {0, 26},
        {0, 45},
        {0, 94},
    };
    UNITTEST_PROLOG_RAND;

    heap_init(&heap,
              sizeof(uint64_t),
              0,
              NULL,
              NULL,
              (array_compar_t)mycmp,
              (heap_swapfn_t)myswap);


    //for (i = 0; i < 20; ++i) {
    //    uint64_t v;

    //    v = random() % 100;
    //    heap_push(&heap, (void *)(intptr_t)v);
    //}
    FOREACHDATA {
        heap_push(&heap, (void *)(intptr_t)CDATA.i);
        //TRACE("pushed %ld", CDATA.i);
    }

    //TRACE("res=%d", res);
    //heap_ify(&heap);
    res = heap_traverse(&heap, mycb, NULL);
    TRACE("res=%d", res);

    while (1) {
        uint64_t v;
        if (heap_pop(&heap, (void **)&v) != 0) {
            break;
        }
        TRACE("popped %ld", v);
    }

    heap_fini(&heap);
}


static void
test1(void)
{
    mnheap_t heap;
    int res;
    struct {
        long rnd;
        uint64_t i;
    } data[] = {
        {0, 5},
    };
    UNITTEST_PROLOG_RAND;

    heap_init(&heap,
              sizeof(uint64_t),
              0,
              NULL,
              NULL,
              (array_compar_t)mycmp,
              (heap_swapfn_t)myswap);


    for (i = 0; i < 20; ++i) {
        uint64_t v;

        v = random() % 100;
        heap_push(&heap, (void *)(intptr_t)v);
    }

    //TRACE("res=%d", res);
    //heap_ify(&heap);
    res = heap_traverse(&heap, mycb, NULL);
    TRACE("res=%d", res);

    for (i = 0; i < 10; ++i) {
        uint64_t v;

        v = 200;
        if (heap_pushpop(&heap, (void **)&v) != 0) {
            break;
        }
        res = heap_traverse(&heap, mycb, NULL);
        TRACE("res=%d", res);
    }

    heap_fini(&heap);
}


static void
test2(void)
{
    mnheap_t heap;
    struct {
        long rnd;
        uint64_t i;
    } data[] = {
        {0, 0},
        {0, 1},
        {0, 2},
        {0, 3},
        {0, 4},
        {0, 5},
        {0, 6},
        {0, 7},
        {0, 8},
        {0, 9},
    };
    UNITTEST_PROLOG;

    heap_init(&heap,
              sizeof(uint64_t),
              0,
              NULL,
              NULL,
              (array_compar_t)mycmp,
              (heap_swapfn_t)myswap);

    FOREACHDATA {
        heap_push(&heap, (void *)(intptr_t)CDATA.i);
        //TRACE("pushed %ld", CDATA.i);
    }
    TRACE("direct");
    heap_traverse(&heap, mycb, NULL);
    TRACE("---");
    heap_traverse_seq(&heap, mycb, NULL);
    while (1) {
        uint64_t v;
        if (heap_pop(&heap, (void **)&v) != 0) {
            break;
        }
        TRACE("popped %ld", v);
    }
    heap_fini(&heap);
}


static void
test3(void)
{
    mnheap_t heap;
    struct {
        long rnd;
        uint64_t i;
    } data[] = {
        {0, 9},
        {0, 8},
        {0, 7},
        {0, 6},
        {0, 5},
        {0, 4},
        {0, 3},
        {0, 2},
        {0, 1},
        {0, 0},
    };
    UNITTEST_PROLOG;

    heap_init(&heap,
              sizeof(uint64_t),
              0,
              NULL,
              NULL,
              (array_compar_t)mycmp,
              (heap_swapfn_t)myswap);

    FOREACHDATA {
        heap_push(&heap, (void *)(intptr_t)CDATA.i);
        //TRACE("pushed %ld", CDATA.i);
    }
    TRACE("reverse");
    heap_traverse(&heap, mycb, NULL);
    TRACE("---");
    heap_traverse_seq(&heap, mycb, NULL);
    while (1) {
        uint64_t v;
        if (heap_pop(&heap, (void **)&v) != 0) {
            break;
        }
        TRACE("popped %ld", v);
    }
    heap_fini(&heap);
}


static void
test4(void)
{
    mnheap_t heap;
    struct {
        long rnd;
        uint64_t i;
    } data[] = {
        {0, 0},
        {0, 1},
        {0, 2},
        {0, 3},
        {0, 4},
        {0, 5},
        {0, 6},
        {0, 7},
        {0, 8},
        {0, 9},
    };
    UNITTEST_PROLOG_RAND;
    uint64_t v;
    int res;

    heap_init(&heap,
              sizeof(uint64_t),
              0,
              NULL,
              NULL,
              (array_compar_t)mycmp,
              (heap_swapfn_t)myswap);

    FOREACHDATA {
        heap_push(&heap, (void *)(intptr_t)CDATA.i);
        //TRACE("pushed %ld", CDATA.i);
    }
    TRACE("random");
    heap_traverse(&heap, mycb, NULL);
    TRACE("---");
    heap_traverse_seq(&heap, mycb, NULL);
    for (i = 0; i < 12; ++i) {
        v = -1;
        res = heap_get(&heap, (void *)(intptr_t)i, (void **)&v);
        TRACE("heap_get(%d): res=%d v=%ld", i, res, v);
    }
    while (1) {
        if (heap_pop(&heap, (void **)&v) != 0) {
            break;
        }
        TRACE("popped %ld", v);
    }
    heap_fini(&heap);
}


int
main(int argc, char **argv)
{
    int i;

    test0();
    test1();
    test2();
    test3();
    test4();
    for (i = 1; i < argc; ++i) {
        TRACE("arg=%s", argv[i]);
    }
    return 0;
}
