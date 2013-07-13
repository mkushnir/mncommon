#ifndef MRKCOMMON_CONF_H
#define MRKCOMMON_CONF_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONF_PARSER_CTX_T_DEFINED
typedef struct _conf_parser_ctx conf_parser_ctx_t;
#define CONF_PARSER_CTX_T_DEFINED
#endif

#ifndef CONF_PARSER_HANDLER_T_DEFINED
typedef int (*conf_parser_handler_t)(conf_parser_ctx_t *, void *);
#define CONF_PARSER_HANDLER_T_DEFINED
#endif
#define CONF_HANDLER_CONT (0)
#define CONF_HANDLER_INTR (2)

#ifndef CONF_PARSER_HANDLER_ENTRY_T_DEFINED
typedef struct _conf_parser_handler_entry conf_parser_handler_entry_t;
#define CONF_PARSER_HANDLER_ENTRY_T_DEFINED
#endif

int line_comment_handler(conf_parser_ctx_t *, void *);

const char *conf_parser_tok(conf_parser_ctx_t *);
const char *conf_parser_orig_tok(conf_parser_ctx_t *);
int conf_parser_toklen(conf_parser_ctx_t *);
int conf_parser_tokidx(conf_parser_ctx_t *);
#define CP_FLAG_EOL 0x01
/* Not enough arguments */
#define CP_FLAG_NEA 0x02
/* token was to big */
#define CP_FLAG_TOOBIG 0x04
int conf_parser_flags(conf_parser_ctx_t *);
FILE *conf_parser_file(conf_parser_ctx_t *);

int conf_parse_fpath(const char *);
int conf_parse_file(FILE *);
int conf_parse_buf(const char *);

int conf_register_word_handler(const char *, int, int,
                               conf_parser_handler_t,
                               conf_parser_handler_t,
                               void *);
int conf_register_prefix_handler(const char *, int, int,
                                 conf_parser_handler_t,
                                 conf_parser_handler_t,
                                 void *);

int conf_init(void);
int conf_fini(void);


#ifdef __cplusplus
}
#endif

#endif
