#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "diag.h"
#include "mrkcommon/util.h"
//#define TRRET_DEBUG
#include "mrkcommon/dumpm.h"
#include "mrkcommon/bytestream.h"
#include <mrkcommon/mpool.h>

int
bytestream_dump(bytestream_t *stream)
{
    TRACE("stream pos=%ld eod=%ld sz=%ld",
          stream->pos, stream->eod, stream->buf.sz);
    TRACE("stream start");
    D16(stream->buf.data, MIN(stream->eod, 128));
    return (0);
}


#define BYTESTREAM_INIT_BODY(mallocfn) \
    stream->buf.sz = growsz; \
    stream->growsz = growsz; \
    if ((stream->buf.data = mallocfn(stream->buf.sz)) == NULL) { \
        TRRET(BYTESTREAM_INIT + 1); \
    } \
 \
    stream->eod = 0; \
    stream->pos = 0; \
    stream->read_more = NULL; \
    stream->write = NULL; \
    stream->udata = NULL; \
    return (0);


int
bytestream_init(bytestream_t *stream, ssize_t growsz)
{
    BYTESTREAM_INIT_BODY(malloc);
}


int
bytestream_init_mpool(mpool_ctx_t *mpool, bytestream_t *stream, ssize_t growsz)
{
#define _malloc(sz) mpool_malloc(mpool, (sz))
    BYTESTREAM_INIT_BODY(_malloc);
#undef _malloc
}


#define BYTESTREAM_GROW_BODY(reallocfn) \
    char *tmp; \
    if ((tmp = reallocfn(stream->buf.data, \
                         stream->buf.sz + incr)) == NULL) { \
        TRRET(BYTESTREAM_GROW + 1); \
    } \
    stream->buf.data = tmp; \
    stream->buf.sz += incr; \
    return 0;

int
bytestream_grow(bytestream_t *stream, size_t incr)
{
    BYTESTREAM_GROW_BODY(realloc);
}


int
bytestream_grow_mpool(mpool_ctx_t *mpool, bytestream_t *stream, size_t incr)
{
#define _realloc(ptr, sz) mpool_realloc(mpool, (ptr), (sz))
    BYTESTREAM_GROW_BODY(_realloc);
#undef _realloc
}


#define BYTESTREAM_READ_MODE_BODY(growfn) \
    ssize_t nread; \
    ssize_t need; \
    need = (stream->eod + sz) - stream->buf.sz; \
    if (need > 0) { \
        if (growfn(stream, \
                   (need < stream->growsz) ? \
                   stream->growsz : \
                   need) != 0) { \
            return -1; \
        } \
    } \
    if ((nread = read(fd, stream->buf.data + stream->eod, sz)) >= 0) { \
        stream->eod += nread; \
    } \
    return nread;


ssize_t
bytestream_read_more(bytestream_t *stream, int fd, ssize_t sz)
{
    BYTESTREAM_READ_MODE_BODY(bytestream_grow);
}


ssize_t
bytestream_read_more_mpool(mpool_ctx_t *mpool,
                           bytestream_t *stream,
                           int fd,
                           ssize_t sz)
{
#define _bytestream_grow(bs, sz) bytestream_grow_mpool(mpool, (bs), (sz))
    BYTESTREAM_READ_MODE_BODY(_bytestream_grow);
#undef _bytestream_grow
}


#define BYTESTREAM_RECV_MODE_BODY(growfn) \
    ssize_t nrecv; \
    ssize_t need; \
    need = (stream->eod + sz) - stream->buf.sz; \
    if (need > 0) { \
        if (growfn(stream, \
                   (need < stream->growsz) ? \
                    stream->growsz : \
                    need) != 0) { \
            return -1; \
        } \
    } \
    if ((nrecv = recv(fd, \
                      stream->buf.data + stream->eod, \
                      (size_t)sz, \
                      0)) >= 0) { \
        stream->eod += nrecv; \
    } \
    return nrecv;


ssize_t
bytestream_recv_more(bytestream_t *stream, int fd, ssize_t sz)
{
    BYTESTREAM_RECV_MODE_BODY(bytestream_grow)
}


ssize_t
bytestream_recv_more_mpool(mpool_ctx_t *mpool,
                           bytestream_t *stream,
                           int fd,
                           ssize_t sz)
{
#define _bytestream_grow(bs, sz) bytestream_grow_mpool(mpool, (bs), (sz))
    BYTESTREAM_RECV_MODE_BODY(_bytestream_grow)
#undef _bytestream_grow
}


ssize_t
bytestream_write(bytestream_t *stream, int fd, size_t sz)
{
    ssize_t nwritten;

    if ((stream->pos + (ssize_t)sz) > stream->eod) {
        return (-1);

    }

    nwritten = write(fd, stream->buf.data + stream->pos, sz);
    stream->pos += nwritten;

    return (nwritten);
}

ssize_t
bytestream_stderr_write(bytestream_t *stream, int fd, size_t sz)
{
    ssize_t nwritten;

    fd = 2;

    if ((stream->pos + (ssize_t)sz) > stream->eod) {
        return (-1);

    }

    nwritten = write(fd, stream->buf.data + stream->pos, sz);
    stream->pos += nwritten;

    if (write(fd, "\n", 1)) {;}

    return (nwritten);
}

ssize_t
bytestream_send(bytestream_t *stream, int fd, size_t sz)
{
    ssize_t nwritten;

    if ((stream->pos + (ssize_t)sz) > stream->eod) {
        return (-1);

    }

    nwritten = send(fd, stream->buf.data + stream->pos, sz, 0);
    stream->pos += nwritten;

    return (nwritten);
}

int
bytestream_consume_data(bytestream_t *stream, int fd)
{
    ssize_t nread;
    ssize_t need;

    assert(stream->read_more != NULL);

    need = (stream->pos + stream->growsz) - stream->eod;
    nread = stream->read_more(stream, fd, need);

    if (nread == 0) {
        /* eof */
        TRRET(-1);

    } else if (nread < 0) {
        TRRET(BYTESTREAM_CONSUME_DATA + 2);
    }

    return (0);
}

int
bytestream_produce_data(bytestream_t *stream, int fd)
{
    assert(stream->write != NULL);

    if (stream->write(stream, fd, SEOD(stream) - SPOS(stream)) < 0) {
        TRRET(BYTESTREAM_PRODUCE_DATA + 1);
    }

    return (0);
}


#define BYTESTREAM_NPRINTF_BODY(growfn) \
    int nused; \
    ssize_t need; \
    va_list ap; \
    need = (stream->eod + sz) - stream->buf.sz; \
    if (need > 0) { \
        if (growfn(stream, \
                   (need < stream->growsz) ? \
                   stream->growsz : \
                   need) != 0) { \
            TRRET(BYTESTREAM_NPRINTF + 1); \
        } \
    } \
    va_start(ap, fmt); \
    nused = vsnprintf(SDATA(stream, stream->eod), sz, fmt, ap); \
    va_end(ap); \
    stream->eod += nused; \
    return 0;


int PRINTFLIKE(3, 4)
bytestream_nprintf(bytestream_t *stream,
                   size_t sz,
                   const char *fmt, ...)
{
    BYTESTREAM_NPRINTF_BODY(bytestream_grow);
}


int PRINTFLIKE(4, 5)
bytestream_nprintf_mpool(mpool_ctx_t *mpool,
                         bytestream_t *stream,
                         size_t sz,
                         const char *fmt, ...)
{
#define _bytestream_grow(bs, sz) bytestream_grow_mpool(mpool, (bs), (sz))
    BYTESTREAM_NPRINTF_BODY(_bytestream_grow);
#undef _bytestream_grow
}


#define BYTESTREAM_CAT_BODY(growfn) \
    ssize_t need; \
    need = (stream->eod + sz) - stream->buf.sz; \
    if (need > 0) { \
        if (growfn(stream, \
                   (need < stream->growsz) ? \
                   stream->growsz : \
                   need) != 0) { \
            TRRET(BYTESTREAM_CAT + 1); \
        } \
    } \
    memcpy(SDATA(stream, stream->eod), data, sz); \
    stream->eod += sz; \
    return 0;


int
bytestream_cat(bytestream_t *stream, size_t sz, const char *data)
{
    BYTESTREAM_CAT_BODY(bytestream_grow);
}


int
bytestream_cat_mpool(mpool_ctx_t *mpool,
                     bytestream_t *stream,
                     size_t sz,
                     const char *data)
{
#define _bytestream_grow(bs, sz) bytestream_grow_mpool(mpool, (bs), (sz))
    BYTESTREAM_CAT_BODY(_bytestream_grow);
#undef _bytestream_grow
}


void
bytestream_rewind(bytestream_t *stream)
{
    stream->pos = 0;
    stream->eod = 0;
}

off_t
bytestream_recycle(bytestream_t *stream, int ngrowsz, off_t from)
{
    int pgsz;

#ifdef PAGE_SIZE
    pgsz = PAGE_SIZE;
#else
    pgsz = getpagesize();
#endif
    from -= (from % pgsz);

    if (from > (stream->growsz * ngrowsz)) {
        memmove(stream->buf.data,
                stream->buf.data + from,
                stream->eod - from);
        stream->eod -= from;
        stream->pos -= from;
        return from;
    }

    return 0;
}


#define BYTESTREAM_FINI_BODY(freefn) \
    if (stream->buf.data != NULL) { \
        freefn(stream->buf.data); \
        stream->buf.data = NULL; \
    } \
    stream->read_more = NULL; \
    stream->write = NULL; \
    stream->udata = NULL;


void
bytestream_fini(bytestream_t *stream)
{
    BYTESTREAM_FINI_BODY(free);
}


void
bytestream_fini_mpool(mpool_ctx_t *mpool, bytestream_t *stream)
{
#define _free(ptr) mpool_free(mpool, (ptr))
    BYTESTREAM_FINI_BODY(_free);
#undef _free
}

