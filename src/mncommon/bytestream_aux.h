#ifndef MNCOMMON_BYTESTREAM_AUX_H
#define MNCOMMON_BYTESTREAM_AUX_H

#include <mncommon/bytestream.h>
#include <mncommon/bytes.h>

#ifdef __cplusplus
extern "C" {
#endif

void bytestream_from_bytes(mnbytestream_t *, const mnbytes_t *);
void bytestream_from_mem(mnbytestream_t *, const char *, size_t);

int bytestream_consume_lines (
    mnbytestream_t *,
    int,
    int (*) (const mnbytestream_t *, const byterange_t *, void *),
    void *);

#ifdef __cplusplus
}
#endif

#endif

