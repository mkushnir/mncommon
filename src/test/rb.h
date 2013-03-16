#include <stdint.h>
#include <sys/tree.h>

struct _rb_node {
    uintptr_t key;
    RB_ENTRY(_rb_node) link;
};

RB_HEAD(rb, _rb_node);
RB_PROTOTYPE(rb, _rb_node, link, _rb_cmp);
