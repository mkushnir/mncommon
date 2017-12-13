#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <mrkcommon/array.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/bytestream.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/profile.h>

#include <mrkcommon/vbytestream.h>

#include "unittest.h"
#include "diag.h"


#define MNUNIT_PARAMETRIZE(name, body) \
    for (size_t _ ## name = 0;         \
         _ ## name < countof(name);    \
         ++_ ## name) {                \
        body                           \
    }                                  \


#define MNUNIT_ARG(name) name[_ ## name]


static const profile_t *vb_nprintf;
static const profile_t *bs_nprintf;
static const profile_t *vb_write;
static const profile_t *bs_write;

static mnbytes_t *
randomword(size_t sz)
{
    mnbytes_t *res;
    unsigned i;

    res = bytes_new(sz + 1);
    for (i = 0; i < sz; ++i) {
        BDATA(res)[i] = random() % 26 + 'a';
    }
    BDATA(res)[sz] = '\0';

    return res;
}


UNUSED static void
test0(void)
{
    int i;
    mnvbytestream_t bs;

    vbytestream_init(&bs, 256, 0);

    for (i = 0; i < 40; ++i) {
        if (vbytestream_nprintf(&bs, 64, "%d ", i) < 0) {
            FAIL("vbytestream_nprintf");
        }
    }

    vbytestream_dump(&bs, /* 0 */ VBYTESTREAM_DUMP_FULL);

    vbytestream_write(&bs, 1);
    vbytestream_fini(&bs);


}


UNUSED static void
test1(void)
{
    int fd;
    mnvbytestream_t bs;
    ssize_t nwritten;
    const char *fname = "/tmp/mysmartctl-ada0";

    srandom(time(NULL));

    vbytestream_init(&bs, 4096, 0);
    if ((fd = open(fname, O_RDONLY)) < 0) {
        FAIL("open");
    }

    while (true) {
        ssize_t sz, nread;

        sz = random() % 1024;
        if (sz == 0) {
            continue;
        }
        if ((nread = vbytestream_read(&bs, fd, sz)) <= 0) {
            break;
        }
        //TRACE("sz=%ld nread=%ld", sz, nread);
    }

    lseek(fd, 0, SEEK_SET);
    (void)close(fd);

    //vbytestream_dump(&bs, 0);

    if ((fd = open("qwe", O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
        FAIL("open");
    }
    nwritten = vbytestream_write(&bs, fd);
    (void)close(fd);
    TRACE("pos %d/%ld/%ld eod %d/%ld/%ld",
          bs.pos.idx,
          bs.pos.offt,
          bs.pos.total,
          bs.eod.idx,
          bs.eod.offt,
          bs.eod.total);
    vbytestream_fini(&bs);
}


UNUSED static void
_test2(size_t growsz, int n)
{
    int fd;
    mnvbytestream_t bs;
    ssize_t nwritten;
    const char *fname = "/tmp/mysmartctl-ada0";
    char fnamebuf[1024];

    srandom(time(NULL));

    vbytestream_init(&bs, growsz, 0);
    if ((fd = open(fname, O_RDONLY)) < 0) {
        FAIL("open");
    }

    while (true) {
        ssize_t sz, nread;
        mnbytes_t *buf;

        sz = random() % n;
        if (sz == 0) {
            continue;
        }
        buf = bytes_new(sz);
        sz = read(fd, BDATA(buf), sz);
        if (sz <= 0) {
            break;
        }
        BSZ(buf) = sz;
        nread = vbytestream_adopt(&bs, buf);
        assert(nread > 0);
        //TRACE("sz=%ld nread=%ld", sz, nread);
    }

    //lseek(fd, 0, SEEK_SET);
    (void)close(fd);

    //vbytestream_dump(&bs, 0);

    (void)snprintf(fnamebuf, sizeof(fnamebuf), "qwe-%08lx-%08x", growsz, n);
    if ((fd = open(fnamebuf, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
        FAIL("open");
    }

    nwritten = vbytestream_write(&bs, fd);
    (void)close(fd);

    //TRACE("%s: nwritten %ld pos %d/%ld/%ld eod %d/%ld/%ld",
    //      fnamebuf,
    //      nwritten,
    //      bs.pos.idx,
    //      bs.pos.offt,
    //      bs.pos.total,
    //      bs.eod.idx,
    //      bs.eod.offt,
    //      bs.eod.total);

    vbytestream_fini(&bs);
}


UNUSED static void
test2(void)
{
    size_t growsz[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096 };
    int n[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096 };

    MNUNIT_PARAMETRIZE(growsz,
    MNUNIT_PARAMETRIZE(n,
        _test2(MNUNIT_ARG(growsz), MNUNIT_ARG(n));
    ));
}


static void
_test3(size_t growsz, int n)
{
    int fd;
    mnvbytestream_t bs0;
    mnbytestream_t bs1;
    char fnamebuf[1024];
    ssize_t nwritten;
    int i;

    srandom(time(NULL));

    vbytestream_init(&bs0, growsz, 0);
    bytestream_init(&bs1, 1024*1024);

    for (i = 0; i < n; ++i) {
        mnbytes_t *word;
        size_t sz;

        sz = random() % (growsz - 8) + 1;
        word = randomword(sz);
        //TRACE("growsz=%ld n=%d sz=%ld word=%s", growsz, i, sz, BDATA(word));
        if (vbytestream_nprintf(&bs0, growsz, ".%s\n", BDATA(word)) < 0) {
            FAIL("vbytestream_nprintf");
        }
        if (bytestream_nprintf(&bs1, growsz, ".%s\n", BDATA(word)) < 0) {
            FAIL("bytestream_nprintf");
        }

        BYTES_DECREF(&word);
    }

    (void)snprintf(fnamebuf, sizeof(fnamebuf), "vb-%08lx-%08x", growsz, n);
    if ((fd = open(fnamebuf, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
        FAIL("open");
    }
    nwritten = vbytestream_write(&bs0, fd);
    (void)close(fd);

    //vbytestream_dump(&bs0, VBYTESTREAM_DUMP_FULL /*  */ );

    (void)snprintf(fnamebuf, sizeof(fnamebuf), "bs-%08lx-%08x", growsz, n);
    if ((fd = open(fnamebuf, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
        FAIL("open");
    }
    nwritten = bytestream_write(&bs1, (void *)(intptr_t)fd, SEOD(&bs1));
    (void)close(fd);
    TRACE("growsz=%ld n=%d nwritten=%ld", growsz, n, nwritten);

    vbytestream_fini(&bs0);
    bytestream_fini(&bs1);
}


UNUSED static void
test3(void)
{
    size_t growsz[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096, };
    int n[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, };

    MNUNIT_PARAMETRIZE(growsz,
    MNUNIT_PARAMETRIZE(n,
        _test3(MNUNIT_ARG(growsz), MNUNIT_ARG(n));
    ));
}


static int
word_fini(mnbytes_t **s)
{
    BYTES_DECREF(s);
    return 0;
}


static void
_test4(size_t growsz, int n, size_t sz)
{
    int fd;
    mnvbytestream_t bs0;
    mnbytestream_t bs1;
    char fnamebuf[1024];
    ssize_t nwritten;
    mnarray_t words;
    int i;

    srandom(time(NULL));

    (void)array_init(&words,
                     sizeof(mnbytes_t *),
                     n,
                     NULL,
                     (array_finalizer_t)word_fini);

    for (i = 0; i < n; ++i) {
        mnbytes_t **word;
        size_t wsz;

        if (MRKUNLIKELY((word = array_get(&words, i)) == NULL)) {
            FAIL("array_incr");
        }
        wsz = random() % MIN(sz, (growsz - 8)) + 1;
        *word = randomword(wsz);
    }

    vbytestream_init(&bs0, growsz, 0);
    bytestream_init(&bs1, growsz);

    profile_start(vb_nprintf);
    for (i = 0; i < n; ++i) {
        mnbytes_t **word;

        if (MRKUNLIKELY((word = array_get(&words, i)) == NULL)) {
            FAIL("array_incr");
        }
        if (vbytestream_nprintf(&bs0, growsz, ".%s\n", BDATA(*word)) < 0) {
            FAIL("vbytestream_nprintf");
        }
    }
    (void)profile_stop(vb_nprintf);

    profile_start(bs_nprintf);
    for (i = 0; i < n; ++i) {
        mnbytes_t **word;

        if (MRKUNLIKELY((word = array_get(&words, i)) == NULL)) {
            FAIL("array_incr");
        }
        if (bytestream_nprintf(&bs1, growsz, ".%s\n", BDATA(*word)) < 0) {
            FAIL("bytestream_nprintf");
        }
    }
    (void)profile_stop(bs_nprintf);

    (void)snprintf(fnamebuf, sizeof(fnamebuf), "vb-%08lx-%08x-%08lx", growsz, n, sz);
    if ((fd = open(fnamebuf, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
        FAIL("open");
    }

    profile_start(vb_write);
    nwritten = vbytestream_write(&bs0, fd);
    (void)profile_stop(vb_write);
    (void)close(fd);

    (void)snprintf(fnamebuf, sizeof(fnamebuf), "bs-%08lx-%08x-%08lx", growsz, n, sz);
    if ((fd = open(fnamebuf, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
        FAIL("open");
    }

    profile_start(bs_write);
    nwritten = bytestream_write(&bs1, (void *)(intptr_t)fd, SEOD(&bs1));
    (void)profile_stop(bs_write);
    (void)close(fd);

    vbytestream_fini(&bs0);
    bytestream_fini(&bs1);

    (void)array_fini(&words);
}


UNUSED static void
test4(void)
{
    size_t growsz[] = { 32768, 65536, };
    int n[] = { 1024, 2048, 4096, };
    size_t sz[] = { 2048, 4096, 8192};

    profile_init_module();
    vb_nprintf = profile_register("vb_nprintf");
    bs_nprintf = profile_register("bs_nprintf");
    vb_write = profile_register("vb_write");
    bs_write = profile_register("bs_write");

    MNUNIT_PARAMETRIZE(growsz,
    MNUNIT_PARAMETRIZE(n,
    MNUNIT_PARAMETRIZE(sz,
        _test4(MNUNIT_ARG(growsz), MNUNIT_ARG(n), MNUNIT_ARG(sz));
    )));

    profile_report_sec();
    profile_fini_module();
}


int
main(void)
{
    //test0();
    //test1();
    //test2();
    //test3();
    //test4();
    return 0;
}

// vim:list
