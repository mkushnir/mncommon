#include <assert.h>
#include <stdlib.h>

#include "unittest.h"
#include "diag.h"
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/dtqueue.h>

typedef struct _asd {
    DTQUEUE_ENTRY(_asd, link);
} asd_t;

typedef struct _qwe {
    DTQUEUE(_asd, q);
} qwe_t;

int
main(void)
{
    int i;
    qwe_t a;
    asd_t *elm;
    DTQUEUE(_asd, qq);

    DTQUEUE_INIT(&a.q);
    DTQUEUE_INIT(&qq);

    for (i = 0; i < 10; ++i) {
        elm = malloc(sizeof(asd_t));
        DTQUEUE_ENTRY_INIT(link, elm);
        DTQUEUE_ENQUEUE(&a.q, link, elm);
        DTQUEUE_ENQUEUE(&qq, link, elm);
    }

    TRACE("a.q length=%ld", DTQUEUE_LENGTH(&a.q));
    TRACE("qq length=%ld", DTQUEUE_LENGTH(&qq));

    while ((elm = DTQUEUE_HEAD(&a.q)) != NULL) {
        TRACE("elm=%p", elm);
        DTQUEUE_DEQUEUE(&a.q, link);
    }

    TRACE("head=%p tail=%p", DTQUEUE_HEAD(&a.q), DTQUEUE_TAIL(&a.q));
    TRACE("a.q length=%ld", DTQUEUE_LENGTH(&a.q));

    while ((elm = DTQUEUE_HEAD(&qq)) != NULL) {
        TRACE("elm=%p", elm);
        DTQUEUE_DEQUEUE_FAST(&qq, link);
        free(elm);
    }

    TRACE("head=%p tail=%p", DTQUEUE_HEAD(&qq), DTQUEUE_TAIL(&qq));
    TRACE("qq length=%ld", DTQUEUE_LENGTH(&qq));
    DTQUEUE_FINI(&qq);
    TRACE("head=%p tail=%p", DTQUEUE_HEAD(&qq), DTQUEUE_TAIL(&qq));
    return 0;
}

// vim:list
