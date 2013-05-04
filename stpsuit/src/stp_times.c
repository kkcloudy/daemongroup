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
* stp_times.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for time related op in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* "Times" API : bridgeTime, rootTimes, portTimes, designatedTimes, msgTimes */
 
#include "stp_base.h"

int
stp_times_compare (IN TIMEVALUES_T *t1, IN TIMEVALUES_T *t2)
{
  if (t1->MessageAge < t2->MessageAge)     return -1;
  if (t1->MessageAge > t2->MessageAge)     return  1;

  if (t1->MaxAge < t2->MaxAge)             return -2;
  if (t1->MaxAge > t2->MaxAge)             return  2;

  if (t1->ForwardDelay < t2->ForwardDelay) return -3;
  if (t1->ForwardDelay > t2->ForwardDelay) return  3;

  if (t1->HelloTime < t2->HelloTime)       return -4;
  if (t1->HelloTime > t2->HelloTime)       return  4;
/*mstp*/
  if (t1->RemainingHops< t2->RemainingHops)       return -5;
  if (t1->RemainingHops> t2->RemainingHops)       return  5;
/*mstp end*/
  return 0;
}

void
stp_times_get (IN BPDU_BODY_T *b, OUT TIMEVALUES_T *v)
{
  v->MessageAge =   ntohs (*((unsigned short*) b->message_age))   >> 8;
  v->MaxAge =       ntohs (*((unsigned short*) b->max_age))       >> 8;
  v->ForwardDelay = ntohs (*((unsigned short*) b->forward_delay)) >> 8;
  v->HelloTime =    ntohs (*((unsigned short*) b->hello_time))    >> 8;
}

void
stp_times_set (IN TIMEVALUES_T *v, OUT BPDU_BODY_T *b)
{
  unsigned short mt;
  #define STP_SET_TIME(f, t)        \
     mt = htons (f << 8);           \
     memcpy (t, &mt, 2); 
  
  STP_SET_TIME(v->MessageAge,   b->message_age);
  STP_SET_TIME(v->MaxAge,       b->max_age);
  STP_SET_TIME(v->ForwardDelay, b->forward_delay);
  STP_SET_TIME(v->HelloTime,    b->hello_time);
}

void 
stp_times_copy (OUT TIMEVALUES_T *t, IN TIMEVALUES_T *f)
{
  t->MessageAge = f->MessageAge;
  t->MaxAge = f->MaxAge;
  t->ForwardDelay = f->ForwardDelay;
  t->HelloTime = f->HelloTime;
  t->RemainingHops = f->RemainingHops ; /*mstp*/
}
#ifdef __cplusplus
}
#endif

