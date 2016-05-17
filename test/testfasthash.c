#include <assert.h>
#include <stdlib.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <zlib.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/btrie.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/util.h>
#include <mrkcommon/fasthash.h>

#include "diag.h"
#include "unittest.h"

#include "zltan-fasthash.h"

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static uint64_t randmod = 0ul;
static int genrandom = 0;

//#define SEED (17337407878196250699ul)
//#define SEED (0x2127599bf4325c37ULL)
//#define SEED (0x880355f21e6d1965ULL)
#define SEED (0)

//#define fasthash(seed, s, sz) fasthash64((const void *)(s), sz, seed)
//#define fasthash(seed, s, sz) (uint64_t)adler32((uLong)seed, (const Bytef *)(s), (uInt)sz)
#define fasthash(seed, s, sz) (uint64_t)crc32((uLong)seed, (const Bytef *)(s), (uInt)sz)


static int
btrie_node_fini(btrie_node_t *trn, UNUSED uint64_t key, UNUSED void *udata)
{
    if (trn->value != NULL) {
        free(trn->value);
        trn->value = NULL;
    }
    return 0;
}

static void
test0(void)
{
    uint64_t i;

    for (i = 0; i < (1024 * 1024); ++i) {
        uint64_t h;

        h = fasthash(SEED, (unsigned char *)&i, sizeof(i));
        if (randmod) {
            h %= randmod;
        }
        printf("%ld %ld\n", (long)i, (long)h);
    }
}

static void
test1(const char *fname)
{
    FILE *f;
    btrie_t tr;
    char *buf;
    size_t bufsz;
    ssize_t sz;

    assert(fname != NULL);

    if ((f = fopen(fname, "r")) == NULL) {
        FAIL("fopen");
    }
    btrie_init(&tr);

    for (buf = NULL;
         (sz = getline(&buf, &bufsz, f)) != -1;
         buf = NULL) {

        uint64_t hash;
        btrie_node_t *trn;

        buf[sz - 1] = '\0';
        --sz;

        hash = fasthash(SEED, (unsigned char *)buf, sz);
        if (randmod) {
            //printf("%ld\n", hash % randmod);
            free(buf);
        } else {
            //printf("%016lx %s\n", hash, buf);
            if ((trn = btrie_find_exact(&tr, hash)) != NULL) {
                TRACE("collision: %016lx\n%s\n%s\n", (long)hash, buf, (char *)(trn->value));
                free(buf);
            } else {
                trn = btrie_add_node(&tr, hash);
                trn->value = buf;
            }
        }
    }

    fclose(f);
    btrie_traverse(&tr, btrie_node_fini, NULL);
    btrie_fini(&tr);

}

static void
test_one_bytes(btrie_t *tr, bytes_t *s)
{
    uint64_t hash;
    btrie_node_t *trn;

    hash = fasthash(SEED, s->data, s->sz);
    if (randmod) {
        //printf("%ld\n", hash % randmod);
    } else {
        //printf("%016lx\n", hash);
        if ((trn = btrie_find_exact(tr, hash)) != NULL) {
            UNUSED bytes_t *ss;

            TRACE("collision: %016lx\n", (long)hash);
            //ss = trn->value;
            //D32(ss->data, ss->sz);
            //TRACE();
            //D32(s->data, s->sz);

        } else {
            trn = btrie_add_node(tr, hash);
            trn->value = s;
        }
    }
}

UNUSED static void
test_bytes(bytes_t **data, size_t sz)
{
    size_t i;
    btrie_t tr;

    btrie_init(&tr);

    for (i = 0; i < sz; ++i) {
        test_one_bytes(&tr, data[i]);
    }

    btrie_fini(&tr);
}


#define N 200000
#define MAXSZ 327680
#define MINSZ 64
static void
test2(void)
{
    btrie_t tr;
    int i;
    bytes_t *s;

    btrie_init(&tr);

    srandom(time(NULL));

    for (i = 0; i < N; ++i) {
        size_t sz, j;

        sz = (random() % (MAXSZ - MINSZ)) + MINSZ;
        s = bytes_new(sz);

        for (j = 0; j < sz; j += 4) {
            uint32_t *n;

            n = (uint32_t *)(s->data + j);
            *n = (uint32_t)random();
        }
        //D8(s->data, s->sz);

        test_one_bytes(&tr, s);

        bytes_decref(&s);
    }

    //TRACE("bytes are now ready");
    //test_bytes(data, N);
    //TRACE("test finished");

    //for (i = 0; i < N; ++i) {
    //    if (data[i] != NULL) {
    //        bytes_decref(&data[i]);
    //    }
    //}

    //free(data);

    btrie_fini(&tr);
}


int
main(int argc, char **argv)
{
    char *fname = NULL;
    int ch;

    while ((ch = getopt(argc, argv, "r:R")) != -1) {
        switch (ch) {
        case 'r':
            randmod = strtol(optarg, NULL, 10);
            break;

        case 'R':
            genrandom = 1;
            break;

        default:
            break;
        }
    }

    argc -= (optind);
    argv += (optind);
    TRACE("optind=%d argc=%d", optind, argc);

    if (argc >= 1) {
        fname = argv[0];
    }

    if (fname == NULL) {
        if (genrandom) {
            test2();
        } else {
            test0();
        }
    } else {
        test1(fname);
    }
    return 0;
}
