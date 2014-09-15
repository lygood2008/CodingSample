/*
 *   FILE: uthread_mtx.c 
 * AUTHOR: Yan Li
 *  DESCR: userland mutexes
 *   DATE: Sat Feb 22 10:40 2014
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "list.h"
#include "uthread.h"
#include "uthread_mtx.h"
#include "uthread_sched.h"

/*
 * uthread_mtx_init
 *
 * Initialize the fields of the specified mutex.
 * param mtx: the pointer to the mutex object
 */
void
uthread_mtx_init(uthread_mtx_t *mtx)
{
	assert(mtx != NULL);

	/* Initialize the queue */
	mtx->m_owner = NULL;
	utqueue_init(&mtx->m_waiters);
}



/*
 * uthread_mtx_lock
 *
 * Lock the mutex.  This call will block if it's already locked.
 * param mtx: the pointer to the mutex object
 */
void
uthread_mtx_lock(uthread_mtx_t *mtx)
{
	assert(mtx != NULL);
 
	/* If the owner is null, then no one is using the lock */
	if(mtx->m_owner == NULL)
	{
		mtx->m_owner = ut_curthr;
	}
	else
	{
		if(ut_curthr != mtx->m_owner)
		{
			utqueue_enqueue(&mtx->m_waiters,ut_curthr);
	   	}
        /* Block the current thread */
		uthread_block();
	}
}


/*
 * uthread_mtx_trylock
 *
 * Try to lock the mutex, return 1 if we get the lock, 0 otherwise.
 * This call should not block.
 * param mtx: the pointer to the mutex object
 */
int
uthread_mtx_trylock(uthread_mtx_t *mtx)
{
	assert(mtx != NULL);

	if(mtx->m_owner == NULL)
	{
		mtx->m_owner = ut_curthr;
		return 1;
	}
	return 0;
}


/*
 * uthread_mtx_unlock
 *
 * Unlock the mutex.  If there are people waiting to get this mutex,
 * explicitly hand off the ownership of the lock to a waiting thread and
 * then wake that thread.
 * param mtx: the pointer to the mutex object
 */
void
uthread_mtx_unlock(uthread_mtx_t *mtx)
{
	assert(mtx != NULL);

	/* 
	   You should be the owner
	   It could be possible that you unlock a lock twice, 
	   From the manual it's like the behavior is undefined
	   I will use assertion to gaurantee that.
	*/

	assert(mtx->m_owner == ut_curthr);
	
	if(utqueue_empty(&mtx->m_waiters))
	{
		/* No body is waiting */
		mtx->m_owner = NULL;
	}else
	{
		/* Wake up him and transfer the control to him */
		uthread_t* next = utqueue_dequeue(&mtx->m_waiters);
		mtx->m_owner = next;
		uthread_wake(next);
	}
}
