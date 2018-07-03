#ifndef __FAKE_RANDOM_H__
#define __FAKE_RANDOM_H__

extern uint32_t rand_uint32(void);
extern void rand_reseed(uint32_t seed);
extern void rand_bytes(uint8_t *data, uint32_t len);
extern uint32_t get_system_rand(void);
extern uint32_t get_system_rand_limit(uint32_t min, uint32_t max);

#endif /* __FAKE_RANDOM_H__ */
