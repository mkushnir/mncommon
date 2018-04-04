#ifndef MRKCOMMON_MALLOC_H
#define MRKCOMMON_MALLOC_H

#include <stdlib.h>

#ifdef CMOCKA_TESTING
#   include <stdarg.h>
#   include <stddef.h>
#   include <setjmp.h>
#   include <cmocka.h>
#   ifdef malloc
#   undef malloc
#   endif
#   ifdef realloc
#   undef realloc
#   endif
#   define malloc(sz) test_malloc(sz)
#   define calloc(num, sz) test_calloc(num, sz)
#   define realloc(ptr, sz) test_realloc(ptr, sz)
#   define free(ptr) test_free(ptr)
#endif

#endif
