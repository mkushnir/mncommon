#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"

#include "mrkcommon/dumpm.h"
#include "mrkcommon/profile.h"

static void
test0(void)
{
    const profile_t *p1, *p2;
    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    p1 = profile_register("P1");

    profile_start(p1);
    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }
    profile_stop(p1);

    p2 = profile_register("P2");

    profile_start(p2);
    profile_stop(p2);
}

int
main(void)
{
    profile_init_module();
    test0();
    test0();
    profile_report();
    profile_fini_module();
    return 0;
}


