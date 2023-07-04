#ifndef MNCOMMON_JSON_H
#define MNCOMMON_JSON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <mncommon/bytes.h>
#include <mncommon/bytestream.h>
#include <mncommon/dtqueue.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * json packing utils
 */
#if GCC_VERSION >= 40200
#pragma GCC diagnostic ignored "-Waddress"
#endif

#define MN_JCAT_CONST(bs, s) bytestream_cat(bs, sizeof(s) - 1, s)


#define MN_JPRINTF_PAIR_BS00(bs, key, value, comma)                    \
do {                                                                   \
    mnbytes_t *_mn_jprintf_pair_bytes_tmp0;                            \
    mnbytes_t *_mn_jprintf_pair_bytes_tmp1;                            \
    _mn_jprintf_pair_bytes_tmp0 = bytes_new(SEOD(value) + 1);          \
    (void)memcpy((char *)_mn_jprintf_pair_bytes_tmp0->data,            \
                 SDATA(value, 0),                                      \
                 SEOD(value));                                         \
    _mn_jprintf_pair_bytes_tmp0->data[SEOD(value)] = '\0';             \
    _mn_jprintf_pair_bytes_tmp1 =                                      \
        bytes_json_escape(_mn_jprintf_pair_bytes_tmp0);                \
    (void)bytestream_nprintf(                                          \
            bs,                                                        \
            sizeof(key) - 1 + 8 + _mn_jprintf_pair_bytes_tmp1->sz - 1, \
            "\"" key "\":\"%s\"" comma,                                \
            _mn_jprintf_pair_bytes_tmp1->data);                        \
    BYTES_DECREF(&_mn_jprintf_pair_bytes_tmp0);                        \
    BYTES_DECREF(&_mn_jprintf_pair_bytes_tmp1);                        \
} while (false)                                                        \


#define MN_JPRINTF_PAIR_BS0(bs, key, value) \
    MN_JPRINTF_PAIR_BS00(bs, key, value, ",")

#define MN_JPRINTF_PAIR_BS1(bs, key, value) \
    MN_JPRINTF_PAIR_BS00(bs, key, value, "")


#define MN_JPRINTF_ITEM_BS00(bs, value, comma)                 \
do {                                                           \
    mnbytes_t *_mn_jprintf_pair_bytes_tmp0;                    \
    mnbytes_t *_mn_jprintf_pair_bytes_tmp1;                    \
    _mn_jprintf_pair_bytes_tmp0 = bytes_new(SEOD(value) + 1);  \
    (void)memcpy((char *)_mn_jprintf_pair_bytes_tmp0->data,    \
                 SDATA(value, 0),                              \
                 SEOD(value));                                 \
    _mn_jprintf_pair_bytes_tmp0->data[SEOD(value)] = '\0';     \
    _mn_jprintf_pair_bytes_tmp1 =                              \
        bytes_json_escape(_mn_jprintf_pair_bytes_tmp0);        \
    (void)bytestream_nprintf(                                  \
            bs,                                                \
            8 + _mn_jprintf_pair_bytes_tmp1->sz - 1,           \
            "\"%s\"" comma,                                    \
            _mn_jprintf_pair_bytes_tmp1->data);                \
    BYTES_DECREF(&_mn_jprintf_pair_bytes_tmp0);                \
    BYTES_DECREF(&_mn_jprintf_pair_bytes_tmp1);                \
} while (false)                                                \


#define MN_JPRINTF_ITEM_BS0(bs, value) \
    MN_JPRINTF_ITEM_BS00(bs, value, ",")

#define MN_JPRINTF_ITEM_BS1(bs, value) \
    MN_JPRINTF_ITEM_BS00(bs, value, "")


#define MN_JPRINTF_PAIR_BYTES00(bs, key, value, comma)                         \
do {                                                                           \
    if (value != NULL) {                                                       \
        mnbytes_t *_mn_jprintf_pair_bytes_tmp;                                 \
        _mn_jprintf_pair_bytes_tmp = bytes_json_escape(value);                 \
        (void)bytestream_nprintf(                                              \
                bs,                                                            \
                sizeof(key) - 1 + 8 + _mn_jprintf_pair_bytes_tmp->sz - 1,      \
                "\"" key "\":\"%s\"" comma,                                    \
                _mn_jprintf_pair_bytes_tmp->data);                             \
        BYTES_DECREF(&_mn_jprintf_pair_bytes_tmp);                             \
    } else {                                                                   \
        (void)bytestream_nprintf(                                              \
                bs,                                                            \
                sizeof(key) - 1 + 16,                                          \
                "\"" key "\":null" comma);                                     \
    }                                                                          \
} while (false)                                                                \


#define MN_JPRINTF_PAIR_BYTES0(bs, key, value) \
    MN_JPRINTF_PAIR_BYTES00(bs, key, value, ",")

#define MN_JPRINTF_PAIR_BYTES1(bs, key, value) \
    MN_JPRINTF_PAIR_BYTES00(bs, key, value, "")


#define MN_JPRINTF_ITEM_BYTES00(bs, value, comma)              \
do {                                                           \
    if (value != NULL) {                                       \
        mnbytes_t *_mn_jprintf_pair_bytes_tmp;                 \
        _mn_jprintf_pair_bytes_tmp = bytes_json_escape(value); \
        (void)bytestream_nprintf(                              \
                bs,                                            \
                8 + _mn_jprintf_pair_bytes_tmp->sz - 1,        \
                "\"%s\"" comma,                                \
                _mn_jprintf_pair_bytes_tmp->data);             \
        BYTES_DECREF(&_mn_jprintf_pair_bytes_tmp);             \
    } else {                                                   \
        (void)bytestream_nprintf( bs, 8, "null" comma);        \
    }                                                          \
} while (false)                                                \


#define MN_JPRINTF_ITEM_BYTES0(bs, value) \
    MN_JPRINTF_ITEM_BYTES00(bs, value, ",")

#define MN_JPRINTF_ITEM_BYTES1(bs, value) \
    MN_JPRINTF_ITEM_BYTES00(bs, value, "")


#define MN_JPRINTF_PAIR_INT00(bs, key, value, comma)   \
    (void)bytestream_nprintf(                          \
            bs,                                        \
            sizeof(key) - 1 + 8 + 1024,                \
            "\"" key "\":%ld" comma, (intmax_t)value)  \


#define MN_JPRINTF_PAIR_INT0(bs, key, value) \
    MN_JPRINTF_PAIR_INT00(bs, key, value, ",")

#define MN_JPRINTF_PAIR_INT1(bs, key, value) \
    MN_JPRINTF_PAIR_INT00(bs, key, value, "")


#define MN_JPRINTF_ITEM_INT00(bs, value, comma)                        \
    (void)bytestream_nprintf( bs, 1024, "%ld" comma, (intmax_t)value)  \


#define MN_JPRINTF_ITEM_INT0(bs, value) \
    MN_JPRINTF_ITEM_INT00(bs, value, ",")

#define MN_JPRINTF_ITEM_INT1(bs, value) \
    MN_JPRINTF_ITEM_INT00(bs, value, "")



#define MN_JPRINTF_PAIR_FLOAT00(bs, key, value, comma) \
    (void)bytestream_nprintf(                          \
            bs,                                        \
            sizeof(key) - 1 + 4 + 1024,                \
            "\"" key "\":%lf" comma, (double)value)    \


#define MN_JPRINTF_PAIR_FLOAT0(bs, key, value) \
    MN_JPRINTF_PAIR_FLOAT00(bs, key, value, ",")

#define MN_JPRINTF_PAIR_FLOAT1(bs, key, value) \
    MN_JPRINTF_PAIR_FLOAT00(bs, key, value, "")


#define MN_JPRINTF_ITEM_FLOAT00(bs, value, comma)                      \
    (void)bytestream_nprintf( bs, 1024, "%lf" comma, (double)value)    \


#define MN_JPRINTF_ITEM_FLOAT0(bs, value) \
    MN_JPRINTF_ITEM_FLOAT00(bs, value, ",")

#define MN_JPRINTF_ITEM_FLOAT1(bs, value) \
    MN_JPRINTF_ITEM_FLOAT00(bs, value, "")


#define MN_JPRINTF_PAIR_BOOL00(bs, key, value, comma)          \
    (void)bytestream_nprintf(                                  \
            bs,                                                \
            sizeof(key) - 1 + 32,                              \
            "\"" key "\":%s" comma, value ? "true" : "false")  \


#define MN_JPRINTF_PAIR_BOOL0(bs, key, value) \
    MN_JPRINTF_PAIR_BOOL00(bs, key, value, ",")

#define MN_JPRINTF_PAIR_BOOL1(bs, key, value) \
    MN_JPRINTF_PAIR_BOOL00(bs, key, value, "")


#define MN_JPRINTF_ITEM_BOOL00(bs, value, comma)                               \
    (void)bytestream_nprintf( bs, 16, "%s" comma, value ? "true" : "false")    \


#define MN_JPRINTF_ITEM_BOOL0(bs, value) \
    MN_JPRINTF_ITEM_BOOL00(bs, value, ",")

#define MN_JPRINTF_ITEM_BOOL1(bs, value) \
    MN_JPRINTF_ITEM_BOOL00(bs, value, "")




#define MN_JPRINTF_PAIR_CB0(bs, key, cb, udata)        \
do {                                                   \
    (void)bytestream_nprintf((bs),                     \
                             sizeof(key) - 1 + 8,      \
                             "\"%s\":", key);          \
    (cb)((bs), (udata));                               \
    (void)bytestream_cat((bs), 1, ",");                \
} while (false)                                        \


#define MN_JPRINTF_PAIR_CB1(bs, key, cb, udata)        \
do {                                                   \
    (void)bytestream_nprintf((bs),                     \
                             sizeof(key) - 1 + 8,      \
                             "\"%s\":", key);          \
    (cb)((bs), (udata));                               \
} while (false)                                        \


#define MN_JPRINTF_BPAIR_CB0(bs, key, cb, udata)       \
do {                                                   \
    (void)bytestream_nprintf((bs),                     \
                             BSZ(key) - 1 + 8,         \
                             "\"%s\":", BDATA(key));   \
    (cb)((bs), (udata));                               \
    (void)bytestream_cat((bs), 1, ",");                \
} while (false)                                        \


#define MN_JPRINTF_BPAIR_CB1(bs, key, cb, udata)       \
do {                                                   \
    (void)bytestream_nprintf((bs),                     \
                             BSZ(key) - 1 + 8,         \
                             "\"%s\":", BDATA(key));   \
    (cb)((bs), (udata));                               \
} while (false)                                        \


#define MN_JCHOP_COMMA(bs)                                     \
if (*SDATA(bs, SEOD(bs) - 1) == ',') SADVANCEEOD(bs, -1)       \


/*
 * json types
 */
typedef enum _json_type {
    JSON_UNDEF,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INT,
    JSON_FLOAT,
    JSON_BOOLEAN,
    JSON_NULL,

    JSON_TYPE_LAST = 16,
    // type hints
    JSON_ITEM,
    JSON_ANY,
    JSON_ONEOF,
} json_type_t;

#define JSON_TYPE_STR(ty) (            \
    (ty) == JSON_UNDEF ? "UNDEF" :     \
    (ty) == JSON_OBJECT ? "OBJECT" :   \
    (ty) == JSON_ARRAY ? "ARRAY" :     \
    (ty) == JSON_STRING ? "STRING" :   \
    (ty) == JSON_INT ? "INT" :         \
    (ty) == JSON_FLOAT ? "FLOAT" :     \
    (ty) == JSON_BOOLEAN ? "BOOLEAN" : \
    (ty) == JSON_NULL ? "NULL" :       \
    "<unknown>"                        \
)                                      \

#define JSON_TYPE_HINT_OBJECT   (1 << JSON_OBJECT)
#define JSON_TYPE_HINT_ARRAY    (1 << JSON_ARRAY)
#define JSON_TYPE_HINT_STRING   (1 << JSON_STRING)
#define JSON_TYPE_HINT_INT      (1 << JSON_INT)
#define JSON_TYPE_HINT_FLOAT    (1 << JSON_FLOAT)
#define JSON_TYPE_HINT_BOOLEAN  (1 << JSON_BOOLEAN)
#define JSON_TYPE_HINT_NULL     (1 << JSON_NULL)
#define JSON_TYPE_HINT_ITEM     (1 << JSON_ITEM)
#define JSON_TYPE_HINT_ANY      (1 << JSON_ANY)
#define JSON_TYPE_HINT_ONEOF    (1 << JSON_ONEOF)


struct _json_ctx;
typedef int (*json_cb) (struct _json_ctx *, void *);

typedef struct _json_ctx {
    const char *in;
    size_t sz;
#   define JPS_START    (1<<0)
#   define JPS_OSTART   (1<<1)
#   define JPS_OSTOP     (1<<2)
#   define JPS_ASTART   (1<<3)
#   define JPS_ASTOP     (1<<4)
#   define JPS_KEYIN    (1<<5)
#   define JPS_KEYESC   (1<<6)
#   define JPS_KEY      (1<<7)
#   define JPS_KEYOUT   (1<<8)
#   define JPS_STRIN    (1<<9)
#   define JPS_STRESC   (1<<10)
#   define JPS_STR      (1<<11)
#   define JPS_STROUT   (1<<12)
#   define JPS_NUMIN    (1<<13)
#   define JPS_NUM      (1<<14)
#   define JPS_NUMOUT   (1<<15)
#   define JPS_TOKIN    (1<<16)
#   define JPS_TOK      (1<<17)
#   define JPS_TOKOUT   (1<<18)
#   define JPS_ENEXT    (1<<19)
#   define JPS_EVALUE   (1<<20)
#   define JPS_OUT      (JPS_STROUT | JPS_NUMOUT | JPS_TOKOUT | JPS_OSTOP | JPS_ASTOP)
#   define JPS_TOSTR(st) (             \
        (st) == JPS_START ? "START" :  \
        (st) == JPS_OSTART ? "OSTART" :\
        (st) == JPS_OSTOP ? "OSTOP" :  \
        (st) == JPS_ASTART ? "ASTART" :\
        (st) == JPS_ASTOP ? "ASTOP" :  \
        (st) == JPS_KEYIN ? "KEYIN" :  \
        (st) == JPS_KEYOUT ? "KEYOUT" :\
        (st) == JPS_STRIN ? "STRIN" :  \
        (st) == JPS_STRESC ? "STRESC" :\
        (st) == JPS_STR ? "STR" :      \
        (st) == JPS_STROUT ? "STROUT" :\
        (st) == JPS_NUMIN ? "NUMIN" :  \
        (st) == JPS_NUM ? "NUM" :      \
        (st) == JPS_NUMOUT ? "NUMOUT" :\
        (st) == JPS_TOKIN ? "TOKIN" :  \
        (st) == JPS_TOK ? "TOK" :      \
        (st) == JPS_TOKOUT ? "TOKOUT" :\
        (st) == JPS_ENEXT ? "ENEXT" :  \
        (st) == JPS_EVALUE ? "EVALUE" :\
        "<unknown>")
    int st;
#   define JPS_FNEEDUNESCAPE    0x01
#   define JPS_FFLOAT           0x02
#   define JPS_FSCIENTIFIC      0x04
    unsigned flags;
    json_type_t ty;
    union {
        long i;
        double f;
        struct {
            size_t start;
            size_t end;
        } s;
        char b:1;
    } v;

    const char *buf;
    size_t idx;
    int nest;

    json_cb cb;
    void *udata;
    json_cb ostart_cb;
    void *ostart_udata;
    json_cb ostop_cb;
    void *ostop_udata;
    json_cb astart_cb;
    void *astart_udata;
    json_cb astop_cb;
    void *astop_udata;
    json_cb key_cb;
    void *key_udata;
    json_cb value_cb;
    void *value_udata;
    json_cb item_cb;
    void *item_udata;
} json_ctx_t;


struct _json_node;
typedef int (*json_node_cb_t)(struct _json_node *, json_ctx_t *, void *);
typedef struct _json_node {
    DTQUEUE_ENTRY(_json_node, link);
    json_node_cb_t cb;
    int nest;
    int ty;
    void *v;
    void **c;
    size_t csz;
} json_node_t;
                                                               \
#define JSON_NODE_INITIALIZER(cb_, ty_, v_, ...)(json_node_t){ \
    .cb = cb_,                                                 \
    .nest = -1,                                                \
    .ty = ty_,                                                 \
    .v = (void *)v_,                                           \
    .c = (void *[]){__VA_ARGS__},                              \
    .csz = (sizeof((void *[]){__VA_ARGS__}) / sizeof(void *))  \
}                                                              \


#define JSON_NODE_DEF(n, cb_, ty_, v_, ...) json_node_t n = JSON_NODE_INITIALIZER(cb_, ty_, v_, __VA_ARGS__)

#define JSON_NODE_CHILD_REF(n, idx) ((json_node_t **)(n).c)[(idx)]

#define JSON_NODE_OBJECT(cb_, ...) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_OBJECT, NULL, __VA_ARGS__)
#define JSON_NODE_ARRAY(cb_, ...) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_ARRAY, NULL, __VA_ARGS__)
#define JSON_NODE_NULL(cb_) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_NULL, NULL)
#define JSON_NODE_INT(cb_) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_INT, NULL)
#define JSON_NODE_FLOAT(cb_) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_FLOAT, NULL)
#define JSON_NODE_STRING(cb_) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_STRING, NULL)
#define JSON_NODE_BOOLEAN(cb_) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_BOOLEAN, NULL)
#define JSON_NODE_AITEM(cb_, ty_, ...) JSON_NODE_INITIALIZER(cb_, ty_ | JSON_TYPE_HINT_ITEM, NULL, __VA_ARGS__)
#define JSON_NODE_OITEM(cb_, ty_, v_, ...) JSON_NODE_INITIALIZER(cb_, ty_ | JSON_TYPE_HINT_ITEM, v_, __VA_ARGS__)
#define JSON_NODE_ANY(cb_) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_ANY, NULL)
#define JSON_NODE_ONEOF(cb_, ...) JSON_NODE_INITIALIZER(cb_, JSON_TYPE_HINT_ONEOF, NULL, __VA_ARGS__)

#define JSON_NODE_OITEM_OBJECT(cb_, v_, ...) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_OBJECT, v_, __VA_ARGS__)
#define JSON_NODE_OITEM_ARRAY(cb_, v_, ...) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_ARRAY, v_, __VA_ARGS__)
#define JSON_NODE_OITEM_NULL(cb_, v_) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_NULL, v_)
#define JSON_NODE_OITEM_INT(cb_, v_) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_INT, v_)
#define JSON_NODE_OITEM_FLOAT(cb_, v_) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_FLOAT, v_)
#define JSON_NODE_OITEM_STRING(cb_, v_) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_STRING, v_)
#define JSON_NODE_OITEM_BOOLEAN(cb_, v_) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_BOOLEAN, v_)
#define JSON_NODE_OITEM_ANY(cb_, v_) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_ANY, v_)
#define JSON_NODE_OITEM_ONEOF(cb_, v_, ...) JSON_NODE_OITEM(cb_, JSON_TYPE_HINT_ONEOF, v_, ...)

#define JSON_NODE_AITEM_OBJECT(cb_, ...) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_OBJECT, __VA_ARGS__)
#define JSON_NODE_AITEM_ARRAY(cb_, ...) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_ARRAY, __VA_ARGS__)
#define JSON_NODE_AITEM_NULL(cb_) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_NULL)
#define JSON_NODE_AITEM_INT(cb_) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_INT)
#define JSON_NODE_AITEM_FLOAT(cb_) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_FLOAT)
#define JSON_NODE_AITEM_STRING(cb_) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_STRING)
#define JSON_NODE_AITEM_BOOLEAN(cb_) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_BOOLEAN)
#define JSON_NODE_AITEM_ANY(cb_) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_ANY)
#define JSON_NODE_AITEM_ONEOF(cb_, ...) JSON_NODE_AITEM(cb_, JSON_TYPE_HINT_ONEOF, __VA_ARGS__)


int json_init(json_ctx_t *, json_cb, void *);
void json_reset(json_ctx_t *);
void json_set_ostart_cb(json_ctx_t *, json_cb, void *);
void json_set_ostop_cb(json_ctx_t *, json_cb, void *);
void json_set_astart_cb(json_ctx_t *, json_cb, void *);
void json_set_astop_cb(json_ctx_t *, json_cb, void *);
void json_set_key_cb(json_ctx_t *, json_cb, void *);
void json_set_value_cb(json_ctx_t *, json_cb, void *);
void json_set_item_cb(json_ctx_t *, json_cb, void *);
int json_fini(json_ctx_t *);
void json_dump(json_ctx_t *);
#define JSON_PARSE_NEEDMORE (-2)
int json_parse(json_ctx_t *, const char *, size_t);
int json_parse_obj(json_ctx_t *);
int json_parse_array(json_ctx_t *);
int json_parse_str(json_ctx_t *);
int json_parse_num(json_ctx_t *);
int json_parse_tok(json_ctx_t *);


const char *json_type_hint_str (json_node_t *);

ssize_t mnjson_bs_pair0(mnbytestream_t *, const mnbytes_t *, mnbytestream_t *);
ssize_t mnjson_bs_pair1(mnbytestream_t *, const mnbytes_t *, mnbytestream_t *);
ssize_t mnjson_bs_item0(mnbytestream_t *, mnbytestream_t *);
ssize_t mnjson_bs_item1(mnbytestream_t *, mnbytestream_t *);
ssize_t mnjson_bytes_pair0(mnbytestream_t *, const mnbytes_t *, const mnbytes_t *);
ssize_t mnjson_bytes_pair1(mnbytestream_t *, const mnbytes_t *, const mnbytes_t *);
ssize_t mnjson_bytes_item0(mnbytestream_t *, const mnbytes_t *);
ssize_t mnjson_bytes_item1(mnbytestream_t *, const mnbytes_t *);
ssize_t mnjson_int_pair0(mnbytestream_t *, const mnbytes_t *, intmax_t);
ssize_t mnjson_int_pair1(mnbytestream_t *, const mnbytes_t *, intmax_t);
ssize_t mnjson_int_item0(mnbytestream_t *, intmax_t);
ssize_t mnjson_int_item1(mnbytestream_t *, intmax_t);
ssize_t mnjson_float_pair0(mnbytestream_t *, const mnbytes_t *, double);
ssize_t mnjson_float_pair1(mnbytestream_t *, const mnbytes_t *, double);
ssize_t mnjson_float_item0(mnbytestream_t *, double);
ssize_t mnjson_float_item1(mnbytestream_t *, double);
ssize_t mnjson_bool_pair0(mnbytestream_t *, const mnbytes_t *, bool);
ssize_t mnjson_bool_pair1(mnbytestream_t *, const mnbytes_t *, bool);
ssize_t mnjson_bool_item0(mnbytestream_t *, bool);
ssize_t mnjson_bool_item1(mnbytestream_t *, bool);
ssize_t mnjson_chop_comma(mnbytestream_t *);

#ifdef __cplusplus
}
#endif

#endif
// vim:list
