#ifndef MNCOMMON_BYTES_H_DEFINED
#define MNCOMMON_BYTES_H_DEFINED

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

#include <mncommon/base64.h>
#include <mncommon/mpool.h>
#include <mncommon/util.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * XXX check out compile_bytes_t(), lkit_compile_expr(), and ltype_compile()
 */
typedef struct _mnbytes {
    ssize_t nref;
    size_t sz;
    uint64_t hash;
    unsigned char data[];
} mnbytes_t;


#define BDATA(b) (b)->data
#define BDATASAFE(b) ((b) != NULL ? (b)->data : NULL)
#define BCDATA(b) (char *)(b)->data
#define BCDATASAFE(b) (char *)((b) != NULL ? (b)->data : NULL)
#define BSZ(b) (b)->sz
#define BSZSAFE(b) ((b) != NULL ? (b)->sz : 0)


static inline const char *
mnbytes_strz(const mnbytes_t *s) { return (const char *)s->data; }

static inline const char *
mnvoidp_strz(const void *s) { return (const char *)s; }

#define MNSTRZ(s) _Generic(s,                  \
        mnbytes_t *: mnbytes_strz,             \
        const mnbytes_t *: mnbytes_strz,       \
        default: mnvoidp_strz                  \
        )(s)                                   \


/*
 * s must be a string literal
 */

#define _BYTES_INITIALIZER(s, n)       \
{                                      \
    .nref = n,                         \
    .sz = sizeof(s),                   \
    .hash = 0l,                        \
    .data = "" s ""                    \
}                                      \


#define BYTES_INITIALIZER(s) _BYTES_INITIALIZER(s, 0x40000000)
#define BYTES_NREF_STATIC_INVARIANT(s) assert((s).nref == 0x40000000)

#define BYTES_INITIALIZERA(s) _BYTES_INITIALIZER(s, 0x70000000)
#define BYTES_NREF_AUTO_INVARIANT(s) assert((s).nref == 0x70000000)

#define BYTES_INITIALIZERB(s) _BYTES_INITIALIZER(s, 0x90000000)
#define BYTES_NREF_REF_INVARIANT(s) assert((s).nref == 0x90000000)


#define BYTES_ALLOCA(n, s)                             \
    struct {                                           \
        ssize_t nref;                                  \
        size_t sz;                                     \
        uint64_t hash;                                 \
        unsigned char data[sizeof(s)];                 \
    } __bytes_alloca_ ## n = BYTES_INITIALIZERA(s);    \
    mnbytes_t *n = (mnbytes_t *) &__bytes_alloca_ ## n \


#define BYTES_REF(s)                   \
    (mnbytes_t const * const)(&(struct {     \
    ssize_t nref;                      \
    size_t sz;                         \
    uint64_t hash;                     \
    unsigned char data[sizeof(s)];     \
    })BYTES_INITIALIZERB(s))           \


#define BYTES_INCREF(b)                                                \
do {                                                                   \
    (++(b)->nref);                                                     \
    /*                                                                 \
        TRACE("B>>> %p %08zx sz=%08zx", (b), (b)->nref, (b)->sz);      \
    */                                                                 \
} while (0)                                                            \


#define _BYTES_DECREF(pb)                              \
do {                                                   \
    if (*(pb) != NULL) {                               \
/*                                                     \
            TRACE("B<<< %p nref=%08zx sz=%08zx",       \
                    *(pb), (*(pb))->nref, (*(pb))->sz);\
 */                                                    \
        --(*(pb))->nref;                               \
        if ((*(pb))->nref <= 0) {                      \
            free(*(pb));                               \
        }                                              \
        *(pb) = NULL;                                  \
    }                                                  \
} while (0)                                            \


#define _BYTES_DECREF_FAST(b)                                  \
do {                                                           \
    --((b))->nref;                                             \
    if (((b))->nref <= 0) {                                    \
/*                                                             \
        TRACE("B<<< %p %ld '%s'", b, (b)->nref, (b)->data);    \
 */                                                            \
        free((b));                                             \
    }                                                          \
} while (0)                                                    \

#define BYTES_DECREF _BYTES_DECREF
#define BYTES_DECREF_FAST _BYTES_DECREF_FAST

char *strrstr(const char *, const char *);

mnbytes_t *bytes_new(size_t);
mnbytes_t *bytes_new_mpool(mpool_ctx_t *, size_t);
mnbytes_t *bytes_new_from_str(const char *);
mnbytes_t *bytes_new_from_str_mpool(mpool_ctx_t *, const char *);
mnbytes_t *bytes_new_from_bytes(const mnbytes_t *);
mnbytes_t *bytes_new_from_bytes_mpool(mpool_ctx_t *, const mnbytes_t *);

/* adding the terminating zero */
mnbytes_t *bytes_new_from_str_len(const char *, size_t);
mnbytes_t *bytes_new_from_str_len_mpool(mpool_ctx_t *, const char *, size_t);
/* setting the terminating zero */
mnbytes_t *bytes_new_from_buf_len(const char *, size_t);
mnbytes_t *bytes_new_from_buf_len_mpool(mpool_ctx_t *, const char *, size_t);
/* doing nothing */
mnbytes_t *bytes_new_from_mem_len(const char *, size_t);
mnbytes_t *bytes_new_from_mem_len_mpool(mpool_ctx_t *, const char *, size_t);

/* setting the terminating zero */
void bytes_memsetz(mnbytes_t *, int);
/* doing nothing */
void bytes_memset(mnbytes_t *, int);
mnbytes_t * PRINTFLIKE(1, 2) bytes_printf(const char *, ...);
mnbytes_t *bytes_vprintf(const char *, va_list);
void bytes_incref(mnbytes_t *);
void bytes_decref(mnbytes_t **);
void bytes_decref_fast(mnbytes_t *);
uint64_t bytes_hash(const mnbytes_t *);
int bytes_cmp(const mnbytes_t *, const mnbytes_t *);
int bytes_cmp_safe(const mnbytes_t *, const mnbytes_t *);
int bytes_cmpv(const mnbytes_t *, const mnbytes_t *);
int bytes_cmpv_safe(const mnbytes_t *, const mnbytes_t *);
int bytes_cmpi(const mnbytes_t *, const mnbytes_t *);
int bytes_cmpi_safe(const mnbytes_t *, const mnbytes_t *);
bool bytes_contains(const mnbytes_t *, const mnbytes_t *);
bool bytes_containsi(const mnbytes_t *, const mnbytes_t *);
void bytes_copy(mnbytes_t * restrict, const mnbytes_t * restrict, size_t);
void bytes_copyz(mnbytes_t * restrict, const mnbytes_t * restrict, size_t);
void bytes_brushdown(mnbytes_t *);
mnbytes_t *bytes_base64_encode_url_str(const mnbytes_t *);
int bytes_base64_decode_url(mnbytes_t *);
void bytes_urldecode(mnbytes_t *);
void bytes_urlencode2(mnbytes_t * restrict , const mnbytes_t * restrict);
void bytes_str_urlencode2(mnbytes_t * restrict, const mnbytes_t * restrict);
void bytes_rstrip_blanks(mnbytes_t *);
mnbytes_t *bytes_set_lower(mnbytes_t *s);
mnbytes_t *bytes_set_upper(mnbytes_t *s);
mnbytes_t *bytes_json_escape(const mnbytes_t *);
void bytes_json_unescape(mnbytes_t *);
void bytes_tr(mnbytes_t * restrict,
              unsigned char * restrict,
              unsigned char * restrict,
              size_t);
bool bytes_is_ascii(const mnbytes_t *);
bool bytes_startswith(const mnbytes_t *, const mnbytes_t *);
bool bytes_endswith(const mnbytes_t *, const mnbytes_t *);
bool bytes_is_null_or_empty(const mnbytes_t *);
typedef int (*bytes_split_cb)(const mnbytes_t *, void *);
int bytes_split_iter(const mnbytes_t *, char *, bytes_split_cb, void *);

#ifdef __cplusplus
}
#endif
#endif /* MNCOMMON_BYTES_H_DEFINED */
