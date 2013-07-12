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

static size_t nallocated = 0;
static memdebug_ctx_t *memdebug_ctxes = NULL;
static int nctxes = 0;

int memdebug_register(const char *name)
{
    memdebug_ctx_t *ctx;
    memdebug_ctxes = realloc(memdebug_ctxes, sizeof(memdebug_ctx_t) * (nctxes + 1));
    ctx = memdebug_ctxes + nctxes;
    ctx->name = name;
    ctx->id = nctxes;
    ++nctxes;
    return ctx->id;
}

void *
memdebug_malloc(size_t sz)
{
    void *res;

    res = malloc(sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_malloc_named(int n, size_t sz)
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
memdebug_calloc(size_t n, size_t sz)
{
    void *res;

    res = calloc(n, sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_calloc_named(int n, size_t e, size_t sz)
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
memdebug_realloc(void * ptr, size_t sz)
{
    void *res;

    if (ptr != NULL) {
        nallocated -= malloc_usable_size(ptr);
    }

    res = realloc(ptr, sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_realloc_named(int n, void * ptr, size_t sz)
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
memdebug_reallocf(void * ptr, size_t sz)
{
    void *res;

    if (ptr != NULL) {
        nallocated -= malloc_usable_size(ptr);
    }

    res = reallocf(ptr, sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_reallocf_named(int n, void * ptr, size_t sz)
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
memdebug_free(void *ptr)
{
    if (ptr != NULL) {
        nallocated -= malloc_usable_size(ptr);
    }

    free(ptr);
}

void
memdebug_free_named(int n, void *ptr)
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
memdebug_strdup(const char *str)
{
    char *res;

    res = strdup(str);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

char *
memdebug_strdup_named(int n, const char *str)
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
memdebug_strndup(const char *str, size_t len)
{
    char *res;

    res = strndup(str, len);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

char *
memdebug_strndup_named(int n, const char *str, size_t len)
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

size_t
memdebug_nallocated(void)
{
    return nallocated;
}

size_t
memdebug_nallocated_named(int n)
{
    memdebug_ctx_t *ctx;


    assert(n < nctxes);
    ctx = memdebug_ctxes + n;

    return ctx->nallocated;
}
