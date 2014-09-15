/*
 *   FILE: uthread_queue.c 
 * AUTHOR: Yan Li
 *  DESCR: queues of threads.
 *   DATE: Feb 20 9:25 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"
#include "uthread_queue.h"


/*
 * utqueue_init
 * Initialize the queue
 * param q: the pointer to the queue object
 */
void
utqueue_init(utqueue_t *q)
{
	assert(q != NULL);
	q->tq_size = 0;
	list_init(&q->tq_waiters);
}

/*
 * utqueue_empty
 * Is the list empty?
 * param *q: the pointer to the queue object
 * return: 1 for empty and 0 for not empty
 */
int
utqueue_empty(utqueue_t *q)
{
	assert(q != NULL);
	assert(list_empty(&q->tq_waiters) == (q->tq_size == 0));

	return (q->tq_size == 0);
}

/*
 * utqueue_enqueue
 * Add a thread onto the front of the queue
 * param q: the pointer to the queue object
 * param thr: the pointer to the uthread object
 */
void
utqueue_enqueue(utqueue_t *q, uthread_t *thr)
{
	assert(thr->ut_link.l_next == NULL && thr->ut_link.l_prev == NULL);

	list_insert_head(&q->tq_waiters, &thr->ut_link);
	q->tq_size++;
}



/*
 * utqueue_dequeue
 * Remove element from the list
 * param *q: the pointer to the queue object
 * return: the pointer to the uthread object
 */
uthread_t *
utqueue_dequeue(utqueue_t *q)
{
	uthread_t	*thr  = NULL;
	list_link_t	*link = NULL;

	assert(q != NULL);

	if (utqueue_empty(q))
	{
		return NULL;
	}

	link = q->tq_waiters.l_prev;
	thr = list_item(link, uthread_t, ut_link);
	list_remove(link);

	q->tq_size--;

	return thr;
}



/*
 * utqueue_remove
 * Remove given thread from queue
 * param *q: the pointer to the queue object
 * param *thr: the pointer to the uthread object
 */
void
utqueue_remove(utqueue_t *q, uthread_t *thr)
{
	assert(thr->ut_link.l_next != NULL && thr->ut_link.l_prev != NULL);

	list_remove(&thr->ut_link);
	q->tq_size--;
}
