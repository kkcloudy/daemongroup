#ifndef _WBMD_THREAD_H
#define _WBMD_THREAD_H

#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include "wbmd_timelib.h"

typedef sem_t WbmdThreadSem;
typedef WbmdThreadSem WbmdThreadTimedSem;


typedef pthread_t WbmdThread;
typedef pthread_mutex_t WbmdThreadMutex;
typedef pthread_cond_t WbmdThreadCondition;
typedef pthread_key_t WbmdThreadSpecific;
typedef pthread_once_t WbmdThreadOnce;

typedef void* (*WBMD_THREAD_FUNCTION)(void*);
typedef int WbmdThreadId;

typedef int 	WbmdTimerID;
typedef void 	*WbmdTimerArg;

enum TimerInfo{
	WBMD_CHECKING = 500,
	WBMD_GETIFINFO = 501,
	WBMD_GETWBINFO = 502,
	WBMD_GETRFINFO = 503,
	WBMD_GETMINTINFO = 504,
};

typedef struct {
 	int TimerType;
	int WBID;
} WBMDThreadTimerArg;


#define			WBMD_THREAD_RETURN_TYPE						void*
#define			WbmdThreadSigMask(how, set, old_set)			pthread_sigmask(how, set, old_set)
#define			WbmdThreadIsEqual(t1, t2)						pthread_equal(t1,t2)
#define			WbmdThreadSelf()								pthread_self()
#define			WbmdThreadKill(t1, signal)					pthread_kill(t1,signal)
#define			WbmdThreadSendSignal							WbmdThreadKill
#define			WBMD_THREAD_ONCE_INIT							PTHREAD_ONCE_INIT
#define			WbmdThreadCallOnce							pthread_once

__inline__ sem_t *WbmdThreadGetSemT(WbmdThreadSem *semPtr);

WBMDBool WbmdThreadInitLib(void);
WBMDBool WbmdCreateThread(WbmdThread *newThread, WBMD_THREAD_FUNCTION threadFunc, void *arg, int less);
WBMDBool WbmdCreateThreadCondition(WbmdThreadCondition *theCondition);
void WbmdDestroyThreadCondition(WbmdThreadCondition *theCondition);
WBMDBool WbmdWaitThreadCondition(WbmdThreadCondition *theCondition, WbmdThreadMutex *theMutex);
WBMDBool WbmdWaitThreadConditionTimeout(WbmdThreadCondition *theCondition, WbmdThreadMutex *theMutex, struct timespec* pTimeout);
void WbmdSignalThreadCondition(WbmdThreadCondition *theCondition);
WBMDBool WbmdCreateThreadMutex(WbmdThreadMutex *theMutex);
void WbmdDestroyThreadMutex(WbmdThreadMutex *theMutex);
WBMDBool WbmdThreadMutexLock(WbmdThreadMutex *theMutex);
WBMDBool WbmdThreadMutexTryLock(WbmdThreadMutex *theMutex);
void WbmdThreadMutexUnlock(WbmdThreadMutex *theMutex);

WBMDBool WbmdThreadCreateSem(WbmdThreadSem *semPtr, int value);
void WbmdThreadDestroySem(WbmdThreadSem *semPtr);
WBMDBool WbmdThreadSemWait(WbmdThreadSem *semPtr);
WBMDBool WbmdThreadSemPost(WbmdThreadSem *semPtr);
WBMDBool WbmdThreadSemGetValue(WbmdThreadSem *semPtr, int *valuePtr);

WBMDBool WbmdThreadCreateSpecific(WbmdThreadSpecific *specPtr, void (*destructor)(void *));
void WbmdThreadDestroySpecific(WbmdThreadSpecific *specPtr);
void *WbmdThreadGetSpecific(WbmdThreadSpecific *specPtr);
WBMDBool WbmdThreadSetSpecific(WbmdThreadSpecific *specPtr, void *valPtr);

void WbmdExitThread(void);

//void *WbmdThreadManageTimers(void *arg);
WBMDBool WbmdTimerCancel(WbmdTimerID *idPtr,int isFree);
WBMDBool WBMDTimerRequest(int sec, WbmdTimerID *idPtr, int TimerType, int WBID);
void WbmdThreadSetSignals(int how, int num, ...);

WBMDBool WbmdThreadCreateTimedSem(WbmdThreadTimedSem *semPtr, int value);
WBMDBool WbmdThreadTimedSemIsZero(WbmdThreadTimedSem *semPtr);
WBMDBool WbmdThreadTimedSemSetValue(WbmdThreadTimedSem *semPtr, int value);
void WbmdThreadDestroyTimedSem(WbmdThreadTimedSem *semPtr);
WBMDBool WbmdThreadTimedSemPost(WbmdThreadTimedSem *semPtr);

#endif

