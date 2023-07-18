#include <mncommon/util.h>
#include <mncommon/ring.h>

#include "diag.h"

void
mnring_init (mnring_t *ring,
             size_t elsz,
             size_t elnum,
             array_initializer_t init,
             array_finalizer_t fini)
{
    if (MNUNLIKELY(array_init(&ring->a, elsz, elnum, init, fini) != 0)) {
        FFAIL("array_init");
    }
    ring->cursor = (mnarray_iter_t){0, NULL};
}


void *
mnring_advance (mnring_t *ring, int incr)
{
    signed mod = (signed)ARRAY_ELNUM(&ring->a);

    incr = ((incr % mod) + mod) % mod;
    ring->cursor.iter += incr;
    ring->cursor.iter %= mod;
    return array_get_iter(&ring->a, &ring->cursor);
}

void *
mnring_first (const mnring_t *ring, mnarray_iter_t *it)
{
    *it = ring->cursor;
    return array_get_iter(&ring->a, it);
}


void *
mnring_last (const mnring_t *ring, mnarray_iter_t *it)
{
    signed mod = (signed)ARRAY_ELNUM(&ring->a);

    it->iter = ((((signed)ring->cursor.iter -1) % mod) + mod) % mod;
    return array_get_iter(&ring->a, it);
}


void *
mnring_next (const mnring_t *ring, mnarray_iter_t *it)
{
    signed mod = (signed)ARRAY_ELNUM(&ring->a);

    it->iter = (((((signed)it->iter) + 1) % mod) + mod) % mod;
    return array_get_iter(&ring->a, it);
}


void *
mnring_next_stop (const mnring_t *ring, mnarray_iter_t *it)
{
    signed mod = (signed)ARRAY_ELNUM(&ring->a);
    signed i = (((((signed)it->iter) + 1) % mod) + mod) % mod;

    if (i == (signed)ring->cursor.iter) {
        return NULL;
    }

    it->iter = i;
    return array_get_iter(&ring->a, it);
}


void *
mnring_prev (const mnring_t *ring, mnarray_iter_t *it)
{
    signed mod = (signed)ARRAY_ELNUM(&ring->a);

    it->iter = (((((signed)it->iter) - 1) % mod) + mod) % mod;
    return array_get_iter(&ring->a, it);
}


void *
mnring_prev_stop (const mnring_t *ring, mnarray_iter_t *it)
{
    signed mod = (signed)ARRAY_ELNUM(&ring->a);
    signed i = (((((signed)it->iter) - 1) % mod) + mod) % mod;
    signed j = (((((signed)ring->cursor.iter) - 1) % mod) + mod) % mod;

    if (i == j) {
        return NULL;
    }

    it->iter = i;
    return array_get_iter(&ring->a, it);
}


void
mnring_fini (mnring_t *ring)
{
    ring->cursor = (mnarray_iter_t){0, NULL};
    (void)array_fini(&ring->a);
}
