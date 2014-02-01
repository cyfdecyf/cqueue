#include "mpsc_q.h"

#include "debug.h"
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

/* Avoid compiler reorder. */
#define barrier() asm volatile ("" : : : "memory")

/* Multi Producer Single Consumer bounded size queue. */
struct mpsc_q {
    void **arr; /* circular array */
    bool *w;    /* mark whether a slot has been written */
    int n;      /* size of array */

    int64_t head; /* use 64bit integer to avoid overflow */
    int64_t tail;
};

struct mpsc_q *mpsc_q_new(int size)
{
    assert(size > 0);
    struct mpsc_q *q = calloc(1, sizeof(struct mpsc_q));
    q->w = calloc(size, sizeof(bool));
    q->arr = calloc(size, sizeof(void **));
    q->n = size;
    return q;
}

void mpsc_q_free(struct mpsc_q *q)
{
    free(q->arr);
    free(q->w);
    q->arr = NULL;
    free(q);
}

void mpsc_q_push(struct mpsc_q *q, void *e)
{
    assert(q->tail >= q->head);

    int64_t tail = __sync_fetch_and_add(&q->tail, 1);
    int64_t idx = tail % q->n;
    while (tail - q->head >= q->n) {
        DPRINTLN("%x push: array full", (int)pthread_self());
        sched_yield();
    }
    DPRINTLN("%x push: head %lld tail %lld %ld|%ld", (int)pthread_self(),
        q->head, tail, (long)e >> 32, (long)e & 0xffffffff);
    /* First store element, then mark it as written.
     * x86 memory model ensures correctness. */
    q->arr[idx] = e;
    barrier();
    assert(!q->w[idx]);
    q->w[idx] = true;
}

void *mpsc_q_pop(struct mpsc_q *q)
{
    while (q->head == q->tail) {
        sched_yield();
    }

    int idx = q->head % q->n;
    while (!q->w[idx]) {
        /* Producer has not made the write, wait a short time. */
        asm volatile ("pause\n" : : : "memory");
    }
    void *r = q->arr[idx];
    /* After pop, mark slot as not written. */
    q->w[idx] = false;
    DPRINTLN("%x pop : head %lld tail %lld %ld|%ld", (int)pthread_self(),
        q->head, q->tail, (long)r >> 32, (long)r & 0xffffffff);
    barrier();
    q->head++;
    assert(q->tail >= q->head);
    return r;
}

bool mpsc_q_empty(struct mpsc_q *q)
{
    assert(q->tail >= q->head);
    return q->head == q->tail;
}
