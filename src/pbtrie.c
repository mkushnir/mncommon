#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h>

#ifndef HAVE_FLSL
#   ifdef __GNUC__
#       define flsl(v) (64 - __builtin_clzl(v))
#   else
#       error "Could not find/define flsl."
#   endif
#endif

#include "diag.h"
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/pbtrie.h>

