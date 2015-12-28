#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"

#include "mrkcommon/hash.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"

static uint64_t
myhash(UNUSED void *key)
{
    return (uint64_t)key;
}

static int
myfini(void *key, void *value)
{
    TRACE("key=%p value=%p", key, value);
    return 0;
}

static int
mycmp(void *a, void *b)
{
    return (int)((intptr_t)a - (intptr_t)b);
}

static int
mycb(void *a, void *b, UNUSED void *udata)
{
    TRACE("%p:%p", a, b);
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
    hash_t dict;
    hash_item_t *v1;
    void *v2;


    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }

    hash_init(&dict, 7, myhash, mycmp, myfini);

    hash_set_item(&dict, (void *)1, (void *)10);
    hash_traverse(&dict, mycb, NULL);

    hash_set_item(&dict, (void *)1, (void *)10);
    hash_set_item(&dict, (void *)1, (void *)10);
    hash_set_item(&dict, (void *)1, (void *)10);
    hash_set_item(&dict, (void *)1, (void *)10);
    hash_traverse(&dict, mycb, NULL);

    v1 = hash_get_item(&dict, (void *)1);
    TRACE("v1=%p", v1->value);

    v2 = hash_remove_item(&dict, (void *)1);
    hash_traverse(&dict, mycb, NULL);
    TRACE("v2=%p", v2);
    v2 = hash_get_item(&dict, (void *)1);
    TRACE("v2=%p", v2);

    for (i = 0; i < 21; ++i) {
        hash_set_item(&dict, (void *)(uintptr_t)i, (void *)(uintptr_t)i);
    }
    hash_traverse(&dict, mycb, NULL);

    hash_fini(&dict);
}


typedef struct _my_item {
    uint64_t hash;
    uint64_t value;
} my_item_t;


static my_item_t*
my_item_new(uint64_t hash, uint64_t value)
{
    my_item_t *res;
    res = malloc(sizeof(my_item_t));
    res->hash = hash;
    res->value = value;
    return res;
}

static void
my_item_destroy(my_item_t **it)
{
    if (*it != NULL) {
        free(*it);
        *it = NULL;
    }
}

static void
my_item_fini(my_item_t *key, my_item_t *value)
{
    my_item_destroy(&key);
    my_item_destroy(&value);
}

static uint64_t
my_item_hash(my_item_t *it)
{
    return it->hash;
}

static int
my_item_cmp(my_item_t *a , my_item_t *b)
{
    return (int)((int64_t)a->value - (int64_t)b->value);
}

static int
my_item_print(UNUSED hash_t *dict, hash_item_t *dit, UNUSED void *udata)
{
    my_item_t *key;

    key = dit->key;
    TRACE("<h=%ld v=%ld>", key->hash, key->value);
    return 0;
}


static int
my_item_delete(hash_t *dict, hash_item_t *dit, void *udata)
{
    my_item_t *key;
    union {
        void *v;
        int i;
    } u;

    u.v = udata;

    key = dit->key;
    if (key->value % u.i == 0) {
        TRACE("deleting %ld ...", key->value);
        hash_delete_pair(dict, dit);
    } else {
        TRACE("keeping %ld ...", key->value);
    }
    return 0;
}


static void
test1(void)
{
    hash_t dict;

    hash_init(&dict,
              3,
              (hash_hashfn_t)my_item_hash,
              (hash_item_comparator_t)my_item_cmp,
              (hash_item_finalizer_t)my_item_fini);

    hash_set_item(&dict, my_item_new(1,1), NULL);
    hash_set_item(&dict, my_item_new(1,2), NULL);
    hash_set_item(&dict, my_item_new(1,3), NULL);
    hash_set_item(&dict, my_item_new(1,4), NULL);
    hash_set_item(&dict, my_item_new(1,5), NULL);
    hash_set_item(&dict, my_item_new(1,6), NULL);
    hash_set_item(&dict, my_item_new(1,7), NULL);
    hash_set_item(&dict, my_item_new(1,8), NULL);
    hash_set_item(&dict, my_item_new(1,9), NULL);
    hash_set_item(&dict, my_item_new(1,10), NULL);
    hash_set_item(&dict, my_item_new(1,11), NULL);
    hash_set_item(&dict, my_item_new(1,12), NULL);
    hash_set_item(&dict, my_item_new(1,13), NULL);
    hash_set_item(&dict, my_item_new(1,14), NULL);
    hash_set_item(&dict, my_item_new(1,15), NULL);
    hash_set_item(&dict, my_item_new(2,80), NULL);
    hash_set_item(&dict, my_item_new(0,81), NULL);

    TRACE("elnum=%ld", hash_get_elnum(&dict));
    hash_dump_stats(&dict);
    hash_traverse_item(&dict, my_item_print, NULL);

    TRACE("elnum=%ld", hash_get_elnum(&dict));
    hash_dump_stats(&dict);
    hash_traverse_item(&dict, my_item_delete, (void *)5);

    TRACE("elnum=%ld", hash_get_elnum(&dict));
    hash_dump_stats(&dict);
    hash_traverse_item(&dict, my_item_delete, (void *)4);

    TRACE("elnum=%ld", hash_get_elnum(&dict));
    hash_dump_stats(&dict);
    hash_traverse_item(&dict, my_item_print, NULL);

    hash_fini(&dict);
}


static void
test2(void)
{
    int i;
    hash_t dict;

    hash_init(&dict,
              3,
              (hash_hashfn_t)my_item_hash,
              (hash_item_comparator_t)my_item_cmp,
              (hash_item_finalizer_t)my_item_fini);

    for (i = 0; i < 10; ++i) {
        hash_set_item(&dict, my_item_new(i, i), NULL);
    }
    TRACE("elnum=%ld", hash_get_elnum(&dict));
    hash_dump_stats(&dict);
    hash_rehash(&dict, 7);
    TRACE("elnum=%ld", hash_get_elnum(&dict));
    hash_dump_stats(&dict);
    hash_fini(&dict);
}

int
main(void)
{
    test0();
    test1();
    test2();
    return 0;
}


