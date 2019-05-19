#ifndef MRKCOMMON_BYTESTREAM_H
#define MRKCOMMON_BYTESTREAM_H

#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>

#include <mrkcommon/util.h>

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
    ssize_t (*read_more)(struct _bytestream *, void *, ssize_t);
    ssize_t (*write)(struct _bytestream *, void *, size_t);
    void *udata;

} mnbytestream_t;

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

int bytestream_init(mnbytestream_t *, ssize_t);
int bytestream_init_data(mnbytestream_t *, char *, ssize_t, ssize_t);
void bytestream_fini(mnbytestream_t *);

mnbytestream_t *bytestream_new(ssize_t);
mnbytestream_t *bytestream_copy(mnbytestream_t *);
void bytestream_destroy(mnbytestream_t **);

int bytestream_grow(mnbytestream_t *, size_t);
int bytestream_ensure_buf(mnbytestream_t *, ssize_t);

ssize_t bytestream_read_more(mnbytestream_t *, void *, ssize_t);
ssize_t bytestream_recv_more(mnbytestream_t *, void *, ssize_t);
ssize_t bytestream_write(mnbytestream_t *, void *, size_t);
ssize_t bytestream_stderr_write(mnbytestream_t *, void *, size_t);
ssize_t bytestream_send(mnbytestream_t *, void *, size_t);

int bytestream_consume_data(mnbytestream_t *, void *);
int bytestream_produce_data(mnbytestream_t *, void *);
void bytestream_rewind(mnbytestream_t *);
void bytestream_zero(mnbytestream_t *);
off_t bytestream_recycle(mnbytestream_t *, int, off_t);
#define BYTESTREAM_NPRINTF_ERROR (-129)
#define BYTESTREAM_NPRINTF_NEEDMORE (-130)
int PRINTFLIKE(3, 4) bytestream_nprintf(mnbytestream_t *, size_t, const char *, ...);
int bytestream_vnprintf(mnbytestream_t *, size_t, const char *, va_list);
#define BYTESTREAM_CAT_ERROR (-127)
int bytestream_cat(mnbytestream_t *, size_t, const char *);

#define SCATC(bs, c)                                   \
do {                                                   \
    if ((bs)->eod >= (bs)->buf.sz) {                   \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    *((bs)->buf.data + (bs)->eod) = c;                 \
    ++(bs)->eod;                                       \
} while (0)                                            \


#define SCATI(bs, ty, _i)                              \
do {                                                   \
    union {                                            \
        char *c;                                       \
        ty *i;                                         \
    } _scati_u;                                        \
    while (((bs)->eod + (ssize_t)sizeof(ty)) >         \
            (bs)->buf.sz) {                            \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    _scati_u.c = (bs)->buf.data + (bs)->eod;           \
    *_scati_u.i = _i;                                  \
    (bs)->eod += sizeof(ty);                           \
} while (0)                                            \


#define SCATI16(bs, _i)                                \
do {                                                   \
    union {                                            \
        char *c;                                       \
        int16_t *i;                                    \
    } _scat16_u;                                       \
    while (((bs)->eod + (ssize_t)sizeof(int16_t)) >    \
            (bs)->buf.sz) {                            \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    _scat16_u.c = (bs)->buf.data + (bs)->eod;          \
    *_scat16_u.i = _i;                                 \
    (bs)->eod += sizeof(int16_t);                      \
} while (0)                                            \


#define SCATI32(bs, _i)                                \
do {                                                   \
    union {                                            \
        char *c;                                       \
        int32_t *i;                                    \
    } _scat32_u;                                       \
    while (((bs)->eod + (ssize_t)sizeof(int32_t)) >    \
            (bs)->buf.sz) {                            \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    _scat32_u.c = (bs)->buf.data + (bs)->eod;          \
    *_scat32_u.i = _i;                                 \
    (bs)->eod += sizeof(int32_t);                      \
} while (0)                                            \


#define SCATI64(bs, _i)                                \
do {                                                   \
    union {                                            \
        char *c;                                       \
        int64_t *i;                                    \
    } _scat64_u;                                       \
    while (((bs)->eod + (ssize_t)sizeof(int64_t)) >    \
            (bs)->buf.sz) {                            \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    _scat64_u.c = (bs)->buf.data + (bs)->eod;          \
    *_scat64_u.i = _i;                                 \
    (bs)->eod += sizeof(int64_t);                      \
} while (0)                                            \


#define SCATF(bs, _f)                                  \
do {                                                   \
    union {                                            \
        char *c;                                       \
        float *f;                                      \
    } _scat64_u;                                       \
    while (((bs)->eod + (ssize_t)sizeof(float)) >      \
            (bs)->buf.sz) {                            \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    _scat64_u.c = (bs)->buf.data + (bs)->eod;          \
    *_scat64_u.f = _f;                                 \
    (bs)->eod += sizeof(float);                        \
} while (0)                                            \


#define SCATD(bs, _d)                                  \
do {                                                   \
    union {                                            \
        char *c;                                       \
        double *d;                                     \
    } _scat64_u;                                       \
    while (((bs)->eod + (ssize_t)sizeof(double)) >     \
            (bs)->buf.sz) {                            \
        (void)bytestream_grow((bs), (bs)->growsz);     \
    }                                                  \
    _scat64_u.c = (bs)->buf.data + (bs)->eod;          \
    *_scat64_u.d = _d;                                 \
    (bs)->eod += sizeof(double);                       \
} while (0)                                            \


int bytestream_dump(mnbytestream_t *);

#ifdef __cplusplus
}
#endif

#endif

