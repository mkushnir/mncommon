#include <stdlib.h>
#include <time.h>
#include "mrkcommon/util.h"

#define _R(_d, _i) _d[_i].rnd
#define _RD(_d, _i) _d[R(_i)]
#define RDATA(_i) _RD(data, _i)
#define _FOREACH(_d, _i) for (_i = 0; _i < countof(_d); ++_i)
#define _FOREACHI(_d) _FOREACH(_d, i)
#define _CD(_d) _RD(_d, i)

#define R(_i) _R(data, _i)
#define FOREACHDATA _FOREACHI(data)
#define CDATA _CD(data)

#define UNITTEST_PROLOG                        \
    unsigned i;                                \
    for (i = 0; i < countof(data); ++i) {      \
        R(i) = i;                              \
    }                                          \


#define UNITTEST_PROLOG_RAND                   \
    unsigned i;                                \
    srandom(time(NULL));                       \
    for (i = 0; i < countof(data); ++i) {      \
        R(i) = i;                              \
    }                                          \
    SHUFFLE                                    \


#define _SHUFFLE(_d, _i)                       \
    for (_i = 0; _i < countof(_d); ++_i) {     \
        int tmp = _R(_d, _i);                  \
        int ii = random() % countof(_d);       \
        _R(_d, _i) = _R(_d, ii);               \
        _R(_d, ii) = tmp;                      \
    }                                          \


#define SHUFFLE _SHUFFLE(data, i)


#define MNSHUFFLE_DATA(a, idx)                                         \
do {                                                                   \
    unsigned _mnshuffle_data_i;                                        \
    for (_mnshuffle_data_i = 0;                                        \
         _mnshuffle_data_i < countof((a));                             \
         ++_mnshuffle_data_i) {                                        \
        (a)[_mnshuffle_data_i].idx = _mnshuffle_data_i;                \
    }                                                                  \
    for (_mnshuffle_data_i = 0;                                        \
         _mnshuffle_data_i < countof((a));                             \
         ++_mnshuffle_data_i) {                                        \
        int _mnshuffle_data_ii = random() % countof((a));              \
        int _mnshuffle_data_idx = (a)[_mnshuffle_data_i].idx;          \
        (a)[_mnshuffle_data_i].idx = (a)[_mnshuffle_data_ii].idx;      \
        (a)[_mnshuffle_data_ii].idx = _mnshuffle_data_idx;             \
    }                                                                  \
} while (0)                                                            \


#define MNSHUFFLE_VALUE(a, idx, i) ((a)[a[(i)].idx])


#define _MNUNIT_PARAMETRIZE(name, body, sz)                    \
    for (size_t _ ## name = 0; _ ## name < sz; ++_ ## name) {  \
        body                                                   \
    }                                                          \


#define MNUNIT_PARAMETRIZE_STATIC(name, body) \
    _MNUNIT_PARAMETRIZE(name, body, countof(name))


#define MNUNIT_ARG(name) name[_ ## name]


