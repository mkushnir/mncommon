#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <mncommon/malloc.h>
#include <mncommon/bytes.h>
#include <mncommon/dumpm.h>
#include <mncommon/fasthash.h>
#include <mncommon/mpool.h>
#include <mncommon/util.h>

#include "diag.h"

#define RFC3986_RESEVED 0x01
#define RFC3986_OTHERS  0x02
#define RFC3986_UNRESERVED  0x04

#define ISSPACE(c) ((c) == ' ' || (c) == '\t')

static char charflags[256] = {
/*  0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3 4 5 6 7 8 9 a b c d e f */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,1,2,1,1,2,1,1,1,1,1,1,1,4,4,1,4,4,4,4,4,4,4,4,4,4,1,1,2,1,2,1,
    1,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,2,1,2,4,
    2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,2,2,4,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
};


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


bool
bytes_startswith(const mnbytes_t *big, const mnbytes_t *small)
{
    bool res;

    res = false;
    if (big->sz >= small->sz) {
        size_t i;

        for (i = 0; i < small->sz && small->data[i] != '\0'; ++i) {
            if (big->data[i] != small->data[i]) {
                goto end;
            }
        }
        res = true;
    }

end:
    return res;
}


bool
bytes_endswith(const mnbytes_t *big, const mnbytes_t *small)
{
    int res;

    res = false;
    if (big->sz >= small->sz) {
        ssize_t i, j;

        for (i = (ssize_t)small->sz - 2, j = (ssize_t)big->sz - 2;
             i >= 0;
             --i, --j) {
            if (big->data[j] != small->data[i]) {
                goto end;
            }
        }
        res = true;
    }

end:
    return res;
}


bool
bytes_is_null_or_empty(const mnbytes_t *s)
{
    return s == NULL || (s->sz == 1 && *s->data == '\0') || s->sz == 0;
}


mnbytes_t *
bytes_json_escape(const mnbytes_t *src)
{
    size_t i, j;
    mnbytes_t *dest;

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
bytes_json_unescape(mnbytes_t *src)
{
    size_t i, j;
    mnbytes_t *dest;

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
    dest->hash = 0;
}


void bytes_tr(mnbytes_t * restrict s,
              unsigned char * restrict from,
              unsigned char * restrict to,
              size_t sz) {
    size_t i;

    for (i = 0; i < s->sz; ++i) {
        size_t j;

        for (j = 0; j < sz; ++j) {
            if (s->data[i] == from[j]) {
                s->data[i] = to[j];
            }
        }
    }
    s->hash = 0;
}


bool
bytes_is_ascii(const mnbytes_t *s)
{
    size_t i, sz;
    int mod;

    mod = s->sz % sizeof(uint64_t);
    sz = s->sz - mod;

    for (i = 0; i < sz; i += sizeof(uint64_t)) {
        uint64_t *n;

        n = (uint64_t *)(s->data + i);
        if (*n & 0x8080808080808080) {
            return false;
        }
    }

    while (--mod >= 0) {
        if (s->data[i + mod] & 0x80) {
            return false;
        }
    }
    return true;
}


mnbytes_t *
bytes_set_lower(mnbytes_t *s)
{
    ssize_t sz;

    s->hash = 0;
    sz = s->sz;
    while (--sz >= 0) {
        s->data[sz] = (unsigned char)tolower((int)s->data[sz]);
    }
    return s;
}


mnbytes_t *
bytes_set_upper(mnbytes_t *s)
{
    ssize_t sz;

    s->hash = 0;
    sz = s->sz;
    while (--sz >= 0) {
        s->data[sz] = (unsigned char)toupper((int)s->data[sz]);
    }
    return s;
}


uint64_t
bytes_hash(const mnbytes_t *bytes)
{
    if (bytes->hash == 0) {
        ((mnbytes_t *)bytes)->hash = fasthash(0, bytes->data, bytes->sz);
    }
    return bytes->hash;
}


#define BYTES_CMP_SAFE_BODY(cmpfn)     \
    if (a == NULL) {                   \
        if (b == NULL) {               \
            return 0;                  \
        } else {                       \
            return -1;                 \
        }                              \
    } else {                           \
        if (b == NULL) {               \
            return 1;                  \
        }                              \
    }                                  \
    return cmpfn(a, b);                \


int
bytes_cmp(const mnbytes_t *a, const mnbytes_t *b)
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
bytes_cmp_safe(const mnbytes_t *a, const mnbytes_t *b)
{
    BYTES_CMP_SAFE_BODY(bytes_cmp)
}


int
bytes_cmpv(const mnbytes_t *a, const mnbytes_t *b)
{
    return strncmp((char *)a->data,
                   (char *)b->data,
                   MIN(a->sz, b->sz));
}


int
bytes_cmpv_safe(const mnbytes_t *a, const mnbytes_t *b)
{
    BYTES_CMP_SAFE_BODY(bytes_cmpv)
}


int
bytes_cmpi(const mnbytes_t *a, const mnbytes_t *b)
{
    return strncasecmp((char *)a->data,
                       (char *)b->data,
                       MIN(a->sz, b->sz));
}


int
bytes_cmpi_safe(const mnbytes_t *a, const mnbytes_t *b)
{
    BYTES_CMP_SAFE_BODY(bytes_cmpi)
}


bool
bytes_contains(const mnbytes_t *a, const mnbytes_t *b)
{
    if (a->data[a->sz - 1] != '\0' || b->data[b->sz - 1] != '\0') {
        return false;
    }
    return strstr((char *)a->data, (char *)b->data) != NULL;
}


bool
bytes_containsi(const mnbytes_t *a, const mnbytes_t *b)
{
    if (a->data[a->sz - 1] != '\0' || b->data[b->sz - 1] != '\0') {
        return false;
    }
    return strcasestr((char *)a->data, (char *)b->data) != NULL;
}


#define BYTES_NEW_BODY(malloc_fn)                              \
    size_t mod, msz;                                           \
    mnbytes_t *res;                                            \
    assert(sz > 0);                                            \
    msz = sz;                                                  \
    mod = sz % 8;                                              \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(mnbytes_t) + msz)) == NULL) {  \
        FAIL("malloc");                                        \
    }                                                          \
    res->nref = 0;                                             \
    res->sz = sz;                                              \
    res->hash = 0;                                             \
    return res

#define BYTES_NEW_FROM_STR_BODY(malloc_fn)                     \
    mnbytes_t *res;                                            \
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
    if ((res = malloc_fn(sizeof(mnbytes_t) + msz)) == NULL) {  \
        FAIL("malloc");                                        \
    }                                                          \
    memcpy(res->data, s, sz);                                  \
    res->nref = 0;                                             \
    res->sz = sz;                                              \
    res->hash = 0;                                             \
    return res


mnbytes_t *
bytes_new(size_t sz)
{
    BYTES_NEW_BODY(malloc);
}


mnbytes_t *
bytes_new_from_str(const char *s)
{
    BYTES_NEW_FROM_STR_BODY(malloc);
}


#define BYTES_NEW_FROM_BYTES_BODY(malloc_fn)                   \
    mnbytes_t *res;                                            \
    size_t mod, msz;                                           \
    assert(s->sz > 0);                                         \
    msz = s->sz;                                               \
    mod = s->sz % 8;                                           \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(mnbytes_t) + msz)) == NULL) {  \
        FAIL("malloc");                                        \
    }                                                          \
    memcpy(res->data, s->data, s->sz);                         \
    res->nref = 0;                                             \
    res->sz = s->sz;                                           \
    res->hash = 0;                                             \
    return res


mnbytes_t *
bytes_new_from_bytes(const mnbytes_t *s)
{
    BYTES_NEW_FROM_BYTES_BODY(malloc);
}


#define BYTES_NEW_FROM_STR_LEN_BODY(malloc_fn, __a0)           \
    mnbytes_t *res;                                            \
    size_t mod, msz;                                           \
    __a0;                                                      \
    msz = sz;                                                  \
    mod = sz % 8;                                              \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(mnbytes_t) + msz)) == NULL) {  \
        FAIL("malloc");                                        \
    }                                                          \
    memcpy(res->data, s, sz);                                  \
    res->data[sz - 1] = '\0';                                  \
    res->nref = 0;                                             \
    res->sz = sz;                                              \
    res->hash = 0;                                             \
    return res                                                 \



mnbytes_t *
bytes_new_from_str_len(const char *s, size_t sz)
{
    BYTES_NEW_FROM_STR_LEN_BODY(malloc, ++sz);
}


mnbytes_t *
bytes_new_from_buf_len(const char *s, size_t sz)
{
    BYTES_NEW_FROM_STR_LEN_BODY(malloc,);
}


#define BYTES_NEW_FROM_MEM_LEN_BODY(malloc_fn, __a0)           \
    mnbytes_t *res;                                            \
    size_t mod, msz;                                           \
    __a0;                                                      \
    msz = sz;                                                  \
    mod = sz % 8;                                              \
    if (mod) {                                                 \
        msz += (8 - mod);                                      \
    } else {                                                   \
        msz += 8;                                              \
    }                                                          \
    if ((res = malloc_fn(sizeof(mnbytes_t) + msz)) == NULL) {  \
        FAIL("malloc");                                        \
    }                                                          \
    memcpy(res->data, s, sz);                                  \
    res->nref = 0;                                             \
    res->sz = sz;                                              \
    res->hash = 0;                                             \
    return res                                                 \



mnbytes_t *
bytes_new_from_mem_len(const char *s, size_t sz)
{
    BYTES_NEW_FROM_MEM_LEN_BODY(malloc,);
}


#define _malloc(sz) mpool_malloc(mpool, (sz))
mnbytes_t *
bytes_new_mpool(mpool_ctx_t *mpool, size_t sz)
{
    BYTES_NEW_BODY(_malloc);
}
mnbytes_t *
bytes_new_from_str_mpool(mpool_ctx_t *mpool, const char *s)
{
    BYTES_NEW_FROM_STR_BODY(_malloc);
}
mnbytes_t *
bytes_new_from_bytes_mpool(mpool_ctx_t *mpool, const mnbytes_t *s)
{
    BYTES_NEW_FROM_BYTES_BODY(_malloc);
}
mnbytes_t *
bytes_new_from_str_len_mpool(mpool_ctx_t *mpool, const char *s, size_t sz)
{
    BYTES_NEW_FROM_STR_LEN_BODY(_malloc, ++sz);
}
mnbytes_t *
bytes_new_from_buf_len_mpool(mpool_ctx_t *mpool, const char *s, size_t sz)
{
    BYTES_NEW_FROM_STR_LEN_BODY(_malloc,);
}
mnbytes_t *
bytes_new_from_mem_len_mpool(mpool_ctx_t *mpool, const char *s, size_t sz)
{
    BYTES_NEW_FROM_MEM_LEN_BODY(_malloc,);
}


#undef _malloc


void
bytes_memset(mnbytes_t *s, int c)
{
    memset(BDATA(s), c, BSZ(s));
    s->hash = 0;
}


void
bytes_memsetz(mnbytes_t *s, int c)
{
    memset(BDATA(s), c, BSZ(s) - 1);
    BDATA(s)[BSZ(s) - 1] = '\0';
    s->hash = 0;
}


mnbytes_t * PRINTFLIKE(1, 2)
bytes_printf(const char *fmt, ...)
{
    int nused;
    char x;
    mnbytes_t *res;
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


mnbytes_t *
bytes_vprintf(const char *fmt, va_list ap0)
{
    int nused;
    char x;
    mnbytes_t *res;
    va_list ap1;

    va_copy(ap1, ap0);
    nused = vsnprintf(&x, 0, fmt, ap0);

    res = bytes_new(nused + 1);

    nused = vsnprintf((char *)res->data, res->sz, fmt, ap1);
    va_end(ap1);
    assert((size_t)nused < res->sz);

    return res;
}


void
bytes_copy(mnbytes_t * restrict dst, const mnbytes_t * restrict src, size_t off)
{
    assert((off + src->sz) <= dst->sz);
    memcpy(dst->data + off, src->data, src->sz);
}


void
bytes_copyz(mnbytes_t * restrict dst, const mnbytes_t * restrict src, size_t off)
{
    assert((off + src->sz - 1) <= dst->sz);
    memcpy(dst->data + off, src->data, src->sz - 1);
    *(dst->data + off + src->sz - 1) = '\0';
}


void
bytes_urldecode(mnbytes_t *str)
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


static size_t
urlencode_reserved(char * restrict dst, const char * restrict src, size_t sz)
{
    unsigned char c;
    unsigned int i, j;

    for (i = 0, j = 0; i < sz; ++i, ++j) {
        c = (unsigned char)(src[i]);

        if (!(charflags[c] & RFC3986_UNRESERVED)) {
            unsigned char cc = c >> 4;

            *(dst + j++) = '%';
            if (cc < 10) {
                *(dst + j++) = '0' + cc;
            } else {
                *(dst + j++) = 'A' + cc - 10;
            }
            cc = (c & 0x0f);
            if (cc < 10) {
                *(dst + j) = '0' + cc;
            } else {
                *(dst + j) = 'A' + cc - 10;
            }

        } else {
            *(dst + j) = (char)c;
        }
    }

    *(dst + j) = '\0';
    return j + 1;
}


void
bytes_urlencode2(mnbytes_t * restrict dst, const mnbytes_t * restrict src)
{
    assert(BSZ(dst) >= (BSZ(src) * 3 + 1));
    BSZ(dst) = urlencode_reserved(BCDATA(dst), BCDATA(src), BSZ(src));
    dst->hash = 0;
}


void
bytes_str_urlencode2(mnbytes_t * restrict dst, const mnbytes_t * restrict src)
{
    assert(BSZ(dst) >= (BSZ(src) * 3 + 1));
    BSZ(dst) = urlencode_reserved(BCDATA(dst), BCDATA(src), BSZ(src) - 1);
    dst->hash = 0;
}


void
bytes_rstrip_blanks(mnbytes_t *str)
{
    ssize_t idx;

    idx = str->sz - 1;
    while (--idx >= 0) {
        if (str->data[idx] == ' ' ||
            str->data[idx] == '\n' ||
            str->data[idx] == '\t') {
            str->data[idx] = '\0';
        } else {
            str->data[idx + 1] = '\0';
            break;
        }
    }
    str->hash = 0;
    str->sz = idx + 2;
}


/*
 * URL decode + remove spaces
 */
void
bytes_brushdown(mnbytes_t *str)
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


mnbytes_t *
bytes_base64_encode_url_str(const mnbytes_t *s)
{
    mnbytes_t *res;
    size_t sz0, sz1;
    size_t m;

    if (BSZ(s) == 0) {
        return NULL;
    }

    sz0 = BSZ(s) - 1;
    m = sz0 % 3;
    sz1 = (sz0 + (m ? (3 - m) : 0)) * 4 / 3;

    res = bytes_new(sz1 + 1);
    if (MNUNLIKELY(mnbase64_encode_url_std(BDATA(s),
                    sz0,
                    BCDATA(res),
                    sz1) != 0)) {
        BYTES_DECREF(&res);
    } else {
        BDATA(res)[sz1] = '\0';
    }

    return res;
}


int
bytes_base64_decode_url(mnbytes_t *s)
{
    int res;
    size_t sz = BSZ(s) - 1;

    res = mnbase64_decode_url_std_inplace(BCDATA(s), &sz);
    BDATA(s)[sz] = '\0';
    BSZ(s) = sz + 1;
    (void)bytes_hash(s);
    return res;
}


int
bytes_split_iter(const mnbytes_t *str, char *delim, bytes_split_cb cb, void *udata)
{
    int res = 0;
    mnbytes_t *tmp = NULL;

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
            tmp->hash = 0;
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
        tmp->hash = 0;
        res = cb(tmp, udata);
    }

end:
    BYTES_DECREF(&tmp);
    return res;
}


void
bytes_decref(mnbytes_t **value)
{
    _BYTES_DECREF(value);
}


void
bytes_decref_fast(mnbytes_t *value)
{
    _BYTES_DECREF_FAST(value);
}


void
bytes_incref(mnbytes_t *value)
{
    BYTES_INCREF(value);
}
