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
     * array of struct iovec
     */
    mnarray_t iov;
    /*
     * the current eod iovec:
     *  idx offt    eod iov
     *  --- ----    -------
     *  0   0       0-th iov, has no data
     *  0   n       0-th iov, has data up to n-1
     *  m   0       m-th iov, has no data (m-1-th iov has data)
     *  m   n       m-th iov, has data up to n-1
     */
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

} mnvbytestream_t;

void vbytestream_init(mnvbytestream_t *, size_t, size_t);
void vbytestream_fini(mnvbytestream_t *);
#define VBYTESTREAM_NPRINTF_ERROR (-129)    // 0xffffffffffffff7f
#define VBYTESTREAM_NPRINTF_NEEDMORE (-130) // 0xffffffffffffff7e
int vbytestream_nprintf(mnvbytestream_t *, ssize_t, const char *, ...);
int vbytestream_cat(mnvbytestream_t *, size_t, const char *);
int vbytestream_adopt(mnvbytestream_t *, mnbytes_t *);
int vbytestream_traverse(mnvbytestream_t *, array_traverser_t, void *);
ssize_t vbytestream_write(mnvbytestream_t *, int);
ssize_t vbytestream_read(mnvbytestream_t *, int, ssize_t);
void vbytestream_rewind(mnvbytestream_t *);
#define VBYTESTREAM_DUMP_FULL 0x01
void vbytestream_dump(mnvbytestream_t *, int);

#ifdef __cplusplus
}
#endif

#endif

