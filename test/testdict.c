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
    dict_item_t *v1, *v2;


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

    v2 = dict_remove_key(&dict, (void *)1);
    dict_traverse(&dict, mycb, NULL);
    TRACE("v2=%p", v2->value);
    v2 = dict_get_item(&dict, (void *)1);
    TRACE("v2=%p", v2);

    for (i = 0; i < 21; ++i) {
        dict_set_item(&dict, (void *)(uintptr_t)i, (void *)(uintptr_t)i);
    }
    dict_traverse(&dict, mycb, NULL);

    dict_fini(&dict);
}

int
main(void)
{
    test0();
    return 0;
}


