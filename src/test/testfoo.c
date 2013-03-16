#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/array.h"
#include "mrkcommon/list.h"

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
}

int
main(void)
{
    test0();
    return 0;
}
