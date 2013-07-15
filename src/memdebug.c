#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <malloc_np.h>

#include "mrkcommon/dumpm.h"

typedef struct _memdebug_ctx {
    const char *name;
    int id;
    size_t nallocated;
} memdebug_ctx_t;

/* copied from mrkcommon/memdebug.h, sigh ... */

typedef struct _memdebug_stat {
    const char *name;
    size_t nallocated;
} memdebug_stat_t;

static memdebug_ctx_t *memdebug_ctxes = NULL;
static int nctxes = 0;

int memdebug_register(const char *name)
{
    memdebug_ctx_t *ctx;
    memdebug_ctxes = realloc(memdebug_ctxes, sizeof(memdebug_ctx_t) * (nctxes + 1));
    ctx = memdebug_ctxes + nctxes;
    ctx->name = name;
    ctx->id = nctxes;
    ctx->nallocated = 0;
    ++nctxes;
    return ctx->id;
}

void *
memdebug_malloc(int n, size_t sz)
{
    void *res;
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    res = malloc(sz);

    if (res != NULL) {
        ctx->nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_calloc(int n, size_t e, size_t sz)
{
    void *res;
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    res = calloc(e, sz);

    if (res != NULL) {
        ctx->nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_realloc(int n, void * ptr, size_t sz)
{
    void *res;
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    if (ptr != NULL) {
        ctx->nallocated -= malloc_usable_size(ptr);
    }

    res = realloc(ptr, sz);

    if (res != NULL) {
        ctx->nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_reallocf(int n, void * ptr, size_t sz)
{
    void *res;
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    if (ptr != NULL) {
        ctx->nallocated -= malloc_usable_size(ptr);
    }

    res = reallocf(ptr, sz);

    if (res != NULL) {
        ctx->nallocated += malloc_usable_size(res);
    }

    return res;
}

void
memdebug_free(int n, void *ptr)
{
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    if (ptr != NULL) {
        ctx->nallocated -= malloc_usable_size(ptr);
    }

    free(ptr);
}

char *
memdebug_strdup(int n, const char *str)
{
    char *res;
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    res = strdup(str);

    if (res != NULL) {
        ctx->nallocated += malloc_usable_size(res);
    }

    return res;
}

char *
memdebug_strndup(int n, const char *str, size_t len)
{
    char *res;
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    res = strndup(str, len);

    if (res != NULL) {
        ctx->nallocated += malloc_usable_size(res);
    }

    return res;
}

void
memdebug_traverse_ctxes(int (*cb)(memdebug_stat_t *, void *), void *udata)
{
    memdebug_stat_t st;
    int i;

    assert(cb != NULL);

    for (i = 0; i < nctxes; ++i) {
        memdebug_ctx_t *ctx;

        ctx = memdebug_ctxes + i;
        st.name = ctx->name;
        st.nallocated = ctx->nallocated;

        if (cb(&st, udata) != 0) {
            break;
        }
    }
}

void
memdebug_stat(int n, memdebug_stat_t *st)
{
    memdebug_ctx_t *ctx;

    assert(n < nctxes);
    assert(st != NULL);

    ctx = memdebug_ctxes + n;

    st->name = ctx->name;
    st->nallocated = ctx->nallocated;
}

static int
my_memdebug_cb(memdebug_stat_t *st, void *udata)
{
    size_t *ntotal = udata;

    TRACEC("%-16s % 12ld", st->name, st->nallocated);
    *(ntotal) += st->nallocated;
    return 0;
}

void
memdebug_print_stats(void)
{
    size_t ntotal = 0;

    memdebug_traverse_ctxes(my_memdebug_cb, &ntotal);
    TRACEC("%-16s %-12s", "----------------", "------------");
    TRACEC("%-16s % 12ld", "Total", ntotal);
}

