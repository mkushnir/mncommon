#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <mrkcommon/bytes.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>

#include "diag.h"

#ifdef DO_MEMDEBUG
#include <mrkcommon/memdebug.h>
MEMDEBUG_DECLARE(bytes);

#define MEMDEBUG_INIT(self)                                    \
do {                                                           \
    (self)->mdtag = (uint64_t)memdebug_get_runtime_scope();    \
} while (0)                                                    \


#define MEMDEBUG_ENTER(self)                                   \
{                                                              \
    int mdtag;                                                 \
    mdtag = memdebug_set_runtime_scope((int)(self)->mdtag);    \


#define MEMDEBUG_LEAVE(self)                   \
    (void)memdebug_set_runtime_scope(mdtag);   \
}                                              \


#else
#define MEMDEBUG_INIT(self)
#define MEMDEBUG_ENTER(self)
#define MEMDEBUG_LEAVE(self)
#endif

char *
strrstr(const char *big, const char *little)
{
    char *res = NULL;
    size_t littlesz;

    littlesz = strlen(little);
    if (littlesz == 0) {
        return (char *)big + strlen(big);
    }
    while (1) {
        char *tmp;

        tmp = strstr(big, little);
        if (tmp == NULL) {
            break;
        }
        res = tmp;
        big += littlesz;
    }
    return res;
}


int
bytes_startswith(const bytes_t *big, const bytes_t *small)
{
    int res;

    res = 0;
    if (big->sz >= small->sz) {
        size_t i;

        for (i = 0; i < small->sz && small->data[i] != '\0'; ++i) {
            if (big->data[i] != small->data[i]) {
                goto end;
            }
        }
        res = 1;
    }

end:
    return res;
}


int
bytes_endswith(const bytes_t *big, const bytes_t *small)
{
    int res;

    res = 0;
    if (big->sz >= small->sz) {
        ssize_t i, j;

        for (i = (ssize_t)small->sz - 2, j = (ssize_t)big->sz - 2;
             i >= 0;
             --i, --j) {
            if (big->data[j] != small->data[i]) {
                goto end;
            }
        }
        res = 1;
    }

end:
    return res;
}


int
bytes_is_null_or_empty(const bytes_t *s)
{
    return s == NULL || (s->sz == 1 && *s->data == '\0') || s->sz == 0;
}


bytes_t *
bytes_json_escape(bytes_t *src)
{
    size_t i, j;
    bytes_t *dest;

    /* partial json string support */
    dest = bytes_new(src->sz * 2);
    for (i = 0, j = 0; i < src->sz; ++i, ++j) {
        unsigned char ch;

        ch = src->data[i];
        if (ch == '\\' || ch == '"' || ch == '/') {
            dest->data[j++] = '\\';
        //} else if (ch == '\a') {
        //    ch = 'a';
        //    dest->data[j++] = '\\';
        } else if (ch == '\b') {
            ch = 'b';
            dest->data[j++] = '\\';
        } else if (ch == '\f') {
            ch = 'f';
            dest->data[j++] = '\\';
        } else if (ch == '\n') {
            ch = 'n';
            dest->data[j++] = '\\';
        } else if (ch == '\r') {
            ch = 'r';
            dest->data[j++] = '\\';
        } else if (ch == '\t') {
            ch = 't';
            dest->data[j++] = '\\';
        //} else if (ch == '\v') {
        //    ch = 'v';
        //    dest->data[j++] = '\\';
        }
        dest->data[j] = ch;
    }
    dest->data[j - 1] = '\0';
    dest->sz = j;
    return dest;
}


void
bytes_json_unescape(bytes_t *src)
{
    size_t i, j;
    bytes_t *dest;

    /* partial json string support */
    dest = src;
    for (i = 0, j = 0; i < src->sz; ++i, ++j) {
        unsigned char ch;

        ch = src->data[i];
        if (ch == '\\') {
            ++i;
            if (i < src->sz) {
                ch = src->data[i];
                if (ch == 'b') {
                    ch = '\b';
                } else if (ch == 'f') {
                    ch = '\f';
                } else if (ch == 'n') {
                    ch = '\n';
                } else if (ch == 'r') {
                    ch = '\r';
                } else if (ch == 't') {
                    ch = '\t';
                } else {
                }
            } else {
                break;
            }
        }
        dest->data[j] = ch;
    }
    dest->data[j - 1] = '\0';
    dest->sz = j;
}


int
bytes_is_ascii(bytes_t *s)
{
    size_t i, sz;
    char mod;

    mod = s->sz % sizeof(uint64_t);
    sz = s->sz - mod;

    for (i = 0; i < sz; i += sizeof(uint64_t)) {
        uint64_t *n;

        n = (uint64_t *)(s->data + i);
        if (*n & 0x8080808080808080) {
            return 0;
        }
    }

    while (--mod >= 0) {
        if (s->data[i + mod] & 0x80) {
            return 0;
        }
    }
    return 1;
}


bytes_t *
bytes_set_lower(bytes_t *s)
{
    ssize_t sz;

    s->hash = 0;
    sz = s->sz;
    while (--sz >= 0) {
        s->data[sz] = (unsigned char)tolower((int)s->data[sz]);
    }
    return s;
}

uint64_t
bytes_hash(bytes_t *bytes)
{
    if (bytes->hash == 0) {
        bytes->hash = fasthash(0, bytes->data, bytes->sz);
    }
    return bytes->hash;
}


int
bytes_cmp(bytes_t *a, bytes_t *b)
{
    uint64_t ha, hb;
    int64_t diff;

    ha = bytes_hash(a);
    hb = bytes_hash(b);
    diff = (int64_t)(ha - hb);
    if (diff == 0) {
        diff =  (a->sz - b->sz);
        if (diff == 0) {
            return memcmp(a->data, b->data, a->sz);
        }
    }
    return diff > 0 ? 1 : -1;
}


int
bytes_cmpi(bytes_t *a, bytes_t *b)
{
    return strncasecmp((char *)a->data,
                       (char *)b->data,
                       MIN(a->sz, b->sz));
}


int
bytes_contains(bytes_t *a, bytes_t *b)
{
    if (a->data[a->sz - 1] != '\0' || b->data[b->sz - 1] != '\0') {
        return 0;
    }
    return strstr((char *)a->data, (char *)b->data) != NULL;
}


int
bytes_containsi(bytes_t *a, bytes_t *b)
{
    if (a->data[a->sz - 1] != '\0' || b->data[b->sz - 1] != '\0') {
        return 0;
    }
    return strcasestr((char *)a->data, (char *)b->data) != NULL;
}


#define BYTES_NEW_BODY(malloc_fn)                              \
    size_t mod, msz;                                           \
    bytes_t *res;                                              \
    assert(sz > 0);                                            \
    msz = sz;                                                  \
    mod = sz % 8;                                              \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(bytes_t) + msz)) == NULL) {    \
        FAIL("malloc");                                        \
    }                                                          \
    MEMDEBUG_INIT(res);                                        \
    res->nref = 0;                                             \
    res->sz = sz;                                              \
    res->hash = 0;                                             \
    return res

#define BYTES_NEW_FROM_STR_BODY(malloc_fn)                     \
    bytes_t *res;                                              \
    size_t mod, msz;                                           \
    size_t sz;                                                 \
    sz = strlen(s) + 1;                                        \
    msz = sz;                                                  \
    mod = sz % 8;                                              \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(bytes_t) + msz)) == NULL) {    \
        FAIL("malloc");                                        \
    }                                                          \
    MEMDEBUG_INIT(res);                                        \
    memcpy(res->data, s, sz);                                  \
    res->nref = 0;                                             \
    res->sz = sz;                                              \
    res->hash = 0;                                             \
    return res


bytes_t *
bytes_new(size_t sz)
{
    BYTES_NEW_BODY(malloc);
}


bytes_t *
bytes_new_from_str(const char *s)
{
    BYTES_NEW_FROM_STR_BODY(malloc);
}


#define BYTES_NEW_FROM_BYTES_BODY(malloc_fn)                   \
    bytes_t *res;                                              \
    size_t mod, msz;                                           \
    assert(s->sz > 0);                                         \
    msz = s->sz;                                               \
    mod = s->sz % 8;                                           \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(bytes_t) + msz)) == NULL) {    \
        FAIL("malloc");                                        \
    }                                                          \
    MEMDEBUG_INIT(res);                                        \
    memcpy(res->data, s->data, s->sz);                         \
    res->nref = 0;                                             \
    res->sz = s->sz;                                           \
    res->hash = 0;                                             \
    return res


bytes_t *
bytes_new_from_bytes(const bytes_t *s)
{
    BYTES_NEW_FROM_BYTES_BODY(malloc);
}


#define _malloc(sz) mpool_malloc(mpool, (sz))
bytes_t *
bytes_new_mpool(mpool_ctx_t *mpool, size_t sz)
{
    BYTES_NEW_BODY(_malloc);
}
bytes_t *
bytes_new_from_str_mpool(mpool_ctx_t *mpool, const char *s)
{
    BYTES_NEW_FROM_STR_BODY(_malloc);
}
bytes_t *
bytes_new_from_bytes_mpool(mpool_ctx_t *mpool, const bytes_t *s)
{
    BYTES_NEW_FROM_BYTES_BODY(_malloc);
}
#undef _malloc


bytes_t * PRINTFLIKE(1, 2)
bytes_printf(const char *fmt, ...)
{
    int nused;
    char x;
    bytes_t *res;
    va_list ap0, ap1;

    va_start(ap0, fmt);
    va_copy(ap1, ap0);
    nused = vsnprintf(&x, 0, fmt, ap0);
    va_end(ap0);

    res = bytes_new(nused + 1);

    nused = vsnprintf((char *)res->data, res->sz, fmt, ap1);
    va_end(ap1);
    assert((size_t)nused < res->sz);

    return res;
}

void
bytes_copy(bytes_t *dst, bytes_t *src, size_t off)
{
    assert((off + src->sz) <= dst->sz);
    memcpy(dst->data + off, src->data, src->sz);
}


void
bytes_copyz(bytes_t *dst, bytes_t *src, size_t off)
{
    assert((off + src->sz - 1) <= dst->sz);
    memcpy(dst->data + off, src->data, src->sz - 1);
    *(dst->data + off + src->sz - 1) = '\0';
}


void
bytes_urldecode(bytes_t *str)
{
    unsigned char *src, *dst;
    unsigned char *end;

    end = str->data + str->sz;
    for (src = str->data, dst = str->data;
         src < end;
         ++src, ++dst) {
        if (*src == '%' && (src + 2) < end) {
            ++src;
            if (*src >= '0' && *src <= '9') {
                *dst = (*src - '0') << 4;
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'A' && *src <= 'F') {
                *dst = (*src - '7') << 4;
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'a' && *src <= 'f') {
                *dst = (*src - 'W') << 4;
                //TRACE("*dst='%02hhd'", *dst);
            } else {
                /* ignore invalid sequence */
            }
            ++src;
            if (*src >= '0' && *src <= '9') {
                *dst |= (*src - '0');
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'A' && *src <= 'F') {
                *dst |= (*src - '7');
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'a' && *src <= 'f') {
                *dst |= (*src - 'W');
                //TRACE("*dst='%02hhd'", *dst);
            } else {
                /* ignore invalid sequence */
            }
        } else if (*src == '+') {
            *dst = ' ';
        } else {
            *dst = *src;
        }
    }
    *(dst - 1) = '\0';
    str->sz = (intptr_t)(dst - str->data);
    str->hash = 0;
}


/*
 * URL decode + remove spaces
 */
void
bytes_brushdown(bytes_t *str)
{
    unsigned char *src, *dst;
    unsigned char *end;

    /*
     * expect alpha-numeric words optionally surrounded by spaces
     */
    end = str->data + str->sz;
    for (src = str->data, dst = str->data;
         src < end;
         ++src, ++dst) {
        if (*src == '%' && (src + 2) < end) {
            ++src;
            if (*src >= '0' && *src <= '9') {
                *dst = (*src - '0') << 4;
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'A' && *src <= 'F') {
                *dst = (*src - '7') << 4;
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'a' && *src <= 'f') {
                *dst = (*src - 'W') << 4;
                //TRACE("*dst='%02hhd'", *dst);
            } else {
                /* ignore invalid sequence */
            }
            ++src;
            if (*src >= '0' && *src <= '9') {
                *dst |= (*src - '0');
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'A' && *src <= 'F') {
                *dst |= (*src - '7');
                //TRACE("*dst='%02hhd'", *dst);
            } else if (*src >= 'a' && *src <= 'f') {
                *dst |= (*src - 'W');
                //TRACE("*dst='%02hhd'", *dst);
            } else {
                /* ignore invalid sequence */
            }
        } else {
            *dst = *src;
        }
        if (*dst == ' ') {
            /* discard SPACE */
            --dst;
        }
    }
    *(dst - 1) = '\0';
    str->sz = (intptr_t)(dst - str->data);
    str->hash = 0;
}


int
bytes_split_iter(bytes_t *str, char *delim, bytes_split_cb cb, void *udata)
{
    int res = 0;
    bytes_t *tmp = NULL;

    if (*delim != '\0') {
        char *p0, *p1;
        size_t sz;
        size_t delimsz;

        delimsz = strlen(delim);
        tmp = bytes_new(str->sz);
        for (p0 = (char *)str->data, p1 = strstr(p0, delim);
             p1 != NULL;
             p0 = p1 + delimsz, p1 = strstr(p0, delim)) {

            sz = p1 - p0;
            memcpy(tmp->data, p0, sz);
            *(tmp->data + sz) = '\0';
            tmp->sz = sz + 1;
            if ((res = cb(tmp, udata)) != 0) {
                goto end;
            }
        }
        /* last */
        p1 = (char *)str->data + str->sz - 1;
        sz = p1 - p0;
        memcpy(tmp->data, p0, sz);
        *(tmp->data + sz) = '\0';
        tmp->sz = sz + 1;
        res = cb(tmp, udata);
    }

end:
    bytes_decref(&tmp);
    return res;
}


void
bytes_decref(bytes_t **value)
{
    BYTES_DECREF(value);
}


void
bytes_decref_fast(bytes_t *value)
{
    BYTES_DECREF_FAST(value);
}


void
bytes_incref(bytes_t *value)
{
    BYTES_INCREF(value);
}
