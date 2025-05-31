#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/uio.h>

#include "diag.h"
#include <mncommon/util.h>
//#define TRRET_DEBUG
#include <mncommon/bytes.h>
#include <mncommon/dumpm.h>
#include <mncommon/vbytestream.h>


#define VBYTESTREAM_IOV_BYTES(iov)  \
    ((mnbytes_t *)((char *)((iov)->iov_base) - sizeof(mnbytes_t)))


static int
iovec_init(void *o)
{
    struct iovec *iov = o;
    iov->iov_base = NULL;
    iov->iov_len = 0;
    return 0;
}


static void
iovec_alloc(struct iovec *iov, size_t sz)
{
    mnbytes_t *s;

    assert(iov->iov_base == NULL && iov->iov_len == 0);

    s = bytes_new(sz);
    BYTES_INCREF(s);
    iov->iov_base = BDATA(s);
    iov->iov_len = sz;
}


static int
iovec_fini(void *o)
{
    struct iovec *iov = o;
    if (iov->iov_base != NULL) {
        mnbytes_t *s;

        s = VBYTESTREAM_IOV_BYTES(iov);
        BYTES_DECREF(&s);
        iov->iov_base = NULL;
        iov->iov_len = 0;
    }
    return 0;
}


static int
iovec_dump_full(struct iovec *iov, UNUSED void *udata)
{
    D16(iov->iov_base, iov->iov_len);
    return 0;
}


static int
iovec_dump_short(struct iovec *iov, UNUSED void *udata)
{
    TRACE("%p:%ld", iov->iov_base, iov->iov_len);
    return 0;
}


static struct iovec *
vbytestream_iovec_get_safe(mnvbytestream_t *stream, int idx)
{
    struct iovec *iov;

    if ((iov = array_get(&stream->iov, idx)) == NULL) {
        (void)array_ensure_len(&stream->iov, idx + 1, ARRAY_FLAG_SAVE);
        iov = array_get(&stream->iov, idx);
    }
    assert(iov != NULL);
    return iov;
}


void
vbytestream_init(mnvbytestream_t *stream, size_t growsz, size_t iovreserve)
{
    assert(growsz > 0);

    (void)array_init(&stream->iov,
                     sizeof(struct iovec),
                     0,
                     iovec_init,
                     iovec_fini);

    if (iovreserve > 0) {
        array_ensure_datasz(
            &stream->iov, iovreserve * stream->iov.elsz, 0);
    }

    stream->eod.idx = 0;
    stream->eod.offt = 0;
    stream->eod.total = 0;
    stream->pos.idx = 0;
    stream->pos.offt = 0;
    stream->pos.total = 0;
    stream->growsz = growsz;
    stream->read_more = NULL;
    stream->write = NULL;
    stream->udata = NULL;
}


void
vbytestream_fini(mnvbytestream_t *stream)
{
    array_fini(&stream->iov);
}


/**
 * Set the edge's iov_len to the eod's offset.  This makes sure the edge
 * does not contain excess data (beyond eod).  May not be used at all,
 * since this is taken care of inline.
 */
UNUSED static void
vbytestream_finalize_edge(mnvbytestream_t *stream)
{
    struct iovec *edge;

    edge = vbytestream_iovec_get_safe(stream, stream->eod.idx);

    if (edge->iov_base != NULL) {
        if ((ssize_t)edge->iov_len > stream->eod.offt) {
            edge->iov_len = stream->eod.offt;
        }
    }
}

ssize_t
vbytestream_read(mnvbytestream_t *stream, int fd, ssize_t sz)
{
    div_t niov;
    int needed, ffree;
    ssize_t nread = 0;

    assert(sz >= 0);

    niov = div(sz, stream->growsz);
    // number of full iovs needed to fit sz into
    needed = niov.quot + (niov.rem > 0 ? 1 : 0);

    if (needed == 0 || needed >= IOV_MAX) {
        return -1;
    }

    assert(stream->eod.idx <= (int)(ssize_t)stream->iov.elnum);

    // "first free" iov, the one next to the last used iov.
    ffree = stream->eod.idx + (stream->eod.offt > 0 ? 1 : 0);
    //TRACE("ffree=%d needed=%d", ffree, needed);

    if (needed > 0) {
        mnarray_iter_t it;
        struct iovec *iov;

        (void)array_ensure_len(&stream->iov, ffree + needed, ARRAY_FLAG_SAVE);

        // the one next to the last iov
        it.iter = stream->iov.elnum;
        for (iov = array_get_iter(&stream->iov, &it);
             iov != NULL;
             iov = array_next(&stream->iov, &it)) {

            iovec_alloc(iov, stream->growsz);
        }

        iov = array_get(&stream->iov, ffree);

        if ((nread = readv(fd, iov, needed)) > 0) {
            // number of full iovs read
            niov = div(nread, stream->growsz);
            stream->eod.idx += niov.quot;
            stream->eod.offt = niov.rem;
            stream->eod.total += nread;
            //TRACE("sz=%ld nread=%ld %d/%d", sz, nread, niov.quot, niov.rem);

            // finalize the last iov
            if (niov.rem > 0) {
                iov = array_get(&stream->iov, stream->eod.idx);
                assert(iov != NULL);
                iov->iov_len = niov.rem;
            }
        }
    }

    return nread;
}


ssize_t
vbytestream_write(mnvbytestream_t *stream, int fd)
{
    size_t iovcnt;
    int partial;
    ssize_t nwritten;

    partial = stream->eod.offt > 0 ? 1 : 0;
    iovcnt = stream->eod.idx + partial - stream->pos.idx;

    if (iovcnt == 0 || iovcnt >= IOV_MAX) {
        return -1;
    }

    if (partial == 1) {
        struct iovec *edge;

        /*
         * eod points at partially filled edge, close edge->iov_len
         * temporarily, to not let excess data out.
         */
        edge = array_get(&stream->iov, stream->eod.idx);
        assert(edge != NULL);
        edge->iov_len = stream->eod.offt;

        nwritten = writev(fd,
                          (struct iovec *)array_get(&stream->iov,
                              stream->pos.idx), iovcnt);

        /*
         * set edge->iov_len back to allow the remaining space to be used.
         */
        edge->iov_len = BSZ(VBYTESTREAM_IOV_BYTES(edge));


    } else {
        nwritten = writev(fd,
                          (struct iovec *)array_get(&stream->iov,
                              stream->pos.idx), iovcnt);
    }

    if (nwritten >= 0 /* >0 ? */) {
        stream->pos.idx += iovcnt - partial;
        stream->pos.offt = stream->eod.offt;
        stream->pos.total += nwritten;
    }
    return nwritten;
}


int PRINTFLIKE(3, 4)
vbytestream_nprintf(mnvbytestream_t *stream, ssize_t sz, const char *fmt, ...)
{
    int avail, nused;
    struct iovec *edge;
    va_list ap;

    edge = vbytestream_iovec_get_safe(stream, stream->eod.idx);

    if (edge->iov_base == NULL) {
        assert(stream->eod.offt == 0);
        iovec_alloc(edge, sz);
    } else {
        assert(stream->eod.offt > 0);
    }
    avail = edge->iov_len - stream->eod.offt;

    va_start(ap, fmt);
    nused = vsnprintf(edge->iov_base + stream->eod.offt, avail, fmt, ap);
    va_end(ap);

    if (nused >= avail) {
        if (stream->eod.offt == 0) {
            /*
             * we have used a fresh edge sz bytes long
             */
            assert(avail == sz);
            (void)iovec_fini(edge);
            TRRET(VBYTESTREAM_NPRINTF_NEEDMORE);
        } else {
            /*
             * we have tried to append to the available edge, with not
             * enough room
             */

            /*
             * finalize current edge
             */
            edge->iov_len = stream->eod.offt;

            ++stream->eod.idx;
            stream->eod.offt = 0;
            edge = vbytestream_iovec_get_safe(stream, stream->eod.idx);
            assert(edge->iov_base == NULL);
            iovec_alloc(edge, sz);

            va_start(ap, fmt);
            nused = vsnprintf(edge->iov_base, sz, fmt, ap);
            va_end(ap);
            if (nused >= sz) {
                (void)iovec_fini(edge);
                TRRET(VBYTESTREAM_NPRINTF_NEEDMORE);
            }
        }
    }
    stream->eod.offt += nused;
    stream->eod.total += nused;

    return nused;
}


int
vbytestream_cat(UNUSED mnvbytestream_t *stream,
                UNUSED size_t sz,
                UNUSED const char *data)
{
    return -1;
}


int
vbytestream_adopt(mnvbytestream_t *stream, mnbytes_t *data)
{
    int last, partial;
    struct iovec *edge;

    partial = stream->eod.offt > 0 ? 1 : 0;
    last = stream->eod.idx + partial;

    (void)array_ensure_len(&stream->iov, last + 1, ARRAY_FLAG_SAVE);

    if (MNUNLIKELY((edge = array_get(&stream->iov, last)) == NULL)) {
        FAIL("array_get");
    }

    assert(edge->iov_base == NULL && edge->iov_len == 0);

    edge->iov_base = BDATA(data);
    edge->iov_len = BSZ(data);
    BYTES_INCREF(data);

    ++stream->eod.idx;
    stream->eod.offt = 0;
    stream->eod.total += BSZ(data);

    return (int)BSZ(data);
}


void *
vbytestream_buf(mnvbytestream_t *stream, size_t sz)
{
    void *res;
    struct iovec *edge;

    edge = vbytestream_iovec_get_safe(stream, stream->eod.idx);

    if (edge->iov_base == NULL) {
        /*
         * fresh iov
         */
        iovec_alloc(edge, sz);
        res = edge->iov_base;
    } else {
        ssize_t avail;

        /*
         * see if there is room
         */
        assert(edge->iov_len > 0);
        avail = (ssize_t)edge->iov_len - stream->eod.offt;

        if (avail >= (ssize_t)sz) {
            res = (void *)((char *)(edge->iov_base) + stream->eod.offt);

        } else {
            /*
             * finalize current edge
             */
            edge->iov_len = stream->eod.offt;

            ++stream->eod.idx;
            stream->eod.offt = 0;
            edge = vbytestream_iovec_get_safe(stream, stream->eod.idx);
            assert(edge->iov_base == NULL);
            iovec_alloc(edge, sz);
            res = edge->iov_base;
        }
    }

    return res;
}


static int
_rewind(struct iovec *iov, UNUSED void *udata)
{
    if (iov->iov_base != NULL) {
        iov->iov_len = BSZ(VBYTESTREAM_IOV_BYTES(iov));
    } else {
        iov->iov_len = 0;
    }
    return 0;
}


void
vbytestream_rewind(mnvbytestream_t *stream)
{
    stream->eod.idx = 0;
    stream->eod.offt = 0;
    stream->eod.total = 0;
    stream->pos.idx = 0;
    stream->pos.offt = 0;
    stream->pos.total = 0;
    if (stream->iov.elnum > 0) {
        (void)array_traverse(&stream->iov, (array_traverser_t)_rewind, NULL);
    }
}


int
vbytestream_traverse(mnvbytestream_t *stream, array_traverser_t cb, void *udata)
{
    return array_traverse(&stream->iov, cb, udata);
}


void
vbytestream_dump(mnvbytestream_t *stream, int flags)
{
    TRACE("pos %d/%ld/%ld eod %d/%ld/%ld",
          stream->pos.idx,
          (long)stream->pos.offt,
          (long)stream->pos.total,
          stream->eod.idx,
          (long)stream->eod.offt,
          (long)stream->eod.total);

    if (flags & VBYTESTREAM_DUMP_FULL) {
        (void)vbytestream_traverse(stream,
                                   (array_traverser_t)iovec_dump_full,
                                   NULL);
    } else {
        (void)vbytestream_traverse(stream,
                                   (array_traverser_t)iovec_dump_short,
                                   NULL);
    }
}
