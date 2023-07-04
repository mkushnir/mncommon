#ifndef MNCOMMON_DTQUEUE_H
#define MNCOMMON_DTQUEUE_H

#include <assert.h>
#include <stddef.h>

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DTQUEUE(ty, n)         \
struct {                       \
    struct ty *dtq_head;       \
    struct ty *dtq_tail;       \
    size_t nelems;             \
} n                            \


#define DTQUEUE_ENTRY(ty, n)   \
struct {                       \
    struct ty *dtq_next;       \
    struct ty *dtq_prev;       \
} n                            \


#define DTQUEUE_INIT(q)        \
    do {                       \
        (q)->dtq_head = NULL;  \
        (q)->dtq_tail = NULL;  \
        (q)->nelems = 0;       \
    } while (0)                \


#define DTQUEUE_FINI DTQUEUE_INIT

#define DTQUEUE_INITIALIZER {NULL, NULL, 0}


#define DTQUEUE_ENTRY_INIT(link, entry)\
    do {                               \
        (entry)->link.dtq_next = NULL; \
        (entry)->link.dtq_prev = NULL; \
    } while (0)                        \


#define DTQUEUE_ENTRY_FINI DTQUEUE_ENTRY_INIT

#define DTQUEUE_ENTRY_INITIALIZER {NULL, NULL}


#define DTQUEUE_ENQUEUE(q, link, entry)                \
    do {                                               \
        if ((q)->dtq_tail == NULL) {                   \
            (q)->dtq_head = entry;                     \
            (q)->dtq_tail = entry;                     \
        } else {                                       \
            (q)->dtq_tail->link.dtq_next = (entry);    \
            (entry)->link.dtq_prev = (q)->dtq_tail;    \
            (entry)->link.dtq_next = NULL;             \
            (q)->dtq_tail = (entry);                   \
        }                                              \
        ++((q)->nelems);                               \
    } while (0)                                        \


#define DTQUEUE_HEAD(q) ((q)->dtq_head)

#define DTQUEUE_NEXT(link, e) ((e)->link.dtq_next)

#define DTQUEUE_PREV(link, e) ((e)->link.dtq_prev)

#define DTQUEUE_TAIL(q) ((q)->dtq_tail)

#define DTQUEUE_LENGTH(q) ((q)->nelems)

#define DTQUEUE_DEQUEUE(q, link)                               \
    do {                                                       \
        if ((q)->dtq_head != NULL) {                           \
            PASTEURIZE_ADDR((q)->dtq_head);                    \
            (q)->dtq_head = (q)->dtq_head->link.dtq_next;      \
            PASTEURIZE_ADDR((q)->dtq_head);                    \
            if ((q)->dtq_head == NULL) {                       \
                (q)->dtq_tail = NULL;                          \
            } else {                                           \
                (q)->dtq_head->link.dtq_prev = NULL;           \
            }                                                  \
            --((q)->nelems);                                   \
        } else {                                               \
            (q)->dtq_tail = NULL;                              \
        }                                                      \
    } while (0)                                                \


#define DTQUEUE_POP(q, link)                                   \
    do {                                                       \
        if ((q)->dtq_tail != NULL) {                           \
            PASTEURIZE_ADDR((q)->dtq_tail);                    \
            (q)->dtq_tail = (q)->dtq_tail->link.dtq_prev;      \
            PASTEURIZE_ADDR((q)->dtq_tail);                    \
            if ((q)->dtq_tail == NULL) {                       \
                (q)->dtq_head = NULL;                          \
            } else {                                           \
                (q)->dtq_tail->link.dtq_next = NULL;           \
            }                                                  \
            --((q)->nelems);                                   \
        } else {                                               \
            (q)->dtq_head = NULL;                              \
        }                                                      \
    } while (0)                                                \


/*
 * XXX DANGEROUS! No check for NULL. Can only be used in:
 *
 *  while ((e = DTQUEUE_HEAD(q)) != NULL) {
 *      [ do something with e, except free(e) ]
 *      DTQUEUE_DEQUEUE_FAST(q, link);
 *  }
 *
 *  -- right before DTQUEUE_FINI(q). DTQUEUE_DEQUEUE_FAST() leaves a queue
 *  in a state where (dtq_head == NULL) AND (dtq_tail != NULL). In this state a
 *  subsequent DTQUEUE_* operations may lead to memory access error.
 */
#define DTQUEUE_DEQUEUE_FAST(q, link)                  \
    do {                                               \
        (q)->dtq_head = (q)->dtq_head->link.dtq_next;  \
        if ((q)->dtq_head != NULL) {                   \
            (q)->dtq_head->link.dtq_prev = NULL;       \
        }                                              \
        --((q)->nelems);                               \
    } while (0)                                        \


#define DTQUEUE_POP_FAST(q, link)                      \
    do {                                               \
        (q)->dtq_tail = (q)->dtq_tail->link.dtq_prev;  \
        if ((q)->dtq_tail != NULL) {                   \
            (q)->dtq_tail->link.dtq_next = NULL;       \
        }                                              \
        --((q)->nelems);                               \
    } while (0)                                        \


#define DTQUEUE_REMOVE_DIRTY(q, link, e)                                       \
    do {                                                                       \
        if ((q)->dtq_head != NULL && (q)->dtq_tail != NULL) {                  \
            if ((e)->link.dtq_prev != NULL) {                                  \
                if ((e)->link.dtq_next != NULL) {                              \
                    ((e)->link.dtq_prev)->link.dtq_next = (e)->link.dtq_next;  \
                    ((e)->link.dtq_next)->link.dtq_prev = (e)->link.dtq_prev;  \
                } else {                                                       \
                    (q)->dtq_tail = (e)->link.dtq_prev;                        \
                    (q)->dtq_tail->link.dtq_next = NULL;                       \
                }                                                              \
            } else {                                                           \
                if ((e)->link.dtq_next != NULL) {                              \
                    (q)->dtq_head = (e)->link.dtq_next;                        \
                    (q)->dtq_head->link.dtq_prev = NULL;                       \
                } else {                                                       \
                    (q)->dtq_tail = NULL;                                      \
                    (q)->dtq_head = NULL;                                      \
                }                                                              \
            }                                                                  \
            --((q)->nelems);                                                   \
        }                                                                      \
    } while (0)                                                                \


#define DTQUEUE_REMOVE(q, link, e)                                     \
    DTQUEUE_REMOVE_DIRTY(q, link, e); DTQUEUE_ENTRY_FINI(link, e)      \


#define DTQUEUE_ORPHAN(q, link, e) (((q)->dtq_head != (e)) && ((e)->link.dtq_prev == NULL) && ((e)->link.dtq_next == NULL))

#define DTQUEUE_EMPTY(q) ((q)->dtq_head == NULL && (q)->dtq_tail == NULL)

#define DTQUEUE_INSERT_AFTER(q, link, a, e)            \
    do {                                               \
        assert((a) != NULL && (e) != NULL);            \
        if ((a)->link.dtq_next == NULL) {              \
            (q)->dtq_tail = (e);                       \
            (e)->link.dtq_next = NULL;                 \
            (a)->link.dtq_next = (e);                  \
            (e)->link.dtq_prev = (a);                  \
        } else {                                       \
            (e)->link.dtq_next = (a)->link.dtq_next;   \
            (a)->link.dtq_next = (e);                  \
            ((e)->link.dtq_next)->link.dtq_prev = (e); \
            (e)->link.dtq_prev = (a);                  \
        }                                              \
        ++((q).nelems);                                \
    } while (0)                                        \


#define DTQUEUE_INSERT_BEFORE(q, link, b, e)           \
    do {                                               \
        assert((b) != NULL && (e) != NULL);            \
        if ((b)->link.dtq_prev == NULL) {              \
            (q)->dtq_head = (e);                       \
            (e)->link.dtq_prev = NULL;                 \
            (b)->link.dtq_prev = (e);                  \
            (e)->link.dtq_next = (b);                  \
        } else {                                       \
            (e)->link.dtq_prev = (b)->link.dtq_prev;   \
            (b)->link.dtq_prev = (e);                  \
            ((e)->link.dtq_prev)->link.dtq_next = (e); \
            (e)->link.dtq_next = (b);                  \
        }                                              \
        ++((q)->nelems);                               \
    } while (0)                                        \


#ifdef __cplusplus
}
#endif

#endif
// vim:list
