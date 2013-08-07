#ifndef _AC_SAMPLE_INTERFACE_H
#define _AC_SAMPLE_INTERFACE_H


#include "ws_intf.h"

//rtmd provide ac interface flow
/* process NAME */
#define PROCESS_NAME_SE_AGENT			0x1
#define PROCESS_NAME_SNMP			0x2
#define PROCESS_NAME_ACSAMPLE			0x3
#define PROCESS_NAME_RTM			0x4

/*cmd*/
#if 1/*move from if_flow_stats.h*/
#define INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_SNMP		36
#define INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_SNMP			37
#define INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE	38
#define INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_ACSAMPLE		39
#define INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM		40
#define INTERFACE_FLOW_STATISTICS_DATA							41

#endif


#define RTM_TO_SNMP_PATH 		"/var/run/rtm_snmp_path"
#define RTM_TO_ACSAMPLE_PATH 		"/var/run/rtm_snmp_path"

struct sample_rtmd_info {
	int sockfd;
	int process;
	int cmd;
	unsigned int timeout;
};


int sample_rtmd_init_socket(struct sample_rtmd_info *info, int process, int cmd, int timeout);

int sample_rtmd_get_interface_flow(struct sample_rtmd_info *info, int *ifnum, struct if_stats_list **head);

int sample_rtmd_close_socket(struct sample_rtmd_info *info);

#endif
