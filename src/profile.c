#include <sys/types.h>
#include <sys/sysctl.h>

#include "diag.h"
#include "mncommon/dumpm.h"
#include "mncommon/array.h"
#include "mncommon/profile.h"
#include "mncommon/util.h"

/* Code profiling */
static int initialized = 0;
static mnarray_t profiles;
static uint64_t tsc_freq;

static inline uint64_t
rdtsc(void)
{
#if defined(__amd64__)
    uint64_t res;
    __asm __volatile ("rdtsc; shl $32,%%rdx; or %%rdx,%%rax"
                      : "=a"(res)
                      :
                     );
    return res;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        FAIL("clock_gettime");
    }
    return ts.tv_nsec + ts.tv_sec * 1000000000;
#endif
}

static int
profile_init(profile_t *p)
{
    p->name = NULL;
    p->id = -1;
    p->n = 0;
    p->running_aggr = 0;
    p->start = 0;
    p->min = 0xffffffffffffffff;
    p->max = 0;
    p->avg = 0.;
    return 0;
}

static int
profile_dump(profile_t *p, UNUSED void *udata)
{
    printf("%s: n=%ld min=%ld avg=%Lf max=%ld total=%Lf\n", p->name, (long)p->n, (long)p->min, p->avg, (long)p->max, p->n * p->avg);
    return 0;
}

static int
profile_dump_sec(profile_t *p, UNUSED void *udata)
{
    printf("%s: n=%ld min=%Lf avg=%Lf max=%Lf total=%Lf\n", p->name, (long)p->n,
          (long double)(p->min) / (long double)tsc_freq,
          p->avg / (long double)tsc_freq, (long double)(p->max) / (long double)tsc_freq, p->n * ((long double)(p->avg) / (long double)tsc_freq));
    return 0;
}

void
profile_init_module(void)
{
#ifdef HAVE_SYSCTLBYNAME
    size_t sz = sizeof(tsc_freq);
    if (sysctlbyname("machdep.tsc_freq", &tsc_freq, &sz, NULL, 0) != 0) {
        if (sysctlbyname("machdep.tsc.frequency", &tsc_freq, &sz, NULL, 0) != 0) {
            FAIL("sysctlbyname");
        }
    }
#else
    /* fake */
    tsc_freq = 3600096762;
#endif

    if (initialized) {
        return;
    }

    if (array_init(&profiles, sizeof(profile_t), 0,
                   (array_initializer_t)profile_init,
                   NULL) != 0) {
        FAIL("array_init");
    }
    array_ensure_datasz(&profiles, 1024, 0);

    initialized = 1;
}

void
profile_fini_module(void)
{
    if (!initialized) {
        return;
    }
    array_fini(&profiles);
    initialized = 0;
}


const profile_t *
profile_register(const char *name)
{
    profile_t *p;

    if ((p = array_incr(&profiles)) == NULL) {
        FAIL("array_incr");
    }

    p->id = profiles.elnum - 1;
    p->name = name;
    return p;
}

void
profile_start(const profile_t *p)
{
    profile_t *pp = (profile_t *)p;

    if (pp->start != 0) {
        /* recursion ?*/
        return;
    }
    pp->start = rdtsc();
    //TRACE("p=%p start=%ld", pp, pp->start);
}

uint64_t
profile_stop(const profile_t *p)
{
    uint64_t stop = rdtsc();
    profile_t *pp = (profile_t *)p;
    uint64_t diff = stop - pp->start;

    //TRACE("p=%p stop=%ld diff=%ld", pp, stop, diff);

    pp->min = MIN(pp->min, diff);
    pp->max = MAX(pp->max, diff);
    pp->running_aggr += diff;
    ++pp->n;
    pp->avg = (long double)pp->running_aggr / (long double)pp->n;
    pp->start = 0;
    return diff;
}

void
profile_report(void)
{
    array_traverse(&profiles, (array_traverser_t)profile_dump, NULL);
}

void
profile_report_sec(void)
{
    array_traverse(&profiles, (array_traverser_t)profile_dump_sec, NULL);
}


