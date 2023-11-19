#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h>

#define DUMPM_INDENT_SIZE 1
#include <mncommon/malloc.h>
#include <mncommon/dumpm.h>
#include <mncommon/mpool.h>
#include <mncommon/util.h>
#include <mncommon/btrie.h>

#ifndef HAVE_FLSL
#   ifdef __GCC__
#       define flsl(v) (v ? (TREE_DEPTH - __builtin_clzl(v)) : 0)
#   else
#       error "Could not find/define flsl."
#   endif
#endif

#include "diag.h"


#define _malloc(sz) mpool_malloc(mpool, (sz))
#define _free(ptr) mpool_free(mpool, (ptr))
#define _btrie_node_cleanup(tr, node) btrie_node_cleanup_mpool(mpool, (tr), (node))
#define _cleanup_orphans(tr, node) cleanup_orphans_mpool(mpool, (tr), (node))
#define _btrie_node_fini(tr, node) btrie_node_fini_mpool(mpool, (tr), (node))


#define MNBTRIE_CHILDSEL(node, key) (((key) >> (node->idx - 1)) & 0x01)
#define MNBTRIE_NODESEL(node, key) (((key) >> node->idx) & 0x01)
#define MNBTRIE_NODESEL0(node) (node->digit >> CHILD_SELECTOR_SHIFT)


static int
mydump(mnbtrie_node_t *node, UNUSED void *udata)
{
    btrie_node_dump(node);
    return 0;
}


void
btrie_node_dump(mnbtrie_node_t *n)
{
    if (n == NULL) {
        TRACEC("NULL\n");
    } else {
        // \033[01;31m%02lx\033[00m
        TRACEC("[%p] i:%02hhd d:%04hx p:%p l:%p r:%p v:%p\n",
              n, n->idx, n->digit, n->parent, n->child[0], n->child[1], n->value);
    }
}


int
btrie_node_dump_cb(mnbtrie_node_t *n, void *arg)
{
    unsigned indent, sel;
    int flags = (intptr_t)arg;
    test_data_t *d;

    if (n == NULL) {
        TRACEC("NULL\n");
        return 0;
    }

    d = (test_data_t *)n->value;

    if (n->child[0] == NULL && n->child[1] == NULL && n->value == NULL) {
        return 0;
    }
    sel = n->digit & (1u << CHILD_SELECTOR_SHIFT);
    assert((sel == 0) || (sel == 1));
    indent = n->digit & ~(1u << CHILD_SELECTOR_SHIFT);
    if (d == NULL) {
        if (flags) {
            TRACE("key=");
            //TRACE("[n=%p value=%p]", n, d);
        } else {
            if (sel) {
                LTRACE(indent + 1, FRED("key="));
            } else {
                LTRACE(indent + 1, "key=");
            }
            //LTRACE(indent + 1, "[n=%p]", n);
        }
        //TRACE("n=%p n->value=%p", n, n->value);
        return 0;
    }
    if (flags) {
        TRACE("key=%02lx", (unsigned long)d->key);
        //TRACE("n=%p value=%p key=%02lx", n, d, d->key);
    } else {
        if (sel) {
            LTRACE(indent + 1, FRED("key=%02lx"), (unsigned long)d->key);
        } else {
            LTRACE(indent + 1, "key=%02lx", (unsigned long)d->key);
        }
        //LTRACE(indent + 1, "n=%p value=%p key=%02lx", n, d, d->key);
    }
    return 0;
}


static void
btrie_node_init(mnbtrie_node_t *n,
                mnbtrie_node_t *parent,
                char idx,
                unsigned short digit,
                void *value)
{
    assert(n != NULL);

    n->parent = parent;
    n->child[0] = NULL;
    n->child[1] = NULL;
    n->idx = idx;
    n->digit = digit;
    n->value = value;
}


void
btrie_init(mnbtrie_t *tr)
{
    unsigned i;

    assert(tr != NULL);

    for (i = 0; i < countof(tr->roots); ++i) {
        btrie_node_init(&tr->roots[i], NULL, i, 0, NULL);
    }
    tr->volume = 0;
    tr->nvals = 0;
}

#define BTRIE_NODE_FINI_BODY(freefn, finifn)   \
    assert(n != NULL);                         \
    if (n->child[0] != NULL) {                 \
        finifn(tr, n->child[0]);               \
        freefn(n->child[0]);                   \
        n->child[0] = NULL;                    \
        --(tr->volume);                        \
    }                                          \
    if (n->child[1] != NULL) {                 \
        finifn(tr, n->child[1]);               \
        freefn(n->child[1]);                   \
        n->child[1] = NULL;                    \
        --(tr->volume);                        \
    }                                          \
    n->parent = NULL                           \
                                               \


static void
btrie_node_fini(mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_NODE_FINI_BODY(free, btrie_node_fini);
}


static void
btrie_node_fini_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_NODE_FINI_BODY(_free, _btrie_node_fini);
}


void
btrie_fini(mnbtrie_t *tr)
{
    unsigned i;

    assert(tr != NULL);

    //btrie_traverse(tr, mydump, NULL);
    for (i = 0; i < countof(tr->roots); ++i) {
        btrie_node_fini(tr, &tr->roots[i]);
    }
}


void
btrie_fini_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr)
{
    unsigned i;

    assert(tr != NULL);

    for (i = 0; i < countof(tr->roots); ++i) {
        btrie_node_fini_mpool(mpool, tr, &tr->roots[i]);
    }
}


inline static int
btrie_node_is_orphan(mnbtrie_node_t *n)
{
    return (n->child[0] == NULL) && (n->child[1] == NULL) && (n->value == NULL);
}


#define BTRIE_NODE_CLEANUP_BODY(freefn, cleanupfn)     \
    assert(n != NULL);                                 \
    if (btrie_node_is_orphan(n)) {                     \
        return;                                        \
    }                                                  \
    if (n->child[0] != NULL) {                         \
        if (!btrie_node_is_orphan(n->child[0])) {      \
            cleanupfn(tr, n->child[0]);                \
            if (btrie_node_is_orphan(n->child[0])) {   \
                freefn(n->child[0]);                   \
                n->child[0] = NULL;                    \
                --(tr->volume);                        \
            }                                          \
        } else {                                       \
            freefn(n->child[0]);                       \
            n->child[0] = NULL;                        \
            --(tr->volume);                            \
        }                                              \
    }                                                  \
    if (n->child[1] != NULL) {                         \
        if (!btrie_node_is_orphan(n->child[1])) {      \
            cleanupfn(tr, n->child[1]);                \
            if (btrie_node_is_orphan(n->child[1])) {   \
                freefn(n->child[1]);                   \
                n->child[1] = NULL;                    \
                --(tr->volume);                        \
            }                                          \
        } else {                                       \
            freefn(n->child[1]);                       \
            n->child[1] = NULL;                        \
            --(tr->volume);                            \
        }                                              \
    }                                                  \


static void
btrie_node_cleanup(mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_NODE_CLEANUP_BODY(free, btrie_node_cleanup);
}


static void
btrie_node_cleanup_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_NODE_CLEANUP_BODY(_free, _btrie_node_cleanup);
}


#define BTRIE_CLEANUP_BODY(cleanupfn)          \
    unsigned i;                                \
    assert(tr != NULL);                        \
    for (i = 0; i < countof(tr->roots); ++i) { \
        cleanupfn(tr, &tr->roots[i]);          \
    }                                          \


void
btrie_cleanup(mnbtrie_t *tr)
{
    BTRIE_CLEANUP_BODY(btrie_node_cleanup);
}


void
btrie_cleanup_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr)
{
    BTRIE_CLEANUP_BODY(_btrie_node_cleanup);
}


int
btrie_node_traverse(mnbtrie_node_t *n,
                   int (*cb)(mnbtrie_node_t *, void *),
                   void *udata)
{
    int res;

    if ((res = cb(n, udata)) != 0) {
        goto end;
    }
    if (n->child[0] != NULL) {
        if ((res = btrie_node_traverse(n->child[0], cb, udata)) != 0) {
            goto end;
        }
    } else {
        if ((res = cb(n->child[0], udata)) != 0) {
            goto end;
        }
    }
    if (n->child[1] != NULL) {
        if ((res = btrie_node_traverse(n->child[1], cb, udata)) != 0) {
            goto end;
        }
    } else {
        if ((res = cb(n->child[1], udata)) != 0) {
            goto end;
        }
    }

end:
    return res;
}


int
btrie_traverse(mnbtrie_t *tr, int (*cb)(mnbtrie_node_t *, void *), void *udata)
{
    int res;
    unsigned i;

    for (i = 0; i < countof(tr->roots); ++i) {
        if ((res = btrie_node_traverse(&tr->roots[i], cb, udata)) != 0) {
            return res;
        }
    }

    return 0;
}


#define BTRIE_ADD_NODE_BODY(mallocfn)                                  \
    int idx;                                                           \
    unsigned digit, sel;                                               \
    mnbtrie_node_t **n;                                                \
    mnbtrie_node_t *cur;                                               \
    idx = flsl(key);                                                   \
    if (idx == 0) {                                                    \
        return NULL;                                                   \
    }                                                                  \
    --idx;                                                             \
    cur = &tr->roots[idx];                                             \
    --idx;                                                             \
    for (digit = 1; idx >= 0; --idx, ++digit) {                        \
        /*                                                             \
         * sel (selector):                                             \
         *  - 0, keys are 0x..x                                        \
         *  - 1, keys are 1x..x                                        \
         */                                                            \
        sel = (key & (1ul << idx)) >> idx;                             \
        assert((sel == 0) || (sel == 1));                              \
        n = &cur->child[sel];                                          \
        if (*n == NULL) {                                              \
            if ((*n = mallocfn(sizeof(mnbtrie_node_t))) == NULL) {     \
                FAIL("malloc");                                        \
            }                                                          \
            btrie_node_init(*n,                                        \
                           cur,                                        \
                           idx,                                        \
                           (digit | (sel << CHILD_SELECTOR_SHIFT)),    \
                           NULL);                                      \
            ++(tr->volume);                                            \
        }                                                              \
        cur = *n;                                                      \
    }                                                                  \
    return cur;                                                        \



mnbtrie_node_t *
btrie_add_node(mnbtrie_t *tr, uintmax_t key)
{
    BTRIE_ADD_NODE_BODY(malloc);
}


mnbtrie_node_t *
btrie_add_node_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr, uintmax_t key)
{
    BTRIE_ADD_NODE_BODY(_malloc);
}


mnbtrie_node_t *
btrie_find_exact(mnbtrie_t *tr, uintmax_t key)
{
    int idx;
    mnbtrie_node_t *cur;

    idx = flsl(key);

    if (idx == 0) {
        return NULL;
    }
    --idx;

    cur = &tr->roots[idx];
    for (--idx; cur != NULL && idx >= 0; --idx) {
        int sel;

        sel = (key >> idx) & 0x01;
        assert((sel == 0) || (sel == 1));
        cur = cur->child[sel];
    }

    return cur;
}


/**
 * Walk down the tree until we find a node with value.
 *
 */
static mnbtrie_node_t *
find_value_reverse(mnbtrie_node_t *node)
{
    mnbtrie_node_t *res = NULL;

    if (node->value != NULL) {
        res = node;
        goto end;
    }

    if (node->child[1] != NULL) {
        if (node->child[1]->value != NULL) {
            res = node->child[1];
            goto end;
        } else {
            if ((res = find_value_reverse(node->child[1])) != NULL) {
                goto end;
            }
        }
    }

    if (node->child[0] != NULL) {
        if (node->child[0]->value != NULL) {
            res = node->child[0];
        } else {
            res = find_value_reverse(node->child[0]);
        }
    }

end:
    return res;
}


static mnbtrie_node_t *
find_value_traverse(mnbtrie_node_t *node)
{
    mnbtrie_node_t *res = NULL;

    if (node->value != NULL) {
        res = node;
        goto end;
    }

    if (node->child[0] != NULL) {
        if (node->child[0]->value != NULL) {
            res = node->child[0];
            goto end;
        } else {
            if ((res = find_value_traverse(node->child[0])) != NULL) {
                goto end;
            }
        }
    }

    if (node->child[1] != NULL) {
        if (node->child[1]->value != NULL) {
            res = node->child[1];
        } else {
            res = find_value_traverse(node->child[1]);
        }
    }

end:
    return res;
}


static mnbtrie_node_t *
find_value_lean_left(mnbtrie_node_t *node)
{
    while (node != NULL) {
        int mysel;
        mnbtrie_node_t *parent;

        if (node->child[0] != NULL) {
            mnbtrie_node_t *tmp;

            if ((tmp = find_value_reverse(node->child[0])) != NULL) {
                node = tmp;
                break;
            }
        }

        if ((parent = node->parent) == NULL) {
            node = NULL;
            break;
        }

        mysel = MNBTRIE_NODESEL0(node);
        assert((mysel == 0) || (mysel == 1));

        if (mysel == 1) {
            node = parent;
        } else {
            while (node->parent != NULL && mysel == 0) {
                node = node->parent;
                mysel = MNBTRIE_NODESEL0(node);
                assert((mysel == 0) || (mysel == 1));
            }
            assert(node != NULL);
            node = node->parent;
        }
    }

    return node;
}


static mnbtrie_node_t *
find_closest_partial_back(mnbtrie_node_t *node,
                          uintmax_t key)
{
    //TRACE("key %02jx", key);
    if (node->idx == 0) {
        if (node->value == NULL) {
            node = NULL;
        }
        goto end;
    }

    while (node->idx > 0) {
        int sel;
        mnbtrie_node_t *tmp;

        sel = MNBTRIE_CHILDSEL(node, key);
        assert((sel == 0) || (sel == 1));
        //TRACEC("sel %d ", sel);
        //btrie_node_dump(node);
        if ((tmp = node->child[sel]) != NULL) {
            node = tmp;
        } else {
            node = find_value_lean_left(node);
            break;
        }
    }

end:
    return node;
}


static mnbtrie_node_t *
find_value_lean_right(mnbtrie_node_t *node)
{
    while (node != NULL) {
        int mysel;
        mnbtrie_node_t *parent;

        if (node->child[1] != NULL) {
            mnbtrie_node_t *tmp;

            if ((tmp = find_value_traverse(node->child[1])) != NULL) {
                node = tmp;
                break;
            }
        }

        if ((parent = node->parent) == NULL) {
            node = NULL;
            break;
        }

        mysel = MNBTRIE_NODESEL0(node);
        assert((mysel == 0) || (mysel == 1));

        if (mysel == 0) {
            node = parent;
        } else {
            while (node->parent != NULL && mysel == 1) {
                node = node->parent;
                mysel = MNBTRIE_NODESEL0(node);
                assert((mysel == 0) || (mysel == 1));
            }
            assert(node != NULL);
            node = node->parent;
        }
    }

    return node;
}


static mnbtrie_node_t *
find_closest_partial_forw(mnbtrie_node_t *node,
                          uintmax_t key)
{
    if (node->idx == 0) {
        if (node->value == NULL) {
            node = NULL;
        }
        goto end;
    }

    while (node->idx > 0) {
        int sel;
        mnbtrie_node_t *tmp;

        sel = MNBTRIE_CHILDSEL(node, key);
        assert((sel == 0) || (sel == 1));
        if ((tmp = node->child[sel]) != NULL) {
            node = tmp;
        } else {
            node = find_value_lean_right(node);
            break;
        }
    }

end:
    return node;
}


mnbtrie_node_t *
btrie_find_closest(mnbtrie_t *tr, uintmax_t key, int direction)
{
    int idx;
    mnbtrie_node_t *root, *res = NULL;

    direction = direction ? 1 : 0;

    idx = flsl(key);
    if (idx == 0) {
        goto end;
    }
    --idx;
    //TRACE("key %02jd idx=%d", key, idx);

    root = &tr->roots[idx];
    //TRACEC("root: ");
    //btrie_node_dump(root);

    if (direction) {
        if ((res = find_closest_partial_forw(root, key)) != NULL) {
            goto end;
        }
        ++idx;
        for (; idx < (int)countof(tr->roots); ++idx) {
            root = &tr->roots[idx];
            if ((res = find_value_traverse(root)) != NULL) {
                break;
            }
        }

    } else {
        if ((res = find_closest_partial_back(root, key)) != NULL) {
            goto end;
        }
        --idx;
        for (; idx >= 0; --idx) {
            root = &tr->roots[idx];
            if ((res = find_value_reverse(root)) != NULL) {
                break;
            }
        }
    }

end:
    return res;
}


#define CLEANUP_ORPHANS_BODY(freefn)                   \
    while (n != NULL) {                                \
        if (n->parent == NULL) {                       \
            break;                                     \
        }                                              \
        if (btrie_node_is_orphan(n)) {                 \
            mnbtrie_node_t *parent;                    \
            parent = n->parent;                        \
            assert(parent != NULL);                    \
            if (parent->child[0] == n) {               \
                parent->child[0] = NULL;               \
            } else {                                   \
                if (parent->child[1] != n) {           \
                    btrie_node_dump(n);                \
                    D8(n, sizeof(*n));                 \
                    btrie_node_dump(parent);           \
                    btrie_traverse(tr, mydump, NULL);  \
                }                                      \
                assert(parent->child[1] == n);         \
                parent->child[1] = NULL;               \
            }                                          \
            freefn(n);                                 \
            --(tr->volume);                            \
            n = parent;                                \
        } else {                                       \
            break;                                     \
        }                                              \
    }                                                  \


static void
cleanup_orphans(mnbtrie_t *tr, mnbtrie_node_t *n)
{
    CLEANUP_ORPHANS_BODY(free);
}


static void
cleanup_orphans_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr, mnbtrie_node_t *n)
{
    CLEANUP_ORPHANS_BODY(_free);
}


#define BTRIE_REMOVE_NODE_BODY(finifn, freefn, __a1)           \
    mnbtrie_node_t *parent;                                    \
    assert(n->value == NULL);                                  \
    parent = n->parent;                                        \
    if (parent != NULL) {                                      \
        if (parent->child[0] == n) {                           \
            parent->child[0] = NULL;                           \
        } else if (parent->child[1] == n) {                    \
            parent->child[1] = NULL;                           \
        } else {                                               \
            TRACE("tr=%p parent=%p n=%p "                      \
                  "parent->child[0]=%p parent->child[1]=%p",   \
                  tr, parent, n,                               \
                  parent->child[0], parent->child[1]);         \
            D8(n, sizeof(*n));                                 \
            btrie_traverse(tr, mydump, NULL);                  \
            FAIL("btrie_node_remove");                         \
        }                                                      \
        finifn(tr, n);                                         \
        freefn(n);                                             \
        --(tr->volume);                                        \
    } else {                                                   \
        finifn(tr, n);                                         \
    }                                                          \
    __a1                                                       \
    return 0                                                   \


int
btrie_remove_node_no_cleanup(mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_REMOVE_NODE_BODY(btrie_node_fini, free, );
}


int
btrie_remove_node_no_cleanup_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_REMOVE_NODE_BODY(_btrie_node_fini, _free, );
}


int
btrie_remove_node(mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_REMOVE_NODE_BODY(btrie_node_fini, free,
            cleanup_orphans(tr, parent);
    );
}


int
btrie_remove_node_mpool(mpool_ctx_t *mpool, mnbtrie_t *tr, mnbtrie_node_t *n)
{
    BTRIE_REMOVE_NODE_BODY(_btrie_node_fini, _free,
            _cleanup_orphans(tr, parent);
    );
}


size_t
btrie_get_volume(mnbtrie_t *tr)
{
    return tr->volume;
}


size_t
btrie_get_nvals(mnbtrie_t *tr)
{
    return tr->nvals;
}


// vim:list
