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
#include <sys/wait.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <semaphore.h>
#include <sys/msg.h>
#include "hmd.h"
#include "hmd/hmdpub.h"
#include "HmdThread.h"
#include "HmdTimeLib.h"
#include "HmdLog.h"
#include "HmdDbus.h"

#define HMD_USE_THREAD_TIMERS
#define HAVE_SEM_TIMEDWAIT

HMD_THREAD_RETURN_TYPE HmdThreadManageTimers(void *arg);

// Creates a thread that will execute a given function with a given parameter
HMDBool HmdCreateThread(HmdThread *newThread, HMD_THREAD_FUNCTION threadFunc, void *arg, int less) {
	pthread_attr_t attr;
	size_t ss;	
	int s = PTHREAD_CREATE_DETACHED;
	if(newThread == NULL) return HMD_FALSE;
	
	pthread_attr_init(&attr);
	if(less){
		pthread_attr_getstacksize(&attr,&ss);	
		ss=(ss*3)/4;
		pthread_attr_setstacksize(&attr,ss);
	}
	pthread_attr_setdetachstate(&attr,s);
		
	if(pthread_create(newThread, &attr, threadFunc, arg) != 0) {
		return HMD_FALSE;
	}

	return HMD_TRUE;
}

// Creates a thread condition (wrapper for pthread_cond_init)
HMDBool HmdCreateThreadCondition(HmdThreadCondition *theCondition) {
	if(theCondition == NULL) return HMD_FALSE;
	
	switch(pthread_cond_init(theCondition, NULL)) {
		case 0: // success
			break;
		default:
			return HMD_FALSE;
	}
	return HMD_TRUE;
}

// Frees a thread condition (wrapper for pthread_cond_destroy)
void HmdDestroyThreadCondition(HmdThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	pthread_cond_destroy(theCondition);
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
HMDBool HmdWaitThreadCondition(HmdThreadCondition *theCondition, HmdThreadMutex *theMutex) {
	if(theCondition == NULL || theMutex == NULL) return HMD_FALSE;
	
	switch(pthread_cond_wait(theCondition, theMutex)) {
		case 0: // success
			break;
		case  ETIMEDOUT:
			return HMD_FALSE;
		default:
			return HMD_FALSE;	
	}
	
	return HMD_TRUE;
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
HMDBool HmdWaitThreadConditionTimeout(HmdThreadCondition *theCondition, HmdThreadMutex *theMutex, struct timespec* pTimeout) {
	if(theCondition == NULL || theMutex == NULL) return HMD_FALSE;
	
	switch(pthread_cond_timedwait(theCondition, theMutex, pTimeout)) {
		case 0: // success
			break;

		case ETIMEDOUT:
			return HMD_FALSE;

		default:
			return HMD_FALSE;	
	}
	
	return HMD_TRUE;
}

// Signal a thread condition (wrapper for pthread_cond_signal)
void HmdSignalThreadCondition(HmdThreadCondition *theCondition) {
	if(theCondition == NULL) return;
	
	pthread_cond_signal(theCondition);
}

// Creates a thread mutex (wrapper for pthread_mutex_init)
HMDBool HmdCreateThreadMutex(HmdThreadMutex *theMutex) {
	if(theMutex == NULL) return HMD_FALSE;
	
	switch(pthread_mutex_init(theMutex, NULL)) {
		case 0: // success
			break;
		case ENOMEM:
			return HMD_FALSE;
		default:
			return HMD_FALSE;
	}
	return HMD_TRUE;
}


// Free a thread mutex (wrapper for pthread_mutex_destroy)
void HmdDestroyThreadMutex(HmdThreadMutex *theMutex)  {
	if(theMutex == NULL) return;
	pthread_mutex_destroy( theMutex );
}

// locks a mutex among threads at the specified address (blocking)
HMDBool HmdThreadMutexLock(HmdThreadMutex *theMutex) {
	if(theMutex == NULL) return HMD_FALSE;
	if(pthread_mutex_lock( theMutex ) != 0) {
		return HMD_FALSE;
	}
/*
	fprintf(stdout, "Mutex %p locked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
	return HMD_TRUE;
}

// locks a mutex among threads at the specified address (non-blocking).
// HMD_TRUE if lock was acquired, HMD_FALSE otherwise
HMDBool HmdThreadMutexTryLock(HmdThreadMutex *theMutex) {
	if(theMutex == NULL) {
		return HMD_FALSE;
	}
	if(pthread_mutex_trylock( theMutex ) == EBUSY) return HMD_FALSE;
	else return HMD_TRUE;
}

// unlocks a mutex among threads at the specified address
void HmdThreadMutexUnlock(HmdThreadMutex *theMutex) {
	if(theMutex == NULL) return;
	pthread_mutex_unlock( theMutex );
/*
	fprintf(stdout, "Mutex %p UNlocked by %p.\n", theMutex, pthread_self());
	fflush(stdout);
*/
}

// creates a semaphore
HMDBool HmdThreadCreateSem(HmdThreadSem *semPtr, int value) {
	if(semPtr == NULL) return HMD_FALSE;
	
	// we use named semaphore on platforms that support only them (e.g. Mac OS X)
	#ifdef HMD_USE_NAMED_SEMAPHORES
	{
		static int semCount = 0;
		char name[32];

		snprintf(name, 32, "/HmdSem-%d-%4.4d", getpid(), semCount++);
		if ( (semPtr->semPtr = sem_open(name, O_CREAT, 0600, value)) == (sem_t *)SEM_FAILED ) {
			HmdErrorRaiseSystemError(HMD_ERROR_GENERAL);
		} else {
			sem_unlink(name);
		}
	}
	#else
		if ( sem_init(semPtr, 0, value) < 0 ) {

		}
	#endif
	
	return HMD_TRUE;
}

// destroy a semaphore
void HmdThreadDestroySem(HmdThreadSem *semPtr) {
#ifdef HMD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return;
#else
	if(semPtr == NULL) return;
#endif
	
	#ifdef HMD_USE_NAMED_SEMAPHORES
		sem_close(semPtr->semPtr);
	#else
		sem_destroy(semPtr);
	#endif
}

// perform wait on a semaphore
HMDBool HmdThreadSemWait(HmdThreadSem *semPtr) {
#ifdef HMD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return HMD_FALSE;
#else
	if(semPtr == NULL) return HMD_FALSE;
#endif

	//HMDDebugLog("Sem Wait");
	
#ifdef HMD_USE_NAMED_SEMAPHORES
	while(sem_wait(semPtr->semPtr) < 0 ) {
#else
	while(sem_wait(semPtr) < 0 ) {
#endif
		if(errno == EINTR) continue;

		else {

		}
	}
	
	return HMD_TRUE;
}

// perform post on a semaphore
HMDBool HmdThreadSemPost(HmdThreadSem *semPtr) {
#ifdef HMD_USE_NAMED_SEMAPHORES
	if(semPtr == NULL || semPtr->semPtr == NULL) return HMD_FALSE;
#else
	if(semPtr == NULL) return HMD_FALSE;
#endif

#ifdef HMD_USE_NAMED_SEMAPHORES
	if(sem_post(semPtr->semPtr) < 0 ) {
#else
	if(sem_post(semPtr) < 0 ) {
#endif

	}
	
	return HMD_TRUE;
}

// get the value of a semaphore
HMDBool HmdThreadSemGetValue(HmdThreadSem *semPtr, int *valuePtr) {
#ifdef HMD_USE_NAMED_SEMAPHORES
	if(valuePtr == NULL || semPtr == NULL || semPtr->semPtr == NULL) return HMD_FALSE;
#else
	if(valuePtr == NULL || semPtr == NULL) return HMD_FALSE;
#endif

#ifdef HMD_USE_NAMED_SEMAPHORES
	if(sem_getvalue(semPtr->semPtr, valuePtr) < 0) { // note: broken on Mac OS X? Btw we don't need it
	}
#else
	if(sem_getvalue(semPtr, valuePtr) < 0) {

	}
#endif
	if(*valuePtr < 0 ) {
		*valuePtr = 0;
	}
	
	return HMD_TRUE;
}


__inline__ sem_t *HmdThreadGetSemT(HmdThreadSem *semPtr) {
	#ifdef HMD_USE_NAMED_SEMAPHORES
		return (semPtr->semPtr);		
	#else		
		return semPtr;	
	#endif
}


// creates a semaphore that can be used with HMDThreadTimedSemWait(). This type of semaphore
// is different from HMDThreadSemaphore to support platforms that don't have sem_timedwait() (e.g. Mac OS X)
HMDBool HmdThreadCreateTimedSem(HmdThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	return HmdThreadCreateSem(semPtr, value);
#else
	// if we don't have sem_timedwait(), the timed semaphore is a pair of unix domain sockets.
	// We write a dummy packet on a socket (client) when we want to post, and select() with timer on the other socket
	// when we want to wait. 
	struct sockaddr_un serverAddr, clientAddr;
	int i;
	
	if(semPtr == NULL) return HMD_FALSE;
	
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
		if(!HmdThreadTimedSemPost(semPtr)) return HMD_FALSE;
	}
	
	return HMD_TRUE;
#endif
}

// HMD_TRUE if the semaphore has zero value, HMD_FALSE otherwise
HMDBool HmdThreadTimedSemIsZero(HmdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	int value;
	if(!HmdThreadSemGetValue(semPtr, &value)) return HMD_FALSE;
	
	return (value==0) ? HMD_TRUE : HMD_FALSE;
#else
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return HMD_FALSE;

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
	
	return (r==0) ? HMD_TRUE : HMD_FALSE;
#endif
}

HMDBool HmdThreadTimedSemSetValue(HmdThreadTimedSem *semPtr, int value) {
#ifdef HAVE_SEM_TIMEDWAIT
	// note: we can implement this, but our implemntation does't really need it in case
	// of a system semaphore. This is useful for our Unix Domain Socket Hack
	return HMD_TRUE;
	//return HMDErrorRaise(HMD_ERROR_NEED_RESOURCE, "Operation Not Supported");
#else
	fd_set fset;
	int r, i;
	struct timeval timeout;
	
	if(semPtr == NULL) return HMD_FALSE;

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
		if(!HmdThreadTimedSemPost(semPtr)) return HMD_FALSE;
	}
	
	return HMD_TRUE;
#endif
}

void HmdThreadDestroyTimedSem(HmdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	HmdThreadDestroySem(semPtr);
#else
	if(semPtr == NULL) return;
	close((*semPtr)[0]);
	close((*semPtr)[1]);
#endif
}

HMDBool HmdThreadTimedSemPost(HmdThreadTimedSem *semPtr) {
#ifdef HAVE_SEM_TIMEDWAIT
	return HmdThreadSemPost(semPtr);
#else
	char dummy = 'D';
	fd_set fset;
	int r;
	struct timeval timeout;
	
	if(semPtr == NULL) return HMD_FALSE;
	

	
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

			return HMD_FALSE;
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
	

	
	return HMD_TRUE;
#endif
}


// wrappers for pthread_key_*()
HMDBool HmdThreadCreateSpecific(HmdThreadSpecific *specPtr, void (*destructor)(void *)) {
	if(specPtr == NULL) return HMD_FALSE;  // NULL destructor is allowed

	if(pthread_key_create(specPtr, destructor) != 0) {

		return HMD_FALSE;
	}
	
	return HMD_TRUE;
}

void HmdThreadDestroySpecific(HmdThreadSpecific *specPtr) {
	if(specPtr == NULL) return;
	pthread_key_delete(*specPtr);
}

void *HmdThreadGetSpecific(HmdThreadSpecific *specPtr) {
	if(specPtr == NULL) return NULL;
	return pthread_getspecific(*specPtr);
}

HMDBool HmdThreadSetSpecific(HmdThreadSpecific *specPtr, void *valPtr) {
	if(specPtr == NULL || valPtr == NULL) return HMD_FALSE;

	switch(pthread_setspecific(*specPtr, valPtr)) {
		case 0: // success
			break;
		case ENOMEM:
			return HMD_FALSE;
		default:
			return HMD_FALSE;
	}
	
	return HMD_TRUE;
}

// terminate the calling thread
void HmdExitThread() {
	//printf("\n*** Exit Thread ***\n");
	hmd_syslog_notice("one wtp exit,thread close");
	pthread_exit((void *) 0);
}


void HmdThreadSetSignals(int how, int num, ...) {
	sigset_t mask;
	va_list args;
	
	sigemptyset(&mask);
	
	va_start(args, num);
	
	for(; num > 0; num--) {
		sigaddset(&mask, va_arg(args, int));
	}
	
	HmdThreadSigMask(how, &mask, NULL);
	
	va_end(args);
}



struct {
	HmdThreadSem requestServiceSem;
	HmdThreadMutex requestServiceMutex;
	HmdThreadSem serviceProvidedSem;

	int requestedSec;
	enum {
		HMD_TIMER_REQUEST,
		HMD_TIMER_CANCEL,
		HMD_TIMER_NONE
	} requestedOp;
	HmdThread *requestedThreadPtr;
	int signalToRaise;
	
	HMDBool error;
	
	HmdTimerID timerID;
} gTimersData;

HMDBool HmdTimerCancel(HmdTimerID *idPtr, int isFree) {

	timer_rem(*idPtr, isFree);
	return HMD_TRUE;
}


void HmdHandleTimer(void* arg) {
	struct HmdMsgQ msg;	
	HMDThreadTimerArg *a = (HMDThreadTimerArg*)arg;
 	int TimerType = a->TimerType;
	int InstID = a->ID;
	int slotid = a->ID;
	int islocaled = a->islocaled;
	int MsgqID;	
	int issend = 0;	
	char ConfigPath[]="/var/run/config";
	char ConfigFile[]="Instconfig";	
	char ConfigFileNew[DEFAULT_LEN] = {0};
	/*fengwenchao add 20130412 for hmd timer config save begin*/
	char cmd[DEFAULT_LEN] = {0};  
	int ret = 0; 
	int reason = 0;
	/*fengwenchao add 20130412 for hmd timer config save end*/
	HmdGetMsgQueue(&MsgqID);
	if(TimerType == HMD_CHECKING)
	{
		if(islocaled){
			if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
				islocaled = HMD_LOCAL_HANSI;
				issend = 1;
			}
		}else{
			if(HOST_BOARD->Hmd_Inst[InstID] != NULL){
				islocaled = HMD_HANSI;
				issend = 1;
			}
		}
		if(issend){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = islocaled*(MAX_INSTANCE) + InstID;
			msg.mqinfo.op = HMD_HANSI_CHECKING;
			msg.mqinfo.type = islocaled;
			msg.mqinfo.InstID = InstID;
			if (msgsnd(MsgqID, (struct HmdMsgQ *)&msg, sizeof(msg.mqinfo), 0) == -1){
				perror("msgsnd");
			}
		}
	}
	else if(TimerType == HMD_CONFIG_LOAD){
		memset(ConfigFileNew, 0, DEFAULT_LEN);
		sprintf(ConfigFileNew,"%s%d",ConfigFile,slotid);
		hmd_load_slot_config("/mnt/rtsuit", "rtmd.conf",1);
		hmd_load_slot_config("/mnt", "dhcp.conf",1);
		hmd_load_slot_config(ConfigPath, ConfigFileNew,0);
	}else if(HMD_CHECKING_UPDATE == TimerType){
		hmd_syslog_info("%s,%d,HMD_CHECKING_UPDATE ageing.\n",__func__,__LINE__);
		hmd_notice_vrrp_config_service_change_state(InstID,0);
		sleep(2);
		hmd_notice_vrrp_config_service_change_state(InstID,1);
	}
	else if(HMD_TIMER_CONFIG_SAVE == TimerType)     //fengwenchao add 20130412 for hmd timer config save
	{
		hmd_syslog_info("%s %d HMD_TIMER_CONFIG_SAVE begin\n",__func__,__LINE__);
		hmd_config_save_timer_init(1);

		sprintf(cmd,"sudo /opt/bin/vtysh -c  \"show running\n\"");
		ret = system(cmd);
		hmd_syslog_info("%s ret:%d(0 success)\n",__func__,ret);
		reason = WEXITSTATUS(ret);	
		if(reason != 0)
		{
			hmd_syslog_err("%s HMD_TIMER_CONFIG_SAVE fail reason =%d\n",__func__,reason);
		}
		sleep(5);
		
		hmd_config_save_timer_init(0);	
		if((isMaster)&&(isActive)&&(HANSI_TIMER_CONFIG_SAVE == 1))
			HMDTimerRequest(HANSI_TIMER,&(HOST_BOARD->HMDTimer_ConfigSave), HMD_TIMER_CONFIG_SAVE, 0, 0);		
	}
	HMD_FREE_OBJECT(a);

	return;
}
HMDBool HMDTimerRequest(int sec, HmdTimerID *idPtr, int TimerType, int ID, int islocaled) {

	HMDThreadTimerArg *arg;

	if(sec < 0 || idPtr == NULL) return HMD_FALSE;
	
	HMD_CREATE_OBJECT_ERR(arg, HMDThreadTimerArg, return HMD_FALSE;);

 	memset(arg, 0 ,sizeof(HMDThreadTimerArg));
 	arg->TimerType = TimerType;
 	arg->ID = ID;
	arg->islocaled = islocaled;
	
	if ((*idPtr = timer_add(sec, 0, &HmdHandleTimer, arg)) == -1) {

		return HMD_FALSE;
	}

	return HMD_TRUE;
}


