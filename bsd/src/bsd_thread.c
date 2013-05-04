#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <semaphore.h>
#include "bsd.h"
#include "bsd_log.h"
#include "bsd_thread.h"
#include "bsd_timerLib.h"

#define BSD_USE_THREAD_TIMERS
#define HAVE_SEM_TIMEDWAIT

BSD_THREAD_RETURN_TYPE BsdThreadManageTimers(void *arg);

// Creates a thread that will execute a given function with a given parameter
BSDBool BsdCreateThread(BsdThread *newThread, BSD_THREAD_FUNCTION threadFunc, void *arg, int less) {
	pthread_attr_t attr;
	size_t ss;	
	int s = PTHREAD_CREATE_DETACHED;
	if(newThread == NULL) return BSD_FALSE;
	
	pthread_attr_init(&attr);
	if(less){
		pthread_attr_getstacksize(&attr,&ss);	
		ss=(ss*3)/4;
		pthread_attr_setstacksize(&attr,ss);
	}
	pthread_attr_setdetachstate(&attr,s);
		
	if(pthread_create(newThread, &attr, threadFunc, arg) != 0) {
		return BSD_FALSE;
	}

	return BSD_TRUE;
}

// Creates a thread condition (wrapper for pthread_cond_init)
BSDBool BsdCreateThreadCondition(BsdThreadCondition *theCondition) {
	if(theCondition == NULL) return BSD_FALSE;
	
	switch(pthread_cond_init(theCondition, NULL)) {
		case 0: // success
			break;
		default:
			return BSD_FALSE;
	}
	return BSD_TRUE;
}

// Frees a thread condition (wrapper for pthread_cond_destroy)
void BsdDestroyThreadCondition(BsdThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	pthread_cond_destroy(theCondition);
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
BSDBool BsdWaitThreadCondition(BsdThreadCondition *theCondition, BsdThreadMutex *theMutex) {
   if(theCondition == NULL || theMutex == NULL) return BSD_FALSE;
	
	switch(pthread_cond_wait(theCondition, theMutex)) {
		case 0: // success
			break;
		case  ETIMEDOUT:
			return BSD_FALSE;
		default:
			return BSD_FALSE;	
	}
	
	return BSD_TRUE;
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
BSDBool BsdWaitThreadConditionTimeout(BsdThreadCondition *theCondition, BsdThreadMutex *theMutex, struct timespec* pTimeout) {
    if(theCondition == NULL || theMutex == NULL) return BSD_FALSE;
	
	switch(pthread_cond_timedwait(theCondition, theMutex, pTimeout)) {
		case 0: // success
			break;

		case ETIMEDOUT:
			return BSD_FALSE;

		default:
			return BSD_FALSE;	
	}
	
	return BSD_TRUE;
}

// Signal a thread condition (wrapper for pthread_cond_signal)
void BsdSignalThreadCondition(BsdThreadCondition *theCondition) {
    if(theCondition == NULL) return;
	
	pthread_cond_signal(theCondition);
}

// Creates a thread mutex (wrapper for pthread_mutex_init)
BSDBool BsdCreateThreadMutex(BsdThreadMutex *theMutex) {
	if(theMutex == NULL) return BSD_FALSE;
	
	switch(pthread_mutex_init(theMutex, NULL)) {
		case 0: // success
			break;
		case ENOMEM:
			return BSD_FALSE;
		default:
			return BSD_FALSE;
	}
	return BSD_TRUE;
}


// Free a thread mutex (wrapper for pthread_mutex_destroy)
void BsdDestroyThreadMutex(BsdThreadMutex *theMutex)  {
	if(theMutex == NULL) return;
	pthread_mutex_destroy( theMutex );
}

// locks a mutex among threads at the specified address (blocking)
BSDBool BsdThreadMutexLock(BsdThreadMutex *theMutex) {
    if(theMutex == NULL) return BSD_FALSE;
	if(pthread_mutex_lock( theMutex ) != 0) {
		return BSD_FALSE;
	}
/*
	fprintf(stdout, "Mutex %p locked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
	return BSD_TRUE;
}

// locks a mutex among threads at the specified address (non-blocking).
// BSD_TRUE if lock was acquired, BSD_FALSE otherwise
BSDBool BsdThreadMutexTryLock(BsdThreadMutex *theMutex) {
	if(theMutex == NULL) {
		return BSD_FALSE;
	}
	if(pthread_mutex_trylock( theMutex ) == EBUSY) return BSD_FALSE;
	else return BSD_TRUE;
}

// unlocks a mutex among threads at the specified address
void BsdThreadMutexUnlock(BsdThreadMutex *theMutex) {
    if(theMutex == NULL) return;
	pthread_mutex_unlock( theMutex );
/*
	fprintf(stdout, "Mutex %p UNlocked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
}

// creates a semaphore
BSDBool BsdThreadCreateSem(BsdThreadSem *semPtr, int value) {
	if(semPtr == NULL) return BSD_FALSE;
	
	// we use named semaphore on platforms that support only them (e.g. Mac OS X)
	#ifdef BSD_USE_NAMED_SEMAPHORES
	{
		static int semCount = 0;
		char name[32];

		snprintf(name, 32, "/BsdSem-%d-%4.4d", getpid(), semCount++);
		if ( (semPtr->semPtr = sem_open(name, O_CREAT, 0600, value)) == (sem_t *)SEM_FAILED ) {
			BsdErrorRaiseSystemError(BSD_ERROR_GENERAL);
		} else {
			sem_unlink(name);
		}
	}
	#else
		if ( sem_init(semPtr, 0, value) < 0 ) {

		}
	#endif
	
	return BSD_TRUE;
}

// destroy a semaphore
void BsdThreadDestroySem(BsdThreadSem *semPtr) {
#ifdef BSD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return;
#else
	if(semPtr == NULL) return;
#endif
	
	#ifdef BSD_USE_NAMED_SEMAPHORES
		sem_close(semPtr->semPtr);
	#else
		sem_destroy(semPtr);
	#endif
}

// perform wait on a semaphore
BSDBool BsdThreadSemWait(BsdThreadSem *semPtr) {
#ifdef BSD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return BSD_FALSE;
#else
	if(semPtr == NULL) return BSD_FALSE;
#endif

	//BSDDebugLog("Sem Wait");
	
#ifdef BSD_USE_NAMED_SEMAPHORES
	while(sem_wait(semPtr->semPtr) < 0 ) {
#else
	while(sem_wait(semPtr) < 0 ) {
#endif
		if(errno == EINTR) continue;

		else {

		}
	}
	
	return BSD_TRUE;
}

// perform post on a semaphore
BSDBool BsdThreadSemPost(BsdThreadSem *semPtr) {
#ifdef BSD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return BSD_FALSE;
#else
	if(semPtr == NULL) return BSD_FALSE;
#endif

#ifdef BSD_USE_NAMED_SEMAPHORES
	if(sem_post(semPtr->semPtr) < 0 ) {
#else
	if(sem_post(semPtr) < 0 ) {
#endif

	}
	
	return BSD_TRUE;
}

// get the value of a semaphore
BSDBool BsdThreadSemGetValue(BsdThreadSem *semPtr, int *valuePtr) {
#ifdef BSD_USE_NAMED_SEMAPHORES
	if(valuePtr == NULL || semPtr == NULL || semPtr->semPtr == NULL) return BSD_FALSE;
#else
	if(valuePtr == NULL || semPtr == NULL) return BSD_FALSE;
#endif

#ifdef BSD_USE_NAMED_SEMAPHORES
	if(sem_getvalue(semPtr->semPtr, valuePtr) < 0) { // note: broken on Mac OS X? Btw we don't need it
	}
#else
	if(sem_getvalue(semPtr, valuePtr) < 0) {

	}
#endif
	if(*valuePtr < 0 ) {
		*valuePtr = 0;
	}
	
	return BSD_TRUE;
}


__inline__ sem_t *BsdThreadGetSemT(BsdThreadSem *semPtr) {
	#ifdef BSD_USE_NAMED_SEMAPHORES
		return (semPtr->semPtr);		
	#else		
		return semPtr;	
	#endif
}

/*
// creates a semaphore that can be used with BSDThreadTimedSemWait(). This type of semaphore
// is different from BSDThreadSemaphore to support platforms that don't have sem_timedwait() (e.g. Mac OS X)
BSDBool BsdThreadCreateTimedSem(BsdThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	return BsdThreadCreateSem((BsdThreadSem *)semPtr, value);
#else
	// if we don't have sem_timedwait(), the timed semaphore is a pair of unix domain sockets.
	// We write a dummy packet on a socket (client) when we want to post, and select() with timer on the other socket
	// when we want to wait. 
	struct sockaddr_un serverAddr, clientAddr;
	int i;
	
	if(semPtr == NULL) return BSD_FALSE;
	
	if((((*semPtr)[0] = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0) ||
		(((*semPtr)[1] = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0)) { // create a pair of datagram unix domain socket
		close((*semPtr)[0]);

	}
	
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sun_family = AF_LOCAL;
	if(tmpnam((char*) &(serverAddr.sun_path)) == NULL) {

	}
	
	bzero(&clientAddr, sizeof(clientAddr));
	clientAddr.sun_family = AF_LOCAL;
	if(tmpnam((char*) &(clientAddr.sun_path)) == NULL) {

	}
	
	if(	(bind((*semPtr)[0], (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) ||
		(bind((*semPtr)[1], (struct sockaddr*) &clientAddr, sizeof(clientAddr)) < 0) ||
		(connect((*semPtr)[1], (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) || // connect each socket to the other
		(connect((*semPtr)[0], (struct sockaddr*) &clientAddr, sizeof(clientAddr)) < 0)
	) {
		close((*semPtr)[0]);
		close((*semPtr)[1]);

	}
	
	for(i = 0; i < value; i++) {
		if(!BsdThreadTimedSemPost(semPtr)) return BSD_FALSE;
	}
	
	return BSD_TRUE;
#endif
}
*/

// BSD_TRUE if the semaphore has zero value, BSD_FALSE otherwise
BSDBool BsdThreadTimedSemIsZero(BsdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	int value;
	if(!BsdThreadSemGetValue((BsdThreadSem *)semPtr, &value)) return BSD_FALSE;
	
	return (value==0) ? BSD_TRUE : BSD_FALSE;
#else
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return BSD_FALSE;

	FD_ZERO(&fset);
	FD_SET((*semPtr)[0], &fset);
	FD_SET((*semPtr)[1], &fset);
	
	timeout.tv_sec = 0; // poll
	timeout.tv_usec = 0;
	
	while((r=select(max((*semPtr)[1], (*semPtr)[0])+1, &fset, NULL, NULL, &timeout)) < 0) {
		if(errno == EINTR) {
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
			continue;
		}

	}
	
	return (r==0) ? BSD_TRUE : BSD_FALSE;
#endif
}

BSDBool BsdThreadTimedSemSetValue(BsdThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	// note: we can implement this, but our implemntation does't really need it in case
	// of a system semaphore. This is useful for our Unix Domain Socket Hack
	return BSD_TRUE;
	//return BSDErrorRaise(BSD_ERROR_NEED_RESOURCE, "Operation Not Supported");
#else
	fd_set fset;
	int r, i;
	struct timeval timeout;
	
	if(semPtr == NULL) return BSD_FALSE;

	FD_ZERO(&fset);
	FD_SET((*semPtr)[0], &fset);
	FD_SET((*semPtr)[1], &fset);
	
	timeout.tv_sec = 0; // poll
	timeout.tv_usec = 0;
	
	// first, remove all the pending packets
	while(1) {
		char dummy;
		while((r=select(max((*semPtr)[1], (*semPtr)[0])+1, &fset, NULL, NULL, &timeout)) < 0) {
			if(errno == EINTR) {
				timeout.tv_sec = 0;
				timeout.tv_usec = 0;
				continue;
			}

		}
		
		if(r == 0) break;
		
		if(FD_ISSET((*semPtr)[0], &fset)) {
			while(read((*semPtr)[0], &dummy, 1) < 0) {
				if(errno == EINTR) continue;

			}
		}
		
		if(FD_ISSET((*semPtr)[1], &fset)) {
			while(read((*semPtr)[1], &dummy, 1) < 0) {
				if(errno == EINTR) continue;

			}
		}
	}
	
	// second, send n packets, where n is the value we want to set for the semaphore
	for(i = 0; i < value; i++) {
		if(!BsdThreadTimedSemPost(semPtr)) return BSD_FALSE;
	}
	
	return BSD_TRUE;
#endif
}

void BsdThreadDestroyTimedSem(BsdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	BsdThreadDestroySem((BsdThreadSem *)semPtr);
#else
	if(semPtr == NULL) return;
	close((*semPtr)[0]);
	close((*semPtr)[1]);
#endif
}

/*
BSDBool BsdThreadTimedSemWait(BsdThreadTimedSem *semPtr, time_t sec, time_t nsec) {
#ifdef HAVE_SEM_TIMEDWAIT
	struct timespec timeout;
	time_t t;
	
	#ifdef BSD_USE_NAMED_SEMAPHORES
		if(semPtr == NULL || semPtr->semPtr == NULL) return BSD_FALSE;
	#else
		if(semPtr == NULL) return BSD_FALSE;
	#endif
	
	
	time(&t);

	timeout.tv_sec = t + sec;
	timeout.tv_nsec = nsec;
	
	#ifdef BSD_USE_NAMED_SEMAPHORES
		while(sem_timedwait(semPtr->semPtr, &timeout) < 0 ) {
	#else
		while(sem_timedwait(semPtr, &timeout) < 0 ) {
	#endif
			if(errno == EINTR) { continue;}
			else if(errno == ETIMEDOUT) {

				return BSD_FALSE;
			} else {

			}
		}
	
#else
	fd_set fset;
	int r;
	struct timeval timeout;
	char dummy;
	
	if(semPtr == NULL) return BSD_FALSE;
	
	BSDDebugLog("Timed Sem Wait");
	
	FD_ZERO(&fset);
	FD_SET((*semPtr)[0], &fset);
	
	timeout.tv_sec = sec;
	timeout.tv_usec = nsec / 1000;
	

	while((r = select(((*semPtr)[0])+1, &fset, NULL, NULL, &timeout)) <= 0) {

		if(r == 0) {

			return BSD_FALSE;
		} else if(errno == EINTR) {
			timeout.tv_sec = sec;
			timeout.tv_usec = nsec / 1000;
			continue;
		}

	}
	

	
	// ready to read
	
	while(read((*semPtr)[0], &dummy, 1) < 0) {
		if(errno == EINTR) continue;

	}
	
	// send ack (three-way handshake)
	
	while(send((*semPtr)[0], &dummy, 1, 0) < 0) {
		if(errno == EINTR) continue;

	}
	
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	

	while((r = select(((*semPtr)[0])+1, &fset, NULL, NULL, &timeout)) <= 0) {

		if(r == 0) {

			return BSD_FALSE;
		} else if(errno == EINTR) {
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
			continue;
		}

	}
	

	
	// read ack
	
	while(read((*semPtr)[0], &dummy, 1) < 0) {
		if(errno == EINTR) continue;

	}
	
#endif
	

	
	return BSD_TRUE;
}
*/

BSDBool BsdThreadTimedSemPost(BsdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	return BsdThreadSemPost((BsdThreadSem *)semPtr);
#else
	char dummy = 'D';
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return BSD_FALSE;
	

	
	while(send((*semPtr)[1], &dummy, 1, 0) < 0) {
		if(errno == EINTR) continue;

	}
	
	// read ack (three-way handshake)
	
	FD_ZERO(&fset);
	FD_SET((*semPtr)[1], &fset);
	
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	

	while((r = select(((*semPtr)[1])+1, &fset, NULL, NULL, &timeout)) <= 0) {

		if(r == 0) { // timeout, server is not responding
			// note: this is not an error in a traditional semaphore, btw it's an error
			// according to our logic

			return BSD_FALSE;
		} else if(errno == EINTR) {
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
			continue;
		}

	}
	

	
	while(read((*semPtr)[1], &dummy, 1) < 0) {
		if(errno == EINTR) continue;

	}
	
	// send ack
	while(send((*semPtr)[1], &dummy, 1, 0) < 0) {
		if(errno == EINTR) continue;

	}
	

	
	return BSD_TRUE;
#endif
}


// wrappers for pthread_key_*()
BSDBool BsdThreadCreateSpecific(BsdThreadSpecific *specPtr, void (*destructor)(void *)) {
	if(specPtr == NULL) return BSD_FALSE;  // NULL destructor is allowed

	if(pthread_key_create(specPtr, destructor) != 0) {

		return BSD_FALSE;
	}
	
	return BSD_TRUE;
}

void BsdThreadDestroySpecific(BsdThreadSpecific *specPtr) {
	if(specPtr == NULL) return;
	pthread_key_delete(*specPtr);
}

void *BsdThreadGetSpecific(BsdThreadSpecific *specPtr) {
	if(specPtr == NULL) return NULL;
	return pthread_getspecific(*specPtr);
}

BSDBool BsdThreadSetSpecific(BsdThreadSpecific *specPtr, void *valPtr) {
	if(specPtr == NULL || valPtr == NULL) return BSD_FALSE;

	switch(pthread_setspecific(*specPtr, valPtr)) {
		case 0: // success
			break;
		case ENOMEM:
			return BSD_FALSE;
		default:
			return BSD_FALSE;
	}
	
	return BSD_TRUE;
}

// terminate the calling thread
void BsdExitThread() {
	//printf("\n*** Exit Thread ***\n");
	bsd_syslog_notice("one wtp exit,thread close");
	pthread_exit((void *) 0);
}


void BsdThreadSetSignals(int how, int num, ...) {
	sigset_t mask;
	va_list args;
	
	sigemptyset(&mask);
	
	va_start(args, num);
	
	for(; num > 0; num--) {
		sigaddset(&mask, va_arg(args, int));
	}
	
	BsdThreadSigMask(how, &mask, NULL);
	
	va_end(args);
}



struct {
	BsdThreadSem requestServiceSem;
	BsdThreadMutex requestServiceMutex;
	BsdThreadSem serviceProvidedSem;

	int requestedSec;
	enum {
		BSD_TIMER_REQUEST,
		BSD_TIMER_CANCEL,
		BSD_TIMER_NONE
	} requestedOp;
	BsdThread *requestedThreadPtr;
	int signalToRaise;
	
	BSDBool error;
	
	BsdTimerID timerID;
} gTimersData;

BSDBool BsdTimerCancel(BsdTimerID *idPtr, int isFree) {

	timer_rem(*idPtr, isFree);
	return BSD_TRUE;
}

