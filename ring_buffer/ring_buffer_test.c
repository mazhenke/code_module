#include "ring_buffer.h"
#include "fake_random.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define RING_BUFFER_SIZE (100)
static uint8_t buffer[RING_BUFFER_SIZE];

static uint8_t test_buf0[RING_BUFFER_SIZE * 2];
static uint8_t test_buf1[RING_BUFFER_SIZE * 2];
static uint8_t test_buf2[RING_BUFFER_SIZE * 2];

static ring_buffer_t rb;

static void test_buf_init(void)
{
    uint32_t i = 0;

    for (i = 0; i < RING_BUFFER_SIZE * 2; i++) {
        test_buf0[i] = i;
    }

    memset(test_buf1, 0, RING_BUFFER_SIZE * 2);
}

void paint_point(uint32_t x, uint32_t base)
{
    int i;
    for (i = 0; i < base; i++) {
        if (x >= base) {
            DBG("@");
        } else if (x == i) {
            DBG("*");
        } else {
            DBG(" ");
        }
    }
}

int test_rb_v0(void)
{
    uint32_t ret = 0;
    uint32_t i, j;

    ring_buffer_init(&rb, buffer, RING_BUFFER_SIZE);
    test_buf_init();

    ASSERT(RING_BUFFER_CAPACITY(&rb) == RING_BUFFER_SIZE - 1);
    ASSERT(IS_RING_BUFFER_EMPYT(&rb));
    ASSERT(!IS_RING_BUFFER_FULL(&rb));
    ASSERT(RING_BUFFER_FREE_SIZE(&rb) == RING_BUFFER_SIZE - 1);
    ASSERT(RING_BUFFER_DATA_SIZE(&rb) == 0);

    test_buf_init();
    ret = ring_buffer_write(
        &rb, (const uint8_t *)test_buf0, RING_BUFFER_SIZE * 2);
    ASSERT(ret == RING_BUFFER_CAPACITY(&rb));

    ret = ring_buffer_read(&rb, (uint8_t *)test_buf1, RING_BUFFER_SIZE * 2);
    ASSERT(ret == RING_BUFFER_CAPACITY(&rb));

    DBG("Test Write!!!!\n");
    /* test write */
    ring_buffer_init(&rb, buffer, RING_BUFFER_SIZE);
    for (i = 0; i < RING_BUFFER_SIZE; i++) {
        DBG("=====================================\n");
        for (j = 0; j < RING_BUFFER_SIZE; j++) {
            rb.read_idx  = i;
            rb.write_idx = j;

            ret = ring_buffer_write(
                &rb, (const uint8_t *)test_buf0, RING_BUFFER_SIZE * 2);
            //            DBG("read_idx: %02d, write_idx: %02d, write: %02d\n",
            //            i, j, ret);
            paint_point(i, RING_BUFFER_SIZE);
            paint_point(j, RING_BUFFER_SIZE);
            paint_point(ret, RING_BUFFER_SIZE);
            printf("\n");
        }
        DBG("=====================================\n");
    }

    DBG("Test Read!!!!\n");
    /* test read */
    ring_buffer_init(&rb, buffer, RING_BUFFER_SIZE);
    for (i = 0; i < RING_BUFFER_SIZE; i++) {
        DBG("=====================================\n");
        for (j = 0; j < RING_BUFFER_SIZE; j++) {
            rb.read_idx  = i;
            rb.write_idx = j;

            ret = ring_buffer_read(&rb, test_buf1, RING_BUFFER_SIZE * 2);
            // DBG("read_idx: %02d, write_idx: %02d, read: %02d\n", i, j, ret);
            paint_point(i, RING_BUFFER_SIZE);
            paint_point(j, RING_BUFFER_SIZE);
            paint_point(ret, RING_BUFFER_SIZE);
            printf("\n");
        }
        DBG("=====================================\n");
    }

    return 0;
}

static sem_t sem;
static uint8_t _test_stop         = 0;
static uint32_t _g_randseed_write = 12345;
uint32_t rand_uint32_write(void)
{
    return (_g_randseed_write = _g_randseed_write * 1664525 + 1013904223);
}

static uint32_t _g_randseed_read = 12345;
uint32_t rand_uint32_read(void)
{
    return (_g_randseed_read = _g_randseed_read * 1664525 + 1013904223);
}

void *write_thread(void *arg)
{
    uint32_t size = 0;
    uint8_t *p    = NULL;
    uint32_t i;
    uint32_t ret;
    while (1) {
        if (_test_stop) {
            goto finish;
        }

        /* generate random size */
        p = test_buf0;

        size = get_system_rand_limit(1, (RING_BUFFER_SIZE / 3) * 2);
        DBG("[W]  size: %d\n", size);
        for (i = 0; i < size; i++) {
            p[i] = (rand_uint32_write() & 0xff);
        }

        /* write */
        do {
            sem_wait(&sem);
            // DBG("[W]  START \n");
            ret = ring_buffer_write(&rb, (const uint8_t *)p, size);
            // DBG("[W]  END \n");
            sem_post(&sem);
            pthread_yield();

            p += ret;
            size -= ret;
        } while (size);
    }
finish:
    return NULL;
}

void *read_thread(void *arg)
{
    uint32_t size = 0;
    uint8_t *p    = NULL;
    uint32_t i;
    uint32_t ret;
    while (1) {
        /* generate random size */
        p = test_buf1;

        size = get_system_rand_limit(1, (RING_BUFFER_SIZE / 3) * 2);
        DBG("[R]  size: %d\n", size);
        memset(p, 0, size);

        /* read */
        do {
            sem_wait(&sem);
            // DBG("[R]  START \n");
            ret = ring_buffer_read(&rb, (uint8_t *)p, size);
            // DBG("[R]  END \n");
            sem_post(&sem);
            pthread_yield();

            p += ret;
            size -= ret;
        } while (size);

        for (i = 0; i < size; i++) {
            uint8_t expect_data = (rand_uint32_read() & 0xff);
            if (p[i] != expect_data) {
                DBG("Error!!! mismatch: 0x%x -- 0x%x\n", p[i], expect_data);
                _test_stop = 1;
                goto finish;
            }
        }
    }
finish:
    return NULL;
}

int test_rb_v1(void)
{
    int ret = 0;
    pthread_t write_tid;
    pthread_t read_tid;

    ring_buffer_init(&rb, buffer, RING_BUFFER_SIZE);

    ret = sem_init(&sem, 0, 1);
    if (ret == -1) {
        ERR("semaphore intitialization failed\n");
        return -1;
    }

    ret = pthread_create(&write_tid, NULL, write_thread, NULL);
    if (ret != 0) {
        ERR("create thread error: %s/n", strerror(ret));
        return -1;
    }

    ret = pthread_create(&read_tid, NULL, read_thread, NULL);
    if (ret != 0) {
        ERR("create thread error: %s/n", strerror(ret));
        return -1;
    }
    pthread_join(read_tid, NULL);
    pthread_join(write_tid, NULL);

    sem_destroy(&sem);

    return 0;
}
int main(void)
{
    return test_rb_v1();
}
