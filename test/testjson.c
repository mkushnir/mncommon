#include <assert.h>
#include <fcntl.h>

#include "unittest.h"
#include "diag.h"
//#define TRRET_DEBUG_VERBOSE
#include "mncommon/dumpm.h"
#include "mncommon/util.h"
#include "mncommon/json.h"

UNUSED static int
mycb(json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
ostartcb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
ostopcb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
astartcb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
astopcb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
keycb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
valuecb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
{
    json_dump(ctx);
    TRRET(0);
}

UNUSED static int
itemcb(UNUSED json_ctx_t *ctx, UNUSED void *udata)
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
        //TRACE("in=%s", CDATA.in);
        json_init(&ctx, mycb, NULL);
        res = json_parse(&ctx, strdup(CDATA.in), strlen(CDATA.in) + 1);
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
    json_set_ostop_cb(&ctx, ostopcb, NULL);
    json_set_astart_cb(&ctx, astartcb, NULL);
    json_set_astop_cb(&ctx, astopcb, NULL);
    json_set_key_cb(&ctx, keycb, NULL);
    json_set_value_cb(&ctx, valuecb, NULL);
    json_set_item_cb(&ctx, itemcb, NULL);
    res = json_parse(&ctx, buf, nread + 1);
    mndiag_local_str(res, buf, sizeof(buf));
    //TRACE("res=%s", buf);
    json_fini(&ctx);
}


#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-value"
#endif
//#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

UNUSED static void
test2 (void)
{
    JSON_NODE_DEF(n, NULL, NULL, 1, 12, NULL);
    JSON_NODE_DEF(nn, NULL, NULL, 1, 13, NULL, &n);
    D16(&n, sizeof(n));
    D16(&nn, sizeof(nn));
    TRACE("ty %d 0.ty %d", nn.ty, ((json_node_t **)(nn.c))[0]->ty);

    JSON_NODE_DEF(
        nnn,
        NULL,
        NULL,
        1,
        14,
        NULL,
        &JSON_NODE_INITIALIZER(NULL, NULL, 1, 120, NULL),
        &JSON_NODE_INITIALIZER(NULL, NULL, 1, 130, NULL),
        &JSON_NODE_INITIALIZER(
            NULL,
            NULL,
            1,
            15,
            NULL,
            &JSON_NODE_INITIALIZER(
                NULL,
                NULL,
                1,
                16,
                NULL,
                &JSON_NODE_INITIALIZER(NULL, NULL, 1, 20, NULL),
                &JSON_NODE_INITIALIZER(NULL, NULL, 1, 21, NULL),
                &JSON_NODE_INITIALIZER(NULL, NULL, 1, 22, NULL)
            )

        )
    );

    json_node_t nnnn = JSON_NODE_INITIALIZER(NULL, NULL, 1, 20, NULL, &JSON_NODE_INITIALIZER(NULL, NULL, 1, 200, NULL));


    json_node_t obdiff = JSON_NODE_OBJECT(
        "obdiff",
        NULL,
        1,
        &JSON_NODE_OITEM_STRING("e", NULL, 1, "e"),
        &JSON_NODE_OITEM_STRING("E", NULL, 1, "E"),
        &JSON_NODE_OITEM_STRING("s", NULL, 1,  "s"),
        &JSON_NODE_OITEM_INT("U", NULL, 1, "U"),
        &JSON_NODE_OITEM_INT("u", NULL, 1, "u"),
        &JSON_NODE_OITEM_ARRAY("a", NULL, 1, "a",
            &JSON_NODE_AITEM_ARRAY(
                "a.pxqty",
                NULL,
                1,
                &JSON_NODE_AITEM_STRING(NULL, NULL, 1)
            ),
        ),
        &JSON_NODE_OITEM_ARRAY("b", NULL, 1, "b",
            &JSON_NODE_AITEM_ARRAY(
                "b.pxqty",
                NULL,
                1,
                &JSON_NODE_AITEM_STRING(NULL, NULL, 1)
            ),
        )
    );

    json_node_t apiresp = JSON_NODE_OBJECT(
        "apiresp",
        NULL,
        1,
        &JSON_NODE_OITEM_INT("apiresp.id", NULL, 1, "id"),
        &JSON_NODE_OITEM_ANY("apiresp.result", NULL, 1, "result")
    );

    json_node_t root = JSON_NODE_ONEOF(
        NULL,
        NULL,
        1,
        &obdiff,
        &apiresp
    );

    D16(&nnn, sizeof(nnn));
    D16(&nnnn, sizeof(nnnn));
    D16(&obdiff, sizeof(obdiff));
    D16(&apiresp, sizeof(apiresp));
    D16(&root, sizeof(root));

    TRACE("ty %d 0.ty %d 1.ty %d 2.ty %d 2.0.ty %d 2.0.1.ty %d",
        nnn.ty,
        JSON_NODE_CHILD_REF(nnn, 0)->ty,
        JSON_NODE_CHILD_REF(nnn, 1)->ty,
        JSON_NODE_CHILD_REF(nnn, 2)->ty,
        JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(nnn, 2)), 0)->ty,
        JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(nnn, 2)), 0)), 1)->ty
    );

    TRACE("csz %zd 0.csz %zd 1.csz %zd 2.csz %zd 2.0.csz %zd 2.0.1.csz %zd",
        nnn.csz,
        JSON_NODE_CHILD_REF(nnn, 0)->csz,
        JSON_NODE_CHILD_REF(nnn, 1)->csz,
        JSON_NODE_CHILD_REF(nnn, 2)->csz,
        JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(nnn, 2)), 0)->csz,
        JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(nnn, 2)), 0)), 1)->csz
    );

    TRACE("ty %s 0.ty %s 1.ty %s 2.ty %s 2.0.ty %s 2.0.1.ty %s",
        json_type_hint_str(&nnn),
        json_type_hint_str(JSON_NODE_CHILD_REF(nnn, 0)),
        json_type_hint_str(JSON_NODE_CHILD_REF(nnn, 1)),
        json_type_hint_str(JSON_NODE_CHILD_REF(nnn, 2)),
        json_type_hint_str(JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(nnn, 2)), 0)),
        json_type_hint_str(JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(*(JSON_NODE_CHILD_REF(nnn, 2)), 0)), 1))
    );

    TRACE("0.5.0.0 %s",
        json_type_hint_str(
            JSON_NODE_CHILD_REF(
                *JSON_NODE_CHILD_REF(
                    *JSON_NODE_CHILD_REF(
                        *JSON_NODE_CHILD_REF(root, 0),
                        5),
                    0),
                0)
        )
    );

    TRACE("0.5.0 %s",
        json_type_hint_str(
            JSON_NODE_CHILD_REF(
                *JSON_NODE_CHILD_REF(
                    *JSON_NODE_CHILD_REF(root, 0),
                    5),
                0)
        )
    );

    TRACE("0.5 %s",
        json_type_hint_str(
            JSON_NODE_CHILD_REF(
                *JSON_NODE_CHILD_REF(root, 0),
                5)
        )
    );

    TRACE("0 %s",
        json_type_hint_str(
            JSON_NODE_CHILD_REF(root, 0)
        )
    );
}


int
main(void)
{
    //test0();
    //test1();
    test2();
    return 0;
}

// vim:list
