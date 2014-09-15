/*
 *   FILE: uthread_mtx..h
 * AUTHOR: Yan Li
 *  DESCR: userland mutexes
 *   DATE: Sat Feb 22 10:36 2014
 */

#ifndef __uthread_mtx_h__
#define __uthread_mtx_h__


#include "uthread_queue.h"


struct uthread;

/*
 * struct: uthread_mtx
 * Brief: mutex structure for uthread
 */
typedef struct uthread_mtx {
	struct uthread	*m_owner;
	utqueue_t	m_waiters;
} uthread_mtx_t;

/*
 * uthread_mtx_init
 *
 * Initialize the fields of the specified mutex.
 * param mtx: the pointer to the mutex object
 */
void uthread_mtx_init(uthread_mtx_t *mtx);

/*
 * uthread_mtx_lock
 *
 * Lock the mutex.  This call will block if it's already locked.
 * param mtx: the pointer to the mutex object
 */
void uthread_mtx_lock(uthread_mtx_t *mtx);

/*
 * uthread_mtx_trylock
 *
 * Try to lock the mutex, return 1 if we get the lock, 0 otherwise.
 * This call should not block.
 * param mtx: the pointer to the mutex object
 */
int uthread_mtx_trylock(uthread_mtx_t *mtx);

/*
 * uthread_mtx_unlock
 *
 * Unlock the mutex.  If there are people waiting to get this mutex,
 * explicitly hand off the ownership of the lock to a waiting thread and
 * then wake that thread.
 * param mtx: the pointer to the mutex object
 */
void uthread_mtx_unlock(uthread_mtx_t *mtx);

#endif /* __uthread_mtx_h__ */
