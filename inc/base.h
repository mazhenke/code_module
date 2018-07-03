#ifndef __BASE_H__
#define __BASE_H__
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum {
    SUCCESS              = 0,
    ERROR_GENERIC        = 0x00000001,
    ERROR_INVALID_ARGS   = 0x00000002,
    ERROR_NULL_POINTER   = 0x00000003,
    ERROR_BAD_FORMAT     = 0x00000004,
    ERROR_BAD_PARAMETERS = 0x00000005,
    ERROR_BAD_STATE      = 0x00000006,
    ERROR_OUT_OF_MEMORY  = 0x00000009,
} err_t;

#define DBG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define ASSERT(condition)                                                      \
    do {                                                                       \
        if (!(condition)) {                                                    \
            ERR("ASSERT : %s, %d\n", __func__, __LINE__);                      \
            while (1)                                                          \
                ;                                                              \
        }                                                                      \
    } while (0)

#define CHECK_RET(format, ...)                                                 \
    do {                                                                       \
        if ((SUCCESS) != (ret)) {                                              \
            ERR("%s line:%d. Error number is 0x%x \n",                         \
                __func__,                                                      \
                __LINE__,                                                      \
                ret);                                                          \
            ERR(format, ##__VA_ARGS__);                                        \
            goto finish;                                                       \
        }                                                                      \
    } while (0)

#define CHECK_NULL(pointer, format, ...)                                       \
    do {                                                                       \
        if (NULL == (pointer)) {                                               \
            ret = ERROR_GENERIC;                                               \
            ERR("%s line:%d. Error number is 0x%x \n",                         \
                __func__,                                                      \
                __LINE__,                                                      \
                ret);                                                          \
            ERR(format, ##__VA_ARGS__);                                        \
            goto finish;                                                       \
        }                                                                      \
    } while (0)

#define CHECK_CONDITION(true_condition, format, ...)                           \
    do {                                                                       \
        if (!(true_condition)) {                                               \
            ret = ERROR_GENERIC;                                               \
            ERR("%s line:%d. Error number is 0x%x \n",                         \
                __func__,                                                      \
                __LINE__,                                                      \
                ret);                                                          \
            ERR(format, ##__VA_ARGS__);                                        \
            goto finish;                                                       \
        }                                                                      \
    } while (0)

#endif /* __BASE_H__ */
