#ifndef _AC_SAMPLE_PORTAL_STATUS_H_
#define _AC_SAMPLE_PORTAL_STATUS_H_

#define PORTAL_SERVER_STATUS 		"/usr/bin/test_portal_server.sh &"
#define PORTAL_SERVER_STATUS_FILE  "/var/run/portal_server_status.log"
#define IP_STR_LEN 		16
#define PORTAL_REACHABLE_VALUE		0	
#define PORTAL_UNREACHABLE_VALUE		100

ac_sample_t *create_ac_sample_portal_server(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time );


#endif

