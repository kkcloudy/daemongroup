#ifndef _HMD_THREAD_H
#define _HMD_THREAD_H

#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include "HmdTimeLib.h"

typedef sem_t HmdThreadSem;
#if 1
typedef HmdThreadSem HmdThreadTimedSem;
#else
typedef int HmdThreadTimedSem[2]; // pair of Unix Domain Socket
#endif

typedef pthread_t HmdThread;
typedef pthread_mutex_t HmdThreadMutex;
typedef pthread_cond_t HmdThreadCondition;
typedef pthread_key_t HmdThreadSpecific;
typedef pthread_once_t HmdThreadOnce;

typedef void* (*HMD_THREAD_FUNCTION)(void*);
typedef int HmdThreadId;

typedef int 	HmdTimerID;
typedef void 	*HmdTimerArg;

enum TimerInfo{
	HMD_CHECKING = 500,
	HMD_CONFIG_LOAD = 501,
	HMD_CHECKING_UPDATE = 502,
	HMD_TIMER_CONFIG_SAVE = 503, //fengwenchao add 20130412 for hmd timer config save
	HMD_CHECK_FOR_DHCP = 504  //supf add for dhcp auto restart
};

typedef struct {
 	int TimerType;
	int ID;
	int islocaled;
} HMDThreadTimerArg;


#define			HMD_THREAD_RETURN_TYPE						void*
#define			HmdThreadSigMask(how, set, old_set)			pthread_sigmask(how, set, old_set)
#define			HmdThreadIsEqual(t1, t2)						pthread_equal(t1,t2)
#define			HmdThreadSelf()								pthread_self()
#define			HmdThreadKill(t1, signal)					pthread_kill(t1,signal)
#define			HmdThreadSendSignal							HmdThreadKill
#define			HMD_THREAD_ONCE_INIT							PTHREAD_ONCE_INIT
#define			HmdThreadCallOnce							pthread_once

__inline__ sem_t *HmdThreadGetSemT(HmdThreadSem *semPtr);

HMDBool HmdThreadInitLib(void);
HMDBool HmdCreateThread(HmdThread *newThread, HMD_THREAD_FUNCTION threadFunc, void *arg, int less);
HMDBool HmdCreateThreadCondition(HmdThreadCondition *theCondition);
void HmdDestroyThreadCondition(HmdThreadCondition *theCondition);
HMDBool HmdWaitThreadCondition(HmdThreadCondition *theCondition, HmdThreadMutex *theMutex);
HMDBool HmdWaitThreadConditionTimeout(HmdThreadCondition *theCondition, HmdThreadMutex *theMutex, struct timespec* pTimeout);
void HmdSignalThreadCondition(HmdThreadCondition *theCondition);
HMDBool HmdCreateThreadMutex(HmdThreadMutex *theMutex);
void HmdDestroyThreadMutex(HmdThreadMutex *theMutex);
HMDBool HmdThreadMutexLock(HmdThreadMutex *theMutex);
HMDBool HmdThreadMutexTryLock(HmdThreadMutex *theMutex);
void HmdThreadMutexUnlock(HmdThreadMutex *theMutex);

HMDBool HmdThreadCreateSem(HmdThreadSem *semPtr, int value);
void HmdThreadDestroySem(HmdThreadSem *semPtr);
HMDBool HmdThreadSemWait(HmdThreadSem *semPtr);
HMDBool HmdThreadSemPost(HmdThreadSem *semPtr);
HMDBool HmdThreadSemGetValue(HmdThreadSem *semPtr, int *valuePtr);

HMDBool HmdThreadCreateSpecific(HmdThreadSpecific *specPtr, void (*destructor)(void *));
void HmdThreadDestroySpecific(HmdThreadSpecific *specPtr);
void *HmdThreadGetSpecific(HmdThreadSpecific *specPtr);
HMDBool HmdThreadSetSpecific(HmdThreadSpecific *specPtr, void *valPtr);

void HmdExitThread(void);

//void *HmdThreadManageTimers(void *arg);
HMDBool HmdTimerCancel(HmdTimerID *idPtr,int isFree);
HMDBool HMDTimerRequest(int sec, HmdTimerID *idPtr, int TimerType, int InstID, int islocaled);
void HmdThreadSetSignals(int how, int num, ...);

HMDBool HmdThreadCreateTimedSem(HmdThreadTimedSem *semPtr, int value);
HMDBool HmdThreadTimedSemIsZero(HmdThreadTimedSem *semPtr);
HMDBool HmdThreadTimedSemSetValue(HmdThreadTimedSem *semPtr, int value);
void HmdThreadDestroyTimedSem(HmdThreadTimedSem *semPtr);
HMDBool HmdThreadTimedSemWait(HmdThreadTimedSem *semPtr, time_t sec, time_t nsec);
HMDBool HmdThreadTimedSemPost(HmdThreadTimedSem *semPtr);

#endif

