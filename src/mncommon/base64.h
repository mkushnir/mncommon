#ifndef MNCOMMON_BASE64_H
#define MNCOMMON_BASE64_H

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
int mnbase64_encode_mime(const unsigned char *, size_t, char *, size_t);
int mnbase64_encode_url_std(const unsigned char *, size_t, char *, size_t);

int mnbase64_decode_mime(const char *, size_t, unsigned char *, size_t *);
int mnbase64_decode_url_std(const char *, size_t, unsigned char *, size_t *);

int mnbase64_decode_mime_inplace(char *, size_t *);
int mnbase64_decode_url_std_inplace(char *, size_t *);


/*
 * tests
 */
void mnbase64_test0(void);
void mnbase64_test1(void);

#ifdef __cplusplus
}
#endif

#endif
