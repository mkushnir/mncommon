#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "rb.h"

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/array.h"
#include "mrkcommon/list.h"

typedef struct _key_item {
    uint64_t key;
    struct _rb_node *n;
} key_item_t;

static key_item_t keys[10];

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
    static uint64_t id = 0;
    return ++id;
}

static void
initialize_ids(void)
{
    unsigned i;

    rbnodes = malloc(sizeof(struct _rb_node) * countof(keys));

    for (i = 0; i < countof(keys); ++i) {
        keys[i].key = new_id_random();
    }
}

static void
test0(void)
{
    struct _rb_node *dup;

    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    initialize_ids();

    RB_INIT(&tr);

    for (i = 0; i < countof(keys); ++i) {
        struct _rb_node *n;

        n = (struct _rb_node *) (rbnodes + i * sizeof(struct _rb_node));
        n->key = keys[i].key;
        TRACE("key=%ld", n->key);
        RB_INSERT(rb, &tr, n);

        assert(n != NULL);
    }
    TRACE("add_node OK");

    dup = RB_MIN(rb, &tr);
    i = 0;
    while (dup != NULL) {
        struct _rb_node *tmp;

        TRACE("key=%016lx", dup->key);

        tmp = RB_NEXT(rb, &tr, dup);
        dup = tmp;
        ++i;
    }
    dup = RB_MIN(rb, &tr);
    i = 0;
    while (dup != NULL) {
        struct _rb_node *tmp;

        tmp = RB_NEXT(rb, &tr, dup);
        RB_REMOVE(rb, &tr, dup);
        dup = tmp;
        ++i;
    }
}

int
main(void)
{
    test0();
    return 0;
}
