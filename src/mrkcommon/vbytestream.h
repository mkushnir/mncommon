#ifndef MRKCOMMON_VBYTESTREAM_H
#define MRKCOMMON_VBYTESTREAM_H

#include <stdarg.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/uio.h>

#include <mrkcommon/array.h>
#include <mrkcommon/bytes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _vbytestream {
    /*
     * array of mnbytes_t *
     */
    mnarray_t bytes;
    /*
     * array of struct iovec
     */
    mnarray_t iov;
    /* the current eod iovec */
    struct {
        int idx;
        off_t offt;
        off_t total;
    } eod;
    /* the current pos iovec */
    struct {
        int idx;
        off_t offt;
        off_t total;
    } pos;
    size_t growsz;
    ssize_t (*read_more)(struct _vbytestream *, int, ssize_t);
    ssize_t (*write)(struct _vbytestream *, int, size_t);
    void *udata;

} vbytestream_t;

void vbytestream_init(vbytestream_t *, size_t, size_t);
void vbytestream_fini(vbytestream_t *);
#define VBYTESTREAM_NPRINTF_ERROR (-129)    // 0xffffffffffffff7f
#define VBYTESTREAM_NPRINTF_NEEDMORE (-130) // 0xffffffffffffff7e
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

