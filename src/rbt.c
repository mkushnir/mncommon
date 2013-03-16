/**
 * Red-black Tree. Source: Wikipaedia.
 */
#include <assert.h>
#include <stdlib.h>

#include "diag.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/rbt.h"

int
rbt_node_init(rbt_node_t *node, uintptr_t key, void *data)
{
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->prev = NULL;
    node->next = NULL;
    node->flags = 0;
    node->key = key;
    node->data = data;
    return 0;
}

int
rbt_init(rbt_t *rbt)
{
    rbt->root = NULL;
    rbt->head = NULL;
    rbt->tail = NULL;
    rbt->nelem = 0;

    return 0;
}

int
rbt_fini(rbt_t *rbt)
{
    rbt_node_t *e = rbt->head;

    while (e != NULL) {
        rbt_remove_node(rbt, e);
        e = rbt->head;
    }

    return 0;
}

#define RBT_ISRED(n) (((n) != NULL) && ((n)->flags & RBT_FLAG_RED))
#define RBT_ISBLACK(n) (((n) == NULL) || (((n)->flags & RBT_FLAG_RED) == 0))
#define RBT_SETBLACK(n) ((n)->flags &= ~RBT_FLAG_RED)
#define RBT_SETRED(n) ((n)->flags |= RBT_FLAG_RED)
#define RBT_SETCOLOR(n1, n2) (RBT_ISRED(n2) ? RBT_SETRED(n1) : RBT_SETBLACK(n1))
#define RBT_ASSIGN(rbt, slot, node) (slot) = node; RBT_SETRED(node); ++(rbt)->nelem

void
rbt_node_dump_tree(rbt_node_t *node, int level)
{
    if (RBT_ISRED(node)) {
        LTRACE(level, "%02lx<-\033[01;31m%02lx\033[00m",
               node->parent != NULL ? node->parent->key : 0x4007,
               node->key);
    } else {
        LTRACE(level, "%02lx<-\033[01;30m%02lx\033[00m",
               node->parent != NULL ? node->parent->key : 0x4007,
               node->key);
    }
}

void
rbt_node_dump_list(rbt_node_t *node)
{
    if (RBT_ISRED(node)) {
        LTRACE(0, "%02lx<-\033[01;31m%02lx\033[00m->%02lx",
               node->prev != NULL ? node->prev->key : 0x4ead,
               node->key,
               node->next != NULL ? node->next->key : 0x7a11);
    } else {
        LTRACE(0, "%02lx<-\033[01;30m%02lx\033[00m->%02lx",
               node->prev != NULL ? node->prev->key : 0x4ead,
               node->key,
               node->next != NULL ? node->next->key : 0x7a11);
    }
}

static void
_rbt_dump_tree(rbt_t *rbt, rbt_node_t *node, int level)
{
    if (node->right != NULL) {
         _rbt_dump_tree(rbt, node->right, level + 1);
    }
    rbt_node_dump_tree(node, level);
    if (node->left != NULL) {
        _rbt_dump_tree(rbt, node->left, level + 1);
    }
}

int rbt_dump_tree(rbt_t *rbt)
{
    TRACE("nelem=%ld", rbt->nelem);
    if (rbt->root != NULL) {
        _rbt_dump_tree(rbt, rbt->root, 1);
    }
    return 0;
}

int rbt_dump_list(rbt_t *rbt)
{
    rbt_node_t *node;

    TRACE("nelem=%ld", rbt->nelem);
    node = rbt->head;
    while (node != NULL) {
        rbt_node_dump_list(node);
        node = node->next;
    }
    return 0;
}

int
rbt_find(rbt_t *rbt, uintptr_t key, rbt_node_t **rv)
{
    rbt_node_t *tmp = rbt->root;

    while (tmp != NULL) {
        if (key == tmp->key) {
            *rv = tmp;
            return 0;
        } else if (key < tmp->key) {
            tmp = tmp->left;
        } else {
            tmp = tmp->right;
        }
    }

    *rv = tmp;
    return 0;
}

static int
_rbt_insert(rbt_t *rbt, rbt_node_t *source, rbt_node_t *target, rbt_node_t **dup)
{
    //TRACE("Assigning %ld", target->key);
    if (source == NULL) {

        RBT_ASSIGN(rbt, rbt->root, target);
        rbt->head = rbt->tail = target;

        return -2;
    }

    if (target->key < source->key) {
        if (source->left != NULL) {
            int res;
            res = _rbt_insert(rbt, source->left, target, dup);
            return res;
        }

        RBT_ASSIGN(rbt, source->left, target);
        target->parent = source;

        target->prev = source->prev;
        if (source->prev == NULL) {
            rbt->head = target;
        } else {
            source->prev->next = target;
        }
        target->next = source;
        source->prev = target;

        return -1;
    }
    if (target->key > source->key) {
        if (source->right != NULL) {
            int res;
            res = _rbt_insert(rbt, source->right, target, dup);
            return res;
        }

        RBT_ASSIGN(rbt, source->right, target);
        target->parent = source;

        target->next = source->next;
        if (source->next == NULL) {
            rbt->tail = target;
        } else {
            source->next->prev = target;
        }
        target->prev = source;
        source->next = target;

        return 1;
    }
    *dup = source;
    return 0;
}

inline static void
_rbt_rotate_left(rbt_t *rbt, rbt_node_t *n)
{
    rbt_node_t *p = n->parent;
    rbt_node_t *nr = n->right;
    rbt_node_t *nrl = nr->left;

    assert(nr != NULL);

    if (p == NULL) {
        rbt->root = nr;

    } else if (n == p->left) {
        p->left = nr;
    } else {
        p->right = nr;
    }
    nr->parent = p;

    n->parent = nr;
    nr->left = n;

    if (nrl != NULL) {
        nrl->parent = n;
    }
    n->right = nrl;
}

inline static void
_rbt_rotate_right(rbt_t *rbt, rbt_node_t *n)
{
    rbt_node_t *p = n->parent;
    rbt_node_t *nl = n->left;
    rbt_node_t *nlr = nl->right;

    assert(nl != NULL);

    if (p == NULL) {
        rbt->root = nl;

    } else if (n == p->left) {
        p->left = nl;
    } else {
        p->right = nl;
    }
    nl->parent = p;

    n->parent = nl;
    nl->right = n;

    if (nlr != NULL) {
        nlr->parent = n;
    }
    n->left = nlr;
}


inline static void
_rbt_adjust_on_insert(rbt_t *rbt, rbt_node_t *node)
{
    rbt_node_t *p, *g, *u;

    /*
     * Case 1.
     */
    if (node == rbt->root) {
        RBT_SETBLACK(node);
        //TRACE("Case 1");
        return;
    }

    /*
     * Case 2.
     */
    if (RBT_ISBLACK(node->parent)) {
        //TRACE("Case 2");
        return;
    }

    /* Relatives */
    p = node->parent;
    //assert(p != NULL);
    g = p->parent;
    if (g == NULL) {
        u = NULL;
    } else {
        if (g->left != node->parent) {
            u = g->left;
        } else {
            u = g->right;
        }
    }
    //u = g != NULL ? (g->left != node->parent ? g->left : g->right) : NULL;

    //assert(RBT_ISRED(p));

    /*
     * Case 3.
     */
    if (RBT_ISRED(u)) {
        RBT_SETBLACK(p);
        RBT_SETBLACK(u);
        RBT_SETRED(g);
        _rbt_adjust_on_insert(rbt, g);
        //TRACE("Case 3");
        return;
    }

    assert(RBT_ISBLACK(u));

    if (g == NULL) {
        return;
    }
    /*
     * Case 4.
     */
    //TRACE("node=%p p=%p g=%p", node, p, g);
    if (node == p->right && p == g->left) {
        //TRACE("Case 4 left");
        _rbt_rotate_left(rbt, p);
        /* recalc after rotation */
        p = node->parent;
        assert(p != NULL);
        g = p->parent;
        //TRACE("Case 4.1");

    } else if (node == p->left && p == g->right) {
        //TRACE("Case 4 right");
        _rbt_rotate_right(rbt, p);
        /* recalc after rotation */
        p = node->parent;
        assert(p != NULL);
        g = p->parent;
        //TRACE("Case 4.2");
    }

    /* fall through to case 5. */

    /*
     * Case 5.
     */
    if (g != NULL /* && p != NULL */ ) {
        if (node == p->left && p == g->left) {
            //TRACE("Case 5 right");
            _rbt_rotate_right(rbt, g);
        } else if (node == p->right && p == g->right) {
            //TRACE("Case 5 left");
            _rbt_rotate_left(rbt, g);
        }
        RBT_SETBLACK(p);
        RBT_SETRED(g);
        //TRACE("Case 5");
    }
}

inline int
rbt_insert(rbt_t *rbt, rbt_node_t *node, rbt_node_t **dup)
{
    int res;

    *dup = NULL;

    if ((res = _rbt_insert(rbt, rbt->root, node, dup)) == 0) {
        assert(*dup != NULL);
        return 0;
    }
    //TRACE("res=%d", res);
    _rbt_adjust_on_insert(rbt, node);
    return res;
}

static void
_rbt_adjust_on_remove(rbt_t *rbt, rbt_node_t *node)
{
    rbt_node_t *sibling, *parent, *sl, *sr;

    /*
     * Case 1.
     */
    if (node->parent == NULL) {
        //TRACE("Case 1");
        return;
    }

    /*
     * Case 2.
     */
    parent = node->parent;
    if (node == parent->left) {
        sibling = parent->right;
        if (RBT_ISRED(sibling)) {
            RBT_SETBLACK(sibling);
            RBT_SETRED(parent);
            _rbt_rotate_left(rbt, parent);

            /* re-calc for the case 3 */
            parent = node->parent;
            sibling = node == parent->left ?
                              parent->right :
                              parent->left;
            //TRACE("Case 2.0");
        }
    } else {
        sibling = parent->left;
        if (RBT_ISRED(sibling)) {
            RBT_SETBLACK(sibling);
            RBT_SETRED(parent);
            _rbt_rotate_right(rbt, parent);

            /* re-calc for the case 3 */
            parent = node->parent;
            sibling = node == parent->left ?
                              parent->right :
                              parent->left;
            //TRACE("Case 2.1");
        }
    }

    if (sibling == NULL) {
        //TRACE("Early return after Case 2.");
        return;
    }

    sl = sibling->left != NULL ? sibling->left : NULL;
    sr = sibling->right != NULL ? sibling->right : NULL;

    /*
     * Case 3.
     */
    if (RBT_ISBLACK(parent) &&
        RBT_ISBLACK(sibling) &&
        RBT_ISBLACK(sl) &&
        RBT_ISBLACK(sr)) {

        RBT_SETRED(sibling);
        assert(parent != NULL);
        //TRACE("Case 3");
        _rbt_adjust_on_remove(rbt, parent);
        return;
    }

    /*
     * Case 4.
     */
    if (RBT_ISRED(parent) &&
        RBT_ISBLACK(sibling) &&
        RBT_ISBLACK(sl) &&
        RBT_ISBLACK(sr)) {

        RBT_SETBLACK(parent);
        RBT_SETRED(sibling);
        //TRACE("Case 4");
        return;
    }

    /*
     * Case 5.
     */
    if (RBT_ISBLACK(sibling)) {
        if (node == parent->left &&
            RBT_ISRED(sl) &&
            RBT_ISBLACK(sr)) {

            RBT_SETRED(sibling);
            if (sl != NULL) {
                RBT_SETBLACK(sl);
            }
            _rbt_rotate_right(rbt, sibling);

            /* re-calc after rotate */
            parent = node->parent;
            sibling = node == parent->left ?
                              parent->right :
                              parent->left;
            sl = sibling->left != NULL ? sibling->left : NULL;
            sr = sibling->right != NULL ? sibling->right : NULL;
            //TRACE("Case 5.1");

        } else if (node == parent->right &&
                   RBT_ISRED(sr) &&
                   RBT_ISBLACK(sl)) {

            RBT_SETRED(sibling);
            if (sr != NULL) {
                RBT_SETBLACK(sr);
            }
            _rbt_rotate_left(rbt, sibling);

            /* re-calc after rotate */
            parent = node->parent;
            sibling = node == parent->left ?
                              parent->right :
                              parent->left;
            sl = sibling->left != NULL ? sibling->left : NULL;
            sr = sibling->right != NULL ? sibling->right : NULL;
            //TRACE("Case 5.2");
        }
    }

    /*
     * Case 6.
     */
    RBT_SETCOLOR(sibling, parent);
    RBT_SETBLACK(parent);

    if (node == parent->left) {
        if (sr != NULL) {
            RBT_SETBLACK(sr);
        }
        _rbt_rotate_left(rbt, parent);
        //TRACE("Case 6.1");
    } else {
        if (sl != NULL) {
            RBT_SETBLACK(sl);
        }
        _rbt_rotate_right(rbt, parent);
        //TRACE("Case 6.2");
    }
}

static void
_rbt_remove_list(rbt_t *rbt, rbt_node_t *node)
{
    if (node->next != NULL) {
        node->next->prev = node->prev;
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
    } else {
        rbt->tail = node->prev;
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
    } else {
        rbt->head = node->next;
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
    }

    node->prev = NULL;
    node->next = NULL;
}

int rbt_remove_node(rbt_t *rbt, rbt_node_t *node)
{
    rbt_node_t *nodeparent;
    rbt_node_t *repl, *replchild, *replparent;

    /*
     * Find a replacement and remove it from the tree, that is put one of
     * its children in its place.
     *
     * If the replacement is NULL, the node is a leaf. Otherwise the
     * replacement always has parent, including the node itself. Also if
     * the replacement is NULL, the replacement's child must be NULL by
     * definition. IOW, if the node is leaf, all of repl, replparent, and
     * replchild are NULL, otherwise repl and replparent must be non-NULL,
     * and replchild may be any.
     *
     * If the replacement has no children, either replacement's parent's
     * left or right will be set to NULL.
     *
     */
    if (node->left != NULL) {
        repl = node->left;

        while (repl->right != NULL) {
            repl = repl->right;
        }

        assert(repl->right == NULL);
        replchild = repl->left;
        //TRACE("repl is %02lx", repl->key);

    } else if (node->right != NULL) {
        repl = node->right;

        while (repl->left != NULL) {
            repl = repl->left;
        }

        assert(repl->left == NULL);
        replchild = repl->right;
        //TRACE("repl is %02lx", repl->key);

    } else {
        /* node is the leaf */
        //TRACE("node %02lx is a leaf ...", node->key);
        repl = NULL;
        replchild = NULL;
        //TRACE("repl is NULL");
    }

    replparent = repl == NULL ? NULL : repl->parent;


    /* put replchld in place of repl */

    if (replparent == NULL) {
        /* node is a leaf */
        assert(repl == NULL);
    } else {
        if (repl == replparent->left) {
            replparent->left = replchild;
        } else {
            replparent->right = replchild;
        }
    }
    if (replchild != NULL) {
        replchild->parent = replparent;
    }

    /*
     * Perform adjust on removal.
     */

    if (RBT_ISBLACK(repl)) {
        if (RBT_ISRED(replchild)) {
            RBT_SETBLACK(replchild);
        } else {
            /* replchild must be black */
            if (replchild != NULL) {
                _rbt_adjust_on_remove(rbt, replchild);
            } else {
                //TRACE("replchild is NULL, node %02lx is leaf?",
                //      repl != NULL ? repl->key : 0xfefe);
            }
        }
    }

    /*
     * Remove node and put replacment in place of node.  The Wikipaedia
     * says to copy values of repl to the node being removed. We here do
     * actual replacement (and copying node's color into that of repl)
     * since rbt_node_t is assumed to be a part of a larger structure
     * which is actually referenced in the node->data.
     */

    nodeparent = node->parent;
    if (nodeparent == NULL) {
        rbt->root = repl;
    } else {
        if (node == nodeparent->left) {
            nodeparent->left = repl;
        } else {
            nodeparent->right = repl;
        }
    }

    if (node->left != NULL) {
        node->left->parent = repl;
    }

    if (node->right != NULL) {
        node->right->parent = repl;
    }

    if (repl != NULL) {
        repl->parent = nodeparent;
        repl->left = node->left;
        repl->right = node->right;
        repl->flags = node->flags;
    }

    _rbt_remove_list(rbt, node);

    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    --rbt->nelem;
    return 0;
}

int
rbt_remove_key(rbt_t *rbt, uintptr_t key)
{
    rbt_node_t *node;

    rbt_find(rbt, key, &node);

    if (node == NULL) {
        return 1;
    }

    return rbt_remove_node(rbt, node);

}

