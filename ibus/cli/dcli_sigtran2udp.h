#ifndef _DCLI_IU_SIGTRAN_H
#define _DCLI_IU_SIGTRAN_H

#define CMD_IU_FAILURE -1
#define CMD_IU_SUCCESS 0

enum iu_debug_type {	
	DEBUG_TYPE_INFO  = 1,
	DEBUG_TYPE_ERROR = 2,
	DEBUG_TYPE_DEBUG = 4,
	DEBUG_TYPE_ALL   = 7,
};

void dcli_sigtran2udp_init(void) ;
#endif 
