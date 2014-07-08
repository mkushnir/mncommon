#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <mrkcommon/bytes.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>

#include "diag.h"

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
        if (ch == '\\' || ch == '"') {
            dest->data[j++] = '\\';
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
        }
        dest->data[j] = ch;
    }
    dest->data[j - 1] = '\0';
    dest->sz = j;
    return dest;
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


#define BYTES_NEW_BODY(malloc_fn) \
    size_t mod, msz; \
    bytes_t *res; \
    assert(sz > 0); \
    msz = sz; \
    mod = sz % 8; \
    if (mod) { \
        msz += (8 - mod); \
    } else { \
        msz += 8; \
    } \
    if ((res = malloc_fn(sizeof(bytes_t) + msz)) == NULL) { \
        FAIL("malloc"); \
    } \
    res->nref = 0; \
    res->sz = sz; \
    res->hash = 0; \
    return res

#define BYTES_NEW_FROM_STR_BODY(malloc_fn) \
    bytes_t *res; \
    size_t mod, msz; \
    size_t sz; \
    sz = strlen(s) + 1; \
    msz = sz; \
    mod = sz % 8; \
    if (mod) { \
        msz += (8 - mod); \
    } else { \
        msz += 8; \
    } \
    if ((res = malloc_fn(sizeof(bytes_t) + msz)) == NULL) { \
        FAIL("malloc"); \
    } \
    memcpy(res->data, s, sz); \
    res->nref = 0; \
    res->sz = sz; \
    res->hash = 0; \
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


#define BYTES_NEW_FROM_BYTES_BODY(malloc_fn) \
    bytes_t *res; \
    size_t mod, msz; \
    msz = s->sz; \
    mod = s->sz % 8; \
    if (mod) { \
        msz += (8 - mod); \
    } else { \
        msz += 8; \
    } \
    if ((res = malloc_fn(sizeof(bytes_t) + msz)) == NULL) { \
        FAIL("malloc"); \
    } \
    memcpy(res->data, s->data, s->sz); \
    res->nref = 0; \
    res->sz = s->sz; \
    res->hash = 0; \
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


void
bytes_copy(bytes_t *dst, bytes_t *src, size_t off)
{
    assert((off + src->sz) <= dst->sz);
    memcpy(dst->data + off, src->data, src->sz);
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
