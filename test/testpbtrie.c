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
#       define flsl(v) (64 - __builtin_clzl(v))
#   else
#       error "Could not find/define flsl."
#   endif
#endif

#include "diag.h"


void
pbtrie_node_dump(pbtrie_node_t *node, int indent)
{
    int i;
    char fmt[32];

    snprintf(fmt, sizeof(fmt), "%%%dhhd ", indent);
    for (i = 0; i < 64 && node->prefix[i] > 0; ++i) {
        fprintf(stderr, fmt, node->prefix[i]);
    }
    TRACEC("\n");
    if (node->child[0] != NULL) {
        pbtrie_node_dump(node->child[0], indent + 1);
    }
    if (node->child[1] != NULL) {
        pbtrie_node_dump(node->child[1], indent + 1);
    }
}

void
pbtrie_node_init(pbtrie_node_t *node)
{
    node->value = NULL;
    memset(node->prefix, 0, sizeof(node->prefix));
    node->child[0] = NULL;
    node->child[1] = NULL;
}

static void
foo(uintmax_t i)
{
    int idx;
    pbtrie_node_t node;

    pbtrie_node_init(&node);
    //pbtrie_node_dump(&node, 0);

    TRACEN("i=%016lx ", i);
    //for (idx = 64 - flsl(i);
    //     i > 0;
    //     i <<= (idx + 1), idx = 64 - flsl(i)) {
    //    TRACE("i=%016lx idx=%d, ii=%016lx", i, idx, i << (idx + 1));
    //}
    for (idx = flsl(i);
         i != 0;
         i <<= 1, idx = flsl(i)) {
        i <<= (64 - idx);
        //TRACE("i=%016lx shift=%d", i, 65 - idx);
        TRACEC("%d ", 65 - idx);
    }
    TRACEC("\n");

    //while (i != 0) {
    //    idx = flsl(i);
    //    i <<= (64 - idx);
    //    TRACE("i=%016lx shift=%d", i, 65 - idx);
    //    i <<= 1;
    //}
}


int
main(void)
{
    uintmax_t i;

    srandom(time(NULL));
    for (i = 0; i < 16; ++i) {
        foo(random() << 32 | random());
    }
    return 0;
}
