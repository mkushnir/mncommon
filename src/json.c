#include <stdlib.h>
#include <string.h>

//#define TRRET_DEBUG_VERBOSE
#include "mrkcommon/dumpm.h"
#include "mrkcommon/json.h"

#include "diag.h"

#define JSN_ISDIGIT(c) (((c) >= '0' && (c) <= '9'))
#define JSN_ISALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define JSN_ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')
int
json_init(json_ctx_t *ctx, json_cb cb, void *udata)
{
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
        TRACEC("\t\"%s\"%s", ctx->v.s, (ctx->st & JPS_KEYOUT ? ":" : ""));
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
    return 0;
}

inline int
json_parse_str(json_ctx_t *ctx, char *in, size_t sz)
{
    ctx->flags &= ~JPS_FNEEDUNESCAPE;

    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];
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
json_parse_num(json_ctx_t *ctx, char *in, size_t sz)
{
    ctx->flags &= ~(JPS_FFLOAT | JPS_FSCIENTIFIC);

    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];
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
json_parse_tok(json_ctx_t *ctx, char *in, size_t sz)
{
    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];
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
json_parse_obj(json_ctx_t *ctx, char *in, size_t sz)
{
    int res;

    ++ctx->obj_level;

    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];

        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ch == '\0') {
            break;
        }

        if (ctx->st == JPS_OSTART) {
            if (ch == '}') {
                ctx->st = JPS_OEND;
                ctx->ty = JSON_OBJECT;

                if (ctx->oend_cb != NULL &&
                    ctx->oend_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                break;

            } else if (ch == '"') {
                char *tmp;

                ctx->st = JPS_STRIN;
                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_str(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                ctx->st = JPS_KEYOUT;
                in[ctx->idx] = '\0';
                ctx->v.s = tmp;

                if (ctx->key_cb != NULL &&
                    ctx->key_cb(ctx, in, ctx->udata) != 0) {

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
                char *tmp;

                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_obj(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

            } else if (ch == '[') {
                char *tmp;

                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_array(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

            } else if (ch == '"') {
                char *tmp;

                ctx->st = JPS_STRIN;
                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_str(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                in[ctx->idx] = '\0';
                ctx->v.s = tmp;

                if (ctx->value_cb != NULL &&
                    ctx->value_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISDIGIT(ch) || ch == '-') {
                char *tmp;

                ctx->st = JPS_NUMIN;
                tmp = in + ctx->idx;

                if ((res = json_parse_num(ctx, in, sz)) != 0) {
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
                    ctx->value_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

            } else if (JSN_ISALPHA(ch)) {
                char *tmp;
                size_t toklen;

                ctx->st = JPS_TOKIN;
                tmp = in + ctx->idx;

                if ((res = json_parse_tok(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

                toklen = in + ctx->idx - tmp + 1;

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
                    ctx->value_cb(ctx, in, ctx->udata) != 0) {

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
                    ctx->oend_cb(ctx, in, ctx->udata) != 0) {

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
                char *tmp;

                ctx->st = JPS_STRIN;
                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_str(ctx, in, sz)) != 0)  {
                    --ctx->obj_level;
                    TRRET(res);
                }

                ctx->st = JPS_KEYOUT;
                in[ctx->idx] = '\0';
                ctx->v.s = tmp;

                if (ctx->key_cb != NULL &&
                    ctx->key_cb(ctx, in, ctx->udata) != 0) {

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
            TRACE("st=%s", JPS_TOSTR(ctx->st));
            --ctx->obj_level;
            TRRET(JSON_PARSE_OBJ + 7);
        }

    }
    --ctx->obj_level;
    TRRET(0);
}

int
json_parse_array(json_ctx_t *ctx, char *in, size_t sz)
{
    int res;

    ++ctx->array_level;

    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];

        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ch == '\0') {
            break;
        }

        if (ctx->st & (JPS_ASTART | JPS_ENEXT)) {
            if (ch == ']') {
                ctx->st = JPS_AEND;
                ctx->ty = JSON_ARRAY;

                if (ctx->aend_cb != NULL &&
                    ctx->aend_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                break;

            } else if (ch == '[') {
                char *tmp;

                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_array(ctx, in, sz)) != 0) {
                    --ctx->array_level;
                    TRRET(JSON_PARSE_ARRAY + 1);
                }

            } else if (ch == '{') {
                char *tmp;

                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_obj(ctx, in, sz)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }

            } else if (ch == '"') {
                char *tmp;

                ctx->st = JPS_STRIN;
                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_str(ctx, in, sz)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }

                in[ctx->idx] = '\0';
                ctx->v.s = tmp;

                if (ctx->item_cb != NULL &&
                    ctx->item_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

            } else if (JSN_ISDIGIT(ch) || ch == '-') {
                char *tmp;

                ctx->st = JPS_NUMIN;
                tmp = in + ctx->idx;

                if ((res = json_parse_num(ctx, in, sz)) != 0) {
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
                    ctx->item_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->array_level;
                    TRRET(-1);
                }

            } else if (JSN_ISALPHA(ch)) {
                char *tmp;
                size_t toklen;

                ctx->st = JPS_TOKIN;
                tmp = in + ctx->idx;

                if ((res = json_parse_tok(ctx, in, sz)) != 0) {
                    --ctx->array_level;
                    TRRET(res);
                }

                toklen = in + ctx->idx - tmp + 1;

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
                    ctx->item_cb(ctx, in, ctx->udata) != 0) {

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
                    ctx->aend_cb(ctx, in, ctx->udata) != 0) {

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
            TRACE("st=%s", JPS_TOSTR(ctx->st));
            --ctx->array_level;
            TRRET(JSON_PARSE_ARRAY + 5);
        }
    }
    TRRET(0);
}

int
json_parse(json_ctx_t *ctx, char *in, size_t sz)
{
    int res;

    for (; ctx->idx < sz; ++(ctx->idx)) {
        char ch;

        ch = in[ctx->idx];

        //TRACE("idx=%ld ch='%c'", ctx->idx, ch);

        if (ch == '\0') {
            break;
        }
        if (ctx->st == JPS_START) {
            if (ch == '{') {
                char *tmp;

                ctx->st = JPS_OSTART;
                ctx->ty = JSON_OBJECT;

                if (ctx->ostart_cb != NULL &&
                    ctx->ostart_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_obj(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }

            } else if (ch == '[') {
                char *tmp;

                ctx->st = JPS_ASTART;
                ctx->ty = JSON_ARRAY;

                if (ctx->astart_cb != NULL &&
                    ctx->astart_cb(ctx, in, ctx->udata) != 0) {

                    --ctx->obj_level;
                    TRRET(-1);
                }

                ++ctx->idx;
                tmp = in + ctx->idx;

                if ((res = json_parse_array(ctx, in, sz)) != 0) {
                    --ctx->obj_level;
                    TRRET(res);
                }
            }
        }
    }
    TRRET(0);
}

// vim:list
