#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <mrkcommon/util.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/pbtrie.h>

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#ifndef HAVE_FLSL
#   ifdef __GNUC__
#       define flsl(v) (v ? (64 - __builtin_clzl(v)) : 0)
#   else
#       error "Could not find/define flsl."
#   endif
#endif

#include "diag.h"


static uint64_t
xmask(uint64_t prefix, uint64_t key)
{
    uint64_t x, xmask;
    int f;
    //int m;
    x = prefix ^ key;
    f = flsl(x);
    //TRACE("x=%016lx f=%d", x, f);
    if (f >= 64) {
        return 0;
    }

    //m = 64 - f;
    xmask = UINT64_MAX << f;
    return xmask;
}

UNUSED static void
test3(void)
{
    uint64_t i;
    struct {
        uint64_t prefix;
        uint64_t key;
        uint64_t xm;
    } pk[] = {
        {0x1234567800000000, 0x1234567800000000, 0xffffffffffffffff},
        {0x1234567800000000, 0x12345678f0000000, 0xffffffff00000000},
        {0x1234567800000000, 0x1234567900000000, 0xfffffffe00000000},
        {0x1234567800000000, 0x1234567800000001, 0xfffffffffffffffe},
        {0x1234567800000000, 0x1a34567800000001, 0xf000000000000000},
        {0x1234567800000000, 0x0234567800000001, 0xe000000000000000},
        {0x1234567800000000, 0x3234567800000000, 0xc000000000000000},
        {0x1234567800000000, 0x5234567800000000, 0x8000000000000000},
        {0x1234567800000000, 0x9234567800000000, 0x0000000000000000},
    };

#if 0
    for (i = 1; i < 123; ++i) {
        int f, m;

        f = flsl(i);
        m = 64 - f;
        TRACE("i=%ld flsl=%d mask=%016lx/%016lx",
              i,
              f,
              UINT64_MAX >> m,
              UINT64_MAX << f);
    }
#endif

    for (i = 0; i < countof(pk); ++i) {
        uint64_t xm;

        xm = xmask(pk[i].prefix, pk[i].key);
        //TRACE("%016lx %016lx %016lx", pk[i].prefix, pk[i].key, xm);
        assert(xm == pk[i].xm);
    }
}


static void
foo(mnpbtrie_t *tr, uint64_t i)
{
    mnpbtrie_node_t *node, *found;

    node = pbtrie_add_node(tr, i);
    found = pbtrie_find_exact(tr, i);
    assert(node == found);
    TRACE("node=%p found=%p", node, found);
}


UNUSED static int
mycb(mnpbtrie_node_t *node, UNUSED void *udata)
{
    TRACE("%016lx:%016lx", node->xmask, node->prefix);
    return 0;
}


static void
test2(void)
{
    mnpbtrie_t tr;
    uint64_t i;

    //srandom(time(NULL));
    srandom(1);

    pbtrie_init(&tr);

    for (i = 0; i < 128; ++i) {
        foo(&tr, random() << 32 | random());
    }

    //pbtrie_dump(&tr);

    //pbtrie_traverse(&tr, mycb, NULL);
    //pbtrie_reverse(&tr, mycb, NULL);

    pbtrie_fini(&tr);
}


int
main(void)
{
    test2();
    test3();
    return 0;
}
