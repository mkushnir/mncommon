#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <err.h>

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/trie.h"
#include "mrkcommon/profile.h"
#include "mrkcommon/util.h"
#include "mrkcommon/memdebug.h"
MEMDEBUG_DECLARE(testperftrie);

typedef struct _key_item {
    uint64_t key;
    trie_node_t *n;
    uint64_t add_time;
    uint64_t find_time;
    uint64_t remove_time;
} key_item_t;

static key_item_t keys[1024 * 1024];

static trie_t tr;

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
    }
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
    trie_init(&tr);

    for (i = 0; i < countof(keys); ++i) {
        trie_node_t *n;

        profile_start(p_add_node);
        n = trie_add_node(&tr, keys[i].key);
        n->value = (void *)(uintptr_t)(keys[i].key);
        keys[i].add_time = profile_stop(p_add_node);

        assert(n != NULL);
    }
    TRACE("add_node OK");

    for (i = 0; i < countof(keys); ++i) {

        profile_start(p_find_node);
        keys[i].n = trie_find_exact(&tr, keys[i].key);
        keys[i].find_time = profile_stop(p_find_node);

        assert(keys[i].n != NULL);
    }
    TRACE("find_node OK");

    for (i = 0; i < countof(keys); ++i) {

        profile_start(p_remove_node);
        trie_remove_node(&tr, keys[i].n);
        keys[i].n = NULL;
        keys[i].remove_time = profile_stop(p_remove_node);

        if ((i % 100000) == 0) {
            trie_cleanup(&tr);
        }
    }

    TRACE("remove_node OK");

    trie_fini(&tr);
    TRACE("trie_fini OK");
    profile_report();

    for (i = 0; i < countof(keys); ++i) {
        printf("%d %ld %ld %ld\n",
               i,
               keys[i].add_time,
               keys[i].find_time,
               keys[i].remove_time);
    }
}

int
main(void)
{
    MEMDEBUG_REGISTER(testperftrie);
#ifndef NDEBUG
    MEMDEBUG_REGISTER(trie);
#endif

    profile_init_module();

    test0();

    memdebug_print_stats();
    profile_fini_module();
    return 0;
}
