#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <err.h>

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/btrie.h"
#include "mrkcommon/util.h"
#ifdef USE_MEMDEBUG
#include "mrkcommon/memdebug.h"
MEMDEBUG_DECLARE(testtrieorder);
#endif

typedef struct _key_item {
    uint64_t key;
    btrie_node_t *n;
} key_item_t;

static key_item_t keys[10];

static btrie_t tr;

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
cb(btrie_node_t *node, UNUSED void *udata)
{
    if (node->value != NULL) {
        uint64_t key;

        key = (uintptr_t)(node->value);
        TRACE("key=%016lx", (long)key);
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

    btrie_init(&tr);

    for (i = 0; i < countof(keys); ++i) {
        btrie_node_t *n;

        n = btrie_add_node(&tr, keys[i].key);
        n->value = (void *)(uintptr_t)keys[i].key;
        TRACE("key=%016lx", (uintptr_t)n->value);

        assert(n != NULL);
    }
    TRACE("add_node OK");

    btrie_traverse(&tr, (btrie_traverser_t)cb, NULL);

    btrie_fini(&tr);
    TRACE("btrie_fini OK");
}

int
main(void)
{
#ifdef USE_MEMDEBUG
    MEMDEBUG_REGISTER(testtrieorder);
#endif
    test0();
    return 0;
}
