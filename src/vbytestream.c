#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/uio.h>

#include "diag.h"
#include <mrkcommon/util.h>
//#define TRRET_DEBUG
#include <mrkcommon/dumpm.h>
#include <mrkcommon/vbytestream.h>


static int
bytes_init(bytes_t **b)
{
    *b = NULL;
    return 0;
}


static int
bytes_fini(bytes_t **b)
{
    BYTES_DECREF(b);
    return 0;
}


static int
iovec_init(struct iovec *v)
{
    v->iov_base = NULL;
    v->iov_len = 0;
    return 0;
}


static int
iovec_fini(struct iovec *v)
{
    if (v->iov_base != NULL) {
        free(v->iov_base);
        v->iov_base = NULL;
        v->iov_len = 0;
    }
    return 0;
}


static int
iovec_dump(struct iovec *iov, UNUSED void *udata)
{
    D16(iov->iov_base, iov->iov_len);
    return 0;
}


void
vbytestream_init(vbytestream_t *stream, size_t growsz, size_t iovreserve)
{
    assert(growsz > 0);

    (void)array_init(&stream->bytes,
                     sizeof(bytes_t *),
                     0,
                     (array_initializer_t)bytes_init,
                     (array_finalizer_t)bytes_fini);

    (void)array_init(&stream->iov,
                     sizeof(struct iovec),
                     0,
                     (array_initializer_t)iovec_init,
                     (array_finalizer_t)iovec_fini);

    if (iovreserve > 0) {
        array_ensure_datasz(&stream->bytes, iovreserve * stream->bytes.elsz, 0);
        array_ensure_datasz(&stream->iov, iovreserve * stream->iov.elsz, 0);
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
vbytestream_fini(vbytestream_t *stream)
{
    array_fini(&stream->iov);
    array_fini(&stream->bytes);
}


ssize_t
vbytestream_read(UNUSED vbytestream_t *stream, UNUSED int fd, UNUSED ssize_t sz)
{
    return -1;
}


ssize_t
vbytestream_write(vbytestream_t *stream, int fd)
{
    size_t iovcnt;
    ssize_t nwritten;

    iovcnt = stream->iov.elnum - stream->pos.idx;

    if (iovcnt == 0) {
        return 0;
    }

    nwritten = writev(fd,
                      (struct iovec *)array_get(&stream->iov,
                          stream->pos.idx), iovcnt);

    if (nwritten < 0) {
        return nwritten;
    }
    stream->pos.idx += iovcnt;
    stream->pos.total += nwritten;
    return nwritten;
}


int PRINTFLIKE(3, 4)
vbytestream_nprintf(UNUSED vbytestream_t *stream,
                    UNUSED ssize_t sz,
                    UNUSED const char *fmt, ...)
{
    return -1;
}


int
vbytestream_cat(UNUSED vbytestream_t *stream, UNUSED size_t sz, UNUSED const char *data)
{
    return -1;
}


static int
_rewind(struct iovec *iov)
{
    iov->iov_len = 0;
    return 0;
}


void
vbytestream_rewind(vbytestream_t *stream)
{
    stream->eod.idx = 0;
    stream->eod.offt = 0;
    stream->eod.total = 0;
    stream->pos.idx = 0;
    stream->pos.offt = 0;
    stream->pos.total = 0;
    (void)array_traverse(&stream->iov, (array_traverser_t)_rewind, NULL);
}


int
vbytestream_traverse(vbytestream_t *stream, array_traverser_t cb, void *udata)
{
    return array_traverse(&stream->iov, cb, udata);
}


void
vbytestream_dump(vbytestream_t *stream, int flags)
{
    (void)vbytestream_traverse(stream, (array_traverser_t)iovec_dump, &flags);
}

