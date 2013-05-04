#ifndef _BSD_THREAD_H
#define _BSD_THREAD_H

#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include "bsd_timerLib.h"

#ifndef MAX_SLOT_NUM
#define MAX_SLOT_NUM 16
#endif

typedef sem_t BsdThreadSem;
typedef int BsdThreadTimedSem[2]; // pair of Unix Domain Socket


typedef pthread_t BsdThread;
typedef pthread_mutex_t BsdThreadMutex;
typedef pthread_cond_t BsdThreadCondition;
typedef pthread_key_t BsdThreadSpecific;
typedef pthread_once_t BsdThreadOnce;

typedef void* (*BSD_THREAD_FUNCTION)(void*);
typedef int BsdThreadId;

typedef int 	BsdTimerID;
typedef void 	*BsdTimerArg;

enum TimerInfo{
	BSD_CHECKING = 500,
};

typedef struct {
 	int TimerType;
	int InstID;
	int islocaled;
} BSDThreadTimerArg;


#define			BSD_THREAD_RETURN_TYPE						void*
#define			BsdThreadSigMask(how, set, old_set)			pthread_sigmask(how, set, old_set)
#define			BsdThreadIsEqual(t1, t2)						pthread_equal(t1,t2)
#define			BsdThreadSelf()								pthread_self()
#define			BsdThreadKill(t1, signal)					pthread_kill(t1,signal)
#define			BsdThreadSendSignal							BsdThreadKill
#define			BSD_THREAD_ONCE_INIT							PTHREAD_ONCE_INIT
#define			BsdThreadCallOnce							pthread_once

extern BsdThreadCondition fileStateCondition[MAX_SLOT_NUM];	
extern BsdThreadMutex fileStateMutex[MAX_SLOT_NUM];


__inline__ sem_t *BsdThreadGetSemT(BsdThreadSem *semPtr);

BSDBool BsdThreadInitLib(void);
BSDBool BsdCreateThread(BsdThread *newThread, BSD_THREAD_FUNCTION threadFunc, void *arg, int less);
BSDBool BsdCreateThreadCondition(BsdThreadCondition *theCondition);
void BsdDestroyThreadCondition(BsdThreadCondition *theCondition);
BSDBool BsdWaitThreadCondition(BsdThreadCondition *theCondition, BsdThreadMutex *theMutex);
BSDBool BsdWaitThreadConditionTimeout(BsdThreadCondition *theCondition, BsdThreadMutex *theMutex, struct timespec* pTimeout);
void BsdSignalThreadCondition(BsdThreadCondition *theCondition);
BSDBool BsdCreateThreadMutex(BsdThreadMutex *theMutex);
void BsdDestroyThreadMutex(BsdThreadMutex *theMutex);
BSDBool BsdThreadMutexLock(BsdThreadMutex *theMutex);
BSDBool BsdThreadMutexTryLock(BsdThreadMutex *theMutex);
void BsdThreadMutexUnlock(BsdThreadMutex *theMutex);

BSDBool BsdThreadCreateSem(BsdThreadSem *semPtr, int value);
void BsdThreadDestroySem(BsdThreadSem *semPtr);
BSDBool BsdThreadSemWait(BsdThreadSem *semPtr);
BSDBool BsdThreadSemPost(BsdThreadSem *semPtr);
BSDBool BsdThreadSemGetValue(BsdThreadSem *semPtr, int *valuePtr);

BSDBool BsdThreadCreateSpecific(BsdThreadSpecific *specPtr, void (*destructor)(void *));
void BsdThreadDestroySpecific(BsdThreadSpecific *specPtr);
void *BsdThreadGetSpecific(BsdThreadSpecific *specPtr);
BSDBool BsdThreadSetSpecific(BsdThreadSpecific *specPtr, void *valPtr);

void BsdExitThread(void);

//void *BsdThreadManageTimers(void *arg);
BSDBool BsdTimerCancel(BsdTimerID *idPtr,int isFree);
void BsdThreadSetSignals(int how, int num, ...);

BSDBool BsdThreadCreateTimedSem(BsdThreadTimedSem *semPtr, int value);
BSDBool BsdThreadTimedSemIsZero(BsdThreadTimedSem *semPtr);
BSDBool BsdThreadTimedSemSetValue(BsdThreadTimedSem *semPtr, int value);
void BsdThreadDestroyTimedSem(BsdThreadTimedSem *semPtr);
BSDBool BsdThreadTimedSemWait(BsdThreadTimedSem *semPtr, time_t sec, time_t nsec);
BSDBool BsdThreadTimedSemPost(BsdThreadTimedSem *semPtr);

#endif

