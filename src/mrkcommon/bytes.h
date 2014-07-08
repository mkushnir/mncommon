#ifndef MRKCOMMON_BYTES_H_DEFINED
#define MRKCOMMON_BYTES_H_DEFINED

#include <sys/types.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/mpool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * XXX check out compile_bytes_t(), lkit_compile_expr(), and ltype_compile()
 */
typedef struct _bytes {
    ssize_t nref;
#define BYTES_SZ_IDX 1
    size_t sz;
    uint64_t hash;
#define BYTES_DATA_IDX 3
    unsigned char data[];
} bytes_t;

#define BYTES_INCREF(b) \
do { \
    (++(b)->nref); \
    /* TRACE("B>>> %p %ld", (b), (b)->nref); */ \
} while (0)

#define BYTES_DECREF(pb) \
do { \
    if (*(pb) != NULL) { \
        --(*(pb))->nref; \
        if ((*(pb))->nref <= 0) { \
            /* TRACE("B<<< %p %ld '%s'", *(pb), (*(pb))->nref, (*(pb))->data); */ \
            free(*(pb)); \
        } \
        *(pb) = NULL; \
    } \
} while (0)

#define BYTES_DECREF_FAST(b) \
do { \
    --((b))->nref; \
    if (((b))->nref <= 0) { \
        /* TRACE("B<<< %p %ld '%s'", b, (b)->nref, (b)->data); */ \
        free((b)); \
    } \
} while (0)

char *strrstr(const char *, const char *);

bytes_t *bytes_new(size_t);
bytes_t *bytes_new_mpool(mpool_ctx_t *, size_t);
bytes_t *bytes_new_from_str(const char *);
bytes_t *bytes_new_from_str_mpool(mpool_ctx_t *, const char *);
bytes_t *bytes_new_from_bytes(const bytes_t *);
bytes_t *bytes_new_from_bytes_mpool(mpool_ctx_t *, const bytes_t *);
void bytes_incref(bytes_t *);
void bytes_decref(bytes_t **);
void bytes_decref_fast(bytes_t *);
uint64_t bytes_hash(bytes_t *);
int bytes_cmp(bytes_t *, bytes_t *);
void bytes_copy(bytes_t *, bytes_t *, size_t);
void bytes_brushdown(bytes_t *);
void bytes_urldecode(bytes_t *);
bytes_t *bytes_json_escape(bytes_t *);

#ifdef __cplusplus
}
#endif
#endif /* MRKCOMMON_BYTES_H_DEFINED */
