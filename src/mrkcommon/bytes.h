#ifndef MRKCOMMON_BYTES_H_DEFINED
#define MRKCOMMON_BYTES_H_DEFINED

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

#include <mrkcommon/base64.h>
#include <mrkcommon/mpool.h>
#include <mrkcommon/util.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DO_MEMDEBUG
#define MEMDEBUG_ENTER_BYTES(self)                             \
{                                                              \
    int mdtag;                                                 \
    mdtag = memdebug_set_runtime_scope((int)(self)-> mdtag);   \


#define MEMDEBUG_LEAVE_BYTES(self)             \
    (void)memdebug_set_runtime_scope(mdtag);   \
}                                              \


#else
#define MEMDEBUG_ENTER_BYTES(seld)
#define MEMDEBUG_LEAVE_BYTES(seld)
#endif
/*
 * XXX check out compile_bytes_t(), lkit_compile_expr(), and ltype_compile()
 */
typedef struct _bytes {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#define BYTES_SZ_IDX 2
#define BYTES_DATA_IDX 4
#else
#define BYTES_SZ_IDX 1
#define BYTES_DATA_IDX 3
#endif
    ssize_t nref;
    size_t sz;
    uint64_t hash;
    unsigned char data[];
} mnbytes_t;


#define BDATA(b) (b)->data
#define BDATASAFE(b) ((b) != NULL ? (b)->data : NULL)
#define BSZ(b) (b)->sz
#define BSZSAFE(b) ((b) != NULL ? (b)->sz : 0)


#define BYTES_INITIALIZER(s)   \
{                              \
    .nref = 0x40000000,        \
    .sz = sizeof(s),           \
    .hash = 0l,                \
    .data = s                  \
}                              \


#define BYTES_INITIALIZERA(s)  \
{                              \
    .nref = 0x70000000,        \
    .sz = sizeof(s),           \
    .hash = 0l,                \
    .data = s                  \
}                              \


#define BYTES_ALLOCA(n, s)                             \
    struct {                                           \
        ssize_t nref;                                  \
        size_t sz;                                     \
        uint64_t hash;                                 \
        unsigned char data[sizeof(s)];                 \
    } __bytes_alloca_ ## n = BYTES_INITIALIZERA(s);    \
    mnbytes_t *n = (mnbytes_t *) &__bytes_alloca_ ## n;\


#define BYTES_INCREF(b)                        \
do {                                           \
    (++(b)->nref);                             \
    /* TRACE("B>>> %p %ld", (b), (b)->nref); */\
} while (0)                                    \


#define _BYTES_DECREF(pb)                                                      \
do {                                                                           \
    if (*(pb) != NULL) {                                                       \
        --(*(pb))->nref;                                                       \
        if ((*(pb))->nref <= 0) {                                              \
/*                                                                             \
            TRACE("B<<< %p %ld '%s'", *(pb), (*(pb))->nref, (*(pb))->data);    \
 */                                                                            \
            MEMDEBUG_ENTER_BYTES(*(pb));                                       \
            free(*(pb));                                                       \
            MEMDEBUG_LEAVE_BYTES(*(pb));                                       \
        }                                                                      \
        *(pb) = NULL;                                                          \
    }                                                                          \
} while (0)                                                                    \


#define _BYTES_DECREF_FAST(b)                                  \
do {                                                           \
    --((b))->nref;                                             \
    if (((b))->nref <= 0) {                                    \
/*                                                             \
        TRACE("B<<< %p %ld '%s'", b, (b)->nref, (b)->data);    \
 */                                                            \
        MEMDEBUG_ENTER_BYTES(b);                               \
        free((b));                                             \
        MEMDEBUG_LEAVE_BYTES(b);                               \
    }                                                          \
} while (0)                                                    \

#ifdef DO_MEMDEBUG
#define BYTES_DECREF bytes_decref
#define BYTES_DECREF_FAST bytes_decref_fast
#else
#define BYTES_DECREF _BYTES_DECREF
#define BYTES_DECREF_FAST _BYTES_DECREF_FAST
#endif

char *strrstr(const char *, const char *);

mnbytes_t *bytes_new(size_t);
mnbytes_t *bytes_new_mpool(mpool_ctx_t *, size_t);
mnbytes_t *bytes_new_from_str(const char *);
mnbytes_t *bytes_new_from_str_mpool(mpool_ctx_t *, const char *);
mnbytes_t *bytes_new_from_bytes(const mnbytes_t *);
mnbytes_t *bytes_new_from_bytes_mpool(mpool_ctx_t *, const mnbytes_t *);
mnbytes_t *bytes_new_from_str_len(const char *, size_t);
mnbytes_t *bytes_new_from_str_len_mpool(mpool_ctx_t *, const char *, size_t);
mnbytes_t * PRINTFLIKE(1, 2) bytes_printf(const char *, ...);
mnbytes_t *bytes_vprintf(const char *, va_list);
void bytes_incref(mnbytes_t *);
void bytes_decref(mnbytes_t **);
void bytes_decref_fast(mnbytes_t *);
uint64_t bytes_hash(mnbytes_t *);
int bytes_cmp(mnbytes_t *, mnbytes_t *);
int bytes_cmp_safe(mnbytes_t *, mnbytes_t *);
int bytes_cmpv(mnbytes_t *, mnbytes_t *);
int bytes_cmpv_safe(mnbytes_t *, mnbytes_t *);
int bytes_cmpi(mnbytes_t *, mnbytes_t *);
int bytes_cmpi_safe(mnbytes_t *, mnbytes_t *);
bool bytes_contains(mnbytes_t *, mnbytes_t *);
bool bytes_containsi(mnbytes_t *, mnbytes_t *);
void bytes_copy(mnbytes_t * restrict, mnbytes_t * restrict, size_t);
void bytes_copyz(mnbytes_t * restrict, mnbytes_t * restrict, size_t);
void bytes_brushdown(mnbytes_t *);
mnbytes_t *bytes_base64_encode_url_str(mnbytes_t *);
int bytes_base64_decode_url(mnbytes_t *);
void bytes_urldecode(mnbytes_t *);
void bytes_urlencode2(mnbytes_t *, mnbytes_t *);
void bytes_str_urlencode2(mnbytes_t *, mnbytes_t *);
void bytes_rstrip_blanks(mnbytes_t *);
mnbytes_t *bytes_set_lower(mnbytes_t *s);
mnbytes_t *bytes_json_escape(mnbytes_t *);
void bytes_json_unescape(mnbytes_t *);
void bytes_tr(mnbytes_t * restrict,
              unsigned char * restrict,
              unsigned char * restrict,
              size_t);
bool bytes_is_ascii(mnbytes_t *);
bool bytes_startswith(const mnbytes_t *, const mnbytes_t *);
bool bytes_endswith(const mnbytes_t *, const mnbytes_t *);
bool bytes_is_null_or_empty(const mnbytes_t *);
typedef int (*bytes_split_cb)(mnbytes_t *, void *);
int bytes_split_iter(mnbytes_t *, char *, bytes_split_cb, void *);

#ifdef __cplusplus
}
#endif
#endif /* MRKCOMMON_BYTES_H_DEFINED */
