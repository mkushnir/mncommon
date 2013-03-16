#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"

#include "mrkcommon/dumpm.h"
#include "mrkcommon/logging.h"

LOGGING_DECLARE(NULL, 0, LOG_TRACE);

static void
test0(void)
{
    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }
    DEBUG("1");
    LOGGING_SETLEVEL(LOG_DEBUG);
    DEBUG("2");
    LOGGING_CLEARLEVEL(LOG_DEBUG);
    DEBUG("3");
}

int
main(void)
{
    logging_init(stdout, "TL", LOG_PID);
    test0();
    logging_fini();
    logging_init(NULL, "TL", LOG_PID);
    test0();
    logging_fini();
    return 0;
}

