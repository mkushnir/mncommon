#ifndef MRKCOMMON_STQUEUE_H
#define MRKCOMMON_STQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#define STQUEUE(ty, n) \
struct _##ty##_stqueue { \
    struct ty *head; \
    struct ty *tail; \
} n

#define STQUEUE_ENTRY(ty, n) \
struct _##ty##_stqueue_entry { \
    struct ty *next; \
} n

#define STQUEUE_INIT(q) \
do { \
    (q).head = NULL; \
    (q).tail = NULL; \
} while (0)

#define STQUEUE_FINI STQUEUE_INIT

#define STQUEUE_ENTRY_INIT(entry, link) (entry)->link.next = NULL

#define STQUEUE_ENQUEUE(q, entry, link) do { \
    if ((q).tail == NULL) { \
        (q).head = entry; \
        (q).tail = entry; \
    } else { \
        (q).tail->link.next = (entry); \
        (q).tail = (entry); \
    } \
} while (0)

#define STQUEUE_HEAD(q) (q).head

#define STQUEUE_TAIL(q) (q).tail

#define STQUEUE_DEQUEUE(q, link) do { \
    if ((q).head != NULL) { \
        (q).head = (q).head->link.next; \
        if ((q).head == NULL) { \
            (q).tail = NULL; \
        } \
    } else { \
        (q).tail = NULL; \
    } \
} while (0)

#define STQUEUE_DEQUEUE_FAST(q, link) do { \
    (q).head = (q).head->link.next; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
// vim:list
