#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

static void dumpl(const char *m, size_t n, size_t l) {
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

void dumpm(const char *m, size_t n, size_t l) {
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


