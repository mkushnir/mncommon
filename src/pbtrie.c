#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h>

#include <mncommon/malloc.h>
#include <mncommon/dumpm.h>
#include <mncommon/util.h>
#include <mncommon/pbtrie.h>

#ifndef HAVE_FLSL
#   ifdef __GCC__
#       define flsl(v) (v ? (64 - __builtin_clzl(v)) : 0)
#   else
#       error "Could not find/define flsl."
#   endif
#endif

#include "diag.h"


static void
pbtrie_node_dump_level(mnpbtrie_node_t *node, int level)
{
    if (node->child[1] != NULL) {
        pbtrie_node_dump_level(node->child[1], level + 1);
    }

    if (node->parent != NULL && node->parent->child[0] == node) {
        LTRACE(level, "%016lx:%016lx", (unsigned long)node->xmask, (unsigned long)node->prefix);

    } else {
        LTRACE(level, FRED("%016lx:%016lx"), (unsigned long)node->xmask, (unsigned long)node->prefix);
    }

    if (node->child[0] != NULL) {
        pbtrie_node_dump_level(node->child[0], level + 1);
    }
}


static void
pbtrie_node_init(mnpbtrie_node_t *node)
{
    node->xmask = UINT64_MAX;
    node->prefix = 0;
    node->value = NULL;
    node->parent = NULL;
    node->child[0] = NULL;
    node->child[1] = NULL;
}


mnpbtrie_node_t *
pbtrie_node_new(void)
{
    mnpbtrie_node_t *res;

    if (MNUNLIKELY((res = malloc(sizeof(mnpbtrie_node_t))) == NULL)) {
        FAIL("malloc");
    }
    pbtrie_node_init(res);
    return res;
}


static void
pbtrie_node_destroy(mnpbtrie_node_t **node)
{
    if (*node != NULL) {
        pbtrie_node_destroy(&(*node)->child[0]);
        pbtrie_node_destroy(&(*node)->child[1]);
        free(*node);
        *node = NULL;
    }
}

mnpbtrie_node_t *
pbtrie_node_min(mnpbtrie_node_t *node)
{
    while (node->child[0] != NULL) {
        node = node->child[0];
    }
    return node;
}


mnpbtrie_node_t *
pbtrie_node_max(mnpbtrie_node_t *node)
{
    while (node->child[1] != NULL) {
        node = node->child[1];
    }
    return node;
}


mnpbtrie_node_t *
pbtrie_node_next(mnpbtrie_node_t *node)
{
    if (node->child[1] != NULL) {
        node = pbtrie_node_min(node->child[1]);
    } else {
        mnpbtrie_node_t *parent;

        for (parent = node->parent;
             parent != NULL;
             parent = node->parent) {
            if (parent->child[1] != NULL && parent->child[1] != node) {
                node = pbtrie_node_min(parent->child[1]);
                break;
            } else {
                node = parent;
            }
        }
    }
    return node;
}


mnpbtrie_node_t *
pbtrie_node_prev(mnpbtrie_node_t *node)
{
    if (node->child[0] != NULL) {
        node = pbtrie_node_max(node->child[0]);
    } else {
        mnpbtrie_node_t *parent;

        for (parent = node->parent;
             parent != NULL;
             parent = node->parent) {
            if (parent->child[0] != NULL && parent->child[0] != node) {
                node = pbtrie_node_max(parent->child[0]);
                break;
            } else {
                node = parent;
            }
        }
    }
    return node;
}


int
pbtrie_traverse(mnpbtrie_t *trie, mnpbtrie_traverser_t cb, void *udata)
{
    int res = 0;
    mnpbtrie_node_t *node;

    for (node = trie->head;
         node != trie->tail;
         node = pbtrie_node_next(node)) {
        if ((res = cb(node, udata)) != 0) {
            break;
        }
    }
    if (res == 0) {
        res = cb(node, udata);
    }
    return res;
}


int
pbtrie_reverse(mnpbtrie_t *trie, mnpbtrie_traverser_t cb, void *udata)
{
    int res = 0;
    mnpbtrie_node_t *node;

    for (node = trie->tail;
         node != trie->head;
         node = pbtrie_node_prev(node)) {
        if ((res = cb(node, udata)) != 0) {
            break;
        }
    }
    if (res == 0) {
        res = cb(node, udata);
    }
    return res;
}


static uint64_t
xmask(uint64_t prefix, uint64_t key)
{
    uint64_t x, xmask;
    int f;
    x = prefix ^ key;
    f = flsl(x);
    if (f >= 64) {
        return 0;
    }
    xmask = UINT64_MAX << f;
    return xmask;
}


static int
childsel(uint64_t mask, uint64_t parent, uint64_t child)
{
    int64_t m;

    m = (int64_t)mask;
    return (child & (m >> 1)) <= parent ? 0 : 1;
    //return (child & (0x8000000000000000 | (mask >> 1))) <= parent ? 0 : 1;
}


mnpbtrie_node_t *
pbtrie_node_find_at(mnpbtrie_node_t *node, uint64_t key)
{
    while (node != NULL /* || true */) {
        uint64_t xm0, xm1;

        if (node->child[0] != NULL) {
            xm0 = xmask(node->child[0]->prefix, key);
            xm0 = MIN(xm0, node->child[0]->xmask);
        } else {
            xm0 = 0;
        }

        if (node->child[1] != NULL) {
            xm1 = xmask(node->child[1]->prefix, key);
            xm1 = MIN(xm1, node->child[1]->xmask);
        } else {
            xm1 = 0;
        }

        if (xm0 > xm1) {
            node = node->child[0];
            if (node->xmask > xm0) {
                break;
            }
        } else if (xm1 > xm0) {
            node = node->child[1];
            if (node->xmask > xm1) {
                break;
            }
        } else {
            assert(node->child[0] == NULL && node->child[1] == NULL);
            break;
        }
    }
    return node;
}


static mnpbtrie_node_t *
pbtrie_node_split(mnpbtrie_node_t *split,
                  uint64_t key)
{
    uint64_t xm;
    mnpbtrie_node_t *new_parent, *old_parent;
    int idx;

    xm = xmask(split->prefix, key);
    assert(xm < split->xmask);
    //TRACE("split:");
    //pbtrie_node_dump_level(split, 0);
    //TRACE("key=%016lx xm=%016lx", key, xm);

    old_parent = split->parent;
    /*
     * split can never be tr->root
     */
    assert(old_parent != NULL);
    new_parent = pbtrie_node_new();
    new_parent->prefix = split->prefix & xm;
    new_parent->xmask = xm;
    new_parent->parent = old_parent;
    if (old_parent->child[0] == split) {
        old_parent->child[0] = new_parent;
    } else {
        assert(old_parent->child[1] == split);
        old_parent->child[1] = new_parent;
    }
    //TRACE("new_paren=%016lx split=%016lx", new_parent->prefix, split->prefix);
    idx = childsel(new_parent->xmask, new_parent->prefix, split->prefix);
    new_parent->child[idx] = split;
    split->parent = new_parent;
    return new_parent;
}


mnpbtrie_node_t *
pbtrie_add_node(mnpbtrie_t *tr, uint64_t key)
{
    mnpbtrie_node_t *node;
    mnpbtrie_node_t *split, *new_parent;
    int idx;

    //TRACE("key=%016lx", key);
    split = pbtrie_node_find_at(tr->root, key);
    new_parent = pbtrie_node_split(split, key);

    node = pbtrie_node_new();
    node->parent = new_parent;
    node->prefix = key;

    idx = childsel(new_parent->xmask, new_parent->prefix, key);

    //TRACE("idx=%d new_parent:", idx);
    //pbtrie_node_dump_level(new_parent, 0);
    assert(new_parent->child[idx] == NULL);
    new_parent->child[idx] = node;

    return node;
}


mnpbtrie_node_t *
pbtrie_find_exact(mnpbtrie_t *tr, uint64_t key)
{
    return pbtrie_node_find_at(tr->root, key);
}


int
pbtrie_remove_node(UNUSED mnpbtrie_t *tr, mnpbtrie_node_t *node)
{
    if (node->parent != NULL) {
        if (node->parent->child[0] == node) {
            pbtrie_node_destroy(&node->parent->child[0]);
        } else {
            assert(node->parent->child[1] == node);
            pbtrie_node_destroy(&node->parent->child[1]);
        }
    } else {
        pbtrie_node_destroy(&node);
    }
    return 0;
}


void
pbtrie_dump(mnpbtrie_t *tr)
{
    pbtrie_node_dump_level(tr->root, 1);
}


void
pbtrie_init(mnpbtrie_t *tr)
{
    tr->root = pbtrie_node_new();
    tr->root->xmask = 0;
    tr->root->prefix = 0;
    tr->head = pbtrie_node_new();
    tr->root->child[0] = tr->head;
    tr->head->parent = tr->root;
    tr->tail = pbtrie_node_new();
    tr->tail->prefix = UINT64_MAX;
    tr->root->child[1] = tr->tail;
    tr->tail->parent = tr->root;
    tr->volume = 0;
    tr->nvals = 0;
}


void
pbtrie_fini(mnpbtrie_t *tr)
{
    mnpbtrie_node_t *node;

    for (node = tr->head;
         node != NULL;
         node = pbtrie_node_next(node)) {
        break;
    }
}
