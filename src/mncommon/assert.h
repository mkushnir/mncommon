#ifndef MNCOMMON_ASSERT_H
#define MNCOMMON_ASSERT_H

#include <assert.h>

#ifdef CMOCKA_TESTING
#   include <stdarg.h>
#   include <stddef.h>
#   include <setjmp.h>
#   include <cmocka.h>
#   undef assert
#   define assert(e) mock_assert((int)(e), #e, __FILE__, __LINE__)
#endif

#endif
