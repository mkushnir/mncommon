#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mrkcommon/mpool.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "diag.h"

#ifdef DO_MEMDEBUG
#include <mrkcommon/memdebug.h>
MEMDEBUG_DECLARE(mpool);
#endif


//#define MPOOL_USE_STD_MALLOC

void
mpool_ctx_reset(mpool_ctx_t *mpool)
{
    mpool->last_chunk = MAX(mpool->last_chunk, mpool->current_chunk);
    mpool->current_chunk = 0;
    mpool->current_pos = 0;
}


#ifdef MPOOL_USE_STD_MALLOC
UNUSED
#endif
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
mpool_malloc(UNUSED mpool_ctx_t *mpool, size_t sz)
{
#ifdef MPOOL_USE_STD_MALLOC
    return malloc(sz);
#else
    int mod;
    size_t sz1;
    struct _mpool_item *res;

    mod = sz % 8;
    sz += mod ? (8 - mod) : 0;
    sz1 = sz + sizeof(struct _mpool_item);

    if (sz1 > mpool->chunksz) {
        void **chunk;

        /* new "big" chunk */
        chunk = new_chunk(mpool, sz1);
        res = (struct _mpool_item *)*chunk;
        res->sz = sz;
#ifndef NDEBUG
        res->flags = 1;
#endif
        (void)new_chunk(mpool, mpool->chunksz);

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
#ifndef NDEBUG

        struct _mpool_item *tmp;
        res->flags = 1;
        if (mpool->current_pos + sizeof(struct _mpool_item) <= mpool->chunksz) {
            tmp = (struct _mpool_item *)(*chunk + mpool->current_pos);
            tmp->flags = 0;
        }
#endif
    }
#ifndef NDEBUG
    memset(res->data, 0xa5, res->sz);
#endif
    //TRACE("m>>> %p sz=%ld in chunk %d pool %p",
    //      res->data,
    //      res->sz,
    //      mpool->current_chunk,
    //      mpool);
    return res->data;
#endif
}

#ifndef NDEBUG
#define DATA_TO_MPOOL_ITEM(d) ((struct _mpool_item *)(((char *)(d)) - sizeof(uint64_t) - sizeof(size_t)))
#else
#define DATA_TO_MPOOL_ITEM(d) ((struct _mpool_item *)(((char *)(d)) - sizeof(size_t)))
#endif

void *
mpool_realloc(UNUSED mpool_ctx_t *mpool, void *p, size_t sz)
{
#ifdef MPOOL_USE_STD_MALLOC
    return realloc(p, sz);
#else
    struct _mpool_item *mpi;

    if (p == NULL) {
        p = mpool_malloc(mpool, sz);
    } else {
        mpi = DATA_TO_MPOOL_ITEM(p);
        if (mpi->sz < sz) {
            void *pp;

            pp = mpool_malloc(mpool, sz);
            memcpy(pp, p, mpi->sz);
            p = pp;
        } else {
#ifndef NDEBUG
            memset(mpi->data + sz, 0xa5, mpi->sz - sz);
#endif
        }
    }
    return p;
#endif
}


void
mpool_free(UNUSED mpool_ctx_t *mpool, UNUSED void *o)
{
#ifdef MPOOL_USE_STD_MALLOC
    free(o);
#else
#ifndef NDEBUG
    if (o != NULL) {
        UNUSED struct _mpool_item *mpi;

        mpi = DATA_TO_MPOOL_ITEM(o);
        memset(mpi->data, 0x5a, mpi->sz);
        //TRACE("f<<< %p", o);
    }
#endif /* NDEBUG */
#endif /* MPOOL_USE_STD_MALLOC */
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


static void
mpool_ctx_chunk_dump_info(mpool_ctx_t *mpool)
{
    size_t i;

    for (i = 0; i < (mpool->arenasz / sizeof(void *)); ++i) {
        char *chunk;
        size_t j;

        TRACE(" chunk %ld", i);
        chunk = mpool->arena[i];

        for (j = 0; j < mpool->chunksz;) {
            struct _mpool_item *mpi;
            size_t sz0, sz1;

            mpi = (struct _mpool_item *)(chunk + j);
#ifndef NDEBUG
            if (mpi->flags == 0) {
                break;
            }
#endif
            TRACE("  item %p sz %ld", mpi, mpi->sz);
            sz0 = mpi->sz + 8 - (mpi->sz % 8);
            sz1 = sz0 + sizeof(struct _mpool_item);
            j += sz1;
        }

    }
}


void
mpool_ctx_dump_info(mpool_ctx_t *mpool)
{
    TRACE("%ld chunks of %ld bytes current %d/%ld last %d",
          mpool->arenasz / sizeof(void *),
          mpool->chunksz,
          mpool->current_chunk,
          mpool->current_pos,
          mpool->last_chunk);
    mpool_ctx_chunk_dump_info(mpool);
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

