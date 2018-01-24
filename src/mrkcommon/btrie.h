#ifndef MRKCOMMON_BTRIE_H
#define MRKCOMMON_BTRIE_H

#include <stdint.h>
#include <limits.h>

#include <mrkcommon/mpool.h>

#define TREE_DEPTH (sizeof(uintmax_t) * 8)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _test_data {
    uintmax_t key;
} test_data_t;

typedef struct _mnbtrie_node {
    struct _mnbtrie_node *parent;
    struct _mnbtrie_node *child[2];
    void *value;
    /*
     * digit is the node level | (CHILD_SELECTOR_SHIFT << selector)
     */
#   define CHILD_SELECTOR_SHIFT (sizeof(unsigned short) * 8 - 1)
    unsigned short digit;
    char idx;
} mnbtrie_node_t;

typedef struct _mntrie {
    /* flsl(3) can return TREE_DEPTH + 1 values for any of its input */
    struct _mnbtrie_node roots[TREE_DEPTH];
    size_t volume;
    size_t nvals;
} mnbtrie_t;

void btrie_node_dump(mnbtrie_node_t *);
int btrie_node_dump_cb(mnbtrie_node_t *, void *);
void btrie_init(mnbtrie_t *);
void btrie_fini(mnbtrie_t *);
void btrie_fini_mpool(mpool_ctx_t *, mnbtrie_t *);
typedef int (*btrie_traverser_t)(mnbtrie_node_t *, void *);
int btrie_node_traverse(mnbtrie_node_t *, btrie_traverser_t, void *);
int btrie_traverse(mnbtrie_t *, int (*)(mnbtrie_node_t *, void *), void *);

mnbtrie_node_t *btrie_add_node(mnbtrie_t *, uintmax_t);
mnbtrie_node_t *btrie_add_node_mpool(mpool_ctx_t *, mnbtrie_t *, uintmax_t);
int btrie_remove_node_no_cleanup(mnbtrie_t *, mnbtrie_node_t *);
int btrie_remove_node_no_cleanup_mpool(mpool_ctx_t *, mnbtrie_t *, mnbtrie_node_t *);
int btrie_remove_node(mnbtrie_t *, mnbtrie_node_t *);
int btrie_remove_node_mpool(mpool_ctx_t *, mnbtrie_t *, mnbtrie_node_t *);
size_t btrie_get_volume(mnbtrie_t *);
size_t btrie_get_nvals(mnbtrie_t *);
void btrie_cleanup(mnbtrie_t *);
void btrie_cleanup_mpool(mpool_ctx_t *, mnbtrie_t *);


mnbtrie_node_t *btrie_find_exact(mnbtrie_t *, uintmax_t);
mnbtrie_node_t *btrie_find_closest(mnbtrie_t *, uintmax_t, int);

#define BTRIE_MIN(t) btrie_find_closest((t), 1, 1)
#define BTRIE_MAX(t) btrie_find_closest((t), UINTMAX_MAX, 0)

#ifdef __cplusplus
}
#endif

#endif

