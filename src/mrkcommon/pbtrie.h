#ifndef MRKCOMMON_PBTRIE_H
#define MRKCOMMON_PBTRIE_H

#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _mnpbtrie_node {
    uint64_t xmask;
    uint64_t prefix;
    void *value;
    struct _mnpbtrie_node *parent;
    struct _mnpbtrie_node *child[2];
} mnpbtrie_node_t;


typedef struct _mnpbtrie_item {
    void *value;
} mnpbtrie_item_t;


typedef int (*mnpbtrie_traverser_t)(mnpbtrie_node_t *, void *);

typedef struct _mnpbtrie {
    struct _mnpbtrie_node *root;
    struct _mnpbtrie_node *head;
    struct _mnpbtrie_node *tail;
    size_t volume;
    size_t nvals;
} mnpbtrie_t;


void pbtrie_init(mnpbtrie_t *);
void pbtrie_fini(mnpbtrie_t *);
void pbtrie_dump(mnpbtrie_t *);
int pbtrie_traverse(mnpbtrie_t *, mnpbtrie_traverser_t, void *);
int pbtrie_reverse(mnpbtrie_t *, mnpbtrie_traverser_t, void *);
mnpbtrie_node_t *pbtrie_node_min(mnpbtrie_node_t *);
mnpbtrie_node_t *pbtrie_node_max(mnpbtrie_node_t *);
mnpbtrie_node_t *pbtrie_node_next(mnpbtrie_node_t *);
mnpbtrie_node_t *pbtrie_node_prev(mnpbtrie_node_t *);
mnpbtrie_node_t *pbtrie_find_exact(mnpbtrie_t *, uint64_t);
int pbtrie_remove_node(mnpbtrie_t *, mnpbtrie_node_t *);

mnpbtrie_node_t *pbtrie_add_node(mnpbtrie_t *, uint64_t);

#ifdef __cplusplus
}
#endif

#endif
