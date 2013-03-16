#ifndef TRIE_H
#define TRIE_H

#include <stdint.h>
#include <sys/limits.h>

#define TREE_DEPTH (sizeof(uintptr_t) * 8)

typedef struct _trie_node {
    struct _trie_node *parent;
    struct _trie_node *child[2];
    void *value;
#   define CHILD_SELECTOR_SHIFT (sizeof(int) * 8 - 1)
    int digit;
} trie_node_t;

typedef struct _trie {
    /* flsl(3) can return TREE_DEPTH + 1 values for any of its input */
    struct _trie_node roots[TREE_DEPTH + 1];
} trie_t;

typedef struct _test_data {
    uint64_t key;
} test_data_t;

void trie_node_dump(trie_node_t *);
int trie_node_dump_cb(trie_node_t *, void *);
void trie_init(trie_t *);
void trie_fini(trie_t *);
typedef int (*trie_traverser_t)(trie_node_t *, void *);
int trie_node_traverse(trie_node_t *, trie_traverser_t, void *);
int trie_traverse(trie_t *, int (*)(trie_node_t *, void *), void *);

trie_node_t *trie_add_node(trie_t *, uintptr_t);
int trie_node_remove(trie_node_t *);

trie_node_t *trie_find_exact(trie_t *, uintptr_t);
trie_node_t *trie_find_closest(trie_t *, uintptr_t, int);

#define TRIE_MIN(t) trie_find_closest((t), 0, 1)
#define TRIE_MAX(t) trie_find_closest((t), ULONG_MAX, 0)

#endif

