#ifndef MRKCOMMON_VBYTESTREAM_H
#define MRKCOMMON_VBYTESTREAM_H

#include <stdarg.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/uio.h>

#include <mrkcommon/array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _vbiovec {
    struct iovec iov;
    size_t sz;
} vbiovec_t;

typedef struct _vbytestream {
    /*
     * array of vbiovec_t
     */
    array_t iov;
    /* the current eod iovec */
    struct {
        int idx;
        off_t offt;
    } ioveod;
    /* total offset of EOD */
    off_t eod;
    /* the current pos iovec */
    struct {
        int idx;
        off_t offt;
    } iovpos;
    /* total offset of POS */
    off_t pos;
    size_t growsz;
    ssize_t (*read_more)(struct _vbytestream *, int, ssize_t);
    ssize_t (*write)(struct _vbytestream *, int, size_t);
    void *udata;

} vbytestream_t;

#define VSDATA(stream, n) vbytestream_data(stream, n)


void vbytestream_init(vbytestream_t *, size_t, size_t);
void vbytestream_fini(vbytestream_t *);
#define VBYTESTREAM_NPRINTF_ERROR (-129)
#define VBYTESTREAM_NPRINTF_NEEDMORE (-130)
int vbytestream_nprintf(vbytestream_t *, ssize_t, const char *, ...);
int vbytestream_cat(vbytestream_t *, size_t, const char *);
int vbytestream_traverse(vbytestream_t *, array_traverser_t, void *);
ssize_t vbytestream_write(vbytestream_t *, int);
ssize_t vbytestream_read(vbytestream_t *, int, ssize_t);
void vbytestream_rewind(vbytestream_t *);
#define VBYTESTREAM_DUMP_FULL 0x01
void vbytestream_dump(vbytestream_t *, int);

#ifdef __cplusplus
}
#endif

#endif

