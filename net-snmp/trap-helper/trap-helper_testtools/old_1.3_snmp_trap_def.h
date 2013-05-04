/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* snmp_trap_def.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* 
*
*
*******************************************************************************/

#ifndef _SNMP_TRAP_DEF_H
#define _SNMP_TRAP_DEF_H

//#include "../subagent/mibs_public.h"

#define NETID_NAME_MAX_LENTH 21
#define SNMP_SCRIPT	"/usr/bin/snmptrap.sh"
#define GET_WTP_NETID_SHELL "/usr/bin/get_wtp_netid.sh"
#define GET_AC_NETID_SHELL "hostname"
#define PORT_TRAP_SHELL "sudo portal_server_monitor.sh"
#define PORT_TRAP_PATH "/usr/bin/portal_server_monitor.sh"
#define SNMPD_CONF_PATH "/opt/services/conf/snmpd_conf.conf"
#define MAX_TRAP_SEND_IP 10

#define WID_WTP_NUM	1024
#define BUF_LEN 128
#define PORT_NUM 24
#define PORT_BANDWITH 100            //单位为M,即每个端口最大流量为100M
#define GET_CPU_STATE  "/proc/stat"
#define GET_MEM_STATE  "/proc/meminfo"
#define AWK_GET_PORT_FLOW_INFO "ifconfig | awk '/RX bytes/{if($8!~/KiB/){print $3,$7}}'"
#define AWK_GET_DROP_PACKET_TOTAL_INFO "ip -s link | sed \"/[a-zA-Z]/d\" | awk 'BEGIN{i=true;total=0}{if(i==true){total+=$1;}i=!i;}END{print total}'"
#define AWK_GET_DROP_PACKET_DROP_INFO "ip -s link | sed \"/[a-zA-Z]/d\" | awk 'BEGIN{i=true;total=0}{if(i==true){total+=$4;}i=!i;}END{print total}'"


#define MAX_OID_LENTH 128

#define TRAP_SWITCH_PATH	"/var/run/apache2/trapswitch"
#ifndef TRAP_MIB_NODE_LOCATION
#define TRAP_MIB_NODE_LOCATION

#define TRAP_OID_START											".1.3.6.1.4.1"
#define ENTERPRISE_TRAP_OID										"31656"
#define PRODUCT_TRAP_OID										"6.1"
#define SYS_MIB_WTP_TRAP_OID_START								".4"
#define SYS_MIB_AC_TRAP_OID_START								".5"

#define WTP_NET_ELEMENT_CODE_OID								".1.2.5.1.3"
#define AC_NET_ELEMENT_CODE_OID									".2.2.3"


/////////WTP inner trap node location////////////////////
#define WTP_INNER_DOWN_TRAP 										".0.1"
#define WTP_INNER_SYSTEM_START_TRAP 								".0.2"
#define WTP_INNER_CHNNNEL_MODIFY_TRAP 								".0.3"
#define WTP_INNER_IP_CHANGE_ALARM 									".0.4"
#define WTP_INNER_FLASH_WRITE_FAIL_TRAP				 				".0.5"
#define WTP_INNER_COLD_START_TRAP 									".0.6"
#define WTP_INNER_AUTH_MODE_CHANGE_TRAP 							".0.7"
#define WTP_INNER_PRESHARE_KEY_CHANGE_TRAP		 					".0.8"
#define WTP_INNER_ELEC_REG_CIRCLE_TRAP 								".0.9"
#define WTP_INNER_AP_UPDATE_TRAP 									".0.10"
#define WTP_INNER_COVER_HOLE_TRAP 									".0.11"
#define WTP_INNER_WIRE_PORT_EXCEPTION_TRAP 							".0.12"

#define WTP_INNER_CPU_USAGE_TOOHIGH_TRAP 							".0.13"
#define WTP_INNER_CPU_USAGE_TOOHIGH_CLEAR_TRAP 						".0.14"
#define WTP_INNER_MEM_USAGE_TOOHIGH_TRAP 							".0.15"
#define WTP_INNER_MEM_USAGE_TOOHIGH_CLEAR_TRAP 						".0.16"
#define WTP_INNER_TEMPER_TOOHIGH_TRAP 								".0.17"
#define WTP_INNER_TEMPER_TOOHIGH_CLEAR_TRAP 						".0.18"
#define WTP_INNER_MT_WORKMODE_CHANGE_TRAP 							".0.19"
#define WTP_INNER_SWITCH_BETWEEN_DIFF_AC_TRAP 						".0.20"
#define WTP_INNER_SSID_KEY_CONFLICT_TRAP 							".0.21"
#define WTP_INNER_WTP_ONLINE_TRAP 									".0.22"
#define WTP_INNER_WTP_OFFLINE_TRAP 									".0.23"
#define WTP_INNER_COVER_HOLE_CLEAR_TRAP 							".0.24"


/////////WTP Application trap node location////////////////////
#define WTP_APP_CHANNEL_OBSTRUCTION_TRAP 							".1.1"
#define WTP_APP_AP_INTEFERENCE_DETECTED_TRAP 						".1.2"
#define WTP_APP_STA_INTEFERENCE_DETECTED_TRAP 						".1.3"
#define WTP_APP_DEVICE_INTEFERENCE_DETECTED_TRAP					".1.4"
#define WTP_APP_SUB_DATABASE_FULL_TRAP 								".1.5"
#define WTP_APP_DFS_FREECOUNT_BELOW_THRESHOLD_TRAP 					".1.6"
#define WTP_APP_FILE_TRANSFER_TRAP		 							".1.7"
#define WTP_APP_STATION_OFF_LINE_TRAP 								".1.8"
#define WTP_APP_LINK_VARIFY_TRAP 									".1.9"
#define WTP_APP_LINK_VARIFY_FAILED_TRAP 							".1.10"
#define WTP_APP_STATION_ON_LINE_TRAP 								".1.11"

#define WTP_APP_INTERF_NEARBY_CLEAR_TRAP 							".1.12"
#define WTP_APP_STA_INTERFERE_CLEAR_TRAP 							".1.13"
#define WTP_APP_OTHER_DEVICE_INTERF_DETECTED_TRAP 					".1.14"
#define WTP_APP_OTHER_DEVICE_INTERF_CLEAR_TRAP 						".1.15"
#define WTP_APP_MODULE_TROUBLE_TRAP 								".1.16"
#define WTP_APP_MODULE_TROUBLE_CLEAR_TRAP 							".1.17"
#define WTP_APP_RADIO_DOWN_TRAP 								 	".1.18"
#define WTP_APP_RADIO_DOWN_CLEAR_TRAP 								".1.19"
#define WTP_APP_TL_AUTH_FAILED_TRAP 								".1.20"
#define WTP_APP_STATION_AUTH_FAILED_TRAP 							".1.21"
#define WTP_APP_STA_ASSIOCIATION_FAILED_TRAP 						".1.22"
#define WTP_APP_USER_WITH_INVALID_CER_BREAK_NETWORK_TRAP 			".1.23"
#define WTP_APP_STATION_REPITIVE_ATTACK_TRAP 						".1.24"
#define WTP_APP_TAMPER_ATTACK_TRAP 									".1.25"
#define WTP_APP_LOW_SAFE_LEVEL_ATTACK_TRAP 							".1.26"
#define WTP_APP_ADDRESS_REDIRECTION_ATTACK_TRAP 					".1.27"
#define WTP_APP_MESH_AUTH_FAILED_TRAP 								".1.28"
#define WTP_APP_CHILD_EXCLUDED_PARENT_TRAP 							".1.29"
#define WTP_APP_PARENT_CHANGED_TRAP 								".1.30"
#define WTP_APP_CHILD_MOVED_TRAP 									".1.31"
#define WTP_APP_EXCESSIVE_PARENT_CHANGED_TRAP 						".1.32"
#define WTP_APP_MESH_ON_SET_SNR_TRAP 								".1.33"
#define WTP_APP_MESH_ABATE_SNR_TRAP 								".1.34"
#define WTP_APP_CONSOLE_LOGIN_TRAP 									".1.35"
#define WTP_APP_QUEUE_OVER_FLOW_TRAP 								".1.36"
#define WTP_APP_ADD_USR_FAIL_CLEAR_TRAP 							".1.37"
#define WTP_APP_CHANNEL_TOO_LOW_CLEAR_TRAP 							".1.38"
#define WTP_APP_AP_FIND_UNSAFE_ESSID								".1.39"
#define WTP_APP_AP_FIND_ATTACK										".1.40"







//////////////trap des//////////////////////////
#define WTP_TRAP_DES 			".2"
#define AC_TRAP_DES 			".2"

#define SEQUENCE_TRAP_DES		".3"
#define WARNTYPE_TRAP_DES		".4"
#define WARNLEVEL_TRAP_DES		".5"
#define TIME_TRAP_DES			".6"
#define STATUS_TRAP_DES			".7"
#define TITLE_TRAP_DES			".8"
#define CONTENT_TRAP_DES		".9"

#define EI_WLANID_TRAP_DES		".2.7.1.1"
#define EI_WTPID_TRAP_DES		".2.11.2.1.1"
#define EI_SN_TRAP_DES			".1.1.1.1.11"
#define EI_MAC_TRAP_DES			".1.2.5.1.9"
#define EI_ESSID_TRAP_DES		".1.5.4.1.4"
#define EI_STA_MAC_TRAP_DES		".1.8.1.1.1"
#define EI_CHANNEL_MAC_TRAP_DES				".1.7.3.1.1"
#define EI_WTP_CPU_USAGE_TRAP_DES			".1.1.2.1.5"
#define EI_WTP_CPU_THRESHOLD_TRAP_DES		".1.1.2.1.6"
#define EI_WTP_MEM_USAGE_TRAP_DES			".1.1.2.1.11"
#define EI_WTP_MEM_THRESHOLD_TRAP_DES		".1.1.2.1.12"
#define EI_WTP_TEMP_USAGE_TRAP_DES			".1.1.2.1.17"
#define EI_AC_IP_ADDR_TRAP_DES				".2.2.1"
#define EI_AC_MAC_TRAP_DES				".2.2.5"

#define EI_AC_CPU_USAGE_TRAP_DES			".2.1.2.13"
#define EI_AC_MEM_USAGE_TRAP_DES			".2.1.2.8"
#define EI_AC_TEMP_USAGE_TRAP_DES			".2.1.2.20"
#define EI_AC_BAND_USAGE_TRAP_DES			".2.3.1.17"
#define EI_AC_DROP_USAGE_TRAP_DES			".2.3.1.18"
#define EI_AC_USER_ONLINE_USAGE_TRAP_DES	".2.3.1.19"
#define EI_AC_RADIUS_REQUEST_RATE_TRAP_DES	".2.3.1.20"
#define EI_AC_DHCP_USAGE_TRAP_DES			".2.6.3.4"
#define EI_AC_DHCP_MAX_USAGE_TRAP_DES		".2.6.3.5"



////////AC INNER TRAP///////////////////////////////////////
#define AC_INNER_SYS_REBOOT_TRAP 									".0.1"
#define AC_INNER_AP_TIME_SYNC_FAIL_TRAP 							".0.2"
#define AC_INNER_CHANGE_IP_TRAP 									".0.3"
#define AC_INNER_TURN_TO_BACK_DEVICE_TRAP 							".0.4"

#define AC_INNER_CONFIGURE_ERROR_TRAP 								".0.5"
#define AC_INNER_SYS_COLD_START_TRAP 								".0.6"
#define AC_INNER_HEART_BEAT_TRAP 									".0.7"


////////AC APP TRAP////////////////////////////////////////
#define AC_APP_DISCOVER_DANGER_AP_TRAP 								".1.1"
#define AC_APP_RADIUS_AUTH_SERVER_NOT_REACH_TRAP 					".1.2"
#define AC_APP_RADIUS_ACCOUNT_SERVER_NOT_REACH_TRAP 				".1.3"
#define AC_APP_PORTAL_SERVER_NOT_REACH_TRAP 						".1.4"
#define AC_APP_AP_LOST_NET_TRAP 									".1.5"
#define AC_APP_CPU_ULTILIZATION_OVER_THRESHOLD_TRAP 				".1.6"
#define AC_APP_MEM_ULTILIZATION_OVER_THRESHOLD_TRAP 				".1.7"
#define AC_APP_BAND_ULTILIZATION_OVER_THRESHOLD_TRAP 				".1.8"
#define AC_APP_LOSTPACKET_RATE_OVER_THRESHOLD_TRAP 					".1.9"
#define AC_APP_MAX_USERNUM_OVER_THRESHOLD_TRAP 						".1.10"
#define AC_APP_DISCNNECT_NUM_OVER_THRESHOLD_TRAP 					".1.11"
#define AC_APP_DROP_RATE_OVER_THRESHOLD_TRAP 						".1.12"
#define AC_APP_OFFLINE_SUC_RATE_BELOW_THRESHOLD_TRAP 				".1.13"
#define AC_APP_AUTH_SUC_RATE_BELOW_THRESHOLD_TRAP 					".1.14"
#define AC_APP_AVE_IPPOOL_OVER_THRESHOLD_TRAP 						".1.15"
#define AC_APP_MAX_IPPOOL_OVER_THRESHOLD_TRAP 						".1.16"
#define AC_APP_DHCP_SUC_RATE_OVER_THRESHOLD_TRAP 					".1.17"

#define AC_APP_POWER_OFF_TRAP 										".1.18"
#define AC_APP_POWER_OFF_CLEAR_TRAP 								".1.19"
#define AC_APP_CPU_USAGE_TOO_HIGH_CLEAR_TRAP 						".1.20"
#define AC_APP_MEM_USAGE_TOO_HIGH_CLEAR_TRAP 						".1.21"
#define AC_APP_TEMPER_TOO_HIGH_TRAP 								".1.22"
#define AC_APP_TEMPER_TOO_HIGH_CLEAR_TRAP 							".1.23"
#define AC_APP_DHCP_ADDRESS_EXHAUSTED_TRAP 							".1.24"
#define AC_APP_DHCP_ADDRESS_EXHAUSTED_CLEAR_TRAP 					".1.25"
#define AC_APP_RADIUS_AUTH_SERVER_NOT_REACH_CLEAR_TRAP 				".1.26"
#define AC_APP_RADIUS_ACCOUNT_SERVER_NOT_REACH_CLEAR_TRAP 			".1.27"
#define AC_APP_PORTER_SERVER_NOT_REACH_CLEAR_TRAP 					".1.28"
#define AC_APP_FIND_SYN_ATTACK 										".1.29"


#define AC_APP_HEAT_TIME_PACKAGE_TRAP 								".1.50"


#define U_INT8  unsigned char  
#define U_INT16  unsigned short
#define U_INT32  unsigned int  
#define U_INT64  unsigned long  


typedef struct{
	U_INT32 send_trap_switch; /* send trap switch for maybe sending sample trap in statistic time*/
	U_INT32 sample_switch;
	U_INT8  every_trap_swich[128]; /*control every trap's switch*/
}STTrapSwitch;

typedef struct{
	U_INT32 vrrp_last_state;
	U_INT32 last_trap_flag;
	U_INT32 last_cpu_trap_flag;
	U_INT32 last_mem_trap_flag;
	U_INT32 last_temp_trap_flag;
	U_INT32 last_porter_server_flag;
}TrapLastState;

typedef struct{
	U_INT32 tatal_cpu_rate ;
	U_INT32 tatal_mem_rate  ;
	U_INT32 tatal_temp_rate  ;
	U_INT32 tatal_band_rate  ;
	U_INT32 tatal_drop_rate  ;
	U_INT32 tatal_user_rate  ;
	U_INT32 tatal_radius_rate  ;
	U_INT32 tatal_ippool_rate  ;

	U_INT32 max_cpu_rate  ;
	U_INT32 max_mem_rate  ;
	U_INT32 max_temp_rate  ;
	U_INT32 max_band_rate  ;
	U_INT32 max_drop_rate  ;
	U_INT32 max_user_rate  ;
	U_INT32 min_radius_rate  ;
	U_INT32 max_ippool_rate  ;

	U_INT32 cpu_rate_num  ;
	U_INT32 mem_rate_num  ;
	U_INT32 temp_rate_num  ;
	U_INT32 band_rate_num  ;
	U_INT32 drop_rate_num  ;
	U_INT32 user_rate_num  ;
	U_INT32 radius_rate_num  ;
	U_INT32 ippool_rate_num  ;
	
}TrapStatistic;


typedef struct{
	U_INT64 start_time ;
	U_INT64 sample_start_time ;
	U_INT64 end_time ;
	U_INT32 SLEEP_TIME;
	U_INT32 SAMPLE_TIME;
	U_INT32 HEART_TIME;
	U_INT32 heartPkg_start_time;
}STTrapTime;


typedef struct{
	char * start_oid;
	char * trap_seqeuence ;
	char * trap_warn_type ;
	char * trap_warn_level ;
	char * trap_time ;
	char * trap_status ;
	char * trap_title ;
	char * trap_content ;
}STTrapParamOID;



typedef struct{
	int CPU_RATE_TRAP_THRESHOLD_VALUE  ;
	int MEM_RATE_TRAP_THRESHOLD_VALUE  ;
	int TEMP_AVE_TRAP_THRESHOLD_VALUE  ;
	int BANDWITH_RATE_TRAP_THRESHOLD_VALUE  ;
	int DROP_RATE_TRAP_THRESHOLD_VALUE  ;
	int MAX_ONLINE_USER_TRAP_THRESHOLD_VALUE  ;
	int RADIUS_REQUEST_SUC_RATE_THRESHOLD_VALUE  ;
	int IP_POOL_RATE_THRESHOLD_VALUE  ;
	int IP_POOL_MAX_THRESHOLD_VALUE  ;
}STTrapThreshold;


typedef struct{
	U_INT32 reload_conf_xml;
	U_INT32 syn_attack_detected;
	U_INT32 TRAP_SEND_FLAG;
	U_INT8 keep_go ;
	U_INT32 trap_signal_number[150];  //0-31为网通和老移动，32-43为电信，44-92为广州移动新增trap,93为新增porter server
	U_INT32 test_num_failed ;
	U_INT32 test_num_normal ;
	char cmd_out[50];
	U_INT32 main_pid;

	STTrapSwitch	stSwitch;
	TrapLastState 	stLastState;
	TrapStatistic 	stStatistic;
	STTrapTime 		stTrapTime;
	STTrapParamOID 	stbindOID;
	STTrapThreshold stThreshold;
	
}STGTrapVar;


#endif
/*将trap des只读取一次相关定义*/
typedef struct trap_dest{
	int version;// 1   2c  3 
	char des_ip[32];
	U_INT16 des_port;
	char param[128];// param  for  1/2c is community  for  3 is  user\md5......
	struct trap_dest *next;
}STTrapDest;

/******************************************
关于param_format的说明：（重要）
因为字符串中可能有空格，这样的变量传递到shell中的时候，会应为空格的存在，
将一个字符串变量解析成多个变量，导致命令不能正常执行。所以在format中要输出字符串的时候，需要添加"号
下面是一个示例：
snmp_send_trap( "IF-MIB::linkDown", "SNMPv2-MIB::sysLocation.0 s  \"\\\"%s\\\"\" ", "just here!" );
传统的在printf中输出字符串用%s就可以了，这里需要写成   \"\\\"%s\\\"\"  在能保证命令正确的传递到shell中。
为了统一操作，这里有将这个进行了一次定义，在写format的时候，使用宏定义就可以了。
如下：
snmp_send_trap( "IF-MIB::linkDown", "SNMPv2-MIB::sysLocation.0 s  "STRING_FORMAT" ", "just here!" );
对于其它数据类型的format，和printf一样就可以了。


不同类型的oid对应foramt时填写的字符
IpAddress		:   a
BITS			:	b
Counter32		:	c
?				:	d
INTEGER			:	i
?				:	n
OBJECT IDENTIFIER :	o
OCTET STRING	:	s
Timeticks		:	t
Unsigned32		:	u
?				:	x
Gauge			:	???
*******************************************/


#define STRING_FORMAT "\"\\\"%s\\\"\""
#define TIME_LEEP 1
#define STATISTIC_LEASE 2
#define PORT_OUPUT_LENTH	256
#define TRAP_PID_FILE "/var/run/trap.pid"

#define TRAP_HELPER_IDENT "trap-helper"

#if 0
#define snmp_send_trap(trap_oid,param_format,...) \
			{\
				char cmd[2046]="";\
				snprintf( cmd,sizeof(cmd)-1, "%s %s "param_format, SNMP_SCRIPT,trap_oid, __VA_ARGS__ );\
				printf("cmd=%s",cmd);\
				system( cmd );\
				start_time = time((time_t*)NULL);\
			}
#define snmp_send_trap_without_param( trap_oid )\
			{\
				char cmd[2046]="";\
				snprintf( cmd,sizeof(cmd)-1, "%s %s", SNMP_SCRIPT,trap_oid );\
				system( cmd );\
			}
#else
#define SNMP_TRAP	"sudo /usr/bin/snmptrap"
#define build_v1trap_cmd( cmd_buff,trap_des, trap_oid, param_format, ... )\
			{\
				snprintf( cmd_buff, sizeof(cmd_buff)-1, SNMP_TRAP" -v 1 %s %s:%hd %s \"\" 6 17 \"\""param_format,\
						trap_des->param, trap_des->des_ip, trap_des->des_port, trap_oid, __VA_ARGS__ );\
			}

#define build_v2trap_cmd( cmd_buff,trap_des, trap_oid, param_format, ... )\
			{\
				snprintf( cmd_buff, sizeof(cmd_buff)-1, SNMP_TRAP" -v 2c %s %s:%hd \"\" %s "param_format,\
						trap_des->param, trap_des->des_ip, trap_des->des_port, trap_oid, __VA_ARGS__ );\
			}

#define build_v3trap_cmd( cmd_buff,trap_des, trap_oid, param_format, ... )\
			{\
				snprintf( cmd_buff, sizeof(cmd_buff)-1, SNMP_TRAP" -v 3 %s %s:%hd %ul %s "param_format,\
						trap_des->param, trap_des->des_ip, trap_des->des_port, time(0)*100,trap_oid, __VA_ARGS__ );\
			}

#define snmp_send_trap(trap_oid,param_format,...) \
			{\
				char cmd[4096]="";\
				STTrapDest *temp;\
				for( temp=list_head; temp; temp=temp->next )\
				{\
					switch(temp->version)\
					{\
						case 1:\
							build_v1trap_cmd(cmd,temp,trap_oid, param_format,__VA_ARGS__);\
							break;\
						case 2:\
							build_v2trap_cmd(cmd,temp,trap_oid, param_format,__VA_ARGS__);\
							break;\
						case 3:\
							build_v3trap_cmd(cmd,temp,trap_oid, param_format,__VA_ARGS__);\
							break;\
						default:\
							continue;\
							break;\
					}\
					system( cmd );\
					memset( cmd, 0, sizeof(cmd));\
					stgTrapVar.stTrapTime.heartPkg_start_time = time((time_t*)NULL);\					
				}\
			}

#endif
//////added by tangsiqi/////////////			
#define wtp_trap_netid_param_by_shell( shell_name, content , wtpid )\
				FILE * fp = NULL;\
				char buf[NETID_NAME_MAX_LENTH+1] = "";\
				char syscom[100] = "";\
				sprintf (syscom, "%s %d",shell_name,wtpid);\
				if ( (fp = popen (syscom, "r")) == NULL )\
				{\
					strcpy (content, "");\
				}\
				else if (fp != NULL)\
				{\
					fgets (buf,NETID_NAME_MAX_LENTH,fp);\
					strncpy (content, buf, ( strcmp(buf, "") )?(strlen(buf)-1) : 0);\
					pclose (fp);\
				}\
				

			
#define ac_trap_netid_param_by_shell( shell_name, content  )\
			{\
				FILE * fp = NULL;\
				char buf[NETID_NAME_MAX_LENTH];\
				if( NULL != (fp = popen (shell_name, "r")) )\
				{\
					fgets(buf,NETID_NAME_MAX_LENTH,fp);\
					strncpy (content, buf, ( strcmp(buf, "") )?(strlen(buf)-1) : 0);\
					pclose(fp);\
				}\
			}\
			
#endif

