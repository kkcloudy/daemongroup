#ifndef _IUH_THREAD_H
#define _IUH_THREAD_H

#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include "Iuh_Timerlib.h"

typedef sem_t IuhThreadSem;
typedef int IuhThreadTimedSem[2]; // pair of Unix Domain Socket


typedef pthread_t IuhThread;
typedef pthread_mutex_t IuhThreadMutex;
typedef pthread_cond_t IuhThreadCondition;
typedef pthread_key_t IuhThreadSpecific;
typedef pthread_once_t IuhThreadOnce;

typedef void* (*Iuh_THREAD_FUNCTION)(void*);
typedef int IuhThreadId;

typedef int 	IuhTimerID;
typedef void 	*IuhTimerArg;

typedef struct {
	int index;
	int interfaceIndex;
} IuhThreadArg; // argument passed to the thread func


#define			Iuh_THREAD_RETURN_TYPE						void*
#define			IuhThreadSigMask(how, set, old_set)			pthread_sigmask(how, set, old_set)
#define			IuhThreadIsEqual(t1, t2)						pthread_equal(t1,t2)
#define			IuhThreadSelf()								pthread_self()
#define			IuhThreadKill(t1, signal)					pthread_kill(t1,signal)
#define			IuhThreadSendSignal							IuhThreadKill
#define			Iuh_THREAD_ONCE_INIT							PTHREAD_ONCE_INIT
#define			IuhThreadCallOnce							pthread_once

__inline__ sem_t *IuhThreadGetSemT(IuhThreadSem *semPtr);

IuhBool IuhThreadInitLib(void);
IuhBool IuhCreateThread(IuhThread *newThread, Iuh_THREAD_FUNCTION threadFunc, void *arg, int less);
IuhBool IuhCreateThreadCondition(IuhThreadCondition *theCondition);
void IuhDestroyThreadCondition(IuhThreadCondition *theCondition);
IuhBool IuhWaitThreadCondition(IuhThreadCondition *theCondition, IuhThreadMutex *theMutex);
IuhBool IuhWaitThreadConditionTimeout(IuhThreadCondition *theCondition, IuhThreadMutex *theMutex, struct timespec* pTimeout);
void IuhSignalThreadCondition(IuhThreadCondition *theCondition);
IuhBool IuhCreateThreadMutex(IuhThreadMutex *theMutex);
void IuhDestroyThreadMutex(IuhThreadMutex *theMutex);
IuhBool IuhThreadMutexLock(IuhThreadMutex *theMutex);
IuhBool IuhThreadMutexTryLock(IuhThreadMutex *theMutex);
void IuhThreadMutexUnlock(IuhThreadMutex *theMutex);

IuhBool IuhThreadCreateSem(IuhThreadSem *semPtr, int value);
void IuhThreadDestroySem(IuhThreadSem *semPtr);
IuhBool IuhThreadSemWait(IuhThreadSem *semPtr);
IuhBool IuhThreadSemPost(IuhThreadSem *semPtr);
IuhBool IuhThreadSemGetValue(IuhThreadSem *semPtr, int *valuePtr);

IuhBool IuhThreadCreateSpecific(IuhThreadSpecific *specPtr, void (*destructor)(void *));
void IuhThreadDestroySpecific(IuhThreadSpecific *specPtr);
void *IuhThreadGetSpecific(IuhThreadSpecific *specPtr);
IuhBool IuhThreadSetSpecific(IuhThreadSpecific *specPtr, void *valPtr);

void IuhExitThread(void);

//void *IuhThreadManageTimers(void *arg);
IuhBool IuhTimerCancel(IuhTimerID *idPtr,int isFree);
IuhBool IuhTimerRequest(int sec, IuhThread *threadPtr, IuhTimerID *idPtr, int signalToRaise, int ID);
void IuhThreadSetSignals(int how, int num, ...);

IuhBool IuhThreadCreateTimedSem(IuhThreadTimedSem *semPtr, int value);
IuhBool IuhThreadTimedSemIsZero(IuhThreadTimedSem *semPtr);
IuhBool IuhThreadTimedSemSetValue(IuhThreadTimedSem *semPtr, int value);
void IuhThreadDestroyTimedSem(IuhThreadTimedSem *semPtr);
IuhBool IuhThreadTimedSemWait(IuhThreadTimedSem *semPtr, time_t sec, time_t nsec);
IuhBool IuhThreadTimedSemPost(IuhThreadTimedSem *semPtr);

#endif
