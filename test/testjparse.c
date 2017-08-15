#include <assert.h>
#include <fcntl.h>

#include "unittest.h"
#include "diag.h"
#define TRRET_DEBUG
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/jparse.h"
#include "mrkcommon/bytes.h"


static int
o1(UNUSED jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    return 0;
}

/*
 * array
 */
static int
aempty(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    long v = 0;
    char buf[64];

    res = jparse_expect_item_int(jctx, &v, NULL);
    mndiag_local_str(res, buf, sizeof(buf));
    TRACE("res=%s", buf);
    return 0;
}


static int
aint(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res, i;
    long v = 0;
    char buf[64];

    res = 0;
    i = 0;
    while (res == 0) {
        res = jparse_expect_item_int(jctx, &v, NULL);
        mndiag_local_str(res, buf, sizeof(buf));
        TRACE("res=%s val[%d]=%ld", buf, i++, v);
    }
    return 0;
}


static int
aarray(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res, i;
    char buf[64];

    res = 0;
    i = 0;
    while (res == 0) {
        res = jparse_expect_item_array(jctx, aint, NULL, NULL);
        mndiag_local_str(res, buf, sizeof(buf));
        TRACE("res=%s [%d]", buf, i++);
    }
    return 0;
}


static int
mycb10(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_array(jctx, aempty, val, udata);
}


static int
mycb11(jparse_ctx_t *jctx, jparse_value_t *val,  void *udata)
{
    return jparse_expect_array(jctx, aint, val, udata);
}


static int
mycb12(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_array(jctx, aarray, val, udata);
}


/*
 * object
 */
static int
mycb1(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    UNUSED int res;

    res = jparse_expect_object(jctx, o1, val, udata);
    return 0;
}

static int
mycb2(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    UNUSED int res;

    res = jparse_expect_object(jctx, o1, val, udata);
    res = jparse_expect_object(jctx, o1, val, udata);
    return 0;
}


static int
oint(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    mnbytes_t *key;
    jparse_value_t val = {.v.i = 0};

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_int(jctx, key, &val.v.i, NULL);
    bytes_decref(&key);
    TRACE("abc=%ld", val.v.i);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_int(jctx, key, &val.v.i, NULL);
    bytes_decref(&key);
    TRACE("qwe=%ld", val.v.i);

end:
    if (res == JPARSE_EOS) {
        res = 0;
    } else {
        TR(res);
    }
    return res;
}


static int
mycb03(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_object(jctx, oint, val, udata);
}


static int
ofloat(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    mnbytes_t *key;
    jparse_value_t val = { .v.f = .0};

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_float(jctx, key, &val.v.f, NULL);
    bytes_decref(&key);
    TRACE("abc=%lf", val.v.f);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_float(jctx, key, &val.v.f, NULL);
    bytes_decref(&key);
    TRACE("qwe=%lf", val.v.f);

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
ostr(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    mnbytes_t *key;
    jparse_value_t val = { .v.s = NULL};

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_str(jctx, key, &val.v.s, NULL);
    bytes_decref(&key);
    TRACE("abc=%s", val.v.s != NULL ? (char *)val.v.s->data : "<null>");
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_str(jctx, key, &val.v.s, NULL);
    bytes_decref(&key);
    TRACE("qwe=%s", val.v.s != NULL ? (char *)val.v.s->data : "<null>");

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
obool(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    mnbytes_t *key;
    jparse_value_t val = { .v.b = 0 };

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_bool(jctx, key, &val.v.b, NULL);
    bytes_decref(&key);
    TRACE("abc=%s", val.v.b ? "#t" : "#f");
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_bool(jctx, key, &val.v.b, NULL);
    bytes_decref(&key);
    TRACE("qwe=%s", val.v.b ? "#t" : "#f");
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("asd");
    res = jparse_expect_kvp_bool(jctx, key, &val.v.b, NULL);
    bytes_decref(&key);
    TRACE("asd=%s", val.v.b ? "#t" : "#f");

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
oarray(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    mnbytes_t *key;

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_array(jctx, key, aint, NULL, NULL);
    bytes_decref(&key);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_array(jctx, key, aint, NULL, NULL);
    bytes_decref(&key);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("asd");
    res = jparse_expect_kvp_array(jctx, key, aint, NULL, NULL);
    bytes_decref(&key);

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
mycb04(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_object(jctx, ofloat, val, udata);
}


static int
mycb05(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_object(jctx, ostr, val, udata);
}


static int
mycb06(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_object(jctx, obool, val, udata);
}


static int
mycb07(jparse_ctx_t *jctx, jparse_value_t *val, void *udata)
{
    return jparse_expect_object(jctx, oarray, val, udata);
}


UNUSED static void
test0(void)
{
    jparse_ctx_t *jctx;
    UNUSED int res;

    struct {
        long rnd;
        const char *in;
        jparse_expect_cb_t cb;
    } data[] = {
        {0, "data/testjparse/empty-object-01", mycb1},
        {0, "data/testjparse/empty-object-02", mycb1},
        {0, "data/testjparse/empty-object-03", mycb1},
        {0, "data/testjparse/empty-object-0301", mycb1},
        {0, "data/testjparse/empty-object-03", mycb2},
        {0, "data/testjparse/empty-object-04", mycb2},

        {0, "data/testjparse/object-int-01", mycb03},
        {0, "data/testjparse/object-int-02", mycb03},
        {0, "data/testjparse/object-float-01", mycb04},
        {0, "data/testjparse/object-float-02", mycb04},
        {0, "data/testjparse/object-str-01", mycb05},
        {0, "data/testjparse/object-str-02", mycb05},
        {0, "data/testjparse/object-bool-01", mycb06},
        {0, "data/testjparse/object-array-01", mycb07},

        {0, "data/testjparse/empty-array-01", mycb10},
        {0, "data/testjparse/array-int-01", mycb11},
        {0, "data/testjparse/array-int-02", mycb11},
        {0, "data/testjparse/array-array-01", mycb12},
        {0, "data/testjparse/array-array-02", mycb12},

    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        TRACE("in=%s", CDATA.in);
        jctx = jparse_ctx_new(4096, 4096);
        res = jparse_ctx_parse(jctx, CDATA.in, CDATA.cb, NULL, NULL);
        jparse_ctx_destroy(&jctx);
        assert(res == 0);
    }
}


static int
mycb100(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    jparse_value_t v;

    res = jparse_expect_ignore(jctx, &v, udata);
    jparse_dump_value(&v);
    return res;
}


DEF_JPARSE_OBJECT_ITERATOR(mycb200, jparse_expect_anykvp_any);

static int
mycb200(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    jparse_value_t v;
    v.cb = REF_JPARSE_OBJECT_ITERATOR(mycb200);

    res = jparse_expect_any(jctx, &v, udata);
    jparse_dump_value(&v);
    TR(res);
    return 0;
}


DEF_JPARSE_ARRAY_ITERATOR(mycb201, jparse_expect_item_any);

static int
mycb201(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, UNUSED void *udata)
{
    int res;
    jparse_value_t v;
    v.cb = REF_JPARSE_ARRAY_ITERATOR(mycb201);

    res = jparse_expect_any(jctx, &v, udata);
    jparse_dump_value(&v);
    TR(res);
    return 0;
}


static void
test1(void)
{
    jparse_ctx_t *jctx;
    UNUSED int res;

    struct {
        long rnd;
        const char *in;
        jparse_expect_cb_t cb;
    } data[] = {
        {0, "data/testjparse/object-int-01", mycb100},
        {0, "data/testjparse/array-int-01", mycb100},
        {0, "data/testjparse/object-int-01", mycb200},
        {0, "data/testjparse/array-int-01", mycb201},

    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        TRACE("in=%s", CDATA.in);
        jctx = jparse_ctx_new(4096, 4096);
        jctx->default_cb = mycb200;
        res = jparse_ctx_parse(jctx, CDATA.in, CDATA.cb, NULL, NULL);
        jparse_ctx_destroy(&jctx);
        assert(res == 0);
    }
}





UNUSED static mnbytes_t _o1 = BYTES_INITIALIZER("o1");
UNUSED static mnbytes_t _o2 = BYTES_INITIALIZER("o2");
UNUSED static mnbytes_t _o3 = BYTES_INITIALIZER("o3");


DEF_JPARSE_ARRAY_ITERATOR(ai, jparse_expect_item_ignore)
DEF_JPARSE_OBJECT_ITERATOR(oi, jparse_expect_anykvp_ignore)

static int
ocb(jparse_ctx_t *jctx,
    jparse_value_t *jval,
    void *udata)
{
    int res;
    jparse_value_t _jval;

    jparse_value_init(&_jval);

    if (jval->ty == JSON_ARRAY) {
        res = REF_JPARSE_ARRAY_ITERATOR(ai)(jctx, &_jval, udata);
    } else if (jval->ty == JSON_OBJECT) {
        res = REF_JPARSE_OBJECT_ITERATOR(oi)(jctx, &_jval, udata);
    } else {
        res = jparse_expect_ignore(jctx, &_jval, udata);
    }
    TR(res);
    return res;
}


static int
oexp(jparse_ctx_t *jctx,
     mnbytes_t **k,
     UNUSED jparse_value_t *jval,
     void *udata)
{
    int res;
    jparse_value_t _jval;

    jparse_value_init(&_jval);
    _jval.cb = ocb;

    res = jparse_expect_anykvp_any(jctx, k, &_jval, udata);
    TR(res);
    if (*k != NULL) {
        TRACE("k=%s", (*k)->data);
        jparse_dump_value(&_jval);
    }
    return res;
}
DEF_JPARSE_OBJECT_ITERATOR(oexp, oexp);


static int
mycb300(jparse_ctx_t *jctx,
        UNUSED jparse_value_t *jval,
        void *udata)
{
    int res;
    jparse_value_t _jval;

    assert(jval == NULL);

    jparse_value_init(&_jval);

    res = jparse_expect_object(jctx,
                               REF_JPARSE_OBJECT_ITERATOR(oexp),
                               &_jval,
                               udata);

    TR(res);
    return 0;
}







UNUSED static mnbytes_t _queries = BYTES_INITIALIZER("queries");
UNUSED static mnbytes_t _id = BYTES_INITIALIZER("id");
UNUSED static mnbytes_t _name = BYTES_INITIALIZER("name");
UNUSED static mnbytes_t _meta = BYTES_INITIALIZER("meta");
UNUSED static mnbytes_t _versions = BYTES_INITIALIZER("versions");


static int
query_field_nonscalar(jparse_ctx_t *jctx, jparse_value_t *jval, void *udata)
{
    int res;
    jparse_value_t _jval;

    jparse_value_init(&_jval);

    if (bytes_cmp(jval->k, &_meta) == 0) {
        // handle meta
        res = REF_JPARSE_OBJECT_ITERATOR(oi)(jctx, &_jval, udata);
    } else if (bytes_cmp(jval->k, &_versions) == 0) {
        // handle versions
        res = REF_JPARSE_ARRAY_ITERATOR(ai)(jctx, &_jval, udata);
    } else if (jval->ty == JSON_ARRAY) {
        res = REF_JPARSE_ARRAY_ITERATOR(ai)(jctx, &_jval, udata);
    } else if (jval->ty == JSON_OBJECT) {
        res = REF_JPARSE_OBJECT_ITERATOR(oi)(jctx, &_jval, udata);
    } else {
        FAIL("query_field_nonscalar");
    }
    return res;
}

static int
query_item(jparse_ctx_t *jctx, UNUSED jparse_value_t *jval, void *udata)
{
    int res;
    mnbytes_t *k;
    jparse_value_t _jval;
    jparse_value_init(&_jval);
    _jval.cb = query_field_nonscalar;
    res = jparse_expect_anykvp_any(jctx, &k, &_jval, udata);
    TRACE("k=%s", _jval.k->data);
    jparse_dump_value(&_jval);
    return res;
}

DEF_JPARSE_ARRAY_ITERATOR(query_item, query_item)



static int
queries_array(jparse_ctx_t *jctx,
              UNUSED jparse_value_t *jval,
              void *udata)
{
    int res;

    jparse_value_t _jval;
    jparse_value_init(&_jval);
    res = jparse_expect_item_object(jctx,
                                     REF_JPARSE_ARRAY_ITERATOR(query_item),
                                     &_jval,
                                     udata);
    //jparse_dump_value(jval);
    //jparse_dump_value(&_jval);
    return res;
}

DEF_JPARSE_ARRAY_ITERATOR(queries_array, queries_array)

static int
queries_key(jparse_ctx_t *jctx,
            UNUSED mnbytes_t **k,
            jparse_value_t *jval,
            void *udata)
{
    int res;
    res = jparse_expect_kvp_array(
            jctx,
            &_queries,
            REF_JPARSE_ARRAY_ITERATOR(queries_array),
            jval,
            udata);
    //jparse_dump_value(jval);
    return res;

}

DEF_JPARSE_OBJECT_ITERATOR(queries, queries_key);


static int
mycb400(jparse_ctx_t *jctx,
        UNUSED jparse_value_t *jval,
        void *udata)
{
    int res;
    jparse_value_t _jval;

    assert(jval == NULL);

    jparse_value_init(&_jval);

    res = jparse_expect_object(jctx,
                               REF_JPARSE_OBJECT_ITERATOR(queries),
                               &_jval,
                               udata);

    TR(res);
    return 0;
}


static void
test2(void)
{
    jparse_ctx_t *jctx;
    int res;

    struct {
        long rnd;
        const char *in;
        jparse_expect_cb_t cb;
    } data[] = {
        {0, "data/testjparse/mixed-01", mycb300},
        {0, "data/testjparse/mixed-02", mycb400},

    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        TRACE("in=%s", CDATA.in);
        jctx = jparse_ctx_new(4096, 4096);
        res = jparse_ctx_parse(jctx, CDATA.in, CDATA.cb, NULL, NULL);
        jparse_ctx_destroy(&jctx);
        assert(res == 0);
    }

}

int
main(void)
{
    //test0();
    test1();
    test2();
    return 0;
}

// vim:list
