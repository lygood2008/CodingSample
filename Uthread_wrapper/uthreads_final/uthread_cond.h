/*
 *   FILE: uthread_cond.h
 * AUTHOR: Yan Li
 *  DESCR: funcation prototypes of uthread conditional variables
 *   DATE: Sat Feb  22 09:59 2014
 *
 */

#ifndef __uthread_cond_h__
#define __uthread_cond_h__


struct uthread_mtx;
struct utqueue;

/*
 * struct: uthread_cond_t
 * Brief: conditional variable structure for uthread
 */
typedef struct uthread_cond {
	struct utqueue	uc_waiters;
} uthread_cond_t;

/*
 * uthread_cond_init
 *
 * Initialize the given condition variable
 * param cond: the pointer to the conditional object
 */
void uthread_cond_init(uthread_cond_t *);

/*
 * uthread_cond_wait
 *
 * Should behave just like a stripped down version of pthread_cond_wait.
 * Block on the given condition variable.
 * param cond: the pointer to the conditional variable object
 * param mtx: the pointer to the mutex object
 */
void uthread_cond_wait(uthread_cond_t *, struct uthread_mtx *);


/*
 * uthread_cond_broadcast
 *
 * Wakeup all the threads waiting on this condition variable.
 * param cond: the pointer to the conditional variable object
 */
void uthread_cond_broadcast(uthread_cond_t *);

/*
 * uthread_cond_signal
 *
 * Wakeup just one thread waiting on the condition variable.
 * Note there may be no threads waiting.
 * param cond: the pointer to the conditional variable object
 */
void uthread_cond_signal(uthread_cond_t *);


#endif /* __uthread_cond_h__ */
