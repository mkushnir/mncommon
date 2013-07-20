#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <err.h>

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/trie.h"
#include "mrkcommon/util.h"
#include "mrkcommon/memdebug.h"
MEMDEBUG_DECLARE(testtrieorder);

typedef struct _key_item {
    uint64_t key;
    trie_node_t *n;
} key_item_t;

static key_item_t keys[10];

static trie_t tr;

UNUSED static uint64_t
new_id_random(void)
{
    return (((uint64_t)random()) << 32) | random();
}

UNUSED static uint64_t
new_id_successive(void)
{
    static uint64_t id = 0;
    return ++id;
}

static void
initialize_ids(void)
{
    unsigned i;

    for (i = 0; i < countof(keys); ++i) {
        keys[i].key = new_id_random();
    }
}

static int
cb(trie_node_t *node, UNUSED void *udata)
{
    if (node->value != NULL) {
        uint64_t key;

        key = (uintptr_t)(node->value);
        TRACE("key=%016lx", key);
    }
    return 0;
}


static void
test0(void)
{
    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    initialize_ids();

    trie_init(&tr);

    for (i = 0; i < countof(keys); ++i) {
        trie_node_t *n;

        n = trie_add_node(&tr, keys[i].key);
        n->value = (void *)(uintptr_t)keys[i].key;
        TRACE("key=%ld", (uintptr_t)n->value);

        assert(n != NULL);
    }
    TRACE("add_node OK");

    trie_traverse(&tr, (trie_traverser_t)cb, NULL);

    trie_fini(&tr);
    TRACE("trie_fini OK");
}

int
main(void)
{
    MEMDEBUG_REGISTER(testtrieorder);
    test0();
    return 0;
}
