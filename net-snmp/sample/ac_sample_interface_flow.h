

#ifndef _AC_SAMPLE_INTERFACE_FLOW_H__
#define _AC_SAMPLE_INTERFACE_FLOW_H__

#include "nm_list.h"

#define U64    		unsigned long long
#define NAME_LEN	128
#define U32			unsigned int

typedef struct trap_state_info
{
	U32		last_send_over_time;
	U32		last_status;	
	
}TRAP_STATE_INFO, *PTRAP_STATE_INFO;


struct if_flow_info_s
{
	unsigned int ifindex;
	char    name[NAME_LEN];
	
	U64  	rxbytes;
	U64     txtytes;
	U64 	packets;
	U64 	drops;
	struct list_head node;

	unsigned int sample_time;
	U64 	rxbandwidth;
	U64 	txbandwidth;
	
	TRAP_STATE_INFO	drop_info;
	//带宽trap发送情况
	TRAP_STATE_INFO  band_info;
};

ac_sample_t *create_ac_sample_interface_flow(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );


#endif



