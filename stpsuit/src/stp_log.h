#ifndef __STP_LOG_H__
#define __STP_LOG_H__

/*
 * STP log level definition
 */
#define STP_LOG_DEF       0x00
#define STP_LOG_ALL       0xFF
#define STP_LOG_DBG       0x1
#define STP_LOG_WAR       0x2
#define STP_LOG_ERR       0x4
#define STP_LOG_EVT       0x8
#define STP_LOG_PKT_REV   0x10
#define STP_LOG_PKT_SED   0x20
#define STP_LOG_PKT_ALL   0x30
#define STP_LOG_PROTOCOL  0x40
#define STP_SYSTEM_STARTUP_LOG_PATH "/var/run/stp.log"


 /*
  * STP debug info 
  */

void stp_set_log_level(int level);
 
 void STP_LOG
(
 	unsigned int level,
 	char* format,...
 );
#endif
