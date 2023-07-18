#ifndef MNRING_H_DEFINED
#define MNRING_H_DEFINED

#include <mncommon/array.h>

typedef struct {
    mnarray_t a;
    mnarray_iter_t cursor;
} mnring_t;


void mnring_init (mnring_t *, size_t, size_t, array_initializer_t, array_finalizer_t);

void mnring_fini (mnring_t *);

void *mnring_advance (mnring_t *, int);
void *mnring_first (const mnring_t *, mnarray_iter_t *);
void *mnring_last (const mnring_t *, mnarray_iter_t *);
void *mnring_next (const mnring_t *, mnarray_iter_t *);
void *mnring_next_stop (const mnring_t *, mnarray_iter_t *);
void *mnring_prev (const mnring_t *, mnarray_iter_t *);
void *mnring_prev_stop (const mnring_t *, mnarray_iter_t *);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* MNRING_H_DEFINED */

