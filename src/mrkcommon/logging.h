#ifndef MRKCOMMON_LOGGING_H
#define MRKCOMMON_LOGGING_H

#include <syslog.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _logmodule {
    const char *name;
    int id;
    int level;
} logmodule_t;

extern void (*logging_log)(int, const char *, ...) __printflike(2, 3);

#define LOG_TRACE (LOG_DEBUG + 1)

#define LOGGING_DECLARE(modname, modid, _level) \
    static logmodule_t __logging_module = {modname, modid, (1<<_level)}

/* XXX fix it to support inclusive levels */
#define LOGGING_SETLEVEL(_level) __logging_module.level |= (1<<_level)
#define LOGGING_CLEARLEVEL(_level) __logging_module.level &= (~(1<<_level))

#ifdef MTRACE
#   undef MTRACE
#endif
#define MTRACE(m, s, ...) do { \
       if ((m)->level & (1<<LOG_TRACE)) { \
           logging_log(LOG_DEBUG, "%s:%d:%s() " s "\n", \
                       __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
       } \
   } while (0)

#ifdef TRACE
#   undef TRACE
#endif
#define TRACE(s, ...) MTRACE(&__logging_module, s, ##__VA_ARGS__)

#ifdef MDEBUG
#   undef MDEBUG
#endif
#define MDEBUG(m, s, ...) do { \
       if ((m)->level & (1<<LOG_DEBUG)) { \
           logging_log(LOG_DEBUG, "%s:%d:%s() " s "\n", \
                       __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
       } \
   } while (0)

#ifdef DEBUG
#   undef DEBUG
#endif
#define DEBUG(s, ...) MDEBUG(&__logging_module, s, ##__VA_ARGS__)

#ifdef MINFO
#   undef MINFO
#endif
#define MINFO(m, s, ...) do { \
       if ((m)->level & (1<<LOG_INFO)) { \
           logging_log(LOG_INFO, "%s:%d:%s() " s "\n", \
                       __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
       } \
   } while (0)

#ifdef INFO
#   undef INFO
#endif
#define INFO(s, ...) MINFO(&__logging_module, s, ##__VA_ARGS__)

#ifdef MWARNING
#   undef MWARNING
#endif
#define MWARNING(m, s, ...) do { \
       if ((m)->level & (1<<LOG_WARNING)) { \
           logging_log(LOG_WARNING, "%s:%d:%s() " s "\n", \
                       __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
       } \
   } while (0)

#ifdef WARNING
#   undef WARNING
#endif
#define WARNING(s, ...) MWARNING(&__logging_module, s, ##__VA_ARGS__)

#ifdef MERROR
#   undef MERROR
#endif
#define MERROR(m, s, ...) do { \
       if ((m)->level & (1<<LOG_ERR)) { \
           logging_log(LOG_ERR, "%s:%d:%s() " s "\n", \
                       __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
       } \
   } while (0)

#ifdef ERROR
#   undef ERROR
#endif
#define ERROR(s, ...) MERROR(&__logging_module, s, ##__VA_ARGS__)


void logging_init(FILE *, const char *, int);
void logging_fini();

#ifdef __cplusplus
}
#endif

#endif
