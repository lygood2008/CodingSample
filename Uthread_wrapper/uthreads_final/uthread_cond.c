/*
 *   FILE: uthread_cond.c 
 * AUTHOR: Yan Li
 *  DESCR: uthreads condition variables
 *   DATE: Sat Feb  22 09:59 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"
#include "uthread_mtx.h"
#include "uthread_cond.h"
#include "uthread_queue.h"
#include "uthread_sched.h"


/*
 * uthread_cond_init
 *
 * Initialize the given condition variable
 * param cond: the pointer to the conditional object
 */
void
uthread_cond_init(uthread_cond_t *cond)
{
	assert(cond != NULL);

	utqueue_init(&cond->uc_waiters);
}


/*
 * uthread_cond_wait
 *
 * Should behave just like a stripped down version of pthread_cond_wait.
 * Block on the given condition variable.
 * param cond: the pointer to the conditional variable object
 * param mtx: the pointer to the mutex object
 */
void
uthread_cond_wait(uthread_cond_t *cond, uthread_mtx_t *mtx)
{
	/* Make sure the mtx is locked by the current thread */

	assert(mtx != NULL && cond != NULL);
	assert(mtx->m_owner == ut_curthr);
	

	utqueue_enqueue(&cond->uc_waiters, ut_curthr);
	uthread_mtx_unlock(mtx);

	/* Block on the conditional variable */
	uthread_block();

	/* Now it gets back, try lock the mutex */
	uthread_mtx_lock(mtx);
}


/*
 * uthread_cond_broadcast
 *
 * Wakeup all the threads waiting on this condition variable.
 * param cond: the pointer to the conditional variable object
 */
void
uthread_cond_broadcast(uthread_cond_t *cond)
{
	assert(cond != NULL);

	while (!utqueue_empty(&cond->uc_waiters))
	{
		uthread_wake(utqueue_dequeue(&cond->uc_waiters));
	}
}



/*
 * uthread_cond_signal
 *
 * Wakeup just one thread waiting on the condition variable.
 * Note there may be no threads waiting.
 * param cond: the pointer to the conditional variable object
 */
void
uthread_cond_signal(uthread_cond_t *cond)
{
	assert(cond != NULL);
	
	if (!utqueue_empty(&cond->uc_waiters))
	{
		uthread_wake(utqueue_dequeue(&cond->uc_waiters));
	}
}
