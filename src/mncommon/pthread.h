#ifndef MNCOMMON_PTHREAD_H
#define MNCOMMON_PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <errno.h>

#if !defined(HAVE_PTHREAD_MUTEX_CONSISTENT)
#   define pthread_mutex_consistent(mtx) (0)
#endif

#if !defined(HAVE_PTHREAD_YIELD)
#   define pthread_yield(mtx)
#endif

#define MN_PTHREAD_MTX_LOCK(mtx)                                       \
do {                                                                   \
    int res;                                                           \
    if (MNUNLIKELY((res = pthread_mutex_lock(mtx)) == EOWNERDEAD)) {   \
        if (MNUNLIKELY(pthread_mutex_consistent(mtx) != 0)) {          \
            FAIL("pthread_mutex_consistent");                          \
        }                                                              \
        if (MNUNLIKELY(pthread_mutex_lock(mtx) != 0)) {                \
            FAIL("pthread_mutex_lock");                                \
        }                                                              \
    } else if (res != 0) {                                             \
        FAIL("pthread_mutex_lock");                                    \
    }                                                                  \
} while (false)                                                        \


#define MN_PTHREAD_MTX_UNLOCK(mtx)                     \
if (MNUNLIKELY(pthread_mutex_unlock(mtx) != 0)) {      \
    FAIL("pthread_mutex_unlock");                      \
}                                                      \


#define MN_PTHREAD_COND_WAIT(cond, mtx)                                \
do {                                                                   \
    int res;                                                           \
    if (MNUNLIKELY(                                                    \
            (res = pthread_cond_wait(cond, mtx)) == EOWNERDEAD)) {     \
        if (MNUNLIKELY(pthread_mutex_consistent(mtx) != 0)) {          \
            FAIL("pthread_mutex_consistent");                          \
        }                                                              \
        if (MNUNLIKELY(pthread_cond_wait(cond, mtx) != 0)) {           \
            FAIL("pthread_cond_wait");                                 \
        }                                                              \
    } else if (res != 0) {                                             \
        FAIL("pthread_cond_wait");                                     \
    }                                                                  \
} while (false)                                                        \


#define MN_PTHREAD_COND_TIMEDWAIT(cond, mtx, millis, align, res)               \
do {                                                                           \
    struct timespec tm;                                                        \
    long ts0, ts1;                                                             \
                                                                               \
    /* cannot align at larger scale than increment */                          \
    assert(millis >= align);                                                   \
                                                                               \
    if (MNUNLIKELY(clock_gettime(CLOCK_REALTIME, &tm) != 0)) {                 \
        FAIL("clock_gettime");                                                 \
    }                                                                          \
                                                                               \
    ts0 = tm.tv_sec * 1000000000 + tm.tv_nsec;                                 \
                                                                               \
    ts1 = ts0 + millis * 1000000;                                              \
    ts1 -= (ts1 % (align * 1000000));                                          \
                                                                               \
    assert(ts1 > ts0);                                                         \
                                                                               \
    tm.tv_sec = (time_t)(ts1 / 1000000000);                                    \
    tm.tv_nsec = (long)((ts1 % 1000000000));                                   \
                                                                               \
    if (MNUNLIKELY(                                                            \
            (res = pthread_cond_timedwait(cond, mtx, &tm)) == EOWNERDEAD)) {   \
        if (MNUNLIKELY(pthread_mutex_consistent(mtx) != 0)) {                  \
            FAIL("pthread_mutex_consistent");                                  \
        }                                                                      \
        if (MNUNLIKELY(                                                        \
                ((res = pthread_cond_timedwait(cond, mtx, &tm)) != 0) &&       \
                (res != ETIMEDOUT))) {                                         \
            FAIL("pthread_cond_wait");                                         \
        }                                                                      \
    } else if ((res != 0) && (res != ETIMEDOUT)) {                             \
        FAIL("pthread_cond_wait");                                             \
    }                                                                          \
} while (false)                                                                \


#define MN_PTHREAD_COND_BROADCAST(cond)                \
if (MNUNLIKELY(pthread_cond_broadcast(cond) != 0)) {   \
    FAIL("pthread_cond_broadcast");                    \
}                                                      \


#define MN_PTHREAD_COND_SIGNAL(cond)                   \
if (MNUNLIKELY(pthread_cond_signal(cond) != 0)) {      \
    FAIL("pthread_cond_signal");                       \
}                                                      \


#define MN_PTHREAD_SLEEP(millis, align)                        \
{                                                              \
    MN_PTHREAD_MTX_LOCK(&mtx);                                 \
    MN_PTHREAD_COND_TIMEDWAIT(                                 \
        &cv, &mtx, (long)(millis), (long)(align), res);        \
    MN_PTHREAD_MTX_UNLOCK(&mtx);                               \
}                                                              \




#ifdef __cplusplus
}
#endif

#endif
