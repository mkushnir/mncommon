#define _WITH_GETLINE
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/array.h"
#include "conf_private.h"
#include "diag.h"


static array_t conf_parser_words;
static array_t conf_parser_prefixes;

/*
 * Configuraiton parser.
 */

static int
conf_parser_handler_entry_fini(UNUSED conf_parser_handler_entry_t *he)
{
    return 0;
}

static conf_parser_handler_entry_t *
conf_parser_get_handler_entry(conf_parser_ctx_t *ctx)
{
    array_iter_t it;
    conf_parser_handler_entry_t *he;
    for (he = array_first(&conf_parser_words, &it);
         he != NULL;
         he = array_next(&conf_parser_words, &it)) {
        if (strcmp(he->tok, ctx->tok) == 0) {
            return he;
        }
    }
    for (he = array_first(&conf_parser_prefixes, &it);
         he != NULL;
         he = array_next(&conf_parser_prefixes, &it)) {
        if (strncmp(he->tok, ctx->tok, strlen(he->tok)) == 0) {
            return he;
        }
    }
    return NULL;
}

static int
conf_parser_check_handler_entry(conf_parser_ctx_t *ctx)
{
    ctx->he = conf_parser_get_handler_entry(ctx);
    return ctx->he != NULL;
}

/* A simple state machine */
#define CONF_PARSER_NEXT_TOK_START 0
#define CONF_PARSER_NEXT_TOK_SPACE 1
#define CONF_PARSER_NEXT_TOK_WORD 2
static int
conf_parser_next_tok(conf_parser_ctx_t *ctx)
{
    int c;

    ctx->flags &= ~CP_FLAG_EOL;

    ctx->toklen = 0;
    flockfile(ctx->f);
    while (ctx->toklen < TOK_MAXLEN) {
        //TRACE("ctx->toklen=%d", ctx->toklen);
        if ((c = getc_unlocked(ctx->f)) == EOF) {
            break;
        }
        if (c == ' ' || c == '\t') {
            if (ctx->state != CONF_PARSER_NEXT_TOK_SPACE) {
                ctx->state = CONF_PARSER_NEXT_TOK_SPACE;
            }
            continue;
        } else if (c == '\n' || c == '\r') {
            ctx->flags |= CP_FLAG_EOL;
            if (ctx->state != CONF_PARSER_NEXT_TOK_SPACE) {
                ctx->state = CONF_PARSER_NEXT_TOK_SPACE;
            }
            continue;
        } else {
            if (ctx->state == CONF_PARSER_NEXT_TOK_SPACE) {
                ctx->state = CONF_PARSER_NEXT_TOK_START;
                ungetc(c, ctx->f);
                break;
            }
            ctx->state = CONF_PARSER_NEXT_TOK_WORD;
            ctx->tok[ctx->toklen++] = c;
        }
    }
    funlockfile(ctx->f);

    ctx->tok[ctx->toklen] = '\0';
    //TRACE("ctx->tok='%s'", ctx->tok);

    if (ctx->toklen == 0) {
        /* EOF */
        //TRACE("Token '%s' is empty.", ctx->tok);
        ctx->flags |= CP_FLAG_EOL;
        return CONF_HANDLER_EOF;
    }
    if (ctx->toklen >= TOK_MAXLEN) {
        //TRACE("Token '%s' exceeded maximum length", ctx->tok);
        ctx->flags |= CP_FLAG_TOOBIG;
        TRRET(CONF_PARSER_NEXT_TOK + 2);
    }
    return CONF_HANDLER_CONT;
}

/*
 * There are two kinds of tokens: words and prefixes.
 * Words are those that match exacly their parsed entities.
 * Prefixes are only starting substrings of the parsed entities.
 *
 * In the handler selection algorithm, words have precedence over
 * prefixes.
 *
 * It is recommendsd that these kinds have nothing in common.
 */
int
conf_register_word_handler(const char *tok, int nreq, int nopt,
                           conf_parser_handler_t handler,
                           conf_parser_handler_t error,
                           void *udata)
{
    conf_parser_handler_entry_t *he;

    if ((he = array_incr(&conf_parser_words)) == NULL) {
        FAIL("array_incr");
    }
    he->tok = tok;
    he->toklen = strlen(tok);
    he->nreq = nreq;
    he->nopt = nopt;
    he->handler = handler;
    he->error = error;
    he->udata = udata;
    return 0;
}

int
conf_register_prefix_handler(const char *tok, int nreq, int nopt,
                             conf_parser_handler_t handler,
                             conf_parser_handler_t error,
                             void *udata)
{
    conf_parser_handler_entry_t *he;

    if ((he = array_incr(&conf_parser_prefixes)) == NULL) {
        FAIL("array_incr");
    }
    he->tok = tok;
    he->toklen = strlen(tok);
    he->nreq = nreq;
    he->nopt = nopt;
    he->handler = handler;
    he->error = error;
    he->udata = udata;
    return 0;
}

/*
 * A well-known parser of #-comments as an example of a handler.
 *
 * # ... until EOL
 */
int
line_comment_handler(conf_parser_ctx_t *ctx, UNUSED void *udata)
{
    do {
        //TRACE("comment '%s'", ctx->tok);
        if (ctx->flags & CP_FLAG_EOL) {
            return 0;
        }
    } while (conf_parser_next_tok(ctx) == 0);
    return 0;
}


const char *
conf_parser_tok(conf_parser_ctx_t *ctx)
{
    return ctx->tok;
}

const char *
conf_parser_orig_tok(conf_parser_ctx_t *ctx)
{
    return ctx->he->tok;
}

int
conf_parser_toklen(conf_parser_ctx_t *ctx)
{
    return ctx->toklen;
}

int
conf_parser_tokidx(conf_parser_ctx_t *ctx)
{
    return ctx->tokidx;
}

int
conf_parser_flags(conf_parser_ctx_t *ctx)
{
    return ctx->flags;
}

FILE *
conf_parser_file(conf_parser_ctx_t *ctx)
{
    return ctx->f;
}

int
conf_parse_handler_wrapper(conf_parser_ctx_t *ctx)
{
    int res;
    conf_parser_handler_entry_t *savedhe = ctx->he;
    int allargs = savedhe->nreq + savedhe->nopt;

    /* token index 0 */
    ctx->tokidx = 0;

    if (savedhe->handler != NULL) {
        if ((res = savedhe->handler(ctx, savedhe->udata)) != 0) {
            return res;
        }
    }

    ++ctx->tokidx;
    for (; ctx->tokidx <= allargs; ++(ctx->tokidx)) {

        if ((res = conf_parser_next_tok(ctx)) > 0) {
            if (savedhe->error != NULL) {
                conf_parser_handler_entry_t *tmphe = ctx->he;

                ctx->he = savedhe;
                (void)savedhe->error(ctx, savedhe->udata);
                ctx->he = tmphe;
            }
            return res;
        }

        if (res == CONF_HANDLER_EOF) {
            /* pass EOF upwards */
            return res;
        }

        if (conf_parser_check_handler_entry(ctx)) {
            if (ctx->tokidx <= savedhe->nreq) {
                if (savedhe->error != NULL) {
                    conf_parser_handler_entry_t *tmphe = ctx->he;

                    ctx->he = savedhe;
                    ctx->flags |= CP_FLAG_NEA;
                    res = savedhe->error(ctx, savedhe->udata);
                    ctx->flags &= ~CP_FLAG_NEA;
                    ctx->he = tmphe;
                    if (res != 0) {
                        TRRET(CONF_PARSE_HANDLER_WRAPPER + 2);
                    }
                }
            }
            return CONF_HANDLER_MORE;
        }

        if (savedhe->handler != NULL) {
            if ((res = savedhe->handler(ctx, savedhe->udata)) != 0) {
                return res;
            }
        }
    }

    return CONF_HANDLER_CONT;
}

int
conf_parse_file(FILE *f)
{
    int res = 0;
    conf_parser_ctx_t ctx;

    ctx.f = f;
    ctx.state = CONF_PARSER_NEXT_TOK_START;
    ctx.flags = 0;
    ctx.toklen = 0;
    ctx.he = NULL;

    while (conf_parser_next_tok(&ctx) <= 0) {
NEXT_TOK_DONE:
        //TRACE("main tok='%s' toklen=%d", ctx.tok, ctx.toklen);
        if (ctx.toklen == 0) {
            /* EOF?*/
            break;
        }
        if ((ctx.he = conf_parser_get_handler_entry(&ctx)) == NULL) {
            //TRACE("Failed to find handler for '%s' token", ctx.tok);
            TRRET(CONF_PARSE_FILE + 1);
        }
        res = conf_parse_handler_wrapper(&ctx);
        //TRACE("res=%d", res);
        switch (res) {
        case CONF_HANDLER_CONT:
            continue;

        case CONF_HANDLER_MORE:
            goto NEXT_TOK_DONE;
        case CONF_HANDLER_EOF:
            /* EOF */
            res = 0;
            break;

        default:
            //TRACE("Handler for %s token returned %d", ctx.he->tok, res);
            return res;
        }
    }
    return res;
}

int
conf_parse_fpath(const char *conf_file)
{
    int res = 0;
    FILE *f = NULL;

    if (conf_file == NULL) {
        //TRACE("Nothing to configure ...");
    } else {
        //TRACE("Configuring from %s ...", conf_file);
        if ((f = fopen(conf_file, "r")) == NULL) {
            TRRET(CONF_PARSE_FPATH + 1);
        }

        if ((res = conf_parse_file(f)) != 0) {
            fclose(f);
            TRRET(CONF_PARSE_FPATH + 2);
        }

        if (fclose(f) != 0) {
            perror("fclose");
        }
    }

    return res;
}

static int
conf_read_buf(void *cookie, char *out, int sz)
{
    struct _read_ctx *ctx = (struct _read_ctx *)cookie;
    int oversz;

    if (cookie == NULL) {
        errno = EBADF;
        return -1;
    }

    if (sz <= 0 || ctx->pos < 0) {
        errno = EINVAL;
        return -1;
    }

    oversz = (ctx->pos + sz) - ctx->sz;
    if (oversz > 0) {
        sz -= oversz;
    }

    errno = 0;
    if (ctx->pos >= ctx->sz) {
        return 0;
    }

    memcpy(out, ctx->buf + ctx->pos, sz);
    ctx->pos += sz;
    return sz;
}

static off_t
conf_seek_buf(void *cookie, off_t offset, int whence)
{
    struct _read_ctx *ctx = (struct _read_ctx *)cookie;

    if (whence == SEEK_SET) {
        ctx->pos = offset;
    } else if (whence == SEEK_CUR) {
        ctx->pos += offset;
    } else if (whence == SEEK_END) {
        ctx->pos += ctx->sz + offset;
#ifdef SEEK_HOLE
    } else if (whence == SEEK_HOLE) {
        errno = ENXIO;
        return -1;
#endif
    } else {
        errno = EINVAL;
        return -1;
    }
    return ctx->pos;
}

int
conf_parse_buf(const char *buf)
{
    int res = 0;
    FILE *f = NULL;
    struct _read_ctx cookie;

    cookie.buf = buf;
    cookie.sz = strlen(buf);
    cookie.pos = 0;

    if ((f = funopen(&cookie, conf_read_buf, NULL, conf_seek_buf, NULL)) == NULL) {
        TRRET(CONF_PARSE_BUF + 1);
    }

    if ((res = conf_parse_file(f)) != 0) {
        fclose(f);
        TRRET(CONF_PARSE_FPATH + 2);
    }

    if (fclose(f) != 0) {
        perror("fclose");
    }

    return res;
}

int
conf_init(void)
{
    if (array_init(&conf_parser_words, sizeof(conf_parser_handler_entry_t),
                   0, NULL,
                   (array_finalizer_t)conf_parser_handler_entry_fini) != 0) {
        TRRET(CONF_INIT + 1);
    }
    if (array_init(&conf_parser_prefixes, sizeof(conf_parser_handler_entry_t),
                   0, NULL,
                   (array_finalizer_t)conf_parser_handler_entry_fini) != 0) {
        TRRET(CONF_INIT + 2);
    }
    return 0;
}

int
conf_fini(void)
{
    array_fini(&conf_parser_words);
    array_fini(&conf_parser_prefixes);
    return 0;
}

