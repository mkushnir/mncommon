#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"

#include "mrkcommon/dict.h"
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
    dict_t dict;
    dict_item_t *v1;
    void *v2;


    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }

    dict_init(&dict, 7, myhash, mycmp, myfini);

    dict_set_item(&dict, (void *)1, (void *)10);
    dict_traverse(&dict, mycb, NULL);

    dict_set_item(&dict, (void *)1, (void *)10);
    dict_set_item(&dict, (void *)1, (void *)10);
    dict_set_item(&dict, (void *)1, (void *)10);
    dict_set_item(&dict, (void *)1, (void *)10);
    dict_traverse(&dict, mycb, NULL);

    v1 = dict_get_item(&dict, (void *)1);
    TRACE("v1=%p", v1->value);

    v2 = dict_remove_item(&dict, (void *)1);
    dict_traverse(&dict, mycb, NULL);
    TRACE("v2=%p", v2);
    v2 = dict_get_item(&dict, (void *)1);
    TRACE("v2=%p", v2);

    for (i = 0; i < 21; ++i) {
        dict_set_item(&dict, (void *)(uintptr_t)i, (void *)(uintptr_t)i);
    }
    dict_traverse(&dict, mycb, NULL);

    dict_fini(&dict);
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
my_item_print(dict_item_t *dit, UNUSED void *udata)
{
    my_item_t *key;

    key = dit->key;
    TRACE("<h=%ld v=%ld>", key->hash, key->value);
    return 0;
}


static int
my_item_delete(dict_t *dict, dict_item_t *dit, void *udata)
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
        dict_delete_pair(dict, dit);
    } else {
        TRACE("keeping %ld ...", key->value);
    }
    return 0;
}


static void
test1(void)
{
    dict_t dict;

    dict_init(&dict, 3, (dict_hashfn_t)my_item_hash, (dict_item_comparator_t)my_item_cmp, (dict_item_finalizer_t)my_item_fini);
    dict_set_item(&dict, my_item_new(1,1), NULL);
    dict_set_item(&dict, my_item_new(1,2), NULL);
    dict_set_item(&dict, my_item_new(1,3), NULL);
    dict_set_item(&dict, my_item_new(1,4), NULL);
    dict_set_item(&dict, my_item_new(1,5), NULL);
    dict_set_item(&dict, my_item_new(1,6), NULL);
    dict_set_item(&dict, my_item_new(1,7), NULL);
    dict_set_item(&dict, my_item_new(1,8), NULL);
    dict_set_item(&dict, my_item_new(1,9), NULL);
    dict_set_item(&dict, my_item_new(1,10), NULL);
    dict_set_item(&dict, my_item_new(1,11), NULL);
    dict_set_item(&dict, my_item_new(1,12), NULL);
    dict_set_item(&dict, my_item_new(1,13), NULL);
    dict_set_item(&dict, my_item_new(1,14), NULL);
    dict_set_item(&dict, my_item_new(1,15), NULL);
    dict_set_item(&dict, my_item_new(2,80), NULL);
    dict_set_item(&dict, my_item_new(0,81), NULL);

    TRACE();
    dict_traverse_item(&dict, my_item_print, NULL);
    TRACE();
    dict_traverse_item(&dict, my_item_delete, (void *)5);
    TRACE();
    dict_traverse_item(&dict, my_item_delete, (void *)4);
    TRACE();
    dict_traverse_item(&dict, my_item_print, NULL);
    TRACE();
    dict_fini(&dict);
}


int
main(void)
{
    test0();
    test1();
    return 0;
}


