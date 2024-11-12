#ifndef MNCOMMON_TIME_H
#define MNCOMMON_TIME_H

#include <mncommon/util.h>

#if defined(__cplusplus)
extern "C" {
#endif


#define MN_TIMEVAL_DIFF2(res, a, b)            \
{                                              \
    (res).tv_sec = (a).tv_sec - (b).tv_sec;    \
    (res).tv_usec = (a).tv_usec - (b).tv_usec; \
                                               \
    if ((res).tv_usec < 0) {                   \
        --(res).tv_sec;                        \
        (res).tv_usec += 1000000;              \
    }                                          \
}                                              \


#define MN_TIMEVAL_SUM2(res, a, b)             \
{                                              \
    (res).tv_sec = (a).tv_sec + (b).tv_sec;    \
    (res).tv_usec = (a).tv_usec + (b).tv_usec; \
                                               \
    if ((res).tv_usec >= 1000000) {            \
        ++(res).tv_sec;                        \
        (res).tv_usec -= 1000000;              \
    }                                          \
}                                              \


#define MN_TIMEVAL_ISZERO(tv) (((tv).tv_sec == 0) && ((tv).tv_usec == 0))

#define MN_TIMEVAL_ZERO(tv) (tv) = (struct timeval){0.0, 0.0}

#define MN_TIMEVAL_CMP(a, b)                   \
    ((a)->tv_sec == (b)->tv_sec) ?             \
        MNCMP((a)->tv_usec, (b)->tv_usec) :    \
            MNCMP((a)->tv_sec, (b)->tv_sec)    \



#define MN_TIMESPEC_DIFF2(res, a, b)           \
{                                              \
    (res).tv_sec = (a).tv_sec - (b).tv_sec;    \
    (res).tv_nsec = (a).tv_usec - (b).tv_nsec; \
                                               \
    if ((res).tv_nsec < 0) {                   \
        --(res).tv_sec;                        \
        (res).tv_nsec += 1000000000;           \
    }                                          \
}                                              \


#define MN_TIMESPEC_SUM2(res, a, b)            \
{                                              \
    (res).tv_sec = (a).tv_sec + (b).tv_sec;    \
    (res).tv_nsec = (a).tv_nsec + (b).tv_nsec; \
                                               \
    if ((res).tv_nsec >= 1000000000) {         \
        ++(res).tv_sec;                        \
        (res).tv_nsec -= 1000000000;           \
    }                                          \
}                                              \


#define MN_TIMESPEC_ISZERO(tv) (((tv).tv_sec == 0) && ((tv).tv_nsec == 0))

#define MN_TIMESPEC_ZERO(tv) (tv) = (struct timespec){0.0, 0.0}

#define MN_TIMESPEC_CMP(a, b)                  \
    ((a)->tv_sec == (b)->tv_sec) ?             \
        MNCMP((a)->tv_nsec, (b)->tv_nsec) :    \
            MNCMP((a)->tv_sec, (b)->tv_sec)    \



#if defined(__cplusplus)
}
#endif
#endif
