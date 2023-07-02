#ifndef MNCOMMON_JPARSE_H
#define MNCOMMON_JPARSE_H

#include <mncommon/bytestream.h>
#include <mncommon/bytes.h>
#include <mncommon/json.h>
#include <mncommon/mpool.h>
#include <mncommon/util.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * https://tools.ietf.org/html/rfc7159
 */
#define JPARSE_EOS (-1)

struct _jparse_ctx;
struct _jparse_value;
typedef int (*jparse_expect_cb_t)(struct _jparse_ctx *, struct _jparse_value *, void *);

typedef struct _jparse_value {
    mnbytes_t *k;
    union {
        long i;
        double f;
        mnbytes_t *s;
        char b;
    } v;
    jparse_expect_cb_t cb;
    void *udata;
    json_type_t ty;
} jparse_value_t;

#define JPARSE_CAST_NUM(jval, ty_)                     \
    ((jval)->ty == JSON_FLOAT) ? (ty_)(jval)->v.f :    \
    ((jval)->ty == JSON_INT) ? (ty_)(jval)->v.i :      \
    ((jval)->ty == JSON_BOOLEAN) ? (ty_)(jval)->v.b :  \
    (ty_)0                                             \

typedef struct _jparse_ctx {
    mpool_ctx_t mpool;
    mnbytestream_t bs;
    jparse_expect_cb_t default_cb;
    void *udata;
    ssize_t errorpos;
    int fd;
} jparse_ctx_t;



void jparse_value_init(jparse_value_t *);


/*
 * scalars
 */
//jparse_expect_cb_t jparse_expect_ignore;
int jparse_expect_ignore(jparse_ctx_t *,
                         jparse_value_t *,
                         void *);

//jparse_expect_cb_t jparse_expect_any;
int jparse_expect_any(jparse_ctx_t *,
                      jparse_value_t *,
                      void *);

int jparse_expect_bool(jparse_ctx_t *,
                       char *,
                       void *);

int jparse_expect_int(jparse_ctx_t *,
                      long *,
                      void *);

int jparse_expect_float(jparse_ctx_t *,
                        double *,
                        void *);

int jparse_expect_str(jparse_ctx_t *,
                      mnbytes_t **,
                      void *);


/*
 * array
 */

/*
 * item
 */
//jparse_expect_cb_t jparse_expect_item_ignore;
int jparse_expect_item_ignore(jparse_ctx_t *,
                              jparse_value_t *,
                              void *);

//jparse_expect_cb_t jparse_expect_item_any;
int jparse_expect_item_any(jparse_ctx_t *,
                           jparse_value_t *,
                           void *);

int jparse_expect_item_bool(jparse_ctx_t *,
                            char *,
                            void *);

int jparse_expect_item_int(jparse_ctx_t *,
                           long *,
                           void *);

int jparse_expect_item_float(jparse_ctx_t *,
                             double *,
                             void *);

int jparse_expect_item_str(jparse_ctx_t *,
                           mnbytes_t **,
                           void *);

int jparse_expect_item_array(jparse_ctx_t *,
                             jparse_expect_cb_t,
                             jparse_value_t *,
                             void *);

int jparse_expect_item_array_iter(jparse_ctx_t *,
                                  jparse_expect_cb_t,
                                  jparse_value_t *,
                                  void *);

int jparse_expect_item_object(jparse_ctx_t *,
                              jparse_expect_cb_t,
                              jparse_value_t *,
                              void *);

int jparse_expect_item_object_iter(jparse_ctx_t *,
                                   jparse_expect_cb_t,
                                   jparse_value_t *,
                                   void *);

/*
 *
 */
int jparse_expect_array(jparse_ctx_t *,
                        jparse_expect_cb_t,
                        jparse_value_t *,
                        void *);

int jparse_expect_array_iter(jparse_ctx_t *,
                             jparse_expect_cb_t,
                             jparse_value_t *,
                             void *);


/*
 * object
 */

/*
 * any
 */
int jparse_expect_kvp_any(jparse_ctx_t *,
                          const mnbytes_t *,
                          jparse_value_t *,
                          void *);

int jparse_expect_skvp_any(jparse_ctx_t *,
                           const char *,
                           jparse_value_t *,
                           void *);

typedef int (*jparse_expect_anykvp_cb_t)(struct _jparse_ctx *,
                                         mnbytes_t **,
                                         jparse_value_t *val,
                                         void *);

//jparse_expect_anykvp_cb_t jparse_expect_anykvp_ignore;
int jparse_expect_anykvp_ignore(jparse_ctx_t *,
                                mnbytes_t **,
                                jparse_value_t *,
                                void *);

//jparse_expect_anykvp_cb_t jparse_expect_anykvp_any;
int jparse_expect_anykvp_any(jparse_ctx_t *,
                             mnbytes_t **,
                             jparse_value_t *,
                             void *);


/*
 * bool
 */
int jparse_expect_kvp_bool(jparse_ctx_t *,
                           const mnbytes_t *,
                           char *,
                           void *);
int jparse_expect_skvp_bool(jparse_ctx_t *,
                            const char *,
                            char *,
                            void *);

int jparse_expect_anykvp_bool(jparse_ctx_t *,
                              mnbytes_t **,
                              char *,
                              void *);

/*
 * int
 */
int jparse_expect_kvp_int(jparse_ctx_t *,
                          const mnbytes_t *,
                          long *,
                          void *);

int jparse_expect_skvp_int(jparse_ctx_t *,
                           const char *,
                           long *,
                           void *);

int jparse_expect_anykvp_int(jparse_ctx_t *,
                             mnbytes_t **,
                             long *,
                             void *);

/*
 * float
 */
int jparse_expect_kvp_float(jparse_ctx_t *,
                            const mnbytes_t *,
                            double *,
                            void *);

int jparse_expect_skvp_float(jparse_ctx_t *,
                             const char *,
                             double *,
                             void *);

int jparse_expect_anykvp_float(jparse_ctx_t *,
                               mnbytes_t **,
                               double *,
                               void *);

/*
 * str
 */
int jparse_expect_kvp_str(jparse_ctx_t *,
                          const mnbytes_t *,
                          mnbytes_t **,
                          void *);

int jparse_expect_skvp_str(jparse_ctx_t *,
                           const char *,
                           mnbytes_t **,
                           void *);

int jparse_expect_anykvp_str(jparse_ctx_t *,
                             mnbytes_t **,
                             mnbytes_t **,
                             void *);

/*
 * array
 */
int jparse_expect_kvp_array(jparse_ctx_t *,
                            const mnbytes_t *,
                            jparse_expect_cb_t,
                            jparse_value_t *,
                            void *);

int jparse_expect_kvp_array_iter(jparse_ctx_t *,
                                 const mnbytes_t *,
                                 jparse_expect_cb_t,
                                 jparse_value_t *,
                                 void *);

int jparse_expect_skvp_array(jparse_ctx_t *,
                             const char *,
                             jparse_expect_cb_t,
                             jparse_value_t *,
                             void *);

int jparse_expect_skvp_array_iter(jparse_ctx_t *,
                                  const char *,
                                  jparse_expect_cb_t,
                                  jparse_value_t *,
                                  void *);

int jparse_expect_anykvp_array(jparse_ctx_t *,
                               mnbytes_t **,
                               jparse_expect_cb_t,
                               jparse_value_t *,
                               void *);

int jparse_expect_anykvp_array_iter(jparse_ctx_t *,
                                    mnbytes_t **,
                                    jparse_expect_cb_t,
                                    jparse_value_t *,
                                    void *);

/*
 * object
 */
int jparse_expect_kvp_object(jparse_ctx_t *,
                             const mnbytes_t *,
                             jparse_expect_cb_t,
                             jparse_value_t *,
                             void *);

int jparse_expect_kvp_object_iter(jparse_ctx_t *,
                                  const mnbytes_t *,
                                  jparse_expect_cb_t,
                                  jparse_value_t *,
                                  void *);

int jparse_expect_skvp_object(jparse_ctx_t *,
                              const char *,
                              jparse_expect_cb_t,
                              jparse_value_t *,
                              void *);

int jparse_expect_skvp_object_iter(jparse_ctx_t *,
                                   const char *,
                                   jparse_expect_cb_t,
                                   jparse_value_t *,
                                   void *);

int jparse_expect_anykvp_object(jparse_ctx_t *,
                                mnbytes_t **,
                                jparse_expect_cb_t,
                                jparse_value_t *,
                                void *);

int jparse_expect_anykvp_object_iter(jparse_ctx_t *,
                                     mnbytes_t **,
                                     jparse_expect_cb_t,
                                     jparse_value_t *,
                                     void *);


/*
 *
 */
int jparse_expect_object(jparse_ctx_t *,
                         jparse_expect_cb_t,
                         jparse_value_t *,
                         void *);

int jparse_expect_object_iter(jparse_ctx_t *,
                              jparse_expect_cb_t,
                              jparse_value_t *,
                              void *);




/*
 * Array Iterator
 */
#define REF_JPARSE_ARRAY_ITERATOR(it) _jparse_array_iterator_##it


#define DECL_JPARSE_ARRAY_ITERATOR(it)                         \
static int REF_JPARSE_ARRAY_ITERATOR(it)(jparse_ctx_t *jctx,   \
                                         jparse_value_t *jval, \
                                         void *udata)          \


#define JPARSE_ARRAY_ITERATOR_BODY(it, expect_fn)                      \
    off_t spos;                                                        \
    int res;                                                           \
    spos = SPOS(&jctx->bs);                                            \
    for (res = 0; res == 0; res = expect_fn(jctx, jval, udata)) {      \
        ;                                                              \
    }                                                                  \
    if (res == JPARSE_EOS || (res != 0 && spos == SPOS(&jctx->bs))) {  \
        res = 0;                                                       \
    } else {                                                           \
        SPOS(&jctx->bs) = spos;                                        \
    }                                                                  \
    return res;                                                        \


#define DEF_JPARSE_ARRAY_ITERATOR(it, expect_fn)       \
DECL_JPARSE_ARRAY_ITERATOR(it)                         \
{                                                      \
    JPARSE_ARRAY_ITERATOR_BODY(it, expect_fn)          \
}                                                      \


/*
 * Object Iterator
 */
#define REF_JPARSE_OBJECT_ITERATOR(it) _jparse_object_iterator_##it


#define DECL_JPARSE_OBJECT_ITERATOR(it)                        \
static int REF_JPARSE_OBJECT_ITERATOR(it)(jparse_ctx_t *jctx,  \
                                          jparse_value_t *jval,\
                                          void *udata)         \


#define JPARSE_OBJECT_ITERATOR_BODY(it, expect_fn)                             \
    off_t spos;                                                                \
    int res;                                                                   \
    spos = SPOS(&jctx->bs);                                                    \
    for (res = 0; res == 0; res = expect_fn(jctx, &jval->k, jval, udata)) {    \
        ;                                                                      \
    }                                                                          \
    if (res == JPARSE_EOS || (res != 0 && spos == SPOS(&jctx->bs))) {          \
        res = 0;                                                               \
    } else {                                                                   \
        SPOS(&jctx->bs) = spos;                                                \
    }                                                                          \
    return res;                                                                \


#define DEF_JPARSE_OBJECT_ITERATOR(it, expect_fn)      \
DECL_JPARSE_OBJECT_ITERATOR(it)                        \
{                                                      \
    JPARSE_OBJECT_ITERATOR_BODY(it, expect_fn)         \
}                                                      \


int jparse_ignore_nonscalar_iterator(jparse_ctx_t *, jparse_value_t *, void *);

void jparse_dump_current_pos(jparse_ctx_t *, ssize_t);
#define JPARSE_DEP_FCLEAR (0x01)
void jparse_dump_error_pos(jparse_ctx_t *, ssize_t, int);
jparse_ctx_t *jparse_ctx_new(size_t, size_t);
void jparse_ctx_complete(jparse_ctx_t *);
void jparse_ctx_destroy(jparse_ctx_t **);
int jparse_ctx_parse(jparse_ctx_t *,
                     const char *,
                     jparse_expect_cb_t,
                     jparse_value_t *,
                     void *);
int jparse_ctx_parse_fd(jparse_ctx_t *,
                        int,
                        jparse_expect_cb_t,
                        jparse_value_t *,
                        void *);
int jparse_ctx_parse_data(jparse_ctx_t *,
                          const char *,
                          size_t,
                          jparse_expect_cb_t,
                          jparse_value_t *,
                          void *);
void jparse_dump_value(jparse_value_t *);

#ifdef __cplusplus
}
#endif

#endif
// vim:list
