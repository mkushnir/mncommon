#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <mncommon/ring.h>

static void
_dump_ring (mnring_t *ring)
{
    mnarray_iter_t it;
    unsigned c;
    void *p;

    for (c = 0, p = mnring_first(ring, &it);
         c < (ARRAY_ELNUM(&ring->a) * 3);
         p = mnring_next(ring, &it), ++c) {
        int i = *(int *)p;
        printf(">>> iter %d i %d\n", it.iter, i);
    }

    for (c = 0, p = mnring_last(ring, &it);
         c < (ARRAY_ELNUM(&ring->a) * 3);
         p = mnring_prev(ring, &it), ++c) {
        int i = *(int *)p;
        printf("<<< iter %d i %d\n", it.iter, i);
    }
}


static void
_dump_ring2 (mnring_t *ring)
{
    mnarray_iter_t it;

    for (void *p = mnring_first(ring, &it);
         p != NULL;
         p = mnring_next_stop(ring, &it)) {
        int i = *(int *)p;
        printf(">>> iter %d i %d\n", it.iter, i);
    }
    for (void *p = mnring_last(ring, &it);
         p != NULL;
         p = mnring_prev_stop(ring, &it)) {
        int i = *(int *)p;
        printf("<<< iter %d i %d\n", it.iter, i);
    }
}



int
main (void)
{
    mnring_t ring;

#ifdef HAVE_SRANDOMDEV
    srandomdev();
#else
    srandom(time(NULL));
#endif

    mnring_init(&ring, sizeof(int), 7, NULL, NULL);

    for (unsigned i = 0;
         i < ARRAY_ELNUM(&ring.a);
         ++i) {
        int *p = ARRAY_GET(int, &ring.a, i);
        *p = (int)random();
    }

    _dump_ring(&ring);

    printf("\n");

    mnring_advance(&ring, 1);

    _dump_ring(&ring);

    printf("\n");

    mnring_advance(&ring, 10);

    _dump_ring(&ring);

    printf("\n");

    _dump_ring2(&ring);

    mnring_fini(&ring);
    return 0;
}
