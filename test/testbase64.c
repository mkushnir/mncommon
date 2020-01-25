#include <assert.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <string.h>

#include <mncommon/dumpm.h>
#include <mncommon/fasthash.h>
#include <mncommon/bytes.h>
#include <mncommon/hash.h>
#include <mncommon/util.h>

#include <mncommon/base64.h>

#include "diag.h"
#include "unittest.h"

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static void
test0(void)
{
    int i;
    char *ss[] = {
        "",
        "a",
        "aa",
        "aaa",
        "aaaa",
        "aaaaa",
        "aaaaaa",
        "aaaaaaa",
        "aaaaaaaa",
        "aaaaaaaaa",
    };

    for (i = 0; i < (int)countof(ss); ++i) {
        char buf[256];
        int res;

        memset(buf, '\0', sizeof(buf));
        res = mnbase64_encode_mime((unsigned char *)ss[i],
                               strlen(ss[i]),
                               buf,
                               sizeof(buf));

        TRACE("res=%d ss[%d] '%s' buf '%s'", res, i, ss[i], buf);
    }
}


static void
test1(void)
{
    int i;
    char *ss[] = {
        "",
        "a",
        "aa",
        "aaa",
        "aaaa",
        "aaaaa",
        "aaaaaa",
        "aaaaaaa",
        "aaaaaaaa",
        "aaaaaaaaa",
    };

    for (i = 0; i < (int)countof(ss); ++i) {
        int j;
        char buf[256];
        int res;

        for (j = 0; j < ((int)strlen(ss[i]) / 3 + 1) * 4 + 4; ++j) {
            memset(buf, '\0', sizeof(buf));
            res = mnbase64_encode_mime((unsigned char *)ss[i],
                                   strlen(ss[i]),
                                   buf,
                                   j);

            TRACE("res=%d ss[%d] '%s' buf(%d) '%s'", res, i, ss[i], j, buf);
        }
    }
}


static void
test2(void)
{
    mnbase64_test1();
}


static void
test3(void)
{
    int i;
    char *ss[] = {
        "",
        "a",
        "ab",
        "abc",
        "abcd",
        "abcde",
        "abcdef",
        "abcdefg",
        "abcdefgh",
        "abcdefghi",
    };

    for (i = 0; i < (int)countof(ss); ++i) {
        int j;
        char buf[256];
        int res;

        for (j = 0; j < ((int)strlen(ss[i]) / 3 + 1) * 4 + 4; ++j) {
            size_t sz;

            memset(buf, '\0', sizeof(buf));
            sz = strlen(ss[i]);
            res = mnbase64_encode_mime((unsigned char *)ss[i],
                                   sz,
                                   buf,
                                   j);
            if (res == 0) {
                UNUSED int res1;
                unsigned char src[256];
                size_t sz1;

                memset(src, '\0', sizeof(src));
                sz1 = sz + (3 - sz % 3);
                res1 = mnbase64_decode_mime(buf, strlen(buf), src, &sz1);
                assert(res1 == 0);
                assert(memcmp(ss[i], src, sz) == 0);
            }
        }
    }
}


static void
test4(void)
{
    int i;
    char *ss[] = {
        "",
        "a",
        "ab",
        "abc",
        "abcd",
        "abcde",
        "abcdef",
        "abcdefg",
        "abcdefgh",
        "abcdefghi",
        "abcdefghij",
        "abcdefghijk",
        "abcdefghijkl",
    };

    for (i = 0; i < (int)countof(ss); ++i) {
        int j;
        char buf[256];
        int res;

        for (j = 0; j < ((int)strlen(ss[i]) / 3 + 1) * 4 + 4; ++j) {
            size_t sz;

            memset(buf, '\0', sizeof(buf));
            sz = strlen(ss[i]);
            res = mnbase64_encode_mime((unsigned char *)ss[i],
                                   sz,
                                   buf,
                                   j);
            if (res == 0) {
                UNUSED int res1;
                size_t sz1;

                sz1 = strlen(buf);
                TRACE("buf=%s", buf);
                res1 = mnbase64_decode_mime_inplace(buf, &sz1);
                buf[sz1] = '\0';
                TRACE("buf=%s", buf);
                assert(res1 == 0);
                assert(memcmp(ss[i], buf, sz) == 0);
            }
        }
    }
}

int
main(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; ++i) {
        TRACE("arg=%s", argv[i]);
    }
    test0();
    test1();
    test2();
    test3();
    test4();
    return 0;
}
