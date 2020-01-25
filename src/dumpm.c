#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

#include <mncommon/util.h>

static
void dumpl(const char *m, size_t n, size_t l) {
  size_t i;
  assert(l >= n);
  for (i = 0; i < n; ++i) {
    fprintf(stderr, " %02x", *((unsigned char *)(m + i)));
  }
  for (i = 0; i < (l - n); ++i) {
    fprintf(stderr, "   ");
  }
  fprintf(stderr, "|");
  for (i = 0; i < n; ++i) {
    unsigned char c = *((unsigned char *)(m + i));
    fprintf(stderr, "%c", isprint(c) ? c : ' ');
  }
}

void
dumpm(const char *m, size_t n, size_t l) {
  div_t d = div(n, l);
  int i;
  for (i = 0; i < d.quot; ++i) {
    fprintf(stderr, "%016lx:", (intptr_t)(m+(i * l)));
    dumpl(m+(i * l), l, l);
    fprintf(stderr, "\n");
  }
  if (d.rem > 0) {
    fprintf(stderr, "%016lx:", (intptr_t)(m+(i * l)));
    dumpl(m+(i * l), d.rem, l);
  }
  fprintf(stderr, "\n\n");
}



#define MNDUMP_LINESZ_BITS (64)
#define MNDUMP_BIT(idx, bit) buf[j++] = (m[(idx)] & bit) ? '1' : '0'
#define MNDUMP_PAD(c) buf[j++] = (c)
#define MNDUMP_BAR() MNDUMP_PAD('|')
#define MNDUMP_EOL() MNDUMP_PAD('\n')
#define MNDUMP_BYTE(idx) buf[j++] = isprint(m[(idx)]) ? m[(idx)] : ' ';

#define MNDUMP_BITS(idx)       \
    MNDUMP_BIT(idx, 0x80);     \
    MNDUMP_BIT(idx, 0x40);     \
    MNDUMP_BIT(idx, 0x20);     \
    MNDUMP_BIT(idx, 0x10);     \
    MNDUMP_BIT(idx, 0x08);     \
    MNDUMP_BIT(idx, 0x04);     \
    MNDUMP_BIT(idx, 0x02);     \
    MNDUMP_BIT(idx, 0x01);     \
    MNDUMP_PAD(' ');           \


#define MNDUMP_PAD8(c) \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \
    MNDUMP_PAD(c);     \


static void
mndump_bits_one_line(const char *m, int sz)
{
    int nwritten;
    char buf[MNDUMP_LINESZ_BITS * 2];
    int bytesz, byteidx, bitidx;
    int rembits;
    int j;


    assert(sz <= MNDUMP_LINESZ_BITS);
    if ((nwritten = snprintf(
                buf, sizeof(buf), "%016lx: ", (uintptr_t)m)) <= 0) {
        // swallow line
        goto end;
    }

    bytesz = sz / 8;
    rembits = sz % 8;
    j = nwritten;

    // full bytes
    for (byteidx = 0; byteidx < bytesz; ++byteidx) {
        MNDUMP_BITS(byteidx);
    }

    // remaining bits
    if (rembits) {
        for (bitidx = 0; bitidx < rembits; ++bitidx) {
            MNDUMP_BIT(byteidx, 1 << (7 - bitidx));
        }
        // pad for full byte
        for (; bitidx < 8; ++bitidx) {
            MNDUMP_PAD(' ');
        }
        MNDUMP_PAD(' ');
        ++byteidx;
    }

    // pad for full remaining bytes
    for (; byteidx < MNDUMP_LINESZ_BITS / 8; ++byteidx) {
        MNDUMP_PAD8(' ');
    }

    MNDUMP_BAR();

    // dump raw bytes
    for (byteidx = 0; byteidx < bytesz; ++byteidx) {
        MNDUMP_BYTE(byteidx);
    }

    MNDUMP_EOL();

    fwrite(buf, j, 1, stderr);

end:
    return;
}


void
mndump_bits(const void *mem, size_t sz)
{
    // 0000000000000000: bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb | aaaaaaaa
    size_t bytesz;
    unsigned byteidx = 0;
    const char *m = (const char *)mem;

    bytesz = sz / 8; // number of full bytes

    if (bytesz > 0) {
        if (bytesz >= (MNDUMP_LINESZ_BITS / 8)) {
            for (byteidx = 0;
                 (byteidx + MNDUMP_LINESZ_BITS / 8) <= bytesz;
                 byteidx += (MNDUMP_LINESZ_BITS / 8)) {
                mndump_bits_one_line(&m[byteidx], MNDUMP_LINESZ_BITS);
            }
        }
        if (sz % MNDUMP_LINESZ_BITS) {
            mndump_bits_one_line(&m[byteidx], sz % MNDUMP_LINESZ_BITS);
        }
    } else {
        mndump_bits_one_line(&m[byteidx], sz);
    }
    fwrite("\n", 1, 1, stderr);
}
