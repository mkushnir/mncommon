/* Must be included before this file */
#include "dumpm.h"

#ifndef MRKCOMMON_LDUMPM_H
#define MRKCOMMON_LDUMPM_H

#include <syslog.h>

#include "mrkcommon/logging.h"

#ifdef MD8
#   undef MD8
#endif
#define MD8(mod, m, n) do { \
    if (mod.level & (1<<LOG_TRACE)) { \
        dumpm(m, n, 8); \
    } ;\
} while (0)

#ifdef D8
#   undef D8
#endif
#define D8(m, n) MD8(LOGGING_MODULE, m, n)

#ifdef MD16
#   undef MD16
#endif
#define MD16(mod, m, n) do { \
    if (mod.level & (1<<LOG_TRACE)) { \
        dumpm(m, n, 8); \
    } ;\
} while (0)

#ifdef D16
#   undef D16
#endif
#define D16(m, n) MD16(LOGGING_MODULE, m, n)

#ifdef MD32
#   undef MD32
#endif
#define MD32(mod, m, n) do { \
    if (mod.level & (1<<LOG_TRACE)) { \
        dumpm(m, n, 8); \
    } ;\
} while (0)

#ifdef D32
#   undef D32
#endif
#define D32(m, n) MD32(LOGGING_MODULE, m, n)

#ifdef MD64
#   undef MD64
#endif
#define MD64(mod, m, n) do { \
    if (mod.level & (1<<LOG_TRACE)) { \
        dumpm(m, n, 8); \
    } ;\
} while (0)

#ifdef D64
#   undef D64
#endif
#define D64(m, n) MD64(LOGGING_MODULE, m, n)

#endif
