#include "base.h"

static uint32_t _g_randseed = 12345;

uint32_t rand_uint32(void)
{
    return (_g_randseed = _g_randseed * 1664525 + 1013904223);
}

void rand_reseed(uint32_t seed)
{
    _g_randseed = seed;
}

void rand_bytes(uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t rand_u32 = 0;
    for (i = 0; i < (len / 4); i++) {
        rand_u32    = rand_uint32();
        data[i + 0] = ((rand_u32 >> 0) & 0xff);
        data[i + 1] = ((rand_u32 >> 8) & 0xff);
        data[i + 2] = ((rand_u32 >> 16) & 0xff);
        data[i + 3] = ((rand_u32 >> 24) & 0xff);
    }

    for (i = len / 4; i < len; i++) {
        data[i] = ((rand_uint32()) & 0xff);
    }
    return;
}

#ifdef __KERNEL__
#include <linux/random.h>
uint32_t get_system_rand(void)
{
    uint32_t data = 0;
    get_random_bytes(&data, (int)(sizeof(uint32_t)));
    return data;
}
#else
uint32_t get_system_rand(void)
{
    // srand((int)time(0));
    return (uint32_t)(rand());
}
#endif

uint32_t get_system_rand_limit(uint32_t min, uint32_t max)
{
    uint32_t ret;
    do {
        ret = get_system_rand();
        if (ret >= max) {
            ret = ret % max;
        }

        if (ret <= min) {
            ret += min;
        }

    } while ((ret <= min) || (ret >= max));
    return ret;
}
