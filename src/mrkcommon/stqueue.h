#ifndef MRKCOMMON_STQUEUE_H
#define MRKCOMMON_STQUEUE_H

#include <assert.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STQUEUE(ty, n) \
struct { \
    struct ty *head; \
    struct ty *tail; \
    size_t nelems; \
} n

#define STQUEUE_ENTRY(ty, n) \
struct { \
    struct ty *next; \
} n

#define STQUEUE_INIT(q) \
    do { \
        (q)->head = NULL; \
        (q)->tail = NULL; \
        (q)->nelems = 0; \
    } while (0)

#define STQUEUE_FINI STQUEUE_INIT

#define STQUEUE_ENTRY_INIT(link, e) \
    do { \
        (e)->link.next = NULL; \
    } while (0)

#define STQUEUE_ENTRY_FINI STQUEUE_ENTRY_INIT

#define STQUEUE_ENQUEUE(q, link, entry) \
    do { \
        if ((q)->tail == NULL) { \
            (q)->head = entry; \
            (q)->tail = entry; \
        } else { \
            (q)->tail->link.next = (entry); \
            (entry)->link.next = NULL; \
            (q)->tail = (entry); \
        } \
        ++((q)->nelems); \
    } while (0)

#define STQUEUE_HEAD(q) ((q)->head)

#define STQUEUE_TAIL(q) ((q)->tail)

#define STQUEUE_LENGTH(q) ((q)->nelems)

#define STQUEUE_DEQUEUE(q, link) \
    do { \
        if ((q)->head != NULL) { \
            (q)->head = (q)->head->link.next; \
            if ((q)->head == NULL) { \
                (q)->tail = NULL; \
            } \
            --((q)->nelems); \
        } else { \
            (q)->tail = NULL; \
        } \
    } while (0)

/*
 * XXX DANGEROUS! No check for NULL. Can only be used in:
 *
 *  while ((e = STQUEUE_HEAD(q)) != NULL) {
 *      [ do something with e, except free(e) ]
 *      STQUEUE_DEQUEUE_FAST(q, link);
 *  }
 *
 *  -- right before STQUEUE_FINI(q). STQUEUE_DEQUEUE_FAST() leaves a queue
 *  in a state where (head == NULL) AND (tail != NULL). In this state a
 *  subsequent STQUEUE_ENQUEUE() may lead to memory access error.
 */
#define STQUEUE_DEQUEUE_FAST(q, link) \
    do { \
        (q)->head = (q)->head->link.next; \
        --((q)->nelems); \
    } while (0)

#define STQUEUE_ORPHAN(q, link, e) (((q)->tail != (e)) && ((e)->link.next == NULL))

#define STQUEUE_EMPTY(q) ((q)->head == NULL && (q)->tail == NULL)

#define STQUEUE_INSERT_AFTER(q, link, a, e) \
    do { \
        assert((a) != NULL && (e) != NULL); \
        if ((a)->link.next == NULL) { \
            (q)->tail = (e); \
            (e)->link.next = NULL; \
            (a)->link.next = (e); \
        } \ else { \
            (e)->link.next = (a)->link.next; \
            (a)->link.next = (e); \
        } \
        ++((q)->nelems); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
// vim:list
