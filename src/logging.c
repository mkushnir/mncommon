#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static FILE *logging_out = NULL;
static int mylogopt;
static char *myident = NULL;
static size_t myident_sz = 0;

void (*logging_log)(int, const char *, ...) __printflike(2, 3) = NULL;

static void
do_log (UNUSED int level, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(logging_out, fmt, ap);
    va_end(ap);
}

static void
do_log_pid (UNUSED int level, const char *fmt, ...)
{
    va_list ap;
    char *ffmt;
    size_t sz;

#define DO_LOG_PID_FMT "%s[%5d]: %s"
    sz = myident_sz + 8 + strlen(fmt) + strlen(DO_LOG_PID_FMT);
    if ((ffmt = malloc(sz)) == NULL) {
        perror("malloc");
        return;
    }
    snprintf(ffmt, sz, DO_LOG_PID_FMT, myident, getpid(), fmt);

    va_start(ap, fmt);
    vfprintf(logging_out, ffmt, ap);
    va_end(ap);
    free(ffmt);
}

/*!
 * @fn void logging_init(FILE *out, const char *ident, int logopt)
 * @brief Initialize the logging module.
 * @param out The stream to write log lines to. If NULL was passed, the
 *              syslog back end will be used.
 * @param ident The openlog(3) ident parameter.
 * @param logopt The openlog(3) logopt parameter.
 */
void
logging_init(FILE *out, const char *ident, int logopt)
{
    logging_out = out;
    if (out == NULL) {
        openlog(ident, logopt, LOG_USER);
        logging_log = syslog;
    } else {
        mylogopt = logopt;
        if (ident != NULL) {
            myident = strdup(ident);
        } else {
            myident = strdup("");
        }
        myident_sz = strlen(myident);

        if (logopt & LOG_PID) {
            logging_log = do_log_pid;
        } else {
            logging_log = do_log;
        }
    }
}

void logging_fini(void)
{
    if (myident != NULL) {
        free(myident);
        myident = NULL;
    }
    closelog();
}
