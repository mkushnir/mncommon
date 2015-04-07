#include <assert.h>
#include <fcntl.h>

#include <mrkcommon/jparse.h>
//#define TRRET_DEBUG
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include "diag.h"

/*
 * scanner
 */
#define JSN_ISDIGIT(c) (((c) >= '0' && (c) <= '9'))
#define JSN_ISALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define JSN_ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

static int
reach_nonblank(jparse_ctx_t *jctx)
{
    int res = 0;
    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
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
        if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
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
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
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
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {           \
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
            /* TRACE("failing %c at %ld", ch, SPOS(&jctx->bs)); */             \
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
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {           \
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
reach_oend(jparse_ctx_t *jctx)
{
    REACH_BODY('}', REACH_OEND)
}


UNUSED static int
probe_oend(jparse_ctx_t *jctx)
{
    PROBE_BODY('}', PROBE_OEND)
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
reach_aend(jparse_ctx_t *jctx)
{
    REACH_BODY(']', REACH_AEND)
}


UNUSED static int
probe_aend(jparse_ctx_t *jctx)
{
    PROBE_BODY(']', PROBE_AEND)
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
 * scalar
 */


DEF_JPARSE_OBJECT_ITERATOR(_ignore, jparse_expect_anykvp_ignore);


DEF_JPARSE_ARRAY_ITERATOR(_ignore, jparse_expect_item_ignore);


int
jparse_expect_ignore(jparse_ctx_t *jctx, jparse_value_t *val)
{
    int res;
    off_t start;

    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_TOK + 1);
    }

    start = SPOS(&jctx->bs);
    if ((res = jparse_expect_maybe_null(jctx)) == 0) {
        val->ty = JSON_NULL;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if ((res = jparse_expect_bool(jctx, &val->v.b)) == 0) {
        val->ty = JSON_BOOLEAN;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if (probe_dquote(jctx) == 0) {
        if ((res = jparse_expect_str(jctx, &val->v.s)) == 0) {
            val->ty = JSON_STRING;
            goto end;
        }
    } else if (probe_ostart(jctx) == 0) {
        if ((res = jparse_expect_object(jctx,
                REF_JPARSE_OBJECT_ITERATOR(_ignore))) == 0) {
            val->ty = JSON_OBJECT;
            goto end;
        }
    } else if (probe_astart(jctx) == 0) {
        if ((res = jparse_expect_array(jctx,
                REF_JPARSE_ARRAY_ITERATOR(_ignore))) == 0) {
            val->ty = JSON_ARRAY;
            goto end;
        }
    } else {
        if ((res = jparse_expect_float(jctx, &val->v.f)) == 0) {
            val->ty = JSON_FLOAT;
            goto end;
        }
    }

    SPOS(&jctx->bs) = start;
    res = JPARSE_EXPECT_IGNORE + 1;

end:
    return res;
}


int
jparse_expect_any(jparse_ctx_t *jctx, jparse_value_t *val)
{
    int res;
    off_t start;

    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_TOK + 1);
    }

    start = SPOS(&jctx->bs);
    if ((res = jparse_expect_maybe_null(jctx)) == 0) {
        val->ty = JSON_NULL;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if ((res = jparse_expect_bool(jctx, &val->v.b)) == 0) {
        val->ty = JSON_BOOLEAN;
        goto end;
    }

    start = SPOS(&jctx->bs);
    if (probe_dquote(jctx) == 0) {
        if ((res = jparse_expect_str(jctx, &val->v.s)) == 0) {
            val->ty = JSON_STRING;
            goto end;
        }
    } else if (probe_ostart(jctx) == 0) {
        if ((res = jparse_expect_object(jctx, val->cb)) == 0) {
            val->ty = JSON_OBJECT;
            goto end;
        }
    } else if (probe_astart(jctx) == 0) {
        if ((res = jparse_expect_array(jctx, val->cb)) == 0) {
            val->ty = JSON_ARRAY;
            goto end;
        }
    } else {
        if ((res = jparse_expect_float(jctx, &val->v.f)) == 0) {
            val->ty = JSON_FLOAT;
            goto end;
        }
    }

    SPOS(&jctx->bs) = start;
    res = JPARSE_EXPECT_ANY + 1;

end:
    return res;
}


int
jparse_expect_tok(jparse_ctx_t *jctx, bytes_t **val)
{
    int res;
    off_t start, stop;
    size_t sz;

    if (reach_nonblank(jctx) != 0) {
        TRRET(JPARSE_EXPECT_TOK + 1);
    }
    res = 0;
    start = SPOS(&jctx->bs);
    while (1) {
        char ch;

        if (SNEEDMORE(&jctx->bs)) {
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
                res = JPARSE_EXPECT_FLOAT + 1;
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


int
jparse_expect_maybe_null(jparse_ctx_t *jctx)
{
    bytes_t *v;
    off_t spos;

    spos = SPOS(&jctx->bs);
    if (jparse_expect_tok(jctx, &v) == 0) {
        if (bytes_cmp(v, jctx->_null) == 0) {
            return 0;
        }
    }
    SPOS(&jctx->bs) = spos;
    return -2;
}


int
jparse_expect_int(jparse_ctx_t *jctx, long *val)
{
    off_t spos;
    int st, res, sign;

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
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
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
jparse_expect_float(jparse_ctx_t *jctx, double *val)
{
    int st, res;
    off_t start, spos;
    UNUSED off_t stop;
    char *endptr;

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
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
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
jparse_expect_str(jparse_ctx_t *jctx, bytes_t **val)
{
    int st, flags, res;
    off_t start, stop, spos;
    size_t sz;

    spos = SPOS(&jctx->bs);
    if (reach_dquote(jctx) != 0) {
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
            if (bytestream_consume_data(&jctx->bs, jctx->fd) != 0) {
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
jparse_expect_bool(jparse_ctx_t *jctx, char *val)
{
    off_t spos;
    bytes_t *v;

    spos = SPOS(&jctx->bs);
    if (jparse_expect_tok(jctx, &v) == 0) {
        if (bytes_cmp(v, jctx->_true) == 0) {
            *val = 1;
            return 0;
        } else if (bytes_cmp(v, jctx->_false) == 0) {
            *val = 0;
            return 0;
        } else {
            SPOS(&jctx->bs) = spos;
            TRRET(JPARSE_EXPECT_BOOL + 1);
        }
    }
    SPOS(&jctx->bs) = spos;
    TRRET(JPARSE_EXPECT_BOOL + 2);
}


/*
 * object
 */
#define EXPECT_ANYKVP_BODY(expect_fn, key, val, __a1, msg)     \
    int res;                                                   \
    res = 0;                                                   \
    if (jparse_expect_str(jctx, key) != 0) {                   \
        TRRET(msg + 1);                                        \
    }                                                          \
    if (reach_colon(jctx) != 0) {                              \
        TRRET(msg + 3);                                        \
    }                                                          \
    if (jparse_expect_maybe_null(jctx) == 0) {                 \
        __a1;                                                  \
    } else {                                                   \
        if ((res = expect_fn(jctx, val)) != 0) {               \
            TRRET(res);                                        \
        }                                                      \
    }                                                          \
    if (reach_comma(jctx) != 0) {                              \
        res = JPARSE_EOS;                                      \
    }                                                          \
    return res;                                                \


#define EXPECT_KVP_BODY(expect_fn, __a1, msg)          \
    int res;                                           \
    bytes_t *v;                                        \
    res = 0;                                           \
    if (jparse_expect_str(jctx, &v) != 0) {            \
        TRRET(msg + 1);                                \
    }                                                  \
    if (bytes_cmp(v, *key) != 0) {                     \
        TRRET(msg + 2);                                \
    }                                                  \
    if (reach_colon(jctx) != 0) {                      \
        TRRET(msg + 3);                                \
    }                                                  \
    if (jparse_expect_maybe_null(jctx) == 0) {         \
        __a1;                                          \
    } else {                                           \
        if ((res = expect_fn(jctx, val)) != 0) {       \
            TRRET(res);                                \
        }                                              \
    }                                                  \
    if (reach_comma(jctx) != 0) {                      \
        res = JPARSE_EOS;                              \
    }                                                  \
    return res;                                        \


#define EXPECT_SKVP_BODY(expect_fn)    \
    int res;                           \
    bytes_t *k;                        \
    k = bytes_new_from_str(key);       \
    res = expect_fn(jctx, &k, val);    \
    bytes_decref(&k);                  \
    return res;                        \


int
jparse_expect_kvp_any(jparse_ctx_t *jctx,
                      bytes_t **key,
                      jparse_value_t *val)
{
    EXPECT_KVP_BODY(jparse_expect_any,
                    ,
                    JPARSE_EXPECT_KVP_ANY)
}


int
jparse_expect_skvp_any(jparse_ctx_t *jctx,
                       const char *key,
                       jparse_value_t *val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_any)
}


int
jparse_expect_anykvp_ignore(jparse_ctx_t *jctx,
                            bytes_t **key,
                            jparse_value_t *val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_ignore,
                       key, val,
                       /* *val = 0 */,
                       JPARSE_EXPECT_KVP_IGNORE)
}


int
jparse_expect_anykvp_any(jparse_ctx_t *jctx,
                         bytes_t **key,
                         jparse_value_t *val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_any,
                       key,
                       val,
                       /* *val = 0 */,
                       JPARSE_EXPECT_KVP_ANY)
}


int
jparse_expect_kvp_int(jparse_ctx_t *jctx,
                      bytes_t **key,
                      long *val)
{
    EXPECT_KVP_BODY(jparse_expect_int,
                    /* *val = 0 */,
                    JPARSE_EXPECT_KVP_INT)
}


int
jparse_expect_skvp_int(jparse_ctx_t *jctx,
                       const char *key,
                       long *val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_int)
}


int
jparse_expect_anykvp_int(jparse_ctx_t *jctx,
                         bytes_t **key,
                         long *val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_int,
                       key,
                       val,
                       /* *val = 0 */,
                       JPARSE_EXPECT_KVP_INT)
}


int
jparse_expect_kvp_float(jparse_ctx_t *jctx,
                      bytes_t **key,
                      double *val)
{
    EXPECT_KVP_BODY(jparse_expect_float,
                    /* *val = .0 */,
                    JPARSE_EXPECT_KVP_FLOAT)
}


int
jparse_expect_skvp_float(jparse_ctx_t *jctx,
                         const char *key,
                         double *val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_float)
}


int
jparse_expect_anykvp_float(jparse_ctx_t *jctx,
                           bytes_t **key,
                           double *val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_float,
                       key,
                       val,
                       /* *val = 0 */,
                       JPARSE_EXPECT_KVP_FLOAT)
}


int
jparse_expect_kvp_str(jparse_ctx_t *jctx,
                      bytes_t **key,
                      bytes_t **val)
{
    EXPECT_KVP_BODY(jparse_expect_str,
                    /* *val = NULL */,
                    JPARSE_EXPECT_KVP_STR)
}


int
jparse_expect_skvp_str(jparse_ctx_t *jctx,
                       const char *key,
                       bytes_t **val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_str)
}


int
jparse_expect_anykvp_str(jparse_ctx_t *jctx,
                         bytes_t **key,
                         bytes_t **val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_str,
                       key,
                       val,
                       /* *val = 0 */,
                       JPARSE_EXPECT_KVP_STR)
}


int
jparse_expect_kvp_bool(jparse_ctx_t *jctx,
                      bytes_t **key,
                      char *val)
{
    EXPECT_KVP_BODY(jparse_expect_bool,
                    /* *val = 0 */,
                    JPARSE_EXPECT_KVP_BOOL)
}


int
jparse_expect_skvp_bool(jparse_ctx_t *jctx,
                        const char *key,
                        char *val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_bool)
}


int
jparse_expect_anykvp_bool(jparse_ctx_t *jctx,
                          bytes_t **key,
                          char *val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_bool,
                       key,
                       val,
                       /* *val = 0 */,
                       JPARSE_EXPECT_KVP_BOOL)
}


int
jparse_expect_kvp_object(jparse_ctx_t *jctx,
                      bytes_t **key,
                      jparse_expect_cb_t val)
{
    EXPECT_KVP_BODY(jparse_expect_object,
                    ,
                    JPARSE_EXPECT_KVP_OBJECT)
}


int
jparse_expect_skvp_object(jparse_ctx_t *jctx,
                          const char *key,
                          jparse_expect_cb_t val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_object)
}


int
jparse_expect_anykvp_object(jparse_ctx_t *jctx,
                            bytes_t **key,
                            jparse_expect_cb_t val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_object,
                       key,
                       val,
                       ,
                       JPARSE_EXPECT_KVP_OBJECT)
}


int
jparse_expect_kvp_array(jparse_ctx_t *jctx,
                      bytes_t **key,
                      jparse_expect_cb_t val)
{
    EXPECT_KVP_BODY(jparse_expect_array,
                    ,
                    JPARSE_EXPECT_KVP_ARRAY)
}


int
jparse_expect_skvp_array(jparse_ctx_t *jctx,
                         const char *key,
                         jparse_expect_cb_t val)
{
    EXPECT_SKVP_BODY(jparse_expect_kvp_array)
}


int
jparse_expect_anykvp_array(jparse_ctx_t *jctx,
                           bytes_t **key,
                           jparse_expect_cb_t val)
{
    EXPECT_ANYKVP_BODY(jparse_expect_array,
                       key,
                       val,
                       ,
                       JPARSE_EXPECT_KVP_ARRAY)
}


int
jparse_expect_object(jparse_ctx_t *jctx, jparse_expect_cb_t cb)
{
    off_t spos;
    int res;

    spos = SPOS(&jctx->bs);
    if (reach_ostart(jctx) != 0) {
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_OBJECT + 1);
    }
    if ((res = cb(jctx)) != 0) {
        SPOS(&jctx->bs) = spos;
        return res;
    }
    if (reach_oend(jctx) != 0) {
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_OBJECT + 2);
    }
    return res;
}


/*
 * array
 */
int
jparse_expect_array(jparse_ctx_t *jctx, jparse_expect_cb_t cb)
{
    off_t spos;
    int res;

    spos = SPOS(&jctx->bs);
    if (reach_astart(jctx) != 0) {
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_ARRAY + 1);
    }
    if ((res = cb(jctx)) != 0) {
        SPOS(&jctx->bs) = spos;
        return res;
    }
    if (reach_aend(jctx) != 0) {
        SPOS(&jctx->bs) = spos;
        TRRET(JPARSE_EXPECT_ARRAY + 2);
    }
    return res;
}


#define EXPECT_ITEM_BODY(expect_fn, __a1, msg)         \
    off_t spos;                                        \
    int res;                                           \
    res = 0;                                           \
    spos = SPOS(&jctx->bs);                            \
    if (jparse_expect_maybe_null(jctx) == 0) {         \
        __a1;                                          \
    } else {                                           \
        if ((res = expect_fn(jctx, val)) != 0) {       \
            SPOS(&jctx->bs) = spos;                    \
            TRRET(res);                                \
        }                                              \
    }                                                  \
    if (reach_comma(jctx) != 0) {                      \
        res = JPARSE_EOS;                              \
    }                                                  \
    return res;                                        \


int
jparse_expect_item_ignore(jparse_ctx_t *jctx, jparse_value_t *val)
{
    EXPECT_ITEM_BODY(jparse_expect_ignore,
                     /* *val = 0 */,
                     JPARSE_EXPECT_ITEM_IGNORE)
}


int
jparse_expect_item_any(jparse_ctx_t *jctx, jparse_value_t *val)
{
    EXPECT_ITEM_BODY(jparse_expect_any,
                     /* *val = 0 */,
                     JPARSE_EXPECT_ITEM_ANY)
}


int
jparse_expect_item_int(jparse_ctx_t *jctx, long *val)
{
    EXPECT_ITEM_BODY(jparse_expect_int,
                     /* *val = 0 */,
                     JPARSE_EXPECT_ITEM_INT)
}


int
jparse_expect_item_float(jparse_ctx_t *jctx, double *val)
{
    EXPECT_ITEM_BODY(jparse_expect_float,
                     /* *val = .0 */,
                     JPARSE_EXPECT_ITEM_FLOAT)
}


int
jparse_expect_item_str(jparse_ctx_t *jctx, bytes_t **val)
{
    EXPECT_ITEM_BODY(jparse_expect_str,
                     /* *val = NULL */,
                     JPARSE_EXPECT_ITEM_STR)
}


int
jparse_expect_item_bool(jparse_ctx_t *jctx, char *val)
{
    EXPECT_ITEM_BODY(jparse_expect_bool,
                     /* *val = 0 */,
                     JPARSE_EXPECT_ITEM_BOOL)
}


int
jparse_expect_item_object(jparse_ctx_t *jctx, jparse_expect_cb_t val)
{
    EXPECT_ITEM_BODY(jparse_expect_object,
                     ,
                     JPARSE_EXPECT_ITEM_OBJECT)
}


int
jparse_expect_item_array(jparse_ctx_t *jctx, jparse_expect_cb_t val)
{
    EXPECT_ITEM_BODY(jparse_expect_array,
                     ,
                     JPARSE_EXPECT_ITEM_ARRAY)
}


/*
 * parse
 */
int
jparse_ctx_parse(jparse_ctx_t *jctx,
                 const char *fname,
                 jparse_expect_cb_t cb,
                 void *udata)
{
    if ((jctx->fd = open(fname, O_RDONLY)) <= 0) {
        TRRET(JPARSE_CTX_PARSE + 1);
    }
    jctx->bs.read_more = bytestream_read_more;
    jctx->udata = udata;
    return cb(jctx);
}


void
jparse_dump_value(jparse_value_t *val)
{
    switch (val->ty) {
    case JSON_INT:
        TRACE("%s:%ld", JSON_TYPE_STR(val->ty), val->v.i);
        break;

    case JSON_FLOAT:
        TRACE("%s:%lf", JSON_TYPE_STR(val->ty), val->v.f);
        break;

    case JSON_STRING:
        TRACE("%s:%s", JSON_TYPE_STR(val->ty), val->v.s != NULL ? (char *)val->v.s->data : "<null>");
        break;

    case JSON_BOOLEAN:
        TRACE("%s:%s", JSON_TYPE_STR(val->ty), val->v.b ? "#t" : "#f");
        break;

    default:
        TRACE("%s:<%p>", JSON_TYPE_STR(val->ty), val);
        break;


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
    jctx->_null = bytes_new_from_str_mpool(&jctx->mpool, "null");
    jctx->_true = bytes_new_from_str_mpool(&jctx->mpool, "true");
    jctx->_false = bytes_new_from_str_mpool(&jctx->mpool, "false");
    bytestream_init(&jctx->bs, bytestream_chunksz);
    jctx->udata = NULL;
    jctx->default_cb = NULL;
    jctx->fd = -1;

    return jctx;
}


void
jparse_ctx_destroy(jparse_ctx_t **pjctx)
{
    if (*pjctx != NULL) {
        if ((*pjctx)->fd >= 0) {
            close((*pjctx)->fd);
            (*pjctx)->fd = -1;
        }
        bytestream_fini(&(*pjctx)->bs);
        (void)mpool_ctx_fini(&(*pjctx)->mpool);
        free(*pjctx);
        *pjctx = NULL;
    }
}


