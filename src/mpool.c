#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mrkcommon/mpool.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "diag.h"


void
mpool_ctx_reset(mpool_ctx_t *mpool)
{
    mpool->last_chunk = MAX(mpool->last_chunk, mpool->current_chunk);
    mpool->current_chunk = 0;
    mpool->current_pos = 0;
}


static void **
new_chunk(mpool_ctx_t *mpool, size_t sz)
{
    void **chunk;

    if (mpool->current_chunk >= mpool->last_chunk) {
        ++mpool->current_chunk;
        if (mpool->current_chunk * sizeof(void *) == mpool->arenasz) {
            mpool->arenasz *= 2;
            mpool->arena = realloc(mpool->arena, mpool->arenasz);
            assert(mpool->arena != NULL);
        }

        chunk = mpool->arena + mpool->current_chunk;
        *chunk = malloc(sz);
        assert(*chunk != NULL);
    } else {
        ++mpool->current_chunk;
        chunk = mpool->arena + mpool->current_chunk;
    }
    mpool->current_pos = 0;
    return chunk;
}

void *
mpool_malloc(mpool_ctx_t *mpool, size_t sz)
{
    size_t sz1;
    struct _mpool_item *res;

    sz = sz + 8 - (sz % 8);
    sz1 = sz + sizeof(struct _mpool_item);

    if (sz1 > mpool->chunksz) {
        void **chunk;

        /* new "big" chunk */
        chunk = new_chunk(mpool, sz1);
        res = (struct _mpool_item *)*chunk;
        res->sz = sz;

    } else {
        void **chunk;
        size_t nleft;

        nleft = mpool->chunksz - mpool->current_pos;

        if (nleft < sz1) {
            chunk = new_chunk(mpool, mpool->chunksz);
        } else {
            chunk = mpool->arena + mpool->current_chunk;
        }

        res = (struct _mpool_item *)(*chunk + mpool->current_pos);
        res->sz = sz;
        mpool->current_pos += sz1;
    }

    return res->data;
}

#define DATA_TO_MPOOL_ITEM(d) ((struct _mpool_item *)(((char *)(d)) - sizeof(size_t)))

void *
mpool_realloc(mpool_ctx_t *mpool, void *p, size_t sz)
{
    struct _mpool_item *mpi;

    mpi = DATA_TO_MPOOL_ITEM(p);
    if (mpi->sz < sz) {
        void *pp;

        pp = mpool_malloc(mpool, sz);
        memcpy(pp, p, mpi->sz);
        p = pp;
    }
    return p;
}


void
mpool_free(UNUSED mpool_ctx_t *mpool, UNUSED void *o)
{
}

int
mpool_ctx_init(mpool_ctx_t *mpool, size_t chunksz)
{
    mpool->chunksz = chunksz;
    mpool->current_chunk = 0;
    mpool->last_chunk = 0;
    mpool->current_pos = 0;
    mpool->arenasz = sizeof(void *);
    mpool->arena = malloc(mpool->arenasz);
    assert(mpool->arena != NULL);
    mpool->arena[0] = malloc(mpool->chunksz);
    assert(mpool->arena[0] != NULL);
    return 0;
}


int
mpool_ctx_fini(mpool_ctx_t *mpool)
{
    int i;

    mpool->last_chunk = MAX(mpool->last_chunk, mpool->current_chunk);

    for (i = mpool->last_chunk; i >= 0; --i) {
        void *chunk;
        chunk = mpool->arena[i];
        assert(chunk != NULL);
        free(chunk);
    }
    free(mpool->arena);
    mpool->arena = NULL;
    return 0;
}

