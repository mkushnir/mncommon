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
o1(UNUSED jparse_ctx_t *jctx)
{
    return 0;
}

/*
 * array
 */
static int
aempty(jparse_ctx_t *jctx)
{
    int res;
    long v = 0;

    res = jparse_expect_item_int(jctx, &v);
    TRACE("res=%s", mrkcommon_diag_str(res));
    return 0;
}


static int
aint(jparse_ctx_t *jctx)
{
    int res, i;
    long v = 0;

    res = 0;
    i = 0;
    while (res == 0) {
        res = jparse_expect_item_int(jctx, &v);
        TRACE("res=%s val[%d]=%ld", mrkcommon_diag_str(res), i++, v);
    }
    return 0;
}


static int
aarray(jparse_ctx_t *jctx)
{
    int res, i;

    res = 0;
    i = 0;
    while (res == 0) {
        res = jparse_expect_item_array(jctx, aint);
        TRACE("res=%s [%d]", mrkcommon_diag_str(res), i++);
    }
    return 0;
}


static int
mycb10(jparse_ctx_t *jctx)
{
    return jparse_expect_array(jctx, aempty);
}


static int
mycb11(jparse_ctx_t *jctx)
{
    return jparse_expect_array(jctx, aint);
}


static int
mycb12(jparse_ctx_t *jctx)
{
    return jparse_expect_array(jctx, aarray);
}


/*
 * object
 */
static int
mycb1(jparse_ctx_t *jctx)
{
    int res;

    res = jparse_expect_object(jctx, o1);
    //TRACE("res=%s", mrkcommon_diag_str(res));
    return 0;
}

static int
mycb2(jparse_ctx_t *jctx)
{
    int res;

    res = jparse_expect_object(jctx, o1);
    //TRACE("res=%s", mrkcommon_diag_str(res));
    res = jparse_expect_object(jctx, o1);
    //TRACE("res=%s", mrkcommon_diag_str(res));
    return 0;
}


static int
oint(jparse_ctx_t *jctx)
{
    int res;
    bytes_t *key;
    jparse_value_t val = {.v.i = 0};

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_int(jctx, &key, &val.v.i);
    bytes_decref(&key);
    TRACE("abc=%ld", val.v.i);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_int(jctx, &key, &val.v.i);
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
mycb03(jparse_ctx_t *jctx)
{
    return jparse_expect_object(jctx, oint);
}


static int
ofloat(jparse_ctx_t *jctx)
{
    int res;
    bytes_t *key;
    jparse_value_t val = { .v.f = .0};

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_float(jctx, &key, &val.v.f);
    bytes_decref(&key);
    TRACE("abc=%lf", val.v.f);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_float(jctx, &key, &val.v.f);
    bytes_decref(&key);
    TRACE("qwe=%lf", val.v.f);

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
ostr(jparse_ctx_t *jctx)
{
    int res;
    bytes_t *key;
    jparse_value_t val = { .v.s = NULL};

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_str(jctx, &key, &val.v.s);
    bytes_decref(&key);
    TRACE("abc=%s", val.v.s != NULL ? (char *)val.v.s->data : "<null>");
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_str(jctx, &key, &val.v.s);
    bytes_decref(&key);
    TRACE("qwe=%s", val.v.s != NULL ? (char *)val.v.s->data : "<null>");

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
obool(jparse_ctx_t *jctx)
{
    int res;
    bytes_t *key;
    jparse_value_t val = { .v.b = 0 };

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_bool(jctx, &key, &val.v.b);
    bytes_decref(&key);
    TRACE("abc=%s", val.v.b ? "#t" : "#f");
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_bool(jctx, &key, &val.v.b);
    bytes_decref(&key);
    TRACE("qwe=%s", val.v.b ? "#t" : "#f");
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("asd");
    res = jparse_expect_kvp_bool(jctx, &key, &val.v.b);
    bytes_decref(&key);
    TRACE("asd=%s", val.v.b ? "#t" : "#f");

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
oarray(jparse_ctx_t *jctx)
{
    int res;
    bytes_t *key;

    key = bytes_new_from_str("abc");
    res = jparse_expect_kvp_array(jctx, &key, aint);
    bytes_decref(&key);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("qwe");
    res = jparse_expect_kvp_array(jctx, &key, aint);
    bytes_decref(&key);
    if (res != 0) {
        goto end;
    }
    key = bytes_new_from_str("asd");
    res = jparse_expect_kvp_array(jctx, &key, aint);
    bytes_decref(&key);

end:
    if (res == JPARSE_EOS) {
        res = 0;
    }
    return res;
}


static int
mycb04(jparse_ctx_t *jctx)
{
    return jparse_expect_object(jctx, ofloat);
}


static int
mycb05(jparse_ctx_t *jctx)
{
    return jparse_expect_object(jctx, ostr);
}


static int
mycb06(jparse_ctx_t *jctx)
{
    return jparse_expect_object(jctx, obool);
}


static int
mycb07(jparse_ctx_t *jctx)
{
    return jparse_expect_object(jctx, oarray);
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
        res = jparse_ctx_parse(jctx, CDATA.in, CDATA.cb, NULL);
        jparse_ctx_destroy(&jctx);
        assert(res == 0);
    }
}


int
main(void)
{
    test0();
    return 0;
}

// vim:list
