#ifndef _AC_SAMPLE_RADIUS_STATUS_H_
#define _AC_SAMPLE_RADIUS_STATUS_H_

#define RADIUS_REACHABLE_VALUE		0	
#define RADIUS_UNREACHABLE_VALUE		100
#define URANDOM_FILE_PATH 	"/dev/urandom"

#define RADIUS_USER_DATA_OK		1
#define RADIUS_ERR_BASE				-500
#define FILE_OPEN_ERR				(RADIUS_ERR_BASE-1)
#define SOCKET_CREATE_ERR			(RADIUS_ERR_BASE-2)
#define RADIUS_DEFAULT_PACK_ERR	(RADIUS_ERR_BASE-3)
#define RADIUS_ADDATTR_ERR		(RADIUS_ERR_BASE-4)

ac_sample_t *create_ac_sample_radius_auth(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );

ac_sample_t *create_ac_sample_radius_count(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );



#endif
