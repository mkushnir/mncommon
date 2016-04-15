#ifndef MRKCOMMON_JSON_H
#define MRKCOMMON_JSON_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * json packing utils
 */
#pragma GCC diagnostic ignored "-Waddress"

#define EQC_JCAT_CONST(bs, s) ((void)bytestream_cat(bs, sizeof(s) - 1, s))


#define EQC_JPRINTF_PAIR_BS00(bs, key, value, comma)                   \
do {                                                                   \
    bytes_t *_eqc_jprintf_pair_bytes_tmp0;                             \
    bytes_t *_eqc_jprintf_pair_bytes_tmp1;                             \
    _eqc_jprintf_pair_bytes_tmp0 = bytes_new(SEOD(value) + 1);         \
    (void)memcpy((char *)_eqc_jprintf_pair_bytes_tmp0->data,           \
                 SDATA(value, 0),                                      \
                 SEOD(value));                                         \
    _eqc_jprintf_pair_bytes_tmp0->data[SEOD(value)] = '\0';            \
    _eqc_jprintf_pair_bytes_tmp1 =                                     \
        bytes_json_escape(_eqc_jprintf_pair_bytes_tmp0);               \
    (void)bytestream_nprintf(                                          \
            bs,                                                        \
            sizeof(key) - 1 + 8 + _eqc_jprintf_pair_bytes_tmp1->sz - 1,\
            "\"" key "\":\"%s\"" comma,                                \
            _eqc_jprintf_pair_bytes_tmp1->data);                       \
    BYTES_DECREF(&_eqc_jprintf_pair_bytes_tmp0);                       \
    BYTES_DECREF(&_eqc_jprintf_pair_bytes_tmp1);                       \
} while (false)                                                        \


#define EQC_JPRINTF_PAIR_BS0(bs, key, value) \
    EQC_JPRINTF_PAIR_BS00(bs, key, value, ",")

#define EQC_JPRINTF_PAIR_BS1(bs, key, value) \
    EQC_JPRINTF_PAIR_BS00(bs, key, value, "")


#define EQC_JPRINTF_ITEM_BS00(bs, value, comma)                \
do {                                                           \
    bytes_t *_eqc_jprintf_pair_bytes_tmp0;                     \
    bytes_t *_eqc_jprintf_pair_bytes_tmp1;                     \
    _eqc_jprintf_pair_bytes_tmp0 = bytes_new(SEOD(value) + 1); \
    (void)memcpy((char *)_eqc_jprintf_pair_bytes_tmp0->data,   \
                 SDATA(value, 0),                              \
                 SEOD(value));                                 \
    _eqc_jprintf_pair_bytes_tmp0->data[SEOD(value)] = '\0';    \
    _eqc_jprintf_pair_bytes_tmp1 =                             \
        bytes_json_escape(_eqc_jprintf_pair_bytes_tmp0);       \
    (void)bytestream_nprintf(                                  \
            bs,                                                \
            8 + _eqc_jprintf_pair_bytes_tmp1->sz - 1,          \
            "\"%s\"" comma,                                    \
            _eqc_jprintf_pair_bytes_tmp1->data);               \
    BYTES_DECREF(&_eqc_jprintf_pair_bytes_tmp0);               \
    BYTES_DECREF(&_eqc_jprintf_pair_bytes_tmp1);               \
} while (false)                                                \


#define EQC_JPRINTF_ITEM_BS0(bs, value) \
    EQC_JPRINTF_ITEM_BS00(bs, value, ",")

#define EQC_JPRINTF_ITEM_BS1(bs, value) \
    EQC_JPRINTF_ITEM_BS00(bs, value, "")


#define EQC_JPRINTF_PAIR_BYTES00(bs, key, value, comma)                        \
do {                                                                           \
    if (value != NULL) {                                                       \
        bytes_t *_eqc_jprintf_pair_bytes_tmp;                                  \
        _eqc_jprintf_pair_bytes_tmp = bytes_json_escape(value);                \
        (void)bytestream_nprintf(                                              \
                bs,                                                            \
                sizeof(key) - 1 + 8 + _eqc_jprintf_pair_bytes_tmp->sz - 1,     \
                "\"" key "\":\"%s\"" comma,                                    \
                _eqc_jprintf_pair_bytes_tmp->data);                            \
        BYTES_DECREF(&_eqc_jprintf_pair_bytes_tmp);                            \
    } else {                                                                   \
        (void)bytestream_nprintf(                                              \
                bs,                                                            \
                sizeof(key) - 1 + 8,                                           \
                "\"" key "\":null" comma);                                     \
    }                                                                          \
} while (false)                                                                \


#define EQC_JPRINTF_PAIR_BYTES0(bs, key, value) \
    EQC_JPRINTF_PAIR_BYTES00(bs, key, value, ",")

#define EQC_JPRINTF_PAIR_BYTES1(bs, key, value) \
    EQC_JPRINTF_PAIR_BYTES00(bs, key, value, "")


#define EQC_JPRINTF_ITEM_BYTES00(bs, value, comma)             \
do {                                                           \
    if (value != NULL) {                                       \
        bytes_t *_eqc_jprintf_pair_bytes_tmp;                  \
        _eqc_jprintf_pair_bytes_tmp = bytes_json_escape(value);\
        (void)bytestream_nprintf(                              \
                bs,                                            \
                8 + _eqc_jprintf_pair_bytes_tmp->sz - 1,       \
                "\"%s\"" comma,                                \
                _eqc_jprintf_pair_bytes_tmp->data);            \
        BYTES_DECREF(&_eqc_jprintf_pair_bytes_tmp);            \
    } else {                                                   \
        (void)bytestream_nprintf( bs, 8, "null" comma);        \
    }                                                          \
} while (false)                                                \


#define EQC_JPRINTF_ITEM_BYTES0(bs, value) \
    EQC_JPRINTF_ITEM_BYTES00(bs, value, ",")

#define EQC_JPRINTF_ITEM_BYTES1(bs, value) \
    EQC_JPRINTF_ITEM_BYTES00(bs, value, "")


#define EQC_JPRINTF_PAIR_INT00(bs, key, value, comma)  \
    (void)bytestream_nprintf(                          \
            bs,                                        \
            sizeof(key) - 1 + 8 + 1024,                \
            "\"" key "\":%ld" comma, (intmax_t)value)  \


#define EQC_JPRINTF_PAIR_INT0(bs, key, value) \
    EQC_JPRINTF_PAIR_INT00(bs, key, value, ",")

#define EQC_JPRINTF_PAIR_INT1(bs, key, value) \
    EQC_JPRINTF_PAIR_INT00(bs, key, value, "")


#define EQC_JPRINTF_ITEM_INT00(bs, value, comma)                       \
    (void)bytestream_nprintf( bs, 1024, "%ld" comma, (intmax_t)value)  \


#define EQC_JPRINTF_ITEM_INT0(bs, value) \
    EQC_JPRINTF_ITEM_INT00(bs, value, ",")

#define EQC_JPRINTF_ITEM_INT1(bs, value) \
    EQC_JPRINTF_ITEM_INT00(bs, value, "")



#define EQC_JPRINTF_PAIR_FLOAT00(bs, key, value, comma)\
    (void)bytestream_nprintf(                          \
            bs,                                        \
            sizeof(key) - 1 + 4 + 1024,                \
            "\"" key "\":%lf" comma, (double)value)    \


#define EQC_JPRINTF_PAIR_FLOAT0(bs, key, value) \
    EQC_JPRINTF_PAIR_FLOAT00(bs, key, value, ",")

#define EQC_JPRINTF_PAIR_FLOAT1(bs, key, value) \
    EQC_JPRINTF_PAIR_FLOAT00(bs, key, value, "")


#define EQC_JPRINTF_ITEM_FLOAT00(bs, value, comma)                     \
    (void)bytestream_nprintf( bs, 1024, "%lf" comma, (double)value)    \


#define EQC_JPRINTF_ITEM_FLOAT0(bs, value) \
    EQC_JPRINTF_ITEM_FLOAT00(bs, value, ",")

#define EQC_JPRINTF_ITEM_FLOAT1(bs, value) \
    EQC_JPRINTF_ITEM_FLOAT00(bs, value, "")


#define EQC_JPRINTF_PAIR_BOOL00(bs, key, value, comma)         \
    (void)bytestream_nprintf(                                  \
            bs,                                                \
            sizeof(key) - 1 + 32,                              \
            "\"" key "\":%s" comma, value ? "true" : "false")  \


#define EQC_JPRINTF_PAIR_BOOL0(bs, key, value) \
    EQC_JPRINTF_PAIR_BOOL00(bs, key, value, ",")

#define EQC_JPRINTF_PAIR_BOOL1(bs, key, value) \
    EQC_JPRINTF_PAIR_BOOL00(bs, key, value, "")


#define EQC_JPRINTF_ITEM_BOOL00(bs, value, comma)                              \
    (void)bytestream_nprintf( bs, 16, "%s" comma, value ? "true" : "false")    \


#define EQC_JPRINTF_ITEM_BOOL0(bs, value) \
    EQC_JPRINTF_ITEM_BOOL00(bs, value, ",")

#define EQC_JPRINTF_ITEM_BOOL1(bs, value) \
    EQC_JPRINTF_ITEM_BOOL00(bs, value, "")


/*
 * json types
 */
typedef enum _json_type {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INT,
    JSON_FLOAT,
    JSON_BOOLEAN,
    JSON_NULL,
} json_type_t;

#define JSON_TYPE_STR(ty) ( \
    (ty) == JSON_OBJECT ? "OBJECT" : \
    (ty) == JSON_ARRAY ? "ARRAY" : \
    (ty) == JSON_STRING ? "STRING" : \
    (ty) == JSON_INT ? "INT" : \
    (ty) == JSON_FLOAT ? "FLOAT" : \
    (ty) == JSON_BOOLEAN ? "BOOLEAN" : \
    (ty) == JSON_NULL ? "NULL" : \
    "<unknown>" \
)

struct _json_ctx;
typedef int (*json_cb) (struct _json_ctx *, void *);

typedef struct _json_ctx {
    const char *in;
    size_t sz;
#   define JPS_START    (1<<0)
#   define JPS_OSTART   (1<<1)
#   define JPS_OEND     (1<<2)
#   define JPS_ASTART   (1<<3)
#   define JPS_AEND     (1<<4)
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
#   define JPS_OUT      (JPS_STROUT | JPS_NUMOUT | JPS_TOKOUT | JPS_OEND | JPS_AEND)
#   define JPS_TOSTR(st) ( \
        (st) == JPS_START ? "START" : \
        (st) == JPS_OSTART ? "OSTART" : \
        (st) == JPS_OEND ? "OEND" : \
        (st) == JPS_ASTART ? "ASTART" : \
        (st) == JPS_AEND ? "AEND" : \
        (st) == JPS_KEYIN ? "KEYIN" : \
        (st) == JPS_KEYESC ? "KEYESC" : \
        (st) == JPS_KEY ? "KEY" : \
        (st) == JPS_KEYOUT ? "KEYOUT" : \
        (st) == JPS_STRIN ? "STRIN" : \
        (st) == JPS_STRESC ? "STRESC" : \
        (st) == JPS_STR ? "STR" : \
        (st) == JPS_STROUT ? "STROUT" : \
        (st) == JPS_NUMIN ? "NUMIN" : \
        (st) == JPS_NUM ? "NUM" : \
        (st) == JPS_NUMOUT ? "NUMOUT" : \
        (st) == JPS_TOKIN ? "TOKIN" : \
        (st) == JPS_TOK ? "TOK" : \
        (st) == JPS_TOKOUT ? "TOKOUT" : \
        (st) == JPS_ENEXT ? "ENEXT" : \
        (st) == JPS_EVALUE ? "EVALUE" : \
        "<unknown>")
    int st;
#   define JPS_FNEEDUNESCAPE    0x01
#   define JPS_FFLOAT           0x02
#   define JPS_FSCIENTIFIC      0x04
    unsigned flags;
    unsigned obj_level;
    unsigned array_level;
    json_type_t ty;
    union {
        long i;
        double f;
        struct {
            size_t start;
            size_t end;
        } s;
        //char *s;
        char b:1;
    } v;

    char *buf;
    size_t idx;

    json_cb cb;
    void *udata;
    json_cb ostart_cb;
    void *ostart_udata;
    json_cb oend_cb;
    void *oend_udata;
    json_cb astart_cb;
    void *astart_udata;
    json_cb aend_cb;
    void *aend_udata;
    json_cb key_cb;
    void *key_udata;
    json_cb value_cb;
    void *value_udata;
    json_cb item_cb;
    void *item_udata;
} json_ctx_t;

int json_init(json_ctx_t *, json_cb, void *);
void json_set_ostart_cb(json_ctx_t *, json_cb, void *);
void json_set_oend_cb(json_ctx_t *, json_cb, void *);
void json_set_astart_cb(json_ctx_t *, json_cb, void *);
void json_set_aend_cb(json_ctx_t *, json_cb, void *);
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

#ifdef __cplusplus
}
#endif

#endif
// vim:list
