#ifndef MRKCOMMON_DTQUEUE_H
#define MRKCOMMON_DTQUEUE_H

#include <assert.h>
#include <stddef.h>

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DTQUEUE(ty, n) \
struct { \
    struct ty *head; \
    struct ty *tail; \
    size_t nelems; \
} n

#define DTQUEUE_ENTRY(ty, n) \
struct { \
    struct ty *next; \
    struct ty *prev; \
} n

#define DTQUEUE_INIT(q) \
    do { \
        (q)->head = NULL; \
        (q)->tail = NULL; \
        (q)->nelems = 0; \
    } while (0)

#define DTQUEUE_FINI DTQUEUE_INIT

#define DTQUEUE_ENTRY_INIT(link, entry) \
    do { \
        (entry)->link.next = NULL; \
        (entry)->link.prev = NULL; \
    } while (0)

#define DTQUEUE_ENTRY_FINI DTQUEUE_ENTRY_INIT

#define DTQUEUE_ENQUEUE(q, link, entry) \
    do { \
        if ((q)->tail == NULL) { \
            (q)->head = entry; \
            (q)->tail = entry; \
        } else { \
            (q)->tail->link.next = (entry); \
            (entry)->link.prev = (q)->tail; \
            (entry)->link.next = NULL; \
            (q)->tail = (entry); \
        } \
        ++((q)->nelems); \
    } while (0)

#define DTQUEUE_HEAD(q) ((q)->head)

#define DTQUEUE_NEXT(link, e) ((e)->link.next)

#define DTQUEUE_PREV(link, e) ((e)->link.prev)

#define DTQUEUE_TAIL(q) ((q)->tail)

#define DTQUEUE_LENGTH(q) ((q)->nelems)

#define DTQUEUE_DEQUEUE(q, link) \
    do { \
        if ((q)->head != NULL) { \
            PASTEURIZE_ADDR((q)->head); \
            (q)->head = (q)->head->link.next; \
            PASTEURIZE_ADDR((q)->head); \
            if ((q)->head == NULL) { \
                (q)->tail = NULL; \
            } else { \
                (q)->head->link.prev = NULL; \
            } \
            --((q)->nelems); \
        } else { \
            (q)->tail = NULL; \
        } \
    } while (0)

/*
 * XXX DANGEROUS! No check for NULL. Can only be used in:
 *
 *  while ((e = DTQUEUE_HEAD(q)) != NULL) {
 *      [ do something with e, except free(e) ]
 *      DTQUEUE_DEQUEUE_FAST(q, link);
 *  }
 *
 *  -- right before DTQUEUE_FINI(q). DTQUEUE_DEQUEUE_FAST() leaves a queue
 *  in a state where (head == NULL) AND (tail != NULL). In this state a
 *  subsequent DTQUEUE_* operations may lead to memory access error.
 */
#define DTQUEUE_DEQUEUE_FAST(q, link) \
    do { \
        (q)->head = (q)->head->link.next; \
        if ((q)->head != NULL) { \
            (q)->head->link.prev = NULL; \
        } \
        --((q)->nelems); \
    } while (0)

#define DTQUEUE_REMOVE_DIRTY(q, link, e) \
    do { \
        if ((q)->head != NULL && (q)->tail != NULL) { \
            if ((e)->link.prev != NULL) { \
                if ((e)->link.next != NULL) { \
                    ((e)->link.prev)->link.next = (e)->link.next; \
                    ((e)->link.next)->link.prev = (e)->link.prev; \
                } else { \
                    (q)->tail = (e)->link.prev; \
                    (q)->tail->link.next = NULL; \
                } \
            } else { \
                if ((e)->link.next != NULL) { \
                    (q)->head = (e)->link.next; \
                    (q)->head->link.prev = NULL; \
                } else { \
                    (q)->tail = NULL; \
                    (q)->head = NULL; \
                } \
            } \
            --((q)->nelems); \
        } \
    } while (0)

#define DTQUEUE_REMOVE(q, link, e) \
    DTQUEUE_REMOVE_DIRTY(q, link, e); DTQUEUE_ENTRY_FINI(link, e)

#define DTQUEUE_ORPHAN(q, link, e) (((q)->head != (e)) && ((e)->link.prev == NULL) && ((e)->link.next == NULL))

#define DTQUEUE_EMPTY(q) ((q)->head == NULL && (q)->tail == NULL)

#define DTQUEUE_INSERT_AFTER(q, link, a, e) \
    do { \
        assert((a) != NULL && (e) != NULL); \
        if ((a)->link.next == NULL) { \
            (q)->tail = (e); \
            (e)->link.next = NULL; \
            (a)->link.next = (e); \
            (e)->link.prev = (a); \
        } \ else { \
            (e)->link.next = (a)->link.next; \
            (a)->link.next = (e); \
            ((e)->link.next)->link.prev = (e); \
            (e)->link.prev = (a); \
        } \
        ++((q).nelems); \
    } while (0)

#define DTQUEUE_INSERT_BEFORE(q, link, b, e) \
    do { \
        assert((b) != NULL && (e) != NULL); \
        if ((b)->link.prev == NULL) { \
            (q)->head = (e); \
            (e)->link.prev = NULL; \
            (b)->link.prev = (e); \
            (e)->link.next = (b); \
        } \ else { \
            (e)->link.prev = (b)->link.prev; \
            (b)->link.prev = (e); \
            ((e)->link.prev)->link.next = (e); \
            (e)->link.next = (b); \
        } \
        ++((q)->nelems); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
// vim:list
