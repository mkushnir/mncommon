#ifndef MRKCOMMON_BYTES_H_DEFINED
#define MRKCOMMON_BYTES_H_DEFINED

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

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
} bytes_t;


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

bytes_t *bytes_new(size_t);
bytes_t *bytes_new_mpool(mpool_ctx_t *, size_t);
bytes_t *bytes_new_from_str(const char *);
bytes_t *bytes_new_from_str_mpool(mpool_ctx_t *, const char *);
bytes_t *bytes_new_from_bytes(const bytes_t *);
bytes_t *bytes_new_from_bytes_mpool(mpool_ctx_t *, const bytes_t *);
bytes_t * PRINTFLIKE(1, 2) bytes_printf(const char *, ...);
void bytes_incref(bytes_t *);
void bytes_decref(bytes_t **);
void bytes_decref_fast(bytes_t *);
uint64_t bytes_hash(bytes_t *);
int bytes_cmp(bytes_t *, bytes_t *);
int bytes_cmpv(bytes_t *, bytes_t *);
int bytes_cmpi(bytes_t *, bytes_t *);
bool bytes_contains(bytes_t *, bytes_t *);
bool bytes_containsi(bytes_t *, bytes_t *);
void bytes_copy(bytes_t *, bytes_t *, size_t);
void bytes_copyz(bytes_t *, bytes_t *, size_t);
void bytes_brushdown(bytes_t *);
void bytes_urldecode(bytes_t *);
void bytes_rstrip_blanks(bytes_t *);
bytes_t *bytes_set_lower(bytes_t *s);
bytes_t *bytes_json_escape(bytes_t *);
void bytes_json_unescape(bytes_t *);
bool bytes_is_ascii(bytes_t *);
bool bytes_startswith(const bytes_t *, const bytes_t *);
bool bytes_endswith(const bytes_t *, const bytes_t *);
bool bytes_is_null_or_empty(const bytes_t *);
typedef int (*bytes_split_cb)(bytes_t *, void *);
int bytes_split_iter(bytes_t *, char *, bytes_split_cb, void *);

#ifdef __cplusplus
}
#endif
#endif /* MRKCOMMON_BYTES_H_DEFINED */
