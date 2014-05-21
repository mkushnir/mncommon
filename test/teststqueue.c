#include <assert.h>
#include <stdlib.h>

#include "unittest.h"
#include "diag.h"
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/stqueue.h>

typedef struct _asd {
    STQUEUE_ENTRY(_asd, link);
} asd_t;

typedef struct _qwe {
    STQUEUE(_asd, q);
} qwe_t;

int
main(void)
{
    int i;
    qwe_t a;
    asd_t *elm;
    STQUEUE(_asd, qq);

    STQUEUE_INIT(&a.q);
    STQUEUE_INIT(&qq);

    for (i = 0; i < 10; ++i) {
        elm = malloc(sizeof(asd_t));
        STQUEUE_ENTRY_INIT(link, elm);
        STQUEUE_ENQUEUE(&a.q, link, elm);
        STQUEUE_ENQUEUE(&qq, link, elm);
    }

    TRACE("a.q length=%ld", STQUEUE_LENGTH(&a.q));
    TRACE("qq length=%ld", STQUEUE_LENGTH(&qq));

    while ((elm = STQUEUE_HEAD(&a.q)) != NULL) {
        TRACE("elm=%p", elm);
        STQUEUE_DEQUEUE(&a.q, link);
    }

    TRACE("head=%p tail=%p", STQUEUE_HEAD(&a.q), STQUEUE_TAIL(&a.q));
    TRACE("a.q length=%ld", STQUEUE_LENGTH(&a.q));

    while ((elm = STQUEUE_HEAD(&qq)) != NULL) {
        TRACE("elm=%p", elm);
        STQUEUE_DEQUEUE_FAST(&qq, link);
        free(elm);
    }

    TRACE("head=%p tail=%p", STQUEUE_HEAD(&qq), STQUEUE_TAIL(&qq));
    TRACE("qq length=%ld", STQUEUE_LENGTH(&qq));
    STQUEUE_FINI(&qq);
    TRACE("head=%p tail=%p", STQUEUE_HEAD(&qq), STQUEUE_TAIL(&qq));
    return 0;
}

// vim:list
