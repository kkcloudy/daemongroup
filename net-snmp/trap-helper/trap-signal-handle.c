/* trap-signal-handle.c */

#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include <syslog.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include "trap-util.h"
#include "trap-def.h"
#include "nm_list.h"
#include "hashtable.h"
#include "trap-list.h"
#include "trap-descr.h"
#include "trap-data.h"
#include "ws_snmpd_engine.h"
#include "trap-resend.h"
#include "trap-signal.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "board/board_define.h"
#include "trap-receiver.h"
#include "trap-instance.h"
#include "trap-hash.h"
#include "trap-signal-handle.h"
#include "dbus/npd/npd_dbus_def.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ac_sample_dbus.h"
#include "ac_sample_def.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef.h"
#include "nm/app/eag/eag_trap.h"
#include "nm/app/manage/ac_manage_def.h"
#include "ws_dbus_list_interface.h"
#include "sem/product.h"
#include "dbus/sem/sem_dbus_def.h"
#include <mcheck.h>

TrapList gSignalList = {0};

#define cmd_test	1
#undef	cmd_test	
#ifdef	cmd_test	
#define	cmd_test_out(cmd) 	fprintf(stderr, "test cmd=%s, line=%d\n", cmd, __LINE__);
#else
#define cmd_test_out(cmd)	(void *)0;
#endif

#if 1
#define INCREASE_TIMES(tDescr) 	\
	do { \
		(tDescr)->frequency++; \
	} while (0)
#else
#define INCREASE_TIMES(tDescr) \
	do { \
	} while (0) 
#endif

/* mac_str = 18, mac = 6 */
char * get_ap_mac_str(char *mac_str, int str_len, unsigned char *mac)
{
	memset(mac_str, 0, str_len);
	
	snprintf(mac_str, str_len, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	return mac_str;
}

/* mac_oid = 187(17 * 11), mac_str = 18 */
char * get_ap_mac_oid(char *mac_oid, int oid_len, char *mac_str)
{
	memset(mac_oid, 0, oid_len);
	
	snprintf(mac_oid, oid_len, ".17.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",
			mac_str[0], mac_str[1], mac_str[2], mac_str[3], mac_str[4], mac_str[5], mac_str[6], mac_str[7], mac_str[8], 
			mac_str[9], mac_str[10], mac_str[11], mac_str[12], mac_str[13], mac_str[14], mac_str[15], mac_str[16]);
	
	return mac_oid;
}

/* mac_str = 18, mac = 6 */
char * get_ac_mac_str(char *mac_str, int str_len, unsigned char *mac)
{
	memset(mac_str, 0, str_len);
	
	snprintf(mac_str, str_len, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	return mac_str;
}

/* mac_oid = 187(17 * 11), mac_str = 18 */
char * get_ac_mac_oid(char *mac_oid, int oid_len, char *mac_str)
{
	memset(mac_oid, 0, oid_len);
	
	snprintf(mac_oid, oid_len, ".17.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",
			mac_str[0], mac_str[1], mac_str[2], mac_str[3], mac_str[4], mac_str[5], mac_str[6], mac_str[7], mac_str[8], 
			mac_str[9], mac_str[10], mac_str[11], mac_str[12], mac_str[13], mac_str[14], mac_str[15], mac_str[16]);
	
	return mac_oid;
}

char *get_ap_netid(char *netid, int len, int wtpindex)
{
	int ret=0;
	DCLI_WTP_API_GROUP_THREE *WTPINFO;
	
	memset(netid, 0, len);
	#if 1
	strncpy(netid, "defaultcode", len);
	return netid;
	#endif
	dbus_parameter parameter;
	memset(&parameter, 0, sizeof(parameter));
	parameter.local_id = 1;
	parameter.instance_id = 0;
	
	void *connection = NULL;
	get_slot_dbus_connection(LOCAL_SLOT_NUM, &connection, SNMPD_INSTANCE_MASTER_V3);
	
	ret=show_wtp_netid(parameter, connection,wtpindex,&WTPINFO);
	if(ret==1)
	{
		strcpy(netid, WTPINFO->netid);
	}else
	{
		strncpy(netid, "defaultcode", len);
	}
	
	if(ret !=-3)  //add mayf if (ret==1)
	{	  
		free_show_wtp_netid(WTPINFO);
	}

	/*if ( 1 !=(ret = show_wtp_netid(wtpindex, netid, NULL))) //warning (netid)  makes integer from pointer without a cast
	{
		strncpy(netid, "defaultcode", len);
		trap_syslog(LOG_DEBUG, "show_wtp_netid wtpindex %d error! netid=%s return=%d\n", wtpindex, netid, ret);
	}*/

	trap_syslog(LOG_DEBUG, "show_wtp_netid wtpindex %d netid=%s return=%d\n", wtpindex, netid, ret);

	return netid;
}

static int manage_get_slot_id_by_ifname(const char *ifname) 
{
	int slotnum = -1;
	int i = 0;
	int count = 0;
	char tmp[32];

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, ifname, strlen(ifname));

	/* eth : cpu */
	if (0 == strncmp(ifname, "eth", 3)) {
		sscanf(ifname, "eth%d-%*d", &slotnum);
	}

	/* ve */
	else if (0 == strncmp(ifname, "ve", 2)) {
		//sscanf(ifname, "ve%d.%*d", &slotnum);
		sscanf(ifname, "ve%2d", &slotnum);
	} 

	/* radio */
	else if (0 == strncmp(ifname, "r", 1)) {
		for (i = 0; i < strlen(ifname); i++) {
			/*use '-' to make sure this radio is local board or remote board */
			if (tmp[i] == '-') {
				count++;
			}			
		}
		
		if (2 == count) {	/*local board*/
			if(VALID_DBM_FLAG == get_dbm_effective_flag())
			{
				slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
			}
		} else if(3 == count) {	/*remote board*/
			sscanf(ifname, "r%d-%*d-%*d-%d.%*d", &slotnum);
		}
	}

	/* wlan */
	else if (0 == strncmp(ifname, "wlan", 4)) {
		for (i = 0; i < strlen(ifname); i++) {
			if(tmp[i] == '-') {
				count++;
			}
		}
		
		if (1 == count) {	/*local board*/
			if(VALID_DBM_FLAG == get_dbm_effective_flag())
			{
				slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
			}
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
		}
	}

	/* ebr */
	else if (0 == strncmp(ifname, "ebr", 3)) {
		for (i = 0; i < strlen(ifname); i++) {
			if (tmp[i] == '-') {
				count++;
			}
		}
		if (1 == count) {	/*local board*/
			if(VALID_DBM_FLAG == get_dbm_effective_flag())
			{
				slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
			}
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
		}
	}

	/* slot all */
	else if (0 == strncmp(ifname, "slot", 4)) {
        sscanf(ifname, "slot%d", &slotnum);
	}
	
	return slotnum;
}


#if 0
ap func
#endif
int wtp_sys_start_func(DBusMessage * message)
{
//WID_DBUS_TRAP_WID_WTP_AP_REBOOT
	cmd_test_out(wtpSysStartTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
    unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32, &local_id,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d,instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpSysStartTrap);
	if (NULL== tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s", EI_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", EI_SN_TRAP_DES, mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_down_func(DBusMessage * message)
{
//WID_DBUS_TRAP_WID_WTP_AP_DOWN
	cmd_test_out(wtpDownTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE,	&wtpmac[2],
								DBUS_TYPE_BYTE,	&wtpmac[3],
								DBUS_TYPE_BYTE,	&wtpmac[4],
								DBUS_TYPE_BYTE,	&wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
                                DBUS_TYPE_UINT32, &local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id,instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
			return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;

	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpDownTrap);
	if (NULL== tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id,instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s", EI_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", EI_SN_TRAP_DES, mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_ip_change_alarm_func(DBusMessage * message)
{
//WID_DBUS_TRAP_WID_WTP_IP_CHANGE_ALARM
	cmd_test_out(wtpIPChangeAlarmTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE,	&wtpmac[0],
								DBUS_TYPE_BYTE,	&wtpmac[1],
								DBUS_TYPE_BYTE,	&wtpmac[2],
								DBUS_TYPE_BYTE,	&wtpmac[3],
								DBUS_TYPE_BYTE,	&wtpmac[4],
								DBUS_TYPE_BYTE,	&wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
                                DBUS_TYPE_UINT32,&local_id, 
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpIPChangeAlarmTrap);
	if (NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s", EI_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", EI_SN_TRAP_DES, mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_modified_func(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WTP_CHANNEL_CHANGE
	cmd_test_out(wtpChannelModifiedTrap);

	unsigned char chan_past;
	unsigned char chan_curr;
	int radioID;
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,	
								DBUS_TYPE_UINT32, &radioID,
								DBUS_TYPE_BYTE,   &chan_past,								
								DBUS_TYPE_BYTE,   &chan_curr,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
                                DBUS_TYPE_UINT32,&local_id, 
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, chan_past=%d, chan_curr=%d,netid=%s, local_id =%d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, chan_past, chan_curr,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpChannelModifiedTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s", 	EI_MAC_TRAP_DES, 			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", 	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", 	WTP_NET_ELEMENT_CODE_OID, 	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chan_curr);
	
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;		
}//LV3

int wtp_file_transfer_func(DBusMessage * message)
{
//WID_DBUS_TRAP_WID_WTP_TRANFER_FILE
	cmd_test_out(wtpFileTransferTrap);
	
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &wtpindex,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id, 
                                    DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s,local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid,local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpFileTransferTrap);
	if (NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s",   EI_MAC_TRAP_DES, 		 mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", EI_SN_TRAP_DES, 			 mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_electrify_register_circle_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_ELECTRIFY_REGISTER_CIRCLE
	cmd_test_out(wtpElectrifyRegisterCircleTrap);
	
	DBusError error;
	unsigned int wtpindex;
	unsigned int registercircle = 0;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
										DBUS_TYPE_UINT32, &wtpindex,
										DBUS_TYPE_UINT32, &registercircle,
										DBUS_TYPE_STRING, &wtpsn,
										DBUS_TYPE_BYTE, &wtpmac[0],
										DBUS_TYPE_BYTE, &wtpmac[1],
										DBUS_TYPE_BYTE, &wtpmac[2],
										DBUS_TYPE_BYTE, &wtpmac[3],
										DBUS_TYPE_BYTE, &wtpmac[4],
										DBUS_TYPE_BYTE, &wtpmac[5],
										DBUS_TYPE_STRING,&netid,
										DBUS_TYPE_UINT32,&instance_id,
                                        DBUS_TYPE_UINT32,&local_id,
										DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, registercircle=%d,netid=%s, local_id = %d, instance_id=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, registercircle, netid, local_id,instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpElectrifyRegisterCircleTrap);
	if (NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s", 	EI_MAC_TRAP_DES, 			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", 	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", 	WTP_NET_ELEMENT_CODE_OID, 	mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
int wtp_ap_update_func(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WTP_ENTER_IMAGEDATA_STATE
	cmd_test_out(wtpAPUpdateTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &wtpindex,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id, 
                                    DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpAPUpdateTrap);
	if ( NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s", 	EI_MAC_TRAP_DES, 			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", 	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s", 	WTP_NET_ELEMENT_CODE_OID, 	mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;	
}

int wtp_cold_start_func(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WTP_CODE_START
	cmd_test_out(wtpColdStartTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
                                DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpColdStartTrap);
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_auth_mode_change_func(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WLAN_ENCRYPTION_TYPE_CHANGE
	cmd_test_out(wtpAuthModeChangeTrap);

	unsigned int wtpindex;
	DBusError err;	
	DBusMessageIter  iter;	
	dbus_error_init(&err);
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	
	dbus_message_iter_init(message, &iter);	
	dbus_message_iter_get_basic(&iter, &wtpindex);
	dbus_message_iter_next(&iter);
	dbus_message_iter_next(&iter);
	dbus_message_iter_next(&iter);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &instance_id);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &local_id);
	
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, local_id = %d instance_id=%d\n", dbus_message_get_member(message), wtpindex, local_id, instance_id);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
		
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpAuthModeChangeTrap);
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	trap_data_append_param_str(tData, "%s s %d", EI_WLANID_TRAP_DES, wtpindex);
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
		
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
		
	#if 0
	unsigned int wlanid;
		char securityid;//-1 means no security policy binding
		unsigned int encryptiontype;//0.1.2.3means NONE/WEP/AES/TKIP
		int num = 0;
		int i = 0;
		unsigned int wtp_index = 0;
		char *wtpsn;
		DBusError err;
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		struct wid_trap_wtp_info wtp_info[WID_WTP_NUM];
		char netid_content[NETID_NAME_MAX_LENTH];
		memset (netid_content, 0, NETID_NAME_MAX_LENTH);
		GET_TIME_COMMAND
		
		dbus_error_init(&err);
	
		dbus_message_iter_init(message,&iter);
		
		dbus_message_iter_get_basic(&iter,&wlanid);
	
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_get_basic(&iter,&securityid);
	
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_get_basic(&iter,&encryptiontype);
	
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_get_basic(&iter,&num);
	
		
		if(num> 0 )
		{
				
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
	
	
			for (i = 0; i < num; i++) {
				wtp_info[i].wtpid = 0;
				memset(wtp_info[i].wtpsn,0,25);
				
				DBusMessageIter iter_struct;
						
				dbus_message_iter_recurse(&iter_array,&iter_struct);
					
				dbus_message_iter_get_basic(&iter_struct,&wtp_index);
					
				dbus_message_iter_next(&iter_struct);
						
				dbus_message_iter_get_basic(&iter_struct,&wtpsn);
					
				dbus_message_iter_next(&iter_array);
	
				//store info
				wtp_info[i].wtpid = wtp_index;
				
				memcpy(wtp_info[i].wtpsn,wtpsn,20);
	
				//printf("wtpid %d wtpsn %s\n",wtp_info[i].wtpid,wtp_info[i].wtpsn);
			}
		
		}
	
		Get_MIB_DEF_NODE_OID(0,WTP_INNER_AUTH_MODE_CHANGE_TRAP);
		Get_MIB_DES_NODE_OID(0,WTP_TRAP_DES);
		
		trap_signal_number[12]++;
		//wtp_trap_netid_param_by_shell (GET_WTP_NETID_SHELL, netid_content , wtp_info[0].wtpid);
	
		if( debug_trap )
			syslog(LOG_NOTICE, "wid_wlan_encryption_type_change wlanid:%d securityid:%d encryptiontype:%d\n",wlanid,securityid,encryptiontype);
		
		//snprintf(TrapDes,sizeof(TrapDes),"serial_number=%d--NE_sysname=%s--EI_WLANID=%d--warning_type=communication--warning_level=major--time=%s--status=1--title=WLAN_ENCRYPTION_TYPE_CHANGE--content=\"\\\"The WLAN has changed it's encryption type\\\"\"",trap_signal_number[12],netid_content,wlanid,str);
	
		//snmp_send_trap( node_oid, \
									"%s s %s", des_oid, TrapDes );
	
		if (every_trap_swich[20] && extra_switch_by_vrrp)
		snmp_send_trap( node_oid, " %s%s s %d	\
									%s s %d 	\
									%s s %s 	\
									%s s %s 	\
									%s s %s 	\
									%s s %d 	\
									%s s %s 	\
									%s s %s ",
									start_oid, EI_WLANID_TRAP_DES, wlanid, \
									trap_seqeuence, trap_signal_number[12], \
									trap_warn_type, "communication", \
									trap_warn_level, "major", \
									trap_time, str, \
									trap_status, 1, \
									trap_title, "WLAN_ENCRYPTION_TYPE_CHANGE",\
									trap_content , "WLAN_ENCRYPTION_TYPE_CHANGE"
									);
	 
	
	
		free(node_oid);
		free(des_oid);
		return 0;

	#endif
}

int wtp_preshared_key_change_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WLAN_PRESHARED_KEY_CHANGE
	cmd_test_out(wtpPreSharedKeyChangeTrap);

	unsigned int wtpindex;
	DBusError err;	
	DBusMessageIter  iter;	
	dbus_error_init(&err);
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_message_iter_init(message, &iter);	
	dbus_message_iter_get_basic(&iter, &wtpindex);
	dbus_message_iter_next(&iter);
	dbus_message_iter_next(&iter);
	dbus_message_iter_next(&iter);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &instance_id);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &local_id);
	
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, local_id = %d, instance_id=%d\n", dbus_message_get_member(message), wtpindex, local_id, instance_id);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
		
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpPreSharedKeyChangeTrap);
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	trap_data_append_param_str(tData, "%s s %d", EI_WLANID_TRAP_DES, wtpindex);
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
		
	#if 0
		 原来接收数据的功能，很多数据没有用到，所以注释掉
	unsigned int wlanid;
		char securityid;//-1 means no security policy binding
		char* wlankey;
		int num = 0;
		int i = 0;
		unsigned int wtp_index = 0;
		char *wtpsn;
		DBusError err;	
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		struct wid_trap_wtp_info wtp_info[WID_WTP_NUM];
		char netid_content[NETID_NAME_MAX_LENTH];
		
		memset (netid_content, 0, NETID_NAME_MAX_LENTH);
		GET_TIME_COMMAND
		
		dbus_error_init(&err);
	
		dbus_message_iter_init(message,&iter);
		
		dbus_message_iter_get_basic(&iter,&wlanid);
	
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_get_basic(&iter,&securityid);
	
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_get_basic(&iter,&wlankey);
	
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_get_basic(&iter,&num);
	
		
		if(num> 0 )
		{
				
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
	
	
			for (i = 0; i < num; i++) {
				wtp_info[i].wtpid = 0;
				memset(wtp_info[i].wtpsn,0,25);
				
				DBusMessageIter iter_struct;
						
				dbus_message_iter_recurse(&iter_array,&iter_struct);
					
				dbus_message_iter_get_basic(&iter_struct,&wtp_index);
					
				dbus_message_iter_next(&iter_struct);
						
				dbus_message_iter_get_basic(&iter_struct,&wtpsn);
					
				dbus_message_iter_next(&iter_array);
	
				//store info
				wtp_info[i].wtpid = wtp_index;
				
				memcpy(wtp_info[i].wtpsn,wtpsn,20);
	
				//printf("wtpid %d wtpsn %s\n",wtp_info[i].wtpid,wtp_info[i].wtpsn);
			}
		
		}
	
	
		Get_MIB_DEF_NODE_OID(0,WTP_INNER_PRESHARE_KEY_CHANGE_TRAP);
		Get_MIB_DES_NODE_OID(0,WTP_TRAP_DES);
		
		trap_signal_number[13]++;
		//wtp_trap_netid_param_by_shell (GET_WTP_NETID_SHELL, netid_content , wtp_info[0].wtpid);
	
		if( debug_trap )
		{
			syslog(LOG_NOTICE, "wid_wlan_preshared_key_change wlanid:%d securityid:%d key:%s\n",wlanid,securityid,wlankey);
		}
		//snprintf(TrapDes,sizeof(TrapDes),"serial_number=%d--NE_sysname=%s--EI_WLANID=%d--warning_type=communication--warning_level=major--time=%s--status=1--title=WLAN_PRESHARED_KEY_CHANGE--content=\"\\\"The WLAN has changed it's preshared key\\\"\"",trap_signal_number[13],netid_content,wlanid,str);
	
		//snmp_send_trap( node_oid, \
									"%s s %s", des_oid, TrapDes );
		
	
		if (every_trap_swich[17] && extra_switch_by_vrrp)
		snmp_send_trap( node_oid, " %s%s s %d "\
								   "%s s %d "\
									"%s s %s "\
									"%s s %s "\
									"%s s %s "\
									"%s s %d "\
									"%s s %s "\
									"%s s %s",
									start_oid, EI_WLANID_TRAP_DES, wlanid, \
									trap_seqeuence, trap_signal_number[13], \
									trap_warn_type, "communication", \
									trap_warn_level, "major", \
									trap_time, str, \
									trap_status, 1, \
									trap_title, "WLAN_PRESHARED_KEY_CHANGE",\
									trap_content , "WLAN_PRESHARED_KEY_CHANGE"
									);
	 
	
	
	
		free(node_oid);
		free(des_oid);

	#endif
}

int wtp_flash_write_fail_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_AP_FLASH_WRITE_FAIL
	cmd_test_out(wtpFlashWriteFailTrap);
	
	DBusError error;
	unsigned int wtpindex;
	unsigned char flag = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							//DBUS_TYPE_UINT32, &flag,//failure--0,successfull -- 1
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id, 
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpFlashWriteFailTrap);	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_divorce_network_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_DIVORCE_NETWORK
	cmd_test_out(acAPLostNetTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char flag = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acAPLostNetTrap);	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_actimesynchrofailure_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE
	cmd_test_out(acAPACTimeSynchroFailureTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char flag;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_BYTE,	&flag,
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)) )
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, flag=%d, netid=%s, local_id = %d, instance_id=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, flag, netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if ( 1==flag )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acAPACTimeSynchroFailureTrapClear);
	}else if ( 0==flag )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acAPACTimeSynchroFailureTrap);	
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id,instance_id);

	tData = trap_data_new_from_descr(tDescr);
		
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
		
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
		
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %s",	AC_NET_ELEMENT_CODE_OID,	mac_oid, gSysInfo.hostname );

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
			
	//trap_data_destroy(tData);
		
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int th_sta_leave_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_STA_LEAVE
	cmd_test_out(wtpStationOffLineTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned int	g_bss=0;
	unsigned char	wlanid=0;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id  = 0;
	unsigned int instance_id = 0;
	unsigned char rssi = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_UINT32, &g_bss,
								DBUS_TYPE_BYTE,   &wlanid,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_BYTE,&rssi,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, g_bss=%d, wlanid=%d, netid=%s, local_id = %d, instance_id=%d,rssi=%d "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, g_bss, wlanid,netid, local_id, instance_id,rssi,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpStationOffLineTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
		
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
			
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
		
		
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);
	trap_data_append_param_str(tData, "%s s %d",	EI_AP_RSSI_TRAP_DES, 		rssi);

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
		
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;	
}

int th_sta_leave_abnormal_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_STA_LEAVE
	cmd_test_out(wtpStationOffLineAbnormalTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned int	g_bss=0;
	unsigned char	wlanid=0;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int instance_id=0;
	unsigned char rssi = 0;
	unsigned int local_id = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_UINT32, &g_bss,
								DBUS_TYPE_BYTE,   &wlanid,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_BYTE,&rssi,	//xiaodawei add 20110301
								DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex = %d, wtpsn = %s, g_bss = %d, wlanid = %d, netid = %s, instance_id = %d"
				            " wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				            dbus_message_get_member(message), wtpindex, wtpsn, g_bss, wlanid, netid, instance_id,
				            wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) )
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpStationOffLineAbnormalTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
		
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
			
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
		
		
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);
	//trap_data_append_param_str(tData, "%s s %d",	EI_AP_RSSI_TRAP_DES, 		rssi);

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
		
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;	
}

int th_sta_assoc_fail_func(DBusMessage * message)
{
	//ASD_DBUS_SIG_STA_ASSOC_FAILED
	cmd_test_out(APStAssociationFailTrap);

	DBusError error;
	unsigned int wtpindex=0;
	
	char str_reason[256];
	memset(str_reason,0, sizeof(str_reason));
	char *wtpsn;
	unsigned short 	reason_code;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_BYTE, &mac[0],
									DBUS_TYPE_BYTE, &mac[1],
									DBUS_TYPE_BYTE, &mac[2],
									DBUS_TYPE_BYTE, &mac[3],
									DBUS_TYPE_BYTE, &mac[4],
									DBUS_TYPE_BYTE, &mac[5],
									DBUS_TYPE_INT32,  &wtpindex,
									DBUS_TYPE_UINT16, &reason_code,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id,
									DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, reason_code=%d, netid=%s, local_id = %d, instance_id=%d, "
							"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, reason_code,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
		
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APStAssociationFailTrap);

	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);	
	switch(reason_code)
	{
		case 1:
			strcpy(str_reason,"Unspecified_failure.");
			break;
		case 14:
			strcpy(str_reason,"Transaction_sequence_number_out_of_expected_sequence.");
			break;
		case 17:
			strcpy(str_reason,"Association_denied_because_AP_is_unable_to_handle_additional_associated_STAs.");
			break;			
		case 23:
			strcpy(str_reason,"Association_request_rejected_because_the_information_in the_Power_Capability_element_is_unacceptable.");
			break;
		case 40:
			strcpy(str_reason,"Invalid_information_element.");
			break;
		case 41:
			strcpy(str_reason,"Invalid_group_cipher.");
			break;
		case 42:
			strcpy(str_reason,"Invalid_pairwise_cipher.");
			break;
		case 43:
			strcpy(str_reason,"Invalid_AKMP.");
			break;
		default:
			strcpy(str_reason,"Unspecified_failure.");	
			break;	
	}
		
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac);
		
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
		
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);

	//trap_data_append_common_param(tData, tDescr);
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, 		tDescr->trap_type);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, 	tDescr->trap_level);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
							trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, 	tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, 	str_reason);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID, 	tDescr->content);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//ap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;		
}

int th_sta_jianquan_fail_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_STA_JIANQUAN_FAILED
	cmd_test_out(APStaAuthErrorTrap);
	
	DBusError error;
	unsigned int wtpindex=0;
	
	char str_reason[100];
	memset(str_reason,0,100);
	char *wtpsn;
	unsigned short 	reason_code;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_BYTE, &mac[0],
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3],
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],
							DBUS_TYPE_INT32,  &wtpindex,
							DBUS_TYPE_UINT16, &reason_code,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, reason_code=%d,netid=%s, local_id = %d, instance_id=%d, "
						"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, reason_code,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APStaAuthErrorTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	switch(reason_code)
	{
	case 1:
		strcpy(str_reason,"802.1X_Authentication_time_out.");
		break;
	case 2:
		strcpy(str_reason,"802.1X_Reauthentication_times_reach_maxium.");
		break;
	case 3:
		strcpy(str_reason,"Invlalid_port.");
		break;
	case 4:
		strcpy(str_reason,"802.1X_Authentication_fail.");
		break;
	case 5:
		strcpy(str_reason,"4way_hand_shake_fail.");
		break;
	default:
		strcpy(str_reason,"Unspecified_failure.");
		break;
	}
		
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);

	//trap_data_append_common_param(tData, tDescr);
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, 		tDescr->trap_type);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, 	tDescr->trap_level);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
							trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, 	tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, 	str_reason);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID, 	tDescr->content);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//ap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

#if 0
//取dbus信息 特殊 
int ssid_key_conflict_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_KEY_CONFLICT
	cmd_test_out(SSIDkeyConflictTrap);

	DBusError error;
	unsigned char type, var1, var2;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &type,
								DBUS_TYPE_BYTE, &var1,
								DBUS_TYPE_BYTE, &var2,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, var1=%d, var2=%d\n",
				dbus_message_get_member(message), type, var1, var2);
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, SSIDkeyConflictTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	tData = trap_data_new_from_descr(tDescr);
	
	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
	
	trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
#endif 

//取dbus信息 特殊 
int wid_ssid_key_conflict_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_SSID_KEY_CONFLICT
	cmd_test_out(SSIDkeyConflictTrap);
	DBusError error;
	unsigned char wtpmac[MAC_LEN]={0};
	unsigned char radio_l_id, keyindex1, keyindex2;
	unsigned char *ssid1=NULL, *ssid2=NULL;
	unsigned int wtpindex=0;
	char *wtpsn;
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32,&wtpindex,
								DBUS_TYPE_STRING,&wtpsn,
								DBUS_TYPE_BYTE,&wtpmac[0],
								DBUS_TYPE_BYTE,&wtpmac[1],
								DBUS_TYPE_BYTE,&wtpmac[2],
								DBUS_TYPE_BYTE,&wtpmac[3],
								DBUS_TYPE_BYTE,&wtpmac[4],
								DBUS_TYPE_BYTE,&wtpmac[5],
								DBUS_TYPE_BYTE,&radio_l_id,
								DBUS_TYPE_STRING,&ssid1,
								DBUS_TYPE_STRING,&ssid2,
								DBUS_TYPE_BYTE,&keyindex1,
								DBUS_TYPE_BYTE,&keyindex2,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, radio_id=%d, local_id = %d, instance_id=%d, "
							"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, \n"
							"ssid1=%s, ssid2=%s,keyindex1=%d,keyindex2=%d \n",
				dbus_message_get_member(message), wtpindex, radio_l_id, local_id, instance_id, 
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				ssid1,ssid2,keyindex1,keyindex2);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, SSIDkeyConflictTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char netid[NETID_NAME_MAX_LENTH];	
	get_ap_netid(netid, sizeof(netid), wtpindex);
		
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %d",	TRAP_RADIO_INFO_OID,			radio_l_id);
	
	
	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//ap_data_destroy(tData);
		
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int th_sta_come_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_STA_COME
	cmd_test_out(wtpStationOnLineTrap);

	DBusError error;
	unsigned int  wtpindex=0;
	unsigned int  g_bss=0;
	unsigned char wlanid=0;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	unsigned char rssi = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_UINT32, &g_bss,
								DBUS_TYPE_BYTE,   &wlanid,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_BYTE,&rssi,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, g_bss=%d, wlanid=%d, netid=%s, local_id = %d, instance_id=%d,rssi=%d "
						"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, g_bss, wlanid,netid, local_id, instance_id,rssi,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpStationOnLineTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);
	trap_data_append_param_str(tData, "%s s %d",	EI_AP_RSSI_TRAP_DES, 		rssi);

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//ap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int th_wtp_deny_sta_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_WTP_DENY_STA
	cmd_test_out(wtpSubscriberDatabaseFullTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpSubscriberDatabaseFullTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//ap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int th_de_wtp_deny_sta_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_DE_WTP_DENY_STA
	cmd_test_out(APAddUserFailClearTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APAddUserFailClearTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//ap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int asd_wapi_trap_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_WAPI_TRAP
	cmd_test_out("ap 1.23-27");
	
	DBusError error;
	unsigned int wtpindex = 0;

	char *trap_reason = NULL;
	char *wtpsn;
	unsigned char reason;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_BYTE,   &reason,
								DBUS_TYPE_INT32,  &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, reason=%d, netid=%s, local_id = %d, instance_id=%d,"
							"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, reason,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	switch(reason){
		case 1:
		{
			trap_reason = "invalid_cert";	/*WAPI非法证书用户侵入网络通告*/
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, APUserWithInvalidCerInbreakNetTrap);

			break;
		}
		case 2:
		{
			trap_reason = "challenge_replay";  /*WAPI客户端重放攻击通告*/
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, APStationRepititiveAttackTrap);
			
			break;
		}
		case 3:
		{
			trap_reason = "mic_juggle";   /*WAPI篡改攻击通告*/
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, APTamperAttackTrap);
			
			break;
		}
		case 4:
		{
			trap_reason = "low_safe_level"; /*WAPI安全等级降低攻击通告*/
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, APLowSafeLevelAttackTrap);
			
			break;
		}
		default:
		{
			trap_reason = "addr_redirection"; /*W	API地址重定向攻击通告*/
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, APAddressRedirectionAttackTrap);
			
			break;
		}
	}
	
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);

	//trap_data_append_common_param(tData, tDescr);
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, 		tDescr->trap_type);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, 	tDescr->trap_level);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
							trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, 	tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, 	trap_reason );
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID, 	trap_reason );

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int asd_sta_verify_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_STA_VERIFY
	cmd_test_out(wtpSolveLinkVarifiedTrap);

	DBusError error;
	unsigned int wtpindex=0;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_INT32,  &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d, "
						"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpSolveLinkVarifiedTrap);
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int asd_sta_verify_failed_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_STA_VERIFY_FAILED
	cmd_test_out(wtpLinkVarifyFailedTrap);

	DBusError error;
	unsigned int wtpindex=0;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_INT32,  &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d, "
						"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpLinkVarifyFailedTrap);
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char sta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

static int
eag_user_logoff_abnormal(DBusMessage *message) 
{
	cmd_test_out(wtpUserLogoffAbnormalTrap);
	
	DBusError error;
	int eag_trap = EAG_TRAP;
	char *apmac = NULL;	
	unsigned int wtpid = 0;
	char *stamac = NULL;	
	char *username = NULL;
	char *staip = NULL;
	int terminate_cause = 0;
	int hansi_id = 0;
	int hansi_type = 0;
	unsigned int local_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &eag_trap,
								DBUS_TYPE_STRING, &apmac,
								DBUS_TYPE_UINT32, &wtpid,
								DBUS_TYPE_STRING, &stamac,
								DBUS_TYPE_STRING, &username,
								DBUS_TYPE_STRING, &staip,
								DBUS_TYPE_INT32, &terminate_cause,
								DBUS_TYPE_INT32, &hansi_id,
								DBUS_TYPE_INT32, &hansi_type,
								DBUS_TYPE_INVALID )))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, eag_trap=%d, apmac=%s, wtpid=%d, stamac=%s, username=%s, "
						"staip=%s, terminate_cause=%d, hansi_id=%d, hansi_type=%d\n",
				dbus_message_get_member(message), eag_trap, apmac, wtpid, stamac, username,
				staip, terminate_cause, hansi_id, hansi_type);

	local_id = hansi_type;

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, hansi_id) )
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpUserLogoffAbnormalTrap);
	if (NULL == tDescr || 0==tDescr->switch_status) {
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;
	}
	
	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpid, tDescr, local_id, hansi_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char wtp_mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(wtp_mac_oid, sizeof(wtp_mac_oid), apmac);
	
	char sta_mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(sta_mac_oid, sizeof(sta_mac_oid), stamac);

	trap_data_append_param_str(tData, "%s s %s", EI_MAC_TRAP_DES, apmac);
	trap_data_append_param_str(tData, "%s%s%s s %s", EI_STA_MAC_TRAP_DES, wtp_mac_oid, sta_mac_oid, stamac);
	trap_data_append_param_str(tData, "%s%s%s s %s", EI_STA_USER_IPADDR, wtp_mac_oid, sta_mac_oid, staip);
	trap_data_append_param_str(tData, "%s%s%s s %s", EI_STA_USER_NAME, wtp_mac_oid, sta_mac_oid, username);
	
	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][hansi_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpid , tDescr, tData, local_id, hansi_id);

	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

#if 0
int asd_radius_connect_failed_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_RADIUS_CONNECT_FAILED
	cmd_test_out(acRadiusAuthenticationServerNotReachTrap);

	DBusError error;
	char *ip = "";
	unsigned char type;
	unsigned int instance_id = 0;
	char radius_type[20];
	memset (radius_type, 0, 20);
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_STRING, &ip,
							DBUS_TYPE_BYTE, &type,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, ip=%s, type=%d\n", dbus_message_get_member(message),
						ip, type);
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if (0==type)
	{
		strcpy (radius_type, "auth");
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acRadiusAuthenticationServerNotReachTrap);
	}
	else
	{
		strcpy (radius_type, "account");
		tDescr = trap_descr_list_get_item(&gDescrList, acRadiusAccountServerNotLinkTrap);
	}
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	tData = trap_data_new_from_descr(tDescr);
	
	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname);

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
		
	trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
#endif

int ac_radius_auth_reach_status_func(DBusMessage *message)
{
	cmd_test_out(acRadiusAuthenticationServerNotReachTrap);

	DBusError error;
	char *ip = "";
	unsigned int type ;
	unsigned short port = 0;
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &type,
							DBUS_TYPE_STRING, &ip,
							DBUS_TYPE_UINT16, &port,
							DBUS_TYPE_UINT32, &local_id,
							DBUS_TYPE_UINT32, &instance_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, type = %d, ip = %s, port = %d, local_id = %d, instance_id = %d \n", 
	                        dbus_message_get_member(message), type, ip, port, local_id, instance_id);
	                        
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) 
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
    
	TrapDescr *tDescr = NULL;

	if (OVER_THRESHOLD_FLAG==type)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acRadiusAuthenticationServerNotReachTrap);
	}else if (NOT_OVER_THRESHOLD==type)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acRadiusAuthServerAvailableTrap);
	}

	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
	TrapData *tData = trap_data_new_from_descr(tDescr);
	if(NULL == tData) {
        trap_syslog(LOG_INFO, "ac_radius_auth_reach_status_func: trap_data_new_from_descr malloc tData error!\n");
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;
	}

	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname);
	trap_data_append_param_str(tData, "%s s %s", EI_AC_RADIUS_AUTH_IP, 	  ip);
	trap_data_append_param_str(tData, "%s s %d", EI_AC_RADIUS_AUTH_PORT,  port);

	trap_data_append_common_param(tData, tDescr);

    trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
    TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, local_id, instance_id); 
            	

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_radius_acct_reach_status_func(DBusMessage *message)
{
	cmd_test_out(acRadiusAccountServerNotLinkTrap);

	DBusError error;
	char *ip = "";
	unsigned int type ;
	unsigned short port = 0;	
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &type,
							DBUS_TYPE_STRING, &ip,
							DBUS_TYPE_UINT16, &port,
                            DBUS_TYPE_UINT32, &local_id,
                            DBUS_TYPE_UINT32, &instance_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, ip=%s, port=%d, local_id = %d, instance_id = %d \n", 
	                        dbus_message_get_member(message), type, ip, port, local_id, instance_id);
    
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) )
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
    
	TrapDescr *tDescr = NULL;

	if (OVER_THRESHOLD_FLAG==type)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acRadiusAccountServerNotLinkTrap);
	}else if (NOT_OVER_THRESHOLD==type)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acRadiusAccServerAvailableTrap);
	}

	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);	

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	

	TrapData *tData = trap_data_new_from_descr(tDescr);
	if(NULL == tData) {
        trap_syslog(LOG_INFO, "ac_radius_acct_reach_status_func: trap_data_new_from_descr malloc tData error!\n");
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;
	}

	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname);
	trap_data_append_param_str(tData, "%s s %s", EI_AC_RADIUS_ACCT_IP, 	  ip);
	trap_data_append_param_str(tData, "%s s %d", EI_AC_RADIUS_ACCT_PORT,  port);

	trap_data_append_common_param(tData, tDescr);
	
    trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
    TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, local_id, instance_id); 

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

#if 0
int asd_radius_connect_failed_clean_func(DBusMessage *message)
{
	//ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN
	cmd_test_out(acRadiusAuthServerAvailableTrap);
	DBusError error;
	char *ip = "";
	unsigned char type;
	unsigned int instance_id = 0;
	char radius_type[20];
	memset (radius_type, 0, 20);

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_STRING, &ip,
								DBUS_TYPE_BYTE, &type,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, ip=%s, type=%d\n", dbus_message_get_member(message),
					ip, type);
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;	

	if (type == 0)
	{
		strcpy (radius_type, "auth");
		tDescr = trap_descr_list_get_item(&gDescrList, acRadiusAuthServerAvailableTrap);
	}
	else
	{
		strcpy (radius_type, "account");
		tDescr = trap_descr_list_get_item(&gDescrList, acRadiusAccServerAvailableTrap);
	}
	
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	tData = trap_data_new_from_descr(tDescr);
	
	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname );

	trap_data_append_common_param(tData, tDescr);
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
		
	trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
#endif

int wtp_wireless_interface_down_func(DBusMessage *message) 	/*无线链路中断告警*/
{
	//WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN
	cmd_test_out(APRadioDownTrap);
	
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APRadioDownTrap);		
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);

	#if 0
	// 这个节点在其他dbus信号中用到了，所以单独设置公共数据
		trap_data_append_common_param(tData, tDescr);
	#endif
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, TRAP_TYPE_ENVIRO );
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, TRAP_LEVEL_MAJOR);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
							trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, "wid_ap_linkdown_error" );
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID, "wid_ap_linkdown_error" );
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_wireless_interface_down_clear_func(DBusMessage *message)  /*无线链路中断告警*/
{
	//WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN_CLEAR
	cmd_test_out(APRadioDownRecovTrap);
	
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id, 
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if(!trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id)) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APRadioDownRecovTrap);	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);

	#if 0
	// 这个节点在其他dbus信号中用到了，所以单独设置公共数据
		trap_data_append_common_param(tData, tDescr);
	#endif
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, TRAP_TYPE_ENVIRO );
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, TRAP_LEVEL_MAJOR);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
							trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, "wid_ap_linkdown_error_clear" );
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID, "wid_ap_linkdown_error_clear" );
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}


int route_method_notify_snmp_by_ip_func(DBusMessage *message)
{
	//NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP
	cmd_test_out(acChangedIPTrap);

	if (ac_trap_get_flag("/var/run/standby_switch_trap_flag"))
		return -1;

	DBusError error;
	unsigned int ifindex;
	unsigned char mask;
	char* ip = "";
	unsigned int isAdd;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &isAdd,
									DBUS_TYPE_BYTE, &mask,
									DBUS_TYPE_STRING, &ip,
									DBUS_TYPE_UINT32, &ifindex,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, ifindex = %d, isAddr=%d, mask=%d, ip=%s\n", dbus_message_get_member(message),
						ifindex, isAdd, mask, ip);
	
	TrapDescr *tDescr = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acChangedIPTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);

    char *ifname = NULL;
	if(0 != get_if_name(ifindex, &ifname) || NULL == ifname) {
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;
	}

	int ifname_slotid = 0;
	int local_slotid = 0;
	ifname_slotid = manage_get_slot_id_by_ifname(ifname);
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slotid = get_product_info(PRODUCT_LOCAL_SLOTID);
	}
	if(ifname_slotid != local_slotid)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_IF_IS_NOT_LOCAL in route_method_notify_snmp_by_ip_func fail\n");
		return TRAP_SIGNAL_HANDLE_IF_IS_NOT_LOCAL;
	}
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "route_method_notify_snmp_by_ip_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}
	        
        	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname );
        	trap_data_append_param_str(tData, "%s s %s", AC_INTERFACE_OID, ifname );
        	trap_data_append_param_str(tData, "%s s %s", AC_INTERFACE_IP_OID, ip );
        	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, AC_INTERFACE_IP_CHANGE_STATE_OID, isAdd ? "add" : "delete" );
        	trap_data_append_common_param(tData, tDescr);
        	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
	    }
	}

	SNMP_FREE(ifname);
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

#if 0
int route_method_notify_snmp_by_vrrp_func(DBusMessage *message)
{
	//NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP
	cmd_test_out(acTurntoBackupDeviceTrap);

	DBusError error;
	unsigned int state;
	char sysmac[6] = {0};
			
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
										DBUS_TYPE_UINT32, &state,
										DBUS_TYPE_BYTE, &sysmac[0],
										DBUS_TYPE_BYTE, &sysmac[1],
										DBUS_TYPE_BYTE, &sysmac[2],
										DBUS_TYPE_BYTE, &sysmac[3],
										DBUS_TYPE_BYTE, &sysmac[4],
										DBUS_TYPE_BYTE, &sysmac[5],
										DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, state=%d, sysmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), state,
				sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);

	if (3 == state)
		gGlobalInfo.vrrp_switch = 1;
	else
		gGlobalInfo.vrrp_switch = 0;
	trap_syslog(LOG_INFO, "vrrp_switch=%d, state=%d\n", gGlobalInfo.vrrp_switch, state);
	
	if (99 == gGlobalInfo.vrrp_last_state && 2 == state) {
		gGlobalInfo.vrrp_last_state = state;
		return 0;
	}
	gGlobalInfo.vrrp_last_state = state;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acTurntoBackupDeviceTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	tData = trap_data_new_from_descr(tDescr);	
	
	char *title   = NULL;
	char *content = NULL;
	switch(state)
	{
		case 2:
		case 99:
			title 	= "AC_TURNTO_BACK_BY_VRRP";
			content = "AC_TURNTO_BACK_BY_VRRP";
			break;
			
		case 3:
			title 	= "AC_TURNTO_MASTER_BY_VRRP";
			content = "AC_TURNTO_MASTER_BY_VRRP";
			break;

		default:
			break;
	}
	
	char mac_str[MAC_STR_LEN];
	get_ac_mac_str(mac_str, sizeof(mac_str), sysmac);
		
	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES,	  mac_str);
	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname );
	//trap_data_append_common_param(tData, tDescr);
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, tDescr->trap_type);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, tDescr->trap_level);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
							trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID,  title);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID,content);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
	
	trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
#endif

void *trap_check_and_set_vrrp_preempt_thread(void *arg)
{
	int numbytes;
	socklen_t addr_len;
	int ret;
	fd_set fdset;
	int i = 0;
	struct timeval tv;						
	int interval = ((VrrpValue*)arg)->interval;
	int profile = ((VrrpValue*)arg)->profile;
	if(arg)
	{
		free(arg);
	}
	int check_sock;
	if ((check_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
	{	
		return;
	}	
	addr_len = sizeof(struct sockaddr);
	FD_ZERO(&fdset);
	FD_SET(check_sock,&fdset);
	while(1){
		if(0 >= interval){
			trap_syslog(LOG_INFO, "%s %d,interval=%d,change to 20min.\n",__func__,__LINE__,interval);
			interval = 20;
		}
		tv.tv_sec = interval*60;
		tv.tv_usec = 0;
		trap_syslog(LOG_INFO, "%s %d,tv.tv_sec=%d\n",__func__,__LINE__,tv.tv_sec);
		ret = select(check_sock + 1,&fdset,(fd_set *) 0,(fd_set *) 0,&tv);

		if(ret == -1){
			trap_syslog(LOG_INFO, "%s %d\n",__func__,__LINE__);

			
		}else if(ret ==0){
			trap_syslog(LOG_INFO, "%s %d\n",__func__,__LINE__);
			trap_notice_vrrp_config_service_change_state(profile,0);
			sleep(10);
			trap_notice_vrrp_config_service_change_state(profile,1);
		}
		break;
	}
	close(check_sock);
	pthread_exit((void *) 0);
}

int trap_set_vrrp_preempt(VrrpValue *argv){
	pthread_attr_t attr;
	int s = PTHREAD_CREATE_DETACHED;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,s);
	pthread_t WID_BAK;
	if(pthread_create(&WID_BAK, &attr, trap_check_and_set_vrrp_preempt_thread, argv) != 0) {
		return NULL;
	}				

}
int route_method_notify_snmp_by_vrrp_func(DBusMessage *message)
{
	//NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP
	cmd_test_out(acTurntoBackupDeviceTrap);

	unsigned int is_active_master = 0;

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}

	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in route_method_notify_snmp_by_vrrp_func fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

	DBusError error;
	unsigned int state=0;//sure hansi state is not include 0
	char sysmac[6] = {0};
	unsigned int instance_id = 0;
	int statistic_hansi_state=0; //statistic hansi state for ac trap if there has hansi is master instance_id 1 is master state else is 0 backup
	char instance_ip[128]={0};
	//u_int32_t uplink_vip=0;
	int state_v = 0;
	int pre_vrrp_state = 0;
	int interval = 20;
	TrapNode *tNode = NULL;
	TrapReceiver *tRcv = NULL;
	TrapList *receivelist = NULL;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
										DBUS_TYPE_UINT32, &instance_id,
										DBUS_TYPE_UINT32, &state,
										DBUS_TYPE_BYTE, &sysmac[0],
										DBUS_TYPE_BYTE, &sysmac[1],
										DBUS_TYPE_BYTE, &sysmac[2],
										DBUS_TYPE_BYTE, &sysmac[3],
										DBUS_TYPE_BYTE, &sysmac[4],
										DBUS_TYPE_BYTE, &sysmac[5],
										//DBUS_TYPE_UINT32,&uplink_vip,
										DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s,instance_id=%d, state=%d, sysmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), instance_id, state, 
				sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);

	if ( 0 == instance_id || instance_id > INSTANCE_NUM )
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	
	receivelist = gInsVrrpState.instance[0][instance_id].receivelist;

#if 0
	if (99 == trap_get_instance_vrrp_state(&gInsVrrpState, 0, instance_id) && 2 == state) //backup state
	{
		trap_set_instance_vrrp_state(&gInsVrrpState, 0, instance_id, state);

		/***clear aptrap and actrap items once more***/
		trap_destory_wtp_by_instance(&global, 0, instance_id);

		trap_vrrp_clear_actrap_items(global.gDescrList_hash, global.actrap);

		close_trap_receiverList_session(receivelist);

		trap_syslog(LOG_DEBUG,"trap_get_instance_vrrp_state remote hansi instance(%d) is backup instance remove all initialized sessions\n", instance_id);
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
		
	}else {
#endif	
		pre_vrrp_state = gInsVrrpState.instance[0][instance_id].pre_vrrp_state;
		gInsVrrpState.instance[0][instance_id].pre_vrrp_state = state;
		trap_set_instance_vrrp_state(&gInsVrrpState, 0, instance_id, 3 == state ? 1 : 0);

		if(trap_instance_is_master(&gInsVrrpState, 0, instance_id ))  
		{	
			#if 0
			/***instance backup local ip***/
			if ( 0 != uplink_vip){
					sprintf(instance_ip,"%d.%d.%d.%d ",((uplink_vip & 0xff000000) >> 24),((uplink_vip & 0xff0000) >> 16),	\
											((uplink_vip& 0xff00) >> 8),(uplink_vip & 0xff));
					
					strncpy(gInsVrrpState.instance[0][instance_id].trap_instance_ip, instance_ip, 128);

					trap_syslog(LOG_DEBUG,"instance_ip(%s)\n", instance_ip );
			}else {
			
				trap_syslog(LOG_INFO,"uplink_vip = 0 gInsVrrpState.instance[instance_id].receivelist init default local ip!\n");
				
			}
			#endif
			/*init trap receverList session*/
			init_trap_receverList_session(receivelist);
		
			/***init wtp by instance***/
			trap_init_wtp_array(&(global.wtp[0][instance_id]), MAXWTPID);	
		
		}
		else{     //state = 99

		    /***clear aptrap and actrap items***/

		    trap_destory_wtp_by_instance(&global, 0, instance_id );

		    trap_vrrp_clear_actrap_items(global.gDescrList_hash,global.actrap);
		    
		    /*close trap receiverList session*/
		    close_trap_receiverList_session(receivelist);
		}
#if 0
	}
#endif
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	trap_syslog(LOG_DEBUG,"trap_get_instance_vrrp_state instance %d is %s state\n", instance_id, (state==3)?"MASTER":(state == 2)?"BACK" :\
										(state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");

#if 0
	if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
#endif

    unsigned int backup_status = 0;
	if (99 == state || 2 == state ){
	    backup_status = 1;  //slave
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acTurntoBackupDeviceTrap);
	}
	else if (3 == state){
	    backup_status = 2;  //master
		TrapDescr *tDescr_tmp = trap_descr_list_get_item(global.gDescrList_hash, acTurntoBackupDeviceTrap);
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acTurntoMasterDeviceTrap);
		tDescr->switch_status = tDescr_tmp->switch_status;
		state_v = ac_preempt_switch_trap_has_sent();
		interval = ac_preempt_interval_trap_has_sent();
		trap_syslog(LOG_INFO, "pre_vrrp_state=%d state_v=%d,interval=%d.\n",pre_vrrp_state,state_v,interval);
		if((2 == pre_vrrp_state)&&(1 == state_v)){
			VrrpValue *value=NULL;
			value = (VrrpValue*)malloc(sizeof(VrrpValue));
			if(value){
				memset(value, 0, sizeof(VrrpValue));
				value->interval = interval;
				value->profile = instance_id;
				trap_syslog(LOG_INFO, "instance_id=%d value.profile=%d value.interval=%d\n",instance_id,value->profile,value->interval);
				trap_set_vrrp_preempt(value);
			}
		}
		
	}else {
		return TRAP_SIGNAL_HANDLE_AC_STATE_ERROR;
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
	tData = trap_data_new_from_descr(tDescr);	
	
	char mac_str[MAC_STR_LEN];
	get_ac_mac_str(mac_str, sizeof(mac_str), sysmac);
		
	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES,	  mac_str);
	trap_data_append_param_str(tData, "%s s %d", TRAP_INSTANCE_OID, instance_id);
	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname );
	trap_data_append_param_str(tData, "%s s %d", AC_BACKUP_STAUTS_OID, backup_status);
	trap_data_append_param_str(tData, "%s s %s", AC_BACKUP_NETWORK_IP_OID, gInsVrrpState.instance[0][instance_id].backup_network_ip);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[0][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, 0, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}


int wtp_channel_device_interference_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE
	cmd_test_out(wtpChannelObstructionTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char chchannel;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_BYTE,   &chchannel,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_BYTE, &mac[0],
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3],
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, chchannel=%d, netid = %s, local_id = %d, instance_id,"
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, chchannel,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	char *strTrapType[] = {wtpChannelObstructionTrap, wtpDeviceInterferenceDetectedTrap};
	int i = 0;
	int has_sent = 0;
	while (i<(sizeof(strTrapType)/sizeof(strTrapType[0])))
	{
		tDescr = NULL;
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, strTrapType[i]);
		i++;
	
		if (NULL == tDescr || 0 == tDescr->switch_status)
			continue;

		INCREASE_TIMES(tDescr);

		TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
		
		tData = trap_data_new_from_descr(tDescr);
	
		char mac_str[MAC_STR_LEN];
		get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

		char sta_mac_str[MAC_STR_LEN];
		get_ap_mac_str( sta_mac_str, sizeof(sta_mac_str), mac );
	
		char mac_oid[MAX_MAC_OID];
		get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
		
//		char netid[NETID_NAME_MAX_LENTH];	
//		get_ap_netid(netid, sizeof(netid), wtpindex);
	
	
		trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
		trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
		trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
		trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		sta_mac_str);
		trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);

		trap_data_append_common_param(tData, tDescr);
		trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

		TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
		
		//trap_data_destroy(tData);
		has_sent = 1;
	}

	if (has_sent)
		return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
	else
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;
}

int wtp_channel_ap_interference_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE
	cmd_test_out(wtpAPInterferenceDetectedTrap);
	
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char chchannel;
	unsigned char mac[MAC_LEN];
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_BYTE,   &chchannel,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
                                DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, chchannel=%d, netid=%s, local_id = %d, instance_id=%d,"
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, chchannel,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpAPInterferenceDetectedTrap);		
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char inter_mac_str[MAC_STR_LEN];
	get_ap_mac_str( inter_mac_str, sizeof(inter_mac_str), mac);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		inter_mac_str);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_terminal_interference_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE
	cmd_test_out(wtpStaInterferenceDetectedTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char chchannel;
	unsigned char mac[MAC_LEN];
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned char radio_l_id;
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_BYTE,   &chchannel,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_BYTE, &radio_l_id,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id, 
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, channel=%d,netid=%s, local_id = %d, instance_id=%d, radio_l_id=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, chchannel,netid, local_id, instance_id,radio_l_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpStaInterferenceDetectedTrap);	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char inter_mac_str[MAC_STR_LEN];
	get_ap_mac_str( inter_mac_str, sizeof(inter_mac_str), mac);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES,		inter_mac_str);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);			
	trap_data_append_param_str(tData, "%s s %d",	TRAP_RADIO_INFO_OID,			radio_l_id);

	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_count_minor_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR
	cmd_test_out(wtpDFSFreeCountBelowThresholdTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int instance_id = 0;
	unsigned int local_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpDFSFreeCountBelowThresholdTrap);		
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);

	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_device_interference_clear_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE_CLEAR
	cmd_test_out(APOtherDevInterfClearTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char chchannel;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_BYTE,   &chchannel,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,channel=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, chchannel,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APOtherDevInterfClearTrap);	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_ap_interference_clear_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE_CLEAR
	cmd_test_out(APInterfClearTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char chchannel;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_BYTE,   &chchannel,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s,netid=%s, local_id = %d, instance_id=%d, channel=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, chchannel,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APInterfClearTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);			
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_terminal_interference_clear_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE_CLEAR
	cmd_test_out(APStaInterfClearTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char chchannel;
	char *wtpsn;
	char *netid = "";
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	unsigned char radio_l_id;
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_BYTE,   &chchannel,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_BYTE, &radio_l_id,
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_STRING, &netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, channel=%d, netid=%s, local_id = %d, instance_id=%d, radio_l_id=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, chchannel, netid, local_id, instance_id, radio_l_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APStaInterfClearTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char inter_mac_str[MAC_STR_LEN];
	get_ap_mac_str( inter_mac_str, sizeof(inter_mac_str), mac);
	
	//char netid[NETID_NAME_MAX_LENTH];	
	//get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES,		inter_mac_str);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);			
	trap_data_append_param_str(tData, "%s s %d",	TRAP_RADIO_INFO_OID,			radio_l_id);

	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_channel_count_minor_clear_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR_CLEAR
	cmd_test_out(APChannelTooLowClearTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s,netid=%s, local_id = %d, instance_id=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APChannelTooLowClearTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,		  mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 		  mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);			
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);

	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_cpu_threshold_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD
	cmd_test_out(CPUusageTooHighTrap); 

	DBusError error;
	unsigned int wtpindex;
	unsigned int cpu = 0;
	unsigned int cpu_threshold = 0;
	unsigned char flag = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_UINT32, &cpu,
								DBUS_TYPE_UINT32, &cpu_threshold,
								DBUS_TYPE_BYTE, &flag,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,cpu=%d, cpu_threshold=%d, flag=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,cpu, cpu_threshold, flag,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if ( 1 == flag )
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, CPUusageTooHighTrap);
	}
	else if( 0 == flag )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, CPUusageTooHighRecovTrap);	
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_WTP_CPU_USAGE_TRAP_DES, 	mac_oid, cpu);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_WTP_CPU_THRESHOLD_TRAP_DES,	mac_oid, cpu_threshold);	
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
	
	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);

	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_mem_threshold_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD
	cmd_test_out(MemUsageTooHighTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned int mem = 0;
	unsigned int mem_threshold = 0;
	unsigned char flag = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE,   &mem,
								DBUS_TYPE_UINT32, &mem_threshold,
								DBUS_TYPE_BYTE, &flag,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,mem=%d, mem_threshold=%d, flag=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, mem, mem_threshold, flag,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if (1==flag)
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, MemUsageTooHighTrap);
	}
	else if (0==flag)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, MemUsageTooHighRecovTrap);	
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_WTP_MEM_USAGE_TRAP_DES, 	mac_oid, mem);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_WTP_MEM_THRESHOLD_TRAP_DES,	mac_oid, mem_threshold);	
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);

	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_temp_threshold_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD
	cmd_test_out(TemperTooHighTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned int temperature = 0;
	unsigned int temp_threshold = 0;
	unsigned char flag = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE,   &temperature,
							DBUS_TYPE_UINT32, &temp_threshold,
							DBUS_TYPE_BYTE, &flag,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id, 
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,temp=%d, tem_threshold=%d, flag=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,temperature, temp_threshold, flag,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20a
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if (1==flag)
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, TemperTooHighTrap);
	}
	else if (0==flag)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, TemperTooHighRecoverTrap);	
	}

//for test 
//	NULL==tDescr? trap_syslog(LOG_DEBUG, "tDescr=%x trap_test\n", tDescr): \
//					trap_syslog(LOG_DEBUG, "tDescr=%x tDescr->swich_status=%d trap_test\n", tDescr, tDescr->switch_status);

	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	
	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_WTP_TEMP_USAGE_TRAP_DES, mac_oid, temperature);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_wifi_if_error_func(DBusMessage *message) //无线模块故障,多余定义，暂时去掉
{
//WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR
	cmd_test_out(APModuleTroubleTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char ifindex = 0;
	unsigned char state = 0;//0-not exist//1-up//2-down//3-error
	unsigned char flag = 2; 
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &ifindex,
							DBUS_TYPE_BYTE, &state,
							DBUS_TYPE_BYTE, &flag,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,ifindex=%d, state=%d, flag=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,ifindex, state, flag,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if (1==flag)
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, APModuleTroubleTrap);
	}
	else if (0==flag)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, APModuleTroubleClearTrap);	
	}
	
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,		  mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 		  mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_ath_if_error_func(DBusMessage *message)  //无线模块中断告警
{
//WID_DBUS_TRAP_WID_AP_ATH_ERROR
	cmd_test_out(APRadioDownTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char radioid = 0;
	unsigned char wlanid = 0;
	unsigned char type = 0;// 1-manual  /2-auto
	unsigned char flag = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &radioid,
							DBUS_TYPE_BYTE, &wlanid,
							DBUS_TYPE_BYTE, &type,// 1-manual  /2-auto
							DBUS_TYPE_BYTE, &flag,// 0-disable /1-enable
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id, 
							DBUS_TYPE_UINT32,&local_id, 
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,radioid=%d, wlanid=%d, type=%d, flag=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, radioid, wlanid, type, flag,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	if (1==flag)
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, APRadioDownRecovTrap);
	}
	else if (0==flag)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, APRadioDownTrap);	
	}
	
	if (NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,		  mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 		  mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_mt_work_mode_chg_func(DBusMessage *message) //无线监控模式更改
{
//WID_DBUS_TRAP_WID_AP_RRM_STATE_CHANGE
	cmd_test_out(APMtWorkModeChgTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char state = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &state,// 0-disable /1-enable
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id, 
							DBUS_TYPE_UINT32,&local_id, 
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s,netid=%s, local_id = %d, instance_id=%d, state=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,state,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APMtWorkModeChgTrap);
	if (NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int power_state_change(DBusMessage *message)
{
//"wid_dbus_trap_power_state_change"
	cmd_test_out(acPowerOffTrap);

	unsigned int is_active_master = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}

	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in power_state_change fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

	DBusError error;
//	int power_state = -1; 	// 0  power  is trap, 1 power clearl
	unsigned int  power_state = -1; 
   	unsigned int  index=0;	                              //1//power 1,power 2,power 3

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &power_state,
							DBUS_TYPE_UINT32,&index,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal  %s, power_state=%d,index=%d\n",
				dbus_message_get_member(message), power_state,index);
		
	TrapDescr *tDescr = NULL;

	switch (power_state)
	{
		case 0:
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, acPowerOffRecovTrap);
			break;

		case 1:
			tDescr = trap_descr_list_get_item(global.gDescrList_hash, acPowerOffTrap );		
			break;

		default:
			break;
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	
	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "power_state_change: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}
	
        	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname);
			trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_AC_POWER_OFF_INDEX_OID , index);
        	trap_data_append_common_param(tData, tDescr);
        	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
	    }
	}
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_run_quit_state(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_AP_RUN_QUIT
	cmd_test_out(wtpOnlineTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char state = 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &state,// 0-quit /1-run
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,state=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, state,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;

	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if (1==state)
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpOnlineTrap);
	}
	else if ( 0==state )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpOfflineTrap);
		if ( NULL!=global.wtp[local_id][instance_id] && NULL!=global.wtp[local_id][instance_id]->ap && NULL!=global.wtp[local_id][instance_id]->ap[wtpindex] )
		{
			trap_ap_offline_clear_aptrap_items(global.gDescrList_hash, global.wtp[local_id][instance_id]->ap[wtpindex]);
		}
	}

	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);

	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}


/*
enum LTEFIQuitResaon {
	0,            //for reserved
	ACC_LEAVE=1,  //ACC超时导致AP断电离线
	2,            //for extend
};
*/
/*wangchao add*/
int ap_lte_run_quit_state(DBusMessage * message)
{
	cmd_test_out(wtpLteOnlineTrap);

	DBusError error;
	unsigned int wtpindex;
	unsigned char quit_reason= 2;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &quit_reason,// 0-quit /1-run
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,quit_reason=%d, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, quit_reason,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);

	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;

	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

    if (0 == quit_reason)
    {
    	tDescr = trap_descr_list_get_item(global.gDescrList_hash, ReservedReason);
    }
	else if (1==quit_reason)
	{	
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, ACCTimeout);
	}
	else if (2 == quit_reason) 
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, ExtendReason);
	}
	/*
	else if ( 0==quit_reason )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpLteOfflineTrap);
		if ( NULL!=global.wtp[local_id][instance_id] && NULL!=global.wtp[local_id][instance_id]->ap && NULL!=global.wtp[local_id][instance_id]->ap[wtpindex] )
		{
			trap_ap_lte_offline_clear_aptrap_items(global.gDescrList_hash, global.wtp[local_id][instance_id]->ap[wtpindex]);
		}
	}
*/
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);

	trap_syslog(LOG_INFO,"mac_str == %s, wtpsn == %s, netid == %s\n", mac_str,wtpsn,netid);
	
	trap_data_append_common_param(tData, tDescr);

	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);

//	trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}



/*wangchao add*/
int wid_dbus_trap_wid_lte_fi_uplink_switch(DBusMessage * message)
{
	cmd_test_out(LteFiUplinkSwitch);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	char *lte_switch_data = "";
	unsigned short band = 0;
	char *ID = "";
	char *MODE = "";

	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING,&lte_switch_data,
							DBUS_TYPE_STRING,&wtpmac, 
							DBUS_TYPE_UINT16,&band, 
							DBUS_TYPE_STRING,&ID,
							DBUS_TYPE_STRING,&MODE,							
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,lte_switch_date=%s, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X,\
						 band = %d, ID = %s ,MOD = %s\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, lte_switch_data,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],band,ID,MODE);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;

	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, LteFiUplinkSwitch);

	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);


	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, lte_switch_data);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, band);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, ID);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, MODE);
	

	trap_syslog(LOG_INFO,"mac_str == %s, wtpsn == %s, netid == %s\n", mac_str,wtpsn,netid);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
//	trap_data_destroy(tData);
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}




int ap_find_unsafe_essid(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WTP_FIND_UNSAFE_ESSID
	cmd_test_out(APFindUnsafeESSID);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *name = "";
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_STRING, &name,
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s,netid=%s, local_id = %d, instance_id=%d, name=%s, wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id, name,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APFindUnsafeESSID);
	
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_ESSID_TRAP_DES,			mac_oid, name);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_find_wids_attack(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WTP_FIND_WIDS_ATTACK
	cmd_test_out(APFindSYNAttack);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char attackStamac[MAC_LEN];
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;

	unsigned char attacktype;
	unsigned char frametype;
	unsigned int attackcount;
	unsigned int fst_attack;
	unsigned int lst_attack;
	unsigned char chchannel;
	unsigned char rssi;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
							DBUS_TYPE_UINT32, &wtpindex,
							DBUS_TYPE_STRING, &wtpsn,
							DBUS_TYPE_BYTE, &wtpmac[0],
							DBUS_TYPE_BYTE, &wtpmac[1],
							DBUS_TYPE_BYTE, &wtpmac[2],
							DBUS_TYPE_BYTE, &wtpmac[3],
							DBUS_TYPE_BYTE, &wtpmac[4],
							DBUS_TYPE_BYTE, &wtpmac[5],
							DBUS_TYPE_BYTE, &attackStamac[0],
							DBUS_TYPE_BYTE, &attackStamac[1],
							DBUS_TYPE_BYTE, &attackStamac[2],
							DBUS_TYPE_BYTE, &attackStamac[3],
							DBUS_TYPE_BYTE, &attackStamac[4],
							DBUS_TYPE_BYTE, &attackStamac[5],
							DBUS_TYPE_STRING,&netid,
							DBUS_TYPE_UINT32,&instance_id,
							DBUS_TYPE_BYTE,&attacktype,	//.1.3.6.1.4.1.31656.6.1.1.13.5.1.3
							DBUS_TYPE_BYTE,&frametype,
							DBUS_TYPE_UINT32,&attackcount,
							DBUS_TYPE_UINT32,&fst_attack,
							DBUS_TYPE_UINT32,&lst_attack,
							DBUS_TYPE_BYTE,&chchannel,	//.1.3.6.1.4.1.31656.6.1.1.13.5.1.11
							DBUS_TYPE_BYTE,&rssi,		//.1.3.6.1.4.1.31656.6.1.1.13.5.1.10
							DBUS_TYPE_UINT32,&local_id,
							DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,attacktype=%d, frametype=%d, attackcount=%u,"
						"fst_attack=%u, lst_attack=%u, chchannel=%d, rssi=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, attackStamac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,attacktype,frametype,attackcount,fst_attack,lst_attack,chchannel,rssi,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				attackStamac[0], attackStamac[1], attackStamac[2], attackStamac[3], attackStamac[4], attackStamac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, APFindSYNAttack);		
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char atta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( atta_mac_str, sizeof(atta_mac_str), attackStamac);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,		  mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 		  mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %d",	EI_WTP_SYN_ATTACK_TYPE, attacktype);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 	  atta_mac_str);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_ROGUE_STA_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);
	trap_data_append_param_str(tData, "%s s %d",	EI_WTP_RSSI, rssi);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ap_neighbor_channel_interfere_func(DBusMessage *message)
{
//WID_DBUS_TRAP_WID_WTP_NEIGHBOR_CHANNEL_AP_INTERFERENCE
	cmd_test_out(ApNeighborChannelInterfTrap);
	
	DBusError error;
	unsigned int wtpindex;
	unsigned char chchannel;
	unsigned char flag;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_BYTE,   &chchannel,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_BYTE, &mac[0],
								DBUS_TYPE_BYTE, &mac[1],
								DBUS_TYPE_BYTE, &mac[2],
								DBUS_TYPE_BYTE, &mac[3],
								DBUS_TYPE_BYTE, &mac[4],
								DBUS_TYPE_BYTE, &mac[5],
								DBUS_TYPE_BYTE, &flag,
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, chchannel=%d, flag=%d,netid=%s, local_id = %d, instance_id=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn, chchannel, flag,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	if (1==flag)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, ApNeighborChannelInterfTrapClear);		
	}else if (0==flag)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, ApNeighborChannelInterfTrap);		
	}
	
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char inter_mac_str[MAC_STR_LEN];
	get_ap_mac_str( inter_mac_str, sizeof(inter_mac_str), mac);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		inter_mac_str);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
int wtp_configuration_error_trap(DBusMessage *message)
{
	cmd_test_out(wtpConfigurationErrorTrap);
	
	DBusError error;
	unsigned int wtpindex;
	unsigned char  err_code;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	char str_reason[50];
	memset(str_reason,0, sizeof(str_reason));
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32,&wtpindex,
								DBUS_TYPE_STRING,&wtpsn,
								DBUS_TYPE_BYTE,&wtpmac[0],
								DBUS_TYPE_BYTE,&wtpmac[1],
								DBUS_TYPE_BYTE,&wtpmac[2],
								DBUS_TYPE_BYTE,&wtpmac[3],
								DBUS_TYPE_BYTE,&wtpmac[4],
								DBUS_TYPE_BYTE,&wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_BYTE,&err_code,// file_missing(1),file_eror(2)
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s,netid=%s, local_id = %d, instance_id=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, error_code=%d\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5], err_code);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;

	switch(err_code)
	{
		case 1:
			strcpy(str_reason,"configuration_file_is_missing.");
			break;
		case 2:
			strcpy(str_reason,"configuration_file_is_error.");
			break;
		default:
			strcpy(str_reason,"Unspecified_type.");	
			break;	
	}

	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpConfigurationErrorTrap);		
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, str_reason);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_user_traffic_overload_func(DBusMessage *message)
{
	cmd_test_out(wtpUserTrafficOverloadTrap);
	
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn=NULL;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	unsigned char mac[MAC_LEN];
	unsigned char is_rx_tx;
	unsigned long long sta_flow = 0;
	char str_reason[30];
	memset(str_reason,0, sizeof(str_reason));
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32,&wtpindex,
								DBUS_TYPE_STRING,&wtpsn,
								DBUS_TYPE_BYTE,&wtpmac[0],
								DBUS_TYPE_BYTE,&wtpmac[1],
								DBUS_TYPE_BYTE,&wtpmac[2],
								DBUS_TYPE_BYTE,&wtpmac[3],
								DBUS_TYPE_BYTE,&wtpmac[4],
								DBUS_TYPE_BYTE,&wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_BYTE,&mac[0],//sta mac
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_BYTE,&is_rx_tx,//rx(1),tx(0)
								DBUS_TYPE_UINT64,&sta_flow,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X, flag_rx_tx=%d, sta_flow=%llu\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],is_rx_tx,sta_flow);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpUserTrafficOverloadTrap);		
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	switch(is_rx_tx)
	{
		case 1:
			strcpy(str_reason,"receiving_traffic.");
			break;
		case 0:
			strcpy(str_reason,"sending_traffic.");
			break;
		default:
			strcpy(str_reason,"Unspecified_type.");	
			break;	
	}


	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	char inter_mac_str[MAC_STR_LEN];
	get_ap_mac_str( inter_mac_str, sizeof(inter_mac_str), mac);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 		inter_mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, str_reason);
	trap_data_append_param_str(tData, "%s s %llu",	EI_STA_USER_TAFFIC,	sta_flow);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_unauthorized_Station_func(DBusMessage *message)
{
	cmd_test_out(wtpUnauthorizedStaMacTrap);
	
	DBusError error;
	unsigned int wtpindex;
	unsigned char chchannel;
	unsigned char flag;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	unsigned char *stamac_str=NULL;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32,&wtpindex,
								DBUS_TYPE_STRING,&wtpsn,
								DBUS_TYPE_BYTE,&wtpmac[0],
								DBUS_TYPE_BYTE,&wtpmac[1],
								DBUS_TYPE_BYTE,&wtpmac[2],
								DBUS_TYPE_BYTE,&wtpmac[3],
								DBUS_TYPE_BYTE,&wtpmac[4],
								DBUS_TYPE_BYTE,&wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id,
								DBUS_TYPE_UINT32,&local_id,
								DBUS_TYPE_STRING,&stamac_str,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d, "
				"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%s\n",
				dbus_message_get_member(message), wtpindex, wtpsn, netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],stamac_str);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpUnauthorizedStaMacTrap);		
	if ( NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %s",	TRAP_AC_OID, TRAP_CONTENT_OID,	stamac_str);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

#if 0
ac func
#endif
int ac_discovery_danger_ap_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_DANGER_AP
	cmd_test_out(acDiscoveryDangerAPTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	unsigned char mac[MAC_LEN];
	
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id=0;

	unsigned short rate;
	unsigned char chchannel;
	unsigned char rssi;
	char *essid=NULL;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &wtpindex,
								DBUS_TYPE_STRING, &wtpsn,
								DBUS_TYPE_BYTE, &wtpmac[0],
								DBUS_TYPE_BYTE, &wtpmac[1],
								DBUS_TYPE_BYTE, &wtpmac[2],
								DBUS_TYPE_BYTE, &wtpmac[3],
								DBUS_TYPE_BYTE, &wtpmac[4],
								DBUS_TYPE_BYTE, &wtpmac[5],
								DBUS_TYPE_STRING,&netid,
								DBUS_TYPE_UINT32,&instance_id, 
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_UINT16,&rate,
								DBUS_TYPE_BYTE,&chchannel,
								DBUS_TYPE_BYTE,&rssi,
								DBUS_TYPE_STRING,&essid,
								DBUS_TYPE_UINT32,&local_id, 
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,rate=%d, chchannel=%d, rssi=%d, essid=%s, "
						"wtpmac=%02X-%02X-%02X-%02X-%02X-%02X, mac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,rate,chchannel,rssi,essid,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5],mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;

	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acDiscoveryDangerAPTrap);
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;	

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	char atta_mac_str[MAC_STR_LEN];
	get_ap_mac_str( atta_mac_str, sizeof(atta_mac_str), mac);

	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_ESSID_TRAP_DES,			mac_oid, essid);
	trap_data_append_param_str(tData, "%s s %s",	EI_STA_MAC_TRAP_DES, 	  atta_mac_str);
	trap_data_append_param_str(tData, "%s%s s %d",	EI_ROGUE_STA_CHANNEL_MAC_TRAP_DES,	mac_oid, chchannel);
	trap_data_append_param_str(tData, "%s s %d",	EI_WTP_RSSI, rssi);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_cover_hole_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE
	cmd_test_out(wtpCoverholeTrap);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &wtpindex,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id, 
									DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpCoverholeTrap);
	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,		  mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 		  mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID, mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_cover_hole_clear_func(DBusMessage *message)
{
	//WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE_CLEAR
	cmd_test_out(wtpCoverHoleClearTrap);
	
	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &wtpindex,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id, 
									DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2010-10-20
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpCoverHoleClearTrap);
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);
	
	tData = trap_data_new_from_descr(tDescr);
	
	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);
	
	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);
	
	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int wtp_software_update_succeed_func ( DBusMessage *message )
{
//WID_DBUS_TRAP_WID_WTP_UPDATE_SUCCESSFUL
	cmd_test_out(wtpSoftWareUpdateSucceed);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &wtpindex,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id, 
									DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id= %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2011-2-18
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpSoftWareUpdateSucceed);
	if ( NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK; 

}

int wtp_software_update_failed_func ( DBusMessage *message )
{
//WID_DBUS_TRAP_WID_WTP_UPDATE_FAIL
	cmd_test_out(wtpSoftWareUpdateFailed);

	DBusError error;
	unsigned int wtpindex;
	char *wtpsn;
	unsigned char wtpmac[MAC_LEN];
	char *netid = "";
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
									DBUS_TYPE_UINT32, &wtpindex,
									DBUS_TYPE_STRING, &wtpsn,
									DBUS_TYPE_BYTE, &wtpmac[0],
									DBUS_TYPE_BYTE, &wtpmac[1],
									DBUS_TYPE_BYTE, &wtpmac[2],
									DBUS_TYPE_BYTE, &wtpmac[3],
									DBUS_TYPE_BYTE, &wtpmac[4],
									DBUS_TYPE_BYTE, &wtpmac[5],
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_UINT32,&instance_id, 
									DBUS_TYPE_UINT32,&local_id,
									DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, wtpindex=%d, wtpsn=%s, netid=%s, local_id = %d, instance_id=%d,wtpmac=%02X-%02X-%02X-%02X-%02X-%02X\n",
				dbus_message_get_member(message), wtpindex, wtpsn,netid, local_id, instance_id,
				wtpmac[0], wtpmac[1], wtpmac[2], wtpmac[3], wtpmac[4], wtpmac[5]);
	
	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) //add 2011-2-18
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, wtpSoftWareUpdateFailed);
	if ( NULL==tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id);

	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ap_mac_str(mac_str, sizeof(mac_str), wtpmac);

	char mac_oid[MAX_MAC_OID];
	get_ap_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
	
//	char netid[NETID_NAME_MAX_LENTH];	
//	get_ap_netid(netid, sizeof(netid), wtpindex);

	trap_data_append_param_str(tData, "%s s %s",	EI_MAC_TRAP_DES,			mac_str);
	trap_data_append_param_str(tData, "%s%s s %s",	EI_SN_TRAP_DES, 			mac_oid, wtpsn);
	trap_data_append_param_str(tData, "%s%s s %s",	WTP_NET_ELEMENT_CODE_OID,	mac_oid, netid);
	trap_data_append_common_param(tData, tDescr);

	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);

	TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id);
	
	//trap_data_destroy(tData);
	
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK; 

}


#if 0
// /var/run/softreboot 
int ac_reboot_func( DBusMessage *message)
{
	//"reboot"
	cmd_test_out( acSystemRebootTrap );
	
	DBusError error;
	unsigned char reboot_set =0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
										DBUS_TYPE_BYTE, &reboot_set,
										DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return 0;
	}
	trap_syslog(LOG_DEBUG, "Handling signal %s\n", "reboot");
	
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acSystemRebootTrap);
	if (NULL==tDescr || 0==tDescr->switch_status)
		return 0;
		
	tData = trap_data_new_from_descr(tDescr);		
	
	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname );
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
	
	trap_data_destroy(tData);
	
	return 0;
}
#endif
int ac_cpu_over_threshold_and_clear_func(DBusMessage *message)
{
	DBusError error;
	int type;
	unsigned int latest;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32,&latest,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, latest=%d\n", dbus_message_get_member(message), type, latest);

	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acCPUUtilizationOverThresholdTrap);
	else if (NOT_OVER_THRESHOLD == type)
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acCPUusageTooHighRecovTrap);

	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_cpu_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
        	
        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, 		mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_CPU_USAGE_TRAP_DES,mac_oid, latest);
        	trap_data_append_common_param(tData, tDescr);

            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_memory_over_threshold_and_clear_func(DBusMessage *message)
{
	DBusError error;
	int type;
	unsigned int latest;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32,&latest,
								DBUS_TYPE_INVALID)))
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, latest=%d\n", dbus_message_get_member(message), type, latest);
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acMemUtilizationOverThresholdTrap);
	else if (NOT_OVER_THRESHOLD == type)
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acMemUsageTooHighRecovTrap);

	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_memory_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_MEM_USAGE_TRAP_DES, mac_oid, latest);
        	trap_data_append_common_param(tData, tDescr);
        	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_temperature_over_threshold_and_clear_func(DBusMessage * message)
{
	unsigned int is_active_master = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}
	
	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in ac_temperature_over_threshold_and_clear_func fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

	DBusError error;
	int type;
	unsigned int latest;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32,&latest,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, latest=%d\n", dbus_message_get_member(message), type, latest);
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acTemperTooHighTrap);
	else if (NOT_OVER_THRESHOLD == type)
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acTemperTooHighRecovTrap);

	if (NULL == tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_temperature_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_TEMP_USAGE_TRAP_DES, mac_oid, latest);
        	trap_data_append_common_param(tData, tDescr);
	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_bandwith_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH
	cmd_test_out(acBandwithOverThresholdTrap);

	DBusError error;
	int type;
	unsigned int latest;
	unsigned int ifindex = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32, &ifindex,
								DBUS_TYPE_UINT32, &latest,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, interface_index=%d, latest=%d\n", 
							dbus_message_get_member(message), type, ifindex, latest);
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acBandwithOverThresholdTrap);
	else													
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;				//暂时没有清除trap

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_bandwith_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,   mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_BAND_USAGE_TRAP_DES, mac_oid, latest);
        	
        	trap_data_append_common_param(tData, tDescr);

            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_drop_rate_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE
	cmd_test_out(acLostPackRateOverThresholdTrap);

	DBusError error;
	int type;
	unsigned int ifindex = 0;
	unsigned int latest;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32, &ifindex,
								DBUS_TYPE_UINT32, &latest,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, latest=%d\n", dbus_message_get_member(message), type, latest);
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acLostPackRateOverThresholdTrap);
	else													
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;				//暂时没有清除trap

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_drop_rate_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}
        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,   mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_DROP_USAGE_TRAP_DES, mac_oid, latest);
        	
        	trap_data_append_common_param(tData, tDescr);
        	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_online_user_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MAX_USER
	cmd_test_out(acMaxUsrNumOverThresholdTrap);

	DBusError error;
	int eag_trap=EAG_TRAP;
	int ins_id = 0;
	int hansi_type = 0;
	int threshold_onlineusernum = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &eag_trap,
								DBUS_TYPE_INT32, &hansi_type,
								DBUS_TYPE_INT32, &ins_id,
								DBUS_TYPE_INT32, &threshold_onlineusernum,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, hansi_type=%d, ins_id=%d, threshold_onlineusernum=%d\n", dbus_message_get_member(message), eag_trap, hansi_type, ins_id, threshold_onlineusernum);
	
	if( !trap_instance_is_master(&gInsVrrpState, hansi_type, ins_id) )
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	
	if (EAG_TRAP == eag_trap) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acMaxUsrNumOverThresholdTrap);
	else if (EAG_TRAP_CLEAR == eag_trap) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acMaxUsrNumOverThresholdTrapClear);
	else													
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;				//暂时没有清除trap

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);
	
	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
	TrapData *tData = trap_data_new_from_descr(tDescr);
	if(NULL == tData) {
        trap_syslog(LOG_INFO, "ac_online_user_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;;
	}

	char mac_str[MAC_STR_LEN];
	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

	char mac_oid[MAX_MAC_OID];
	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,   		 mac_oid, gSysInfo.hostname);
	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_USER_ONLINE_USAGE_TRAP_DES, mac_oid, threshold_onlineusernum);
	
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(gInsVrrpState.instance[hansi_type][ins_id].receivelist, &gV3UserList, tData);
	TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, hansi_type, ins_id); 
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_auth_fail_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ
	cmd_test_out(acAuthSucRateBelowThresholdTrap);

	DBusError error;
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	unsigned int access_accept_rate = 0;
	unsigned int threshold = 0;
	unsigned int type = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
                                DBUS_TYPE_INT32,  &type,
								DBUS_TYPE_UINT32, &local_id,
								DBUS_TYPE_UINT32, &instance_id,
                                DBUS_TYPE_UINT32, &access_accept_rate,
                                DBUS_TYPE_UINT32, &threshold,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, local_id = %d, instance = %d, access_accept_rate = %d, threshold = %d, type=%d\n", 
	                        dbus_message_get_member(message), local_id, instance_id, access_accept_rate, threshold, type);


	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) ) 
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acAuthSucRateBelowThresholdTrap);
	else													
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;				//暂时没有清除trap

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
	TrapData *tData = trap_data_new_from_descr(tDescr);
	if(NULL == tData) {
        trap_syslog(LOG_INFO, "ac_auth_fail_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;
	}

	char mac_str[MAC_STR_LEN];
	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

	char mac_oid[MAX_MAC_OID];
	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,   		   mac_oid, gSysInfo.hostname);
	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_RADIUS_REQUEST_RATE_TRAP_DES, mac_oid, access_accept_rate);
	
	trap_data_append_common_param(tData, tDescr);
	
    trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
    TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, local_id, instance_id); 
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_ip_pool_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL
	cmd_test_out(acAveIPPoolOverThresholdTrap);

	DBusError error;
	int type;
	unsigned int latest;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32,&latest,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, latest=%d\n", dbus_message_get_member(message), type, latest);
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acAveIPPoolOverThresholdTrap);
	else													
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;				//暂时没有清除trap

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_ip_pool_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,    mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_DHCP_USAGE_TRAP_DES,  mac_oid, latest);
        	
        	trap_data_append_common_param(tData, tDescr);

            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_portal_reach_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTER_REQ
	cmd_test_out(acPortalServerNotReachTrap);

	DBusError error;
	int type;
	char *cur_ip = "";
	short port = 0;
	unsigned int local_id = 0;
	unsigned int instance_id = 0;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &type,
								DBUS_TYPE_STRING, &cur_ip,
								DBUS_TYPE_UINT16, &port,
								DBUS_TYPE_UINT32, &local_id,
								DBUS_TYPE_UINT32, &instance_id,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type = %d, ip:port = %s:%d, local_id = %d, instance_id = %d\n", 
	                        dbus_message_get_member(message), type, cur_ip, port, local_id, instance_id);

	if( !trap_is_ap_trap_enabled(&gInsVrrpState, local_id, instance_id) )
		return TRAP_SIGNAL_HANDLE_HANSI_BACKUP;
	
	TrapDescr *tDescr = NULL;

	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acPortalServerNotReachTrap);
	else if (NOT_OVER_THRESHOLD == type)												
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acPortalServerAvailableTrap);

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);

	TrapData *tData = trap_data_new_from_descr(tDescr);
	if(NULL == tData) {
        trap_syslog(LOG_INFO, "ac_portal_reach_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;
	}

	char mac_str[MAC_STR_LEN];
	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

	char mac_oid[MAX_MAC_OID];
	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,    mac_oid, gSysInfo.hostname);
	trap_data_append_param_str(tData, "%s s %s:%d",   EI_AC_PORTAL_URL,    cur_ip, port);
	
	trap_data_append_common_param(tData, tDescr);
	
    trap_send(gInsVrrpState.instance[local_id][instance_id].receivelist, &gV3UserList, tData);
    TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, local_id, instance_id); 
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;	
}

int ac_dhcp_exhaust_over_threshold_and_clear_func(DBusMessage * message)
{
	//"dhcp_notify_web_pool_event"
	cmd_test_out(acDHCPAddressExhaustTrap);

	DBusError error;
	unsigned int flags=0; 
	unsigned int not_used=0;
	unsigned int total=0;
	char *poolname="";
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &flags,
								DBUS_TYPE_UINT32, &total,  
								DBUS_TYPE_UINT32, &not_used,  
								DBUS_TYPE_STRING, &poolname, 
								DBUS_TYPE_INVALID)) ) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, flags=%d, total=%d, not_used=%d, poolname=%s\n", 
							dbus_message_get_member(message), flags, total, not_used, poolname );
	
	TrapDescr *tDescr = NULL;
	
	if (1==flags) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acDHCPAddressExhaustTrap);
	else if (0==flags)												
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acDHCPAddressExhaustRecovTrap);

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_dhcp_exhaust_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,    mac_oid, gSysInfo.hostname);
        	trap_data_append_param_str(tData, "%s%s s %d", EI_AC_DHCP_USAGE_TRAP_DES,  mac_oid, not_used);
        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_DHCP_IP_POOL_NAME_DES,poolname);
        	
        	trap_data_append_common_param(tData, tDescr);

            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_find_attack_over_threshold_and_clear_func(DBusMessage * message)
{
	//AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK
	cmd_test_out(acFindAttackTrap);

	DBusError error;
	int type;
	unsigned int latest;
	
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_INT32, &type,
								DBUS_TYPE_UINT32,&latest,
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}

	trap_syslog(LOG_INFO, "Handling signal %s, type=%d, latest=%d\n", dbus_message_get_member(message), type, latest);
	
	TrapDescr *tDescr = NULL;
	
	if (OVER_THRESHOLD_FLAG == type) 
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acFindAttackTrap);
	else													
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;				//暂时没有清除trap

	if (NULL==tDescr || 0==tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_find_attack_over_threshold_and_clear_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);

        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s",   EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID,    mac_oid, gSysInfo.hostname);
        	
        	trap_data_append_common_param(tData, tDescr);
	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int ac_system_start_func(DBusMessage *message)
{
	cmd_test_out(acSystemRebootTrap);

	unsigned int is_active_master = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}

	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in ac_system_start_func fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

	if(1 == ac_trap_get_flag("/var/run/ac_restart_trap_flag"))
	{
		trap_syslog(LOG_INFO,"ac_system_start\n");
		return -1;
	}

	DBusError error;
	unsigned int system_state = 0;
		
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32, &system_state,
								DBUS_TYPE_INVALID)) )
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, system_state = %d\n", dbus_message_get_member(message), system_state);
	
	if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
	
	TrapDescr *tDescr = NULL;

	if ( SYSTEM_READY == system_state )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acSysColdStartTrap);
	}
	else if ( SYSTEM_REBOOT == system_state )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acSystemRebootTrap);	
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	INCREASE_TIMES(tDescr);

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
	int i = 0, j = 0;
	for(i = 0; i < VRRP_TYPE_NUM; i++) {
		for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {	
			TrapData *tData = trap_data_new_from_descr(tDescr);
			if(NULL == tData) {
				trap_syslog(LOG_INFO, "ac_system_start_func: trap_data_new_from_descr malloc tData error!\n");
				continue;
			}

			char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
        	
        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
        	trap_data_append_common_param(tData, tDescr);
        	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
		}
	}
	

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
/*dongt*/
int ac_insert_extract_func(DBusMessage *message)
{
	cmd_test_out(acBoardExtractTrap);

	unsigned int is_active_master = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}

	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in ac_insert_extract_func fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

	if(1 == ac_trap_get_flag("/var/run/ac_restart_trap_flag"))
	{
		trap_syslog(LOG_INFO,"ac_insert_extract_func\n");
		return -1;
	}

	unsigned int board_id=0;
	unsigned int slot_state=0;

	char str_reason[50];
	memset(str_reason,0, sizeof(str_reason));

	DBusError error;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32,&slot_state,//板卡状态0--extract, 1--insert
								DBUS_TYPE_UINT32, &board_id,//槽号
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, slot_state = %d\n", dbus_message_get_member(message), slot_state);
	
	if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
	
	TrapDescr *tDescr = NULL;
	if ( 0 == slot_state )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acBoardExtractTrap);
		sprintf(str_reason,"slot_%d_is_pulled_out\n",board_id);
	}
	else if ( 1 == slot_state )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acBoardInsertTrap);
		sprintf(str_reason,"slot_%d_is_pushed_in\n",board_id);
	}
	if((tDescr == NULL) || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;
	
	INCREASE_TIMES(tDescr);
	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);

	int i = 0,j = 0;
	for(i = 0; i < VRRP_TYPE_NUM; i++)
		for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++)
		{
			TrapData *tData = trap_data_new_from_descr(tDescr);
			if(NULL == tData)
			{
				trap_syslog(LOG_INFO, "ac_insert_extract_func: trap_data_new_from_descr malloc tData error!\n");
				continue;
			}
			
			char mac_str[MAC_STR_LEN]={0};
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
        	
        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);


			trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
			trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, str_reason);
			
        	trap_data_append_common_param(tData, tDescr);
			
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 

			break;
		}
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
int ac_port_down_func(DBusMessage *message)
{

	cmd_test_out(acPortDownTrap);

	unsigned int is_active_master = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}

	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in ac_port_down_func fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

	if(1 == ac_trap_get_flag("/var/run/ac_restart_trap_flag"))
	{
		trap_syslog(LOG_INFO,"ac_port_down_func\n");
		return -1;
	}

	char str_reason[50];
	memset(str_reason,0, sizeof(str_reason));

	
	unsigned int port_state;
	unsigned int board_id;
	unsigned int port_id;
	DBusError error;
	dbus_error_init(&error);
	if (!(dbus_message_get_args(message, &error,
								DBUS_TYPE_UINT32,&port_state,//0--down,1--up
								DBUS_TYPE_UINT32, &board_id,
								DBUS_TYPE_UINT32, &port_id,
								
								DBUS_TYPE_INVALID))) 
	{
		trap_syslog(LOG_WARNING, "Get args failed, %s, %s\n", dbus_message_get_member(message), error.message);
		dbus_error_free(&error);
		return TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR;
	}
	trap_syslog(LOG_INFO, "Handling signal %s, port_state = %d\n", dbus_message_get_member(message), port_state);
	
	if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
	
	TrapDescr *tDescr = NULL;
	if ( 0 == port_state)
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acPortDownTrap);
		sprintf(str_reason,"port:%d-%d_is_down\n",board_id,port_id);
	}	
	else if ( 1 == port_state )
	{
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acPortUpTrap);
		sprintf(str_reason,"port:%d-%d_is_up\n",board_id,port_id);
	}

	INCREASE_TIMES(tDescr);
	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);

	int i = 0,j = 0;
	for(i = 0; i < VRRP_TYPE_NUM; i++)
		for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++)
		{
			TrapData *tData = trap_data_new_from_descr(tDescr);
			if(NULL == tData)
			{
				trap_syslog(LOG_INFO, "ac_port_down_func: trap_data_new_from_descr malloc tData error!\n");
				continue;
			}
			
			char mac_str[MAC_STR_LEN]={0};
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
        	
        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);


			trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
			trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, str_reason);

        	trap_data_append_common_param(tData, tDescr);
			
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 

			break;
		}

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
void trap_signal_register_all(TrapList *list, hashtable *ht)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	trap_signal_list_init(list);
	TrapSignal *tSignal_tmp = NULL;
	//ap
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AP_REBOOT , 				wtp_sys_start_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AP_DOWN , 				wtp_down_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_IP_CHANGE_ALARM, 			wtp_ip_change_alarm_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_CHANGE, 			wtp_channel_modified_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_TRANFER_FILE, 			wtp_file_transfer_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_ELECTRIFY_REGISTER_CIRCLE,wtp_electrify_register_circle_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_ENTER_IMAGEDATA_STATE, 	wtp_ap_update_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CODE_START, 				wtp_cold_start_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WLAN_ENCRYPTION_TYPE_CHANGE, 	wtp_auth_mode_change_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WLAN_PRESHARED_KEY_CHANGE, 	wtp_preshared_key_change_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AP_FLASH_WRITE_FAIL,		wtp_flash_write_fail_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_DIVORCE_NETWORK,			ap_divorce_network_func);			
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE,		ap_actimesynchrofailure_func);			
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_LEAVE,							th_sta_leave_func);		
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_LEAVE_ABNORMAL,			    th_sta_leave_abnormal_func);		
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_ASSOC_FAILED,					th_sta_assoc_fail_func);		
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_JIANQUAN_FAILED,				th_sta_jianquan_fail_func);		
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_SSID_KEY_CONFLICT,			wid_ssid_key_conflict_func);			
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_COME,							th_sta_come_func);		
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_WTP_DENY_STA,						th_wtp_deny_sta_func);		
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_DE_WTP_DENY_STA,					th_de_wtp_deny_sta_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_WAPI_TRAP,							asd_wapi_trap_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_VERIFY,						asd_sta_verify_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_STA_VERIFY_FAILED,					asd_sta_verify_failed_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, "eag_send_trap_user_logoff_abnormal",			eag_user_logoff_abnormal);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	//tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_RADIUS_CONNECT_FAILED,				asd_radius_connect_failed_func);
	//tSignal_tmp=trap_signal_list_register(list, ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN,		asd_radius_connect_failed_clean_func);

	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN,	wtp_wireless_interface_down_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN_CLEAR,wtp_wireless_interface_down_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP,		route_method_notify_snmp_by_ip_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP,		route_method_notify_snmp_by_vrrp_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE,wtp_channel_device_interference_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE, 	wtp_channel_ap_interference_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE, wtp_channel_terminal_interference_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR, 		wtp_channel_count_minor_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE_CLEAR, wtp_channel_device_interference_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE_CLEAR, wtp_channel_ap_interference_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE_CLEAR, wtp_channel_terminal_interference_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR_CLEAR, wtp_channel_count_minor_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD, 			ap_cpu_threshold_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD, 			ap_mem_threshold_func);	
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD, 			ap_temp_threshold_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR, 			ap_wifi_if_error_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_ATH_ERROR, 				ap_ath_if_error_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_RRM_STATE_CHANGE, 			ap_mt_work_mode_chg_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, "wid_dbus_trap_power_state_change", 			power_state_change);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_RUN_QUIT, 					ap_run_quit_state);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_FIND_UNSAFE_ESSID, 		ap_find_unsafe_essid);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_FIND_WIDS_ATTACK, 		wtp_find_wids_attack);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_NEIGHBOR_CHANNEL_AP_INTERFERENCE, ap_neighbor_channel_interfere_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, "wid_dbus_trap_wtp_configure_error", wtp_configuration_error_trap);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, "wid_dbus_trap_sta_flow_rx_tx_overflow", wtp_user_traffic_overload_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, "wid_dbus_trap_wtp_sta_unauthroized_mac", wtp_unauthorized_Station_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	//ac
	//tSignal_tmp=trap_signal_list_register(list, "reboot", 										ac_reboot_func);
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_DANGER_AP, 	ac_discovery_danger_ap_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE, 	wtp_cover_hole_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE_CLEAR, wtp_cover_hole_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_UPDATE_SUCCESSFUL, wtp_software_update_succeed_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_UPDATE_FAIL, wtp_software_update_failed_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	/*wangchao add*/
	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_LTE_FI_RUN_QUIT, ap_lte_run_quit_state);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);	

	tSignal_tmp=trap_signal_list_register(list, WID_DBUS_TRAP_WID_LTE_FI_UPLINK_SWITCH, wid_dbus_trap_wid_lte_fi_uplink_switch);

	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);		

/*ac*/	
	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU, 			ac_cpu_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE, 		ac_memory_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE, 	ac_temperature_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH, 		ac_bandwith_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE, 		ac_drop_rate_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, EAG_ONLINE_USER_NUM_THRESHOLD_SIGNAL, 			ac_online_user_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ, 	ac_auth_fail_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL, 		ac_ip_pool_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH, 	ac_portal_reach_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH, 	ac_radius_auth_reach_status_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC, 	ac_radius_acct_reach_status_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, "dhcp_notify_web_pool_event", 					ac_dhcp_exhaust_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK,ac_find_attack_over_threshold_and_clear_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, SEM_TRAP_SYSTEM_STATE,							ac_system_start_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, SEM_TRAP_BOARD_STATE,							ac_insert_extract_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, SEM_TRAP_PORT_STATE,							ac_port_down_func);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	TRAP_SHOW_HASHTABLE(ht, tSignal_tmp, signal_hash_node ,signal_name);
}


void trap_signal_register_proxy(TrapList *list, hashtable *ht)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	trap_signal_list_init(list);
	TrapSignal *tSignal_tmp = NULL;
	
#if 0

	tSignal_tmp=trap_signal_list_register(list, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP,		    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU, 			    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE, 		    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE, 	    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH, 		    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE, 		    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MAX_USER, 		    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK,    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
#endif
    
	tSignal_tmp=trap_signal_list_register(list, "wid_dbus_trap_power_state_change", 			    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
#if 0
	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ, 	    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
#endif
	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL, 		    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH, 	    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH, 	    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC, 	    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);
	
	tSignal_tmp=trap_signal_list_register(list, "dhcp_notify_web_pool_event", 					    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, "set_boot_img_failed_by_mib", 					    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);

	tSignal_tmp=trap_signal_list_register(list, "config_file_error_by_mib", 					    NULL);
	INIT_SIGNAL_HASH_LIST(tSignal_tmp,ht);


	TRAP_SHOW_HASHTABLE(ht, tSignal_tmp, signal_hash_node ,signal_name);
}


void trap_descr_register_all(TrapList *list, hashtable *ht)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	TrapDescr *descr_tmp=NULL;
	
	trap_descr_list_init(list);
	
//ap inner
//0.1
	descr_tmp=trap_descr_list_register(list, WTPRESTART, wtpDownTrap, TRAP_SRC_AP,  ".0.1",TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "ap_power_off", "ap_power_off");
	//TRAP_TRACE_LOG(LOG_INFO,"++++++++++++TrapDescr->trap_name=%s strlen(descr_tmp->trap_name)=%d\n",descr_tmp->trap_name,strlen(descr_tmp->trap_name));
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, WTPRESTART, wtpSysStartTrap,  TRAP_SRC_AP, ".0.2", TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "ap_reboot", "ap_reboot");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPCHANNELMODIFIED, wtpChannelModifiedTrap,  TRAP_SRC_AP, ".0.3", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR,	"WTP_CHANNEL_CHANGE", "WTP_CHANNEL_CHANGE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPIPCHANGE, wtpIPChangeAlarmTrap,  TRAP_SRC_AP, ".0.4", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "wtp_ip_change_alarm", "wtp_ip_change_alarm");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPFLASHWRITEFAILORWTPUPDATE, wtpFlashWriteFailTrap, TRAP_SRC_AP, ".0.5", TRAP_TYPE_PROCESS_ERR, 
									TRAP_LEVEL_SECONDARY, "ap_flash_write_failure", "ap_flash_write_failure");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPRESTART,  wtpColdStartTrap, TRAP_SRC_AP, ".0.6", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "wtp_cold_reboot", "wtp_cold_reboot");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPAUTHMODECHANGE, wtpAuthModeChangeTrap, TRAP_SRC_AP,  ".0.7",TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "WLAN_ENCRYPTION_TYPE_CHANGE", "WLAN_ENCRYPTION_TYPE_CHANGE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPPRESHAREDKEYCHANGE, wtpPreSharedKeyChangeTrap,  TRAP_SRC_AP, ".0.8", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "WLAN_PRESHARED_KEY_CHANGE", "WLAN_PRESHARED_KEY_CHANGE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPELECTRIFYREGISTECIRECLE, wtpElectrifyRegisterCircleTrap,  TRAP_SRC_AP, ".0.9", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR,	"ELECTRIFY_REGISTER", "ELECTRIFY_REGISTER");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPFLASHWRITEFAILORWTPUPDATE, wtpAPUpdateTrap,  TRAP_SRC_AP, ".0.10", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "ap_version_update_begin", "ap_version_update_begin");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

//0.10
	descr_tmp=trap_descr_list_register(list, WTPCOVERHOLE, wtpCoverholeTrap, TRAP_SRC_AP, ".0.11", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wtp_discovery_cover_hole", "wtp_discovery_cover_hole");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPCPUUSAGETOOHIGH, CPUusageTooHighTrap, TRAP_SRC_AP,  ".0.13",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_cpu_over_threshold", "wid_ap_cpu_over_threshold");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPCPUUSAGETOOHIGH, CPUusageTooHighRecovTrap,  TRAP_SRC_AP, ".0.14", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_cpu_over_threshold_clear", "wid_ap_cpu_over_threshold_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPMEMUSAGETOOHIGH, MemUsageTooHighTrap,  TRAP_SRC_AP, ".0.15", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR,	"wid_ap_mem_over_threshold", "wid_ap_mem_over_threshold");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPMEMUSAGETOOHIGH, MemUsageTooHighRecovTrap,  TRAP_SRC_AP, ".0.16", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_mem_over_threshold_clear", "wid_ap_mem_over_threshold_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPTEMPERTOOHIGH, TemperTooHighTrap, TRAP_SRC_AP, ".0.17", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "AP_temperature_is_too_high", "AP_temperature_is_too_high");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPTEMPERTOOHIGH, TemperTooHighRecoverTrap, TRAP_SRC_AP, ".0.18", 	TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "AP_temperature_is_recover", "AP_temperature_is_recover");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPMTWORKMODECHANGE, APMtWorkModeChgTrap, TRAP_SRC_AP,  ".0.19",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_rrm_state_change", "wid_ap_rrm_state_change");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//0.21
	descr_tmp=trap_descr_list_register(list, WTPSSIDkEYCONFLICT, SSIDkeyConflictTrap,  TRAP_SRC_AP, ".0.21", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "station_key_conflict", "station_key_conflict");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPONLINEOFFLINE, wtpOnlineTrap,  TRAP_SRC_AP, ".0.22", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC,	"ap_is_running", "ap_is_running");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPONLINEOFFLINE, wtpOfflineTrap,  TRAP_SRC_AP, ".0.23", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "AP_is_quit", "AP_is_quit");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPCOVERHOLE, wtpCoverHoleClearTrap, TRAP_SRC_AP, ".0.24", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wtp_repair_cover_hole", "wtp_repair_cover_hole");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, WTPFLASHWRITEFAILORWTPUPDATE, wtpSoftWareUpdateSucceed, TRAP_SRC_AP, ".0.25", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "ap_software_update_succeed", "ap_software_update_succeed");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, WTPFLASHWRITEFAILORWTPUPDATE, wtpSoftWareUpdateFailed, TRAP_SRC_AP, ".0.29", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "ap_software_update_failed", "ap_software_update_failed");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, WTPCONFIGERROR, wtpConfigurationErrorTrap, TRAP_SRC_AP, ".0.30", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wtp_configuration_error", "wtp_configuration_error");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	/*wangchao*/
	descr_tmp=trap_descr_list_register(list, LTEFIQUITREASON, ReservedReason, TRAP_SRC_AP, ".0.31", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "ReservedReason", "reserved_reason_of_Lte_Fi_quit");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, LTEFIQUITREASON, ACCTimeout, TRAP_SRC_AP, ".0.32", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "ACCTimeout", "ACCTimeout_resulted_in_Lte_Fi_quit");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );	
	
	descr_tmp=trap_descr_list_register(list, LTEFIQUITREASON, ExtendReason, TRAP_SRC_AP, ".0.33", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "ExtendReason", "extend_reason_of_Lte_Fi_quit");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, LTEUPLINKSWITCH, LteFiUplinkSwitch, TRAP_SRC_AP, ".0.34", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "Lte_uplink_switch", "Lte_uplink_switch");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );	
	
//ap app
//1.1
	descr_tmp=trap_descr_list_register( list, WTPCHANNELOBSTRUCT, wtpChannelObstructionTrap, TRAP_SRC_AP, ".1.1", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "wtp_channel_obstruction", "wtp_channel_obstruction");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPINTERFERENCEDETECTED, wtpAPInterferenceDetectedTrap, TRAP_SRC_AP,  ".1.2",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_interfere_detected", "ap_interfere_detected");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTAINTERFERENCEDETECTED, wtpStaInterferenceDetectedTrap,  TRAP_SRC_AP, ".1.3", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "wtp_sta_interference_detected", "wtp_sta_interference_detected");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPOTHERDEVICEINTERFERENCE, wtpDeviceInterferenceDetectedTrap,  TRAP_SRC_AP, ".1.4", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY,	"wtp_device_interference_detected", "wtp_device_interference_detected");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSUBSCRIBERDATABASEFULL, wtpSubscriberDatabaseFullTrap,  TRAP_SRC_AP, ".1.5", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL,	"wtp_deny_sta", "wtp_deny_sta");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPDFSFREECOUNTBELOWTHRESHOLD, wtpDFSFreeCountBelowThresholdTrap,  TRAP_SRC_AP, ".1.6", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL,	"channel_count_minor", "channel_count_minor");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPFLASHWRITEFAILORWTPUPDATE, wtpFileTransferTrap,  TRAP_SRC_AP, ".1.7", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "WTP_TRANFER_FILE", "WTP_TRANFER_FILE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTATIONOFFLINE, wtpStationOffLineTrap, TRAP_SRC_AP, ".1.8", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "station_leave", "station_leave");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTATIONOFFLINEABNORMAL, wtpStationOffLineAbnormalTrap, TRAP_SRC_AP, ".1.44", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "station_leave_abnormal", "station_leave_abnormal");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPUSERLOGOOFFABNORMAL, wtpUserLogoffAbnormalTrap, TRAP_SRC_AP, ".1.45", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "user_logoff_abnormal", "user_logoff_abnormal");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPLINKVARIFYFAILED, wtpSolveLinkVarifiedTrap, TRAP_SRC_AP, ".1.9", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_station_verify", "asd_station_verify");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPLINKVARIFYFAILED, wtpLinkVarifyFailedTrap, TRAP_SRC_AP,  ".1.10",TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_station_verify_failed", "asd_station_verify_failed");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//1.11
	descr_tmp=trap_descr_list_register(list, WTPSTATIONOFFLINE, wtpStationOnLineTrap,  TRAP_SRC_AP, ".1.11", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "station_come", "station_come");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPINTERFERENCEDETECTED, APInterfClearTrap,  TRAP_SRC_AP, ".1.12", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY,	"ap_interfere_detected_clear", "ap_interfere_detected_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTAINTERFERENCEDETECTED, APStaInterfClearTrap,  TRAP_SRC_AP, ".1.13", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_sta_interf_clear", "ap_sta_interf_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPOTHERDEVICEINTERFERENCE, APOtherDevInterfClearTrap, TRAP_SRC_AP, ".1.15", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "ap_other_dev_interf_clear", "ap_other_dev_interf_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPMODULETROUBLE, APModuleTroubleTrap, TRAP_SRC_AP, ".1.16", 	TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_wifi_interface_error", "wid_ap_wifi_interface_error");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPMODULETROUBLE, APModuleTroubleClearTrap, TRAP_SRC_AP,  ".1.17",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_wifi_interface_error_clear", "wid_ap_wifi_interface_error_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPRADIODOWN,  APRadioDownTrap,  TRAP_SRC_AP, ".1.18", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_ath_module_error", "wid_ap_ath_module_error");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPRADIODOWN, APRadioDownRecovTrap,  TRAP_SRC_AP, ".1.19", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC,	"wid_ap_ath_module_error_clear", "wid_ap_ath_module_error_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//1.21
	descr_tmp=trap_descr_list_register(list, WTPSTAAUTHERROR, APStaAuthErrorTrap,  TRAP_SRC_AP, ".1.21", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "station_auth_fail", "station_auth_fail");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTAASSOCIATEFAIL, APStAssociationFailTrap, TRAP_SRC_AP, ".1.22", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "station_assoc_fail", "station_assoc_fail");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTAWITHINVALIDCERINBREAKNET, APUserWithInvalidCerInbreakNetTrap, TRAP_SRC_AP, ".1.23", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "Invalid_certification_user_attack", "Invalid_certification_user_attack");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPSTAREPITITIVEATTACK, APStationRepititiveAttackTrap, TRAP_SRC_AP,  ".1.24",TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "Client_Re-attack", "Client_Re-attack");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPTAMPERATTACK, APTamperAttackTrap,  TRAP_SRC_AP, ".1.25", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "tamper_attack", "tamper_attack");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPLOWSAFELEVELATTACK, APLowSafeLevelAttackTrap,  TRAP_SRC_AP, ".1.26", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR,	"low_safe_level_attack", "low_safe_level_attack");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPADDRESSREDIRECTATTACK, APAddressRedirectionAttackTrap,  TRAP_SRC_AP, ".1.27", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "address_redirect_attack", "address_redirect_attack");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//1.31
	descr_tmp=trap_descr_list_register(list, WTPADDUSERFAIL, APAddUserFailClearTrap,  TRAP_SRC_AP, ".1.37", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL, "wtp_deny_sta_clear", "wtp_deny_sta_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPCHANNELTOOLOWCLEAR, APChannelTooLowClearTrap,  TRAP_SRC_AP, ".1.38", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL, "channel_minor_clear", "channel_minor_clear");	
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPFINDUNSAFEESSID, APFindUnsafeESSID,  TRAP_SRC_AP, ".1.39", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_find_unsafe_essid", "wid_ap_find_unsafe_essid");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPFINDSYNATTACK, APFindSYNAttack,  TRAP_SRC_AP, ".1.40", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_dbus_trap_wtp_find_wids_attack", "wid_dbus_trap_wtp_find_wids_attack");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPNEIGHBORCHANNELINTER, ApNeighborChannelInterfTrap,  TRAP_SRC_AP, ".1.41", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_neighbor_channel_interfere", "ap_neighbor_channel_interfere");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPNEIGHBORCHANNELINTER, ApNeighborChannelInterfTrapClear,  TRAP_SRC_AP, ".1.42", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_neighbor_channel_interfere_clear", "ap_neighbor_channel_interfere_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, WTPUSERTAFFICOVERLOAD, wtpUserTrafficOverloadTrap,  TRAP_SRC_AP, ".1.51", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "wtp_userTraffic_overload", "wtp_userTraffic_overload");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, WTPUNAUTHORIZEDSTAMACTRAP, wtpUnauthorizedStaMacTrap,  TRAP_SRC_AP, ".1.52", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "wtp_Unauthorized_Station", "wtp_unauthorized_station");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//ac inner
//0.1
	descr_tmp=trap_descr_list_register(list, ACRESTART, acSystemRebootTrap, TRAP_SRC_AC, ".0.1", TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "ac_reboot", "ac_reboot");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACAPTIMESYNCHROFAIL, acAPACTimeSynchroFailureTrap, TRAP_SRC_AC, ".0.2", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "wtp_ap_actimesynchrofailure", "wtp_ap_actimesynchrofailure");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACCHANGEDIP, acChangedIPTrap, TRAP_SRC_AC, ".0.3", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "AC_CHANGED_IP", "AC_CHANGED_IP");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACMASTERANDSTANDBYSWITCH, acTurntoBackupDeviceTrap, TRAP_SRC_AC, ".0.4", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_CRITIC, "AC_TURNTO_BACK_BY_VRRP", "AC_TURNTO_BACK_BY_VRRP");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, ACMASTERANDSTANDBYSWITCH, acTurntoMasterDeviceTrap, TRAP_SRC_AC, ".0.4", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_CRITIC, "AC_TURNTO_MASTER_BY_VRRP", "AC_TURNTO_MASTER_BY_VRRP");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACCONFIGERROR, acConfigurationErrorTrap, TRAP_SRC_AC, ".0.5", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "AC_PARSE_CONFIGURE_ERROR", "AC_PARSE_CONFIGURE_ERROR");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACRESTART, acSysColdStartTrap, TRAP_SRC_AC, ".0.6", TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "AC_COLD_START", "AC_COLD_START");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//0.22
	descr_tmp=trap_descr_list_register(list, ACAPTIMESYNCHROFAIL, acAPACTimeSynchroFailureTrapClear, TRAP_SRC_AC, ".0.22", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "wtp_ap_actimesynchrofailure_clear", "wtp_ap_actimesynchrofailure_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACBOARDPULLOUT, acBoardExtractTrap, TRAP_SRC_AC, ".0.24", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "ac_board_extract", "ac_board_extract");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );	

	descr_tmp=trap_descr_list_register(list, ACBOARDPULLOUT, acBoardInsertTrap, TRAP_SRC_AC, ".0.25", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "ac_board_insert", "ac_board_insert");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );	
	
	descr_tmp=trap_descr_list_register(list, ACPORTDOWN, acPortUpTrap, TRAP_SRC_AC, ".0.26", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "ac_port_up", "ac_port_up");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );	
	
	descr_tmp=trap_descr_list_register(list, ACPORTDOWN, acPortDownTrap, TRAP_SRC_AC, ".0.27", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "ac_port_down", "ac_port_down");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );	
	
// ac app
//1.1
	descr_tmp=trap_descr_list_register(list, ACDISCOVERDANGERAP, acDiscoveryDangerAPTrap, TRAP_SRC_AC, ".1.1", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ac_discovery_danger_ap", "ac_discovery_danger_ap");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACRADIUSAUTHSERVERNOTREACH, acRadiusAuthenticationServerNotReachTrap, TRAP_SRC_AC, ".1.2", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_auth_radius_connect_failed", "asd_auth_radius_connect_failed");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACRADIUSACCOUNTSERVERNOTREACH, acRadiusAccountServerNotLinkTrap, TRAP_SRC_AC, ".1.3", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_account_radius_connect_failed", "asd_account_radius_connect_failed");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACPORTALSERVERNOTREACH, acPortalServerNotReachTrap, TRAP_SRC_AC, ".1.4", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "Portal_Server_can_not_reached", "Portal_Server_can_not_reached");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACAPLOSTNET, acAPLostNetTrap, TRAP_SRC_AC, ".1.5", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "wid_set_wtp_divorce_network", "wid_set_wtp_divorce_network");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACCPUUTILIZATIONOVERTHRESHOLD, acCPUUtilizationOverThresholdTrap, TRAP_SRC_AC, ".1.6", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "CPU_RATE_OVER_THRESHOLD_TRAP", "CPU_RATE_OVER_THRESHOLD_TRAP");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACMEMUTILIZATIONOVERTHRESHOLD, acMemUtilizationOverThresholdTrap, TRAP_SRC_AC, ".1.7", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MEM_RATE_OVER_THRESHOLD_TRAP", "MEM_RATE_OVER_THRESHOLD_TRAP");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACBANDWITHOVERTHRESHOLD, acBandwithOverThresholdTrap, TRAP_SRC_AC, ".1.8", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "BANDWITH_RATE_OVER_THRESHOLD_VALUE", "BANDWITH_RATE_OVER_THRESHOLD_VALUE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACLOSTPACKRATEOVERTHRESHOLD, acLostPackRateOverThresholdTrap, TRAP_SRC_AC, ".1.9", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "DROP_RATE_OVER_THRESHOLD_VALUE", "DROP_RATE_OVER_THRESHOLD_VALUE");		
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACMAXUSRNUMOVERTHRESHOLD, acMaxUsrNumOverThresholdTrap, TRAP_SRC_AC, ".1.10", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MAX_ONLINE_USER_OVER_THRESHOLD_VALUE", "MAX_ONLINE_USER_OVER_THRESHOLD_VALUE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	descr_tmp=trap_descr_list_register(list, ACMAXUSRNUMOVERTHRESHOLD, acMaxUsrNumOverThresholdTrapClear, TRAP_SRC_AC, ".1.30", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MAX_ONLINE_USER_OVER_THRESHOLD_VALUE_CLEAR", "MAX_ONLINE_USER_OVER_THRESHOLD_VALUE_CLEAR");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//1.11
	descr_tmp=trap_descr_list_register(list, ACAUTHSUCCRATEBELOWTHRESHOLD, acAuthSucRateBelowThresholdTrap, TRAP_SRC_AC, ".1.14", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "AUTH_SUC_RATE_OVER_THRESHOLD_VALUE", "AUTH_SUC_RATE_OVER_THRESHOLD_VALUE");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACAVEIPPOOLOVERTHRESHOLD, acAveIPPoolOverThresholdTrap, TRAP_SRC_AC, ".1.15", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "IP_POOL_AVE_RATE_OVER_THRESHOLD", "IP_POOL_AVE_RATE_OVER_THRESHOLD");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACMAXIPPOOLOVERTHRESHOLD, acMaxIPPoolOverThresholdTrap, TRAP_SRC_AC, ".1.16", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "IP_POOL_MAX_OVER_THRESHOLD", "IP_POOL_MAX_OVER_THRESHOLD");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACPOWEROFF, acPowerOffTrap, TRAP_SRC_AC, ".1.18", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_power_state_change", "wid_power_state_change");	
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACPOWEROFF, acPowerOffRecovTrap, TRAP_SRC_AC, ".1.19", 	TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_power_state_change_clear", "wid_power_state_change_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACCPUUTILIZATIONOVERTHRESHOLD, acCPUusageTooHighRecovTrap, TRAP_SRC_AC, ".1.20", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "CPU_RATE_OVER_THRESHOLD_TRAP_CLEAR", "CPU_RATE_OVER_THRESHOLD_TRAP_CLEAR");	
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//1.21
	descr_tmp=trap_descr_list_register(list, ACMEMUTILIZATIONOVERTHRESHOLD, acMemUsageTooHighRecovTrap, TRAP_SRC_AC, ".1.21", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MEM_RATE_OVER_THRESHOLD_TRAP_CLEAR", "MEM_RATE_OVER_THRESHOLD_TRAP_CLEAR");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACTEMPERTOOHIGH, acTemperTooHighTrap, TRAP_SRC_AC, ".1.22", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "TEMP_RATE_OVER_THRESHOLD_TRAP", "TEMP_RATE_OVER_THRESHOLD_TRAP");	
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACTEMPERTOOHIGH, acTemperTooHighRecovTrap, TRAP_SRC_AC, ".1.23", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "TEMP_RATE_OVER_THRESHOLD_TRAP_CLEAR", "TEMP_RATE_OVER_THRESHOLD_TRAP_CLEAR");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACDHCPADDRESSEXHAUST, acDHCPAddressExhaustTrap, TRAP_SRC_AC, ".1.24", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "DHCP_IP_POOL_EXHAUSTED", "DHCP_IP_POOL_EXHAUSTED");	
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACDHCPADDRESSEXHAUST, acDHCPAddressExhaustRecovTrap, TRAP_SRC_AC, ".1.25", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "DHCP_IP_POOL_EXHAUSTED_CLEAR", "DHCP_IP_POOL_EXHAUSTED_CLEAR");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACRADIUSAUTHSERVERNOTREACH, acRadiusAuthServerAvailableTrap, TRAP_SRC_AC, ".1.26", 	TRAP_TYPE_PROCESS_ERR, 
									TRAP_LEVEL_SECONDARY, "asd_auth_radius_connect_failed_clear", "asd_auth_radius_connect_failed_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACRADIUSACCOUNTSERVERNOTREACH, acRadiusAccServerAvailableTrap, TRAP_SRC_AC, ".1.27", TRAP_TYPE_PROCESS_ERR, 
									TRAP_LEVEL_SECONDARY, "asd_account_radius_connect_failed_clear", "asd_account_radius_connect_failed_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACPORTALSERVERNOTREACH, acPortalServerAvailableTrap, TRAP_SRC_AC, ".1.28", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "Portal_Server_not_reached_clear", "Portal_Server_not_reached_clear");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
	descr_tmp=trap_descr_list_register(list, ACFINDATTACK, acFindAttackTrap, TRAP_SRC_AC, ".1.29", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "AC_FIND_ATTACK", "AC_FIND_ATTACK");
	INIT_DESCR_HASH_LIST( descr_tmp , ht );
	
//1.50
	descr_tmp=trap_descr_list_register(list, ACHEARTTIMEPACKAGE, acHeartTimePackageTrap, TRAP_SRC_AC, ".1.50", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "HEART_TIME_SEND_PACKAGES", "HEART_TIME_SEND_PACKAGES");	
	INIT_DESCR_HASH_LIST( descr_tmp , ht );

	TRAP_SHOW_HASHTABLE(ht, descr_tmp, descr_hash_node ,trap_name);
}

#if 0
void trap_signal_register_all(TrapList *list)
{
	trap_signal_list_init(list);

	//ap
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AP_REBOOT , 				wtp_sys_start_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AP_DOWN , 				wtp_down_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_IP_CHANGE_ALARM, 			wtp_ip_change_alarm_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_CHANGE, 			wtp_channel_modified_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_TRANFER_FILE, 			wtp_file_transfer_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_ELECTRIFY_REGISTER_CIRCLE,wtp_electrify_register_circle_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_ENTER_IMAGEDATA_STATE, 	wtp_ap_update_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CODE_START, 				wtp_cold_start_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WLAN_ENCRYPTION_TYPE_CHANGE, 	wtp_auth_mode_change_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WLAN_PRESHARED_KEY_CHANGE, 	wtp_preshared_key_change_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AP_FLASH_WRITE_FAIL,		wtp_flash_write_fail_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_DIVORCE_NETWORK,			ap_divorce_network_func);			
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE,		ap_actimesynchrofailure_func);			
	trap_signal_list_register(list, ASD_DBUS_SIG_STA_LEAVE,							th_sta_leave_func);		
	
	trap_signal_list_register(list, ASD_DBUS_SIG_STA_ASSOC_FAILED,					th_sta_assoc_fail_func);		
	trap_signal_list_register(list, ASD_DBUS_SIG_STA_JIANQUAN_FAILED,				th_sta_jianquan_fail_func);		
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_SSID_KEY_CONFLICT,			wid_ssid_key_conflict_func);			
	
	trap_signal_list_register(list, ASD_DBUS_SIG_STA_COME,							th_sta_come_func);		
	trap_signal_list_register(list, ASD_DBUS_SIG_WTP_DENY_STA,						th_wtp_deny_sta_func);		
	trap_signal_list_register(list, ASD_DBUS_SIG_DE_WTP_DENY_STA,					th_de_wtp_deny_sta_func);
	trap_signal_list_register(list, ASD_DBUS_SIG_WAPI_TRAP,							asd_wapi_trap_func);
	trap_signal_list_register(list, ASD_DBUS_SIG_STA_VERIFY,						asd_sta_verify_func);
	trap_signal_list_register(list, ASD_DBUS_SIG_STA_VERIFY_FAILED,					asd_sta_verify_failed_func);

	//trap_signal_list_register(list, ASD_DBUS_SIG_RADIUS_CONNECT_FAILED,				asd_radius_connect_failed_func);
	//trap_signal_list_register(list, ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN,		asd_radius_connect_failed_clean_func);

	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN,	wtp_wireless_interface_down_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN_CLEAR,wtp_wireless_interface_down_clear_func);
	trap_signal_list_register(list, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP,		route_method_notify_snmp_by_ip_func);
	
	trap_signal_list_register(list, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP,		route_method_notify_snmp_by_vrrp_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE,wtp_channel_device_interference_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE, 	wtp_channel_ap_interference_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE, wtp_channel_terminal_interference_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR, 		wtp_channel_count_minor_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE_CLEAR, wtp_channel_device_interference_clear_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE_CLEAR, wtp_channel_ap_interference_clear_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE_CLEAR, wtp_channel_terminal_interference_clear_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR_CLEAR, wtp_channel_count_minor_clear_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD, 			ap_cpu_threshold_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD, 			ap_mem_threshold_func);	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD, 			ap_temp_threshold_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR, 			ap_wifi_if_error_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_ATH_ERROR, 				ap_ath_if_error_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_RRM_STATE_CHANGE, 			ap_mt_work_mode_chg_func);
	trap_signal_list_register(list, "wid_dbus_trap_power_state_change", 			power_state_change);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_AP_RUN_QUIT, 					ap_run_quit_state);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_FIND_UNSAFE_ESSID, 		ap_find_unsafe_essid);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_FIND_WIDS_ATTACK, 		wtp_find_wids_attack);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_NEIGHBOR_CHANNEL_AP_INTERFERENCE, ap_neighbor_channel_interfere_func);
	
	//ac
	//trap_signal_list_register(list, "reboot", 										ac_reboot_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_DANGER_AP, 	ac_discovery_danger_ap_func);
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE, 	wtp_cover_hole_func);
	
	trap_signal_list_register(list, WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE_CLEAR, wtp_cover_hole_clear_func);

	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU, 			ac_cpu_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE, 		ac_memory_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE, 	ac_temperature_over_threshold_and_clear_func);

	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH, 		ac_bandwith_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE, 		ac_drop_rate_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MAX_USER, 		ac_online_user_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ, 	ac_auth_fail_over_threshold_and_clear_func);

	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL, 		ac_ip_pool_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH, 	ac_portal_reach_over_threshold_and_clear_func);

	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH, 	ac_radius_auth_reach_status_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC, 	ac_radius_acct_reach_status_func);

	trap_signal_list_register(list, "dhcp_notify_web_pool_event", 					ac_dhcp_exhaust_over_threshold_and_clear_func);
	trap_signal_list_register(list, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK,ac_find_attack_over_threshold_and_clear_func);
	
}

void trap_descr_register_all(TrapList *list)
{
	trap_descr_list_init(list);
	
//ap inner
//0.1
	trap_descr_list_register(list, wtpDownTrap, TRAP_SRC_AP,  ".0.1",TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "ap_power_off", "ap_power_off");

	trap_descr_list_register(list,  wtpSysStartTrap,  TRAP_SRC_AP, ".0.2", TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "ap_reboot", "ap_reboot");
	
	trap_descr_list_register(list,  wtpChannelModifiedTrap,  TRAP_SRC_AP, ".0.3", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR,	"WTP_CHANNEL_CHANGE", "WTP_CHANNEL_CHANGE");
	
	trap_descr_list_register(list,  wtpIPChangeAlarmTrap,  TRAP_SRC_AP, ".0.4", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "wtp_ip_change_alarm", "wtp_ip_change_alarm");
	
	trap_descr_list_register(list,  wtpFlashWriteFailTrap, TRAP_SRC_AP, ".0.5", TRAP_TYPE_PROCESS_ERR, 
									TRAP_LEVEL_SECONDARY, "ap_version_update_failure", "ap_version_update_failure");

	trap_descr_list_register(list,  wtpColdStartTrap, TRAP_SRC_AP, ".0.6", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "wtp_cold_reboot", "wtp_cold_reboot");

	trap_descr_list_register(list, wtpAuthModeChangeTrap, TRAP_SRC_AP,  ".0.7",TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "WLAN_ENCRYPTION_TYPE_CHANGE", "WLAN_ENCRYPTION_TYPE_CHANGE");

	trap_descr_list_register(list,  wtpPreSharedKeyChangeTrap,  TRAP_SRC_AP, ".0.8", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "WLAN_PRESHARED_KEY_CHANGE", "WLAN_PRESHARED_KEY_CHANGE");
	
	trap_descr_list_register(list,  wtpElectrifyRegisterCircleTrap,  TRAP_SRC_AP, ".0.9", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR,	"ELECTRIFY_REGISTER", "ELECTRIFY_REGISTER");
	
	trap_descr_list_register(list,  wtpAPUpdateTrap,  TRAP_SRC_AP, ".0.10", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "ap_version_update_begin", "ap_version_update_begin");
//0.10
	trap_descr_list_register(list,  wtpCoverholeTrap, TRAP_SRC_AP, ".0.11", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wtp_discovery_cover_hole", "wtp_discovery_cover_hole");
									
	trap_descr_list_register(list, CPUusageTooHighTrap, TRAP_SRC_AP,  ".0.13",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_cpu_over_threshold", "wid_ap_cpu_over_threshold");

	trap_descr_list_register(list,  CPUusageTooHighRecovTrap,  TRAP_SRC_AP, ".0.14", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_cpu_over_threshold_clear", "wid_ap_cpu_over_threshold_clear");
	
	trap_descr_list_register(list,  MemUsageTooHighTrap,  TRAP_SRC_AP, ".0.15", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR,	"wid_ap_mem_over_threshold", "wid_ap_mem_over_threshold");
	
	trap_descr_list_register(list,  MemUsageTooHighRecovTrap,  TRAP_SRC_AP, ".0.16", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_mem_over_threshold_clear", "wid_ap_mem_over_threshold_clear");
	
	trap_descr_list_register(list,  TemperTooHighTrap, TRAP_SRC_AP, ".0.17", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "AP_temperature_is_too_high", "AP_temperature_is_too_high");

	trap_descr_list_register(list,  TemperTooHighRecoverTrap, TRAP_SRC_AP, ".0.18", 	TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "AP_temperature_is_recover", "AP_temperature_is_recover");
	
	trap_descr_list_register(list, APMtWorkModeChgTrap, TRAP_SRC_AP,  ".0.19",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_rrm_state_change", "wid_ap_rrm_state_change");
//0.21
	trap_descr_list_register(list,  SSIDkeyConflictTrap,  TRAP_SRC_AP, ".0.21", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "station_key_conflict", "station_key_conflict");
	
	trap_descr_list_register(list,  wtpOnlineTrap,  TRAP_SRC_AP, ".0.22", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC,	"ap_is_running", "ap_is_running");
	
	trap_descr_list_register(list,  wtpOfflineTrap,  TRAP_SRC_AP, ".0.23", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "AP_is_quit", "AP_is_quit");
	
	trap_descr_list_register(list,  wtpCoverHoleClearTrap, TRAP_SRC_AP, ".0.24", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wtp_repair_cover_hole", "wtp_repair_cover_hole");
//ap app
//1.1
	trap_descr_list_register( list, wtpChannelObstructionTrap, TRAP_SRC_AP, ".1.1", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "wtp_channel_obstruction", "wtp_channel_obstruction");

	trap_descr_list_register(list, wtpAPInterferenceDetectedTrap, TRAP_SRC_AP,  ".1.2",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_interfere_detected", "ap_interfere_detected");

	trap_descr_list_register(list,  wtpStaInterferenceDetectedTrap,  TRAP_SRC_AP, ".1.3", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "wtp_sta_interference_detected", "wtp_sta_interference_detected");
	
	trap_descr_list_register(list,  wtpDeviceInterferenceDetectedTrap,  TRAP_SRC_AP, ".1.4", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY,	"wtp_device_interference_detected", "wtp_device_interference_detected");

	trap_descr_list_register(list,  wtpSubscriberDatabaseFullTrap,  TRAP_SRC_AP, ".1.5", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL,	"wtp_deny_sta", "wtp_deny_sta");

	trap_descr_list_register(list,  wtpDFSFreeCountBelowThresholdTrap,  TRAP_SRC_AP, ".1.6", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL,	"channel_count_minor", "channel_count_minor");
	
	trap_descr_list_register(list,  wtpFileTransferTrap,  TRAP_SRC_AP, ".1.7", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "WTP_TRANFER_FILE", "WTP_TRANFER_FILE");
	
	trap_descr_list_register(list,  wtpStationOffLineTrap, TRAP_SRC_AP, ".1.8", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "station_leave", "station_leave");

	trap_descr_list_register(list,  wtpSolveLinkVarifiedTrap, TRAP_SRC_AP, ".1.9", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_station_verify", "asd_station_verify");
	
	trap_descr_list_register(list, wtpLinkVarifyFailedTrap, TRAP_SRC_AP,  ".1.10",TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_station_verify_failed", "asd_station_verify_failed");
//1.11
	trap_descr_list_register(list,  wtpStationOnLineTrap,  TRAP_SRC_AP, ".1.11", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "station_come", "station_come");
	
	trap_descr_list_register(list,  APInterfClearTrap,  TRAP_SRC_AP, ".1.12", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY,	"ap_interfere_detected_clear", "ap_interfere_detected_clear");
	
	trap_descr_list_register(list,  APStaInterfClearTrap,  TRAP_SRC_AP, ".1.13", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_sta_interf_clear", "ap_sta_interf_clear");
	
	trap_descr_list_register(list,  APOtherDevInterfClearTrap, TRAP_SRC_AP, ".1.15", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "ap_other_dev_interf_clear", "ap_other_dev_interf_clear");

	trap_descr_list_register(list,  APModuleTroubleTrap, TRAP_SRC_AP, ".1.16", 	TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_wifi_interface_error", "wid_ap_wifi_interface_error");
	
	trap_descr_list_register(list, APModuleTroubleClearTrap, TRAP_SRC_AP,  ".1.17",TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_wifi_interface_error_clear", "wid_ap_wifi_interface_error_clear");

	trap_descr_list_register(list,  APRadioDownTrap,  TRAP_SRC_AP, ".1.18", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_ap_ath_module_error", "wid_ap_ath_module_error");
	
	trap_descr_list_register(list,  APRadioDownRecovTrap,  TRAP_SRC_AP, ".1.19", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC,	"wid_ap_ath_module_error_clear", "wid_ap_ath_module_error_clear");
//1.21
	trap_descr_list_register(list,  APStaAuthErrorTrap,  TRAP_SRC_AP, ".1.21", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "station_auth_fail", "station_auth_fail");
	
	trap_descr_list_register(list,  APStAssociationFailTrap, TRAP_SRC_AP, ".1.22", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "station_assoc_fail", "station_assoc_fail");

	trap_descr_list_register(list,  APUserWithInvalidCerInbreakNetTrap, TRAP_SRC_AP, ".1.23", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "Invalid_certification_user_attack", "Invalid_certification_user_attack");
	
	trap_descr_list_register(list, APStationRepititiveAttackTrap, TRAP_SRC_AP,  ".1.24",TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "Client_Re-attack", "Client_Re-attack");

	trap_descr_list_register(list,  APTamperAttackTrap,  TRAP_SRC_AP, ".1.25", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "tamper_attack", "tamper_attack");
	
	trap_descr_list_register(list,  APLowSafeLevelAttackTrap,  TRAP_SRC_AP, ".1.26", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR,	"low_safe_level_attack", "low_safe_level_attack");
	
	trap_descr_list_register(list,  APAddressRedirectionAttackTrap,  TRAP_SRC_AP, ".1.27", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "address_redirect_attack", "address_redirect_attack");
//1.31
	trap_descr_list_register(list,  APAddUserFailClearTrap,  TRAP_SRC_AP, ".1.37", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL, "wtp_deny_sta_clear", "wtp_deny_sta_clear");

	trap_descr_list_register(list,  APChannelTooLowClearTrap,  TRAP_SRC_AP, ".1.38", TRAP_TYPE_QOS, 
									TRAP_LEVEL_GENERAL, "channel_minor_clear", "channel_minor_clear");	

	trap_descr_list_register(list,  APFindUnsafeESSID,  TRAP_SRC_AP, ".1.39", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_MAJOR, "wid_ap_find_unsafe_essid", "wid_ap_find_unsafe_essid");
	
	trap_descr_list_register(list,  APFindSYNAttack,  TRAP_SRC_AP, ".1.40", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_dbus_trap_wtp_find_wids_attack", "wid_dbus_trap_wtp_find_wids_attack");

	trap_descr_list_register(list,  ApNeighborChannelInterfTrap,  TRAP_SRC_AP, ".1.41", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_neighbor_channel_interfere", "ap_neighbor_channel_interfere");

	trap_descr_list_register(list,  ApNeighborChannelInterfTrapClear,  TRAP_SRC_AP, ".1.42", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ap_neighbor_channel_interfere_clear", "ap_neighbor_channel_interfere_clear");
//ac inner
//0.1
	trap_descr_list_register(list,  acSystemRebootTrap, TRAP_SRC_AC, ".0.1", TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "ac_reboot", "ac_reboot");

	trap_descr_list_register(list,  acAPACTimeSynchroFailureTrap, TRAP_SRC_AC, ".0.2", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "wtp_ap_actimesynchrofailure", "wtp_ap_actimesynchrofailure");
	
	trap_descr_list_register(list,  acChangedIPTrap, TRAP_SRC_AC, ".0.3", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_GENERAL, "AC_CHANGED_IP", "AC_CHANGED_IP");

	trap_descr_list_register(list,  acTurntoBackupDeviceTrap, TRAP_SRC_AC, ".0.4", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_CRITIC, "AC_TURNTO_BACK_BY_VRRP", "AC_TURNTO_BACK_BY_VRRP");

	trap_descr_list_register(list,  acConfigurationErrorTrap, TRAP_SRC_AC, ".0.5", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "AC_PARSE_CONFIGURE_ERROR", "AC_PARSE_CONFIGURE_ERROR");

	trap_descr_list_register(list,  acSysColdStartTrap, TRAP_SRC_AC, ".0.6", TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_CRITIC, "AC_COLD_START", "AC_COLD_START");

//0.22
	trap_descr_list_register(list,  acAPACTimeSynchroFailureTrapClear, TRAP_SRC_AC, ".0.22", 	TRAP_TYPE_DEVICE, 
									TRAP_LEVEL_MAJOR, "wtp_ap_actimesynchrofailure_clear", "wtp_ap_actimesynchrofailure_clear");
	
// ac app
//1.1
	trap_descr_list_register(list,  acDiscoveryDangerAPTrap, TRAP_SRC_AC, ".1.1", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_SECONDARY, "ac_discovery_danger_ap", "ac_discovery_danger_ap");
	
	trap_descr_list_register(list,  acRadiusAuthenticationServerNotReachTrap, TRAP_SRC_AC, ".1.2", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_auth_radius_connect_failed", "asd_auth_radius_connect_failed");
	
	trap_descr_list_register(list,  acRadiusAccountServerNotLinkTrap, TRAP_SRC_AC, ".1.3", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "asd_account_radius_connect_failed", "asd_account_radius_connect_failed");

	trap_descr_list_register(list,  acPortalServerNotReachTrap, TRAP_SRC_AC, ".1.4", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "Portal_Server_can_not_reached", "Portal_Server_can_not_reached");

	trap_descr_list_register(list,  acAPLostNetTrap, TRAP_SRC_AC, ".1.5", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "wid_set_wtp_divorce_network", "wid_set_wtp_divorce_network");

	trap_descr_list_register(list,  acCPUUtilizationOverThresholdTrap, TRAP_SRC_AC, ".1.6", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "CPU_RATE_OVER_THRESHOLD_TRAP", "CPU_RATE_OVER_THRESHOLD_TRAP");

	trap_descr_list_register(list,  acMemUtilizationOverThresholdTrap, TRAP_SRC_AC, ".1.7", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MEM_RATE_OVER_THRESHOLD_TRAP", "MEM_RATE_OVER_THRESHOLD_TRAP");
	
	trap_descr_list_register(list,  acBandwithOverThresholdTrap, TRAP_SRC_AC, ".1.8", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "BANDWITH_RATE_OVER_THRESHOLD_VALUE", "BANDWITH_RATE_OVER_THRESHOLD_VALUE");
	
	trap_descr_list_register(list,  acLostPackRateOverThresholdTrap, TRAP_SRC_AC, ".1.9", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "DROP_RATE_OVER_THRESHOLD_VALUE", "DROP_RATE_OVER_THRESHOLD_VALUE");		
	
	trap_descr_list_register(list,  acMaxUsrNumOverThresholdTrap, TRAP_SRC_AC, ".1.10", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MAX_ONLINE_USER_OVER_THRESHOLD_VALUE", "MAX_ONLINE_USER_OVER_THRESHOLD_VALUE");
//1.11
	trap_descr_list_register(list,  acAuthSucRateBelowThresholdTrap, TRAP_SRC_AC, ".1.14", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "AUTH_SUC_RATE_OVER_THRESHOLD_VALUE", "AUTH_SUC_RATE_OVER_THRESHOLD_VALUE");
									
	trap_descr_list_register(list,  acAveIPPoolOverThresholdTrap, TRAP_SRC_AC, ".1.15", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "IP_POOL_AVE_RATE_OVER_THRESHOLD", "IP_POOL_AVE_RATE_OVER_THRESHOLD");
									
	trap_descr_list_register(list,  acMaxIPPoolOverThresholdTrap, TRAP_SRC_AC, ".1.16", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "IP_POOL_MAX_OVER_THRESHOLD", "IP_POOL_MAX_OVER_THRESHOLD");

	trap_descr_list_register(list,  acPowerOffTrap, TRAP_SRC_AC, ".1.18", TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_power_state_change", "wid_power_state_change");	

	trap_descr_list_register(list,  acPowerOffRecovTrap, TRAP_SRC_AC, ".1.19", 	TRAP_TYPE_ENVIRO, 
									TRAP_LEVEL_CRITIC, "wid_power_state_change_clear", "wid_power_state_change_clear");
		
	trap_descr_list_register(list,  acCPUusageTooHighRecovTrap, TRAP_SRC_AC, ".1.20", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "CPU_RATE_OVER_THRESHOLD_TRAP_CLEAR", "CPU_RATE_OVER_THRESHOLD_TRAP_CLEAR");	
//1.21
	trap_descr_list_register(list,  acMemUsageTooHighRecovTrap, TRAP_SRC_AC, ".1.21", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "MEM_RATE_OVER_THRESHOLD_TRAP_CLEAR", "MEM_RATE_OVER_THRESHOLD_TRAP_CLEAR");
									
	trap_descr_list_register(list,  acTemperTooHighTrap, TRAP_SRC_AC, ".1.22", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "TEMP_RATE_OVER_THRESHOLD_TRAP", "TEMP_RATE_OVER_THRESHOLD_TRAP");	

	trap_descr_list_register(list,  acTemperTooHighRecovTrap, TRAP_SRC_AC, ".1.23", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "TEMP_RATE_OVER_THRESHOLD_TRAP_CLEAR", "TEMP_RATE_OVER_THRESHOLD_TRAP_CLEAR");
									
	trap_descr_list_register(list,  acDHCPAddressExhaustTrap, TRAP_SRC_AC, ".1.24", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "DHCP_IP_POOL_EXHAUSTED", "DHCP_IP_POOL_EXHAUSTED");	
									
	trap_descr_list_register(list,  acDHCPAddressExhaustRecovTrap, TRAP_SRC_AC, ".1.25", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_MAJOR, "DHCP_IP_POOL_EXHAUSTED_CLEAR", "DHCP_IP_POOL_EXHAUSTED_CLEAR");

	trap_descr_list_register(list,  acRadiusAuthServerAvailableTrap, TRAP_SRC_AC, ".1.26", 	TRAP_TYPE_PROCESS_ERR, 
									TRAP_LEVEL_SECONDARY, "asd_auth_radius_connect_failed_clear", "asd_auth_radius_connect_failed_clear");

	trap_descr_list_register(list,  acRadiusAccServerAvailableTrap, TRAP_SRC_AC, ".1.27", TRAP_TYPE_PROCESS_ERR, 
									TRAP_LEVEL_SECONDARY, "asd_account_radius_connect_failed_clear", "asd_account_radius_connect_failed_clear");

	trap_descr_list_register(list,  acPortalServerAvailableTrap, TRAP_SRC_AC, ".1.28", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "Portal_Server_not_reached_clear", "Portal_Server_not_reached_clear");
									
	trap_descr_list_register(list,  acFindAttackTrap, TRAP_SRC_AC, ".1.29", TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "AC_FIND_ATTACK", "AC_FIND_ATTACK");
//1.50
	trap_descr_list_register(list,  acHeartTimePackageTrap, TRAP_SRC_AC, ".1.50", 	TRAP_TYPE_COMMUNIC, 
									TRAP_LEVEL_SECONDARY, "HEART_TIME_SEND_PACKAGES", "HEART_TIME_SEND_PACKAGES");	
}

#endif

void trap_signal_list_show(TrapList *tSignalList)
{
	TrapNode *tNode;
	TrapSignal *tSignal;
	
	for (tNode = tSignalList->first; tNode; tNode = tNode->next) {
		tSignal = tNode->data;
		printf("\tsignal_name = %s, signal_handle_func = %p\n", tSignal->signal_name, tSignal->signal_handle_func);
	}
}

void trap_descr_list_show(TrapList *tDescrList)
{
	TrapNode *tNode;
	TrapDescr *tDescr;
	
	for (tNode = tDescrList->first; tNode; tNode = tNode->next) {
		tDescr = tNode->data;
		printf("\ttrap_name = %s, event_source = %d, trap_oid = %s, switch_status=%d, "
			"trap_type = %s, trap_level = %s, title = %s, content = %s\n", 
			tDescr->trap_name, tDescr->event_source, tDescr->trap_oid, tDescr->switch_status,
			tDescr->trap_type, tDescr->trap_level, tDescr->title, tDescr->content);
	}
}

#if 0
void trap_signal_test(void)
{
	printf("Signal Test:\n");
	printf("trap signal register all\n");
	trap_signal_register_all(&gSignalList);
	trap_signal_list_show(&gSignalList);
	
	printf("trap signal list destroy\n");
	trap_signal_list_destroy(&gSignalList);
	trap_signal_list_show(&gSignalList);
}

void trap_descr_test(void)
{
	printf("Descr Test:\n");
	printf("trap descr register all\n");
	trap_descr_register_all(&gSignalList);
	trap_descr_load_switch(&gSignalList);
	trap_descr_list_show(&gSignalList);
	
	TrapDescr *tDescr = NULL;
	printf("trap_descr_list_get_item\n");
	tDescr = trap_descr_list_get_item(&gSignalList, acTemperTooHighRecovTrap);
	if (NULL != tDescr)
		printf("\ttrap_name = %s, event_source = %d, trap_oid = %s, trap_type = %s, "
				"trap_level = %s, title = %s, content = %s\n", 
				tDescr->trap_name, tDescr->event_source, tDescr->trap_oid, tDescr->trap_type,
				tDescr->trap_level, tDescr->title, tDescr->content);
	
	printf("trap descr list destroy\n");
	trap_descr_list_destroy(&gSignalList);
	trap_descr_list_show(&gSignalList);
}
#endif
