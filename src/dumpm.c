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
    printf(" %02x", *((unsigned char *)(m + i)));
  }
  for (i = 0; i < (l - n); ++i) {
    printf("   ");
  }
  printf("|");
  for (i = 0; i < n; ++i) {
    unsigned char c = *((unsigned char *)(m + i));
    printf("%c", isprint(c) ? c : ' ');
  }
}

void dumpm(const char *m, size_t n, size_t l) {
  div_t d = div(n, l);
  int i;
  for (i = 0; i < d.quot; ++i) {
    printf("%016lx:", (intptr_t)(m+(i * l)));
    dumpl(m+(i * l), l, l);
    printf("\n");
  }
  if (d.rem > 0) {
    printf("%016lx:", (intptr_t)(m+(i * l)));
    dumpl(m+(i * l), d.rem, l);
  }
  printf("\n\n");
}


