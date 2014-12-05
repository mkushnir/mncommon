#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"
#include "diag.h"
#include <mrkcommon/bytes.h>
#include <mrkcommon/dumpm.h>

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static void
test0(void)
{
    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };

    bytes_t *b1;

    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }

    b1 = bytes_new_from_str("This is the test");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("This is the test%");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("This is the test%f");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("This is the test%30%gg");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("%gg");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("%g");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("      This is the test      ");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("%20%20%20%20This is the test      ");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);

    b1 = bytes_new_from_str("http://origin.contentabc.com/ads/design4/ads/bz_950x250_186890/950x250_bzxiktgxrk.swf?clickTag1=http%3A%2F%2Fpt.trafficjunky.net%2Fpt_click%3Fad_id%3D186890_148593_55225%26req%3D186890%26rot%3D148593%26zone%3D55225%26prod%3D49%26lp%3Dhttp%253A%252F%252Fenter.brazzersnetwork.com%252Ftrack%252FNTAwMDI4OjQ5Mjo0MQ%252F%253Ftour%253DTGP&clickTag2=http%3A%2F%2Fpt.trafficjunky.net%2Fpt_click%3Fad_id%3D186890_148593_55226%26req%3D186890%26rot%3D148593%26zone%3D55226%26prod%3D1%26lp%3Dhttp%253A%252F%252Fenter.iknowthatgirl.com%252Ftrack%252FNDAwNTU0NC40Ni4xNC41Ni4yMTEuMC4wLjAuMA&clickTag3=http%3A%2F%2Fpt.trafficjunky.net%2Fpt_click%3Fad_id%3D186890_148593_55227%26req%3D186890%26rot%3D148593%26zone%3D55227%26prod%3D211%26lp%3Dhttp%253A%252F%252Fwww.rk.com%252F4%252Fmain.htm%253Fthumbs%253Dmedium%2526id%253Dypbidhouse%2526cmp%253D186890_YP_FTR_RK_TGP_BID_ALL&s=1393937314&e=1393944514&h=977835cd270b4a5bd639f6e5ba7b9e0e");
    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);

    bytes_brushdown(b1);
    TRACE("brushdown '%s'", b1->data);
    //D8(b1->data, b1->sz);
}


static void
test1(void)
{
    unsigned i;
    bytes_t *b1, *b2;
    const char *s[] = {
        "This is the test",
        "This is the test\\",
        "\\\\\\\\\\",
        "\\\b\f\n\r\t\"'''\"",
    };

    for (i = 0; i < countof(s); ++i) {
        b1 = bytes_new_from_str(s[i]);
        b2 = bytes_json_escape(b1);

        TRACE("b1='%s' b2='%s'", b1->data, b2->data);
        D8(b1->data, b1->sz);
        D8(b2->data, b2->sz);
    }

}


static void
test2(void)
{
    struct {
        long rnd;
        const char *in1;
        const char *in2;
        int expected;
    } data[] = {
        {0, "This is the test", "This", 1},
        {0, "This is the test", "ThiS", 0},
        {0, "This is the test", "This is the test", 1},
        {0, "This is the test", "This is the test?", 0},
        {0, "This is the test", "", 1},
    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        TRACE("startswith '%s', '%s'   expected=%d", CDATA.in1, CDATA.in2, CDATA.expected);
        assert(bytes_startswith(bytes_new_from_str(CDATA.in1), bytes_new_from_str(CDATA.in2)) == CDATA.expected);
    }
}


static void
test3(void)
{
    struct {
        long rnd;
        const char *in1;
        const char *in2;
        int expected;
    } data[] = {
        {0, "This is the test", "test", 1},
        {0, "This is the test", "tesT", 0},
        {0, "This is the test", "This is the test", 1},
        {0, "This is the test", "This is the test?", 0},
        {0, "This is the test", "", 1},
    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        TRACE("startswith '%s', '%s'   expected=%d", CDATA.in1, CDATA.in2, CDATA.expected);
        assert(bytes_endswith(bytes_new_from_str(CDATA.in1), bytes_new_from_str(CDATA.in2)) == CDATA.expected);
    }
}


static void
test4(void)
{
    struct {
        long rnd;
        const char *in;
        int expected;
    } data[] = {
        {0, "This is the test", 1},
    };
    UNITTEST_PROLOG;

    FOREACHDATA {
        assert(bytes_is_ascii(bytes_new_from_str(CDATA.in)) == CDATA.expected)
    }
}


int
main(void)
{
    test0();
    test1();
    test2();
    test3();
    test4();
    return 0;
}
