#include <assert.h>
#include <stdio.h>
#include "mrkcommon/util.h"

intptr_t
mn_check_type_failure(const char *ty)
{
    fprintf(stderr, "mrkcommon_check_type_failure: not a %s\n", ty);
    assert(0);
    return 0;
}

