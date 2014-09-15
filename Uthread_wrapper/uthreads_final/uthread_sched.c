/*
 *   FILE: uthread_sched.c 
 * AUTHOR: Yan Li
 *  DESCR: scheduling wack for uthreads
 *   DATE: Thu Feb 20 08:19:51 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>
#include <string.h>
#include "uthread.h"
#include "uthread_private.h"
#include "uthread_ctx.h"
#include "uthread_queue.h"
#include "uthread_bool.h"
#include "uthread_sched.h"


static utqueue_t  runq_table[UTH_MAXPRIO + 1];	/* priority runqueues */


/*
 * uthread_yield
 *
 * Causes the currently running thread to yield use of the processor to
 * another thread. 
 */
void
uthread_yield(void)
{
	/* Yield the processor to somebody */
	/* Always succeeds */
	/* When we call this function, the thread is not in CPU anymore */
	int offset = ut_curthr - &uthreads[0];
	/* Make sure it's not hack */
	if (offset >= 0 && 
		offset < UTH_MAX_UTHREADS &&
		ut_curthr->ut_state == UT_ON_CPU)
	{
		ut_curthr->ut_state = UT_RUNNABLE;
		/* Set the priority */
		if (uthread_setprio(ut_curthr->ut_id, ut_curthr->ut_prio) != 0)
		{
			char pbuffer[256] = {0};
			sprintf(pbuffer, "error in setprio\n");  
			int ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
			if (ret < 0) 
			{
				perror("write");
			}
			exit(EXIT_FAILURE);
		}
	}
	/* do switch */
	uthread_switch();
}



/*
 * uthread_block
 *
 * Put the current thread to sleep, pending an appropriate call to 
 * uthread_wake().
 */
void
uthread_block(void) 
{
	/* Already waiting? deadlock */
	if (ut_curthr->ut_state == UT_WAIT)
	{
		uthread_switch();
	}
	/* If the current thread is on cpu, then do context switch */
	if (ut_curthr->ut_state == UT_ON_CPU)
	{
		ut_curthr->ut_state = UT_WAIT;
		uthread_switch();
	}
	/* If the current thread is in the queue, remove it from the queue */
	else if (ut_curthr->ut_state == UT_RUNNABLE)
	{
		ut_curthr->ut_state = UT_WAIT;
		utqueue_remove(&runq_table[ut_curthr->ut_prio], ut_curthr);
	}
}


/*
 * uthread_wake
 *
 * Wakes up the supplied thread (schedules it to be run again).
 * uthr: the pointer to the uthread object
 */
void
uthread_wake(uthread_t *uthr)
{
	assert(uthr != NULL && uthr->ut_state != UT_NO_STATE);
	/* Only wake up him if he is waiting! */
	if (uthr->ut_state == UT_WAIT)
	{
		uthr->ut_state = UT_RUNNABLE;
		/* Put it into the appropriate queue */
		if (uthread_setprio(uthr->ut_id, uthr->ut_prio) != 0)
		{
			char pbuffer[256] = {0};
			sprintf(pbuffer, "error in setprio\n");  
			int ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
			if (ret < 0) 
			{
				perror("write");
			}
			exit(EXIT_FAILURE);
		}
	}
}


/*
 * uthread_setprio
 *
 * Changes the priority of the indicated thread. 
 * param id: the uthread id
 * param prio: the priority to be set
 */
int
uthread_setprio(uthread_id_t id, int prio)
{
    
	/* If prio is invalid */
	if (prio < 0 || prio > UTH_MAXPRIO)
	{
		ut_curthr->ut_errno = EINVAL;
		return EINVAL;
	}

	/* Invalid thread id */
	if (id < 0 || id >= UTH_MAX_UTHREADS ||
	    uthreads[id].ut_state == UT_NO_STATE ||
	    uthreads[id].ut_state == UT_ZOMBIE)
	{
		ut_curthr->ut_errno = ESRCH;
		return ESRCH;
	}

	/* If the current state is runnable */
	if (uthreads[id].ut_state == UT_RUNNABLE)
	{
		if (uthreads[id].ut_link.l_prev != NULL && 
			uthreads[id].ut_link.l_next != NULL)
		utqueue_remove(&runq_table[uthreads[id].ut_prio], &uthreads[id]);
		uthreads[id].ut_prio = prio;
		utqueue_enqueue(&runq_table[prio], &uthreads[id]);
	}
	else 
	{
		/* If it is waiting or on CPU, just set the priority */
		/* Because it's not in the queue! */
		uthreads[id].ut_prio = prio;
	}
	return 0;
}


/*
 * uthread_switch()
 *
 * Wait until there is a runnable thread, and then switch to it using 
 * uthread_swapcontext(). 
 *
 */
void
uthread_switch(void)
{
	int i = 0;
	while (1)
	{
		uthread_idle();
		for(i = UTH_MAXPRIO; i >= 0; i--)
		{
			if (!utqueue_empty(&runq_table[i]))
			{
				uthread_t* backup = ut_curthr;
				uthread_t* new_thread = utqueue_dequeue(&runq_table[i]);

				assert(new_thread->ut_state == UT_RUNNABLE);
				assert(new_thread->ut_link.l_prev == NULL &&
				       new_thread->ut_link.l_next == NULL);
				/* Swap the new thread */
				ut_curthr = new_thread;
				ut_curthr->ut_state = UT_ON_CPU;
				/* Swap context */
				uthread_swapcontext(&backup->ut_ctx, &ut_curthr->ut_ctx);
				return;
			}
		}
	}
}



/*
 * uthread_sched_init
 *
 * Setup the scheduler. This is called once from uthread_init().
 */
void
uthread_sched_init(void)
{	
    /* Just initialize the queue */
	int i;
	for(i = 0; i < UTH_MAXPRIO + 1; i++)
	{
		utqueue_init(&runq_table[i]);
	}
}
