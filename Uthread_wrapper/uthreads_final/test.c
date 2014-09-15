/*
 *  FILE: test_mtx.c 
 *  AUTHOR: Yan Li (yanli)
 *  DESCR: a simple test program for uthreads mtx.
 *  DATE:  Feb 27 11:02 2014
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

#define BUF_SIZE 256 /* Buffer size */
#define LOOP     10  /* How many loops? */

volatile int counter = 0; // Shared counter

/* uthread global stuff */

uthread_id_t    thr_1;
uthread_id_t    thr_2;
uthread_mtx_t	mtx_1;
uthread_mtx_t   mtx_2;
uthread_cond_t	cond;
        
/*
 * tester
 * Test thread. The thread will yield to other thread when it's holding locks.
 * The other thread will block there and return the CPU to the current thread
 * param a0: the argument list length
 * param *a1: the argument list
 */
static void
tester(long a0, void *a1)
{
    int	i = 0, ret = 0;
    char pbuffer[BUF_SIZE] = {0};
    
    while (i < LOOP)
    {
		uthread_mtx_lock(&mtx_1);
		uthread_mtx_lock(&mtx_2);

		i++;
		sprintf(pbuffer, "thread %i is in the critical section.\n", 
                uthread_self());  
		ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
		if (ret < 0) 
		{
			perror("uthreads_test");
			exit(EXIT_FAILURE);
		}

		/* Yield the processor */
		if(uthread_self() == 2)
			uthread_yield();
	    uthread_mtx_unlock(&mtx_2);
		uthread_mtx_unlock(&mtx_1);
		sprintf(pbuffer, "thread %i leaves the critical section.\n", 
                uthread_self());  
		ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
		if (ret < 0) 
		{
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

    sprintf(pbuffer, "thread %i exiting.\n", uthread_self());  
    ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
    if (ret < 0) 
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    uthread_exit(a0);
}

int
main(int ac, char **av)
{
    /* uthread initialization */
    uthread_init();

	/* Initialize the mutexes*/
    uthread_mtx_init(&mtx_1);
    uthread_mtx_init(&mtx_2);
	uthread_cond_init(&cond);

	uthread_create(&thr_1,tester, 0, NULL, 0);
	uthread_create(&thr_2,tester, 0, NULL, 0);

	char pbuffer[BUF_SIZE] = {0};
	int	tmp = 1, ret = 0;
	
    /* Join thread 1 */
	uthread_join(thr_1, &tmp);
	sprintf(pbuffer, "joined with thread %i, exited %i.\n", thr_1, tmp);  
	ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
	if (ret < 0) 
	{
		perror("write");
		return EXIT_FAILURE;
	}

    /* Join thread 2*/
	uthread_join(thr_2, &tmp);
	sprintf(pbuffer, "joined with thread %i, exited %i.\n", thr_2, tmp);  
	ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
	if (ret < 0) 
	{
		perror("write");
		return EXIT_FAILURE;
	}
	
    /* thread exit */
	uthread_exit(0);
    return 0;
}
