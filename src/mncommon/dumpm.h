#ifndef MNCOMMON_DUMPM_H
#define MNCOMMON_DUMPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#ifdef USE_SYSLOG
#include <syslog.h>
#endif
#include <time.h>
#include <unistd.h>

#include <mncommon/bytestream.h>

#ifndef DUMPM_INDENT_SIZE
#   define DUMPM_INDENT_SIZE 4
#endif

#ifndef DUMPM_OUTFILE
#   define DUMPM_OUTFILE stderr
#endif

#ifdef USE_SYSLOG
# define TRACEN(s, ...)     (syslog(LOG_DEBUG,    "[T]%s:%d:%s() " s,      __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define TRACEC(s, ...)     (syslog(LOG_DEBUG,                     s,                                    ##__VA_ARGS__))
# define TRACE(s, ...)      (syslog(LOG_DEBUG,    "[T]%s:%d:%s() " s "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define DEBUG(s, ...)      (syslog(LOG_DEBUG,    "[D]%s:%d:%s() " s "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define INFO(s, ...)       (syslog(LOG_INFO,     "[I]%s:%d:%s() " s "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define WARNING(s, ...)    (syslog(LOG_WARNING,  "[W]%s:%d:%s() " s "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define ERROR(s, ...)      (syslog(LOG_ERR,      "[E]%s:%d:%s() " s "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define LTRACE(lvl, s, ...) (syslog(LOG_DEBUG,   "%*c"            s "\n", DUMPM_INDENT_SIZE * (lvl), ' ', ##__VA_ARGS__))
# define LTRACEN(lvl, s, ...) (syslog(LOG_DEBUG,  "%*c"            s,      DUMPM_INDENT_SIZE * (lvl), ' ', ##__VA_ARGS__))
#else
# define TRACEN(s, ...) (fprintf(DUMPM_OUTFILE, "[%5d] %s:%d:%s() " s, getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define TRACEC(s, ...) (fprintf(DUMPM_OUTFILE, s, ##__VA_ARGS__))
# define TRACE(s, ...) (fprintf(DUMPM_OUTFILE, "[%5d] %s:%d:%s() " s "\n", getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__))
# define DEBUG   TRACE
# define INFO    TRACE
# define WARNING TRACE
# define ERROR   TRACE
# define LTRACE(lvl, s, ...) fprintf(DUMPM_OUTFILE, "%*c" s "\n", DUMPM_INDENT_SIZE * (lvl), ' ', ##__VA_ARGS__)
# define LTRACEN(lvl, s, ...) fprintf(DUMPM_OUTFILE, "%*c" s, DUMPM_INDENT_SIZE * (lvl), ' ', ##__VA_ARGS__)

#endif

#ifndef MNCOMMON_DUMPM_DIAG_H
#define MNCOMMON_DUMPM_DIAG_H "diag.h"
#endif
#ifdef TRRET_DEBUG_VERBOSE
#   include MNCOMMON_DUMPM_DIAG_H
#   define TR(n) do { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); } while (0)
#   define TRRET(n) do { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); return (n); } while (0)
#   define TRRETNULL(n) do { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); return (NULL); } while (0)
#   define TRRETVOID(n) do { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); return; } while (0)
#else
#   ifdef TRRET_DEBUG
#       include MNCOMMON_DUMPM_DIAG_H
#       define TR(n) do { if (n) { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); } } while (0)
#       define TRRET(n) do { if (n) { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); } return (n); } while (0)
#       define TRRETNULL(n) do { if(n) { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); } return (NULL); } while (0)
#       define TRRETVOID(n) do { if(n) { char _tr_buf[64]; mndiag_local_str(n, _tr_buf, sizeof(_tr_buf)); TRACE("%s", _tr_buf); } return; } while (0)
#   else
#       define TR(n)
#       define TRRET(n) return (n)
#       define TRRETNULL(n) return (NULL)
#       define TRRETVOID(n) return
#   endif
#endif
void dumpm(const void * m, size_t n, size_t l);
void bytestream_dumpm(mnbytestream_t *bs, const void * m, size_t n, size_t l);
#define D8(m, n) dumpm(m, n, 8)
#define D16(m, n) dumpm(m, n, 16)
#define D32(m, n) dumpm(m, n, 32)
#define D64(m, n) dumpm(m, n, 64)
#define BSD8(bs, m, n) bytestream_dumpm((bs), m, n, 8)
#define BSD16(bs, m, n) bytestream_dumpm((bs), m, n, 16)
#define BSD32(bs, m, n) bytestream_dumpm((bs), m, n, 32)
#define BSD64(bs, m, n) bytestream_dumpm((bs), m, n, 64)
void mndump_bits(const void *, size_t);

#   define TRACEBINLSB(c)                      \
for (uint8_t i = 0; i < 8; ++i) {              \
    TRACEC((c & (0x01 << i)) ? "1" : "0");     \
}                                              \


#   define TRACEBINMSB(c)              \
for (uint8_t i = 0; i < 8; ++i) {      \
    TRACEC((c & (0x01 << (7 - i)))     \
            ? "1" : "0");              \
}                                      \

#ifndef NOFCOLOR
#   define FCOLOR(b, c, s) "\033[0" b ";3" c "m" s "\033[00m"
#else
#   define FCOLOR(b, c, s) s
#endif
#define FRED(s)         FCOLOR("0", "1", s)
#define FGREEN(s)       FCOLOR("0", "2", s)
#define FYELLOW(s)      FCOLOR("0", "3", s)
#define FBLUE(s)        FCOLOR("0", "4", s)
#define FMAGENTA(s)     FCOLOR("0", "5", s)
#define FCYAN(s)        FCOLOR("0", "6", s)
#define FWHITE(s)       FCOLOR("0", "7", s)
#define FBLACK(s)       FCOLOR("0", "0", s)
#define FBRED(s)        FCOLOR("1", "1", s)
#define FBGREEN(s)      FCOLOR("1", "2", s)
#define FBYELLOW(s)     FCOLOR("1", "3", s)
#define FBBLUE(s)       FCOLOR("1", "4", s)
#define FBMAGENTA(s)    FCOLOR("1", "5", s)
#define FBCYAN(s)       FCOLOR("1", "6", s)
#define FBWHITE(s)      FCOLOR("1", "7", s)
#define FBBLACK(s)      FCOLOR("1", "0", s)

#ifdef __cplusplus
}
#endif

#endif /* DUMPM_H */
