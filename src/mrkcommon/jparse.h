#ifndef MRKCOMMON_JPARSE_H
#define MRKCOMMON_JPARSE_H

#include <mrkcommon/bytestream.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/json.h>
#include <mrkcommon/mpool.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JPARSE_EOS (-1)

typedef struct _jparse_value {
    json_type_t ty;
    union {
        long i;
        double f;
        bytes_t *s;
        char b;
    } v;
} jparse_value_t;

typedef struct _jparse_ctx {
    mpool_ctx_t mpool;
    bytes_t *_null;
    bytes_t *_true;
    bytes_t *_false;
    bytestream_t bs;
    void *udata;
    int fd;
} jparse_ctx_t;

typedef int (*jparse_expect_cb_t)(jparse_ctx_t *);

int jparse_expect_any(jparse_ctx_t *, jparse_value_t *);
int jparse_expect_tok(jparse_ctx_t *, bytes_t **);
int jparse_expect_maybe_null(jparse_ctx_t *);
int jparse_expect_int(jparse_ctx_t *, long *);
int jparse_expect_float(jparse_ctx_t *, double *);
int jparse_expect_str(jparse_ctx_t *, bytes_t **);
int jparse_expect_bool(jparse_ctx_t *, char *);

int jparse_expect_object(jparse_ctx_t *, jparse_expect_cb_t);
int jparse_expect_kvp_int(jparse_ctx_t *, bytes_t **, long *);
int jparse_expect_kvp_float(jparse_ctx_t *, bytes_t **, double *);
int jparse_expect_kvp_str(jparse_ctx_t *, bytes_t **, bytes_t **);
int jparse_expect_kvp_bool(jparse_ctx_t *, bytes_t **, char *);
int jparse_expect_kvp_array(jparse_ctx_t *, bytes_t **, jparse_expect_cb_t);
int jparse_expect_kvp_object(jparse_ctx_t *, bytes_t **, jparse_expect_cb_t);

int jparse_expect_array(jparse_ctx_t *, jparse_expect_cb_t);
int jparse_expect_item_int(jparse_ctx_t *, long *);
int jparse_expect_item_float(jparse_ctx_t *, double *);
int jparse_expect_item_str(jparse_ctx_t *, bytes_t **);
int jparse_expect_item_bool(jparse_ctx_t *, char *);
int jparse_expect_item_array(jparse_ctx_t *, jparse_expect_cb_t);
int jparse_expect_item_object(jparse_ctx_t *, jparse_expect_cb_t);

jparse_ctx_t *jparse_ctx_new(size_t, size_t);
void jparse_ctx_destroy(jparse_ctx_t **);
int jparse_ctx_parse(jparse_ctx_t *, const char *, jparse_expect_cb_t, void *);

#ifdef __cplusplus
}
#endif

#endif
// vim:list
