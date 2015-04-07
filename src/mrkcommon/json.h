#ifndef MRKCOMMON_JSON_H
#define MRKCOMMON_JSON_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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
