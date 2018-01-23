#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "rb.h"
#include "mrkcommon/profile.h"
#include "mrkcommon/util.h"
#ifdef DO_MEMDEBUG
#include "mrkcommon/memdebug.h"
MEMDEBUG_DECLARE(testperfrb);
#endif

typedef struct _key_item {
    uint64_t key;
    struct _rb_node *n;
    uint64_t add_time;
    uint64_t find_time;
    uint64_t remove_time;
} key_item_t;

static key_item_t keys[1024 * 1024];

char *rbnodes;
struct rb tr = RB_INITIALIZER();

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

    rbnodes = malloc(sizeof(struct _rb_node) * countof(keys));

    for (i = 0; i < countof(keys); ++i) {
        keys[i].key = new_id_successive();
        //keys[i].key = new_id_random();
    }
}

static void
test0(void)
{
    const profile_t *p_add_node;
    const profile_t *p_find_node;
    const profile_t *p_remove_node;
    struct _rb_node *dup;

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

    RB_INIT(&tr);

    for (i = 0; i < countof(keys); ++i) {
        struct _rb_node *n;

        n = (struct _rb_node *) (rbnodes + i * sizeof(struct _rb_node));
        n->key = keys[i].key;
        profile_start(p_add_node);
        RB_INSERT(rb, &tr, n);
        keys[i].add_time = profile_stop(p_add_node);

        assert(n != NULL);
    }
    TRACE("add_node OK");

    for (i = 0; i < countof(keys); ++i) {
        struct _rb_node find;

        find.key = keys[i].key;
        profile_start(p_find_node);
        keys[i].n = RB_FIND(rb, &tr, &find);
        keys[i].find_time = profile_stop(p_find_node);

        assert(keys[i].n != NULL);
    }
    TRACE("find_node OK");

    dup = RB_MIN(rb, &tr);
    i = 0;
    while (dup != NULL) {
        struct _rb_node *tmp = RB_NEXT(rb, &tr, dup);
        profile_start(p_remove_node);
        RB_REMOVE(rb, &tr, dup);
        keys[i].remove_time = profile_stop(p_remove_node);
        dup = tmp;
        ++i;
    }

    TRACE("remove_node OK");

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
#ifdef DO_MEMDEBUG
    MEMDEBUG_REGISTER(testperfrb);
#endif

    profile_init_module();

    test0();

#ifdef DO_MEMDEBUG
    memdebug_print_stats();
#endif

    profile_fini_module();

    return 0;
}
