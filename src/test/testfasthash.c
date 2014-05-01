#include <assert.h>
#include <stdlib.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <time.h>

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
test0(const char *fname)
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

        hash = fasthash(0, (unsigned char *)buf, sz);
        printf("%016lx %s\n", hash, buf);
        if ((trn = trie_find_exact(&tr, hash)) != NULL) {
            TRACE("collision %016lx %s %s", hash, buf, (char *)(trn->value));
            free(buf);
        } else {
            trn = trie_add_node(&tr, hash);
            trn->value = buf;
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
    if (argc >= 2) {
        fname = argv[1];
    }
    test0(fname);
    return 0;
}
