#include <assert.h>
#include <stdio.h>
#include "mncommon/util.h"

intptr_t
mn_check_type_failure(const char *ty)
{
    fprintf(stderr, "mncommon_check_type_failure: not a %s\n", ty);
    assert(0);
    return 0;
}

