/*
 *   FILE: uthread_queue.h 
 * AUTHOR: Yan Li
 *  DESCR: a queue of threads
 *   DATE: Feb 20 9:25 2014
 *
 */

#ifndef __uthread_queue_h__
#define __uthread_queue_h__

#include "list.h"


struct uthread;

/*
 * struct utqueue
 * Brief: queue structure storing uthreads, using linked list
 */
typedef struct utqueue {
	list_t	tq_waiters;
	int	tq_size;
} utqueue_t;

/*
 * utqueue_init
 * Initialize the queue
 * param q: the pointer to the queue object
 */
void utqueue_init(utqueue_t *q);

/*
 * utqueue_empty
 * Is the list empty?
 * param *q: the pointer to the queue object
 * return: 1 for empty and 0 for not empty
 */
int utqueue_empty(utqueue_t *q);

/*
 * utqueue_enqueue
 * Add a thread onto the front of the queue
 * param q: the pointer to the queue object
 * param thr: the pointer to the uthread object
 */
void utqueue_enqueue(utqueue_t *q, struct uthread *thr);

/*
 * utqueue_dequeue
 * Remove element from the list
 * param *q: the pointer to the queue object
 * return: the pointer to the uthread object
 */
struct uthread *utqueue_dequeue(utqueue_t *q);

/*
 * utqueue_remove
 * Remove given thread from queue
 * param *q: the pointer to the queue object
 * param *thr: the pointer to the uthread object
 */
void utqueue_remove(utqueue_t *q, struct uthread *thr);

#endif /* __uthread_queue_h__ */
