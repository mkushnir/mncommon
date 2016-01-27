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
vbiovec_init(vbiovec_t *v)
{
    v->iov.iov_base = NULL;
    v->iov.iov_len = 0;
    v->sz = 0;
    return 0;
}


static int
vbiovec_fini(vbiovec_t *v)
{
    if (v->iov.iov_base != NULL) {
        free(v->iov.iov_base);
        v->iov.iov_base = NULL;
        v->iov.iov_len = 0;
        v->sz = 0;
    }
    return 0;
}


static int
vbiovec_dump(vbiovec_t *iov, void *udata)
{
    int *flags = udata;

    if (*flags & VBYTESTREAM_DUMP_FULL) {
        D8(iov->iov.iov_base, iov->sz);
    } else {
        D16(iov->iov.iov_base, iov->iov.iov_len);
    }
    return 0;
}


void
vbytestream_init(vbytestream_t *stream, size_t growsz, size_t iovreserve)
{
    assert(growsz > 0);

    (void)array_init(&stream->iov,
                     sizeof(vbiovec_t),
                     0,
                     (array_initializer_t)vbiovec_init,
                     (array_finalizer_t)vbiovec_fini);

    if (iovreserve > 0) {
        array_ensure_datasz(&stream->iov, iovreserve * stream->iov.elsz, 0);
    }

    stream->ioveod.idx = 0;
    stream->ioveod.offt = 0;
    stream->eod = 0;
    stream->iovpos.idx = 0;
    stream->iovpos.offt = 0;
    stream->pos = 0;
    stream->growsz = growsz;
    stream->read_more = NULL;
    stream->write = NULL;
    stream->udata = NULL;
}


void
vbytestream_fini(vbytestream_t *stream)
{
    array_fini(&stream->iov);
}


ssize_t
vbytestream_read(UNUSED vbytestream_t *stream, UNUSED int fd, UNUSED ssize_t sz)
{
    return -1;
}


ssize_t
vbytestream_write(vbytestream_t *stream, int fd)
{
    array_iter_t it;
    vbiovec_t *iov;
    size_t iovcnt;
    ssize_t nwritten, sz;

    iovcnt = stream->iov.elnum - stream->iovpos.idx;

    if (iovcnt == 0) {
        return 0;
    }

    nwritten = writev(fd,
                      (struct iovec *)array_get(&stream->iov,
                          stream->iovpos.idx), iovcnt);

    if (nwritten < 0) {
        return nwritten;
    }
    /*
     *
     */
    iovcnt = 0;
    sz = nwritten;
    it.iter = stream->iovpos.idx;
    for (iov = array_get_iter(&stream->iov, &it);
         iov != NULL;
         iov = array_next(&stream->iov, &it)) {
        if (sz >= (ssize_t)iov->iov.iov_len) {
            ++iovcnt;
            sz -= iov->iov.iov_len;
        } else {
            /* sz is residual */
            break;
        }
    }
    stream->iovpos.idx += iovcnt;
    stream->iovpos.offt = sz;
    stream->pos += nwritten;
    return nwritten;
}


static vbiovec_t *
new_iovec(vbytestream_t *stream, ssize_t sz)
{
    vbiovec_t *iov;

    if ((iov = array_incr(&stream->iov)) == NULL) {
        FAIL("array_incr");
    }
    iov->sz = MAX((size_t)sz, stream->growsz);
    if ((iov->iov.iov_base = malloc(iov->sz)) == NULL) {
        FAIL("malloc");
    }
    stream->ioveod.offt = 0;
    stream->ioveod.idx = stream->iov.elnum - 1;
    return iov;
}

int PRINTFLIKE(3, 4)
vbytestream_nprintf(vbytestream_t *stream,
                    ssize_t sz,
                    const char *fmt, ...)
{
    vbiovec_t *iov;
    int nused;
    va_list ap;

    if ((iov = array_get(&stream->iov, stream->ioveod.idx)) == NULL) {
        iov = new_iovec(stream, sz);
    } else {
        ssize_t need;

        need = sz - (ssize_t)iov->sz + stream->ioveod.offt;
        if (need > 0) {
            iov = new_iovec(stream, sz);
        }
    }
    va_start(ap, fmt);
    nused = vsnprintf(iov->iov.iov_base + stream->ioveod.offt,
                      iov->sz - stream->ioveod.offt, fmt, ap);
    va_end(ap);
    if (nused >= ((ssize_t)(iov->sz - stream->ioveod.offt))) {
        TRRET(VBYTESTREAM_NPRINTF_NEEDMORE);
    } else {
        iov->iov.iov_len += nused;
    }
    stream->ioveod.offt += nused;
    stream->eod += nused;
    return nused;
}


int
vbytestream_cat(UNUSED vbytestream_t *stream, UNUSED size_t sz, UNUSED const char *data)
{
    return -1;
}


static int
_rewind(vbiovec_t *iov)
{
    iov->iov.iov_len = 0;
    return 0;
}


void
vbytestream_rewind(vbytestream_t *stream)
{
    stream->ioveod.idx = 0;
    stream->ioveod.offt = 0;
    stream->eod = 0;
    stream->iovpos.idx = 0;
    stream->iovpos.offt = 0;
    stream->pos = 0;
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
    (void)vbytestream_traverse(stream, (array_traverser_t)vbiovec_dump, &flags);
}

