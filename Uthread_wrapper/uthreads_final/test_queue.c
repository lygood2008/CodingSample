/*
 *   FILE: test_queue.c 
 * AUTHOR: Yan Li (yanli)
 *  DESCR: a simple concurrent queue for multiple threads
 *   DATE: Feb 27 11:30 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "uthread.h"
#include "uthread_mtx.h"
#include "uthread_cond.h"
#include "uthread_sched.h"

#define MAX_QUEUE_SIZE 5   /* Max queue size*/
#define ENQUEUER       10  /* Enqueuers */
#define DEQUEUER       10  /* Dequeuers */
#define BUF_SIZE       256 /* Buffer size */
#define OPS            10  /* Operations */

/* uthread stuff */
uthread_id_t thr_enq[ENQUEUER];
uthread_id_t thr_deq[DEQUEUER];

uthread_mtx_t	mtx_enq;
uthread_mtx_t   mtx_deq;
uthread_cond_t	cond_enq;
uthread_cond_t  cond_deq;


volatile int counter = 0; // shared counter

/*
 * struct: node
 * Brief: single node in queue
 */
typedef struct node
{
	struct node* next;
	int val;
}node_t;

/*
 * struct: queue
 * Brief: the queue structure. It's indeed a linked list
 */
typedef struct queue
{
	node_t* head;
	node_t* tail;
	int size;
}  queue_t;

queue_t q;

/*
 * queue_init
 * Initialize the queue
 * param *q: pointer to the queue object
 */
static void
queue_init(queue_t *q)
{
	assert(q != NULL);

	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
}

/*
 * queue_empty
 * Is the queue empty?
 * param q: pointer to the queue object
 * return: 1 for empty and 0 for not empty
 */
static int
queue_empty(queue_t *q)
{
	assert(q != NULL);
	return (q->size == 0);
}

/*
 * queue_full
 * Is the queue full?
 * param q: the pointer to q object
 * return: 1 for full and 0 for not full
 */
static int 
queue_full(queue_t *q)
{
	return q->size == MAX_QUEUE_SIZE;
}

/*
 * queue_enqueue
 * Enqueue an item into the queue
 * param *q: pointer to the queue object
 * param num: the new item
 * return: 0 for success, -1 for failure
 */
static int
queue_enqueue(queue_t *q, int num)
{
	if (queue_full(q))
		return -1;
	else
	{
		node_t* newNode = (node_t*)malloc(sizeof(newNode));
		newNode->val = num;
		if (q->head == NULL)
		{
			q->head = q->tail = newNode;
		}
		else
		{
			q->tail->next = newNode;
			q->tail = q->tail->next;
		}
		q->size++;
		return 0;
	}
}

/*
 * queue_dequeue
 * Dequeue an item from the queue
 * param *q: the pointer to the queue object
 * return: the size after dequeuing, or -1 for underflow
 */
static int
queue_dequeue(queue_t *q)
{
	if (queue_empty(q))
	{
		return -1;
	}
	else
	{
		node_t* head = q->head;
		q->head = head->next;
		int tmp = head->val;
		free(head);
		if (q->head == NULL)
		{
			q->tail = NULL;
		}
		q->size--;
		return tmp;
	}
}

/*
 * enqueuer
 * Thread for enqueuer
 * param a0: the argument list length
 * param *a1: the argument list
 */
static void
enqueuer(long a0, void *a1)
{
	int i = 0, ret = 0;
	char pbuffer[BUF_SIZE];

	while (i < OPS)
	{
		int must_wake = 0;
		uthread_mtx_lock(&mtx_enq);

		while (queue_full(&q))
		{
			uthread_cond_wait(&cond_enq, &mtx_enq);
		}

		queue_enqueue(&q, counter);
		if (q.size == MAX_QUEUE_SIZE)
		{
			must_wake = 1;
		}
		sprintf(pbuffer, "thread %i: enqueue! (%i)\n", uthread_self(),counter);  
		counter++;
		i++;
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write:");
            exit(EXIT_FAILURE);
        }
		
		uthread_mtx_unlock(&mtx_enq);

		if (must_wake == 1)
		{
			uthread_mtx_lock(&mtx_deq);
			uthread_cond_broadcast(&cond_deq);
			uthread_mtx_unlock(&mtx_deq);
		}
	}
	uthread_exit(0);
}

/*
 * dequeuer
 * Thread for dequeuer
 * param a0: the argument list length
 * param a1: the argument list
 */
static void 
dequeuer(long a0, void *1)
{
	int i = 0, ret = 0;
	char pbuffer[BUF_SIZE];

	while (i < OPS)
	{
		int must_wake = 0;
		uthread_mtx_lock(&mtx_deq);

		while (queue_empty(&q))
		{
			uthread_cond_wait(&cond_deq, &mtx_deq);
		}

		int val = queue_dequeue(&q);
		if (q.size == 0)
		{
			must_wake = 1;
		}
		sprintf(pbuffer, "thread %i: dequeue! (%i)\n", uthread_self(),val);  
		ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
		i++;
		
        if (ret < 0) 
        {
            perror("uthreads_test");
            exit(EXIT_FAILURE);
        }

		uthread_mtx_unlock(&mtx_deq);

		if (must_wake == 1)
		{
			uthread_mtx_lock(&mtx_enq);
			uthread_cond_broadcast(&cond_enq);
			uthread_mtx_unlock(&mtx_enq);
		}
	}
	uthread_exit(0);
}


int
main(int ac, char **av)
{
	/* Initialize the queue */
	queue_init(&q);
	
	/* Initialize uthread global stuff */
    uthread_init();

    uthread_mtx_init(&mtx_enq);
    uthread_mtx_init(&mtx_deq);
	uthread_cond_init(&cond_enq);
	uthread_cond_init(&cond_deq);

	int i;
	
	/* Create enqueuers */
	for(i = 0; i < ENQUEUER; i++)
	{
		uthread_create(&thr_enq[i], enqueuer, i, NULL, 0);
	}

	/* Create dequeuers */
	for(i = 0; i < DEQUEUER; i++)
	{
		uthread_create(&thr_deq[i], dequeuer, i+ENQUEUER, NULL, 0);
	}

    /* Wait for dequeuers to finish */
	int	tmp = 1, ret = 0;
	for (i = 0; i < DEQUEUER; i++)
    {
        char pbuffer[BUF_SIZE];
		uthread_join(thr_deq[i], &tmp);
        sprintf(pbuffer, "joined with dequeue thread %i, exited %i.\n",
        	    thr_deq[i], tmp);  
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            return EXIT_FAILURE;
		}
	}

    /* Wait for enqueuers to finish */
	for (i = 0; i < ENQUEUER; i++)
    {
        char pbuffer[BUF_SIZE];
		uthread_join(thr_enq[i], &tmp);
        sprintf(pbuffer, "joined with enqueue thread %i, exited %i.\n",
        	    thr_enq[i], tmp);  
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            return EXIT_FAILURE;
		}
	}

	/* uthread exit */
	uthread_exit(0);
	return 0;
}


