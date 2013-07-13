#ifndef MRKCOMMON_LIST_H
#define MRKCOMMON_LIST_H

#include <assert.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*list_initializer_t) (void *);
typedef int (*list_finalizer_t) (void *);
typedef int (*list_traverser_t) (void *, void *);
typedef int (*list_compar_t) (const void *, const void *);

struct _dlist_entry {
    struct _dlist_entry *next;
    struct _dlist_entry *prev;
    char _data[0];
};

#define DLE_DATA(e) ((e) != NULL ? (void *)((e)->_data) : NULL)

struct _dlist {
    struct _dlist_entry *head;
    struct _dlist_entry *tail;
};

typedef struct _list {
    size_t elsz;
    size_t elnum;
    struct _dlist data;
    struct _dlist_entry **index;

    list_initializer_t init;
    list_finalizer_t fini;
    list_compar_t compar;
} list_t;

typedef struct _list_iter {
    unsigned iter;
} list_iter_t;

int list_init(list_t *, size_t, size_t,
               list_initializer_t,
               list_finalizer_t);

int list_move(list_t *, list_t *);

#define LIST_FLAG_SAVE 0x01
int list_ensure_len(list_t *, size_t, unsigned int);
void *list_get(const list_t *, unsigned);
void *list_get_iter(const list_t *, list_iter_t *);
int list_clear_item(list_t *, unsigned);
void *list_incr(list_t *);
void *list_incr_iter(list_t *, list_iter_t *);
int list_decr(list_t *);
int list_fini(list_t *);
void *list_first(const list_t *, list_iter_t *);
void *list_last(const list_t *, list_iter_t *);
void *list_next(const list_t *, list_iter_t *);
void *list_prev(const list_t *, list_iter_t *);
void *list_find(const list_t *, const void *);
int list_traverse(list_t *, list_traverser_t, void *);

#ifdef __cplusplus
}
#endif

#endif
