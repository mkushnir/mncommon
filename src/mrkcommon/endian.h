#ifndef MRKCOMMON_ENDIAN_H
#define MRKCOMMON_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#ifdef HAVE_ENDIAN_H
#   include <endian.h>
#else
#   ifdef HAVE_SYS_ENDIAN_H
#       include <sys/endian.h>
#   else
#       error "Neither endian.h nor sys/endian.h found"
#   endif
#endif

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif

#ifndef htobe16
#define htobe16(x) bswap_16 (x)
#endif

#ifndef htole16
#define htole16(x) ((uint16_t)(x))
#endif

#ifndef be16toh
#define be16toh(x) bswap_16 (x)
#endif

#ifndef le16toh
#define le16toh(x) ((uint16_t)(x))
#endif

#ifndef htobe32
#define htobe32(x) bswap_32 (x)
#endif

#ifndef htole32
#define htole32(x) ((uint32_t)(x))
#endif

#ifndef be32toh
#define be32toh(x) bswap_32 (x)
#endif

#ifndef le32toh
#define le32toh(x) ((uint32_t)(x))
#endif

#ifndef htobe64
#define htobe64(x) bswap_64 (x)
#endif

#ifndef htole64
#define htole64(x) ((uint64_t)(x))
#endif

#ifndef be64toh
#define be64toh(x) bswap_64 (x)
#endif

#ifndef le64toh
#define le64toh(x) ((uint64_t)(x))
#endif


#ifdef __cplusplus
}
#endif

#endif
