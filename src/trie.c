#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h>

#ifndef HAVE_FLSL
#   ifdef __GNUC__
#       define flsl(v) (((v) != 0L) ? (64 - __builtin_clzl(v)) : 0)
#   else
#       error "Could not find/define flsl."
#   endif
#endif

#include <diag.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>
#include <mrkcommon/trie.h>

#ifdef DO_MEMDEBUG
#include "mrkcommon/memdebug.h"
MEMDEBUG_DECLARE(trie);
#endif

void
trie_node_dump(trie_node_t *n)
{
    if (n == NULL) {
        TRACE("NULL");
    } else {
        // \033[01;31m%02lx\033[00m
        TRACE("[%p]-> %08x %p/%p %p",
              n, n->digit, n->child[0], n->child[1], n->value);
    }
}

int
trie_node_dump_cb(trie_node_t *n, UNUSED uint64_t key, void *arg)
{
    int indent, selector;
    int flags = (intptr_t)arg;
    test_data_t *d;

    if (n == NULL) {
        TRACE("NULL");
        return 0;
    }

    d = (test_data_t *)n->value;

    if (n->child[0] == NULL && n->child[1] == NULL && n->value == NULL) {
        return 0;
    }
    if (n->digit == -1) {
        selector = 0;
        indent = n->digit;
    } else {
        selector = n->digit & (1 << CHILD_SELECTOR_SHIFT);
        indent = n->digit & ~(1 << CHILD_SELECTOR_SHIFT);
    }
    if (d == NULL) {
        if (flags) {
            TRACE("key=");
            //TRACE("[n=%p value=%p]", n, d);
        } else {
            if (selector) {
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
        TRACE("key=%02lx", d->key);
        //TRACE("n=%p value=%p key=%02lx", n, d, d->key);
    } else {
        if (selector) {
            LTRACE(indent + 1, FRED("key=%02lx"), d->key);
        } else {
            LTRACE(indent + 1, "key=%02lx", d->key);
        }
        //LTRACE(indent + 1, "n=%p value=%p key=%02lx", n, d, d->key);
    }
    return 0;
}

void
trie_node_init(trie_node_t *n, trie_node_t *parent, int digit, void *value)
{
    assert(n != NULL);

    n->parent = parent;
    n->child[0] = NULL;
    n->child[1] = NULL;
    n->digit = digit;
    n->value = value;
}

void
trie_init(trie_t *tr)
{
    unsigned i;

    assert(tr != NULL);

    for (i = 0; i < countof(tr->roots); ++i) {
        trie_node_init(&tr->roots[i], NULL, -1, NULL);
    }
    tr->volume = 0;
    tr->nvals = 0;
}


#define TRIE_NODE_FINI_BODY(freefn, finifn) \
    assert(n != NULL); \
    if (n->child[0] != NULL) { \
        finifn(tr, n->child[0]); \
        freefn(n->child[0]); \
        n->child[0] = NULL; \
        --(tr->volume); \
    } \
    if (n->child[1] != NULL) { \
        finifn(tr, n->child[1]); \
        freefn(n->child[1]); \
        n->child[1] = NULL; \
        --(tr->volume); \
    } \
    n->parent = NULL

static void
trie_node_fini(trie_t *tr, trie_node_t *n)
{
    TRIE_NODE_FINI_BODY(free, trie_node_fini);
}

static void
trie_node_fini_mpool(mpool_ctx_t *mpool, trie_t *tr, trie_node_t *n)
{
#define _free(ptr) mpool_free(mpool, (ptr))
#define _trie_node_fini(tr, node) trie_node_fini_mpool(mpool, (tr), (node))
    TRIE_NODE_FINI_BODY(_free, _trie_node_fini);
#undef _free
#undef _trie_node_fini
}

void
trie_fini(trie_t *tr)
{
    unsigned i;

    assert(tr != NULL);

    for (i = 0; i < countof(tr->roots); ++i) {
        trie_node_fini(tr, &tr->roots[i]);
    }
}

void
trie_fini_mpool(mpool_ctx_t *mpool, trie_t *tr)
{
    unsigned i;

    assert(tr != NULL);

    for (i = 0; i < countof(tr->roots); ++i) {
        trie_node_fini_mpool(mpool, tr, &tr->roots[i]);
    }
}

static int
trie_node_is_orphan(trie_node_t *n)
{
    return (n->child[0] == NULL) && (n->child[1] == NULL && n->value == NULL);

}


#define TRIE_NODE_CLEANUP_BODY(freefn, cleanupfn) \
    if (n == NULL) { \
        return; \
    } \
    if (trie_node_is_orphan(n)) { \
        return; \
    } \
    if (n->child[0] != NULL) { \
        if (!trie_node_is_orphan(n->child[0])) { \
            cleanupfn(tr, n->child[0]); \
            if (trie_node_is_orphan(n->child[0])) { \
                freefn(n->child[0]); \
                n->child[0] = NULL; \
                --(tr->volume); \
            } \
        } else { \
            freefn(n->child[0]); \
            n->child[0] = NULL; \
            --(tr->volume); \
        } \
    } \
    if (n->child[1] != NULL) { \
        if (!trie_node_is_orphan(n->child[1])) { \
            cleanupfn(tr, n->child[1]); \
            if (trie_node_is_orphan(n->child[1])) { \
                freefn(n->child[1]); \
                n->child[1] = NULL; \
                --(tr->volume); \
            } \
        } else { \
            freefn(n->child[1]); \
            n->child[1] = NULL; \
            --(tr->volume); \
        } \
    }

static void
trie_node_cleanup(trie_t *tr, trie_node_t *n)
{
    TRIE_NODE_CLEANUP_BODY(free, trie_node_cleanup);
}


static void
trie_node_cleanup_mpool(mpool_ctx_t *mpool, trie_t *tr, trie_node_t *n)
{
#define _free(ptr) mpool_free(mpool, (ptr))
#define _trie_node_cleanup(tr, node) trie_node_cleanup_mpool(mpool, (tr), (node))
    TRIE_NODE_CLEANUP_BODY(_free, _trie_node_cleanup);
#undef _free
#undef _trie_node_cleanup
}


#define TRIE_CLEANUP_BODY(cleanupfn) \
    unsigned i; \
    assert(tr != NULL); \
    for (i = 0; i < countof(tr->roots); ++i) { \
        cleanupfn(tr, &tr->roots[i]); \
    }


void
trie_cleanup(trie_t *tr)
{
    TRIE_CLEANUP_BODY(trie_node_cleanup);
}


void
trie_cleanup_mpool(mpool_ctx_t *mpool, trie_t *tr)
{
#define _trie_node_cleanup(tr, node) trie_node_cleanup_mpool(mpool, (tr), (node))
    TRIE_CLEANUP_BODY(_trie_node_cleanup);
#undef _trie_node_cleanup
}


int
trie_node_traverse(trie_node_t *n,
                   int idx,
                   uint64_t key,
                   int (*cb)(trie_node_t *, uint64_t, void *),
                   void *udata)
{
    int res;

    //TRACE("key=%016lx", key);
    if (n == NULL) {
        return 0;
    }

    idx = idx ? (idx - 1) : 0;

    if (n->child[0] != NULL) {
        //TRACE("child[0]");
        if ((res = trie_node_traverse(n->child[0], idx, key, cb, udata)) != 0) {
            return res;
        }
    }

    //TRACE("me");
    if ((res = cb(n, key, udata)) != 0) {
        return res;
    }

    if (n->child[1] != NULL) {
        //TRACE("child[1]");
        if ((res = trie_node_traverse(n->child[1], idx, key | (1ul << idx), cb, udata)) != 0) {
            return res;
        }
    }

    return 0;
}

int
trie_traverse(trie_t *tr, int (*cb)(trie_node_t *, uint64_t, void *), void *udata)
{
    int res;
    unsigned i;

    for (i = 0; i < (countof(tr->roots) - 0); ++i) {
        uint64_t key;

        if (i == 0) {
            key = 0ul;
        } else {
            key = 1ul << (i-1);
        }
        if ((res = trie_node_traverse(&tr->roots[i], i, key, cb, udata)) != 0) {
            return res;
        }
    }

    return 0;
}

#define TRIE_ADD_NODE_BODY(mallocfn)                                           \
    int idx, i, sel;                                                           \
    trie_node_t **n;                                                           \
    trie_node_t *cur;                                                          \
    idx = flsl(key);                                                           \
    cur = &tr->roots[idx];                                                     \
    if (idx == 0) {                                                            \
        ++idx;                                                                 \
    }                                                                          \
    i = 0;                                                                     \
    do {                                                                       \
        /*                                                                     \
         * sel (selector):                                                     \
         *  - 0, keys are 0x..x                                                \
         *  - 1, keys are 1x..x                                                \
         */                                                                    \
        sel = (key & (1ul << (idx - 1))) >> (idx - 1);                         \
        n = &cur->child[sel];                                                  \
        if (*n == NULL) {                                                      \
            if ((*n = mallocfn(sizeof(trie_node_t))) == NULL) {                \
                FAIL("malloc");                                                \
            }                                                                  \
            trie_node_init(*n,                                                 \
                           cur,                                                \
                           (int)((unsigned)i |                                 \
                               ((unsigned)sel << CHILD_SELECTOR_SHIFT)),       \
                           NULL);                                              \
            ++(tr->volume);                                                    \
        }                                                                      \
        cur = *n;                                                              \
        ++i;                                                                   \
    } while (--idx);                                                           \
    ++(tr->nvals);                                                             \
    return cur;                                                                \



trie_node_t *
trie_add_node(trie_t *tr, uintptr_t key)
{
    TRIE_ADD_NODE_BODY(malloc);
}

trie_node_t *
trie_add_node_mpool(mpool_ctx_t *mpool, trie_t *tr, uintptr_t key)
{
#define _malloc(sz) mpool_malloc(mpool, (sz))
    TRIE_ADD_NODE_BODY(_malloc);
#undef _malloc
}

trie_node_t *
trie_find_exact(trie_t *tr, uintptr_t key)
{
    int idx;
    trie_node_t **n;
    trie_node_t *cur;

    idx = flsl(key);

    cur = &tr->roots[idx];
    if (idx == 0) {
        ++idx;
    }

    do {
        n = &cur->child[(key & (1ul << (idx - 1))) >> (idx - 1)];
        if (*n == NULL) {
            return NULL;
        }
        cur = *n;
    } while (--idx);

    return cur;
}

/**
 * Walk down the tree until we find a node with value.
 *
 */
UNUSED static trie_node_t *
trie_descend(trie_node_t *n, int bias)
{
    trie_node_t *res;

    assert(bias == 0 || bias == 1);
    assert(n != NULL);

    if (n->child[bias] != NULL) {
        res = trie_descend(n->child[bias], bias);
        if (res->value != NULL) {
            return res;
        }
    }

    if (n->child[1 ^ bias] != NULL) {
        res = trie_descend(n->child[1 ^ bias], bias);
        if (res->value != NULL) {
            return res;
        }
    }

    /* couldn't descend further, return what we were passed in */
    return n;
}


/**
 * An iterative version of trie_descend()
 *
 */
static trie_node_t *
trie_find_value(trie_node_t *n, int bias)
{
    assert(bias == 0 || bias == 1);
    assert(n != NULL);

    while (1) {
        if (n->child[bias] != NULL) {
            n = n->child[bias];
            continue;
        }
        if (n->value != NULL) {
            break;
        }
        if (n->child[bias ^ 1] != NULL) {
            n = n->child[bias ^ 1];
            continue;
        }
        if (n->value != NULL) {
            break;
        }
        break;
    }

    return n;
}

static trie_node_t *
find_closest_partial(trie_node_t *node, int idx, uintptr_t key, int direction)
{
    /*
     * The exact match loop.
     */
    //TRACE("idx=%d key=%lx", idx, key);
    //trie_node_dump(node);
    do {
        trie_node_t **n;
        int child_idx = (key & (1ul << (idx - 1))) >> (idx - 1);

        //TRACE("idx=%d child_idx=%d", idx, child_idx);

        n = &node->child[child_idx];
        if (*n != NULL) {
            node = *n;

        } else {
            /*
             * Not an exact match, the outer do {} while () will be
             * abandoned. We proceed with the closest match logic now.
             *
             * The closest match logic is to descend the part of the tree
             * that is in the given direction. That is, if the direction is
             * 0, we select the zeroth child, zeroth brothers, otherwise
             * one'th child and one'th brothers.
             *
             * The brother(idx, node) returns the idx'th brother of either
             * the node, or its ascendant, or NULL, if not found.
             *
             * In the selected part of the tree, we descend in the flipped
             * direction. The generalized algorithm is:
             *
             * 1. child_idx ^ dir ? descend(node, 1 ^ dir) : (void)
             * 2. descend(brother(dir, node), 1 ^ dir)
             *
             */
            trie_node_t *res;

            /* 1. first descend the other child if applicable */
            //TRACE("child_idx ^ direction = %d", child_idx ^ direction);
            if (child_idx ^ direction) {
                res = trie_find_value(node, 1 ^ direction);
                if (res->value != NULL) {
                    return res;
                }
            }

            /* now descend the relatives in the given direction */
            while (1) {
                int node_idx;
                trie_node_t *bro = NULL;

                /* 2a. brother(dir, ...) */
                while (bro == NULL) {
                    //trie_node_dump(node);
                    if (node->parent == NULL) {
                        /* root */
                        return NULL;
                    }

                    node_idx = (((unsigned)(node->digit)) &
                                (1 << CHILD_SELECTOR_SHIFT)) >>
                                    CHILD_SELECTOR_SHIFT;

                    //TRACE("node_idx=%d", node_idx);

                    if (node_idx ^ direction) {
                        bro = node->parent->child[direction];
                        if (bro == NULL) {
                            node = node->parent;
                        }
                    } else {
                        node = node->parent;
                    }
                }

                /* 2b. descend(bro, 1 ^ dir) */
                res = trie_find_value(bro, 1 ^ direction);

                if (res->value != NULL) {
                    return res;
                } else {
                    bro = NULL;
                    node = node->parent;
                }
            }
        }
        //TRACE("node=%p", node);
    } while (--idx);

    return node;
}

trie_node_t *
trie_find_closest(trie_t *tr, uintptr_t key, int direction)
{
    unsigned idx;
    trie_node_t *root, *res;

    direction = direction ? 1 : 0;

    idx = flsl(key);

    if (direction) {
        /* exception for 0 */
        if (idx == 0) {
            root = &tr->roots[idx];
            ++idx;
            if ((res = find_closest_partial(root,
                                            idx,
                                            key,
                                            direction)) != NULL) {
                return res;
            }
            assert(idx == 1);
            key = 0ul;
        }
        /* first, or second */
        root = &tr->roots[idx];
        if ((res = find_closest_partial(root,
                                        idx,
                                        key,
                                        direction)) != NULL) {
            return res;
        }
        key = 0ul;
        ++idx;
        /* the rest */
        for (; idx <= TREE_DEPTH; ++idx) {
            root = &tr->roots[idx];
            if ((res = find_closest_partial(root,
                                            idx,
                                            key,
                                            direction)) != NULL) {
                return res;
            }
        }
    } else {
        if (idx == 0) {
            root = &tr->roots[idx];
            ++idx;
            if ((res = find_closest_partial(root,
                                            idx,
                                            key,
                                            direction)) != NULL) {
                return res;
            }
        } else {
            /* first */
            root = &tr->roots[idx];
            if ((res = find_closest_partial(root,
                                            idx,
                                            key,
                                            direction)) != NULL) {
                return res;
            }
            key = ULONG_MAX;
            --idx;
            /* the rest */
            for (; idx > 0; --idx) {
                root = &tr->roots[idx];
                if ((res = find_closest_partial(root,
                                                idx,
                                                key,
                                                direction)) != NULL) {
                    return res;
                }
            }
            /* exception for 0 */
            assert(idx == 0);
            root = &tr->roots[idx];
            ++idx;
            if ((res = find_closest_partial(root,
                                            idx,
                                            key,
                                            direction)) != NULL) {
                return res;
            }
        }
    }
    return NULL;
}

#define CLEANUP_ORPHANS_BODY(freefn)                   \
    trie_node_t *parent;                               \
    while (n != NULL) {                                \
        if (n->parent == NULL) {                       \
            break;                                     \
        }                                              \
        if (trie_node_is_orphan(n)) {                  \
            parent = n->parent;                        \
            if (parent != NULL) {                      \
                if (parent->child[0] == n) {           \
                    parent->child[0] = NULL;           \
                } else {                               \
                    assert(parent->child[1] == n);     \
                    parent->child[1] = NULL;           \
                }                                      \
            }                                          \
            freefn(n);                                 \
            --(tr->volume);                            \
            n = parent;                                \
        } else {                                       \
            break;                                     \
        }                                              \
    }                                                  \


static void
cleanup_orphans(trie_t *tr, trie_node_t *n)
{
    CLEANUP_ORPHANS_BODY(free);
}


static void
cleanup_orphans_mpool(mpool_ctx_t *mpool, trie_t *tr, trie_node_t *n)
{
#define _free(ptr) mpool_free(mpool, (ptr))
    CLEANUP_ORPHANS_BODY(_free);
#undef _free
}


#define TRIE_REMOVE_NODE_BODY(finifn, freefn, cleanupfn)       \
    trie_node_t *parent;                                       \
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
            FAIL("trie_node_remove");                          \
        }                                                      \
    }                                                          \
    finifn(tr, n);                                             \
    freefn(n);                                                 \
    --(tr->volume);                                            \
    --(tr->nvals);                                             \
    cleanupfn(tr, parent);                                     \
    return 0;                                                  \


int
trie_remove_node(trie_t *tr, trie_node_t *n)
{
    TRIE_REMOVE_NODE_BODY(trie_node_fini, free, cleanup_orphans);
}

int
trie_remove_node_mpool(mpool_ctx_t *mpool, trie_t *tr, trie_node_t *n)
{
#define _trie_node_fini(tr, node) trie_node_fini_mpool(mpool, (tr), (node))
#define _free(ptr) mpool_free(mpool, (ptr))
#define _cleanup_orphans(tr, node) cleanup_orphans_mpool(mpool, (tr), (node))
    TRIE_REMOVE_NODE_BODY(_trie_node_fini, _free, _cleanup_orphans);
#undef _trie_node_fini
#undef _free
#undef _cleanup_orphans
}

size_t
trie_get_volume(trie_t *tr)
{
    return tr->volume;
}

size_t
trie_get_nvals(trie_t *tr)
{
    return tr->nvals;
}


// vim:list
