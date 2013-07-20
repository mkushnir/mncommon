#include <sys/types.h>
#include <sys/sysctl.h>

#include "diag.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/list.h"
#include "mrkcommon/profile.h"
#include "mrkcommon/util.h"

/* Code profiling */
static list_t profiles;
static uint64_t tsc_freq;

static inline uint64_t
rdtsc(void)
{
  uint32_t lo, hi;

  //__asm __volatile ("" ::: "memory");
  __asm __volatile ("rdtsc" : "=a"(lo), "=d"(hi));
  return (uint64_t) hi << 32 | lo;
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
    TRACE("profile: %s n=%ld min=%ld avg=%Lf max=%ld total=%Lf", p->name, p->n, p->min, p->avg, p->max, p->n * p->avg);
    return 0;
}

void
profile_init_module(void)
{
    size_t sz = sizeof(tsc_freq);
    if (sysctlbyname("machdep.tsc_freq", &tsc_freq, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname");
    }

    if (list_init(&profiles, sizeof(profile_t), 0,
                   (list_initializer_t)profile_init,
                   NULL) != 0) {
        FAIL("list_init");
    }
}

void
profile_fini_module(void)
{
    list_fini(&profiles);
}


const profile_t *
profile_register(const char *name)
{
    profile_t *p;

    if ((p = list_incr(&profiles)) == NULL) {
        FAIL("list_incr");
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
    //TRACE("start=%ld", pp->start);
}

uint64_t
profile_stop(const profile_t *p)
{
    profile_t *pp = (profile_t *)p;
    uint64_t stop = rdtsc();
    uint64_t diff;

    diff = stop - pp->start;

    //TRACE("stop=%ld diff=%ld", stop, diff);

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
    list_traverse(&profiles, (list_traverser_t)profile_dump, NULL);
}


