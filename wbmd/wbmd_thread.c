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
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <semaphore.h>
#include <sys/msg.h>
#include "wbmd.h"
#include "wbmd/wbmdpub.h"
#include "wbmd_thread.h"
#include "wbmd_timelib.h"
#include "wbmd_log.h"

#define WBMD_USE_THREAD_TIMERS
#define HAVE_SEM_TIMEDWAIT

WBMD_THREAD_RETURN_TYPE WbmdThreadManageTimers(void *arg);

// Creates a thread that will execute a given function with a given parameter
WBMDBool WbmdCreateThread(WbmdThread *newThread, WBMD_THREAD_FUNCTION threadFunc, void *arg, int less) {
	pthread_attr_t attr;
	size_t ss;	
	int s = PTHREAD_CREATE_DETACHED;
	if(newThread == NULL) return WBMD_FALSE;
	
	pthread_attr_init(&attr);
	if(less){
		pthread_attr_getstacksize(&attr,&ss);	
		ss=(ss*3)/4;
		pthread_attr_setstacksize(&attr,ss);
	}
	pthread_attr_setdetachstate(&attr,s);
		
	if(pthread_create(newThread, &attr, threadFunc, arg) != 0) {
		return WBMD_FALSE;
	}

	return WBMD_TRUE;
}

// Creates a thread condition (wrapper for pthread_cond_init)
WBMDBool WbmdCreateThreadCondition(WbmdThreadCondition *theCondition) {
	if(theCondition == NULL) return WBMD_FALSE;
	
	switch(pthread_cond_init(theCondition, NULL)) {
		case 0: // success
			break;
		default:
			return WBMD_FALSE;
	}
	return WBMD_TRUE;
}

// Frees a thread condition (wrapper for pthread_cond_destroy)
void WbmdDestroyThreadCondition(WbmdThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	pthread_cond_destroy(theCondition);
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
WBMDBool WbmdWaitThreadCondition(WbmdThreadCondition *theCondition, WbmdThreadMutex *theMutex) {
	if(theCondition == NULL || theMutex == NULL) return WBMD_FALSE;
	
	switch(pthread_cond_wait(theCondition, theMutex)) {
		case 0: // success
			break;
		case  ETIMEDOUT:
			return WBMD_FALSE;
		default:
			return WBMD_FALSE;	
	}
	
	return WBMD_TRUE;
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
WBMDBool WbmdWaitThreadConditionTimeout(WbmdThreadCondition *theCondition, WbmdThreadMutex *theMutex, struct timespec* pTimeout) {
	if(theCondition == NULL || theMutex == NULL) return WBMD_FALSE;
	
	switch(pthread_cond_timedwait(theCondition, theMutex, pTimeout)) {
		case 0: // success
			break;

		case ETIMEDOUT:
			return WBMD_FALSE;

		default:
			return WBMD_FALSE;	
	}
	
	return WBMD_TRUE;
}

// Signal a thread condition (wrapper for pthread_cond_signal)
void WbmdSignalThreadCondition(WbmdThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	
	pthread_cond_signal(theCondition);
}

// Creates a thread mutex (wrapper for pthread_mutex_init)
WBMDBool WbmdCreateThreadMutex(WbmdThreadMutex *theMutex) {
	if(theMutex == NULL) return WBMD_FALSE;
	
	switch(pthread_mutex_init(theMutex, NULL)) {
		case 0: // success
			break;
		case ENOMEM:
			return WBMD_FALSE;
		default:
			return WBMD_FALSE;
	}
	return WBMD_TRUE;
}


// Free a thread mutex (wrapper for pthread_mutex_destroy)
void WbmdDestroyThreadMutex(WbmdThreadMutex *theMutex)  {
	if(theMutex == NULL) return;
	pthread_mutex_destroy( theMutex );
}

// locks a mutex among threads at the specified address (blocking)
WBMDBool WbmdThreadMutexLock(WbmdThreadMutex *theMutex) {
	if(theMutex == NULL) return WBMD_FALSE;
	if(pthread_mutex_lock( theMutex ) != 0) {
		return WBMD_FALSE;
	}
/*
	fprintf(stdout, "Mutex %p locked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
	return WBMD_TRUE;
}

// locks a mutex among threads at the specified address (non-blocking).
// WBMD_TRUE if lock was acquired, WBMD_FALSE otherwise
WBMDBool WbmdThreadMutexTryLock(WbmdThreadMutex *theMutex) {
	if(theMutex == NULL) {
		return WBMD_FALSE;
	}
	if(pthread_mutex_trylock( theMutex ) == EBUSY) return WBMD_FALSE;
	else return WBMD_TRUE;
}

// unlocks a mutex among threads at the specified address
void WbmdThreadMutexUnlock(WbmdThreadMutex *theMutex) {
	if(theMutex == NULL) return;
	pthread_mutex_unlock( theMutex );
/*
	fprintf(stdout, "Mutex %p UNlocked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
}

// creates a semaphore
WBMDBool WbmdThreadCreateSem(WbmdThreadSem *semPtr, int value) {
	if(semPtr == NULL) return WBMD_FALSE;
	
	// we use named semaphore on platforms that support only them (e.g. Mac OS X)
	#ifdef WBMD_USE_NAMED_SEMAPHORES
	{
		static int semCount = 0;
		char name[32];

		snprintf(name, 32, "/WbmdSem-%d-%4.4d", getpid(), semCount++);
		if ( (semPtr->semPtr = sem_open(name, O_CREAT, 0600, value)) == (sem_t *)SEM_FAILED ) {
			WbmdErrorRaiseSystemError(WBMD_ERROR_GENERAL);
		} else {
			sem_unlink(name);
		}
	}
	#else
		if ( sem_init(semPtr, 0, value) < 0 ) {

		}
	#endif
	
	return WBMD_TRUE;
}

// destroy a semaphore
void WbmdThreadDestroySem(WbmdThreadSem *semPtr) {
#ifdef WBMD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return;
#else
	if(semPtr == NULL) return;
#endif
	
	#ifdef WBMD_USE_NAMED_SEMAPHORES
		sem_close(semPtr->semPtr);
	#else
		sem_destroy(semPtr);
	#endif
}

// perform wait on a semaphore
WBMDBool WbmdThreadSemWait(WbmdThreadSem *semPtr) {
#ifdef WBMD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return WBMD_FALSE;
#else
	if(semPtr == NULL) return WBMD_FALSE;
#endif

	//WBMDDebugLog("Sem Wait");
	
#ifdef WBMD_USE_NAMED_SEMAPHORES
	while(sem_wait(semPtr->semPtr) < 0 ) {
#else
	while(sem_wait(semPtr) < 0 ) {
#endif
		if(errno == EINTR) continue;

		else {

		}
	}
	
	return WBMD_TRUE;
}

// perform post on a semaphore
WBMDBool WbmdThreadSemPost(WbmdThreadSem *semPtr) {
#ifdef WBMD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return WBMD_FALSE;
#else
	if(semPtr == NULL) return WBMD_FALSE;
#endif

#ifdef WBMD_USE_NAMED_SEMAPHORES
	if(sem_post(semPtr->semPtr) < 0 ) {
#else
	if(sem_post(semPtr) < 0 ) {
#endif

	}
	
	return WBMD_TRUE;
}

// get the value of a semaphore
WBMDBool WbmdThreadSemGetValue(WbmdThreadSem *semPtr, int *valuePtr) {
#ifdef WBMD_USE_NAMED_SEMAPHORES
	if(valuePtr == NULL || semPtr == NULL || semPtr->semPtr == NULL) return WBMD_FALSE;
#else
	if(valuePtr == NULL || semPtr == NULL) return WBMD_FALSE;
#endif

#ifdef WBMD_USE_NAMED_SEMAPHORES
	if(sem_getvalue(semPtr->semPtr, valuePtr) < 0) { // note: broken on Mac OS X? Btw we don't need it
	}
#else
	if(sem_getvalue(semPtr, valuePtr) < 0) {

	}
#endif
	if(*valuePtr < 0 ) {
		*valuePtr = 0;
	}
	
	return WBMD_TRUE;
}


__inline__ sem_t *WbmdThreadGetSemT(WbmdThreadSem *semPtr) {
	#ifdef WBMD_USE_NAMED_SEMAPHORES
		return (semPtr->semPtr);		
	#else		
		return semPtr;	
	#endif
}


// creates a semaphore that can be used with WBMDThreadTimedSemWait(). This type of semaphore
// is different from WBMDThreadSemaphore to support platforms that don't have sem_timedwait() (e.g. Mac OS X)
WBMDBool WbmdThreadCreateTimedSem(WbmdThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	return WbmdThreadCreateSem(semPtr, value);
#else
	// if we don't have sem_timedwait(), the timed semaphore is a pair of unix domain sockets.
	// We write a dummy packet on a socket (client) when we want to post, and select() with timer on the other socket
	// when we want to wait. 
	struct sockaddr_un serverAddr, clientAddr;
	int i;
	
	if(semPtr == NULL) return WBMD_FALSE;
	
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
		if(!WbmdThreadTimedSemPost(semPtr)) return WBMD_FALSE;
	}
	
	return WBMD_TRUE;
#endif
}

// WBMD_TRUE if the semaphore has zero value, WBMD_FALSE otherwise
WBMDBool WbmdThreadTimedSemIsZero(WbmdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	int value;
	if(!WbmdThreadSemGetValue(semPtr, &value)) return WBMD_FALSE;
	
	return (value==0) ? WBMD_TRUE : WBMD_FALSE;
#else
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return WBMD_FALSE;

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
	
	return (r==0) ? WBMD_TRUE : WBMD_FALSE;
#endif
}

WBMDBool WbmdThreadTimedSemSetValue(WbmdThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	// note: we can implement this, but our implemntation does't really need it in case
	// of a system semaphore. This is useful for our Unix Domain Socket Hack
	return WBMD_TRUE;
	//return WBMDErrorRaise(WBMD_ERROR_NEED_RESOURCE, "Operation Not Supported");
#else
	fd_set fset;
	int r, i;
	struct timeval timeout;
	
	if(semPtr == NULL) return WBMD_FALSE;

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
		if(!WbmdThreadTimedSemPost(semPtr)) return WBMD_FALSE;
	}
	
	return WBMD_TRUE;
#endif
}

void WbmdThreadDestroyTimedSem(WbmdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	WbmdThreadDestroySem(semPtr);
#else
	if(semPtr == NULL) return;
	close((*semPtr)[0]);
	close((*semPtr)[1]);
#endif
}

WBMDBool WbmdThreadTimedSemPost(WbmdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	return WbmdThreadSemPost(semPtr);
#else
	char dummy = 'D';
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return WBMD_FALSE;
	

	
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

			return WBMD_FALSE;
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
	

	
	return WBMD_TRUE;
#endif
}


// wrappers for pthread_key_*()
WBMDBool WbmdThreadCreateSpecific(WbmdThreadSpecific *specPtr, void (*destructor)(void *)) {
	if(specPtr == NULL) return WBMD_FALSE;  // NULL destructor is allowed

	if(pthread_key_create(specPtr, destructor) != 0) {

		return WBMD_FALSE;
	}
	
	return WBMD_TRUE;
}

void WbmdThreadDestroySpecific(WbmdThreadSpecific *specPtr) {
	if(specPtr == NULL) return;
	pthread_key_delete(*specPtr);
}

void *WbmdThreadGetSpecific(WbmdThreadSpecific *specPtr) {
	if(specPtr == NULL) return NULL;
	return pthread_getspecific(*specPtr);
}

WBMDBool WbmdThreadSetSpecific(WbmdThreadSpecific *specPtr, void *valPtr) {
	if(specPtr == NULL || valPtr == NULL) return WBMD_FALSE;

	switch(pthread_setspecific(*specPtr, valPtr)) {
		case 0: // success
			break;
		case ENOMEM:
			return WBMD_FALSE;
		default:
			return WBMD_FALSE;
	}
	
	return WBMD_TRUE;
}

// terminate the calling thread
void WbmdExitThread() {
	//printf("\n*** Exit Thread ***\n");
	wbmd_syslog_notice("one wtp exit,thread close");
	pthread_exit((void *) 0);
}


void WbmdThreadSetSignals(int how, int num, ...) {
	sigset_t mask;
	va_list args;
	
	sigemptyset(&mask);
	
	va_start(args, num);
	
	for(; num > 0; num--) {
		sigaddset(&mask, va_arg(args, int));
	}
	
	WbmdThreadSigMask(how, &mask, NULL);
	
	va_end(args);
}



struct {
	WbmdThreadSem requestServiceSem;
	WbmdThreadMutex requestServiceMutex;
	WbmdThreadSem serviceProvidedSem;

	int requestedSec;
	enum {
		WBMD_TIMER_REQUEST,
		WBMD_TIMER_CANCEL,
		WBMD_TIMER_NONE
	} requestedOp;
	WbmdThread *requestedThreadPtr;
	int signalToRaise;
	
	WBMDBool error;
	
	WbmdTimerID timerID;
} gTimersData;

WBMDBool WbmdTimerCancel(WbmdTimerID *idPtr, int isFree) {

	timer_rem(*idPtr, isFree);
	return WBMD_TRUE;
}


void WbmdHandleTimer(void* arg) {
	WBMDThreadTimerArg *a = (WBMDThreadTimerArg*)arg;
 	int TimerType = a->TimerType;
	int WBID = a->WBID;
	int MsgqID;	
	struct WbmdCheckMsgQ msg;
	struct WbmdMsgQ msg2;
	WbmdGetMsgQueue(&MsgqID);
	if(TimerType == WBMD_CHECKING)
	{	
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = 99;
		msg.mqinfo.type = WBMD_CHECK;
		msg.mqinfo.WBID = WBID;		
		if (msgsnd(MsgqID, (struct WbmdCheckMsgQ *)&msg, sizeof(msg.mqinfo), IPC_NOWAIT) == -1){
			wbmd_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");			
			WbmdTimerCancel(&(wBridge[WBID]->CheckTimerID),1);					
			WBMDTimerRequest(10,&(wBridge[WBID]->CheckTimerID),WBMD_CHECKING,wBridge[WBID]->WBID);
		}
		
	}else if(TimerType == WBMD_GETIFINFO){
		memset((char*)&msg2, 0, sizeof(msg2));
		msg2.mqid = WBID%THREAD_NUM + 1;
		msg2.mqinfo.op = WBMD_GET;
		msg2.mqinfo.type = WBMD_IF;		
		msg2.mqinfo.WBID = WBID;		
		if (msgsnd(MsgqID, (struct WbmdMsgQ *)&msg2, sizeof(msg2.mqinfo), IPC_NOWAIT) == -1){
			wbmd_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
			WbmdTimerCancel(&(wBridge[WBID]->GetIfInfoTimes),1);					
			WBMDTimerRequest(10*wBridge[WBID]->GetIfInfoTimes,&(wBridge[WBID]->GetIfInfoTimerID),WBMD_GETIFINFO,wBridge[WBID]->WBID);
		}
	}else if(TimerType == WBMD_GETWBINFO){
		memset((char*)&msg2, 0, sizeof(msg2));
		msg2.mqid = WBID%THREAD_NUM + 1;
		msg2.mqinfo.op = WBMD_GET;
		msg2.mqinfo.type = WBMD_WB;		
		msg2.mqinfo.WBID = WBID;		
		if (msgsnd(MsgqID, (struct WbmdMsgQ *)&msg2, sizeof(msg2.mqinfo), IPC_NOWAIT) == -1){
			wbmd_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}else if(TimerType == WBMD_GETRFINFO){
		memset((char*)&msg2, 0, sizeof(msg2));
		msg2.mqid = WBID%THREAD_NUM + 1;
		msg2.mqinfo.op = WBMD_GET;
		msg2.mqinfo.type = WBMD_RF;		
		msg2.mqinfo.WBID = WBID;		
		if (msgsnd(MsgqID, (struct WbmdMsgQ *)&msg2, sizeof(msg2.mqinfo), IPC_NOWAIT) == -1){
			wbmd_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
			WbmdTimerCancel(&(wBridge[WBID]->GetMintInfoTimes),1);					
			WBMDTimerRequest(11*wBridge[WBID]->GetMintInfoTimes,&(wBridge[WBID]->GetMintInfoTimerID),WBMD_GETMINTINFO,wBridge[WBID]->WBID);
		}
	}else if(TimerType == WBMD_GETMINTINFO){
		memset((char*)&msg2, 0, sizeof(msg2));
		msg2.mqid = WBID%THREAD_NUM + 1;
		msg2.mqinfo.op = WBMD_GET;
		msg2.mqinfo.type = WBMD_MINT; 	
		msg2.mqinfo.WBID = WBID;		
		if (msgsnd(MsgqID, (struct WbmdMsgQ *)&msg2, sizeof(msg2.mqinfo), IPC_NOWAIT) == -1){
			wbmd_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
			WbmdTimerCancel(&(wBridge[WBID]->GetRfInfoTimes),1);					
			WBMDTimerRequest(12*wBridge[WBID]->GetRfInfoTimes,&(wBridge[WBID]->GetRfInfoTimerID),WBMD_GETRFINFO,wBridge[WBID]->WBID);
		}
	}

	WBMD_FREE_OBJECT(a);

	return;
}
WBMDBool WBMDTimerRequest(int sec, WbmdTimerID *idPtr, int TimerType, int WBID) {

	WBMDThreadTimerArg *arg;

	if(sec < 0 || idPtr == NULL) return WBMD_FALSE;
	
	WBMD_CREATE_OBJECT_ERR(arg, WBMDThreadTimerArg, return WBMD_FALSE;);

 	memset(arg, 0 ,sizeof(WBMDThreadTimerArg));
 	arg->TimerType = TimerType;
 	arg->WBID = WBID;
	
	if ((*idPtr = timer_add(sec, 0, &WbmdHandleTimer, arg)) == -1) {

		return WBMD_FALSE;
	}

	return WBMD_TRUE;
}


