/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* Timerlib.h
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

#ifndef TIMERLIB_H
#define TIMERLIB_H
extern unsigned int vrrid;
extern unsigned int slotid;
extern unsigned int local;

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
