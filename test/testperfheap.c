#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <err.h>

#include "unittest.h"
#include "mncommon/dumpm.h"
#include "mncommon/heap.h"
#include "mncommon/profile.h"
#include "mncommon/util.h"
#ifdef DO_MEMDEBUG
#include "mncommon/memdebug.h"
MEMDEBUG_DECLARE(testperfheap);
#endif

typedef struct _key_item {
    uint64_t key;
    void *n;
    uint64_t add_time;
    uint64_t find_time;
    uint64_t remove_time;
} key_item_t;

#define TESTPERFN (1024*1024)
static key_item_t keys[TESTPERFN];

static mnheap_t heap;

UNUSED static uint64_t
new_id_random(void)
{
    return (((uint64_t)random()) << 32) | random();
}

UNUSED static uint64_t
new_id_successive(void)
{
    static uint64_t id = 0xffffffffffffffff;
    return --id;
}

static void
initialize_ids(void)
{
    unsigned i;

    for (i = 0; i < countof(keys); ++i) {
        keys[i].key = new_id_successive();
        //keys[i].key = new_id_random();
    }
}


static int
mycmp(void **a, void **b)
{
    return MNCMP(*a, *b);
}


static void
test0(void)
{
    const profile_t *p_add_node;
    const profile_t *p_find_node;
    const profile_t *p_remove_node;

    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }

    p_add_node = profile_register("add_node");
    p_find_node = profile_register("find_node");
    p_remove_node = profile_register("remove_node");
    TRACE("Started");
    initialize_ids();
    TRACE("initialize_ids OK");
    heap_init(&heap,
              sizeof(uint64_t),
              TESTPERFN,
              NULL,
              NULL,
              (array_compar_t)mycmp,
              heap_pointer_swap);

    for (i = 0; i < countof(keys); ++i) {
        void *v;
        v = (void *)keys[i].key;
        profile_start(p_add_node);
        heap_pushpop(&heap, &v);
        keys[i].add_time = profile_stop(p_add_node);
    }
    TRACE("add_node OK");

    for (i = 0; i < countof(keys); ++i) {
        void *v;
        profile_start(p_find_node);
        /* TODO */
        (void)heap_get(&heap, keys[i].n, &v);
        keys[i].find_time = profile_stop(p_find_node);
    }
    TRACE("find_node OK");

    for (i = 0; i < countof(keys); ++i) {
        uint64_t v;
        profile_start(p_remove_node);
        heap_pop(&heap, (void **)&v);
        keys[i].n = NULL;
        keys[i].remove_time = profile_stop(p_remove_node);
    }

    TRACE("remove_node OK");

    heap_fini(&heap);
    TRACE("hash_fini OK");
    profile_report();

    for (i = 0; i < countof(keys); ++i) {
        printf("%d %ld %ld %ld\n",
               i,
               (long)keys[i].add_time,
               (long)keys[i].find_time,
               (long)keys[i].remove_time);
    }
}

int
main(void)
{
#ifndef NDEBUG
#ifdef DO_MEMDEBUG
    MEMDEBUG_REGISTER(testperfheap);
    //MEMDEBUG_REGISTER(heap);
#endif
#endif

    profile_init_module();

    test0();

#ifdef DO_MEMDEBUG
    memdebug_print_stats();
#endif
    profile_fini_module();
    return 0;
}

