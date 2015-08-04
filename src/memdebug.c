#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_MALLOC_NP_H
#include <malloc_np.h>
#else
#include <malloc.h>
#endif

#ifndef HAVE_REALLOCF
void *
reallocf(void *ptr, size_t sz)
{
    void *tmp = realloc(ptr, sz);
    if (tmp == NULL)
        free(ptr);
    return tmp;
}
#endif

#include "mrkcommon/dumpm.h"
#include <mrkcommon/util.h>


#define MEMDEBUG_BODY(ptr, op1, op2)                           \
if ((ptr) != NULL) {                                           \
    assert(n >= 0 && n < (int)countof(memdebug_ctxes));        \
    if (n < nctxes) {                                          \
        memdebug_ctx_t *ctx;                                   \
        ctx = memdebug_ctxes + n;                              \
        ctx->nallocated op1 malloc_usable_size(ptr);           \
        op2 ctx->nitems;                                       \
    }                                                          \
    if (runtime_scope >= 0) {                                  \
        assert(runtime_scope < (int)countof(memdebug_ctxes));  \
        if (runtime_scope < nctxes) {                          \
            memdebug_ctx_t *ctx;                               \
            ctx = memdebug_ctxes + runtime_scope;              \
            ctx->nallocated op1 malloc_usable_size(ptr);       \
            op2 ctx->nitems;                                   \
        }                                                      \
    } else {                                                   \
/*                                                             \
        TRACE("problem runtime_scope: %d", runtime_scope);     \
 */                                                            \
    }                                                          \
}                                                              \


#define MEMDEBUG_BODY_INC(ptr) MEMDEBUG_BODY(ptr, +=, ++)
#define MEMDEBUG_BODY_DEC(ptr) MEMDEBUG_BODY(ptr, -=, --)


typedef struct _memdebug_ctx {
    const char *name;
    int id;
    size_t nallocated;
    size_t nitems;
} memdebug_ctx_t;

/* copied from mrkcommon/memdebug.h, sigh ... */

typedef struct _memdebug_stat {
    const char *name;
    size_t nallocated;
    size_t nitems;
} memdebug_stat_t;

static memdebug_ctx_t memdebug_ctxes[256];
static int nctxes = 0;
static int runtime_scope = -1;


int
memdebug_register(const char *name)
{
    memdebug_ctx_t *ctx;
    assert(nctxes < (int)countof(memdebug_ctxes));
    ctx = memdebug_ctxes + nctxes;
    ctx->name = name;
    ctx->id = nctxes;
    ctx->nallocated = 0;
    ctx->nitems = 0;
    ++nctxes;
    return ctx->id;
}


void
memdebug_clear(void)
{
}


int
memdebug_set_runtime_scope(int n)
{
    int res;

    res = runtime_scope;
    runtime_scope = n;
    return res;
}


int
memdebug_get_runtime_scope(void)
{
    return runtime_scope;
}


void *
memdebug_malloc(int n, size_t sz)
{
    void *res;
    res = malloc(sz);

    MEMDEBUG_BODY_INC(res)

    return res;
}


void *
memdebug_calloc(int n, size_t e, size_t sz)
{
    void *res;
    res = calloc(e, sz);

    MEMDEBUG_BODY_INC(res)

    return res;
}


void *
memdebug_realloc(int n, void *ptr, size_t sz)
{
    void *res;

    MEMDEBUG_BODY_DEC(ptr)

    res = realloc(ptr, sz);

    MEMDEBUG_BODY_INC(res)

    return res;
}


void *
memdebug_reallocf(int n, void *ptr, size_t sz)
{
    void *res;

    MEMDEBUG_BODY_DEC(ptr)

    res = reallocf(ptr, sz);

    MEMDEBUG_BODY_INC(res)

    return res;
}


void
memdebug_free(int n, void *ptr)
{

    MEMDEBUG_BODY_DEC(ptr)

    free(ptr);
}


char *
memdebug_strdup(int n, const char *str)
{
    char *res;

    res = strdup(str);

    MEMDEBUG_BODY_INC(res)

    return res;
}


char *
memdebug_strndup(int n, const char *str, size_t len)
{
    char *res;

    res = strndup(str, len);

    MEMDEBUG_BODY_INC(res)

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
        st.nitems = ctx->nitems;

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
    st->nitems = ctx->nitems;
}


static int
my_memdebug_cb(memdebug_stat_t *st, void *udata)
{
    struct {
        size_t nallocated_total;
        size_t nitems_total;
    } *params;

    params = udata;

    TRACEC("%-16s % 12ld % 12ld\n", st->name, st->nallocated, st->nitems);
    params->nallocated_total += st->nallocated;
    params->nitems_total += st->nitems;
    return 0;
}


void
memdebug_print_stats(void)
{
    struct {
        size_t nallocated_total;
        size_t nitems_total;
    } params = { 0, 0};

    memdebug_traverse_ctxes(my_memdebug_cb, &params);
    TRACEC("%-16s %-12s %-12s\n", "----------------", "----------------", "------------");
    TRACEC("%-16s % 12ld % 12ld\n", "Total", params.nallocated_total, params.nitems_total);
}


static int
my_memdebug_oneline_cb(memdebug_stat_t *st, UNUSED void *udata)
{
    struct {
        size_t nallocated_total;
        size_t nitems_total;
    } *params;

    params = udata;

    TRACEC("%s\t%ld\t", st->name, st->nallocated);
    params->nallocated_total += st->nallocated;
    params->nitems_total += st->nitems;
    return 0;
}


void
memdebug_print_stats_oneline(void)
{
    struct {
        size_t nallocated_total;
        size_t nitems_total;
    } params = { 0, 0};

    TRACEC("memdebug\t%ld\t", time(NULL));
    memdebug_traverse_ctxes(my_memdebug_oneline_cb, &params);
    TRACEC("total\t%ld\n", params.nallocated_total);
}

