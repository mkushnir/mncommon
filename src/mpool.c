#include <assert.h>
#include <stdlib.h>

#include <mrkcommon/mpool.h>
#include <mrkcommon/dumpm.h>

#include "diag.h"


void
mpool_ctx_reset(mpool_ctx_t *mpool)
{
    mpool->current_chunk = 0;
    mpool->current_pos = 0;
}


static char **
new_chunk(mpool_ctx_t *mpool)
{
    char **chunk;

    chunk = array_incr(&mpool->chunks);
    assert(chunk != NULL);
    *chunk = malloc(mpool->chunksz);
    assert(*chunk != NULL);
    ++mpool->current_chunk;
    mpool->current_pos = 0;
    return chunk;
}

void *
mpool_malloc(mpool_ctx_t *mpool, size_t sz)
{
    void *res;

    sz = sz + 8 - (sz % 8);
    assert(sz != 0);

    if (sz > mpool->chunksz) {
        char **chunk;

        /* new "big" chunk */
        chunk = array_incr(&mpool->chunks);
        assert(chunk != NULL);
        *chunk = malloc(sz);
        assert(*chunk != NULL);
        ++mpool->current_chunk;
        mpool->current_pos = mpool->chunksz;

        res = *chunk;

    } else {
        char **chunk;
        size_t nleft;

        nleft = mpool->chunksz - mpool->current_pos;
        if (nleft < sz) {
            chunk = new_chunk(mpool);
        } else {
            chunk = array_get(&mpool->chunks, mpool->current_chunk);
            assert(chunk != NULL);
        }
        res = (*chunk) + mpool->current_pos;
        mpool->current_pos += sz;
    }

    return res;
}


static int
chunk_fini(void **v)
{
    if (*v != NULL) {
        free(*v);
        *v = NULL;
    }
    return 0;
}


int
mpool_init_ctx(mpool_ctx_t *mpool, size_t chunksz)
{

    mpool->chunksz = chunksz;
    mpool->current_chunk = 0;
    mpool->current_pos = 0;
    if (array_init(&mpool->chunks,
                   sizeof(char *),
                   0,
                   NULL,
                   (array_finalizer_t)chunk_fini) != 0) {
        FAIL("array_init");
    }
    return 0;
}


int
mpool_ctx_fini(mpool_ctx_t *mpool)
{
    array_fini(&mpool->chunks);
    return 0;
}

