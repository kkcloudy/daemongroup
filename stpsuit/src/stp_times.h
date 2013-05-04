#ifndef _STP_TIMES_H__
#define _STP_TIMES_H__
/*避免BPDU包被无休止的计算，每个configuration message包
* 都包含一个message age和max age，message age每经过一个
* 交换机加一，当message age>Maxage时，包被丢弃。
* 一下的这些值都是由root设置。
*/
typedef struct timevalues_t {
  unsigned short MessageAge;
  unsigned short MaxAge;
  unsigned short ForwardDelay;
  unsigned short HelloTime;
  unsigned char  RemainingHops;  /*mstp*/
  unsigned char  Reserved[3]; /*mstp*/
} TIMEVALUES_T;

int
stp_times_compare (IN TIMEVALUES_T* t1, IN TIMEVALUES_T* t2);

void
stp_times_get (IN BPDU_BODY_T* b, OUT TIMEVALUES_T* v);

void
stp_times_set (IN TIMEVALUES_T* v, OUT BPDU_BODY_T* b);

void
stp_times_copy (OUT TIMEVALUES_T* t, IN TIMEVALUES_T* f);

#endif /* _RSTP_TIMES_H__ */



