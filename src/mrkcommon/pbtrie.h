#ifndef MRKCOMMON_PBTRIE_H
#define MRKCOMMON_PBTRIE_H

#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _pbtrie_node {
    void *value;
    char prefix[64];
    struct _pbtrie_node *child[2];
} pbtrie_node_t;

typedef struct _pbtrie_item {
    void *value;
} pbtrie_item_t;

#ifdef __cplusplus
}
#endif

#endif
