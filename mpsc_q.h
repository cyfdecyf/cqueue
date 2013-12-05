#ifndef __MPSC_Q_H
#define __MPSC_Q_H

#include <stdbool.h>

/* Multiple Producer Single Consumer bounded size queue. */
struct mpsc_q;

struct mpsc_q *mpsc_q_new(int size);
void  mpsc_q_free(struct mpsc_q *q);

void mpsc_q_push(struct mpsc_q *q, void *e);
void *mpsc_q_pop(struct mpsc_q *q); /* Block if queue is empty. */
bool mpsc_q_empty(struct mpsc_q *q);

#endif /* __MPSC_Q_H */
