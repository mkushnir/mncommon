#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"
#include <mncommon/dumpm.h>
#include <mncommon/array.h>
#include <mncommon/util.h>

#include "diag.h"

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
