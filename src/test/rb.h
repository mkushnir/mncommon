#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdint.h>
#ifdef HAVE_SYS_TREE_H
#include <sys/tree.h>
#else
#include <mrkcommon/freebsd/tree.h>
#endif

struct _rb_node {
    uintptr_t key;
    RB_ENTRY(_rb_node) link;
};

RB_HEAD(rb, _rb_node);
RB_PROTOTYPE(rb, _rb_node, link, _rb_cmp);
