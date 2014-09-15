/*
 *   FILE: uthread_sched.h 
 * AUTHOR: Yan Li
 *  DESCR: scheduling wack for uthreads
 *   DATE: Thu Feb 20 08:19:20 2014
 */

#ifndef __uthread_sched_h__
#define __uthread_sched_h__

#include <sys/types.h>
#include "uthread.h"

/*
 * uthread_yield
 *
 * Causes the currently running thread to yield use of the processor to
 * another thread. 
 */
void uthread_yield(void);

/*
 * uthread_block
 *
 * Put the current thread to sleep, pending an appropriate call to 
 * uthread_wake().
 */
void uthread_block(void);

/*
 * uthread_wake
 *
 * Wakes up the supplied thread (schedules it to be run again).
 * uthr: the pointer to the uthread object
 */
void uthread_wake(uthread_t *uthr);

/*
 * uthread_setprio
 *
 * Changes the priority of the indicated thread. 
 * param id: the uthread id
 * param prio: the priority to be set
 */
int uthread_setprio(uthread_id_t id, int prio);

/*
 * uthread_switch()
 *
 * Wait until there is a runnable thread, and then switch to it using 
 * uthread_swapcontext(). 
 * 
 */
void uthread_switch(void);

/*
 * uthread_sched_init
 *
 * Setup the scheduler. This is called once from uthread_init().
 */
void uthread_sched_init(void);

#endif /* __uthread_sched_h__ */
