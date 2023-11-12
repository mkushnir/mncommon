#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "mncommon/dumpm.h"
#include "mncommon/util.h"
#include "mncommon/btrie.h"

#ifndef HAVE_FLSL
#   ifdef __GCC__
#       define flsl(v) (v ? (TREE_DEPTH - __builtin_clzl(v)) : 0)
#   else
#       error "Could not find/define flsl."
#   endif
#endif

UNUSED static void
test0(void)
{
    //int values[] = {777};
    int values[] = {0, 1, 2, 4, 8, 9, 10, 11, 12};
    int values1[] = {0,1,2,3,4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1,2,3,4,4};
    //int values[] = {0x00, 0x200};
    //int values1[] = {0x400};
    unsigned i;
    mnbtrie_t tr;
    mnbtrie_node_t *n;

    btrie_init(&tr);

    for (i = 0; i < countof(values); ++i) {
        test_data_t *d;

        n = btrie_add_node(&tr, values[i]);
        if (values[i] == 0) {
            assert(n == NULL);
            continue;
        }
        d = malloc(sizeof(test_data_t));
        d->key = values[i];
        n->value = d;
    }

    btrie_traverse(&tr, btrie_node_dump_cb, NULL);

    for (i = 0; i < countof(values1); ++i) {
        //TRACE("querying: %x", values1[i]);
        //n = btrie_find_exact(&tr, values1[i]);
        //btrie_node_dump_cb(n, NULL);

        //TRACE("key to find: %d", values1[i] - 1);
        //n = btrie_find_closest(&tr, values1[i] - 1, 1);
        //btrie_node_dump_cb(n, NULL);

        TRACE("key to find: %d", values1[i] + 1);
        n = btrie_find_closest(&tr, values1[i] + 1, 0);
        btrie_node_dump_cb(n, NULL);

    }

    for (i = 0; i < countof(values1); ++i) {
        //TRACE("removing: %d", values1[i]);
        n = btrie_find_exact(&tr, values1[i]);
        if (n != NULL) {
            n->value = NULL;
            btrie_remove_node(&tr, n);
        }
    }

    btrie_fini(&tr);
}


UNUSED static void
test1(void)
{
    uint64_t values[] = {0, 1, 0, 9, 11, 31, 200, 40, 70, 80, 199};
    UNUSED uint64_t values1[] = {0, 1, 0, 19, 11, 31, 199, 200, 40, 70, 80, 81, 201};
    UNUSED uint64_t values2[] = {0, 1, 0, 11};
    //uint64_t values1[] = {0, 11, 22, 33, 44, 55, 66, 77, 88};
    //uint64_t values[] = {4};
    unsigned i;
    mnbtrie_t tr;
    mnbtrie_node_t *n;

    btrie_init(&tr);

    //TRACE("test add");

    for (i = 0; i < countof(values); ++i) {
        n = btrie_add_node(&tr, values[i]);
        if (values[i] == 0) {
            assert(n == NULL);
            continue;
        }
        if (n->value == NULL) {
            test_data_t *d;

            d = malloc(sizeof(test_data_t));
            d->key = values[i];
            n->value = d;
        }
        //btrie_node_traverse(n, btrie_node_dump_cb, NULL);
    }

    //btrie_traverse(&tr, btrie_node_dump_cb, NULL);

    //TRACE("test find");

    for (i = 0; i < countof(values1); ++i) {
        //TRACE("searching for key=%02lx", values1[i]);
        n = btrie_find_exact(&tr, values1[i]);
        if (n == NULL) {
            //TRACE("N/A");
        } else {
            //btrie_node_dump_cb(n, (void *)1);
        }
    }

    //TRACE("test remove");

    for (i = 0; i < countof(values2); ++i) {
        n = btrie_find_exact(&tr, values2[i]);
        //TRACE("found %p for %ld", n, values2[i]);
        //btrie_node_dump(n);
        if (n != NULL) {
            n->value = NULL;
            btrie_remove_node(&tr, n);
            //TRACE("removed %ld OK", values2[i]);
        } else {
            //TRACE("removed %ld FAIL", values2[i]);
        }
    }

    for (i = 0; i < 202; ++i) {
        //TRACE("searching closest for key=%02lx", (uint64_t)i);
        n = btrie_find_closest(&tr, i, 1);
        if (n == NULL) {
            //TRACE("N/A");
        } else {
            //btrie_node_dump_cb(n, (void *)1);
        }
    }

    //btrie_traverse(&tr, btrie_node_dump_cb, NULL);

    btrie_fini(&tr);
}


UNUSED static void
test2(void)
{
    uint64_t keys[] = {
        0xffffffffffffffff,
        0x7fffffffffffffff,
        0x3fffffffffffffff,
        0x1fffffffffffffff,
        0x0ffffffffffffffe,
        0x5a5a5a5a5a5a5a5a,
    };
    unsigned i;
    mnbtrie_t tr;
    mnbtrie_node_t *n;

    btrie_init(&tr);

    for (i = 0; i < countof(keys); ++i) {
        test_data_t *d;

        n = btrie_add_node(&tr, keys[i]);
        d = malloc(sizeof(test_data_t));
        d->key = keys[i];
        n->value = d;
    }

    //btrie_traverse(&tr, btrie_node_dump_cb, NULL);

    for (i = 0; i < countof(keys); ++i) {
        TRACE("querying: %016lx", (long)keys[i]);
        n = btrie_find_exact(&tr, keys[i]);
        //btrie_node_dump_cb(n, NULL);

        n = btrie_find_closest(&tr, keys[i] - 1, 1);
        btrie_node_dump_cb(n, NULL);
        n = btrie_find_closest(&tr, keys[i] + 1, 0);
        btrie_node_dump_cb(n, NULL);

        //n = btrie_find_closest(&tr, keys[i], 1);
        //btrie_node_dump_cb(n, NULL);
    }

    for (i = 0; i < countof(keys); ++i) {
        TRACE("removing: %016lx", (long)keys[i]);
        n = btrie_find_exact(&tr, keys[i]);
        if (n != NULL) {
            n->value = NULL;
            btrie_remove_node(&tr, n);
        }
    }

    //btrie_traverse(&tr, btrie_node_dump_cb, NULL);
    btrie_fini(&tr);
}


UNUSED static int
mydump(mnbtrie_node_t *node, UNUSED void *udata)
{
    btrie_node_dump(node);
    return 0;
}


UNUSED static void
test3(void)
{
    uintmax_t keys[] = {
        1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31
    };
    uintmax_t probe;
    unsigned i;
    mnbtrie_t tr;
    mnbtrie_node_t *n;

    btrie_init(&tr);
    for (i = 0; i < countof(keys); ++i) {
        UNUSED test_data_t *d;
        uintmax_t key;

        key = keys[i] + 1024;
        n = btrie_add_node(&tr, key);
#if USE_TEST_DATA
        d = malloc(sizeof(test_data_t));
        d->key = key;
        n->value = d;
#else
        n->value = (void *)key;
#endif
    }

    //btrie_traverse(&tr, mydump, NULL);

    for (probe = 0; probe < 64; ++probe) {
        //if (probe != 2) {
        //    continue;
        //}
        n = btrie_find_closest(&tr, probe, 0);
        TRACEC("probe %02jx ", probe);
#if USE_TEST_DATA
        btrie_node_dump_cb(n, NULL);
#else
        btrie_node_dump(n);
#endif
    }

    TRACE();

    for (probe = 0; probe < 64; ++probe) {
        //if (probe != 2) {
        //    continue;
        //}
        n = btrie_find_closest(&tr, probe, 1);
        TRACEC("probe %02jx ", probe);
#if USE_TEST_DATA
        btrie_node_dump_cb(n, NULL);
#else
        btrie_node_dump(n);
#endif
    }

    while ((n = BTRIE_MIN(&tr)) != NULL) {
        TRACE("reoving %p", n->value);
        n->value = NULL;
        btrie_remove_node(&tr, n);
    }
    btrie_fini(&tr);
}


static int
mycb (mnbtrie_node_t *node, UNUSED void *udata)
{
    union {
        uint64_t i;
        double d;
    } u;
    if (node != NULL) {
        u.i = (uint64_t)(uintptr_t)node->value;
        if (u.d != 0.0) {
            TRACE("d %lf (%016lx)", u.d, (unsigned long)u.i);
        }
    }
    return 0;
}


static void
test4 (void)
{
    mnbtrie_t tr;
    mnbtrie_node_t *n;
    union {
        uint64_t i;
        double d;
    } u;

    srandomdev();

    btrie_init(&tr);

    for (int i = 0; i < 20; ++i) {
        long l = random() % 1000000;
        u.d = ((double)l) / 1000;
        n = btrie_add_node(&tr, u.i);
        n->value = (void *)(uintptr_t)u.i;

    }

    btrie_traverse(&tr, mycb, NULL);

    u.d = INFINITY;
    u.i = ULONG_MAX;
    TRACE(">>> %016lx", (unsigned long)u.i);
    while ((n = btrie_find_closest(&tr, u.i, 0)) != NULL) {
        (void)mycb(n, NULL);
        u.i = ((uintmax_t)(uintptr_t)n->value) - 1;
    }

    btrie_fini(&tr);
}

int
main(void)
{
    //test0();
    //test1();
    //test2();
    //test3();
    test4();
    return 0;
}
