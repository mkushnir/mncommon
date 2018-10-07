#ifndef MRKCOMMON_BYTESTREAM_AUX_H
#define MRKCOMMON_BYTESTREAM_AUX_H

#include <mrkcommon/bytestream.h>
#include <mrkcommon/bytes.h>

#ifdef __cplusplus
extern "C" {
#endif

void bytestream_from_bytes(mnbytestream_t *, const mnbytes_t *);
void bytestream_from_mem(mnbytestream_t *, const char *, size_t);

#ifdef __cplusplus
}
#endif

#endif

