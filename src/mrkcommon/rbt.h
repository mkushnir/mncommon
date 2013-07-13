#ifndef MRKCOMMON_RBT_H
#define MRKCOMMON_RBT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _rbt_node {
    struct _rbt_node *parent;
    struct _rbt_node *left;
    struct _rbt_node *right;
    struct _rbt_node *prev;
    struct _rbt_node *next;
#define RBT_FLAG_RED 0x0000000100000000
    uintptr_t flags;
    uintptr_t key;
    void *data;
} rbt_node_t;

typedef struct _rbt {
    rbt_node_t *root;
    rbt_node_t *head;
    rbt_node_t *tail;
    size_t nelem;
} rbt_t;


int rbt_init(rbt_t *);
int rbt_fini(rbt_t *);
int rbt_dump_tree(rbt_t *);
int rbt_dump_list(rbt_t *);
int rbt_find(rbt_t *, uintptr_t, rbt_node_t **);
int rbt_insert(rbt_t *, rbt_node_t *, rbt_node_t **);
#define RBT_DUPLICATE -1

int rbt_remove_node(rbt_t *, rbt_node_t *);
int rbt_remove_key(rbt_t *, uintptr_t);

int rbt_node_init(rbt_node_t *, uintptr_t, void *);
void rbt_node_dump_tree(rbt_node_t *, int);
void rbt_node_dump_list(rbt_node_t *);

#ifdef __cplusplus
}
#endif

#endif
