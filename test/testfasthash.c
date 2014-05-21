#include <assert.h>
#include <stdlib.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/trie.h>
#include <mrkcommon/util.h>
#include <mrkcommon/fasthash.h>

#include "diag.h"
#include "unittest.h"

#include "zltan-fasthash.h"

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static uint64_t randmod = 0ul;

//#define SEED (17337407878196250699ul)
//#define SEED (0x2127599bf4325c37ULL)
//#define SEED (0x880355f21e6d1965ULL)
#define SEED (0)

//#define fasthash(seed, s, sz) fasthash64((const void *)(s), sz, seed)


static int
trie_node_fini(trie_node_t *trn, UNUSED uint64_t key, UNUSED void *udata)
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
        printf("%ld %ld\n", i, h);
    }
}

static void
test1(const char *fname)
{
    FILE *f;
    trie_t tr;
    char *buf;
    size_t bufsz;
    ssize_t sz;

    assert(fname != NULL);

    if ((f = fopen(fname, "r")) == NULL) {
        FAIL("fopen");
    }
    trie_init(&tr);

    for (buf = NULL;
         (sz = getline(&buf, &bufsz, f)) != -1;
         buf = NULL) {

        uint64_t hash;
        trie_node_t *trn;

        buf[sz - 1] = '\0';
        --sz;

        hash = fasthash(SEED, (unsigned char *)buf, sz);
        if (randmod) {
            printf("%ld\n", hash % randmod);
            free(buf);
        } else {
            printf("%016lx %s\n", hash, buf);
            if ((trn = trie_find_exact(&tr, hash)) != NULL) {
                TRACE("collision: %016lx\n%s\n%s\n", hash, buf, (char *)(trn->value));
                free(buf);
            } else {
                trn = trie_add_node(&tr, hash);
                trn->value = buf;
            }
        }
    }

    fclose(f);
    trie_traverse(&tr, trie_node_fini, NULL);
    trie_fini(&tr);

}

int
main(int argc, char **argv)
{
    char *fname = NULL;
    int ch;

    while ((ch = getopt(argc, argv, "r:")) != -1) {
        switch (ch) {
        case 'r':
            randmod = strtol(optarg, NULL, 10);
            break;
        default:
            break;
        }
    }

    argc -= (optind);
    argv += (optind);
    TRACE("optind=%d argc=%d", optind, argc);

    if (argc >= 2) {
        fname = argv[1];
    }

    if (fname == NULL) {
        test0();
    } else {
        test1(fname);
    }
    return 0;
}
