#include <stdlib.h>
#include "rb.h"


static int
_rb_cmp(struct _rb_node *a, struct _rb_node *b)
{
    return (int)(a->key - b->key);
}
RB_GENERATE(rb, _rb_node, link, _rb_cmp);
