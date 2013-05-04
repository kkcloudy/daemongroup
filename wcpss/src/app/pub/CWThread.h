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
* CWThread.h
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


#ifndef __CAPWAP_CWThread_HEADER__
#define __CAPWAP_CWThread_HEADER__

#include "CWErrorHandling.h"
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
//#include "CWTimer.h"
#include "timerlib.h"

#ifdef __APPLE__
	// Mac OS X
	#define CW_USE_NAMED_SEMAPHORES
	#include <unistd.h>
	typedef struct {
		sem_t *semPtr;
	} CWThreadSem;
#else
	typedef sem_t CWThreadSem;
#endif


#ifdef HAVE_SEM_TIMEDWAIT
	typedef CWThreadSem CWThreadTimedSem;
#else	
	typedef int CWThreadTimedSem[2]; // pair of Unix Domain Socket
#endif


typedef pthread_t CWThread;
typedef pthread_mutex_t CWThreadMutex;
typedef pthread_cond_t CWThreadCondition;
typedef pthread_key_t CWThreadSpecific;
typedef pthread_once_t CWThreadOnce;

typedef void* (*CW_THREAD_FUNCTION)(void*);
typedef int CWThreadId;

typedef int 	CWTimerID;
typedef void 	*CWTimerArg;

#define			CW_THREAD_RETURN_TYPE						void*
#define			CWThreadSigMask(how, set, old_set)			pthread_sigmask(how, set, old_set)
#define			CWThreadIsEqual(t1, t2)						pthread_equal(t1,t2)
#define			CWThreadSelf()								pthread_self()
#define			CWThreadKill(t1, signal)					pthread_kill(t1,signal)
#define			CWThreadSendSignal							CWThreadKill
#define			CW_THREAD_ONCE_INIT							PTHREAD_ONCE_INIT
#define			CWThreadCallOnce							pthread_once

__inline__ sem_t *CWThreadGetSemT(CWThreadSem *semPtr);

CWBool CWThreadInitLib(void);
CWBool CWCreateThread(CWThread *newThread, CW_THREAD_FUNCTION threadFunc, void *arg, int less);
CWBool CWCreateThreadCondition(CWThreadCondition *theCondition);
void CWDestroyThreadCondition(CWThreadCondition *theCondition);
CWBool CWWaitThreadCondition(CWThreadCondition *theCondition, CWThreadMutex *theMutex);
CWBool CWWaitThreadConditionTimeout(CWThreadCondition *theCondition, CWThreadMutex *theMutex, struct timespec* pTimeout);
void CWSignalThreadCondition(CWThreadCondition *theCondition);
CWBool CWCreateThreadMutex(CWThreadMutex *theMutex);
void CWDestroyThreadMutex(CWThreadMutex *theMutex);
CWBool CWThreadMutexLock(CWThreadMutex *theMutex);
CWBool CWThreadMutexTryLock(CWThreadMutex *theMutex);
void CWThreadMutexUnlock(CWThreadMutex *theMutex);

CWBool CWThreadCreateSem(CWThreadSem *semPtr, int value);
void CWThreadDestroySem(CWThreadSem *semPtr);
CWBool CWThreadSemWait(CWThreadSem *semPtr);
CWBool CWThreadSemPost(CWThreadSem *semPtr);
CWBool CWThreadSemGetValue(CWThreadSem *semPtr, int *valuePtr);

CWBool CWThreadCreateSpecific(CWThreadSpecific *specPtr, void (*destructor)(void *));
void CWThreadDestroySpecific(CWThreadSpecific *specPtr);
void *CWThreadGetSpecific(CWThreadSpecific *specPtr);
CWBool CWThreadSetSpecific(CWThreadSpecific *specPtr, void *valPtr);

void CWExitThread(void);

//void *CWThreadManageTimers(void *arg);
CWBool CWTimerCancel(CWTimerID *idPtr,int isFree);
CWBool CWTimerRequest(int sec, CWThread *threadPtr, CWTimerID *idPtr, int signalToRaise, int ID);
void CWThreadSetSignals(int how, int num, ...);

CWBool CWThreadCreateTimedSem(CWThreadTimedSem *semPtr, int value);
CWBool CWThreadTimedSemIsZero(CWThreadTimedSem *semPtr);
CWBool CWThreadTimedSemSetValue(CWThreadTimedSem *semPtr, int value);
void CWThreadDestroyTimedSem(CWThreadTimedSem *semPtr);
#if 0
CWBool CWThreadTimedSemWait(CWThreadTimedSem *semPtr, time_t sec, time_t nsec);
#endif
CWBool CWThreadTimedSemPost(CWThreadTimedSem *semPtr);

#endif
