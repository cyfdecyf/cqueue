#include "mpsc_q.h"

#include "debug.h"

#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NPRODUCER 8
#define QUEUE_SIZE 4

#define ITER 100000

static struct mpsc_q *queue;

void *producer(void *d)
{
    long i;
    long pid = (long)d;

    for (i = 0; i < ITER; i++) {
        /* message: thread id | i */
        /*printf("producer %ld|%ld\n", pid, i);*/
        mpsc_q_push(queue, (void *)((pid<<32) | i));
    }

    return NULL;
}

void *consumer(void *dummy)
{
    long i;
    long expect[NPRODUCER];

    memset(expect, 0, sizeof(expect));

    for (i = 0; i < NPRODUCER*ITER; i++) {
        void *msg = mpsc_q_pop(queue);

        long pid = (long)msg >> 32;
        long val = (long)msg & 0xffffffff;
        DPRINTF("%x msg %ld: %ld|%ld\n", (int)pthread_self(), i, pid, val);
        if (val != expect[pid]) {
            printf("[ERROR] %x msg %ld: %ld|%ld, expected val %ld\n",
                    (int)pthread_self(), i, pid, val, expect[pid]);
            exit(1);
        }
        expect[pid]++;
    }

    return NULL;
}

int main(int argc, const char *argv[])
{
    int i;
    pthread_t thr[NPRODUCER+1];

    queue = mpsc_q_new(QUEUE_SIZE);

    for (i = 0; i < NPRODUCER; i++) {
        pthread_create(&thr[i], NULL, producer, (void *)(long)i);
    }
    pthread_create(&thr[NPRODUCER], NULL, consumer, NULL);

    for (i = 0; i < NPRODUCER+1; i++) {
        pthread_join(thr[i], NULL);
    }

    mpsc_q_free(queue);
    return 0;
}
