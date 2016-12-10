#include <mrkcommon/bytestream.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/util.h>

static ssize_t
bytestream_read_more_bytes(mnbytestream_t *bs, UNUSED int fd, ssize_t sz)
{
    ssize_t over;
    ssize_t nleft;

    nleft = bs->buf.sz - bs->eod;

    if (nleft <= 0) {
        return -1;
    }
    sz = MIN(sz, nleft);
    over = (bs->eod + sz) - bs->buf.sz;
    if (over > 0) {
        return -1;
    }
    bs->eod += sz;
    return sz;
}


void
bytestream_from_bytes(mnbytestream_t *bs, const mnbytes_t *str)
{
    bytestream_init(bs, 0);
    bs->read_more = bytestream_read_more_bytes;
    bs->buf.sz = str->sz;
    bs->buf.data = (char *)str->data;
}
