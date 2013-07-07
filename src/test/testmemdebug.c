#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/array.h"
#include "mrkcommon/list.h"
#include "mrkcommon/memdebug.h"

static void
test0(void)
{
    void *p1, *p2;
    char *s1, *s2;

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

    TRACE("nallocated=%ld", memdebug_nallocated());
    p1 = malloc(100);
    TRACE("nallocated=%ld", memdebug_nallocated());
    p1 = realloc(p1, 200);
    TRACE("nallocated=%ld", memdebug_nallocated());
    p1 = reallocf(p1, 400);
    TRACE("nallocated=%ld", memdebug_nallocated());
    p2 = calloc(4, 100);
    TRACE("nallocated=%ld", memdebug_nallocated());
    s1 = strdup("This is the test.");
    TRACE("nallocated=%ld", memdebug_nallocated());
    s2 = strndup(s1, 32);
    TRACE("nallocated=%ld", memdebug_nallocated());

    free(p1);
    free(p2);
    free(s1);
    free(s2);
    TRACE("nallocated=%ld", memdebug_nallocated());
    assert(memdebug_nallocated() == 0);
}

int
main(void)
{
    test0();
    return 0;
}
