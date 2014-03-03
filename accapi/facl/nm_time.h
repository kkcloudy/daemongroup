#ifndef _NM_TIME_H
#define _NM_TIME_H
#include <sys/time.h>

int nm_time_init();
int 
nm_time_gettimeofday(struct timeval *tp, void *tzp);
#if 0
void
nm_time_get_relativetime(struct timeval * tp);
#endif
int 
nm_time_set_time(struct timeval *time);

struct timeval
timeval_adjust(struct timeval a);

struct timeval
timeval_subtract(struct timeval a, struct timeval b);

long
timeval_cmp(struct timeval a, struct timeval b);

unsigned long
timeval_elapsed(struct timeval a, struct timeval b);

#endif/*_NM_TIME_H*/

