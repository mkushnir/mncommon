#ifndef CONF_PRIVATE_H
#define CONF_PRIVATE_H

#include <stdio.h>

struct _read_ctx {
    const char *buf;
    int pos;
    int sz;
};

struct _conf_parser_handler_entry;

typedef struct _conf_parser_ctx {
    FILE *f;
    char tok[1024];
    int tokidx;
    int toklen;
    int state;
    int flags;
    struct _conf_parser_handler_entry *he;
} conf_parser_ctx_t;
#define CONF_PARSER_CTX_T_DEFINED

typedef int (*conf_parser_handler_t)(conf_parser_ctx_t *, void *);
#define CONF_PARSER_HANDLER_T_DEFINED
#define CONF_HANDLER_MORE (1)
#define CONF_HANDLER_EOF (-1)

typedef struct _conf_parser_handler_entry {
    const char *tok;
    int toklen;
    int nreq;
    int nopt;
    conf_parser_handler_t handler;
    conf_parser_handler_t error;
    void *udata;
} conf_parser_handler_entry_t;
#define CONF_PARSER_HANDLER_ENTRY_T_DEFINED

#define TOK_MAXLEN 1023

#include "mrkcommon/conf.h"

#endif
