/*
 *   FILE: uthread_idle.c 
 * AUTHOR: Yan Li
 *  DESCR: uthread_idle
 *   DATE: Feb 20 16:30 2014
 *
 */

#include <sched.h>

#include "uthread.h"
#include "uthread_private.h"


/*
 * uthread_idle
 *
 * Just call linux's yield() function.
 */
void
uthread_idle(void)
{
  sched_yield();
}
