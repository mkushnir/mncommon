#include <assert.h>
#include <stdlib.h>

#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/trie.h"

UNUSED static void
test0(void)
{
    //int values[] = {777};
    int values[] = {0, 1, 2, 4, 8, 9, 10, 11, 12};
    int values1[] = {0,1,2,3,4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1,2,3,4,4};
    //int values[] = {0x00, 0x200};
    //int values1[] = {0x400};
    unsigned i;
    trie_t tr;
    trie_node_t *n;

    trie_init(&tr);

    for (i = 0; i < countof(values); ++i) {
        test_data_t *d;

        n = trie_add_node(&tr, values[i]);
        d = malloc(sizeof(test_data_t));
        d->key = values[i];
        n->value = d;
    }

    //trie_traverse(&tr, trie_node_dump_cb, NULL);

    for (i = 0; i < countof(values1); ++i) {
        //TRACE("querying: %x", values1[i]);
        n = trie_find_exact(&tr, values1[i]);
        //trie_node_dump_cb(n, NULL);
        n = trie_find_closest(&tr, values1[i], 0);
        //trie_node_dump_cb(n, NULL);
        n = trie_find_closest(&tr, values1[i], 1);
        //trie_node_dump_cb(n, NULL);
    }

    for (i = 0; i < countof(values1); ++i) {
        //TRACE("removing: %d", values1[i]);
        n = trie_find_exact(&tr, values1[i]);
        if (n != NULL) {
            trie_remove_node(&tr, n);
        }
    }

    trie_fini(&tr);
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
    trie_t tr;
    trie_node_t *n;

    trie_init(&tr);

    //TRACE("test add");

    for (i = 0; i < countof(values); ++i) {
        n = trie_add_node(&tr, values[i]);
        if (n->value == NULL) {
            test_data_t *d;

            d = malloc(sizeof(test_data_t));
            d->key = values[i];
            n->value = d;
        }
        //trie_node_traverse(n, trie_node_dump_cb, NULL);
    }

    //trie_traverse(&tr, trie_node_dump_cb, NULL);

    //TRACE("test find");

    for (i = 0; i < countof(values1); ++i) {
        //TRACE("searching for key=%02lx", values1[i]);
        n = trie_find_exact(&tr, values1[i]);
        if (n == NULL) {
            //TRACE("N/A");
        } else {
            //trie_node_dump_cb(n, (void *)1);
        }
    }

    //TRACE("test remove");

    for (i = 0; i < countof(values2); ++i) {
        n = trie_find_exact(&tr, values2[i]);
        //TRACE("found %p for %ld", n, values2[i]);
        //trie_node_dump(n);
        if (n != NULL) {
            trie_remove_node(&tr, n);
            //TRACE("removed %ld OK", values2[i]);
        } else {
            //TRACE("removed %ld FAIL", values2[i]);
        }
    }

    for (i = 0; i < 202; ++i) {
        //TRACE("searching closest for key=%02lx", (uint64_t)i);
        n = trie_find_closest(&tr, i, 1);
        if (n == NULL) {
            //TRACE("N/A");
        } else {
            //trie_node_dump_cb(n, (void *)1);
        }
    }

    //trie_traverse(&tr, trie_node_dump_cb, NULL);

    trie_fini(&tr);
}

int
main(void)
{
    test0();
    test1();
    return 0;
}
