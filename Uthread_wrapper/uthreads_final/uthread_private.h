/*
 *   FILE: uthread_private.h 
 * AUTHOR: Yan Li
 *  DESCR: uthreads private stuff.
 *   DATE: Feb 20 16:30 2014
 *
 */

#ifndef __uthread_private_h__
#define __uthread_private_h__


/*
 * scheduler initialization
 */
void uthread_sched_init(void);


/*
 * important function: do context switch here
 */
void uthread_switch(void);


/*
 * idle the current thread
 */
void uthread_idle(void);


#endif /* __uthread_private_h__ */
