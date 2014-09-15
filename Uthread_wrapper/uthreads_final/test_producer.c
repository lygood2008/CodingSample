/*
 *   FILE: test_producer.c 
 * AUTHOR: Yan Li (yanli)
 *  DESCR: a simple program of producer-consumer problem
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

#define SLOT_SIZE 2  /* Slot size */
#define PRODUCER  10 /* Producers */
#define CONSUMER  10 /* Consumers */
#define BUF_SIZ   256 /* Buffer size*/
#define OPS       10 /* Operations */

uthread_id_t thr_enq[PRODUCER]; // Producer threads 
uthread_id_t thr_deq[CONSUMER]; // Consumer threads 

uthread_mtx_t	mtx_enq;
uthread_mtx_t   mtx_deq;
uthread_cond_t	cond_enq;
uthread_cond_t  cond_deq;

volatile int counter = 0; /* Shared counter */

int slots[SLOT_SIZE] = {0}; // Slots, farm!

/*
 * producer
 * Producer thread, in charge of producing item in slots
 * a0: the argument list length
 * *a1: the argument list
 */
static void
producer(long a0, void *a1)
{
	int i = 0, ret = 0;
	char pbuffer[BUF_SIZ] = {0};

	while(i < OPS)
	{
		int must_wake = 0;
		uthread_mtx_lock(&mtx_enq);

		// Wait
		while(counter == SLOT_SIZE)
		{
			uthread_cond_wait(&cond_enq, &mtx_enq);
		}

		slots[counter++] = 1;
		if(counter == SLOT_SIZE)
		{
			must_wake = 1;
		}
		sprintf(pbuffer, "thread %i: produce!\n", uthread_self());  
		
		i++;
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

		uthread_mtx_unlock(&mtx_enq);

		if(must_wake == 1)
		{
			uthread_mtx_lock(&mtx_deq);
			uthread_cond_broadcast(&cond_deq);
			uthread_mtx_unlock(&mtx_deq);
		}
	}
	uthread_exit(0);
}

/*
 * consumer
 * Consumer thread, in charge of consuming item in slot
 * a0: the argument list length
 * *a1: the argument list
 */
static void 
consumer(long a0, void *a1)
{
	int i = 0, ret = 0;
	char pbuffer[BUF_SIZ] = {0};

	while(i < OPS)
	{
		int must_wake = 0;
		uthread_mtx_lock(&mtx_deq);

		while(counter == 0)
		{
			uthread_cond_wait(&cond_deq, &mtx_deq);
		}

		slots[counter--] = 0;
		if(counter == 0)
		{
			must_wake = 1;
		}
		sprintf(pbuffer, "thread %i: consume\n", uthread_self());  
        i++;

		ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

		uthread_mtx_unlock(&mtx_deq);
		if(must_wake == 1)
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
	/* uhtead stuff */
    uthread_init();

    uthread_mtx_init(&mtx_enq);
    uthread_mtx_init(&mtx_deq);
	uthread_cond_init(&cond_enq);
	uthread_cond_init(&cond_deq);

	int i;
	
	/* Creat the producer threads */
	for(i = 0; i < PRODUCER; i++)
	{
		uthread_create(&thr_enq[i], producer, i, NULL, 0);
	}

    /* Create the consumer threads */
	for(i = 0; i < CONSUMER; i++)
	{
		uthread_create(&thr_deq[i], consumer, i+PRODUCER, NULL, 0);
	}

	int	tmp = 1, ret = 0;
	
	/* Wait for consumer threads to finish */
	for (i = 0; i < CONSUMER; i++)
    {
        char pbuffer[BUF_SIZ] = {0};
		uthread_join(thr_deq[i], &tmp);
        sprintf(pbuffer, "joined with consumer thread %i, exited %i.\n",
         	    thr_deq[i], tmp);  
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            return EXIT_FAILURE;
		}
	}

    /* Wait for  producer threads to finish */
	for (i = 0; i < PRODUCER; i++)
    {
        char pbuffer[BUF_SIZ] = {0};
		uthread_join(thr_enq[i], &tmp);
        sprintf(pbuffer, "joined with producer thread %i, exited %i.\n",
                thr_enq[i], tmp);  
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            return EXIT_FAILURE;
		}
	}

	/* thread exit */
	uthread_exit(0);
	return 0;
}


