#include <assert.h>
#include <sys/types.h>
#include <mncommon/util.h>

#include <mncommon/base64.h>
#include <mncommon/dumpm.h>


#define MN_BASE64_ALPHABET(c62, c63)                                          \
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" #c62 #c63     \
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" #c62 #c63     \
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" #c62 #c63     \
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" #c62 #c63 "=" \


static char *alphabet_mime = MN_BASE64_ALPHABET(+, /);

static char *alphabet_url_std = MN_BASE64_ALPHABET(-, _);


static char codes_mime[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


static char codes_url_std[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x3f,
    0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


static void
encode3(const char a[64 * 4 + 1], const unsigned char i[3], char o[4])
{
    /*
     * (0)76543210 -> ..765432
     *
     * (0)76543210 -> ..10....
     * (1)76543210 -> ....7654
     *
     * (1)76543210 -> ..3210..
     * (2)76543210 -> ......76
     *
     * (2)76543210 -> ..543210
     */

    //TRACE("a=%s", a);
    o[0] = a[i[0] >> 2];
    //TRACE("i[0]>>2=%02hhx r=%02hhx",
    //      (unsigned char)(i[0] >> 2),
    //      a[i[0] >> 2]);

    o[1] = a[(unsigned char)(i[0] << 4) | (unsigned char)(i[1] >> 4)];
    //TRACE("i[0]<<4=%02hhx i[1]>>4=%02hhx r[%x]=%02hhx",
    //      (unsigned char)(i[0] << 4),
    //      (unsigned char)(i[1] >> 4),
    //      (unsigned char)(i[0] << 4) | (unsigned char)(i[1] >> 4),
    //      a[(unsigned char)(i[0] << 4) | (unsigned char)(i[1] >> 4)]);

    o[2] = a[(unsigned char)(i[1] << 2) | (unsigned char)(i[2] >> 6)];
    //TRACE("i[1]<<2=%02hhx i[2]>>6=%02hhx r[%x]=%02hhx",
    //      (unsigned char)(i[1] << 2),
    //      (unsigned char)(i[2] >> 6),
    //      ((unsigned char)i[1] << 2) | (unsigned char)(i[2] >> 6),
    //      a[(unsigned char)(i[1] << 2) | (unsigned char)(i[2] >> 6)]);

    o[3] = a[i[2]];
    //TRACE("i[2]<<2=%02hhx r=%02hhx",
    //      (unsigned char)(i[2]),
    //      a[i[2]]);
}


static void
encode2(const char a[64 * 4 + 1], const unsigned char i[2], char o[4])
{
    /*
     * (0)76543210 -> ..765432
     *
     * (0)76543210 -> ..10....
     * (1)76543210 -> ....7654
     *
     * (1)76543210 -> ..3210..
     *
     *             -> =
     */

    o[0] = a[i[0] >> 2];
    o[1] = a[(unsigned char)(i[0] << 4) | (unsigned char)(i[1] >> 4)];
    o[2] = a[(unsigned char)(i[1] << 2)];
    o[3] = '=';
}


static void
encode1(const char a[64 * 4 + 1], const unsigned char i[1], char o[4])
{
    /*
     * (0)76543210 -> ..765432
     *
     * (0)76543210 -> ..10....
     *
     *             -> =
     *
     *             -> =
     */

    o[0] = a[i[0] >> 2];
    o[1] = a[(unsigned char)(i[0] << 4)];
    o[2] = '=';
    o[3] = '=';
}


#define MNBASE64_ENCODE_BODY(alphabet)                                        \
    size_t i, sz;                                                              \
/*                                                                             \
    TRACE("dstsz=%ld srcsz=%ld srcsz / 3 + 1 = %ld (srcsz / 3 + 1) * 4 = %ld", \
          dstsz,                                                               \
          srcsz,                                                               \
          (srcsz / 3 + 1),                                                     \
          (srcsz / 3 + 1) * 4);                                                \
 */                                                                            \
    if (MNUNLIKELY(dstsz < ((srcsz / 3 + 1) * 4))) {                          \
        return -1;                                                             \
    }                                                                          \
    sz = srcsz / 3;                                                            \
    for (i = 0; i < sz; ++i) {                                                 \
        encode3(alphabet,                                                      \
                (const unsigned char *)(src + i * 3),                          \
                (char *)dst + i * 4);                                          \
    }                                                                          \
    /* now encode the remaining 1 or two bytes */                              \
    if ((srcsz % 3) == 2) {                                                    \
        encode2(alphabet, src + i * 3, dst + i * 4);                           \
    } else if ((srcsz % 3) == 1) {                                             \
        encode1(alphabet, src + i * 3, dst + i * 4);                           \
    } else {                                                                   \
        assert(srcsz % 3 == 0);                                                \
    }                                                                          \
    return 0                                                                   \


int
mnbase64_encode_mime(const unsigned char *src,
                 size_t srcsz,
                 char *dst,
                 size_t dstsz)
{
    MNBASE64_ENCODE_BODY(alphabet_mime);
}


int
mnbase64_encode_url_std(const unsigned char *src,
                 size_t srcsz,
                 char *dst,
                 size_t dstsz)
{
    MNBASE64_ENCODE_BODY(alphabet_url_std);
}


#define MNBASE64_DECODE_LOOP(codes)                                           \
    union {                                                                    \
        uint32_t *i;                                                           \
        const char *c;                                                         \
    } u = { .c = src + i * 4 };                                                \
    if (*(u.i) & 0x80808080 || !((*u.i) & 0xe0e0e0e0)) {                       \
        return -1;                                                             \
    }                                                                          \
    if (src[i * 4] == '=' || src[i * 4 + 1] == '=') {                          \
        break;                                                                 \
    }                                                                          \
    dst[i * 3] =                                                               \
        codes[(int)src[i * 4]] << 2 | codes[(int)src[i * 4 + 1]]  >> 4;        \
    ++(*dstsz);                                                                \
                                                                               \
    if (src[i * 4 + 2] == '=') {                                               \
        break;                                                                 \
    }                                                                          \
    dst[i * 3 + 1] =                                                           \
        (codes[(int)src[i * 4 + 1]]) << 4 | codes[(int)src[i * 4 + 2]] >> 2;   \
    ++(*dstsz);                                                                \
                                                                               \
    if (src[i * 4 + 3] == '=') {                                               \
        break;                                                                 \
    }                                                                          \
    dst[i * 3 + 2] =                                                           \
        (codes[(int)src[i * 4 + 2]]) << 6 | codes[(int)src[i * 4 + 3]] >> 0;   \
    ++(*dstsz)                                                                 \



int
mnbase64_decode_mime(const char *src,
                      size_t srcsz,
                      unsigned char *dst,
                      size_t *dstsz)
{
    size_t m = srcsz % 4, sz, i;
    if (MNUNLIKELY((m != 0) || (*dstsz / 3) * 4 < srcsz)) {
        return -1;
    }
    *dstsz = 0;
    sz = (srcsz + (m ? 4 - m : 0)) / 4;
    for (i = 0; i < sz; ++i) {
        MNBASE64_DECODE_LOOP(codes_mime);
    }
    return 0;
}


int
mnbase64_decode_url_std(const char *src,
                         size_t srcsz,
                         unsigned char *dst,
                         size_t *dstsz)
{
    size_t m = srcsz % 4, sz, i;
    if (MNUNLIKELY((m != 0) || (*dstsz / 3) * 4 < srcsz)) {
        return -1;
    }
    *dstsz = 0;
    sz = (srcsz + (m ? 4 - m : 0)) / 4;
    for (i = 0; i < sz; ++i) {
        MNBASE64_DECODE_LOOP(codes_url_std);
    }
    return 0;
}


int
mnbase64_decode_mime_inplace(char *s, size_t *sz)
{
    unsigned char *dst = (unsigned char *)s;
    size_t srcsz = *sz;
    return mnbase64_decode_mime(s, srcsz, dst, sz);
}


int
mnbase64_decode_url_std_inplace(char *s, size_t *sz)
{
    unsigned char *dst = (unsigned char *)s;
    size_t srcsz = *sz;
    return mnbase64_decode_url_std(s, srcsz, dst, sz);
}



void
mnbase64_test0(void)
{
    int i;
    union {
        uint32_t i;
        unsigned char c[3];
    } u0 = { .i = 0 };
    union {
        char t[5];
        char c[4];
    } u1 = {.t = {0}};

    for (i = 0; i <= 0x00ffffff; ++i) {
        u0.i = i;
        encode3(alphabet_mime, u0.c, u1.c);
        TRACE("%02hhx %02hhx %02hhx -> %-3s",
               u0.c[0],
               u0.c[1],
               u0.c[2],
               (char *)u1.t);
    }
}


void
mnbase64_test1(void)
{
    int i;

#define MNBASE64_TEST1_BODY(codes, alphabet)          \
    for (i = 0; i < 64; ++i) {                         \
        codes[(int)alphabet[i]] = i;                   \
    }                                                  \
    TRACEC("static char " #codes "[256] = {\n    ");   \
    for (i = 0; i < 256; ++i) {                        \
        if ((i + 1) % 8 == 0) {                        \
            TRACEC("0x%02hhx,\n    ", codes[i]);       \
        } else {                                       \
            TRACEC("0x%02hhx, ", codes[i]);            \
        }                                              \
    }                                                  \
    TRACEC("};\n\n")                                   \

    MNBASE64_TEST1_BODY(codes_mime, alphabet_mime);
    MNBASE64_TEST1_BODY(codes_url_std, alphabet_url_std);
}


