#include <stdlib.h>
#include <time.h>

#include "mncommon/dumpm.h"
#include "mncommon/util.h"
#include "mncommon/profile.h"
#include "mncommon/rbt.h"
#include "mncommon/btrie.h"
#include "rb.h"

static unsigned niter = 10;
static unsigned nelem = 1024 * 1024;

struct rb foo = RB_INITIALIZER();
mnrbt_t rbt;
mnbtrie_t tr;

void *rbnodes;
void *trienodes;
void *rbtnodes;
intmax_t *keys;

enum {
    RBT_TEST_PROF_RB,
    RBT_TEST_PROF_RBT,
    RBT_TEST_PROF_TRIE,
    RBT_TEST_PROF_RB_INSERT,
    RBT_TEST_PROF_RBT_INSERT,
    RBT_TEST_PROF_BTRIE_INSERT,
    RBT_TEST_PROF_RB_FIND,
    RBT_TEST_PROF_RBT_FIND,
    RBT_TEST_PROF_BTRIE_FIND,
    RBT_TEST_PROF_RB_REMOVE,
    RBT_TEST_PROF_RBT_REMOVE,
    RBT_TEST_PROF_BTRIE_REMOVE,
};

static struct {
    const profile_t *profile;
    const char *name;
} profiles[] = {
    {NULL, "rb"},
    {NULL, "rbt"},
    {NULL, "trie"},
    {NULL, "rb_insert"},
    {NULL, "rbt_insert"},
    {NULL, "btrie_insert"},
    {NULL, "rb_find"},
    {NULL, "rbt_find"},
    {NULL, "btrie_find"},
    {NULL, "rb_remove"},
    {NULL, "rbt_remove"},
    {NULL, "btrie_remove"},
};

#ifdef PROFILE_START
#undef PROFILE_START
#endif
#define PROFILE_START(p) profile_start(profiles[p].profile)
#ifdef PROFILE_STOP
#undef PROFILE_STOP
#endif
#define PROFILE_STOP(p) profile_stop(profiles[p].profile)

UNUSED static void
test_rb(void)
{
    unsigned i;
    struct _rb_node UNUSED *res, UNUSED *dup;
    struct _rb_node *n;
    struct _rb_node find;

    RB_INIT(&foo);

    PROFILE_START(RBT_TEST_PROF_RB_INSERT);
    for (i = 0; i < nelem; ++i) {
        n = (struct _rb_node *) (rbnodes + i * sizeof(struct _rb_node));
        n->key = *(keys + i);
        res = RB_INSERT(rb, &foo, n);
    }
    PROFILE_STOP(RBT_TEST_PROF_RB_INSERT);

    PROFILE_START(RBT_TEST_PROF_RB_FIND);
    for (i = 0; i < nelem; ++i) {
        find.key = *(keys + i);
        res = RB_FIND(rb, &foo, &find);
    }
    PROFILE_STOP(RBT_TEST_PROF_RB_FIND);

    dup = RB_MIN(rb, &foo);
    PROFILE_START(RBT_TEST_PROF_RB_REMOVE);
    while (dup != NULL) {
        struct _rb_node *tmp = RB_NEXT(rb, &foo, dup);
        RB_REMOVE(rb, &foo, dup);
        dup = tmp;
    }
    PROFILE_STOP(RBT_TEST_PROF_RB_REMOVE);
}

UNUSED static void
test_trie(void)
{
    unsigned i;
    mnbtrie_t tr;
    intmax_t *n;
    mnbtrie_node_t *nn;

    btrie_init(&tr);

    PROFILE_START(RBT_TEST_PROF_BTRIE_INSERT);
    for (i = 0; i < nelem; ++i) {
        n = (intmax_t *) (trienodes + i * sizeof(intmax_t));
        *n = *(keys + i);
        nn = btrie_add_node(&tr, *n);
        nn->value = (void *)*n;
    }
    PROFILE_STOP(RBT_TEST_PROF_BTRIE_INSERT);

    PROFILE_START(RBT_TEST_PROF_BTRIE_FIND);
    for (i = 0; i < nelem; ++i) {
        n = (intmax_t *) (trienodes + i * sizeof(intmax_t));
        *n = *(keys + i);
        nn = btrie_find_exact(&tr, *n);
    }
    PROFILE_STOP(RBT_TEST_PROF_BTRIE_FIND);

    PROFILE_START(RBT_TEST_PROF_BTRIE_REMOVE);
    for (i = 0; i < nelem; ++i) {
        n = (intmax_t *) (trienodes + i * sizeof(intmax_t));
        *n = *(keys + i);
        nn = btrie_find_exact(&tr, *n);
        nn->value = NULL;
        btrie_remove_node(&tr, nn);
    }
    PROFILE_STOP(RBT_TEST_PROF_BTRIE_REMOVE);
}

UNUSED static void
test_rbt(void)
{
    unsigned i;
    UNUSED int res;
    mnrbt_node_t *dup;
    mnrbt_node_t *n;

    rbt_init(&rbt);

    PROFILE_START(RBT_TEST_PROF_RBT_INSERT);
    for (i = 0; i < nelem; ++i) {
        n = (mnrbt_node_t *) (rbtnodes + i * sizeof(mnrbt_node_t));
        rbt_node_init(n, *(keys + i), NULL);
        //n->key = *(keys + i);
        res = rbt_insert(&rbt, n, &dup);
        //TRACE("i=%d key=%d dup=%p res=%d", i, i, dup, res);
    }
    PROFILE_STOP(RBT_TEST_PROF_RBT_INSERT);
//    rbt_dump_tree(&rbt);

//    rbt_remove_key(&rbt, 9);
//    rbt_dump_tree(&rbt);
//
//    rbt_remove_key(&rbt, 8);
//    rbt_dump_tree(&rbt);
//
//    rbt_remove_key(&rbt, 7);
//    rbt_dump_tree(&rbt);
//
//    rbt_remove_key(&rbt, 5);
//    rbt_dump_tree(&rbt);
//
//    rbt_remove_key(&rbt, 3);
//    rbt_dump_tree(&rbt);

//    rbt_remove_key(&rbt, 7);
//    rbt_dump_tree(&rbt);
//    rbt_remove_key(&rbt, 3);
//    rbt_dump_tree(&rbt);
//    rbt_remove_key(&rbt, 1);
//    rbt_dump_tree(&rbt);
//    rbt_remove_key(&rbt, 0);
//    rbt_dump_tree(&rbt);

//    /* delete red nodes */
//    rbt_remove_key(&rbt, 0x1d);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x1b);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x1a);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x1c);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x17);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x16);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x19);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x18);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x0f);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x0e);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x09);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x08);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x0a);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x13);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x12);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x15);
//    rbt_dump_list(&rbt);
//    rbt_remove_key(&rbt, 0x14);
//    rbt_dump_list(&rbt);

//    /* enumerate */
//    dup = rbt.head;
//    while (dup != NULL) {
//        rbt_node_dump_list(dup);
//        dup = dup->next;
//    }
//    dup = rbt.tail;
//    while (dup != NULL) {
//        rbt_node_dump_list(dup);
//        dup = dup->prev;
//    }

//    TRACE();
//    dup = rbt.head;
//    while (dup != NULL) {
//        rbt_remove_node(&rbt, dup);
//        //rbt_dump_list(&rbt);
//        rbt_dump_tree(&rbt);
//        dup = rbt.head;
//        //sleep(1);
//    }
//    mnrbt_node_t *n = malloc(sizeof(mnrbt_node_t));
//    rbt_node_init(n, 10, (void *)123);
//    rbt_insert(&rbt, n, &dup);
//    TRACE("n=%p dup=%p", n, dup);
//    rbt_node_dump_list(n);
//    if (dup != NULL) {
//        rbt_node_dump_list(dup);
//    }

    PROFILE_START(RBT_TEST_PROF_RBT_FIND);
    for (i = 0; i < nelem; ++i) {
        rbt_find(&rbt, *(keys + i), &dup);
    }
    PROFILE_STOP(RBT_TEST_PROF_RBT_FIND);

    rbt_find(&rbt, rbt.head->key, &dup);
    PROFILE_START(RBT_TEST_PROF_RBT_REMOVE);
    while (dup != NULL) {
        mnrbt_node_t *tmp = dup->next;
        rbt_remove_node(&rbt, dup);
        dup = tmp;
    }
    PROFILE_STOP(RBT_TEST_PROF_RBT_REMOVE);

    rbt_fini(&rbt);
}

int
main(UNUSED int argc, UNUSED char *argv[])
{
    unsigned i;

    profile_init_module();
    for (i = 0; i < countof(profiles); ++i) {
        profiles[i].profile = profile_register(profiles[i].name);
    }

    rbnodes = malloc(sizeof(struct _rb_node) * nelem);
    trienodes = malloc(sizeof(intmax_t) * nelem);
    rbtnodes = malloc(sizeof(mnrbt_node_t) * nelem);
    keys = malloc(sizeof(intmax_t) * nelem);

    srandom(time(NULL));
    for (i = 0; i < nelem; ++i) {
        //*(keys + i) = random() + 1;
        *(keys + i) = i + 1;
    }

    PROFILE_START(RBT_TEST_PROF_RB);
    for (i = 0; i < niter; ++i) {
        //TRACE("test_rb %d", i);
        test_rb();
    }
    PROFILE_STOP(RBT_TEST_PROF_RB);

    PROFILE_START(RBT_TEST_PROF_RBT);
    for (i = 0; i < niter; ++i) {
        //TRACE("test_rbt %d", i);
        test_rbt();
    }
    PROFILE_STOP(RBT_TEST_PROF_RBT);

    PROFILE_START(RBT_TEST_PROF_TRIE);
    for (i = 0; i < niter; ++i) {
        //TRACE("test_trie %d", i);
        test_trie();
    }
    PROFILE_STOP(RBT_TEST_PROF_TRIE);

    profile_report_sec();
    profile_fini_module();
    return 0;
}

