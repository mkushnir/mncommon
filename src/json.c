#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// strptime
#include <time.h>
// struct timeval
#include <sys/time.h>

//#define TRRET_DEBUG_VERBOSE
#include <mncommon/malloc.h>
#include <mncommon/bytes.h>
#include <mncommon/bytestream.h>
#include <mncommon/dumpm.h>
#include <mncommon/json.h>
#include <mncommon/util.h>

#include "diag.h"

#define JSN_ISDIGIT(c) (((c) >= '0' && (c) <= '9'))
#define JSN_ISALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define JSN_ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')
int
json_init(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->in = NULL;
    ctx->sz = 0;
    ctx->cb = cb;
    ctx->udata = udata;
    ctx->ostart_cb = cb;
    ctx->ostart_udata = udata;
    ctx->ostop_cb = cb;
    ctx->ostop_udata = udata;
    ctx->astart_cb = cb;
    ctx->astart_udata = udata;
    ctx->astop_cb = cb;
    ctx->astop_udata = udata;
    ctx->key_cb = cb;
    ctx->key_udata = udata;
    ctx->value_cb = cb;
    ctx->value_udata = udata;
    ctx->item_cb = cb;
    ctx->item_udata = udata;
    ctx->st = JPS_START;
    ctx->flags = 0;
    ctx->idx = 0;
    ctx->nest = -1;
    if (MNUNLIKELY(
            array_init(&ctx->stack, sizeof(void *), 0, NULL, NULL) != 0)) {
        FFAIL("array_init");
    }
    array_ensure_datasz_dirty(&ctx->stack, 8, 0);
    ctx->current_key = NULL;
    return 0;
}

void
json_reset(json_ctx_t *ctx)
{
    BYTES_DECREF(&ctx->current_key);
    if (MNUNLIKELY(array_clear(&ctx->stack) != 0)) {
        FFAIL("array_clear");
    }
    ctx->in = NULL;
    ctx->sz = 0;
    ctx->st = JPS_START;
    ctx->flags = 0;
    ctx->idx = 0;
    ctx->nest = -1;
}

void
json_set_ostart_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->ostart_cb = cb;
    ctx->ostart_udata = udata;
}

void
json_set_ostop_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->ostop_cb = cb;
    ctx->ostop_udata = udata;
}

void
json_set_astart_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->astart_cb = cb;
    ctx->astart_udata = udata;
}

void
json_set_astop_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->astop_cb = cb;
    ctx->astop_udata = udata;
}

void
json_set_key_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->key_cb = cb;
    ctx->key_udata = udata;
}

void
json_set_value_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->value_cb = cb;
    ctx->value_udata = udata;
}

void
json_set_item_cb(json_ctx_t *ctx, json_cb_t cb, void *udata)
{
    ctx->item_cb = cb;
    ctx->item_udata = udata;
}

void
json_dump(json_ctx_t *ctx)
{
    TRACEN("[% 6ld] % 2d:%s", ctx->idx, ctx->nest, JPS_TOSTR(ctx->st));
    if (ctx->ty == JSON_STRING) {
        char *tmp;
        size_t sz = ctx->v.s.end - ctx->v.s.start;

        if ((tmp = malloc(sz + 1)) == NULL) {
            FAIL("malloc");
        }
        memcpy(tmp, &ctx->in[ctx->v.s.start], sz);
        tmp[sz] = '\0';
        TRACEC("\t\"%s\"%s", tmp, (ctx->st & JPS_KEYOUT ? ":" : ""));
        free(tmp);
    } else if (ctx->ty == JSON_INT) {
        TRACEC("\t%jd", ctx->v.i);
    } else if (ctx->ty == JSON_FLOAT) {
        TRACEC("\t%lf", ctx->v.f);
    } else if (ctx->ty == JSON_BOOLEAN) {
        TRACEC("\t%s", ctx->v.b ? "#t" : "#f");
    } else if (ctx->ty == JSON_NULL) {
        TRACEC("\tnull");
    } else if (ctx->ty == JSON_OBJECT) {
        TRACEC("\t%s", (ctx->st == JPS_OSTART ? "{" : "}"));
    } else if (ctx->ty == JSON_ARRAY) {
        TRACEC("\t%s", (ctx->st == JPS_ASTART ? "[" : "]"));
    } else {
        TRACEC("\t???");
    }
}


const char *
json_type_hint_str (const json_node_t *n)
{
#define JSON_TYPE_HINT_STR_BUFSZ (256)
    static char bufs[16][JSON_TYPE_HINT_STR_BUFSZ];
    char *buf;
    static unsigned idx = 0;
    int nwritten = 0;
    ssize_t sz;
    ssize_t tmp0, tmp1;


    if (idx >= countof(bufs)) {
        idx = 0;
    }
    buf = bufs[idx];

    nwritten += snprintf(buf, JSON_TYPE_HINT_STR_BUFSZ, "% 3d:", n->nest);

#define _COPY_ONE(s)                                           \
    sz = strlen(s);                                            \
    tmp0 = MIN(nwritten, (JSON_TYPE_HINT_STR_BUFSZ - 1));      \
    tmp1 = JSON_TYPE_HINT_STR_BUFSZ - tmp0;                    \
    sz = MIN(sz, tmp1);                                        \
    (void)memcpy(buf + nwritten, s, sz);                       \
    nwritten += sz

    if (n->ty & JSON_TYPE_HINT_OBJECT) {
        _COPY_ONE("object|");
    }
    if (n->ty & JSON_TYPE_HINT_ARRAY) {
        _COPY_ONE("array|");
    }
    if (n->ty & JSON_TYPE_HINT_STRING) {
        _COPY_ONE("string|");
    }
    if (n->ty & JSON_TYPE_HINT_INT) {
        _COPY_ONE("int|");
    }
    if (n->ty & JSON_TYPE_HINT_FLOAT) {
        _COPY_ONE("float|");
    }
    if (n->ty & JSON_TYPE_HINT_BOOLEAN) {
        _COPY_ONE("boolean|");
    }
    if (n->ty & JSON_TYPE_HINT_NULL) {
        _COPY_ONE("null|");
    }
    if (n->ty & JSON_TYPE_HINT_ANY) {
        _COPY_ONE("any|");
    }
    if (n->ty & JSON_TYPE_HINT_ONEOF) {
        _COPY_ONE("oneof|");
    }
    if (n->ty & JSON_TYPE_HINT_ITEM) {
        if (n->v != NULL) {
            _COPY_ONE("item(");
            _COPY_ONE((const char *)(n->v));
            _COPY_ONE(")|");
        } else {
            _COPY_ONE("item|");
        }
    }

    if ((nwritten > 0) && buf[nwritten - 1] == '|') {
        --nwritten;
    }
    buf[nwritten] = '\0';

    ++idx;

    return buf;
}


void
json_ctx_push(json_ctx_t *ctx, void *o)
{
    void **p;
    if (MNUNLIKELY((p = array_incr(&ctx->stack)) == NULL)) {
        FFAIL("array_incr");
    }
    *p = o;
}


void *
json_ctx_pop(json_ctx_t *ctx)
{
    void **v;

    if (ARRAY_ELNUM(&ctx->stack) == 0) {
        return NULL;
    }

    v = ARRAY_GET(void *, &ctx->stack, ARRAY_ELNUM(&ctx->stack) - 1);

    (void)array_decr_fast(&ctx->stack);

    return *v;
}


void *
json_ctx_top(const json_ctx_t *ctx)
{
    void **v;

    if (ARRAY_ELNUM(&ctx->stack) == 0) {
        return NULL;
    }

    v = ARRAY_GET(void *, &ctx->stack, ARRAY_ELNUM(&ctx->stack) - 1);

    return *v;
}


unsigned
json_ctx_consistent(const json_ctx_t *ctx)
{
    unsigned res = 0;

    if (ctx->idx != ctx->sz) {
        res |= JSON_CTX_INCONSISTENT_IDX_SZ;
    }

    if (!(ctx->st & JPS_OUT)) {
        res |= JSON_CTX_INCONSISTENT_STATE;
    }

    if (ctx->nest >= 0) {
        res |= JSON_CTX_INCONSISTENT_NEST;
    }

    if (ARRAY_ELNUM(&ctx->stack) > 0) {
        res |= JSON_CTX_INCONSISTENT_STACK;
    }

    return res;
}


int
json_ctx_notice_key(json_ctx_t *ctx, UNUSED void *udata)
{
    BYTES_DECREF(&ctx->current_key);
    ctx->current_key = bytes_new_from_str_len(
        ctx->in + ctx->v.s.start,
        ctx->v.s.end - ctx->v.s.start);
    return 0;
}


mnbytes_t *
json_ctx_bytes_from_value(const json_ctx_t *ctx)
{
    return bytes_new_from_str_len(
        ctx->in + ctx->v.s.start,
        ctx->v.s.end - ctx->v.s.start);
}


double
json_ctx_strtod (const json_ctx_t *ctx)
{
    return strtod(ctx->in + ctx->v.s.start, NULL);
}


intmax_t
json_ctx_strtoimax (const json_ctx_t *ctx, int base)
{
    return strtoimax(ctx->in + ctx->v.s.start, NULL, base);
}


int
json_ctx_strptime_from_value (const json_ctx_t *ctx,
                              const char * restrict fmt,
                              struct timeval * restrict tv)
{
    int res = 0;
    struct tm tm;
    char *dot;

    if ((dot = strptime(ctx->in + ctx->v.s.start, fmt, &tm)) == NULL) {
        res = JSON_CTX + 1;
        goto end;
    }

    if ((tv->tv_sec = timegm(&tm)) == -1) {
        res = JSON_CTX + 2;
        goto end;
    }

    if (*dot == '.') {
        tv->tv_usec =
            (suseconds_t)(strtod(dot, NULL) * 1000000.0);
    } else {
        tv->tv_usec = 0;
    }

end:
    return res;
}


int
json_fini(json_ctx_t *ctx)
{
    BYTES_DECREF(&ctx->current_key);
    array_fini(&ctx->stack);
    ctx->cb = NULL;
    ctx->udata = NULL;
    ctx->in = NULL;
    ctx->sz = 0;
    return 0;
}


inline int
json_parse_str(json_ctx_t *ctx)
{
    ctx->flags &= ~JPS_FNEEDUNESCAPE;

    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];
        //TRACE("ch='%c'", ch);

        if (ctx->st == JPS_STRIN) {
            if (ch == '\\') {
                ctx->st = JPS_STRESC;
                ctx->flags |= JPS_FNEEDUNESCAPE;
            } else if (ch == '"') {
                ctx->st = JPS_STROUT;
                break;
            } else {
                ctx->st = JPS_STR;
            }

        } else if (ctx->st == JPS_STRESC) {
            ctx->st = JPS_STR;

        } else if (ctx->st == JPS_STR) {
            if (ch == '\\') {
                ctx->st = JPS_STRESC;
            } else if (ch == '"') {
                ctx->st = JPS_STROUT;
                break;
            } else {
                ctx->st = JPS_STR;
            }
        } else {
            TRRET(JSON_PARSE_STR + 1);
        }
    }

    if (ctx->st == JPS_STROUT) {
        ctx->ty = JSON_STRING;
        TRRET(0);
    }

    TRRET(JSON_PARSE_NEEDMORE);
}


inline int
json_parse_num(json_ctx_t *ctx)
{
    ctx->flags &= ~(JPS_FFLOAT | JPS_FSCIENTIFIC);

    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];
        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ctx->st == JPS_NUMIN) {
            if (JSN_ISDIGIT(ch) || ch == '-') {
                ctx->st = JPS_NUM;
            } else if (ch == '.') {
                ctx->st = JPS_NUM;
                ctx->flags |= JPS_FFLOAT;
            } else {
                ctx->st = JPS_NUMOUT;
                break;
            }

        } else if (ctx->st == JPS_NUM) {
            if (JSN_ISDIGIT(ch)) {
                ctx->st = JPS_NUM;
            } else if (ch == '.') {
                ctx->st = JPS_NUM;
                ctx->flags |= JPS_FFLOAT;
            } else if (ch == 'e' || ch == 'E' || ch == '-') {
                ctx->st = JPS_NUM;
                ctx->flags |= JPS_FSCIENTIFIC;
            } else {
                ctx->st = JPS_NUMOUT;
                break;
            }
        } else {
            TRRET(JSON_PARSE_NUM + 1);
        }

    }

    if (ctx->st == JPS_NUMOUT) {
        --ctx->idx;
        if (ctx->flags & (JPS_FFLOAT | JPS_FSCIENTIFIC)) {
            ctx->ty = JSON_FLOAT;
        } else {
            ctx->ty = JSON_INT;
        }
        TRRET(0);
    }

    TRRET(JSON_PARSE_NEEDMORE);
}

inline int
json_parse_tok(json_ctx_t *ctx)
{
    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];
        //TRACE("ch='%c'", ch);

        if (ctx->st == JPS_TOKIN) {
            if (JSN_ISALPHA(ch) || JSN_ISDIGIT(ch) || (ch == '.')) {
                ctx->st = JPS_TOK;
            } else {
                ctx->st = JPS_TOKOUT;
                break;
            }

        } else if (ctx->st == JPS_TOK) {
            if (JSN_ISALPHA(ch) || JSN_ISDIGIT(ch) || (ch == '.')) {
                ctx->st = JPS_TOK;
            } else {
                ctx->st = JPS_TOKOUT;
                break;
            }

        } else {
            TRRET(JSON_PARSE_TOK + 1);
        }

    }


    if (ctx->st == JPS_TOKOUT) {
        --ctx->idx;
        ctx->ty = JSON_NULL;
        TRRET(0);
    }

    TRRET(JSON_PARSE_NEEDMORE);
}

int
json_parse_obj(json_ctx_t *ctx)
{
    int res;

    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];

        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ch == '\0') {
            break;
        }

        if (ctx->st == JPS_OSTART) {
            if (ch == '}') {
                ctx->st = JPS_OSTOP;
                ctx->ty = JSON_OBJECT;

                if (ctx->ostop_cb != NULL &&
                    (res = ctx->ostop_cb(ctx, ctx->ostop_udata)) != 0) {

                    TRRET(res);
                }
                --ctx->nest;

                break;

            } else if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0) {
                    TRRET(res);
                }

                ctx->st = JPS_KEYOUT;
                ctx->v.s.end = ctx->idx;

                ++ctx->nest;
                if (ctx->key_cb != NULL &&
                    (res = ctx->key_cb(ctx, ctx->key_udata)) != 0) {
                    TRRET(res);
                }
                --ctx->nest;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_OBJ + 1);
            }

        } else if (ctx->st == JPS_KEYOUT) {
            if (ch == ':') {
                ctx->st = JPS_EVALUE;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_OBJ + 2);
            }

        } else if (ctx->st & (JPS_EVALUE)) {
            if (ch == '{') {
                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;
                ++ctx->nest;

                if (ctx->ostart_cb != NULL &&
                    (res = ctx->ostart_cb(ctx, ctx->ostart_udata)) != 0) {
                    TRRET(res);
                }

                if (ctx->value_cb != NULL &&
                    (res = ctx->value_cb(ctx, ctx->value_udata)) != 0) {
                    TRRET(res);
                }

                ++ctx->idx;

                if ((res = json_parse_obj(ctx)) != 0) {
                    TRRET(res);
                }

            } else if (ch == '[') {
                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;
                ++ctx->nest;

                if (ctx->astart_cb != NULL &&
                    (res = ctx->astart_cb(ctx, ctx->astart_udata)) != 0) {
                    TRRET(res);
                }

                if (ctx->value_cb != NULL &&
                    (res = ctx->value_cb(ctx, ctx->value_udata)) != 0) {
                    TRRET(res);
                }

                ++ctx->idx;

                if ((res = json_parse_array(ctx)) != 0) {
                    TRRET(res);
                }

            } else if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0) {
                    TRRET(res);
                }

                ctx->v.s.end = ctx->idx;

                ++ctx->nest;
                if (ctx->value_cb != NULL &&
                    (res = ctx->value_cb(ctx, ctx->value_udata)) != 0) {
                    TRRET(res);
                }
                --ctx->nest;

            } else if (JSN_ISDIGIT(ch) || ch == '-') {
                const char *tmp;

                ctx->st = JPS_NUMIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_num(ctx)) != 0) {
                    TRRET(res);
                }

                if (ctx->ty == JSON_INT) {
                    ctx->v.i = strtol(tmp, NULL, 10);
                } else if (ctx->ty == JSON_FLOAT) {
                    ctx->v.f = strtod(tmp, NULL);
                } else {
                    TRRET(JSON_PARSE_OBJ + 3);
                }

                ++ctx->nest;
                if (ctx->value_cb != NULL &&
                    (res = ctx->value_cb(ctx, ctx->value_udata)) != 0) {
                    TRRET(res);
                }
                --ctx->nest;

            } else if (JSN_ISALPHA(ch)) {
                const char *tmp;
                size_t toklen;

                ctx->st = JPS_TOKIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_tok(ctx)) != 0) {
                    TRRET(res);
                }

                toklen = ctx->in + ctx->idx - tmp + 1;

                if (toklen == 4 && memcmp(tmp, "true", 4) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = true;
                } else if (toklen == 5 && memcmp(tmp, "false", 5) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = 0;
                } else if (toklen == 4 && memcmp(tmp, "null", 4) == 0) {
                    ctx->ty = JSON_NULL;
                }

                ++ctx->nest;
                if (ctx->value_cb != NULL &&
                    (res = ctx->value_cb(ctx, ctx->value_udata)) != 0) {
                    TRRET(res);
                }
                --ctx->nest;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_OBJ + 4);
            }

        } else if (ctx->st & JPS_OUT) {
            if (ch == ',') {
                ctx->st = JPS_ENEXT;

            } else if (ch == '}') {
                ctx->st = JPS_OSTOP;
                ctx->ty = JSON_OBJECT;

                if (ctx->ostop_cb != NULL &&
                    (res = ctx->ostop_cb(ctx, ctx->ostop_udata)) != 0) {
                    TRRET(res);
                }
                --ctx->nest;

                break;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_OBJ + 5);
            }

        } else if (ctx->st == JPS_ENEXT) {
            if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0)  {
                    TRRET(res);
                }

                ctx->st = JPS_KEYOUT;
                ctx->v.s.end = ctx->idx;

                ++ctx->nest;
                if (ctx->key_cb != NULL &&
                    (res = ctx->key_cb(ctx, ctx->key_udata)) != 0) {
                    TRRET(res);
                }
                --ctx->nest;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_OBJ + 6);
            }

        } else {
            //TRACE("st=%s", JPS_TOSTR(ctx->st));
            TRRET(JSON_PARSE_OBJ + 7);
        }

    }

    TRRET(0);
}

int
json_parse_array(json_ctx_t *ctx)
{
    int res;

    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];

        //TRACE("idx=%ld ch='%c' st=%s", ctx->idx, ch, JPS_TOSTR(ctx->st));

        if (ch == '\0') {
            break;
        }

        if (ctx->st & (JPS_ASTART | JPS_ENEXT)) {
            if (ch == ']') {
                ctx->st = JPS_ASTOP;
                ctx->ty = JSON_ARRAY;

                if (ctx->astop_cb != NULL &&
                    ctx->astop_cb(ctx, ctx->astop_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 1);
                }
                --ctx->nest;

                break;

            } else if (ch == '[') {
                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;
                ++ctx->nest;

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, ctx->astart_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 2);
                }

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 3);
                }

                ++ctx->idx;

                if ((res = json_parse_array(ctx)) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 4);
                }

            } else if (ch == '{') {
                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;
                ++ctx->nest;

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, ctx->ostart_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 5);
                }

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 6);
                }

                ++ctx->idx;

                if ((res = json_parse_obj(ctx)) != 0) {
                    TRRET(res);
                }

            } else if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0) {
                    TRRET(res);
                }


                ctx->v.s.end = ctx->idx;

                ++ctx->nest;
                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 7);
                }
                --ctx->nest;

            } else if (JSN_ISDIGIT(ch) || ch == '-') {
                const char *tmp;

                ctx->st = JPS_NUMIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_num(ctx)) != 0) {
                    TRRET(res);
                }

                if (ctx->ty == JSON_INT) {
                    ctx->v.i = strtol(tmp, NULL, 10);
                } else if (ctx->ty == JSON_FLOAT) {
                    ctx->v.f = strtod(tmp, NULL);
                } else {
                    TRRET(JSON_PARSE_ARRAY + 8);
                }

                ++ctx->nest;
                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 9);
                }
                --ctx->nest;

            } else if (JSN_ISALPHA(ch)) {
                const char *tmp;
                size_t toklen;

                ctx->st = JPS_TOKIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_tok(ctx)) != 0) {
                    TRRET(res);
                }

                toklen = ctx->in + ctx->idx - tmp + 1;

                if (toklen == 4 && memcmp(tmp, "true", 4) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = true;
                } else if (toklen == 5 && memcmp(tmp, "false", 5) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = 0;
                } else if (toklen == 4 && memcmp(tmp, "null", 4) == 0) {
                    ctx->ty = JSON_NULL;
                }

                ++ctx->nest;
                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 10);
                }
                --ctx->nest;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_ARRAY + 11);
            }

        } else if (ctx->st & JPS_OUT) {
            if (ch == ',') {
                ctx->st = JPS_ENEXT;

            } else if (ch == ']') {
                ctx->st = JPS_ASTOP;
                ctx->ty = JSON_ARRAY;

                if (ctx->astop_cb != NULL &&
                    ctx->astop_cb(ctx, ctx->astop_udata) != 0) {
                    TRRET(JSON_PARSE_ARRAY + 12);
                }
                --ctx->nest;

                break;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                TRRET(JSON_PARSE_ARRAY + 13);
            }

        } else {
            //TRACE("st=%s", JPS_TOSTR(ctx->st));
            TRRET(JSON_PARSE_ARRAY + 14);
        }
    }

    TRRET(0);
}

int
json_parse(json_ctx_t *ctx, const char *in, size_t sz)
{
    int res;

    ctx->in = in;
    ctx->sz = sz;

    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];

        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ch == '\0') {
            break;
        }
        if (ctx->st == JPS_START) {
            if (ch == '{') {
                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;
                ++ctx->nest;

                if (ctx->ostart_cb != NULL &&
                    (res = ctx->ostart_cb(ctx, ctx->ostart_udata)) != 0) {
                    TRRET(res);
                }

                if (ctx->value_cb != NULL &&
                    (res = ctx->value_cb(ctx, ctx->item_udata)) != 0) {
                    TRRET(res);
                }

                ++ctx->idx;

                if ((res = json_parse_obj(ctx)) != 0) {
                    TRRET(res);
                }

            } else if (ch == '[') {
                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;
                ++ctx->nest;

                if (ctx->astart_cb != NULL &&
                    (res = ctx->astart_cb(ctx, ctx->astart_udata)) != 0) {
                    TRRET(res);
                }

                if (ctx->item_cb != NULL &&
                    (res = ctx->item_cb(ctx, ctx->item_udata)) != 0) {
                    TRRET(res);
                }

                ++ctx->idx;

                if ((res = json_parse_array(ctx)) != 0) {
                    TRRET(res);
                }
            }
        }
    }
    TRRET(0);
}


#define MNJSON_BS_PAIR_BODY(comma)                                             \
    ssize_t res;                                                               \
    mnbytes_t *tmp0, *tmp1;                                                    \
    tmp0 = bytes_new(SEOD(value) + 1);                                         \
    (void)memcpy(BCDATA(tmp0), SDATA(value, 0), SEOD(value));                  \
    BDATA(tmp0)[SEOD(value)] = '\0';                                           \
    tmp1 = bytes_json_escape(tmp0);                                            \
    res = bytestream_nprintf(bs,                                               \
                             BSZ(key) - 1 + 8 + BSZ(tmp1) - 1,                 \
                             "\"%s\":\"%s\"" comma, BDATA(key), BDATA(tmp1));  \
    BYTES_DECREF(&tmp0);                                                       \
    BYTES_DECREF(&tmp1);                                                       \
    return res                                                                 \


ssize_t
mnjson_bs_pair0(mnbytestream_t *bs, const mnbytes_t *key, mnbytestream_t *value)
{
    MNJSON_BS_PAIR_BODY(",");
}


ssize_t
mnjson_bs_pair1(mnbytestream_t *bs, const mnbytes_t *key, mnbytestream_t *value)
{
    MNJSON_BS_PAIR_BODY("");
}


#define MNJSON_BS_ITEM_BODY(comma)                                     \
    ssize_t res;                                                       \
    mnbytes_t *tmp0, *tmp1;                                            \
    tmp0 = bytes_new(SEOD(value) + 1);                                 \
    (void)memcpy(BCDATA(tmp0), SDATA(value, 0), SEOD(value));          \
    BDATA(tmp0)[SEOD(value)] = '\0';                                   \
    tmp1 = bytes_json_escape(tmp0);                                    \
    res = bytestream_nprintf(bs,                                       \
                             4 + BSZ(tmp1) - 1,                        \
                             "\"%s\"" comma, BDATA(tmp1));             \
    BYTES_DECREF(&tmp0);                                               \
    BYTES_DECREF(&tmp1);                                               \
    return res                                                         \


ssize_t
mnjson_bs_item0(mnbytestream_t *bs, mnbytestream_t *value)
{
    MNJSON_BS_ITEM_BODY(",");
}


ssize_t
mnjson_bs_item1(mnbytestream_t *bs, mnbytestream_t *value)
{
    MNJSON_BS_ITEM_BODY("");
}




#define MNJSON_BYTES_PAIR_BODY(comma)                                          \
    ssize_t res;                                                               \
    mnbytes_t *tmp;                                                            \
    tmp = bytes_json_escape(value);                                            \
    res = bytestream_nprintf(bs,                                               \
                             BSZ(key) - 1 + 8 + BSZ(tmp) - 1,                  \
                             "\"%s\":\"%s\"" comma, BDATA(key), BDATA(tmp));   \
    BYTES_DECREF(&tmp);                                                        \
    return res                                                                 \



ssize_t
mnjson_bytes_pair0(mnbytestream_t *bs, const mnbytes_t *key, const mnbytes_t *value)
{
    MNJSON_BYTES_PAIR_BODY(",");
}


ssize_t
mnjson_bytes_pair1(mnbytestream_t *bs, const mnbytes_t *key, const mnbytes_t *value)
{
    MNJSON_BYTES_PAIR_BODY("");
}


#define MNJSON_BYTES_ITEM_BODY(comma)                          \
    ssize_t res;                                               \
    mnbytes_t *tmp;                                            \
    tmp = bytes_json_escape(value);                            \
    res = bytestream_nprintf(bs,                               \
                             4 + BSZ(tmp) - 1,                 \
                             "\"%s\"" comma, BDATA(tmp));      \
    BYTES_DECREF(&tmp);                                        \
    return res                                                 \


ssize_t
mnjson_bytes_item0(mnbytestream_t *bs, const mnbytes_t *value)
{
    MNJSON_BYTES_ITEM_BODY(",");
}


ssize_t
mnjson_bytes_item1(mnbytestream_t *bs, const mnbytes_t *value)
{
    MNJSON_BYTES_ITEM_BODY("");
}


#define MNJSON_INT_PAIR_BODY(comma)                                    \
    return bytestream_nprintf(bs,                                      \
                              BSZ(key) - 1 + 64,                       \
                              "\"%s\":%ld" comma, BDATA(key), value)   \


ssize_t
mnjson_int_pair0(mnbytestream_t *bs, const mnbytes_t *key, intmax_t value)
{
    MNJSON_INT_PAIR_BODY(",");
}


ssize_t
mnjson_int_pair1(mnbytestream_t *bs, const mnbytes_t *key, intmax_t value)
{
    MNJSON_INT_PAIR_BODY("");
}



#define MNJSON_INT_ITEM_BODY(comma)                            \
    return bytestream_nprintf(bs, 64, "%ld" comma, value)      \



ssize_t
mnjson_int_item0(mnbytestream_t *bs, intmax_t value)
{
    MNJSON_INT_ITEM_BODY(",");
}


ssize_t
mnjson_int_item1(mnbytestream_t *bs, intmax_t value)
{
    MNJSON_INT_ITEM_BODY("");
}



#define MNJSON_FLOAT_PAIR_BODY(comma)                                  \
    return bytestream_nprintf(bs,                                      \
                              BSZ(key) - 1 + 1024,                     \
                              "\"%s\":%lf" comma, BDATA(key), value)   \


ssize_t
mnjson_float_pair0(mnbytestream_t *bs, const mnbytes_t *key, double value)
{
    MNJSON_FLOAT_PAIR_BODY(",");
}


ssize_t
mnjson_float_pair1(mnbytestream_t *bs, const mnbytes_t *key, double value)
{
    MNJSON_FLOAT_PAIR_BODY("");
}



#define MNJSON_FLOAT_ITEM_BODY(comma)                            \
    return bytestream_nprintf(bs, 1024, "%lf" comma, value)      \



ssize_t
mnjson_float_item0(mnbytestream_t *bs, double value)
{
    MNJSON_FLOAT_ITEM_BODY(",");
}


ssize_t
mnjson_float_item1(mnbytestream_t *bs, double value)
{
    MNJSON_FLOAT_ITEM_BODY("");
}



#define MNJSON_BOOL_PAIR_BODY(comma)                                   \
    return bytestream_nprintf(bs,                                      \
                              BSZ(key) - 1 + 8,                        \
                              "\"%s\":%s" comma,                       \
                              BDATA(key), value ? "true" : "flse")     \


ssize_t
mnjson_bool_pair0(mnbytestream_t *bs, const mnbytes_t *key, bool value)
{
    MNJSON_BOOL_PAIR_BODY(",");
}


ssize_t
mnjson_bool_pair1(mnbytestream_t *bs, const mnbytes_t *key, bool value)
{
    MNJSON_BOOL_PAIR_BODY("");
}



#define MNJSON_BOOL_ITEM_BODY(comma)                   \
    return bytestream_nprintf(bs, 8, "%s" comma,       \
                              value ? "true" : "false")\



ssize_t
mnjson_bool_item0(mnbytestream_t *bs, bool value)
{
    MNJSON_BOOL_ITEM_BODY(",");
}


ssize_t
mnjson_bool_item1(mnbytestream_t *bs, bool value)
{
    MNJSON_BOOL_ITEM_BODY("");
}


#define MNJSON_CB_PAIR_BODY(_comma)                    \
    ssize_t res;                                       \
    res = bytestream_nprintf(bs,                       \
                             BSZ(key) - 1 + 8,         \
                             "\"%s\":", BDATA(key));   \
    res += cb(bs, udata);                              \
    _comma                                             \
    return res                                         \


ssize_t
mnjson_cb_pair0 (mnbytestream_t *bs,
                 const mnbytes_t *key,
                 ssize_t (*cb) (mnbytestream_t *, void *udata),
                 void *udata)
{
    MNJSON_CB_PAIR_BODY(
        res += bytestream_cat(bs, 1, ",");
    );
}


ssize_t
mnjson_cb_pair1 (mnbytestream_t *bs,
                 const mnbytes_t *key,
                 ssize_t (*cb) (mnbytestream_t *, void *udata),
                 void *udata)
{
    MNJSON_CB_PAIR_BODY(;);
}


#define MNJSON_CB_ITEM_BODY(_comma)    \
    ssize_t res;                       \
    res = cb(bs, udata);               \
    _comma                             \
    return res                         \


ssize_t
mnjson_cb_item0 (mnbytestream_t *bs,
                 ssize_t (*cb) (mnbytestream_t *, void *udata),
                 void *udata)
{
    MNJSON_CB_ITEM_BODY(
        res += bytestream_cat(bs, 1, ",");
    );
}


ssize_t
mnjson_cb_item1 (mnbytestream_t *bs,
                 ssize_t (*cb) (mnbytestream_t *, void *udata),
                 void *udata)
{
    MNJSON_CB_ITEM_BODY(;);
}


ssize_t
mnjson_chop_comma(mnbytestream_t *bs)
{
    ssize_t res;
    res = 0;
    if (*SDATA(bs, SEOD(bs) - 1) == ',') {
        SADVANCEEOD(bs, -1);
        res = -1;
    }
    return res;
}





// vim:list
