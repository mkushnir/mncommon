#ifndef MNCOMMON_RBT_H
#define MNCOMMON_RBT_H

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
} mnrbt_node_t;

typedef struct _rbt {
    mnrbt_node_t *root;
    mnrbt_node_t *head;
    mnrbt_node_t *tail;
    size_t nelem;
} mnrbt_t;


int rbt_init(mnrbt_t *);
int rbt_fini(mnrbt_t *);
int rbt_dump_tree(mnrbt_t *);
int rbt_dump_list(mnrbt_t *);
int rbt_find(mnrbt_t *, uintptr_t, mnrbt_node_t **);
int rbt_insert(mnrbt_t *, mnrbt_node_t *, mnrbt_node_t **);
#define RBT_DUPLICATE -1

int rbt_remove_node(mnrbt_t *, mnrbt_node_t *);
int rbt_remove_key(mnrbt_t *, uintptr_t);

int rbt_node_init(mnrbt_node_t *, uintptr_t, void *);
void rbt_node_dump_tree(mnrbt_node_t *, int);
void rbt_node_dump_list(mnrbt_node_t *);

#ifdef __cplusplus
}
#endif

#endif
