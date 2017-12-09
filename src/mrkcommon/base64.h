#ifndef MRKCOMMON_BASE64_H
#define MRKCOMMON_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Supported:
 *  rfc 4648 (MIME) +/
 *  rfc 4648 (url) -_
 *
 *
 */
int mrkbase64_encode_mime(const unsigned char *, size_t, char *, size_t);
int mrkbase64_encode_url_std(const unsigned char *, size_t, char *, size_t);

int mrkbase64_decode_mime(const char *, size_t, unsigned char *, size_t *);
int mrkbase64_decode_url_std(const char *, size_t, unsigned char *, size_t *);

int mrkbase64_decode_mime_inplace(char *, size_t *);
int mrkbase64_decode_url_std_inplace(char *, size_t *);


/*
 * tests
 */
void mrkbase64_test0(void);
void mrkbase64_test1(void);

#ifdef __cplusplus
}
#endif

#endif
