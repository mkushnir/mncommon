#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
json_init(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->in = NULL;
    ctx->sz = 0;
    ctx->cb = cb;
    ctx->udata = udata;
    ctx->ostart_cb = cb;
    ctx->ostart_udata = udata;
    ctx->oend_cb = cb;
    ctx->oend_udata = udata;
    ctx->astart_cb = cb;
    ctx->astart_udata = udata;
    ctx->aend_cb = cb;
    ctx->aend_udata = udata;
    ctx->key_cb = cb;
    ctx->key_udata = udata;
    ctx->value_cb = cb;
    ctx->value_udata = udata;
    ctx->item_cb = cb;
    ctx->item_udata = udata;
    ctx->st = JPS_START;
    ctx->flags = 0;
    ctx->obj_level = 0;
    ctx->array_level = 0;
    ctx->buf = NULL;
    ctx->idx = 0;
    return 0;
}

void
json_set_ostart_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->ostart_cb = cb;
    ctx->ostart_udata = udata;
}

void
json_set_oend_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->oend_cb = cb;
    ctx->oend_udata = udata;
}

void
json_set_astart_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->astart_cb = cb;
    ctx->astart_udata = udata;
}

void
json_set_aend_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->aend_cb = cb;
    ctx->aend_udata = udata;
}

void
json_set_key_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->key_cb = cb;
    ctx->key_udata = udata;
}

void
json_set_value_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->value_cb = cb;
    ctx->value_udata = udata;
}

void
json_set_item_cb(json_ctx_t *ctx, json_cb cb, void *udata)
{
    ctx->item_cb = cb;
    ctx->item_udata = udata;
}

void
json_dump(json_ctx_t *ctx)
{
    TRACEN("idx=%ld st=%s", ctx->idx, JPS_TOSTR(ctx->st));
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
        TRACEC("\t%ld", ctx->v.i);
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


int
json_fini(json_ctx_t *ctx)
{
    ctx->cb = NULL;
    ctx->udata = NULL;
    if (ctx->buf != NULL) {
        free(ctx->buf);
        ctx->buf = NULL;
    }
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

    ++ctx->obj_level;

    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];

        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ch == '\0') {
            break;
        }

        if (ctx->st == JPS_OSTART) {
            if (ch == '}') {
                ctx->st = JPS_OEND;
                ctx->ty = JSON_OBJECT;

                if (ctx->oend_cb != NULL &&
                    ctx->oend_cb(ctx, ctx->oend_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                break;

            } else if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                ctx->st = JPS_KEYOUT;
                ctx->v.s.end = ctx->idx;

                if (ctx->key_cb != NULL &&
                    ctx->key_cb(ctx, ctx->key_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->obj_level;
                TRRET(JSON_PARSE_OBJ + 1);
            }

        } else if (ctx->st == JPS_KEYOUT) {
            if (ch == ':') {
                ctx->st = JPS_EVALUE;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->obj_level;
                TRRET(JSON_PARSE_OBJ + 2);
            }

        } else if (ctx->st & (JPS_EVALUE)) {

            if (ch == '{') {
                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, ctx->value_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, ctx->ostart_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;

                if ((res = json_parse_obj(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

            } else if (ch == '[') {
                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, ctx->value_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, ctx->astart_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;

                if ((res = json_parse_array(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

            } else if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                ctx->v.s.end = ctx->idx;

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, ctx->value_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISDIGIT(ch) || ch == '-') {
                const char *tmp;

                ctx->st = JPS_NUMIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_num(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                if (ctx->ty == JSON_INT) {
                    ctx->v.i = strtol(tmp, NULL, 10);
                } else if (ctx->ty == JSON_FLOAT) {
                    ctx->v.f = strtod(tmp, NULL);
                } else {
                    --ctx->obj_level;
                    TRRET(JSON_PARSE_OBJ + 3);
                }

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, ctx->value_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISALPHA(ch)) {
                const char *tmp;
                size_t toklen;

                ctx->st = JPS_TOKIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_tok(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                toklen = ctx->in + ctx->idx - tmp + 1;

                if (toklen == 4 && memcmp(tmp, "true", 4) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = 1;
                } else if (toklen == 5 && memcmp(tmp, "false", 5) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = 0;
                } else if (toklen == 4 && memcmp(tmp, "null", 4) == 0) {
                    ctx->ty = JSON_NULL;
                }

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, ctx->value_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->obj_level;
                TRRET(JSON_PARSE_OBJ + 4);
            }

        } else if (ctx->st & JPS_OUT) {

            if (ch == ',') {
                ctx->st = JPS_ENEXT;

            } else if (ch == '}') {
                ctx->st = JPS_OEND;
                ctx->ty = JSON_OBJECT;

                if (ctx->oend_cb != NULL &&
                    ctx->oend_cb(ctx, ctx->oend_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                break;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->obj_level;
                TRRET(JSON_PARSE_OBJ + 5);
            }


        } else if (ctx->st == JPS_ENEXT) {
            if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0)  {
                    --ctx->obj_level;
                    TRRET(res);
                }

                ctx->st = JPS_KEYOUT;
                ctx->v.s.end = ctx->idx;

                if (ctx->key_cb != NULL &&
                    ctx->key_cb(ctx, ctx->key_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->obj_level;
                TRRET(JSON_PARSE_OBJ + 6);
            }


        } else {
            //TRACE("st=%s", JPS_TOSTR(ctx->st));
            --ctx->obj_level;
            TRRET(JSON_PARSE_OBJ + 7);
        }

    }
    --ctx->obj_level;
    TRRET(0);
}

int
json_parse_array(json_ctx_t *ctx)
{
    int res;

    ++ctx->array_level;

    for (; ctx->idx < ctx->sz; ++(ctx->idx)) {
        char ch;

        ch = ctx->in[ctx->idx];

        //TRACE("idx=%ld ch='%c' st=%s", ctx->idx, ch, JPS_TOSTR(ctx->st));

        if (ch == '\0') {
            break;
        }

        if (ctx->st & (JPS_ASTART | JPS_ENEXT)) {
            if (ch == ']') {
                ctx->st = JPS_AEND;
                ctx->ty = JSON_ARRAY;

                if (ctx->aend_cb != NULL &&
                    ctx->aend_cb(ctx, ctx->aend_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                break;

            } else if (ch == '[') {
                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, ctx->astart_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                ++ctx->idx;

                if ((res = json_parse_array(ctx)) != 0) {
                    --ctx->array_level;
                    TRRET(JSON_PARSE_ARRAY + 1);
                }

            } else if (ch == '{') {
                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, ctx->ostart_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                ++ctx->idx;

                if ((res = json_parse_obj(ctx)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }

            } else if (ch == '"') {
                ctx->st = JPS_STRIN;
                ++ctx->idx;
                ctx->v.s.start = ctx->idx;

                if ((res = json_parse_str(ctx)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }


                ctx->v.s.end = ctx->idx;

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

            } else if (JSN_ISDIGIT(ch) || ch == '-') {
                const char *tmp;

                ctx->st = JPS_NUMIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_num(ctx)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }

                if (ctx->ty == JSON_INT) {
                    ctx->v.i = strtol(tmp, NULL, 10);
                } else if (ctx->ty == JSON_FLOAT) {
                    ctx->v.f = strtod(tmp, NULL);
                } else {
                    --ctx->array_level;
                    TRRET(JSON_PARSE_ARRAY + 2);
                }

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

            } else if (JSN_ISALPHA(ch)) {
                const char *tmp;
                size_t toklen;

                ctx->st = JPS_TOKIN;
                tmp = ctx->in + ctx->idx;

                if ((res = json_parse_tok(ctx)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }

                toklen = ctx->in + ctx->idx - tmp + 1;

                if (toklen == 4 && memcmp(tmp, "true", 4) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = 1;
                } else if (toklen == 5 && memcmp(tmp, "false", 5) == 0) {
                    ctx->ty = JSON_BOOLEAN;
                    ctx->v.b = 0;
                } else if (toklen == 4 && memcmp(tmp, "null", 4) == 0) {
                    ctx->ty = JSON_NULL;
                }

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, ctx->item_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->array_level;
                TRRET(JSON_PARSE_ARRAY + 3);
            }

        } else if (ctx->st & JPS_OUT) {

            if (ch == ',') {
                ctx->st = JPS_ENEXT;

            } else if (ch == ']') {
                ctx->st = JPS_AEND;
                ctx->ty = JSON_ARRAY;

                if (ctx->aend_cb != NULL &&
                    ctx->aend_cb(ctx, ctx->aend_udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                break;

            } else if (JSN_ISSPACE(ch)) {
                continue;

            } else {
                --ctx->array_level;
                TRRET(JSON_PARSE_ARRAY + 4);
            }

        } else {
            //TRACE("st=%s", JPS_TOSTR(ctx->st));
            --ctx->array_level;
            TRRET(JSON_PARSE_ARRAY + 5);
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

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, ctx->ostart_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;

                if ((res = json_parse_obj(ctx)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

            } else if (ch == '[') {
                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, ctx->astart_udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;

                if ((res = json_parse_array(ctx)) != 0) {
                    --ctx->obj_level;
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
mnjson_bs_pair0(mnbytestream_t *bs, mnbytes_t *key, mnbytestream_t *value)
{
    MNJSON_BS_PAIR_BODY(",");
}


ssize_t
mnjson_bs_pair1(mnbytestream_t *bs, mnbytes_t *key, mnbytestream_t *value)
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
mnjson_bytes_pair0(mnbytestream_t *bs, mnbytes_t *key, mnbytes_t *value)
{
    MNJSON_BYTES_PAIR_BODY(",");
}


ssize_t
mnjson_bytes_pair1(mnbytestream_t *bs, mnbytes_t *key, mnbytes_t *value)
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
mnjson_bytes_item0(mnbytestream_t *bs, mnbytes_t *value)
{
    MNJSON_BYTES_ITEM_BODY(",");
}


ssize_t
mnjson_bytes_item1(mnbytestream_t *bs, mnbytes_t *value)
{
    MNJSON_BYTES_ITEM_BODY("");
}


#define MNJSON_INT_PAIR_BODY(comma)                                    \
    return bytestream_nprintf(bs,                                      \
                              BSZ(key) - 1 + 64,                       \
                              "\"%s\":%ld" comma, BDATA(key), value)   \


ssize_t
mnjson_int_pair0(mnbytestream_t *bs, mnbytes_t *key, intmax_t value)
{
    MNJSON_INT_PAIR_BODY(",");
}


ssize_t
mnjson_int_pair1(mnbytestream_t *bs, mnbytes_t *key, intmax_t value)
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
mnjson_float_pair0(mnbytestream_t *bs, mnbytes_t *key, double value)
{
    MNJSON_FLOAT_PAIR_BODY(",");
}


ssize_t
mnjson_float_pair1(mnbytestream_t *bs, mnbytes_t *key, double value)
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
mnjson_bool_pair0(mnbytestream_t *bs, mnbytes_t *key, bool value)
{
    MNJSON_BOOL_PAIR_BODY(",");
}


ssize_t
mnjson_bool_pair1(mnbytestream_t *bs, mnbytes_t *key, bool value)
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
