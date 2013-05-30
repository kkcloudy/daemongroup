#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include "vtysh/vtysh.h"
#include <dbus/dbus.h>

#include "command.h"

#include "../dcli_main.h"
#include "wcpss/waw.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "hmd/hmdpub.h"
//#include "dbus/hmd/HmdDbusPath.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "../dcli_vrrp.h"
#include "../dcli_dhcp.h"
#include "../dcli_pppoe.h"
#include "memory.h"
#include "dcli_local_hansi.h"
struct cmd_node local_hansi_node =
{
	LOCAL_HANSI_NODE,
	"%s(local-hansi %d-%d)# ",
	1
};

#define DCLI_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255/32"))
#define DCLI_IPMASK_STRING_MINLEN	(strlen("0.0.0.0/0"))
#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))
/*wcl add*/
extern int boot_flag;
/*end*/

#if 1
unsigned char *dcli_local_hansi_err_msg[] = {	\
/*   0 */	"%% Error none\n",
/*   1 */	"%% General failure\n",
/*   2 */	"%% Profile out of range\n",
/*   3 */	"%% Profile has already exist\n",
/*   4 */	"%% Profile not exist\n",
/*   5 */	"%% Memory malloc failed\n",
/*   6 */	"%% Bad parameter input\n",
/*   7 */	"%% Heartbeatlink or uplink,downlink,vgateway not configured\n",
/*	 8 */	"%% Service should be disabled first\n",
/*   9 */	"%% Virtual gateway have not setted\n",
/*	 A */	"%% Did not choose the appropriate (link+priority) mode\n",
/*	 B */	"%% Interface name error\n",
/*	 C */	"%% Virtual IP address has setted\n",
/*	 D */	"%% Virtual IP address has not setted\n",
/*	 E */	"%% Virtual IP address is last one, not allow to delete\n",
/*	 F */	"%% Interface has exist\n",
/*	 10 */	"%% Interface not exist\n",
/*    11 */	"%% No more items can be configed for this command!\n",
/*	 12 */	"%% Virtual mac should be enable first\n",
};
#endif 

/*fengwenchao add 20120413 for hmd timer config save*/
int parse_int_ID(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		if((endptr[0] == '\0')||(endptr[0] == '\n'))
			return 0;
		else
			return -1;
	}
	else
		return -1;
}

/*fengwenchao add end */

int check_ip_format
(
   char* buf,
   int* split_count
)
{
    char *split = "/";
	char *str = NULL;
	int length = 0,i = 0,splitCount = 0;
	if(NULL == buf){
        return CMD_WARNING;
	}	
	str = buf;
	length = strlen(str);
	if( length > DCLI_IPMASK_STRING_MAXLEN ||  \
		length < DCLI_IP_STRING_MINLEN ){
		return CMD_WARNING;
	}
	if((str[0] > '9')||(str[0] < '1')){
		return CMD_WARNING;
	}
	for(i = 0; i < length; i++){
		if('/' == str[i]){
            splitCount++;
			if((i == length - 1)||('0' > str[i+1])||(str[i+1] > '9')){
                return CMD_WARNING;
			}
		}
		if((str[i] > '9'||str[i]<'0') &&  \
			str[i] != '.' &&  \
			str[i] != '/' &&  \
			str[i] != '\0'
		){
			return CMD_WARNING;
		}
	}  
    *split_count = splitCount;
	return CMD_SUCCESS;
}

/**
 * check if hmd process has started before or not
 * now we use 'ps -ef | grep hmd #' format to indicate 
 * the corresponding process is running or not
 */
int dcli_hmd_hansi_is_running
(
	struct vty* vty,
	unsigned int slot_id,
	int islocal,
	int instID
)
{
	int isRunning = 0, ret = 0, fd = -1, iCnt = 0;	
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0}, readBuf[4] = {0};
	FILE *pidmap_file = NULL, *hadpid_file = NULL, *pathpid_file = NULL;
	char msg[DCLI_VRRP_SYS_COMMAND_LEN] = {0}, str_pid[DCLI_VRRP_SYS_COMMAND_LEN] = {0}, pathpid[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	char *sep=" ", *token = NULL;
	int hmdpid = 0;


	sprintf(commandBuf, "/var/run/hmd/hmd%d-%d-%d.pid", islocal,slot_id,instID);
	hadpid_file = fopen(commandBuf, "r");
	//printf("%s,commandBuf=%s,hadpid_file=%p.\n",__func__,commandBuf,hadpid_file);
	if (!hadpid_file) {
		//printf("%s,line:%d\n",__func__,__LINE__);
		return INSTANCE_NO_CREATED;
	}
	fclose(hadpid_file);
	#if 0
	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/hmd%d-%d-%d.pidmap", islocal,slot_id,instID);
	pidmap_file = fopen(commandBuf, "r");
	if (!pidmap_file) {
		return DCLI_VRRP_INSTANCE_NO_CREATED;
	}
	while (fgets(msg, DCLI_VRRP_SYS_COMMAND_LEN, pidmap_file)) {
		token = strtok(msg, sep);
		while (token) {
			strcpy(str_pid, token);
			token = strtok(NULL, sep);
		}
		hmdpid = strtoul(str_pid, NULL, 10);
		sprintf(pathpid, "/proc/%d", hmdpid);
		pathpid_file = fopen(pathpid, "r");
		if (!pathpid_file) {
			fclose(pidmap_file);
			return DCLI_VRRP_INSTANCE_NO_CREATED;
		}
		fclose(pathpid_file);
		memset(msg, 0, DCLI_VRRP_SYS_COMMAND_LEN);
		memset(pathpid, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	}
	
	fclose(pidmap_file);
	
	if (!hmdpid) {
		printf("%s,line:%d\n",__func__,__LINE__);
		return INSTANCE_NO_CREATED;
	}
#endif
	//printf("%s,line:%d\n",__func__,__LINE__);
	return INSTANCE_CREATED;
}

int dcli_hmd_show_hansi_running_cfg
(
	struct vty *vty,
	unsigned int profile,
	unsigned int slotid,
	unsigned int islocal
)
{	
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	char *showStr = NULL;
	char tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};

	int instRun = 0;

	char *showRunningCfg_str = NULL;
	char *cursor = NULL;
	int totalLen = 0;

	showRunningCfg_str = (char *)malloc(SHOW_RUNNING_CONFIG_LEN);
	if (NULL == showRunningCfg_str) {
		return 1;
	}
	memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);

//	DBusConnection *dcli_dbus_connection = NULL;
//	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,
										 HMD_DBUS_OBJPATH,
										 HMD_DBUS_INTERFACE,
										 HMD_DBUS_METHOD_SHOW_RUNNING);
	dbus_error_init(&err);
	dbus_message_append_args(query,
 							 DBUS_TYPE_UINT32, &slotid,
 							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &islocal,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_local, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(showRunningCfg_str);
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		cursor = showRunningCfg_str;
		totalLen = 0;
		totalLen += sprintf(cursor, "==================================================\n");
		cursor = showRunningCfg_str + totalLen;
		
		totalLen += sprintf(cursor, "%s", showStr);
		cursor = showRunningCfg_str + totalLen;
		totalLen += sprintf(cursor, "==================================================\n");
		cursor = showRunningCfg_str + totalLen;
		vty_out(vty, "%s", showRunningCfg_str);
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(showRunningCfg_str);
		return 1;
	}

	dbus_message_unref(reply);
	free(showRunningCfg_str);
	return 0;	
}

char *
dcli_hmd_show_running_config(int localid, int slot_id, int index)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,
										 HMD_DBUS_OBJPATH,
										 HMD_DBUS_INTERFACE,
										 HMD_DBUS_METHOD_HANSI_SHOW_RUNNING);

	dbus_error_init(&err);
	dbus_message_append_args(query,
 							 DBUS_TYPE_UINT32, &slot_id,
 							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &localid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);  //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show hansi_ebr start config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));			
		dbus_message_unref(reply);
		return tmp; 
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return NULL;	
	}

}


int dcli_local_hansi_show_running_config(struct vty *vty)
{	
	char *showStr = NULL, ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int timeout = 0;
	int profile = 0;
	int instRun = 0, ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	/* flag for one time insert string "VRRP section" into show running-config  */
	int bulid_vrrp_moudle_flg = 0;
	char *showRunningCfg_str = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	char *tmp = NULL;
	char wireless_cmd[] = "show interface wireless_config local";	
	char ebr_cmd[] = "show interface ebr_config local";
	char wireless_cmd1[128];
	showRunningCfg_str = (char *)malloc(SHOW_RUNNING_CONFIG_LEN);
	int check_result = DCLI_VRRP_INSTANCE_CREATED;
	unsigned char service_name[4][4] = {"hmd","wid", "asd", "wsm"};
	unsigned int service_index = 0;
	int localid = 1;
	int slot_id = 0;
	int dhcp_flag = 0;
	if (NULL == showRunningCfg_str) {
		//syslog_ax_vlan_err("memory malloc error\n");
		return 1;
	}
	memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);
	dcli_license_show_running_config(vty);

	if (0 == bulid_vrrp_moudle_flg) {
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "HMD");
		vtysh_add_show_string(_tmpstr);
		bulid_vrrp_moudle_flg = 1;
	}
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){
		for (profile = 0; profile <= DCLI_VRRP_INSTANCE_CNT; profile++) {
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
			if(INSTANCE_CREATED == instRun){
				dhcp_flag = 1;
				break;
			}
		}
		if(INSTANCE_NO_CREATED == instRun){
			if(active_master_slot == slot_id)
				dhcp_flag = 1;
			else
				dhcp_flag = 0;
		}
	for (profile = 0; profile <= DCLI_VRRP_INSTANCE_CNT; profile++) {
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			continue;
		}
		else if (INSTANCE_CHECK_FAILED == instRun) {
			continue;
		}
#if 0
		if (0 != profile) {
			for (service_index = 1; service_index < 3; service_index++) {
				check_result = dcli_vrrp_check_service_started(vty, service_name[service_index], profile);
				if (DCLI_VRRP_INSTANCE_CREATED != check_result) {
					break;					
				}
			}
		}
		if (DCLI_VRRP_INSTANCE_CREATED != check_result) {
				continue;					
		}
#endif
		memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;

#ifdef _D_WCPSS_
		totalLen += sprintf(cursor, "config local-hansi %d-%d\n",slot_id,profile);
				cursor = showRunningCfg_str + totalLen;
#if 1
		if (0 != profile) {
			if(dhcp_flag == 0){
				dhcp_flag = 1;
				tmp = dcli_dhcp_show_running_cfg2(slot_id);			
				if(tmp != NULL){
					if (0 != strlen(tmp)) {					
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;

						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}

				tmp = dcli_dhcrelay_show_running_cfg2(vty,slot_id); 		
				if(tmp != NULL){
					if (0 != strlen(tmp)) { 				
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;
		
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}

				tmp = dcli_dhcp_snp_show_running_config2(vty, slot_id); 		
				if(tmp != NULL){
					if (0 != strlen(tmp)) { 				
						totalLen += sprintf(cursor, " \n");
						cursor = showRunningCfg_str + totalLen;
		
						totalLen += sprintf(cursor, "%s\n",tmp);
						cursor = showRunningCfg_str + totalLen;
					}
					free(tmp);
					tmp = NULL;
				}else{
					free(showRunningCfg_str);
					return 1;
				}
				
			}
			tmp = dcli_hansi_ebr_show_running_config_start(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					
					totalLen += sprintf(cursor, " \n");
					cursor = showRunningCfg_str + totalLen;

					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			memset(wireless_cmd1, 0, 128);
			sprintf(wireless_cmd1, "%s %d %d",ebr_cmd,slot_id,profile);
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,1,0);
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			tmp = dcli_hansi_security_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_ac_ip_list_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wlan_show_running_config_start(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wtp_show_running_config_start(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wlan_list_show_running_config(localid, slot_id,profile); 
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wtp_list_show_running_config(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_bss_list_show_running_config(localid, slot_id,profile); 
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			memset(wireless_cmd1, 0, 128);
			sprintf(wireless_cmd1, "%s %d %d",wireless_cmd,slot_id,profile);
			tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], wireless_cmd1); 	
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);					
//					dcli_config_write(tmp,1,slot_id,profile,0,0);
					cursor = showRunningCfg_str + totalLen;
				}
				XFREE(MTYPE_TMP,tmp);
				tmp = NULL;
			}		
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);
			vtysh_add_show_string(showRunningCfg_str);		
			memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);
			cursor = showRunningCfg_str;
			totalLen = 0;
			tmp = dcli_hansi_wlan_show_running_config_end(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_wtp_show_running_config_end(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			tmp = dcli_hansi_ebr_show_running_config_end(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
			/*eag*/
			//memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);
			//cursor = showRunningCfg_str;
			//totalLen = 0;
			tmp = dcli_eag_show_running_config_2(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}		
			//vtysh_add_show_string(showRunningCfg_str);
			/*end*/
			/* pdc */
			tmp = dcli_pdc_show_running_config_2(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}		
			/*end*/
			/* rdc */
			tmp = dcli_rdc_show_running_config_2(localid, slot_id,profile);
			if(tmp != NULL){
				if (0 != strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;
				}
				free(tmp);
				tmp = NULL;
			}else{
				free(showRunningCfg_str);
				return 1;
			}
					
		/* pppoe config*/
		#ifndef _VERSION_18SP7_
			tmp = dcli_pppoe_show_running_config(localid, slot_id, profile);
			if (tmp){
				if (strlen(tmp)) {
					totalLen += sprintf(cursor, "%s\n",tmp);
					cursor = showRunningCfg_str + totalLen;

				}
				free(tmp);
				tmp = NULL;
			}
		#endif /* !_VERSION_18SP7_ */
			/*end*/
		}
		
		totalLen += sprintf(cursor, "exit\n");
				cursor = showRunningCfg_str + totalLen;
		/* dhcp config section */
		tmp = dcli_dhcp_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}
		
		tmp = dcli_dhcrelay_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}

		tmp = dcli_dhcp_snp_show_running_hansi_cfg(slot_id,profile,localid);
		if(tmp != NULL){
			if (0 != strlen(tmp)) {
				totalLen += sprintf(cursor, "%s\n",tmp);
				cursor = showRunningCfg_str + totalLen;
			}
			free(tmp);
			tmp = NULL;
		}else{
			free(showRunningCfg_str);
			return 1;
		}
		dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,0);		
		vtysh_add_show_string(showRunningCfg_str);
#endif

#endif		
		}
}
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++)			
	for (profile = 1; profile < MAX_INSTANCE; profile++) 
	{
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			continue;
		}
		else if (INSTANCE_CHECK_FAILED == instRun) {
			continue;
		}
		//profile = 0;
		memset(showRunningCfg_str, 0, SHOW_RUNNING_CONFIG_LEN);
		cursor = showRunningCfg_str;
		totalLen = 0;

		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,
											 HMD_DBUS_OBJPATH,
											 HMD_DBUS_INTERFACE,
											 HMD_DBUS_METHOD_SHOW_RUNNING);

		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32, &slot_id,
								 DBUS_TYPE_UINT32, &profile,
								 DBUS_TYPE_UINT32, &localid,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

		dbus_message_unref(query);
		if (NULL == reply) {
			printf("failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			free(showRunningCfg_str);
			return 1;
		}

		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_STRING, &showStr,
						DBUS_TYPE_INVALID)) 
		{
			/* [5] add showRunningCfg_str to vtysh process */
			totalLen += sprintf(cursor, "%s", showStr);
			cursor = showRunningCfg_str + totalLen;
			/* add string "exit" */
			//totalLen += sprintf(cursor, "exit\n");
			//cursor = showRunningCfg_str + totalLen;
			dcli_config_write(showRunningCfg_str,localid,slot_id,profile,0,1);		
			vtysh_add_show_string(showRunningCfg_str);
		} 
		else
		{
			printf("Failed get args.\n");
			if (dbus_error_is_set(&err)) 
			{
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			free(showRunningCfg_str);
			return 1;
		}

		dbus_message_unref(reply);
	}

	free(showRunningCfg_str);
	return 0;	
}

int dcli_license_show_running_config(struct vty *vty){
	char *showStr = NULL, ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,
										 HMD_DBUS_OBJPATH,
										 HMD_DBUS_INTERFACE,
										 HMD_DBUS_METHOD_LICENSE_SHOW_RUNNING);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "License Assign");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
	} 
	else
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}	
	dbus_message_unref(reply);	
	return 1;
}

DEFUN(config_local_hansi_cmd_func,
	  config_local_hansi_cmd,
	  "config local-hansi PARAMETER",
	  CONFIG_STR
	  "Local hansi information\n"
	  "hansi id that you want to config\n"
	 )
{	int ret;
	unsigned char status;
	unsigned int insID;
	unsigned int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*wlan_id = atoi(argv[0]);*/
	DBusMessage *query, *reply;
	DBusError err;
	int slot_num = 0;
	int slot_no = 0;
	int slot_no1 = 0;
	int local_id = 1;
	int flag = 0;
	int instRun = 0;
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0}; /*wcl add*/
	ret = parse_slot_hansi_id((char*)argv[0],&slot_id,&insID);
	if(ret != WID_DBUS_SUCCESS){
		slot_id = HostSlotId;
		flag = 1;
		ret = parse_int_ID((char*)argv[0], &insID);
		if(ret != WID_DBUS_SUCCESS){
	            if(ret == WID_ILLEGAL_INPUT){
	            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
	            }
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
			return CMD_WARNING;
		}	
	}
	if(distributFag == 0){
		if(slot_id != 0){
			vty_out(vty,"<error> slot id should be 0\n");
			return CMD_WARNING;
		}	
	}else if(flag == 1){
		slot_id = HostSlotId;
	}
	if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
		vty_out(vty,"<error> local hansi id should be 1 to %d\n",MAX_SLOT_NUM-1);
		return CMD_WARNING;
	}	
	if(insID >= MAX_INSTANCE || insID == 0){
		vty_out(vty,"<error> local hansi id should be 1 to %d\n",MAX_INSTANCE-1);
		return CMD_WARNING;
	}
	/*wcl add*/
	instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,insID);
		if(INSTANCE_NO_CREATED == instRun) {
			/* user verification */
			if (!boot_flag) {
				printf("Are you sure you want to start new hansi(yes/no)?");
				fscanf(stdin, "%s", cmd);
				if (!strncasecmp("no", cmd, strlen(cmd))){
					printf("\n%% User cancelled command.\n");
					return CMD_WARNING;
				}
				if (strncasecmp("yes", cmd, strlen(cmd))){
					printf("\nPlease type 'yes' or 'no':"); 
					fscanf(stdin, "%s", cmd);
					if (!strncasecmp("no", cmd, strlen(cmd))){
						printf("\n%% User cancelled command.\n");
						return CMD_WARNING;
					}
					else if (strncasecmp("yes", cmd, strlen(cmd))){
						printf("\n%% Bad choice given.\n");
						return CMD_WARNING;
					}
				}
				printf("\n");
			}
			}
	/*end*/

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LOCAL_HANSI);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&insID,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,150000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_UINT32,&slot_num,
					DBUS_TYPE_INT32,&slot_no,
					DBUS_TYPE_INT32,&slot_no1,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			if(vty->node == CONFIG_NODE){
				vty->node = LOCAL_HANSI_NODE;
				vty->index = (void *)insID;
				vty->slotindex = slot_id;
				vty->slotindex1 = slot_no1;
				vty->local	= 1;
			}
		}else if(ret == HMD_DBUS_PERMISSION_DENIAL){
			vty_out(vty,"<error> The Slot is not active master board, permission denial\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}else if(ret == HMD_DBUS_SLOT_ID_NOT_EXIST){
			vty_out(vty,"<error> The Slot is not EXIST\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}else if (ret == HMD_DBUS_ID_NO_EXIST){
			vty_out(vty,"<error> Local hansi has been created\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}else{
			vty_out(vty,"<error>  %d\n",ret);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(delete_local_hansi_cmd_func,
	  delete_local_hansi_cmd,
	  "delete local-hansi PARAMETER",
	  CONFIG_STR
	  "Local hansi information\n"
	  "hansi id that you want to config\n"
	 )
{	int ret;
	unsigned char status;
	unsigned char insID;
	unsigned int id;
	unsigned int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*wlan_id = atoi(argv[0]);*/
	DBusMessage *query, *reply;
	DBusError err;
	int slot_num = 0;
	int slot_no = 0;
	int slot_no1 = 0;
	int local_id = 1;
	int flag = 0;
	int instRun = 0;  /*wcl add*/

	ret = parse_slot_hansi_id((char*)argv[0],&slot_id,&id);
	insID = (unsigned char)id;
	if(ret != WID_DBUS_SUCCESS){
		slot_id = HostSlotId;
		flag = 1;
		ret = parse_char_ID((char*)argv[0], &insID);
		if(ret != WID_DBUS_SUCCESS){
	            if(ret == WID_ILLEGAL_INPUT){
	            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
	            }
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
			return CMD_SUCCESS;
		}	
	}
	id = insID; /*wcl add*/
	if(distributFag == 0){
		if(slot_id != 0){
			vty_out(vty,"<error> slot id should be 0\n");
			return CMD_SUCCESS;
		}	
	}else if(flag == 1){
		slot_id = HostSlotId;
	}
	if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
		vty_out(vty,"<error> local hansi id should be 1 to %d\n",MAX_SLOT_NUM-1);
		return CMD_SUCCESS;
	}	
	if(insID >= MAX_INSTANCE || insID == 0){
		vty_out(vty,"<error> local hansi id should be 1 to %d\n",MAX_INSTANCE-1);
		return CMD_SUCCESS;
	}
		/*wcl add*/
	instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,id);
	if (INSTANCE_NO_CREATED == instRun) {
		/*vty_out(vty, "had instance %d not created!\n", profile);*/
		printf("This local-hansi %d had not created!\n",id);
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	/*end*/

		/*wcl add*/
	char cmd[DCLI_VRRP_DBUSNAME_LEN * 2] = {0};
	
		if (!boot_flag) {
		printf("Are you sure you want to delete local-hansi(yes/no)?");
		fscanf(stdin, "%s", cmd);
		if (!strncasecmp("no", cmd, strlen(cmd))){
			printf("\n%% User cancelled command.\n");
			return CMD_WARNING;
		}
		if (strncasecmp("yes", cmd, strlen(cmd))){
			printf("\nPlease type 'yes' or 'no':"); 
			fscanf(stdin, "%s", cmd);
			if (!strncasecmp("no", cmd, strlen(cmd))){
				printf("\n%% User cancelled command.\n");
				return CMD_WARNING;
			}
			else if (strncasecmp("yes", cmd, strlen(cmd))){
				printf("\n%% Bad choice given.\n");
				return CMD_WARNING;
			}
		}
		printf("\n");
	}
	/*end*/

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_LOCAL_HANSI);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&insID,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,150000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			vty_out(vty,"delete Local hansi %d-%d successfulliy\n",slot_id, insID);
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(set_local_hansi_state_cmd_func,
	  set_local_hansi_state_cmd,
	  "set local-hansi (active|bakup)",
	  CONFIG_STR
	  "Local hansi information\n"
	  "hansi id that you want to config\n"
	 )
{	
	int ret;
	int index = 0;
	int local = 1;
	int state = 0;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	
	if(!strcmp(argv[0],"active")){
		state = 1;
	}
	else if(!strcmp(argv[0],"bakup")){
		state = 0;
	}
	else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,\
						HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_STATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&index,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			vty_out(vty,"set local-hansi %d %s Succesfull!\n",index,argv[0]);
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
/*only uplink*/
DEFUN(config_hmd_uplink_cmd_func,
	config_hmd_uplink_cmd,
	"config uplink IFNAME (A.B.C.D|A.B.C.D/M) priority <1-255>",
	CONFIG_STR
	"Config uplink\n"
	"L3 interface name\n"
	"L3 interface virtual ip\n"
	"Local Hansi priority\n"
	"Local Hansi priority value, valid range [1-255]\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int priority = 0;
	int add = 1;
	int split = 0;
	char *uplink_ifname = NULL;
	unsigned int downlink_mask = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	unsigned long uplink_ip = 0;
	unsigned int uplink_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == uplink_ifname) {
		return CMD_WARNING;
	}
	memset(uplink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(uplink_ifname, argv[0], strlen(argv[0]));
	
	op_ret = check_ip_format((char*)argv[1], &split);
	if (CMD_SUCCESS == op_ret) {
		if (0 == split) {
			/* mask default is 32 */
			uplink_ip = (unsigned long)inet_addr((char*)argv[1]);
			uplink_mask = 32;
		}
		else if (1 == split) {
			op_ret = ip_address_format2ulong((char**)&argv[1], &uplink_ip, &uplink_mask); 
			if (CMD_SUCCESS != op_ret) {
				free(uplink_ifname);
				vty_out(vty, "%% Bad parameter: %s !", argv[1]);
				return CMD_WARNING;
			}
		}
	}
	else {
		free(uplink_ifname);
		uplink_ifname = NULL;
		vty_out(vty, "%%Illegal IP address %s!", argv[1]);
		return CMD_WARNING;		
	}

	priority = strtoul((char *)argv[2],NULL,10);
	if (priority < 1 ||
		priority > 255) {
		vty_out(vty, "%%error priority %s, valid range [1-255]!", argv[2]);
		if(uplink_ifname){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		return CMD_WARNING;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_UPLINK_IF);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &slot_id,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_STRING, &uplink_ifname,
							 DBUS_TYPE_UINT32, &uplink_ip,
							 DBUS_TYPE_UINT32, &uplink_mask,
							 DBUS_TYPE_UINT32, &priority,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{	
        if(0 == op_ret){
            vty_out(vty,"successfull.\n");
		}
		else{
            vty_out(vty,"ret = %d.\n",op_ret);
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(uplink_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*only downlink*/
DEFUN(config_hmd_downlink_cmd_func,
	  config_hmd_downlink_cmd,
	  "config downlink IFNAME (A.B.C.D|A.B.C.D/M) priority <1-255>",
	  CONFIG_STR
	  "Config downlink\n"
	  "L3 interface name\n"
	  "L3 interface virtual ip\n"
	  "Local hansi priority\n"
	  "Local hansi priority value\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int priority = 0;
	int add = 1;
	int split = 0;
	int index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	char* downlink_ifname = NULL;
    unsigned  long downlink_ip = 0;
	unsigned int downlink_mask = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return CMD_WARNING;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,argv[0],strlen(argv[0]));
	
    op_ret = check_ip_format((char*)argv[1],&split);
    if(CMD_SUCCESS == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)argv[1]);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&argv[1],&downlink_ip,&downlink_mask); 
		   if(op_ret==CMD_FAILURE){
			   free(downlink_ifname);
			   return CMD_FAILURE;
		   }
	   }
	}
	else{
	   free(downlink_ifname);
	   return CMD_FAILURE;		
	}


	priority = strtoul((char *)argv[2],NULL,10);
	if((priority < 1)||priority > 255){
        vty_out(vty,"%% Bad parameter : %s !",argv[2]);
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return CMD_WARNING;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_DOWNLINK_IF);

	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&index,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_INVALID);
	vty_out(vty," 1  \n");
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	vty_out(vty,"  2 \n");
	dbus_message_unref(query);
	vty_out(vty," 3  \n");
	if (NULL == reply) {
		cli_syslog_info("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
		vty_out(vty,"	4\n");
        if(0 == op_ret){
            vty_out(vty,"successfull.\n");
		}
		else{
            vty_out(vty,"ret = %d.\n",op_ret);
		}
	} 
	else 
	{	
		vty_out(vty,"	5\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"	6\n");
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty,"   7\n");
	free(downlink_ifname);
	vty_out(vty,"   8\n");
	dbus_message_unref(reply);
	vty_out(vty,"   9\n");
	return CMD_SUCCESS;
}

DEFUN(config_hmd_vgateway_cmd_func,
	config_hmd_vgateway_cmd,
	"config vgateway IFNAME (A.B.C.D|A.B.C.D/M)",
	CONFIG_STR
	"config vgateway ifname ip\n"
	"Config vgateway\n"
	"L3 interface name\n"
	"L3 interface virtual ip\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	int add = 1;
	int split = 0;
	char *vgateway_ifname = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	unsigned long vgateway_ip = 0;
	unsigned int vgateway_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	vgateway_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == vgateway_ifname) {
		return CMD_WARNING;
	}
	memset(vgateway_ifname, 0, MAX_IFNAME_LEN);
	memcpy(vgateway_ifname, argv[0], strlen(argv[0]));
	
	op_ret = dcli_vrrp_check_ip_format((char*)argv[1], &split);
	if (CMD_SUCCESS == op_ret) {
		if (0 == split) {
			/* mask default is 32 */
			vgateway_ip = (unsigned long)inet_addr((char*)argv[1]);
			vgateway_mask = 32;
		}
		else if (1 == split) {
			op_ret = ip_address_format2ulong((char**)&argv[1], &vgateway_ip, &vgateway_mask); 
			if (CMD_SUCCESS != op_ret) {
				free(vgateway_ifname);
				vty_out(vty, "%% Bad parameter: %s !", argv[1]);
				return CMD_WARNING;
			}
		}
	}
	else {
		free(vgateway_ifname);
		vty_out(vty, "%%Illegal IP address %s!", argv[1]);
		return CMD_WARNING;		
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_GATEWAY_IF);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &slot_id,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_STRING, &vgateway_ifname,
							 DBUS_TYPE_UINT32, &vgateway_ip,
							 DBUS_TYPE_UINT32, &vgateway_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{	
        if(0 == op_ret){
            vty_out(vty,"successfull.\n");
		}
		else{
            vty_out(vty,"ret = %d.\n",op_ret);
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(vgateway_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(delete_hmd_iface_cmd_func,
	delete_hmd_iface_cmd,
	"delete (uplink|downlink|vgateway) IFNAME ",
	CONFIG_STR
	"Delete l3 interface virtual ip\n"
	"Config uplink interface\n"
	"Config downlink interface\n"
	"Config vageway interface\n"
	"L3 interface name\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	int add = 1;
	int split = 0;
	char *link_ifname = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	unsigned long vgateway_ip = 0;
	unsigned int vgateway_mask = 0;
	unsigned int link_type = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(argv[0], "uplink", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_UPLINK;
	}else if (!strncmp(argv[0], "downlink", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_DOWNLINK;
	}
	else if(!strncmp(argv[0], "vgateway", strlen(argv[0]))) {
		link_type = DCLI_VRRP_LINK_TYPE_VGATEWAY;
	}else {
		vty_out(vty, "%% Unknown link type %s!\n", argv[0]);
		return CMD_WARNING;
	}

	link_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == link_ifname) {
		return CMD_WARNING;
	}
	memset(link_ifname, 0, MAX_IFNAME_LEN);
	memcpy(link_ifname, argv[1], strlen(argv[1]));
#if 0	
	op_ret = dcli_vrrp_check_ip_format((char*)argv[1], &split);
	if (CMD_SUCCESS == op_ret) {
		if (0 == split) {
			/* mask default is 32 */
			vgateway_ip = (unsigned long)inet_addr((char*)argv[1]);
			vgateway_mask = 32;
		}
		else if (1 == split) {
			op_ret = ip_address_format2ulong((char**)&argv[1], &vgateway_ip, &vgateway_mask); 
			if (CMD_SUCCESS != op_ret) {
				free(link_ifname);
				vty_out(vty, "%% Bad parameter: %s !", argv[1]);
				return CMD_WARNING;
			}
		}
	}
	else {
		free(link_ifname);
		vty_out(vty, "%%Illegal IP address %s!", argv[1]);
		return CMD_WARNING;		
	}
#endif
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_LOCAL_HANSI_LINK_IF);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &slot_id,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &link_type,
							 DBUS_TYPE_STRING, &link_ifname,
						//	 DBUS_TYPE_UINT32, &vgateway_ip,
						//	 DBUS_TYPE_UINT32, &vgateway_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{	
        if(0 == op_ret){
            vty_out(vty,"successfull.\n");
		}
		else{
            vty_out(vty,"ret = %d.\n",op_ret);
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	/*release malloc mem*/
	free(link_ifname);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_local_hansi_service_cmd_func,
	  config_local_hansi_service_cmd,
	  "config service (enable|disable)",
	  CONFIG_STR
	  "Config local_hansi service\n"
	  "Enable local_hansi service\n"
	  "Disable local_hansi service\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int priority = 0;
	int enable = -1;
	int index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if (!strncmp(argv[0], "enable", strlen(argv[0]))) {
        enable = 1;
	}
	else if (!strncmp(argv[0], "disable", strlen(argv[0]))) {
        enable = 0;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	if(HANSI_NODE==vty->node){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	//DBusConnection *dcli_dbus_connection = NULL;
	//ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_DISABLE_ENABLE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
		                     DBUS_TYPE_UINT32,&index,
							 DBUS_TYPE_UINT32,&enable,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,150000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if(0 == op_ret){
            vty_out(vty,"successfull.\n");
		}
		else{
            vty_out(vty,"ret = %d.\n",op_ret);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
/*fengwenchao add 20110616*/
DEFUN(show_hmd_info_cmd_func,
	show_hmd_info_cmd,
	"show hmd info",
	SHOW_STR
	"Show hmd hansi\n"
	"Hmd info"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int index = 0;
	int slot_id = HostSlotId;
	int ret = 0;
	int board_num = 0;
	int i =0;
	int j = 0;
	int k = 0;
	int state = 0;
	unsigned char *ip = NULL;
	struct Hmd_Board_Info_show* hmd_board_head = NULL;
	struct Hmd_Board_Info_show* hmd_board_show = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if(vty->node == CONFIG_NODE){
		index = 0;
	}
	else if (HANSI_NODE==vty->node){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	hmd_board_head = show_hmd_info_show(dcli_dbus_connection, &board_num, &ret);

	if((hmd_board_head !=NULL)&&(ret == 0))
	{
		vty_out(vty,"BOARDNum:%-5d  \n",board_num);
		vty_out(vty,"========================================================================== \n");
		for(i = 0; i < board_num; i++)
		{
			if(hmd_board_show == NULL)
				hmd_board_show = hmd_board_head->hmd_board_list;
			else 
				hmd_board_show = hmd_board_show->next;

			if(hmd_board_show == NULL)
				break;

			vty_out(vty,"-------------------------------------------------------------------------\n");
			vty_out(vty,"BOARD information:\n");
			vty_out(vty,"slot_no:%-5d  \n",hmd_board_show->slot_no);
			vty_out(vty,"InstNum:%-5d  \n",hmd_board_show->InstNum);
			vty_out(vty,"LocalInstNum:%-5d  \n",hmd_board_show->LocalInstNum);
			vty_out(vty,"-------------------------------------------------------------------------\n");
			vty_out(vty,"Hmd_Inst  information:\n");

			for(j = 0; j <hmd_board_show->InstNum; j++)
			{
				state = hmd_board_show->Hmd_Inst[j]->isActive;
				vty_out(vty,"Inst_ID:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_ID);
				vty_out(vty,"slot_no:%-5d  \n",hmd_board_show->Hmd_Inst[j]->slot_no);
				vty_out(vty,"InstState:%-5d  \n",hmd_board_show->Hmd_Inst[j]->InstState);
				vty_out(vty,"Inst_UNum:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_UNum);
				vty_out(vty,"Inst_DNum:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_DNum);
				vty_out(vty,"Inst_GNum:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_GNum);
				vty_out(vty,"priority:%-5d  \n",hmd_board_show->Hmd_Inst[j]->priority);
				vty_out(vty,"isActive:%-5d  \n",hmd_board_show->Hmd_Inst[j]->isActive);
				vty_out(vty,"isActive state:%s  \n",(state==3)?"Master":((state==2)?"Backup":"Disable"));

				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Hb  information:\n");
				vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Inst[j]->Inst_Hb.ifname);
				ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Hb.real_ip);
				vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
				ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Hb.vir_ip);
				vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
				ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Hb.remote_r_ip);
				vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);

				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Uplink  information:\n");
				for(k = 0; k < hmd_board_show->Hmd_Inst[j]->Inst_UNum; k++)
				{
					vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Inst[j]->Inst_Uplink[k].ifname);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Uplink[k].real_ip);
					vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Uplink[k].vir_ip);
					vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Uplink[k].remote_r_ip);
					vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_Uplink[k].mask);
					vty_out(vty,"-------------------------------------------------------------------------\n");

				}

				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Downlink  information:\n");

				for(k = 0; k < hmd_board_show->Hmd_Inst[j]->Inst_DNum; k++)
				{
					vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Inst[j]->Inst_Downlink[k].ifname);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Downlink[k].real_ip);
					vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Downlink[k].vir_ip);
					vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Downlink[k].remote_r_ip);
					vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_Downlink[k].mask);
					vty_out(vty,"-------------------------------------------------------------------------\n");

				}

				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Gateway  information:\n");

				for(k = 0; k < hmd_board_show->Hmd_Inst[j]->Inst_GNum; k++)
				{
					vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Inst[j]->Inst_Gateway[k].ifname);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Gateway[k].real_ip);
					vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Gateway[k].vir_ip);
					vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Inst[j]->Inst_Gateway[k].remote_r_ip);
					vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Inst[j]->Inst_Gateway[k].mask);
					vty_out(vty,"-------------------------------------------------------------------------\n");

				}				

			}
			
			vty_out(vty,"-------------------------------------------------------------------------\n");
			vty_out(vty,"Hmd_Local_Inst  information:\n");		
				
			for(j = 0; j <hmd_board_show->LocalInstNum; j++)
			{
				state = hmd_board_show->Hmd_Local_Inst[j]->isActive;
				vty_out(vty,"Inst_ID:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_ID);
				vty_out(vty,"slot_no:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->slot_no);
				vty_out(vty,"InstState:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->InstState);
				vty_out(vty,"Inst_UNum:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_UNum);
				vty_out(vty,"Inst_DNum:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_DNum);
				vty_out(vty,"Inst_GNum:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_GNum);
				vty_out(vty,"priority:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->priority);
				vty_out(vty,"isActive:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->isActive);				
				vty_out(vty,"isActive state:%s  \n",(state==3)?"Master":((state==2)?"Backup":"Disable"));

				/*vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Hb  information:\n");
				vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Local_Inst[j]->ifname);
				vty_out(vty,"real_ip:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->real_ip);
				vty_out(vty,"vir_ip:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->vir_ip);
				vty_out(vty,"remote_r_ip:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->remote_r_ip);
				vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->umask);
				vty_out(vty,"dmask:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->dmask);*/

				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Uplink  information:\n");

				for(k = 0; k < hmd_board_show->Hmd_Local_Inst[j]->Inst_UNum; k++)
				{
					vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_Uplink[k].ifname);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Uplink[k].real_ip);
					vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Uplink[k].vir_ip);
					vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Uplink[k].remote_r_ip);
					vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_Uplink[k].mask);
					vty_out(vty,"-------------------------------------------------------------------------\n");				
				}
				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Downlink  information:\n");				
				for(k = 0; k < hmd_board_show->Hmd_Local_Inst[j]->Inst_DNum; k++)
				{
					vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_Downlink[k].ifname);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Downlink[k].real_ip);
					vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Downlink[k].vir_ip);
					vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Downlink[k].remote_r_ip);
					vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_Downlink[k].mask);
					vty_out(vty,"-------------------------------------------------------------------------\n");					
				}
				vty_out(vty,"-------------------------------------------------------------------------\n");
				vty_out(vty,"Inst_Gateway  information:\n");				
				for(k = 0; k < hmd_board_show->Hmd_Local_Inst[j]->Inst_GNum; k++)
				{
					vty_out(vty,"ifname:%-5s  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_Gateway[k].ifname);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Gateway[k].real_ip);
					vty_out(vty,"real_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Gateway[k].vir_ip);
					vty_out(vty,"vir_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					ip = &(hmd_board_show->Hmd_Local_Inst[j]->Inst_Gateway[k].remote_r_ip);
					vty_out(vty,"remote_r_ip:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					vty_out(vty,"umask:%-5d  \n",hmd_board_show->Hmd_Local_Inst[j]->Inst_Gateway[k].mask);
					vty_out(vty,"-------------------------------------------------------------------------\n");					
				}

			}

		vty_out(vty,"========================================================================== \n");
		}
	}

	if(ret == 0)
	{
		
	}
	else if(ret == HMD_DBUS_ID_NO_EXIST)
	{
		vty_out(vty," there is no board \n");				
	}
	else
	{
		vty_out(vty," <error> ret = %d\n",ret);	
	}
	dcli_free_HmdBoardInfo(hmd_board_head);
	
	return CMD_SUCCESS;
}
/*fengwenchao add end*/

DEFUN(show_hmd_runconfig_cmd_func,
	show_hmd_runconfig_cmd,
	"show hmd hansi running-config",
	SHOW_STR
	"Show hmd hansi\n"
	"Hmd running configuration"
	"Hmd hansi id\n"
)
{
	unsigned int profile = 0;
	unsigned int slotid = 0;
	unsigned int islocal = 1;
	if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
		slotid = vty->slotindex;
		islocal = 0;
	}else if(LOCAL_HANSI_NODE == vty->node){
		profile = (unsigned int)(vty->index);
		slotid = vty->slotindex;
	}

	dcli_hmd_show_hansi_running_cfg(vty, profile,slotid,islocal);

	return CMD_SUCCESS;
}

/*fengwenchao add 20130412 for hmd timer config save*/
DEFUN(set_hmd_timer_config_save_cmd_func,
	  set_hmd_timer_config_save_cmd,
	  "set hmd-timer-config-save (enable|disable)",
	  CONFIG_STR
	  "Local hansi information\n"
	  "hansi id that you want to config\n"
	 )
{
	int ret;
	int index = 0;
	int local = 1;
	int state = 0;
	int slot_id = HostSlotId;
	
	if(!strcmp(argv[0],"enable")){
		state = 1;
	}
	else if(!strcmp(argv[0],"disable")){
		state = 0;
	}
	else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	if(vty->node == HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = hmd_timer_config_save_state(dcli_dbus_connection,slot_id,state);
	if(ret == 0){
		vty_out(vty,"set hmd-timer-config-save %s Succesfull!\n",argv[0]);
	}
	else if(ret == HMD_DBUS_IS_NOT_MASTER)
		vty_out(vty,"your slot is not master \n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;

}
DEFUN(set_hmd_timer_config_save_time_cmd_func,
	  set_hmd_timer_config_save_time_cmd,
	  "set hmd-timer-config-save time TIMER",
	  CONFIG_STR
	  "Local hansi information\n"
	  "hansi id that you want to config\n"
	 )
{
	int ret;
	int index = 0;
	int local = 1;
	int state = 0;
	int slot_id = HostSlotId;
	int timer = 0;
	ret = parse_int_ID((char *)argv[0],&timer);
	
	if(-1 == ret)
	{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	if((timer >7200)||(timer < 60))
	{
		vty_out(vty,"<error> input parameter should be 60~7200\n",argv[0]);
		return CMD_SUCCESS;
	}
	
	if(vty->node == HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = hmd_timer_config_save_timer(dcli_dbus_connection,slot_id,timer);
	if(ret == 0){
		vty_out(vty,"set hmd-timer-config-save time %s Succesfull!\n",argv[0]);
	}
	else if(ret == HMD_DBUS_TIMER_CONFIG_SAVE_STATE_ENABLE)
	{
		vty_out(vty,"<error>hmd-timer-config-save enable now,please disable it first!\n");
	}
	else if(ret == HMD_DBUS_IS_NOT_MASTER)
	{
		vty_out(vty,"<error>is not master slot !\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}

/*fengwenchao add end*/

DEFUN(set_hansi_check_state_cmd_func,
	  set_hansi_check_state_cmd,
	  "set hansi-check (enable|disable)",
	  CONFIG_STR
	  "Local hansi information\n"
	  "hansi id that you want to config\n"
	 )
{	
	int ret;
	int index = 0;
	int local = 1;
	int state = 0;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	
	if(!strcmp(argv[0],"enable")){
		state = 1;
	}
	else if(!strcmp(argv[0],"disable")){
		state = 0;
	}
	else{
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	if(vty->node == HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = hmd_wireless_check_setting(dcli_dbus_connection,slot_id,state);
	if(ret == 0){
		vty_out(vty,"set local-hansi %d %s Succesfull!\n",index,argv[0]);
	}
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}

/*zhaoruijia,20100913,for ap auto update,start*/
DEFUN(service_tftp_state_cmd_func,
	     service_tftp_state_cmd,
	     "service tftp (enable|disable)",
	     CONFIG_STR
	     "Enable or Disable tftpd-hpa service\n"
	  
	  )
{
    
	int ret;
    DBusMessage *query = NULL;
	DBusMessage  *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int i = 0;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned int ap_auto_update_service_tftp = 0;
	
	if (!strcmp(argv[0],"enable"))
	{
		ap_auto_update_service_tftp = 1;
	}
	else if (!strcmp(argv[0],"disable"))
	{
		
		ap_auto_update_service_tftp = 0;
	}
    else
	{
		vty_out(vty, "Please input enable or disable.\n");
		return CMD_SUCCESS;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;
	   query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SERVICE_TFTP_SWITCH);

	   dbus_error_init(&err);

	   dbus_message_append_args(query,
	                            DBUS_TYPE_UINT32,&ap_auto_update_service_tftp,
	                            DBUS_TYPE_INVALID);

	   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	       
	   dbus_message_unref(query);
	   if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			

			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
	    if (ret == WID_DBUS_SUCCESS)
	    {
	        if(1== ap_auto_update_service_tftp){
	          vty_out(vty,"slot %d service tftp enable successfull \n",i);
			}
			else{
	          vty_out(vty,"slot %d service tftp disable successfull \n",i);
			}
		}
		else if(ret == WID_DBUS_ERROR)
		{
	       vty_out(vty,"<error>slot %d service tftp set failed \n",i);
	       
	    }


		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;

}


DEFUN(show_service_tftp_state_cmd_func,
	     show_service_tftp_state_cmd,
		 "show service tftp",
		 CONFIG_STR
		 "show tftpd-hpa service state \n"
	  
	  )
{

  int ret = 0;
  DBusMessage *query = NULL;
  DBusMessage  *reply = NULL;	
  DBusMessageIter	 iter;
  DBusError err;
  int index = 0;
  char BUSNAME[PATH_LEN];
  char OBJPATH[PATH_LEN];
  char INTERFACE[PATH_LEN];
  unsigned int ap_auto_update_service_tftp = 0;
  query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_SERVICE_TFTP_SWITCH);
  dbus_error_init(&err);
  reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
  
  dbus_message_unref(query);
	  
  if (NULL == reply)
  {
		  cli_syslog_info("<error> failed get reply.\n");
		  if (dbus_error_is_set(&err))
		  {
			  cli_syslog_info("%s raised: %s",err.name,err.message);
			  dbus_error_free_for_dcli(&err);
		  }
		  
  
		  return CMD_SUCCESS;
   }

 dbus_message_iter_init(reply,&iter);
 dbus_message_iter_get_basic(&iter,&ap_auto_update_service_tftp);


  vty_out(vty,"==============================================================================\n");
 
   if(1 == ap_auto_update_service_tftp){
       vty_out(vty, "SERVICE TFTP: enable \n");
  }
  else if(0 == ap_auto_update_service_tftp){
       vty_out(vty, "SERVICE TFTP: disable \n");
  }
  else{
       vty_out(vty, "SERVICE TFTP: failed  \n");
  }
  
  vty_out(vty,"==============================================================================\n");
  
  dbus_message_unref(reply);
  return CMD_SUCCESS;
}
/*zhaoruijia,20100913,for ap auto update,end*/
DEFUN(service_ftp_state_cmd_func,
	     service_ftp_state_cmd,
	     "service ftp (enable|disable)",
	     CONFIG_STR
	     "Enable or Disable pure-ftpd service\n"
	  
	  )
{
    
	int ret;
    DBusMessage *query = NULL;
	DBusMessage  *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned int service_ftp_state = 0;
	DBusConnection *dcli_dbus_connection = NULL;
	int i = 0;	
	if (!strcmp(argv[0],"enable"))
	{
		service_ftp_state = 1;
	}
	else if (!strcmp(argv[0],"disable"))
	{
		service_ftp_state = 0;
	}
    else
	{
		vty_out(vty, "Please input enable or disable.\n");
		return CMD_SUCCESS;
	}
	for(i = 1; i < MAX_SLOT_NUM; i++){
		if((NULL != dbus_connection_dcli[i])&&(NULL != dbus_connection_dcli[i]->dcli_dbus_connection)){
			dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
		}else
			continue;

	   query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE, HMD_DBUS_CONF_METHOD_SERVICE_FTP_SWITCH);

	   dbus_error_init(&err);

	   dbus_message_append_args(query,
	                            DBUS_TYPE_UINT32,&service_ftp_state,
	                            DBUS_TYPE_INVALID);

	   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	       
	   dbus_message_unref(query);
	   if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			

			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
	    if (ret == WID_DBUS_SUCCESS)
	    {
	        if(1== service_ftp_state){
	          vty_out(vty,"slot %d service ftp enable successfull \n",i);
			}
			else{
	          vty_out(vty,"slot %d service ftp disable successfull \n",i);
			}
		}
		else if(ret == WID_DBUS_ERROR)
		{
	       vty_out(vty,"<error>slot %d service ftp set failed \n",i);
	    }


		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;

}


DEFUN(show_service_ftp_state_cmd_func,
	     show_service_ftp_state_cmd,
		 "show service ftp",
		 CONFIG_STR
		 "show pure-ftpd service state \n"
	  
	  )
{

  int ret = 0;
  DBusMessage *query = NULL;
  DBusMessage  *reply = NULL;	
  DBusMessageIter	 iter;
  DBusError err;
  int index = 0;
  char BUSNAME[PATH_LEN];
  char OBJPATH[PATH_LEN];
  char INTERFACE[PATH_LEN];
  unsigned int service_ftp_state = 0;
  query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_SERVICE_FTP_SWITCH);
  dbus_error_init(&err);
  reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
  
  dbus_message_unref(query);
	  
  if (NULL == reply)
  {
		  cli_syslog_info("<error> failed get reply.\n");
		  if (dbus_error_is_set(&err))
		  {
			  cli_syslog_info("%s raised: %s",err.name,err.message);
			  dbus_error_free_for_dcli(&err);
		  }
		  
  
		  return CMD_SUCCESS;
   }

 dbus_message_iter_init(reply,&iter);
 dbus_message_iter_get_basic(&iter,&service_ftp_state);


  vty_out(vty,"==============================================================================\n");
 
   if(1 == service_ftp_state){
       vty_out(vty, "SERVICE FTP: enable \n");
  }
  else if(0 == service_ftp_state){
       vty_out(vty, "SERVICE FTP: disable \n");
  }
  else{
       vty_out(vty, "SERVICE FTP: failed  \n");
  }
  
  vty_out(vty,"==============================================================================\n");
  
  dbus_message_unref(reply);
  return CMD_SUCCESS;
}

DEFUN(show_ap_img_file_cmd_func,
	     show_ap_img_file_cmd,
		 "show ap_img_file",
		 CONFIG_STR
		 "show ap img file\n"
	  
	  )
{

  int ret = 0;
  DBusMessage *query = NULL;
  DBusMessage  *reply = NULL;	
  DBusMessageIter	 iter;
  DBusError err;
  int index = 0;
  char BUSNAME[PATH_LEN];
  char OBJPATH[PATH_LEN];
  char INTERFACE[PATH_LEN];
  int  slot_id = 0;
  char *info = NULL;
  if(vty->node == HANSI_NODE){
	  index = vty->index;
	  slot_id = vty->slotindex;
  }else if (vty->node == LOCAL_HANSI_NODE){
	  index = vty->index;
	  slot_id = vty->slotindex;
  }
  DBusConnection *dcli_dbus_connection = NULL;
  ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

  query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_AP_IMG_FILES);
  dbus_error_init(&err);
  reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
  
  dbus_message_unref(query);
	  
  if (NULL == reply)
  {
		  cli_syslog_info("<error> failed get reply.\n");
		  if (dbus_error_is_set(&err))
		  {
			  cli_syslog_info("%s raised: %s",err.name,err.message);
			  dbus_error_free_for_dcli(&err);
		  }
		  
  
		  return CMD_SUCCESS;
   }

 dbus_message_iter_init(reply,&iter);
 dbus_message_iter_get_basic(&iter,&info);


  vty_out(vty,"==============================================================================\n");
 
  vty_out(vty, "%s\n",info);
  
  vty_out(vty,"==============================================================================\n");
  
  dbus_message_unref(reply);
  return CMD_SUCCESS;
}

/*wuwl add for clear binding interface flag*/  //fengwenchao copy from 1318 for AXSSZFI-839
DEFUN(set_ac_clear_wid_binding_interface_flag_func,
		  set_ac_clear_wid_binding_interface_flag_cmd,
		  "clear wireless binding interfaceflag IFNAME",
		  "clear interface binding config\n"
		  "wireless binding interface name\n"
	 )
{
	int ret,ret1;

	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;

	int len = 0;
	char *name;
	ret = WID_DBUS_SUCCESS;
	len = strlen(argv[0]);
	if(len > 15)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}

	name = (char*)malloc(strlen(argv[0])+1);
	memset(name, 0, strlen(argv[0])+1);
	memcpy(name, argv[0], strlen(argv[0]));	
	
	int index = 0;
	int  slot_id = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_WID_CLEAR_BINDING_INTERFACE_FLAG);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	
		
	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);



	vty_out(vty,"set wireless-control dynamic_ap_login_binding wlan %s successfully\n",argv[0]);

	free(name);
	name = NULL;
	return CMD_SUCCESS;			
}

/*wuwl add for set hansi bakup forever */ 
DEFUN(set_set_hansi_bak_forever_func,
		set_set_hansi_bak_forever_cmd,
		"config (hansi|localhansi) bak forever (enable|disable)",
		"config vrrp\n"
		"config vrrp settings\n"
		"config vrrp hansi bak configuration\n"
	 )
{
	int ret,ret1;

	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int h_type = 0;
	unsigned int type = 0;
	ret = WID_DBUS_SUCCESS;
	if (!strcmp(argv[0],"hansi"))
	{
		h_type = 0;
	}
	else if (!strcmp(argv[0],"localhansi"))
	{
		h_type = 1;
	}
    else
	{
		vty_out(vty, "Please input hansi or localhansi.\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[1],"enable"))
	{
		type = 1;
	}
	else if (!strcmp(argv[1],"disable"))
	{
		type = 0;
	}
    else
	{
		vty_out(vty, "Please input enable or disable.\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int  slot_id = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slot_id = vty->slotindex;
	}
	//DBusConnection *dcli_dbus_connection = NULL;
	//ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_VRRP_BAKUP_FOREVER);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_UINT32,&index,
								DBUS_TYPE_UINT32,&h_type,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	
		
	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(0 == ret)
		vty_out(vty,"config %s hansi bak forever %s successfully\n",argv[0],argv[1]);
	else if(HMD_DBUS_INVALID_SLOT_NUM == ret)
		vty_out(vty,"slot id(%d) not host slot id.\n",slot_id);
	else if(HMD_DBUS_ID_NO_EXIST == ret)
		vty_out(vty,"invalid slot id(%d).\n",slot_id);
	else if(HMD_DBUS_HANSI_ID_NOT_EXIST == ret)
		vty_out(vty,"Hansi[%d-%d] not exsit.\n",slot_id,index);
	else if(HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST == ret)
		vty_out(vty,"Lhansi[%d-%d] not exsit.\n",slot_id,index);
	else
		vty_out(vty,"config %s hansi bak forever %s fail,ret(%d).\n",argv[0],argv[1],ret);

	return CMD_SUCCESS;			
}

DEFUN(show_hmd_bak_forever_cmd_func,
	     show_hmd_bak_forever_cmd,
		 "show hmd bak forever info",
		 CONFIG_STR
		 "show hmd bak forever info\n"
	  
	  )
{

	int ret = 0;
	DBusMessage *query = NULL;
	DBusMessage  *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int  slot_id = 0;
	int  local = 0;
	int sw_state = 0;
	int WidReadyTimes = 0;
	int AsdReadyTimes = 0;
	time_t WidLastReadyTime = 0;
	time_t AsdLastReadyTime = 0;
	int i = 1;
	if(vty->node == HANSI_NODE){
	  index = vty->index;
	  slot_id = vty->slotindex;
	  local = vty->local;
	}else if (vty->node == LOCAL_HANSI_NODE){
	  index = vty->index;
	  slot_id = vty->slotindex;
	  local = vty->local;
	}else{
	 	vty_out(vty, "invalid node(%d),only can be HANSI_NODE or LOCAL_HANSI_NODE.\n",vty->node);
		return CMD_SUCCESS;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	for(i = 1; i < MAX_SLOT_NUM; i++){
		slot_id = i;
		dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(NULL == dcli_dbus_connection)
			continue;
		query = NULL;
		reply = NULL;	
		query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_HMD_BAK_FOREVER_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
							  DBUS_TYPE_UINT32,&slot_id,
							  DBUS_TYPE_UINT32,&index,
							  DBUS_TYPE_UINT32,&local,
							  DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

		dbus_message_unref(query);
		  
		if (NULL == reply)
		{
			  cli_syslog_info("<error> failed get reply.\n");
			  if (dbus_error_is_set(&err))
			  {
				  cli_syslog_info("%s raised: %s",err.name,err.message);
				  dbus_error_free_for_dcli(&err);
			  }
			  

			  return CMD_SUCCESS;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		vty_out(vty,"==============================================================================\n");
		if(0 == ret){
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&sw_state);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&WidReadyTimes);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&AsdReadyTimes);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&WidLastReadyTime);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&AsdLastReadyTime);
			vty_out(vty,"HANSI %d-%d.\n",slot_id,index);
			vty_out(vty,"bakup forever:%s.\n",(sw_state==1)?"enable":"disable");
			vty_out(vty,"WidReadyTimes:%d.\n",WidReadyTimes);
			vty_out(vty,"AsdReadyTimes:%d.\n",AsdReadyTimes);
			time_t now,online_time;
			time(&now);
			online_time = now - (WidLastReadyTime);
			int hour,min,sec;
			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
			vty_out(vty,"WidLastReadyTime:	%s",ctime(&WidLastReadyTime));
			vty_out(vty,"WidLastReadyTime:	%02d:%02d:%02d\n",hour,min,sec);

			now = online_time = 0;
			time(&now);
			online_time = now - (AsdLastReadyTime);
			hour = min = sec = 0;
			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
			vty_out(vty,"AsdLastReadyTime:	%s",ctime(&AsdLastReadyTime));
			vty_out(vty,"AsdLastReadyTime:	%02d:%02d:%02d\n",hour,min,sec);
		}else{
			if(HMD_DBUS_INVALID_SLOT_NUM == ret)
			  vty_out(vty,"slot id(%d) not host slot id.\n",slot_id);
			else if(HMD_DBUS_ID_NO_EXIST == ret)
			  vty_out(vty,"invalid slot id(%d).\n",slot_id);
			else if(HMD_DBUS_HANSI_ID_NOT_EXIST == ret)
			  vty_out(vty,"Hansi[%d-%d] not exsit.\n",slot_id,index);
			else if(HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST == ret)
			  vty_out(vty,"Lhansi[%d-%d] not exsit.\n",slot_id,index);
			else
			  vty_out(vty,"fail(index:%d,slotid:%d,local:%d,ret:%d).\n",index,slot_id,local,ret);
		}
		vty_out(vty,"==============================================================================\n");

		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
	}


void dcli_local_hansi_init(void) {	
	
	install_node (&local_hansi_node, dcli_local_hansi_show_running_config, "LOCAL_HANSI_NODE");
	install_default(LOCAL_HANSI_NODE);

	
	install_element(CONFIG_NODE,&config_local_hansi_cmd);
	install_element(CONFIG_NODE,&delete_local_hansi_cmd);
	install_element(CONFIG_NODE,&show_hmd_runconfig_cmd);
	install_element(CONFIG_NODE,&show_hmd_info_cmd);
	install_element(CONFIG_NODE,&set_hansi_check_state_cmd);
	install_element(CONFIG_NODE,&show_hmd_bak_forever_cmd);
	install_element(LOCAL_HANSI_NODE,&set_hansi_check_state_cmd);
	install_element(HANSI_NODE,&set_hansi_check_state_cmd);
	install_element(LOCAL_HANSI_NODE,&set_local_hansi_state_cmd);
	install_element(HANSI_NODE,&show_ap_img_file_cmd);
	install_element(HANSI_NODE,&set_ac_clear_wid_binding_interface_flag_cmd);//fengwenchao copy from 1318 for AXSSZFI-839
	install_element(HANSI_NODE,&set_set_hansi_bak_forever_cmd);
	install_element(HANSI_NODE,&show_hmd_runconfig_cmd);
	install_element(HANSI_NODE,&show_hmd_bak_forever_cmd);
	install_element(HANSI_NODE,&set_hmd_timer_config_save_time_cmd);  //fengwenchao add 20130415 for hmd timer config save
	install_element(HANSI_NODE,&set_hmd_timer_config_save_cmd);//fengwenchao add 20130415 for hmd timer config save
	install_element(LOCAL_HANSI_NODE,&show_ap_img_file_cmd);
	install_element(LOCAL_HANSI_NODE,&show_hmd_bak_forever_cmd);
	install_element(LOCAL_HANSI_NODE,&config_hmd_uplink_cmd);
	install_element(LOCAL_HANSI_NODE,&config_hmd_downlink_cmd);
	install_element(LOCAL_HANSI_NODE,&config_hmd_vgateway_cmd);
	install_element(LOCAL_HANSI_NODE,&config_local_hansi_service_cmd);
	install_element(LOCAL_HANSI_NODE,&show_hmd_runconfig_cmd);
	install_element(LOCAL_HANSI_NODE,&delete_hmd_iface_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ac_clear_wid_binding_interface_flag_cmd);//fengwenchao copy from 1318 for AXSSZFI-839
	
	install_element(CONFIG_NODE,&service_tftp_state_cmd);
	install_element(CONFIG_NODE,&show_service_tftp_state_cmd);
	install_element(CONFIG_NODE,&service_ftp_state_cmd);
	install_element(CONFIG_NODE,&show_service_ftp_state_cmd);
	install_element(LOCAL_HANSI_NODE,&set_set_hansi_bak_forever_cmd);
	install_element(LOCAL_HANSI_NODE,&set_hmd_timer_config_save_time_cmd);  //fengwenchao add 20130415 for hmd timer config save
	install_element(LOCAL_HANSI_NODE,&set_hmd_timer_config_save_cmd);//fengwenchao add 20130415 for hmd timer config save	
	return;
}

#endif
