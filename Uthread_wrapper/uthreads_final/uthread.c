/*
 *   FILE: uthread.c
 * AUTHOR: Yan Li
 *  DESCR: userland threads
 *   DATE: Wed Feb 19 18:45 2014
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "uthread.h"
#include "uthread_private.h"
#include "uthread_queue.h"
#include "uthread_bool.h"
#include "uthread_sched.h"

uthread_t	*ut_curthr = NULL;		    /* current running thread */
uthread_t	uthreads[UTH_MAX_UTHREADS];	/* threads on the system */

static list_t		reap_queue;		    /* dead threads */
static uthread_id_t	reaper_thr_id;		/* to wake reaper */

static void create_first_thr(void);
static uthread_id_t uthread_alloc(void);
static void uthread_destroy(uthread_t *thread);

static char *alloc_stack(void);
static void free_stack(char *stack);

static void reaper_init(void);
static void reaper(long a0, void *a1);
static void make_reapable(uthread_t *uth);

/*
 * uthread_init
 * Called exactly once when the user process
 */
void
uthread_init(void)
{
	/* These should go last, and in this order */
	uthread_sched_init();
	reaper_init();
	create_first_thr();
}

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
int
uthread_create(uthread_id_t *uidp, uthread_func_t func,
	       long arg1, void *arg2, int prio)
{
	int rv = 0;
	if ((rv = uthread_alloc()) == -1)
	{
		return -1;
	}

	*uidp = rv;

	/* Create the thread structure */ 
	memset(&uthreads[*uidp], 0, sizeof(uthread_t));
	uthreads[*uidp].ut_id = *uidp;
	uthreads[*uidp].ut_state = UT_RUNNABLE;
	if (uthread_setprio(*uidp, prio) != 0)
	{
		char pbuffer[256];
		sprintf(pbuffer, "error in setprio\n");  
		int ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
		if (ret < 0) 
		{
			perror("write");
		}
		exit(EXIT_FAILURE);
	}
	/* Allocate empty stack */
	char* stack = alloc_stack();
	/* Make the context */
	uthread_makecontext(&uthreads[*uidp].ut_ctx, stack,
	                    UTH_STACK_SIZE, (void(*)())func, arg1, arg2);

	return 0;
}



/*
 * uthread_exit
 *
 * Terminate the current thread.  Should set all the related flags and
 * such in the uthread_t. 
 * param status: the exit status
 */
void
uthread_exit(int status)
{
	assert(ut_curthr != NULL && ut_curthr->ut_state == UT_ON_CPU);

	ut_curthr->ut_state      = UT_ZOMBIE;
	ut_curthr->ut_exit       = status;
	ut_curthr->ut_has_exited = true;

	if (ut_curthr->ut_detached == true)
	{
		make_reapable(ut_curthr);
	}
	else
	{
		if (ut_curthr->ut_waiter != NULL)
		{
			uthread_wake(ut_curthr->ut_waiter);
		}
	}
	uthread_switch();

	PANIC("returned to a dead thread");
}



/*
 * uthread_join
 *
 * Wait for the given thread to finish executing.
 * param uid: uthread id
 * param *return_value: the return value from the thread we join
 * return: 0 for success; EDEADLK for deadlock; ESRCH for no such process
 *         EINVAL for invalid operation
 */
int
uthread_join(uthread_id_t uid, int *return_value)
{
	assert(ut_curthr != NULL && ut_curthr->ut_state == UT_ON_CPU);
	
	/* Invalid id
	   It's possible to join a zombie thread */
	if (uid < 0 || 
		uid >= UTH_MAX_UTHREADS ||
	    uthreads[uid].ut_state == UT_NO_STATE )
	{
		ut_curthr->ut_errno = ESRCH;
		return ESRCH;
	}

	/* I'm waiting on you, which is dead lock! */
	if (ut_curthr->ut_waiter == &uthreads[uid])
	{
		ut_curthr->ut_errno = EDEADLK;
		return EDEADLK;
	}

	/* If there is another thread already waiting to join this
	   thread or the thread is already detached
	 */
	if (ut_curthr->ut_waiter != NULL || uthreads[uid].ut_detached == true)
	{
		ut_curthr->ut_errno = EINVAL;
		return EINVAL; 
	}

	/* If not exited, block the current thread */
	uthreads[uid].ut_waiter = ut_curthr;

	while (uthreads[uid].ut_has_exited == false)
	{
		uthread_block();
	}
	
	/* Set the return value equals the target's return value */
	*return_value = uthreads[uid].ut_exit;
	/* Set the target to be detached, so it can be reaped */
	uthreads[uid].ut_detached = true;
	/* Can be reaped now! */
	make_reapable(&uthreads[uid]);
	
	return 0;
}



/*
 * uthread_detach
 *
 * Detach the given thread. 
 * param uid: the uthread id that you want to detach
 * return: 0 for success; ESRCH: no such process; EINVAL: invalid operation
 */
int
uthread_detach(uthread_id_t uid)
{
	if (uid < 0 || 
		uid >= UTH_MAX_UTHREADS ||
	    uthreads[uid].ut_state == UT_NO_STATE)
	{
		ut_curthr->ut_errno = ESRCH;
		return ESRCH;
	}

	/* If it's already detached!!
	   If some threads are waiting for this thread (join), I take 
	   it as an error
	 */
	if (uthreads[uid].ut_detached == true || ut_curthr->ut_waiter != NULL)
	{
		ut_curthr->ut_errno = EINVAL;
		return EINVAL;
	}

	/* Firstly check if it's already in zombie state */
	if (uthreads[uid].ut_state == UT_ZOMBIE)
	{
		make_reapable(&uthreads[uid]);
	}
	else
	{
		uthreads[uid].ut_detached = true;
	}

	return 0;
}



/*
 * uthread_self
 *
 * Returns the id of the currently running thread.
 * return: the thread id that we are currently running
 */
uthread_id_t
uthread_self(void)
{
	assert(ut_curthr != NULL);
	return ut_curthr->ut_id;
}

/*
 * uthread_alloc
 *
 * Find a free uthread_t, returns the id.
 * return: the first NO_STATE uthread id
 */
static uthread_id_t uthread_alloc(void)
{
	/* Find the first one with state "no state", which means it's not used */
	int i;
	for(i = 0; i < UTH_MAX_UTHREADS; i++)
	{
		if (uthreads[i].ut_state == UT_NO_STATE)
			return i;
	}
	return -1;
}

/*
 * uthread_destroy
 *
 * Cleans up resources associated with a thread (since it's now finished
 * executing). 
 * param uth: pointer to the uthread object
 */
static void uthread_destroy(uthread_t *uth)
{
	/* Free the stack because it's allocated in heap */
	free_stack(uth->ut_stack);
	/* Set all of the data irewn the structure to 0 */
	memset(uth, 0, sizeof(uthread_t));
}

/*
 * reaper_init
 *
 * startup the reaper thread
 */
static void
reaper_init(void)
{
	list_init(&reap_queue);
	uthread_create(&reaper_thr_id, reaper, 0, NULL, UTH_MAXPRIO);

	assert(reaper_thr_id != -1);
}



/*
 * reaper
 *
 * This is responsible for going through all the threads on the dead
 * threads list (which should all be in the ZOMBIE state) and
 * cleaning up all the threads that have been detached/joined with
 * already.
 * param a0: the argument list length
 * param *a1: the argument list
 */
static void
reaper(long a0, void *a1)
{
	while (1)
	{
		uthread_t* thread = NULL;
		int th;

		/* block.  someone will wake me up when it is time */
		uthread_block();

		/* go through dead threads, find detached and
		 * call uthread_destroy() on them
		 */
		list_iterate_begin(&reap_queue, thread, uthread_t, ut_link)
		{
			assert(thread->ut_state == UT_ZOMBIE);

			list_remove(&thread->ut_link);
			uthread_destroy(thread);
		}
		list_iterate_end();

		/* check and see if there are still runnable threads */
		for (th = 0; th < UTH_MAX_UTHREADS; th++)
		{
			if (th != reaper_thr_id &&
			    uthreads[th].ut_state != UT_NO_STATE)
			{
				break;
			}
		}

		if (th == UTH_MAX_UTHREADS)
		{
			/* we leak the reaper's stack */
			fprintf(stderr, "uthreads: no more threads.\n");
			fprintf(stderr, "uthreads: bye!\n");
			exit(0);
		}
	}
}



/*
 * Turns the main context (the 'main' method that initialized
 * this process) into a regular uthread that can be switched
 * into and out of. 
 */
static void
create_first_thr(void)
{
	uthread_t	hack;
	uthread_id_t	main_thr;

	memset(&hack, 0, sizeof(uthread_t));
	ut_curthr = &hack;
	ut_curthr->ut_state = UT_ON_CPU;

	uthread_create(&main_thr,
		       (uthread_func_t)0, 0, NULL,
			UTH_MAXPRIO);
	assert(main_thr != -1);

	uthread_detach(main_thr);
	uthread_getcontext(&uthreads[main_thr].ut_ctx);

	if (ut_curthr == &hack)
	{
		uthread_switch();
	}
	else
	{
		/* this should be the 'main_thr' */
		assert(uthread_self() == main_thr);
	}

}

/*
 * Adds the given thread to the reaper's queue, and wakes up the reaper.
 * param *uth: pointer to the uthread object
 */
static void make_reapable(uthread_t *uth)
{
	assert(uth->ut_detached);
	assert(uth->ut_state == UT_ZOMBIE);

	list_insert_tail(&reap_queue, &uth->ut_link);
	uthread_wake(&uthreads[reaper_thr_id]);
}

/*
 * Allocate heap space (assume it's stack)
 * return: the new space of size UTH_STACK_SIZE
 */
static char *alloc_stack(void)
{
	return (char *)malloc(UTH_STACK_SIZE);
}

/*
 * Free heap space
 * param *stack: free the memory that stack points to 
 */
static void free_stack(char *stack)
{
	free(stack);
}



