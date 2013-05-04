/* trap-util.h */

#ifndef TRAP_UTIL_H
#define TRAP_UTIL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <dbus/dbus.h>
#include "ws_dbus_def.h"

#define MAXWTPID 4096
//#define MAXAPTRAPNUM 56
//#define MAXACTRAPNUM 32


#define ENTERPRISE_OID_FILE 	"/devinfo/enterprise_snmp_oid"
#define PRODUCT_OID_FILE		"/devinfo/snmp_sys_oid"
#define MAC_FILE				"/devinfo/base_mac"
#define VRRPNUM 17 //0-15 hansi 16 ac
#define TRAP_HANSI_ALL 0

#define TRAP_HANSI_STATE 0
#define TRAP_HANSI_LAST_STATE 1
#define TRAP_HANSI_MASTER 1
#define TRAP_HANSI_BACKUP 0

#define TRAP_MALLOC_ERROR			2
#define TRAP_RESEND_SUCCEED		3
#define TRAP_RESEND_FAILED			4
#define TRAP_OK				0
#define TRAP_ERROR			1
#define TRAP_TRUE			1
#define TRAP_FALSE			0


typedef void (* SystemSignalHandlerFunc) (int sig);

int trap_get_product_info(char *filename);

void trap_set_system_signal_handler (int sig, SystemSignalHandlerFunc func);

void trap_openlog (void);
void trap_syslog (int priority, const char *message, ...);
void trap_syslogv(int priority, const char *message, va_list args);

int trap_file_exists (const char *file);

int trap_write_pid_file(const char *filename);
int trap_become_daemon(void);
void *trap_malloc (unsigned int malloc_size, const char * malloc_file_name, const char * malloc_func_name, int malloc_line_num );


typedef struct SystemInfo {
	char enterprise_oid[16];
	char product_oid[32];
	unsigned char ac_mac[6];
	char hostname[128];
} SystemInfo;

extern SystemInfo gSysInfo;

typedef struct GlobalInfo {
	int keep_loop;
//	int vrrp_switch;
//	int vrrp_last_state;
} GlobalInfo;

extern GlobalInfo gGlobalInfo;
extern DBusConnection *ccgi_dbus_connection;

#define TRAP_MALLOC(size) trap_malloc(size,__FILE__,__FUNCTION__,__LINE__)

#define TRAP_FREE(s)	do {\
	if (s) {	\
		TRAP_TRACE_LOG(LOG_DEBUG,"free: address is %p\n",s);	\
		free((void *)s);	\
		s=NULL;		\
	}else{	\
		TRAP_TRACE_LOG(LOG_DEBUG,"free: error ptr is NULL\n");	\
	}	\
} while(0)

//int trap_is_ap_trap_enabled(TrapVrrpState *vrrpState, int vrrp_id);

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define _TRAP_FUNCTION_NAME __func__
#elif defined(__GNUC__) || defined(_MSC_VER)
#define _TRAP_FUNCTION_NAME __FUNCTION__
#else
#define _TRAP_FUNCTION_NAME "unknown function"
#endif

/***this function not detected log_level param***/
#define TRAP_RETURN_VAL_IF_FAILED(condition, val, log_level) do {   \
     if (!(condition)) {                                                                   \
    trap_syslog (log_level,"arguments to %s() were incorrect, assertion \"%s\" failed in file %s line %d.\n",                       \
                             _TRAP_FUNCTION_NAME, #condition, __FILE__, __LINE__);      \
    return (val);                                                                       \
  } } while (0)
  
#define TRAP_RETURN_IF_FAILED(condition, log_level) do {   \
		  if (!(condition)) {																	\
		 trap_syslog (log_level,"arguments to %s() were incorrect, assertion \"%s\" failed in file %s line %d.\n",						 \
								  _TRAP_FUNCTION_NAME, #condition, __FILE__, __LINE__); 	 \
		 return;																		 \
  } } while (0)

#define TRAP_TRACE_LOG(log_level,format,... ) do {  \
	trap_syslog(log_level,"[%s@%s,%d]:"format"\n",_TRAP_FUNCTION_NAME,__FILE__,__LINE__,##__VA_ARGS__);\
}while (0)

char *trap_get_enterprise_oid(char *enterprise_oid, int len);

char *trap_get_product_oid(char *product_oid, int len);

unsigned char *trap_get_ac_mac(unsigned char *mac, int len);

char *trap_get_hostname(char *hostname, int len);

void trap_system_info_get(SystemInfo *sysInfo);

char *trap_get_time_str(char *time_str, int len);

void trap_util_test(void);


#define ACIPSIZ 128

typedef struct {
    unsigned int local_id;
    unsigned int instance_id;
    
	int interval;  // in seconds
	int mode;		//0-always send when timeout; 1-not  do if has sended in  the interval seconds.
	time_t last_trap_send_time;
	time_t last_heartbeat_send_time;
	char ac_ip[ACIPSIZ]; 
} HeartbeatInfo;

extern HeartbeatInfo gHeartbeatInfo[VRRP_TYPE_NUM][INSTANCE_NUM + 1];

int trap_instance_heartbeat_init(HeartbeatInfo *info, unsigned int local_id, unsigned int instance_id);

int trap_heartbeat_is_demanded(HeartbeatInfo *info);

void trap_heartbeat_update_last_time(HeartbeatInfo *info, int is_heartbeat_trap);

int trap_heartbeat_get_timeout(HeartbeatInfo *info);

void trap_enable_debug(void);
int  ac_trap_ip_param_by_shell( char *shell_name, char *content  );


#ifdef  __cplusplus
}
#endif

#endif		/* TRAP_UTIL_H */

