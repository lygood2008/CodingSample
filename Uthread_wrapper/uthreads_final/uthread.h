/*
 *   FILE: uthread.h
 * AUTHOR: Yan Li
 *  DESCR: userland threads declarations
 *   DATE: Wed Feb 19 18:45 2014
 *
 */

#ifndef __uthread_h__
#define __uthread_h__

#include <sys/types.h>
#include "uthread_ctx.h"
#include "uthread.h"
#include "list.h"


#define UTH_MAXPRIO		    7		    /* max thread prio */
#define UTH_MAX_UTHREADS	64		    /* max threads */
#define	UTH_STACK_SIZE		64*1024		/* stack size */

#define	PANIC(err) \
	do { \
		fprintf(stderr, "PANIC at %s:%i -- %s\n", \
			__FILE__, __LINE__, err); \
		abort(); \
	} while(0);

#undef errno
#define	errno	(ut_curthr->ut_errno)

typedef int uthread_id_t;
typedef void(*uthread_func_t)(long, void*);

/*
 * enum: uthread
 */
typedef enum
{
	UT_NO_STATE,		/* invalid thread state */
	UT_ON_CPU,		    /* thread is running */
	UT_RUNNABLE,		/* thread is runnable */
	UT_WAIT,		    /* thread is blocked */
	UT_ZOMBIE,		    
	UT_NUM_THREAD_STATES
} uthread_state_t;

/*
    struct: uthread_t
    Brief: the uthread structure mocks the structure of pthread
 */
typedef struct uthread {
    list_link_t		ut_link;	/* link on waitqueue / scheduler */

    uthread_ctx_t	ut_ctx;		/* context */
    char	*ut_stack;	        /* user stack */

    uthread_id_t	ut_id;		/* thread's id */
    uthread_state_t	ut_state;	/* thread state */
    int			ut_prio;	    /* thread's priority */
    int			ut_errno;	    /* thread's errno */
    int			ut_has_exited;	/* thread exited? */
    int			ut_exit;	    /* thread's exit value */
    int			ut_detached;	/* thread is detached? */
    struct uthread	*ut_waiter;	/* thread waiting to join with me */
} uthread_t;

#define	PRINT(messg)\
	do{\
         fprintf(stderr, messg);\
}while(0)

extern uthread_t uthreads[UTH_MAX_UTHREADS];
extern uthread_t *ut_curthr;

/*
 * uthread_init
 *
 * Called exactly once when the user process 
 */
void uthread_init(void);

/*
 * uthread_create
 *
 * Create a uthread to execute the specified function <func> with argument
 * <arg> and initial priority <prio>. 
 * param uidp: the pointer to the uthread object
 * param func: uthread task
 * param arg1: the argument list length
 * param *arg2: the argument list
 * param prio: the priority of this thread
 * return 0 for success and -1 for error
 */
int uthread_create(uthread_id_t *id, uthread_func_t func, long arg1, 
		   void *arg2, int prio);

/*
 * uthread_exit
 *
 * Terminate the current thread.  Should set all the related flags and
 * such in the uthread_t. 
 * param status: the exit status
 */
void uthread_exit(int status);

/*
 * uthread_join
 *
 * Wait for the given thread to finish executing.
 * param uid: uthread id
 * param *return_value: the return value from the thread we join
 * return: 0 for success; EDEADLK for deadlock; ESRCH for no such process
 *         EINVAL for invalid operation
 */
int uthread_join(uthread_id_t id, int *exit_value);

/*
 * uthread_detach
 *
 * Detach the given thread. 
 * param uid: the uthread id that you want to detach
 * return: 0 for success, ESRCH: no such process, EINVAL: invalid operation
 */
int uthread_detach(uthread_id_t id);

/*
 * uthread_self
 *
 * Returns the id of the currently running thread.
 * return: the thread id that we are currently running
 */
uthread_id_t uthread_self(void);

#endif /* __uthread_h__ */
