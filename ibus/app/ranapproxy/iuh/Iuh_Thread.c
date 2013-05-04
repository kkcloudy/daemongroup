#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <semaphore.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include "iuh/Iuh.h"
#include "Iuh_Thread.h"
#include "Iuh_SockOP.h"
#include "Iuh_DBus.h"
#include "Iuh_IuRecv.h"
#include "Iuh_ManageHNB.h"
#include "Iuh_log.h"
#define Iuh_USE_THREAD_TIMERS
#define HAVE_SEM_TIMEDWAIT

Iuh_THREAD_RETURN_TYPE IuhThreadManageTimers(void *arg);

// Creates a thread that will execute a given function with a given parameter
IuhBool IuhCreateThread(IuhThread *newThread, Iuh_THREAD_FUNCTION threadFunc, void *arg, int less) {
	pthread_attr_t attr;
	size_t ss;	
	int s = PTHREAD_CREATE_DETACHED;
	if(newThread == NULL) return Iuh_FALSE;
	
//	IuhDebugLog("Create Thread\n");
	pthread_attr_init(&attr);
	if(less){
		pthread_attr_getstacksize(&attr,&ss);	
		ss=(ss*3)/4;
		pthread_attr_setstacksize(&attr,ss);
	}
	pthread_attr_setdetachstate(&attr,s);
		
	if(pthread_create(newThread, &attr, threadFunc, arg) != 0) {
		return Iuh_FALSE;
	}

	return Iuh_TRUE;
}

// Creates a thread condition (wrapper for pthread_cond_init)
IuhBool IuhCreateThreadCondition(IuhThreadCondition *theCondition) {
	if(theCondition == NULL) return Iuh_FALSE;
	
	switch(pthread_cond_init(theCondition, NULL)) {
		case 0: // success
			break;
		default:
			return Iuh_FALSE;
	}
	return Iuh_TRUE;
}

// Frees a thread condition (wrapper for pthread_cond_destroy)
void IuhDestroyThreadCondition(IuhThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	pthread_cond_destroy(theCondition);
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
IuhBool IuhWaitThreadCondition(IuhThreadCondition *theCondition, IuhThreadMutex *theMutex) {
	if(theCondition == NULL || theMutex == NULL) return Iuh_FALSE;
	
	switch(pthread_cond_wait(theCondition, theMutex)) {
		case 0: // success
			break;
		case  ETIMEDOUT:
			return Iuh_FALSE;
		default:
			return Iuh_FALSE;	
	}
	
	return Iuh_TRUE;
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
IuhBool IuhWaitThreadConditionTimeout(IuhThreadCondition *theCondition, IuhThreadMutex *theMutex, struct timespec* pTimeout) {
	if(theCondition == NULL || theMutex == NULL) return Iuh_FALSE;
	
	switch(pthread_cond_timedwait(theCondition, theMutex, pTimeout)) {
		case 0: // success
			break;

		case ETIMEDOUT:
			return Iuh_FALSE;

		default:
			return Iuh_FALSE;	
	}
	
	return Iuh_TRUE;
}

// Signal a thread condition (wrapper for pthread_cond_signal)
void IuhSignalThreadCondition(IuhThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	
	pthread_cond_signal(theCondition);
}

// Creates a thread mutex (wrapper for pthread_mutex_init)
IuhBool IuhCreateThreadMutex(IuhThreadMutex *theMutex) {
	if(theMutex == NULL) return Iuh_FALSE;
	
	switch(pthread_mutex_init(theMutex, NULL)) {
		case 0: // success
			break;
		case ENOMEM:
			return Iuh_FALSE;
		default:
			return Iuh_FALSE;
	}
	return Iuh_TRUE;
}


// Free a thread mutex (wrapper for pthread_mutex_destroy)
void IuhDestroyThreadMutex(IuhThreadMutex *theMutex)  {
	if(theMutex == NULL) return;
	pthread_mutex_destroy( theMutex );
}

// locks a mutex among threads at the specified address (blocking)
IuhBool IuhThreadMutexLock(IuhThreadMutex *theMutex) {
	if(theMutex == NULL) return Iuh_FALSE;
	if(pthread_mutex_lock( theMutex ) != 0) {
		return Iuh_FALSE;
	}
/*
	fprintf(stdout, "Mutex %p locked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
	return Iuh_TRUE;
}

// locks a mutex among threads at the specified address (non-blocking).
// Iuh_TRUE if lock was acquired, Iuh_FALSE otherwise
IuhBool IuhThreadMutexTryLock(IuhThreadMutex *theMutex) {
	if(theMutex == NULL) {
		return Iuh_FALSE;
	}
	if(pthread_mutex_trylock( theMutex ) == EBUSY) return Iuh_FALSE;
	else return Iuh_TRUE;
}

// unlocks a mutex among threads at the specified address
void IuhThreadMutexUnlock(IuhThreadMutex *theMutex) {
	if(theMutex == NULL) return;
	pthread_mutex_unlock( theMutex );
/*
	fprintf(stdout, "Mutex %p UNlocked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
}

// creates a semaphore
IuhBool IuhThreadCreateSem(IuhThreadSem *semPtr, int value) {
	if(semPtr == NULL) return Iuh_FALSE;
	
	// we use named semaphore on platforms that support only them (e.g. Mac OS X)
	#ifdef Iuh_USE_NAMED_SEMAPHORES
	{
		static int semCount = 0;
		char name[32];

		snprintf(name, 32, "/IuhSem-%d-%4.4d", getpid(), semCount++);
		if ( (semPtr->semPtr = sem_open(name, O_CREAT, 0600, value)) == (sem_t *)SEM_FAILED ) {
			IuhErrorRaiseSystemError(Iuh_ERROR_GENERAL);
		} else {
			sem_unlink(name);
		}
	}
	#else
		if ( sem_init(semPtr, 0, value) < 0 ) {

		}
	#endif
	
	return Iuh_TRUE;
}

// destroy a semaphore
void IuhThreadDestroySem(IuhThreadSem *semPtr) {
#ifdef Iuh_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return;
#else
	if(semPtr == NULL) return;
#endif
	
	#ifdef Iuh_USE_NAMED_SEMAPHORES
		sem_close(semPtr->semPtr);
	#else
		sem_destroy(semPtr);
	#endif
}

// perform wait on a semaphore
IuhBool IuhThreadSemWait(IuhThreadSem *semPtr) {
#ifdef Iuh_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return Iuh_FALSE;
#else
	if(semPtr == NULL) return Iuh_FALSE;
#endif

	//IuhDebugLog("Sem Wait");
	
#ifdef Iuh_USE_NAMED_SEMAPHORES
	while(sem_wait(semPtr->semPtr) < 0 ) {
#else
	while(sem_wait(semPtr) < 0 ) {
#endif
		if(errno == EINTR) continue;

		else {

		}
	}
	
	return Iuh_TRUE;
}

// perform post on a semaphore
IuhBool IuhThreadSemPost(IuhThreadSem *semPtr) {
#ifdef Iuh_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return Iuh_FALSE;
#else
	if(semPtr == NULL) return Iuh_FALSE;
#endif

#ifdef Iuh_USE_NAMED_SEMAPHORES
	if(sem_post(semPtr->semPtr) < 0 ) {
#else
	if(sem_post(semPtr) < 0 ) {
#endif

	}
	
	return Iuh_TRUE;
}

// get the value of a semaphore
IuhBool IuhThreadSemGetValue(IuhThreadSem *semPtr, int *valuePtr) {
#ifdef Iuh_USE_NAMED_SEMAPHORES
	if(valuePtr == NULL || semPtr == NULL || semPtr->semPtr == NULL) return Iuh_FALSE;
#else
	if(valuePtr == NULL || semPtr == NULL) return Iuh_FALSE;
#endif

#ifdef Iuh_USE_NAMED_SEMAPHORES
	if(sem_getvalue(semPtr->semPtr, valuePtr) < 0) { // note: broken on Mac OS X? Btw we don't need it
	}
#else
	if(sem_getvalue(semPtr, valuePtr) < 0) {

	}
#endif
	if(*valuePtr < 0 ) {
		*valuePtr = 0;
	}
	
	return Iuh_TRUE;
}


__inline__ sem_t *IuhThreadGetSemT(IuhThreadSem *semPtr) {
	#ifdef Iuh_USE_NAMED_SEMAPHORES
		return (semPtr->semPtr);		
	#else		
		return semPtr;	
	#endif
}


// creates a semaphore that can be used with IuhThreadTimedSemWait(). This type of semaphore
// is different from IuhThreadSemaphore to support platforms that don't have sem_timedwait() (e.g. Mac OS X)
IuhBool IuhThreadCreateTimedSem(IuhThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	return IuhThreadCreateSem((IuhThreadSem*)semPtr, value);
#else
	// if we don't have sem_timedwait(), the timed semaphore is a pair of unix domain sockets.
	// We write a dummy packet on a socket (client) when we want to post, and select() with timer on the other socket
	// when we want to wait. 
	struct sockaddr_un serverAddr, clientAddr;
	int i;
	
	if(semPtr == NULL) return Iuh_FALSE;
	
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
		if(!IuhThreadTimedSemPost(semPtr)) return Iuh_FALSE;
	}
	
	return Iuh_TRUE;
#endif
}

// Iuh_TRUE if the semaphore has zero value, Iuh_FALSE otherwise
IuhBool IuhThreadTimedSemIsZero(IuhThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	int value;
	if(!IuhThreadSemGetValue((IuhThreadSem*)semPtr, &value)) return Iuh_FALSE;
	
	return (value==0) ? Iuh_TRUE : Iuh_FALSE;
#else
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return Iuh_FALSE;

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
	
	return (r==0) ? Iuh_TRUE : Iuh_FALSE;
#endif
}

IuhBool IuhThreadTimedSemSetValue(IuhThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	// note: we can implement this, but our implemntation does't really need it in case
	// of a system semaphore. This is useful for our Unix Domain Socket Hack
	return Iuh_TRUE;
	//return IuhErrorRaise(Iuh_ERROR_NEED_RESOURCE, "Operation Not Supported");
#else
	fd_set fset;
	int r, i;
	struct timeval timeout;
	
	if(semPtr == NULL) return Iuh_FALSE;

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
		if(!IuhThreadTimedSemPost(semPtr)) return Iuh_FALSE;
	}
	
	return Iuh_TRUE;
#endif
}

void IuhThreadDestroyTimedSem(IuhThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	IuhThreadDestroySem((IuhThreadSem*)semPtr);
#else
	if(semPtr == NULL) return;
	close((*semPtr)[0]);
	close((*semPtr)[1]);
#endif
}

IuhBool IuhThreadTimedSemWait(IuhThreadTimedSem *semPtr, time_t sec, time_t nsec) {
#ifdef HAVE_SEM_TIMEDWAIT
	struct timespec timeout;
	time_t t;
	
	#ifdef Iuh_USE_NAMED_SEMAPHORES
		if(semPtr == NULL || semPtr->semPtr == NULL) return Iuh_FALSE;
	#else
		if(semPtr == NULL) return Iuh_FALSE;
	#endif
	
	
	time(&t);

	timeout.tv_sec = t + sec;
	timeout.tv_nsec = nsec;
	
	#ifdef Iuh_USE_NAMED_SEMAPHORES
		while(sem_timedwait(semPtr->semPtr, &timeout) < 0 ) {
	#else
		while(sem_timedwait(semPtr, &timeout) < 0 ) {
	#endif
			if(errno == EINTR) { continue;}
			else if(errno == ETIMEDOUT) {

				return Iuh_FALSE;
			} else {

			}
		}
	
#else
	fd_set fset;
	int r;
	struct timeval timeout;
	char dummy;
	
	if(semPtr == NULL) return Iuh_FALSE;
	
	IuhDebugLog("Timed Sem Wait");
	
	FD_ZERO(&fset);
	FD_SET((*semPtr)[0], &fset);
	
	timeout.tv_sec = sec;
	timeout.tv_usec = nsec / 1000;
	

	while((r = select(((*semPtr)[0])+1, &fset, NULL, NULL, &timeout)) <= 0) {

		if(r == 0) {

			return Iuh_FALSE;
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

			return Iuh_FALSE;
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
	

	
	return Iuh_TRUE;
}

IuhBool IuhThreadTimedSemPost(IuhThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	return IuhThreadSemPost((IuhThreadSem*)semPtr);
#else
	char dummy = 'D';
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return Iuh_FALSE;
	

	
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

			return Iuh_FALSE;
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
	

	
	return Iuh_TRUE;
#endif
}


// wrappers for pthread_key_*()
IuhBool IuhThreadCreateSpecific(IuhThreadSpecific *specPtr, void (*destructor)(void *)) {
	if(specPtr == NULL) return Iuh_FALSE;  // NULL destructor is allowed

	if(pthread_key_create(specPtr, destructor) != 0) {

		return Iuh_FALSE;
	}
	
	return Iuh_TRUE;
}

void IuhThreadDestroySpecific(IuhThreadSpecific *specPtr) {
	if(specPtr == NULL) return;
	pthread_key_delete(*specPtr);
}

void *IuhThreadGetSpecific(IuhThreadSpecific *specPtr) {
	if(specPtr == NULL) return NULL;
	return pthread_getspecific(*specPtr);
}

IuhBool IuhThreadSetSpecific(IuhThreadSpecific *specPtr, void *valPtr) {
	if(specPtr == NULL || valPtr == NULL) return Iuh_FALSE;

	switch(pthread_setspecific(*specPtr, valPtr)) {
		case 0: // success
			break;
		case ENOMEM:
			return Iuh_FALSE;
		default:
			return Iuh_FALSE;
	}
	
	return Iuh_TRUE;
}

// terminate the calling thread
void IuhExitThread() {
	//printf("\n*** Exit Thread ***\n");
	iuh_syslog_notice("one wtp exit,thread close");
	pthread_exit((void *) 0);
}


void IuhThreadSetSignals(int how, int num, ...) {
	sigset_t mask;
	va_list args;
	
	sigemptyset(&mask);
	
	va_start(args, num);
	
	for(; num > 0; num--) {
		sigaddset(&mask, va_arg(args, int));
	}
	
	IuhThreadSigMask(how, &mask, NULL);
	
	va_end(args);
}



struct {
	IuhThreadSem requestServiceSem;
	IuhThreadMutex requestServiceMutex;
	IuhThreadSem serviceProvidedSem;

	int requestedSec;
	enum {
		Iuh_TIMER_REQUEST,
		Iuh_TIMER_CANCEL,
		Iuh_TIMER_NONE
	} requestedOp;
	IuhThread *requestedThreadPtr;
	int signalToRaise;
	
	IuhBool error;
	
	IuhTimerID timerID;
} gTimersData;

IuhBool IuhTimerCancel(IuhTimerID *idPtr, int isFree) {

	timer_rem(*idPtr, isFree);
	return Iuh_TRUE;
}


