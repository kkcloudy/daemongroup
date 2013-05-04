#ifndef __HAD_LOG_H__
#define __HAD_LOG_H__

/*
 * VRRP log level definition
 */
#define VRRP_LOG_DEF       0x00
#define VRRP_LOG_ALL       0xFF
#define VRRP_LOG_DBG       0x1
#define VRRP_LOG_WAR       0x2
#define VRRP_LOG_ERR       0x4
#define VRRP_LOG_EVT       0x8
#define VRRP_LOG_PKT_REV   0x10
#define VRRP_LOG_PKT_SED   0x20
#define VRRP_LOG_PKT_ALL   0x30
#define VRRP_LOG_PROTOCOL  0x40

#define VRRP_INVALID_FD		(-1)
#define VRRP_FD_INIT		VRRP_INVALID_FD
#define VRRP_PID_BUFFER_LEN		128
#define VRRP_PID_PATH_LEN		VRRP_PID_BUFFER_LEN

#define VRRP_PID_FILE_PATH "/var/run/"
#define VRRP_PID_FILE_PREFIX "had"
#define VRRP_PID_FILE_SUFFIX ".pidmap"

extern int g_vrrp_pid_fd;

 /*
  * VRRP debug info 
  */
/**********************************************************************************
 *	vrrp_syslog_error
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_error
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_warning
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_warning
(
	char *format,
	...
);


/**********************************************************************************
 *	vrrp_syslog_dbg
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_dbg
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_event
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_event
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_protocol
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_protocol
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_packet_send
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_packet_send
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_packet_receive
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_packet_receive
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_packet_all
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_packet_all
(
	char *format,
	...
);

/**********************************************************************************
 *	vrrp_syslog_info
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_info
(
	char *format,
	...
);

int set_debug_value
(
	unsigned int val_mask
);

int set_no_debug_value
(
	unsigned int val_mask
);

/*
 *	write current thread pid to pid file
 */
void vrrp_debug_tell_whoami
(
	char * myName,
	int isLast
);


#if 0
 void vrrp_set_log_level(int level);

 void VRRP_LOG
(
 	unsigned int level,
 	char* format,
 	...
 );
#endif
#endif

