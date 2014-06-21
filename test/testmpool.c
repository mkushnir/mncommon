#include <assert.h>
#include <stdlib.h>

#include "unittest.h"

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/mpool.h>

mpool_ctx_t mpool;

UNUSED static void
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

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }
}


UNUSED static void *
run_mpool_malloc(void)
{
    void *p = NULL;
    int i;

    for (i = 0; i < 1024 * 1024; ++i) {
        size_t sz;

        sz = random() % 4096;

        if (sz > 0) {
            p = mpool_malloc(&mpool, sz);
            //if (p == NULL) {
            //    break;
            //}
        }
    }
    //TRACE("i=%d p=%p", i, p);
    return p;
}


UNUSED static void *
run_std_malloc(void)
{
    void *p = NULL;
    int i;

    for (i = 0; i < 1024 * 1024; ++i) {
        size_t sz;


        sz = random() % 4096;

        if (sz > 0) {
            p = malloc(sz);
            //if (p == NULL) {
            //    break;
            //}
        }
    }
    //TRACE("i=%d p=%p", i, p);
    return p;
}


static void
test1(void)
{
    void *p;

#define f run_mpool_malloc
    mpool_ctx_reset(&mpool);
    p = f();
    TRACE("p=%p", p);
    mpool_ctx_reset(&mpool);
    p = f();
    TRACE("p=%p", p);
    mpool_ctx_reset(&mpool);
    p = f();
    TRACE("p=%p", p);
}


int
main(void)
{
    mpool_ctx_init(&mpool, 1024*1024);
    //test0();
    test1();
    mpool_ctx_fini(&mpool);
    return 0;
}

