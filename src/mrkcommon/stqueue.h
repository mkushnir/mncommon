#ifndef MRKCOMMON_STQUEUE_H
#define MRKCOMMON_STQUEUE_H

#include <assert.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STQUEUE(ty, n)         \
struct {                       \
    struct ty *stq_head;       \
    struct ty *stq_tail;       \
    size_t nelems;             \
} n                            \


#define STQUEUE_ENTRY(ty, n)   \
struct {                       \
    struct ty *stq_next;       \
} n                            \


#define STQUEUE_INIT(q)        \
    do {                       \
        (q)->stq_head = NULL;  \
        (q)->stq_tail = NULL;  \
        (q)->nelems = 0;       \
    } while (0)                \


#define STQUEUE_FINI STQUEUE_INIT


#define STQUEUE_ENTRY_INIT(link, e)    \
    do {                               \
        (e)->link.stq_next = NULL;     \
    } while (0)                        \


#define STQUEUE_ENTRY_FINI STQUEUE_ENTRY_INIT


#define STQUEUE_ENQUEUE(q, link, entry)                \
    do {                                               \
        if ((q)->stq_tail == NULL) {                   \
            (q)->stq_head = entry;                     \
            (q)->stq_tail = entry;                     \
        } else {                                       \
            (q)->stq_tail->link.stq_next = (entry);    \
            (entry)->link.stq_next = NULL;             \
            (q)->stq_tail = (entry);                   \
        }                                              \
        ++((q)->nelems);                               \
    } while (0)                                        \


#define STQUEUE_HEAD(q) ((q)->stq_head)

#define STQUEUE_TAIL(q) ((q)->stq_tail)

#define STQUEUE_LENGTH(q) ((q)->nelems)

#define STQUEUE_DEQUEUE(q, link)                               \
    do {                                                       \
        if ((q)->stq_head != NULL) {                           \
            (q)->stq_head = (q)->stq_head->link.stq_next;      \
            if ((q)->stq_head == NULL) {                       \
                (q)->stq_tail = NULL;                          \
            }                                                  \
            --((q)->nelems);                                   \
        } else {                                               \
            (q)->stq_tail = NULL;                              \
        }                                                      \
    } while (0)                                                \


/*
 * XXX DANGEROUS! No check for NULL. Can only be used in:
 *
 *  while ((e = STQUEUE_HEAD(q)) != NULL) {
 *      [ do something with e, except free(e) ]
 *      STQUEUE_DEQUEUE_FAST(q, link);
 *  }
 *
 *  -- right before STQUEUE_FINI(q). STQUEUE_DEQUEUE_FAST() leaves a queue
 *  in a state where (stq_head == NULL) AND (stq_tail != NULL). In this state a
 *  subsequent STQUEUE_ENQUEUE() may lead to memory access error.
 */
#define STQUEUE_DEQUEUE_FAST(q, link)                  \
    do {                                               \
        (q)->stq_head = (q)->stq_head->link.stq_next;  \
        --((q)->nelems);                               \
    } while (0)                                        \


#define STQUEUE_ORPHAN(q, link, e) (((q)->stq_tail != (e)) && ((e)->link.stq_next == NULL))


#define STQUEUE_EMPTY(q) ((q)->stq_head == NULL && (q)->stq_tail == NULL)


#define STQUEUE_INSERT_AFTER(q, link, a, e)            \
    do {                                               \
        assert((a) != NULL && (e) != NULL);            \
        if ((a)->link.stq_next == NULL) {              \
            (q)->stq_tail = (e);                       \
            (e)->link.stq_next = NULL;                 \
            (a)->link.stq_next = (e);                  \
        } \ else {                                     \
            (e)->link.stq_next = (a)->link.stq_next;   \
            (a)->link.stq_next = (e);                  \
        }                                              \
        ++((q)->nelems);                               \
    } while (0)                                        \


#ifdef __cplusplus
}
#endif

#endif
// vim:list
