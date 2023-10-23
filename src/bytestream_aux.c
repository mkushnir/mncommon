#include <mncommon/bytestream.h>
#include <mncommon/bytes.h>
#include <mncommon/util.h>

static ssize_t
bytestream_read_more_bytes(mnbytestream_t *bs, UNUSED void * fd, ssize_t sz)
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


void
bytestream_from_mem(mnbytestream_t *bs, const char *s, size_t sz)
{
    bytestream_init(bs, 0);
    bs->read_more = bytestream_read_more_bytes;
    bs->buf.sz = sz;
    bs->buf.data = (char *)s;
}


int
bytestream_consume_lines(
        int fd,
        int (*linecb) (const mnbytestream_t *, const byterange_t *, void *),
        void *udata)
{
    int res = 0;
    mnbytestream_t bs;
    byterange_t line = {0, 0};

    (void)bytestream_init(&bs,
#ifdef PAGE_SIZE
        PAGE_SIZE
#else
        4096
#endif
    );
    bs.read_more = bytestream_read_more;

    line.start = SPOS(&bs);

    while (true) {
        if (SNEEDMORE(&bs)) {
            if (bytestream_consume_data(&bs, (void *)(intptr_t)fd) != 0) {
                break;
            }
        }

        //assert(SAVAIL(&bs) > 0);

        while (SAVAIL(&bs) > 0) {
            char ch;

            ch = *SPDATA(&bs);

            switch (ch) {
            case '\n':
                line.end = SPOS(&bs);

                if ((res = linecb(&bs, &line, udata)) != 0) {
                    goto end;
                }

                SADVANCEPOS(&bs, 1);
                line.start = SPOS(&bs);
                break;

            default:
                SADVANCEPOS(&bs, 1);
                break;
            }
        }

        (void)bytestream_recycle(&bs, 2, SPOS(&bs) - (SPOS(&bs) % bs.growsz));
    }

end:
    bytestream_fini(&bs);

    return res;
}
