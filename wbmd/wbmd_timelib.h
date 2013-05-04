#ifndef _WBMD_TIMERLIB_H
#define _WBMD_TIMERLIB_H

/*
 * Initializes the timer library. It is important that the calling thread and
 * any other thread in the process block the signal SIGALRM.
 *
 * Return: 1 in case of success; 0 otherwise.
 */
int timer_init();

/*
 * Tears down any data allocated for the library.
 */
void timer_destroy();

/*
 * Adds a timer to expire "sec" seconds + "usec" milliseconds after the
 * invocation. When the timer will expire, function "hndlr" will be executed
 * with arg "hndlr_arg". The id of the timer (>=0) is returned.
 *
 * Return: id>=0 in case of success; -1 in case of error.
 *
 * Remarks: sec and usec must define a future time relative to "now". If
 * one is negative or both equal zero the funtion will return -1. The
 * library don't manipulate in any way the address pointed by "hndlr_arg",
 * it is responsibility of the calling thread to free any  memory, if 
 * allocated (a good place is at the end of "hndlr" function ;).
 * There is no costraint about relative time definition, you can set a two
 * second timer as (sec,usec)=(2,0) or (sec,usec)=(0,2000000).
 * Since this library implementation do not rely on "sigaction()" syscall,
 * inside "hndlr" it is perfectly legal to call any not signal safe and 
 * pthread_* function. The only limitation is that timer_destroy() MUST
 * NOT be called inside hndlr.
 * Note that this fuction can be called inside "hndlr" to create 
 * "chain timers".
 */
int timer_add(long /*sec*/, long /*usec*/, void(* /*hndlr*/)(void *), 
	      void * /*hndlr_arg*/);

/*
 * Remove from the queue the timer of identifier "id", if it retured
 * from a previous succesful call to timer_add().
 *
 * Remarks: if "id" is a value not returned by timer_add() nothing happens.
 */
void timer_rem(int /*id*/, int isFree);

/*
 * Prints the content of the timer queue (do not use, it's for debug purpose).
 */
void timer_print();

#endif

