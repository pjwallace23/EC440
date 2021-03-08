#include "ec440threads.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

// set up a sigalarm: include unistd.h, set up a timer based on the interval (50 ms)
// what sends the signal is U_ALARM --> SA_NODIFFER
/* You can support more threads. At least support this many. */
#define MAX_THREADS 128

/* Your stack should be this many bytes in size */
#define THREAD_STACK_SIZE 32767

/* Number of microseconds between scheduling events */
#define SCHEDULER_INTERVAL_USECS (50 * 1000)

/* Extracted from private libc headers. These are not part of the public
 * interface for jmp_buf.
 */
#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC  7

/* thread_status identifies the current state of a thread. You can add, rename,
 * or delete these values. This is only a suggestion. */
enum thread_status
{
	TS_EXITED,
	TS_RUNNING,
	TS_READY,
	TS_DORMENT
};

/* The thread control block stores information about a thread. You will
 * need one of this per thread.
 */
struct thread_control_block {
	/* TODO: add a thread ID */
	pthread_t pthread_id;
	/* TODO: add information about its stack -->easier to malloc a stack on the heap somewhere
	  size of stack pointer, RSP pointer etc. --> use unsigned long*/
	char *thread_stack;
	/* TODO: add information about its registers */
	jmp_buf thread_registers;
	/* TODO: add information about the status (e.g., use enum thread_status) */
	enum thread_status status;
	/* Add other information you need to manage this thread */
};

	static struct thread_control_block TCB[MAX_THREADS];
	static int thread_count = 0;
	static int current_thread = 0;
	static int pthread_id_next = 1;

static void schedule(int signal)
{
	/* TODO: implement your round-robin scheduler 
	 * 1. Use setjmp() to update your currently-active thread's jmp_buf
	 *    You DON'T need to manually modify registers here.
	 * 2. Determine which is the next thread that should run
	 * 3. Switch to the next thread (use longjmp on that thread's jmp_buf)
	 */

	//printf("schedule 1\n");

	if (setjmp(TCB[current_thread].thread_registers) == 0)
	{ 
		int index;
		for (int i = current_thread + 1; i < (2 * MAX_THREADS); i++)
		{
			index = i % MAX_THREADS;
			if (TCB[index].status == TS_READY)
			{
				break;
			}
		}
		current_thread = index;
		longjmp(TCB[current_thread].thread_registers, 1);
		//printf("longjmp occured");
		//TCB[current_thread].status = TS_RUNNING;
	}
}

static void scheduler_init()
{	
	// use struct sigaction
	// use SA_NODEFER
	// use ualarm(50 ms, 50 ms)
	

	// kinda need to use a circularly linked list

	// initialize main thread
	TCB[0].pthread_id = 0;
	TCB[0].status = TS_READY;
	TCB[0].thread_stack = NULL;
	setjmp(TCB[0].thread_registers);
	thread_count++;

	for (int i = 1; i < MAX_THREADS; i++)
	{
		TCB[i].status = TS_DORMENT;
		TCB[i].thread_stack = NULL;
		//TCB[i].pthread_id = 1;
	}

	// set up signal handler and alarm
	struct sigaction sa = {{0}};
	sa.sa_handler = schedule;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NODEFER;
	sigaction(SIGALRM, &sa, NULL);
	ualarm(SCHEDULER_INTERVAL_USECS, SCHEDULER_INTERVAL_USECS);
	//printf("scheduler init\n");

	/* TODO: do everything that is needed to initialize your scheduler. For example:
	 * - Allocate/initialize global threading data structures
	 * - Create a TCB for the main thread. Note: This is less complicated
	 *   than the TCBs you create for all other threads. In this case, your
	 *   current stack and registers are already exactly what they need to be!
	 *   Just make sure they are correctly referenced in your TCB.
	 * - Set up your timers to call schedule() at a 50 ms interval (SCHEDULER_INTERVAL_USECS)
	 */
}

// setjmp: C doesn't really like when you substitute the pthreads library with another function (pthread_create is an actual pthread function)
//	setjmp allows you to store values in a register stack
// when setting new threads, change status to ready so that schduler knows to lock current process
//	and start next one

int pthread_create( // to ask: how do we modify registers? e.g., how do I get arg in R13 to use start_thunk to move it to RDI
	pthread_t *thread, const pthread_attr_t *attr,
	void *(*start_routine) (void *), void *arg)
	// attr = NULL
	// start_routine and arg are the command and arguments to pass to the new thread
	// new thread = copy all variables in the stack but the heap stays the same
	// how to push variables to function arguments, pointer to PC to thread --> use setjmp()
	// stores everything in register file and prepares main file to switch over
	// when creating new TCB malloc stack unsigned long pointer
{
	// Create the timer and handler for the scheduler. Create thread 0.
	if (thread_count >= MAX_THREADS)
	{
		//printf("Maximum number of threads created");
		return 0;
	}

	static bool is_first_call = true;
	if (is_first_call)
	{
		is_first_call = false;
		scheduler_init();
	}

	int i;
	int free_thread;
	for (i = 1; i < MAX_THREADS; i++)
	{
		if (TCB[i].thread_stack == NULL)
		{
			break;
		}
	}
	free_thread = i;

	// set up the stack and make it's head be pthread_exit
	TCB[free_thread].thread_stack = malloc(THREAD_STACK_SIZE);
	char *stack_ptr = TCB[free_thread].thread_stack;
	stack_ptr = stack_ptr + THREAD_STACK_SIZE - sizeof(unsigned long int) - 1;
	*(unsigned long int *)stack_ptr = (unsigned long int)pthread_exit;
	// stack_ptr = stack_ptr - 4; // an int in c is 2 bytes, need to subtract 8 bytes

	// save current state of registers
	setjmp(TCB[free_thread].thread_registers);

	// modify registers directly using jmpbuf and ptr_mangle
	TCB[free_thread].thread_registers[0].__jmpbuf[JB_RSP] = ptr_mangle((unsigned long int)stack_ptr);
	TCB[free_thread].thread_registers[0].__jmpbuf[JB_PC] = ptr_mangle((unsigned long int)start_thunk);
	TCB[free_thread].thread_registers[0].__jmpbuf[JB_R12] = (unsigned long int)start_routine;
	TCB[free_thread].thread_registers[0].__jmpbuf[JB_R13] = (unsigned long int)arg;


	// set *thread
	*thread = pthread_id_next;
	TCB[thread_count].pthread_id = *thread;
	pthread_id_next++;
	
	// set status to ready
	TCB[thread_count].status = TS_READY;
	// increase current thread count
	thread_count++;
	//printf("new thread created\n");
	//schedule(1);
	return 0;
	/* TODO: Return 0 on successful thread creation, non-zero for an error.
	 *       Be sure to set *thread on success.
	 * Hints:
	 * The general purpose is to create a TCB:
	 * - Create a stack.
	 * - Assign the stack pointer in the thread's registers. Important: where
	 *   within the stack should the stack pointer be? It may help to draw
	 *   an empty stack diagram to answer that question.
	 * - Assign the program counter in the thread's registers.
	 * - Wait... HOW can you assign registers of that new stack? 
	 *   1. call setjmp() to initialize a jmp_buf with your current thread
	 *   2. modify the internal data in that jmp_buf to create a new thread environment
	 *      env->__jmpbuf[JB_...] = ...
	 *      See the additional note about registers below
	 *   3. Later, when your scheduler runs, it will longjmp using your
	 *      modified thread environment, which will apply all the changes
	 *      you made here.
	 * - Remember to set your new thread as TS_READY, but only  after you
	 *   have initialized everything for the new thread.
	 * - Optionally: run your scheduler immediately (can also wait for the
	 *   next scheduling event).
	 */
	/*
	 * Setting registers for a new thread:
	 * When creating a new thread that will begin in start_routine, we
	 * also need to ensure that `arg` is passed to the start_routine.
	 * We cannot simply store `arg` in a register and set PC=start_routine.
	 * This is because the AMD64 calling convention keeps the first arg in
	 * the EDI register, which is not a register we control in jmp_buf.
	 * We provide a start_thunk function that copies R13 to RDI then jumps
	 * to R12, effectively calling function_at_R12(value_in_R13). So
	 * you can call your start routine with the given argument by setting
	 * your new thread's PC to be ptr_mangle(start_thunk), and properly
	 * assigning R12 and R13.
	 *
	 * Don't forget to assign RSP too! Functions know where to
	 * return after they finish based on the calling convention (AMD64 in
	 * our case). The address to return to after finishing start_routine
	 * should be the first thing you push on your stack.
	 */
}

void pthread_exit(void *value_ptr) // don't necessarily need to free the stack, can mark it as ready to free through TS_EXITED and then free it later
{
	/* TODO: Exit the current thread instead of exiting the entire process.
	 * Hints:
	 * - Release all resources for the current thread. CAREFUL though.
	 *   If you free() the currently-in-use stack then do something like
	 *   call a function or add/remove variables from the stack, bad things
	 *   can happen.
	 * - Update the thread's status to indicate that it has exited
	 */

	// status update
	TCB[current_thread].status = TS_EXITED;

	// free the stack
	free(TCB[current_thread].thread_stack);

	// update number of active threads
	thread_count--;
	//printf("current thread %d exited\n", current_thread);
	// call scheduler if threads remain
	if (thread_count > 0)
	{ 
		schedule(1);
	}
	else exit(0);

	__builtin_unreachable();
}

pthread_t pthread_self(void)
{
	/* TODO: Return the current thread instead of -1
	 * Hint: this function can be implemented in one line, by returning
	 * a specific variable instead of -1.
	 */
	return (TCB[current_thread].pthread_id);
	
}

/* Don't implement main in this file!
 * This is a library of functions, not an executable program. If you
 * want to run the functions in this file, create separate test programs
 * that have their own main functions.
 */
