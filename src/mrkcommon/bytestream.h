#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include <stdarg.h>
#include <sys/types.h>

/*
 * This macro is also defined in snproto_bytes.h for a different structure.
 * Surprisingly, it appears to be exactly the same.
 *
 */
#define BDATA(b) ((b) != NULL ? (b)->data : NULL)

typedef struct _bytestream {
    struct {
        char *data;
        ssize_t sz;
    } buf;
    off_t eod;
    off_t pos;
    ssize_t (*read_more)(struct _bytestream *, int, ssize_t);
    ssize_t (*write)(struct _bytestream *, int, size_t);
    void *udata;

} bytestream_t;

#define SNCHR(stream, n) ((stream)->buf.data[(n)])
#define SPCHR(stream) SNCHR((stream), (stream)->pos)
#define SDATA(stream, n) ((stream)->buf.data + (n))
#define SPOS(stream) (stream)->pos
#define SEOD(stream) (stream)->eod
#define SSIZE(stream) (stream)->buf.sz
#define SNEEDMORE(stream) (SPOS(stream) >= SEOD(stream))
#define SAVAIL(stream) (SEOD(stream) - SPOS(stream))
#define SADVANCEEOD(stream, n) (stream)->eod += (n)
#define SINCR(stream) ++((stream)->pos)
#define SADVANCEPOS(stream, n) (stream)->pos += (n)

int bytestream_init(bytestream_t *);
void bytestream_fini(bytestream_t *);

int bytestream_grow(bytestream_t *, size_t);

ssize_t bytestream_read_more(bytestream_t *, int, ssize_t);
ssize_t bytestream_recv_more(bytestream_t *, int, ssize_t);
ssize_t bytestream_write(bytestream_t *, int, size_t);
ssize_t bytestream_stderr_write(bytestream_t *, int, size_t);
ssize_t bytestream_send(bytestream_t *, int, size_t);

int bytestream_consume_data(bytestream_t *, int);
int bytestream_produce_data(bytestream_t *, int);
void bytestream_rewind(bytestream_t *);
off_t bytestream_recycle(bytestream_t *, off_t);
int bytestream_nprintf(bytestream_t *, size_t, const char *, ...);
int bytestream_cat(bytestream_t *, size_t, const char *);
int bytestream_dump(bytestream_t *);
#endif

