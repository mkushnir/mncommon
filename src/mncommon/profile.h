#ifndef MNCOMMON_PROFILE_H
#define MNCOMMON_PROFILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _profile {
    const char *name;
    int id;
    uint64_t n;
    uint64_t running_aggr;
    uint64_t start;
    uint64_t min;
    uint64_t max;
    long double avg;
} profile_t;

void profile_init_module(void);
void profile_fini_module(void);
const profile_t *profile_register(const char *);
void profile_start(const profile_t *);
uint64_t profile_stop(const profile_t *);
void profile_report(void);
void profile_report_sec(void);

#ifndef NO_PROFILE
#   define PROFILE_INIT_MODULE() profile_init_module()
#   define PROFILE_FINI_MODULE() profile_fini_module()
#   define PROFILE_REGISTER(name) profile_register((name))
#   define PROFILE_START(id) profile_start((id))
#   define PROFILE_STOP(id) profile_stop((id))
#   define PROFILE_REPORT() profile_report()
#   define PROFILE_REPORT_SEC() profile_report_sec()
#   define PROFILE_UNUSED
#else
#   define PROFILE_INIT_MODULE()
#   define PROFILE_FINI_MODULE()
#   define PROFILE_REGISTER(name) (NULL)
#   define PROFILE_START(id)
#   define PROFILE_STOP(id)
#   define PROFILE_REPORT()
#   define PROFILE_REPORT_SEC()
#   define PROFILE_UNUSED UNUSED
#endif

#ifdef __cplusplus
}
#endif

#endif /* PROFILE_H */
