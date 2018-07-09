#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "base.h"
typedef struct _ring_buffer_t {
    uint8_t *buf;
    uint32_t size;
    uint32_t read_idx;
    uint32_t write_idx;
} ring_buffer_t;

#define RING_BUFFER_CAPACITY(_rb_p_) ((_rb_p_)->size - 1)

#define RING_BUFFER_FREE_SIZE(_rb_p_)                                          \
    (((_rb_p_)->write_idx >= (_rb_p_)->read_idx)                               \
         ? (RING_BUFFER_CAPACITY(_rb_p_)                                       \
            - ((_rb_p_)->write_idx - (_rb_p_)->read_idx))                      \
         : ((_rb_p_)->read_idx - (_rb_p_)->write_idx) - 1)

#define IS_RING_BUFFER_EMPYT(_rb_p_)                                           \
    (RING_BUFFER_FREE_SIZE(_rb_p_) == RING_BUFFER_CAPACITY(_rb_p_))
#define IS_RING_BUFFER_FULL(_rb_p_) (RING_BUFFER_FREE_SIZE(_rb_p_) == 0)

#define RING_BUFFER_DATA_SIZE(_rb_p_)                                          \
    (RING_BUFFER_CAPACITY(_rb_p_) - RING_BUFFER_FREE_SIZE(_rb_p_))

#define __RING_BUFFER_WRITE_NO_CHECK(_rb_p_, _data_, _size_)                   \
    do {                                                                       \
        memcpy((_rb_p_)->buf + (_rb_p_)->write_idx, (_data_), (_size_));       \
        (_rb_p_)->write_idx += (_size_);                                       \
        if ((_rb_p_)->write_idx == (_rb_p_)->size) {                           \
            (_rb_p_)->write_idx = 0;                                           \
        }                                                                      \
    } while (0);

#define __RING_BUFFER_READ_NO_CHECK(_rb_p_, _data_, _size_)                    \
    do {                                                                       \
        memcpy((_data_), (_rb_p_)->buf + (_rb_p_)->read_idx, (_size_));        \
        (_rb_p_)->read_idx += (_size_);                                        \
        if ((_rb_p_)->read_idx == (_rb_p_)->size) {                            \
            (_rb_p_)->read_idx = 0;                                            \
        }                                                                      \
    } while (0);

static inline void
ring_buffer_init(ring_buffer_t *rb, uint8_t *buffer, uint32_t size)
{
    rb->buf       = buffer;
    rb->size      = size;
    rb->read_idx  = 0;
    rb->write_idx = 0;
    return;
}

static inline uint32_t
ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, uint32_t size)
{
    uint32_t write_size       = 0;
    uint32_t total_write_size = 0;
    uint32_t write_tail_size  = 0;
    uint32_t write_head_size  = 0;

    if (rb->write_idx >= rb->read_idx) {
        write_head_size = ((rb->read_idx) ? (rb->read_idx - 1) : (0));
        write_tail_size = RING_BUFFER_FREE_SIZE(rb) - write_head_size;
    } else {
        write_tail_size = 0;
        write_head_size = RING_BUFFER_FREE_SIZE(rb);
    }

#if 0
    DBG("ring buffer wirte: size: %d, free: %d write_idx: %d, read_idx: %d, write_head: %d, write_tail: %d, data_size: %d\n",
        rb->size, RING_BUFFER_FREE_SIZE(rb), rb->write_idx, rb->read_idx, write_head_size, write_tail_size, size);
#endif

    if (write_tail_size && size) {
        write_size = ((write_tail_size > size) ? (size) : (write_tail_size));
        __RING_BUFFER_WRITE_NO_CHECK(rb, data, write_size);
        size -= write_size;
        data += write_size;
        total_write_size += write_size;
    }

    if (write_head_size && size) {
        ASSERT(write_tail_size == write_size);

        write_size = ((write_head_size > size) ? (size) : (write_head_size));
        __RING_BUFFER_WRITE_NO_CHECK(rb, data, write_size);
        size -= write_size;
        data += write_size;
        total_write_size += write_size;
    }

    return total_write_size;
}

static uint32_t
ring_buffer_read(ring_buffer_t *rb, uint8_t *data, uint32_t size)
{
    uint32_t read_size       = 0;
    uint32_t total_read_size = 0;
    uint32_t read_tail_size  = 0;
    uint32_t read_head_size  = 0;

    if (rb->read_idx > rb->write_idx) {
        read_head_size = rb->write_idx;
        read_tail_size = RING_BUFFER_DATA_SIZE(rb) - read_head_size;
    } else {
        read_tail_size = 0;
        read_head_size = RING_BUFFER_DATA_SIZE(rb);
    }

#if 0
    DBG("ring buffer read: size: %d, DATA: %d, write_idx: %d, read_idx: %d, read_head: %d, read_tail: %d, data_size: %d\n",
        rb->size, RING_BUFFER_DATA_SIZE(rb), rb->write_idx, rb->read_idx, read_head_size, read_tail_size, size);
#endif
    if (read_tail_size && size) {
        read_size = ((read_tail_size > size) ? (size) : (read_tail_size));
        __RING_BUFFER_READ_NO_CHECK(rb, data, read_size);
        size -= read_size;
        data += read_size;
        total_read_size += read_size;
    }

    if (read_head_size && size) {
        ASSERT(read_tail_size == read_size);

        read_size = ((read_head_size > size) ? (size) : (read_head_size));
        __RING_BUFFER_READ_NO_CHECK(rb, data, read_size);
        size -= read_size;
        data += read_size;
        total_read_size += read_size;
    }

    return total_read_size;
}

static void ring_buffer_get_linear_data_array(ring_buffer_t *rb,
                                              uint8_t **ptr1,
                                              uint32_t *size1,
                                              uint8_t **ptr2,
                                              uint32_t *size2)
{
    uint32_t read_tail_size = 0;
    uint32_t read_head_size = 0;

    if (rb->read_idx > rb->write_idx) {
        read_head_size = rb->write_idx;
        read_tail_size = RING_BUFFER_DATA_SIZE(rb) - read_head_size;
        *ptr1          = rb->buf + rb->read_idx;
        *size1         = read_tail_size;
        *ptr2          = rb->buf;
        *size2         = read_head_size;
    } else {
        read_tail_size = 0;
        read_head_size = RING_BUFFER_DATA_SIZE(rb);
        *ptr1          = rb->buf + rb->read_idx;
        *size1         = read_head_size;
        *ptr2          = NULL;
        *size2         = 0;
    }
    return;
}
#endif /* __RING_BUFFER_H__ */
