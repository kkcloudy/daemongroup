#ifndef _WID_WQOS_H
#define _WID_WQOS_H


typedef enum{
	qos_check_dot1p_state=0,
	qos_check_comma_state,
	qos_check_fail_state,
	qos_check_end_state,
	qos_check_success_state
}dot1p_list_state;

#define QOS_CWMIN_NUM	16
#define QOS_CWMAX_NUM	16
#define QOS_AIFS_NUM	16
#define QOS_TXOPLIMIT_NUM	8192
#define QOS_DOT1P_COMMA 	','	

typedef enum{
	check_wtpid=0,
	check_sub,
	check_sub2,	
	check_sub3,
	check_radioid,
	check_wlanid,
	check_point,
	check_fail,
	check_end,
	check_success,
	check_vrrip,	
	check_slotid
}radio_ifname_state;

#define PARSE_RADIO_IFNAME_SUB '-'
#define PARSE_RADIO_IFNAME_POINT '.'
#define DCLIWQOS_QOS_FLOW_NUM	4
#define DCLIWQOS_DOT1P_LIST_NUM	16


int wid_wqos_parse_char_ID(char* str,unsigned char* ID);

int wid_wqos_parse_short_ID(char* str,unsigned short* ID);


int parse_dot1p_list(char* ptr,unsigned char* count,unsigned char dot1plist[]);

unsigned char QosRemoveListRepId(unsigned char list[],unsigned char num);


int parse_qos_flow_parameter_type(char* str,unsigned int type);
int parse_qos_parameter_type(char* str,unsigned int type);
int parse_radio_ifname(char* ptr,int *wtpid,int *radioid,int *wlanid);

int dcli_wqos_method_parse_fist(char *DBUS_METHOD);
void* dcli_wqos_wireless_qos_show_config_info(
	int index,
	unsigned int* ret,
	unsigned int id,
	unsigned int qos_id,
	unsigned int vid,
//	AC_QOS **qos,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	);
void dcli_wqos_free_fun(char *DBUS_METHOD,DCLI_WQOS *WQOS);
#endif

