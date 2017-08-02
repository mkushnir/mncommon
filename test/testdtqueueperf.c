#include <assert.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "unittest.h"
#include "diag.h"
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/dtqueue.h>


#ifndef NDEBUG
#define NGX_DEBUG 1
#endif



/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
typedef struct ngx_queue_s  ngx_queue_t;

struct ngx_queue_s {
    ngx_queue_t  *prev;
    ngx_queue_t  *next;
};


#define ngx_queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q


#define ngx_queue_empty(h)                                                    \
    (h == (h)->prev)


#define ngx_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x


#define ngx_queue_insert_after   ngx_queue_insert_head


#define ngx_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x


#define ngx_queue_head(h)                                                     \
    (h)->next


#define ngx_queue_last(h)                                                     \
    (h)->prev


#define ngx_queue_sentinel(h)                                                 \
    (h)


#define ngx_queue_next(q)                                                     \
    (q)->next


#define ngx_queue_prev(q)                                                     \
    (q)->prev


#if (NGX_DEBUG)

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next;                                              \
    (x)->prev = NULL;                                                         \
    (x)->next = NULL

#else

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

#endif


#define ngx_queue_split(h, q, n)                                              \
    (n)->prev = (h)->prev;                                                    \
    (n)->prev->next = n;                                                      \
    (n)->next = q;                                                            \
    (h)->prev = (q)->prev;                                                    \
    (h)->prev->next = h;                                                      \
    (q)->prev = n;


#define ngx_queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                              \
    (n)->next->prev = (h)->prev;                                              \
    (h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;


#define ngx_queue_data(q, type, link)                                         \
    (type *) ((u_char *) q - offsetof(type, link))





/*
 *
 */

typedef struct _mnitem {
    ngx_queue_t ngx_link;
    DTQUEUE_ENTRY(_mnitem, mrk_link);
} mnitem_t;

DTQUEUE(_mnitem, mrkq) = DTQUEUE_INITIALIZER;
ngx_queue_t ngxq;


#define N 100000000

int
main(int argc, char **argv)
{
    mnitem_t *items;
    size_t i;

    //srandom(time(NULL));

    DTQUEUE_INIT(&mrkq);
    ngx_queue_init(&ngxq);

    if ((items = malloc(sizeof(mnitem_t) * N)) == NULL) {
        errx(1, "malloc");
    }

    if (argc == 2) {
        if (strcmp(argv[1], "ngx") == 0) {
            for (i = 0; i < N; ++i) {
                ngx_queue_init(&(items + i)->ngx_link);
            }
            for (i = 0; i < N; ++i) {
                ngx_queue_insert_tail(&ngxq, &(items + i)->ngx_link);
            }
            while (!ngx_queue_empty(&ngxq)) {
                ngx_queue_t *qe;
                qe = ngx_queue_head(&ngxq);
                ngx_queue_remove(qe);
            }
        } else if (strcmp(argv[1], "mrk") == 0) {
            for (i = 0; i < N; ++i) {
                DTQUEUE_ENTRY_INIT(mrk_link, items + i);
            }
            for (i = 0; i < N; ++i) {
                DTQUEUE_ENQUEUE(&mrkq, mrk_link, items + i);
            }
            while (!DTQUEUE_EMPTY(&mrkq)) {
                UNUSED mnitem_t *item;
                item = DTQUEUE_HEAD(&mrkq);
                DTQUEUE_DEQUEUE(&mrkq, mrk_link);
            }
        } else {
            errx(1, "arg can be one of ngx/mrk\n");
        }
    }


    return 0;
}
