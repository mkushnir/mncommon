#ifndef MRKCOMMON_BYTESTREAM_H
#define MRKCOMMON_BYTESTREAM_H

#include <stdarg.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _byterange {
    off_t start;
    off_t end;
} byterange_t;

typedef struct _bytestream {
#ifdef DO_MEMDEBUG
    uint64_t mdtag;
#endif
    struct {
        char *data;
        ssize_t sz;
    } buf;
    ssize_t growsz;
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
#define SDPOS(stream, addr) (addr - (stream)->buf.data)
#define SPDATA(stream) ((stream)->buf.data + SPOS(stream))
#define SEOD(stream) (stream)->eod
#define SEDATA(stream) ((stream)->buf.data + SEOD(stream))
#define SSIZE(stream) (stream)->buf.sz
#define SGRSZ(stream) (stream)->growsz
#define SNEEDMORE(stream) (SPOS(stream) >= SEOD(stream))
#define SAVAIL(stream) (SEOD(stream) - SPOS(stream))
#define SADVANCEEOD(stream, n) (stream)->eod += (n)
#define SINCR(stream) ++((stream)->pos)
#define SDECR(stream) --((stream)->pos)
#define SADVANCEPOS(stream, n) (stream)->pos += (n)

int bytestream_init(bytestream_t *, ssize_t);
int bytestream_init_data(bytestream_t *, char *, ssize_t, ssize_t);
void bytestream_fini(bytestream_t *);

bytestream_t *bytestream_new(ssize_t);
bytestream_t *bytestream_copy(bytestream_t *);
void bytestream_destroy(bytestream_t **);

int bytestream_grow(bytestream_t *, size_t);

ssize_t bytestream_read_more(bytestream_t *, int, ssize_t);
ssize_t bytestream_recv_more(bytestream_t *, int, ssize_t);
ssize_t bytestream_write(bytestream_t *, int, size_t);
ssize_t bytestream_stderr_write(bytestream_t *, int, size_t);
ssize_t bytestream_send(bytestream_t *, int, size_t);

int bytestream_consume_data(bytestream_t *, int);
int bytestream_produce_data(bytestream_t *, int);
void bytestream_rewind(bytestream_t *);
off_t bytestream_recycle(bytestream_t *, int, off_t);
#define BYTESTREAM_NPRINTF_ERROR (-129)
#define BYTESTREAM_NPRINTF_NEEDNORE (-130)
int bytestream_nprintf(bytestream_t *, size_t, const char *, ...);
int bytestream_cat(bytestream_t *, size_t, const char *);
int bytestream_dump(bytestream_t *);

#ifdef __cplusplus
}
#endif

#endif

