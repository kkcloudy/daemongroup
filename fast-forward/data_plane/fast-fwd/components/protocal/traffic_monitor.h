#ifndef _TRAFFIC_MONITOR_H_
#define _TRAFFIC_MONITOR_H_

#define TRAFFIC_STATS_TIME  10

void clear_traffic_stats();
int get_traffic_stats();


extern inline void traffic_statistics(cvmx_wqe_t* work);

#endif

