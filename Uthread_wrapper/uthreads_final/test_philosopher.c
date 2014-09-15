/*
 *   FILE: test_philosopher.c 
 * AUTHOR: Yan Li (yanli)
 *  DESCR: a simple program of philosopher dining problem. Should cause a deadlock.
           However because it's not preemptive, and scheduler is pretty simple,
           it may not cause a deadlock here if there are many philosophers. I 
           have tested that 3 Philosophers will cause a deadlock.
 *   DATE: Feb 27 11:30 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "uthread.h"
#include "uthread_mtx.h"
#include "uthread_cond.h"
#include "uthread_sched.h"

#define PHI_NUM  3    /* Philosophers */
#define BUF_SIZE 256 /* Buffer size */

uthread_id_t philosopher[PHI_NUM];

/*
 * struct: fork
 * Brief: shared fork structure, only has one mutex
 */
typedef struct fork
{
	uthread_mtx_t mtx;
} fork_t;

fork_t forks[PHI_NUM];

/*
 * struct: forks
 * Brief: Each philosopher should have two forks (left and right)
 */
typedef struct forks
{
	fork_t* f1;
	fork_t* f2;
} forks_t;

/*
 * eat
 * Each philosopher will eat many times.
 * a0: the argument list length
 * *a2: the argument list
 */
static void 
eat(long a0, void *a2)
{
	int i = 0, ret = 0;
	char pbuffer[BUF_SIZE] = {0};

	forks_t* ft = (forks_t*)a2;
	fork_t* f1 = (ft->f1);
	fork_t* f2 = (ft->f2);

	while(i < 10000000)
	{
		uthread_mtx_lock(&f1->mtx);
		uthread_mtx_lock(&f2->mtx);

		sprintf(pbuffer, "thread %i: eat!\n", uthread_self());  
		
		i++;
		
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
		uthread_mtx_unlock(&f2->mtx);
		uthread_mtx_unlock(&f1->mtx);
	}
	uthread_exit(0);
}

int
main(int ac, char **av)
{
	/* uthread initialization */
    uthread_init();
	
	int i;

	forks_t forkds[PHI_NUM];
	for(i = 0; i < PHI_NUM; i++)
	{
		forkds[i].f1 = &forks[i];
		forkds[i].f2 = &forks[(i+1)%PHI_NUM];
	}

	/* mutex initialization */
	for(i = 0; i < PHI_NUM; i++)
	{
		uthread_mtx_init(&forks[i].mtx);
	}

	/* Create PHI_NUM philosophers */
	for(i = 0; i < PHI_NUM; i++)
	{
		uthread_create(&philosopher[i], eat, i, (void*)&forkds[i], 0);
	}

	/* Wait for them to finish eating */
	for (i = 0; i < PHI_NUM; i++)
    {
        char pbuffer[BUF_SIZE];
        int	tmp = 1, ret = 0;
		uthread_join(philosopher[i], &tmp);
        sprintf(pbuffer, "joined with consumer thread %i, exited %i.\n",
                philosopher[i], tmp);  
        ret = write(STDOUT_FILENO, pbuffer, strlen(pbuffer));
        if (ret < 0) 
        {
            perror("write");
            return EXIT_FAILURE;
		}
	}

	/* uthread exit */
	uthread_exit(0);
	return 0;
}


