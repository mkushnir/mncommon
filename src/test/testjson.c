#include <assert.h>
#include <fcntl.h>

#include "unittest.h"
#include "diag.h"
//#define TRRET_DEBUG_VERBOSE
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/json.h"

UNUSED static int
mycb(json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
ostartcb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
oendcb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
astartcb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
aendcb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
keycb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
valuecb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
itemcb(UNUSED json_ctx_t *ctx, UNUSED const char *in, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static void
test0(void)
{
    json_ctx_t ctx;
    UNUSED int res;

    struct {
        long rnd;
        const char *in;
    } data[] = {
        //{0, ""},
        //{0, "null"},
        //{0, "\"This is the \\ test\""},
        {0, "{\"\":123}"},
        {0, "{\"\"        : 123    }"},
        {0, "{\"\"        : 123    }"},
        {0, "{\"qwejh\":\"QWEQWE\"}"},
        {0, "   {   \"asd\"    :    \"QWEQWE\"    }    "},
        {0, "{\"asd\":-123123}"},
        {0, "   {   \"asd\"    :    -123123    }    "},
        {0, "{\"asd\":-123123.345}"},
        {0, "   {   \"asd\"    :    -123123.345    }    "},
        {0, "{\"asd\":-123123.345e2}"},
        {0, "   {   \"asd\"    :    -123123.345E-2    }    "},
        {0, "{\"asd\":true}"},
        {0, "   {   \"asd\"    :    true    }    "},
        {0, "{\"asd\":false}"},
        {0, "   {   \"asd\"    :    false    }    "},
        {0, "{\"asd\":null}"},
        {0, "   {   \"asd\"    :    null    }    "},
        {0, "{\"outer\":{\"inner\":\"123\"}}"},
        {0, "   {  \"outer\"  :   {   \"inner\"   :   \"123\"   }   }   "},
        {0, "[1,2,3,4]"},
        {0, " [  1  ,   2   ,   3   ,   4   ]   "},
        {0, "[1,\"two\",[\"three\",33]]"},
        {0, "{\"test\":[1,\"two\",[\"three\",33]]}"},
        {0, "   {   \"test\"   :   [   1   ,   \"two\"   ,   [   \"three\"   ,   33   ]   ]   }   "},
        {0, "[{\"one\":1},{\"NULL\":null}, true,false,null]"},
    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        TRACE("in=%s", CDATA.in);
        json_init(&ctx, mycb, NULL);
        res = json_parse(&ctx, strdup(CDATA.in), strlen(CDATA.in) + 1);
        //TRACE("res=%s", diag_str(res));
        json_fini(&ctx);
        assert(res == 0);
    }

}

UNUSED static void
test1(void)
{
    json_ctx_t ctx;
    int res;
    int f;
    char buf[4096];
    ssize_t nread;

    if ((f = open("data-01", O_RDONLY)) < 0) {
        assert(0);
    }
    if ((nread = read(f, buf, 4096)) < 0) {
        assert(0);
    }
    buf[nread] = '\0';

    close(f);

    json_init(&ctx, NULL, NULL);
    json_set_ostart_cb(&ctx, ostartcb, NULL);
    json_set_oend_cb(&ctx, oendcb, NULL);
    json_set_astart_cb(&ctx, astartcb, NULL);
    json_set_aend_cb(&ctx, aendcb, NULL);
    json_set_key_cb(&ctx, keycb, NULL);
    json_set_value_cb(&ctx, valuecb, NULL);
    json_set_item_cb(&ctx, itemcb, NULL);
    res = json_parse(&ctx, buf, nread + 1);
    TRACE("res=%s", diag_str(res));
    json_fini(&ctx);
}

int
main(void)
{
    test0();
    //test1();
    return 0;
}

// vim:list
