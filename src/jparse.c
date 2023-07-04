#include <assert.h>
#include <fcntl.h>

#include <mncommon/malloc.h>
#include <mncommon/jparse.h>
//#define TRRET_DEBUG
#include <mncommon/dumpm.h>
#include <mncommon/util.h>
#include "diag.h"


/*
 * scanner
 */
#define JSN_ISDIGIT(c) (((c) >= '0' && (c) <= '9'))
#define JSN_ISALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define JSN_ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

static int jparse_expect_tok(jparse_ctx_t *, mnbytes_t **);
static int expect_maybe_null(jparse_ctx_t *);

static mnbytes_t _null = BYTES_INITIALIZER("null");
static mnbytes_t _true = BYTES_INITIALIZER("true");
static mnbytes_t _false = BYTES_INITIALIZER("false");


static int
reach_nonblank(jparse_ctx_t *jctx)
{
    int res = 0;
    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs,
                                        (void *)(intptr_t)jctx->fd) != 0) {
                res = REACH_NONBLANK + 1;
                break;
            }
        }
        ch = SPCHR(&jctx->bs);
        if (JSN_ISSPACE(ch)) {
            SINCR(&jctx->bs);
        } else {
            break;
        }
    }
    return res;
}


UNUSED static int
probe_nonblank(jparse_ctx_t *jctx)
{
    int res = 0;
    char ch;

    if (SNEEDMORE(&jctx->bs)) {
        if (bytestream_consume_data(&jctx->bs,
                                    (void *)(intptr_t)jctx->fd) != 0) {
            res = PROBE_NONBLANK + 1;
            goto end;
        }
    }
    ch = SPCHR(&jctx->bs);
    if (!JSN_ISSPACE(ch)) {
        res = PROBE_NONBLANK + 2;
    }

end:
    return res;
}


UNUSED static int
reach_blank(jparse_ctx_t *jctx)
{
    int res = 0;
    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs, (void *)(intptr_t)jctx->fd) != 0) {
                res = REACH_BLANK + 1;
                break;
            }
        }
        ch = SPCHR(&jctx->bs);
        if (!JSN_ISSPACE(ch)) {
            SINCR(&jctx->bs);
        } else {
            break;
        }
    }
    return res;
}


#define REACH_BODY(delim, msg)                                                 \
    off_t spos = SPOS(&jctx->bs);                                              \
    int res = 0;                                                               \
    while (1) {                                                                \
        char ch;                                                               \
                                                                               \
        if (SNEEDMORE(&jctx->bs)) {                                            \
            if (bytestream_consume_data(                                       \
                        &jctx->bs,                                             \
                        (void *)(intptr_t)jctx->fd) != 0) {                    \
                res = msg + 1;                                                 \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        ch = SPCHR(&jctx->bs);                                                 \
        if (ch == delim) {                                                     \
            SINCR(&jctx->bs);                                                  \
            break;                                                             \
                                                                               \
        } else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {      \
            SINCR(&jctx->bs);                                                  \
        } else {                                                               \
/*                                                                             \
            TRACE("failing %c at %ld", ch, SPOS(&jctx->bs));                   \
            jparse_dump_current_pos(jctx, 16);                                 \
 */                                                                            \
            if (jctx->errorpos == -1) {                                        \
                jctx->errorpos = SPOS(&jctx->bs);                              \
            }                                                                  \
            SPOS(&jctx->bs) = spos;                                            \
            res = msg + 2;                                                     \
            break;                                                             \
        }                                                                      \
    }                                                                          \
    TRRET(res);                                                                \


#define PROBE_BODY(delim, msg)                                                 \
    off_t spos = SPOS(&jctx->bs);                                              \
    int res = 0;                                                               \
    while (1) {                                                                \
        char ch;                                                               \
                                                                               \
        if (SNEEDMORE(&jctx->bs)) {                                            \
            if (bytestream_consume_data(                                       \
                        &jctx->bs,                                             \
                        (void *)(intptr_t)jctx->fd) != 0) {                    \
                res = msg + 1;                                                 \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        ch = SPCHR(&jctx->bs);                                                 \
        if (ch == delim) {                                                     \
            break;                                                             \
                                                                               \
        } else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {      \
            SINCR(&jctx->bs);                                                  \
        } else {                                                               \
            /* TRACE("failing %c at %ld", ch, SPOS(&jctx->bs)); */             \
            if (jctx->errorpos == -1) {                                        \
                jctx->errorpos = SPOS(&jctx->bs);                              \
            }                                                                  \
            SPOS(&jctx->bs) = spos;                                            \
            res = msg + 2;                                                     \
            break;                                                             \
        }                                                                      \
    }                                                                          \
    TRRET(res);                                                                \


static int
reach_ostart(jparse_ctx_t *jctx)
{
    REACH_BODY('{', REACH_OSTART)
}


static int
probe_ostart(jparse_ctx_t *jctx)
{
    PROBE_BODY('{', PROBE_OSTART)
}


static int
reach_ostop(jparse_ctx_t *jctx)
{
    REACH_BODY('}', REACH_OSTOP)
}


UNUSED static int
probe_ostop(jparse_ctx_t *jctx)
{
    PROBE_BODY('}', PROBE_OSTOP)
}


static int
reach_astart(jparse_ctx_t *jctx)
{
    REACH_BODY('[', REACH_ASTART)
}


static int
probe_astart(jparse_ctx_t *jctx)
{
    PROBE_BODY('[', PROBE_ASTART)
}


static int
reach_astop(jparse_ctx_t *jctx)
{
    REACH_BODY(']', REACH_ASTOP)
}


UNUSED static int
probe_astop(jparse_ctx_t *jctx)
{
    PROBE_BODY(']', PROBE_ASTOP)
}


static int
reach_comma(jparse_ctx_t *jctx)
{
    REACH_BODY(',', REACH_COMMA)
}


UNUSED static int
probe_comma(jparse_ctx_t *jctx)
{
    PROBE_BODY(',', PROBE_COMMA)
}


static int
reach_dquote(jparse_ctx_t *jctx)
{
    REACH_BODY('"', REACH_DQUOTE)
}


static int
probe_dquote(jparse_ctx_t *jctx)
{
    PROBE_BODY('"', PROBE_DQUOTE)
}


static int
reach_colon(jparse_ctx_t *jctx)
{
    REACH_BODY(':', REACH_COLON)
}


UNUSED static int
probe_colon(jparse_ctx_t *jctx)
{
    PROBE_BODY(':', PROBE_COLON)
}


/*
 * jparse_value_t
 */

void
jparse_value_init(jparse_value_t *jval)
{
    jval->k = NULL;
    jval->v.s = NULL;
    jval->cb = NULL;
    jval->udata = NULL;
    jval->ty = JSON_UNDEF;
}


/*
 * scalar
 */
static int
_jparse_expect_object_ignore_cb(jparse_ctx_t *jctx,
                                jparse_value_t *jval,
                                void *udata)
{
    return jparse_expect_anykvp_ignore(jctx, &jval->k, jval, udata);
}


static int
_jparse_expect_array_ignore_cb(jparse_ctx_t *jctx,
                               jparse_value_t *jval,
                               void *udata)
{
    return jparse_expect_item_ignore(jctx, jval, udata);
}


int
jparse_expect_ignore(jparse_ctx_t *jctx, jparse_value_t *jval, void *udata)
{
    int res;
    off_t start;

    jctx->errorpos = -1;
    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_IGNORE + 1);
    }

    start = SPOS(&jctx->bs);
    if ((res = expect_maybe_null(jctx)) == 0) {
        jval->ty = JSON_NULL;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if ((res = jparse_expect_bool(jctx, &jval->v.b, udata)) == 0) {
        jval->ty = JSON_BOOLEAN;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if (probe_dquote(jctx) == 0) {
        if ((res = jparse_expect_str(jctx, &jval->v.s, udata)) == 0) {
            jval->ty = JSON_STRING;
            goto end;
        }
    } else if (probe_ostart(jctx) == 0) {
        jval->ty = JSON_OBJECT;
        if ((res = jparse_expect_object_iter(jctx,
                _jparse_expect_object_ignore_cb,
                jval,
                udata)) == 0) {
            jval->ty = JSON_OBJECT;
            goto end;
        }
    } else if (probe_astart(jctx) == 0) {
        jval->ty = JSON_ARRAY;
        if ((res = jparse_expect_array_iter(jctx,
                _jparse_expect_array_ignore_cb,
                jval,
                udata)) == 0) {
            goto end;
        }
    } else {
        if ((res = jparse_expect_float(jctx, &jval->v.f, udata)) == 0) {
            jval->ty = JSON_FLOAT;
            goto end;
        }
    }

    if (jctx->errorpos == -1) {
        jctx->errorpos = SPOS(&jctx->bs);
    }
    SPOS(&jctx->bs) = start;
    res = JPARSE_EXPECT_IGNORE + 1;

end:
    return res;
}


int
jparse_expect_any(jparse_ctx_t *jctx, jparse_value_t *jval, void *udata)
{
    int res;
    off_t start;

    jctx->errorpos = -1;
    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANY + 1);
    }

    start = SPOS(&jctx->bs);
    if ((res = expect_maybe_null(jctx)) == 0) {
        jval->ty = JSON_NULL;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if ((res = jparse_expect_bool(jctx, &jval->v.b, udata)) == 0) {
        jval->ty = JSON_BOOLEAN;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if (probe_dquote(jctx) == 0) {
        if ((res = jparse_expect_str(jctx, &jval->v.s, udata)) == 0) {
            jval->ty = JSON_STRING;
            goto end;
        }
    } else if (probe_ostart(jctx) == 0) {
        jval->ty = JSON_OBJECT;
        if ((res = jparse_expect_object(jctx,
                                        jval->cb,
                                        jval,
                                        jval->udata)) == 0) {
            goto end;
        }
    } else if (probe_astart(jctx) == 0) {
        jval->ty = JSON_ARRAY;
        if ((res = jparse_expect_array(jctx,
                                       jval->cb,
                                       jval,
                                       jval->udata)) == 0) {
            goto end;
        }
    } else {
        if ((res = jparse_expect_float(jctx, &jval->v.f, udata)) == 0) {
            jval->ty = JSON_FLOAT;
            goto end;
        }
    }

    if (jctx->errorpos == -1) {
        jctx->errorpos = SPOS(&jctx->bs);
    }
    SPOS(&jctx->bs) = start;
    res = JPARSE_EXPECT_ANY + 2;

end:
    return res;
}


static int
jparse_expect_tok(jparse_ctx_t *jctx, mnbytes_t **val)
{
    int res;
    off_t start, stop;
    size_t sz;

    jctx->errorpos = -1;
    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_TOK + 1);
    }
    res = 0;
    start = SPOS(&jctx->bs);
    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs,
                                        (void *)(intptr_t)jctx->fd) != 0) {
                res = JPARSE_EXPECT_TOK + 1;
                break;
            }
        }

        ch = SPCHR(&jctx->bs);
        if (!JSN_ISALPHA(ch)) {
            break;
        }
        SINCR(&jctx->bs);
    }

    stop = SPOS(&jctx->bs);
    assert(stop >= start);
    sz = stop - start;
    *val = bytes_new_mpool(&jctx->mpool, sz + 1);
    memcpy((*val)->data, SDATA(&jctx->bs, start), sz);
    (*val)->data[sz] = '\0';
    //TRACE("tok=%s", (*val)->data);
    return res;
}


static int
expect_maybe_null(jparse_ctx_t *jctx)
{
    mnbytes_t *v;
    off_t spos;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (jparse_expect_tok(jctx, &v) == 0) {
        if (bytes_cmp(v, &_null) == 0) {
            return 0;
        }
    }
    if (jctx->errorpos == -1) {
        jctx->errorpos = SPOS(&jctx->bs);
    }
    SPOS(&jctx->bs) = spos;
    return -2;
}


int
jparse_expect_int(jparse_ctx_t *jctx, long *val, UNUSED void *udata)
{
    off_t spos;
    int st, res, sign;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_INT + 1);
    }

    st = JPS_NUMIN;
    res = 0;
    *val = 0;
    sign = 1;

    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs,
                                        (void *)(intptr_t)jctx->fd) != 0) {
                res = JPARSE_EXPECT_INT + 1;
                break;
            }
        }

        ch = SPCHR(&jctx->bs);
        if (st == JPS_NUMIN) {
            if (JSN_ISDIGIT(ch)) {
                *val = *val * 10 + (ch - '0');
                st = JPS_NUM;
            } else if (ch == '-') {
                sign = -1;
                st = JPS_NUM;
            } else if (ch == '+') {
                st = JPS_NUM;
            } else {
                res = JPARSE_EXPECT_INT + 1;
                if (jctx->errorpos == -1) {
                    jctx->errorpos = SPOS(&jctx->bs);
                }
                SPOS(&jctx->bs) = spos;
                break;
            }

        } else if (st == JPS_NUM) {
            if (JSN_ISDIGIT(ch)) {
                *val = *val * 10 + (ch - '0');
            } else {
                st = JPS_NUMOUT;
                break;
            }
        } else {
            res = JPARSE_EXPECT_INT + 2;
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            break;
        }
        SINCR(&jctx->bs);
    }
    *val *= sign;
    return res;
}


#define JPS_FLOATIN 0
#define JPS_FLOAT1 1
#define JPS_FLOATDOT 2
#define JPS_FLOAT2 3
#define JPS_FLOATE 4
#define JPS_FLOATE1 5
#define JPS_FLOATOUT 6
int
jparse_expect_float(jparse_ctx_t *jctx, double *val, UNUSED void *udata)
{
    int st, res;
    off_t start, spos;
    UNUSED off_t stop;
    char *endptr;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_FLOAT + 1);
    }

    st = JPS_FLOATIN;
    res = 0;
    *val = .0;
    start = SPOS(&jctx->bs);
    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs,
                                        (void *)(intptr_t)jctx->fd) != 0) {
                res = JPARSE_EXPECT_FLOAT + 1;
                break;
            }
        }

        ch = SPCHR(&jctx->bs);
        if (st == JPS_FLOATIN) {
            if (JSN_ISDIGIT(ch) || ch == '-' || ch == '+') {
                st = JPS_FLOAT1;
            } else if (ch == '.') {
                st = JPS_FLOATDOT;
            } else {
                break;
            }

        } else if (st == JPS_FLOAT1) {
            if (JSN_ISDIGIT(ch)) {
            } else if (ch == '.') {
                st = JPS_FLOATDOT;
            } else if (ch == 'e' || ch == 'E') {
                st = JPS_FLOATE;
            } else {
                st = JPS_FLOATOUT;
                break;
            }

        } else if (st == JPS_FLOATDOT) {
            if (JSN_ISDIGIT(ch)) {
                st = JPS_FLOAT2;
            } else if (ch == 'e' || ch == 'E') {
                st = JPS_FLOATE;
            } else {
                st = JPS_FLOATOUT;
                break;
            }

        } else if (st == JPS_FLOAT2) {
            if (JSN_ISDIGIT(ch)) {
            } else if (ch == 'e' || ch == 'E') {
                st = JPS_FLOATE;
            } else {
                st = JPS_FLOATOUT;
                break;
            }

        } else if (st == JPS_FLOATE) {
            if (JSN_ISDIGIT(ch) || ch == '-' || ch == '+') {
                st = JPS_FLOATE1;
            } else {
                break;
            }

        } else if (st == JPS_FLOATE1) {
            if (JSN_ISDIGIT(ch) || ch == '-' || ch == '+') {
            } else {
                break;
            }

        } else {
            res = JPARSE_EXPECT_INT + 2;
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            break;
        }
        SINCR(&jctx->bs);
    }
    stop = SPOS(&jctx->bs);

    *val = strtod(SDATA(&jctx->bs, start), &endptr);

    //if (endptr > SDATA(&jctx->bs, stop)) {
    //    TRACE("%p/%p", endptr, SDATA(&jctx->bs, stop));
    //    D8(SDATA(&jctx->bs, start), stop - start);
    //    D8(SDATA(&jctx->bs, start),
    //             (endptr - SDATA(&jctx->bs, start)) - start);
    //    FAIL("jparse_expect_float");
    //}

    return res;
}


int
jparse_expect_str(jparse_ctx_t *jctx, mnbytes_t **val, UNUSED void *udata)
{
    int st, flags, res;
    off_t start, stop, spos;
    size_t sz;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_dquote(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_STR + 1);
    }

    st = JPS_STRIN;
    flags = 0;
    res = 0;
    start = SPOS(&jctx->bs);

    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs,
                                        (void *)(intptr_t)jctx->fd) != 0) {
                res = JPARSE_EXPECT_STR + 2;
                break;
            }
        }
        ch = SPCHR(&jctx->bs);
        if (st == JPS_STRIN) {
            if (ch == '\\') {
                st = JPS_STRESC;
                flags |= JPS_FNEEDUNESCAPE;
            } else if (ch == '"') {
                st = JPS_STROUT;
                break;
            } else {
                st = JPS_STR;
            }

        } else if (st == JPS_STRESC) {
            st = JPS_STR;

        } else if (st == JPS_STR) {
            if (ch == '\\') {
                st = JPS_STRESC;
                flags |= JPS_FNEEDUNESCAPE;
            } else if (ch == '"') {
                st = JPS_STROUT;
                break;
            } else {
                st = JPS_STR;
            }
        } else {
            res = JPARSE_EXPECT_STR + 3;
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            break;
        }
        SINCR(&jctx->bs);
    }
    stop = SPOS(&jctx->bs);
    assert(stop >= start);
    sz = stop - start;
    *val = bytes_new_mpool(&jctx->mpool, sz + 1);
    memcpy((*val)->data, SDATA(&jctx->bs, start), sz);
    (*val)->data[sz] = '\0';
    if (flags & JPS_FNEEDUNESCAPE) {
        bytes_json_unescape(*val);
    }
    SINCR(&jctx->bs); // closing "
    return res;
}


int
jparse_expect_bool(jparse_ctx_t *jctx, char *val, UNUSED void *udata)
{
    off_t spos;
    mnbytes_t *v;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (jparse_expect_tok(jctx, &v) == 0) {
        if (bytes_cmp(v, &_true) == 0) {
            *val = 1;
            return 0;
        } else if (bytes_cmp(v, &_false) == 0) {
            *val = 0;
            return 0;
        } else {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(JPARSE_EXPECT_BOOL + 1);
        }
    }
    if (jctx->errorpos == -1) {
        jctx->errorpos = SPOS(&jctx->bs);
    }
    SPOS(&jctx->bs) = spos;
    TRRET(JPARSE_EXPECT_BOOL + 2);
}


/*
 * object
 */
int
jparse_expect_kvp_any(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      jparse_value_t *jval,
                      void *udata)
{
    int res;
    mnbytes_t *v;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ANY + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ANY + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ANY + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_any(jctx, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_any(jparse_ctx_t *jctx,
                       const char *key,
                       jparse_value_t *jval,
                       void *udata)
{
    int res;
    mnbytes_t *k;

    jctx->errorpos = -1;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_any(jctx, k, jval, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_ignore(jparse_ctx_t *jctx,
                            mnbytes_t **key,
                            jparse_value_t *jval,
                            void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_IGNORE + 1);
    }
    jval->k = *key;
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_IGNORE + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_ignore(jctx, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_anykvp_any(jparse_ctx_t *jctx,
                         mnbytes_t **key,
                         jparse_value_t *jval,
                         void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_ANY + 1);
    }
    jval->k = *key;
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_ANY + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_any(jctx, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_int(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      long *val,
                      void *udata)
{
    int res;

    jctx->errorpos = -1;
    mnbytes_t *v;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_INT + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_INT + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_INT + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_int(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_int(jparse_ctx_t *jctx,
                       const char *key,
                       long *val,
                       void *udata)
{
    int res;

    jctx->errorpos = -1;
    mnbytes_t *k;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_int(jctx, k, val, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_int(jparse_ctx_t *jctx,
                         mnbytes_t **key,
                         long *val,
                         void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_INT + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_INT + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_int(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_float(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      double *val,
                      void *udata)
{
    int res;

    jctx->errorpos = -1;
    mnbytes_t *v;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_FLOAT + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_FLOAT + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_FLOAT + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_float(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_float(jparse_ctx_t *jctx,
                         const char *key,
                         double *val,
                         void *udata)
{
    int res;

    jctx->errorpos = -1;
    mnbytes_t *k;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_float(jctx, k, val, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_float(jparse_ctx_t *jctx,
                           mnbytes_t **key,
                           double *val,
                           void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_FLOAT + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_FLOAT + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_float(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_str(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      mnbytes_t **val,
                      void *udata)
{
    int res;
    mnbytes_t *v;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_STR + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_STR + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_STR + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_str(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_str(jparse_ctx_t *jctx,
                       const char *key,
                       mnbytes_t **val,
                       void *udata)
{
    int res;
    mnbytes_t *k;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_str(jctx, k, val, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_str(jparse_ctx_t *jctx,
                         mnbytes_t **key,
                         mnbytes_t **val,
                         void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_STR + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_STR + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_str(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_bool(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      char *val,
                      void *udata)
{
    int res;
    mnbytes_t *v;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_BOOL + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_BOOL + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_BOOL + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_bool(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_bool(jparse_ctx_t *jctx,
                        const char *key,
                        char *val,
                        void *udata)
{
    int res;
    mnbytes_t *k;

    jctx->errorpos = -1;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_bool(jctx, k, val, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_bool(jparse_ctx_t *jctx,
                          mnbytes_t **key,
                          char *val,
                          void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_BOOL + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_BOOL + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_bool(jctx, val, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_object(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      jparse_expect_cb_t cb,
                      jparse_value_t *jval,
                      void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &jval->k, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_OBJECT + 1);
    }
    if (bytes_cmp(jval->k, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_OBJECT + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_OBJECT + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_object(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_object_iter(jparse_ctx_t *jctx,
                      const mnbytes_t *key,
                      jparse_expect_cb_t cb,
                      jparse_value_t *jval,
                      void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &jval->k, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_OBJECT_ITER + 1);
    }
    if (bytes_cmp(jval->k, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_OBJECT_ITER + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_OBJECT_ITER + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_object_iter(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_object(jparse_ctx_t *jctx,
                          const char *key,
                          jparse_expect_cb_t cb,
                          jparse_value_t *jval,
                          void *udata)
{
    int res;
    mnbytes_t *k;

    jctx->errorpos = -1;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_object(jctx, k, cb, jval, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_object(jparse_ctx_t *jctx,
                            mnbytes_t **key,
                            jparse_expect_cb_t cb,
                            jparse_value_t *jval,
                            void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_OBJECT + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_OBJECT + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        jval->k = *key;
        if ((res = jparse_expect_object(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_anykvp_object_iter(jparse_ctx_t *jctx,
                            mnbytes_t **key,
                            jparse_expect_cb_t cb,
                            jparse_value_t *jval,
                            void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_OBJECT_ITER + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_OBJECT_ITER + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        jval->k = *key;
        if ((res = jparse_expect_object_iter(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_array(jparse_ctx_t *jctx,
                        const mnbytes_t *key,
                        jparse_expect_cb_t cb,
                        jparse_value_t *jval,
                        void *udata)
{
    int res;
    mnbytes_t *v;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ARRAY + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ARRAY + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ARRAY + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_array(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_kvp_array_iter(jparse_ctx_t *jctx,
                        const mnbytes_t *key,
                        jparse_expect_cb_t cb,
                        jparse_value_t *jval,
                        void *udata)
{
    int res;
    mnbytes_t *v;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, &v, udata) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ARRAY_ITER + 1);
    }
    if (bytes_cmp(v, key) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ARRAY_ITER + 2);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_KVP_ARRAY_ITER + 3);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_array_iter(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_skvp_array(jparse_ctx_t *jctx,
                         const char *key,
                         jparse_expect_cb_t cb,
                         jparse_value_t *jval,
                         void *udata)
{
    int res;
    mnbytes_t *k;

    jctx->errorpos = -1;
    //k = bytes_new_from_str(key);
    k = bytes_new_from_str_mpool(&jctx->mpool, key);
    res = jparse_expect_kvp_array(jctx, k, cb, jval, udata);
    //BYTES_DECREF(&k);
    return res;
}


int
jparse_expect_anykvp_array(jparse_ctx_t *jctx,
                           mnbytes_t **key,
                           jparse_expect_cb_t cb,
                           jparse_value_t *jval,
                           void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_ARRAY + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_ARRAY + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        jval->k = *key;
        if ((res = jparse_expect_array(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_anykvp_array_iter(jparse_ctx_t *jctx,
                           mnbytes_t **key,
                           jparse_expect_cb_t cb,
                           jparse_value_t *jval,
                           void *udata)
{
    int res;

    jctx->errorpos = -1;
    res = 0;
    if (jparse_expect_str(jctx, key, udata) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_ARRAY_ITER + 1);
    }
    if (reach_colon(jctx) != 0) {
        TRRET(JPARSE_EXPECT_ANYKVP_ARRAY_ITER + 2);
    }
    if (expect_maybe_null(jctx) == 0) {
    } else {
        jval->k = *key;
        if ((res = jparse_expect_array_iter(jctx, cb, jval, udata)) != 0) {
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_object(jparse_ctx_t *jctx,
                     jparse_expect_cb_t cb,
                     jparse_value_t *jval,
                     void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_ostart(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_OBJECT + 1);
    }
    if ((res = cb(jctx, jval, udata)) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        return res;
    }
    if (reach_ostop(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_OBJECT + 2);
    }
    return res;
}


int
jparse_expect_object_iter(jparse_ctx_t *jctx,
                          jparse_expect_cb_t cb,
                          jparse_value_t *jval,
                          void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_ostart(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_OBJECT_ITER + 1);
    }
    spos = SPOS(&jctx->bs);
    for (res = cb(jctx, jval, udata); res == 0; res = cb(jctx, jval, udata)) {
        ;
    }
    if (res == JPARSE_EOS || (res != 0 && spos == SPOS(&jctx->bs))) {
        res = 0;
        if (reach_ostop(jctx) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(JPARSE_EXPECT_OBJECT_ITER + 2);
        }
    } else {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        return res;
    }
    return res;
}


/*
 * array
 */
int
jparse_expect_array(jparse_ctx_t *jctx,
                    jparse_expect_cb_t cb,
                    jparse_value_t *jval,
                    void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_astart(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_ARRAY + 1);
    }
    if ((res = cb(jctx, jval, udata)) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        return res;
    }
    if (reach_astop(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_ARRAY + 2);
    }
    return res;
}


int
jparse_expect_array_iter(jparse_ctx_t *jctx,
                         jparse_expect_cb_t cb,
                         jparse_value_t *jval,
                         void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    spos = SPOS(&jctx->bs);
    if (reach_astart(jctx) != 0) {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_ARRAY_ITER + 1);
    }

    spos = SPOS(&jctx->bs);
    for (res = cb(jctx, jval, udata); res == 0; res = cb(jctx, jval, udata)) {
        ;
    }
    if (res == JPARSE_EOS || (res != 0 && spos == SPOS(&jctx->bs))) {
        res = 0;
        if (reach_astop(jctx) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(JPARSE_EXPECT_ARRAY_ITER + 2);
        }
    } else {
        if (jctx->errorpos == -1) {
            jctx->errorpos = SPOS(&jctx->bs);
        }
        SPOS(&jctx->bs) = spos;
        return res;
    }
    return res;
}


int
jparse_expect_item_ignore(jparse_ctx_t *jctx,
                          jparse_value_t *jval,
                          void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_ignore(jctx, jval, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_any(jparse_ctx_t *jctx, jparse_value_t *jval, void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_any(jctx, jval, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_int(jparse_ctx_t *jctx, long *val, void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_int(jctx, val, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_float(jparse_ctx_t *jctx, double *val, void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_float(jctx, val, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_str(jparse_ctx_t *jctx, mnbytes_t **val, void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_str(jctx, val, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_bool(jparse_ctx_t *jctx, char *val, void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_bool(jctx, val, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_object(jparse_ctx_t *jctx,
                          jparse_expect_cb_t cb,
                          jparse_value_t *jval,
                          void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_object(jctx, cb, jval, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_object_iter(jparse_ctx_t *jctx,
                               jparse_expect_cb_t cb,
                               jparse_value_t *jval,
                               void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_object_iter(jctx, cb, jval, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_array(jparse_ctx_t *jctx,
                         jparse_expect_cb_t cb,
                         jparse_value_t *jval,
                         void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_array(jctx, cb, jval, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


int
jparse_expect_item_array_iter(jparse_ctx_t *jctx,
                              jparse_expect_cb_t cb,
                              jparse_value_t *jval,
                              void *udata)
{
    off_t spos;
    int res;

    jctx->errorpos = -1;
    res = 0;
    spos = SPOS(&jctx->bs);
    if (expect_maybe_null(jctx) == 0) {
    } else {
        if ((res = jparse_expect_array_iter(jctx, cb, jval, udata)) != 0) {
            if (jctx->errorpos == -1) {
                jctx->errorpos = SPOS(&jctx->bs);
            }
            SPOS(&jctx->bs) = spos;
            TRRET(res);
        }
    }
    if (reach_comma(jctx) != 0) {
        res = JPARSE_EOS;
    }
    return res;
}


/*
 *
 */
DEF_JPARSE_ARRAY_ITERATOR(ai, jparse_expect_item_ignore)

DEF_JPARSE_OBJECT_ITERATOR(oi, jparse_expect_anykvp_ignore)

int
jparse_ignore_nonscalar_iterator(jparse_ctx_t *jctx,
                                 jparse_value_t *jval,
                                 UNUSED void *udata)
{
    int res;
    jparse_value_t _jval;

    jctx->errorpos = -1;
    res = 0;
    jparse_value_init(&_jval);
    _jval.cb = jparse_ignore_nonscalar_iterator;
    if (jval->ty == JSON_ARRAY) {
        //TRACE("ignoring: %s", jval->k->data);
        res = REF_JPARSE_ARRAY_ITERATOR(ai)(jctx, &_jval, NULL);
    } else if (jval->ty == JSON_OBJECT) {
        //TRACE("ignoring: %s", jval->k->data);
        res = REF_JPARSE_OBJECT_ITERATOR(oi)(jctx, &_jval, NULL);
    } else if (jval->ty == JSON_UNDEF) {
        //TRACE("ignoring: %s", jval->k->data);
    } else {
        jparse_dump_value(jval);
        FAIL("jparse_ignore_nonscalar_iterator");
    }
    return res;
}


/*
 * parse
 */
int
jparse_ctx_parse(jparse_ctx_t *jctx,
                 const char *fname,
                 jparse_expect_cb_t cb,
                 jparse_value_t *jval,
                 void *udata)
{
    if ((jctx->fd = open(fname, O_RDONLY)) <= 0) {
        TRRET(JPARSE_CTX_PARSE + 1);
    }
    jctx->bs.read_more = bytestream_read_more;
    jctx->udata = udata;
    return cb(jctx, jval, udata);
}


int
jparse_ctx_parse_fd(jparse_ctx_t *jctx,
                    int fd,
                    jparse_expect_cb_t cb,
                    jparse_value_t *jval,
                    void *udata)
{
    jctx->fd = fd;
    jctx->bs.read_more = bytestream_read_more;
    jctx->udata = udata;
    return cb(jctx, jval, udata);
}


static ssize_t
_jparse_ctx_parse_data_read_more(UNUSED mnbytestream_t *bs,
                                 UNUSED void *fd,
                                 UNUSED ssize_t sz)
{
    return 0;
}


int
jparse_ctx_parse_data(jparse_ctx_t *jctx,
                      const char *data,
                      size_t sz,
                      jparse_expect_cb_t cb,
                      jparse_value_t *jval,
                      void *udata)
{
    int res;

    bytestream_fini(&jctx->bs);
    jctx->bs.buf.data = (char *)data;
    jctx->bs.buf.sz = sz;
    jctx->bs.eod = sz;
    jctx->bs.read_more = _jparse_ctx_parse_data_read_more;
    jctx->udata = udata;
    res = cb(jctx, jval, udata);
    jctx->bs.buf.data = NULL;
    return res;
}


void
jparse_dump_value(jparse_value_t *jval)
{
    switch (jval->ty) {
    case JSON_INT:
        TRACE("%s/%s:%ld", jval->k ? jval->k->data : NULL, JSON_TYPE_STR(jval->ty), jval->v.i);
        break;

    case JSON_FLOAT:
        TRACE("%s/%s:%lf", jval->k ? jval->k->data : NULL, JSON_TYPE_STR(jval->ty), jval->v.f);
        break;

    case JSON_STRING:
        TRACE("%s/%s:%s", jval->k ? jval->k->data : NULL,
              JSON_TYPE_STR(jval->ty),
              jval->v.s != NULL ? (char *)jval->v.s->data : "<null>");
        break;

    case JSON_BOOLEAN:
        TRACE("%s/%s:%s", jval->k ? jval->k->data : NULL, JSON_TYPE_STR(jval->ty), jval->v.b ? "#t" : "#f");
        break;

    default:
        TRACE("%s/%s:<%p/%p>", jval->k ? jval->k->data : NULL, JSON_TYPE_STR(jval->ty), jval->cb, jval->udata);
        break;
    }
}


void
jparse_dump_current_pos(jparse_ctx_t *jctx, ssize_t sz)
{
    D16(SPDATA(&jctx->bs), MIN(sz, SEOD(&jctx->bs) - SPOS(&jctx->bs)));
}


void
jparse_dump_error_pos(jparse_ctx_t *jctx, ssize_t sz, int flags)
{
    if (jctx->errorpos >= 0) {
        TRACE("error at pos %ld:", jctx->errorpos);
        D16(SDATA(&jctx->bs, jctx->errorpos),
           MIN(sz, SEOD(&jctx->bs) - SPOS(&jctx->bs)));
    }
    if (flags & JPARSE_DEP_FCLEAR) {
        jctx->errorpos = -2;
    }
}


/*
 * context
 */
jparse_ctx_t *
jparse_ctx_new(size_t mpool_chunksz, size_t bytestream_chunksz)
{
    jparse_ctx_t *jctx;

    if ((jctx = malloc(sizeof(jparse_ctx_t))) == NULL) {
        FAIL("malloc");
    }
    (void)mpool_ctx_init(&jctx->mpool, mpool_chunksz);
    bytestream_init(&jctx->bs, bytestream_chunksz);
    jctx->udata = NULL;
    jctx->default_cb = NULL;
    jctx->errorpos = -1;
    jctx->fd = -1;

    return jctx;
}


void
jparse_ctx_complete(jparse_ctx_t *jctx)
{
    mpool_ctx_reset(&jctx->mpool);
    bytestream_rewind(&jctx->bs);
    jctx->udata = NULL;
    jctx->errorpos = -1;
    jctx->fd = -1;
}


void
jparse_ctx_destroy(jparse_ctx_t **pjctx)
{
    if (*pjctx != NULL) {
        if ((*pjctx)->fd >= 0) {
            // do not close stdin
            if ((*pjctx)->fd > 0) {
                close((*pjctx)->fd);
            }
            (*pjctx)->fd = -1;
        }
        bytestream_fini(&(*pjctx)->bs);
        (void)mpool_ctx_fini(&(*pjctx)->mpool);
        free(*pjctx);
        *pjctx = NULL;
    }
}


