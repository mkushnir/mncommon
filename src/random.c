#include <mncommon/random.h>
#include <mncommon/util.h>

long
randint_weighted (long a, long b, size_t wsz, float const weights[static wsz])
{
    long res = 0;
    float u0, u1;
    unsigned idx;

    while (true) {
        u0 = ((float)random()) / ((float)RAND_MAX);

        float z0 = (b - a) * u0;

        idx = (int)(z0 / ((float)(b - a)) * ((float)wsz));

        if (!INB1(0, idx, wsz)) {
            idx = wsz - 1;
        }

        u1 = ((float)random()) / ((float)RAND_MAX);

        if (u1 < weights[idx]) {
            res = a + (long)z0;
            break;
        }
    }

    return res;
}


long
randint_biased (float a, float b, float bias)
{
    float normed = ((float)random()) / ((float)RAND_MAX);
    float probe = a + (b - a) * normed;
    return (probe <= bias) ? a : b;
}
