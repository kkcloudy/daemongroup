#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include "hmd.h"
#include "HmdDbus.h"
#include "HmdThread.h"
#include "HmdLog.h"
#include "hmd/hmdpub.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "hmd/hmdpub.h"
#include "HmdManagement.h"
#include "HmdTimeLib.h"
#include "HmdMonitor.h"
#include "HmdThread.h"
#include <dirent.h>  /*fengwenchao add 20120228 for AXSSZFI-680*/
#include "HmdStateListen.h"
#include "HmdDbusHandler.h"
static DBusConnection * hmd_dbus_connection = NULL;
static DBusConnection * hmd_dbus_connection2 = NULL;
//static DBusConnection * hmd_dbus_connection_t[THREAD_NUM];
int HmdDBUS_MSGQ;

void hmd_realease(struct Hmd_Board_Info **HMD_BOARD,int ID){
	if((HMD_BOARD == NULL)||(HMD_BOARD[ID] == NULL)){
		return;		
	}else{
		free(HMD_BOARD[ID]);
		HMD_BOARD[ID] = NULL;
	}
}
char * inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize )
{
	if(ip_str == NULL)
	{
		return "";
	}
	snprintf( ip_str, buffsize, "%u.%u.%u.%u", ip_addr>>24, (ip_addr&0xff0000)>>16,
				(ip_addr&0xff00)>>8, (ip_addr&0xff) );
				
	return ip_str;
}

void show_running_config_hmd_local(struct Hmd_L_Inst_Mgmt **LOCALHANSI,int i,char *cursor,char **showStr2,char *showStr_new,int *totalLen_T,int *str_len_T, int *slotID){
	int j = 0;
	if((i < 0) || (i >= MAX_INSTANCE) || (LOCALHANSI == NULL) || (LOCALHANSI[i] == NULL))
		return ;
	int Inst_ID = LOCALHANSI[i]->Inst_ID;
	char uplink_ip[17] = {0},downlink_ip[17] = {0},vgateway_ip[17] = {0};
	unsigned int uplink_mask = 0,downlink_mask = 0,vmask = 0;
	int totalLen = *totalLen_T;
	int str_len = *str_len_T;
	char *showStr = *showStr2;
	hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);

	if(totalLen + 1024 > str_len) {
		str_len *= 2;
		showStr_new = (char*)realloc(showStr,str_len);
		if(showStr_new == NULL){
			hmd_syslog_info("hmd show running realloc failed\n");
			return ;
		}else {
			showStr = showStr_new;
			*showStr2 = showStr;
			memset(showStr+str_len/2,0,str_len/2);
			showStr_new = NULL;
		}
		hmd_syslog_debug_debug(HMD_DBUS,"hmd show running totalLen %d realloc strlen %d\n",totalLen,str_len);
	}
	cursor = showStr + totalLen;
	hmd_syslog_info("%s,line=%d,LOCALHANSI[%d]->isActive=%d.\n",__func__,__LINE__,i,LOCALHANSI[i]->isActive);
	if((i < MAX_INSTANCE)&&LOCALHANSI&&LOCALHANSI[i]){
		totalLen += sprintf(cursor,"config local-hansi %d-%d\n",slotID[i],Inst_ID);
		cursor = showStr + totalLen;
	}
	if((i < MAX_INSTANCE)&&LOCALHANSI&&LOCALHANSI[i]&&(LOCALHANSI[i]->isActive == 1)){
		
		hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		totalLen += sprintf(cursor,"set local-hansi active\n");
		cursor = showStr + totalLen;	
	}
	for(j=0;(j < MAX_IFNAME_NUM)&&(j < LOCALHANSI[i]->Inst_DNum);j++){
		
		hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		memset(downlink_ip,0,17);
		downlink_mask = 0;
		if((i < MAX_INSTANCE)&&(LOCALHANSI[i])){
			
			hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
			sprintf(downlink_ip,"%d.%d.%d.%d",
				((LOCALHANSI[i]->Inst_Downlink[j].vir_ip & 0xff000000) >> 24),
				((LOCALHANSI[i]->Inst_Downlink[j].vir_ip & 0xff0000) >> 16),
				((LOCALHANSI[i]->Inst_Downlink[j].vir_ip & 0xff00) >> 8),
				( LOCALHANSI[i]->Inst_Downlink[j].vir_ip & 0xff));
				downlink_mask = LOCALHANSI[i]->Inst_Downlink[j].mask;
			if(0 == downlink_mask){
				
				hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				totalLen += sprintf(cursor,"config downlink %s %s priority %d \n",LOCALHANSI[i]->Inst_Downlink[j].ifname,downlink_ip,LOCALHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}else{
			
			hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				totalLen += sprintf(cursor,"config downlink %s %s/%d priority %d \n",LOCALHANSI[i]->Inst_Downlink[j].ifname,downlink_ip,downlink_mask,LOCALHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}
		}
	}
	for(j=0;(j < MAX_IFNAME_NUM)&&(j < LOCALHANSI[i]->Inst_UNum);j++){
		memset(uplink_ip,0,17);
		uplink_mask = 0;
		if((i < MAX_INSTANCE)&&(LOCALHANSI)&&(LOCALHANSI[i])){
			sprintf(uplink_ip,"%d.%d.%d.%d",
				((LOCALHANSI[i]->Inst_Uplink[j].vir_ip & 0xff000000) >> 24),
				((LOCALHANSI[i]->Inst_Uplink[j].vir_ip & 0xff0000) >> 16),
				((LOCALHANSI[i]->Inst_Uplink[j].vir_ip & 0xff00) >> 8),
				( LOCALHANSI[i]->Inst_Uplink[j].vir_ip & 0xff));
				uplink_mask = LOCALHANSI[i]->Inst_Uplink[j].mask;
			if(0 == uplink_mask){
				totalLen += sprintf(cursor,"config uplink %s %s priority %d \n",LOCALHANSI[i]->Inst_Uplink[j].ifname,uplink_ip,LOCALHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}else{
				totalLen += sprintf(cursor,"config uplink %s %s/%d priority %d \n",LOCALHANSI[i]->Inst_Uplink[j].ifname,uplink_ip,uplink_mask,LOCALHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}
		}
	}
	for(j=0;(j < MAX_IFNAME_NUM)&&(j < LOCALHANSI[i]->Inst_GNum);j++){
		hmd_syslog_info("LOCALHANSI[i]->Inst_GNum =%d.\n",LOCALHANSI[i]->Inst_GNum);	//for test
		hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		memset(vgateway_ip,0,17);
		vmask = 0;
		if((i < MAX_INSTANCE)&&(LOCALHANSI)&&(LOCALHANSI[i])){
			sprintf(vgateway_ip,"%d.%d.%d.%d",
				((LOCALHANSI[i]->Inst_Gateway[j].vir_ip & 0xff000000) >> 24),
				((LOCALHANSI[i]->Inst_Gateway[j].vir_ip & 0xff0000) >> 16),
				((LOCALHANSI[i]->Inst_Gateway[j].vir_ip & 0xff00) >> 8),
				( LOCALHANSI[i]->Inst_Gateway[j].vir_ip & 0xff));
				vmask = LOCALHANSI[i]->Inst_Gateway[j].mask;
			if(0 == vmask){
				hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				totalLen += sprintf(cursor,"config vgateway %s %s \n",LOCALHANSI[i]->Inst_Gateway[j].ifname,vgateway_ip);
				cursor = showStr + totalLen;	
			}else{
				hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				totalLen += sprintf(cursor,"config vgateway %s %s/%d \n",LOCALHANSI[i]->Inst_Gateway[j].ifname,vgateway_ip,vmask);
				cursor = showStr + totalLen;	
			}
		}
	}
	if((i < MAX_INSTANCE)&&LOCALHANSI&&LOCALHANSI[i]&&(LOCALHANSI[i]->HmdBakForever == 1)){
		totalLen += sprintf(cursor,"config localhansi bak forever enable\n");
		cursor = showStr + totalLen;	
	}
	if((i < MAX_INSTANCE)&&LOCALHANSI&&LOCALHANSI[i]&&(LOCALHANSI[i]->InstState == 1)){
		totalLen += sprintf(cursor,"config service enable\n");
		cursor = showStr + totalLen;	
	}


	totalLen += sprintf(cursor,"exit\n");
	cursor = showStr + totalLen;	

	*totalLen_T = totalLen;
	*str_len_T = str_len;
	return;
}

void show_running_config_hmd_remote(struct Hmd_Inst_Mgmt ** REMOTEHANSI,int i,char *cursor,char **showStr2,char *showStr_new,int *totalLen_T,int *str_len_T){
	int j = 0;
	int totalLen = *totalLen_T;
	int str_len = *str_len_T;
	char *showStr = *showStr2;
	char uplink_ip[17] = {0},downlink_ip[17] = {0},vgateway_ip[17] = {0};
	unsigned int uplink_mask = 0,downlink_mask = 0,vmask = 0;
	if((i < 0) || (i >= MAX_INSTANCE) || (REMOTEHANSI == NULL) || (REMOTEHANSI[i] == NULL))
		return ;
	int Inst_ID = REMOTEHANSI[i]->Inst_ID;

	if(totalLen + 1024 > str_len) {
		str_len *= 2;
		showStr_new = (char*)realloc(showStr,str_len);
		if(showStr_new == NULL){
			hmd_syslog_info("hmd show running realloc failed\n");
			return ;
		}else {
			showStr = showStr_new;
			*showStr2 = showStr;
			memset(showStr+str_len/2,0,str_len/2);
			showStr_new = NULL;
		}
		hmd_syslog_debug_debug(HMD_DBUS,"hmd show running totalLen %d realloc strlen %d\n",totalLen,str_len);
	}
	cursor = showStr + totalLen;
	hmd_syslog_info("%s,line=%d,REMOTEHANSI[%d]->isActive=%d.\n",__func__,__LINE__,i,REMOTEHANSI[i]->isActive);

	if((i < MAX_INSTANCE)&&REMOTEHANSI&&REMOTEHANSI[i]){
		totalLen += sprintf(cursor,"config hansi-profile %d-%d\n",REMOTEHANSI[i]->slot_no,Inst_ID);
		cursor = showStr + totalLen;
	}

	for(j=0;(j < MAX_IFNAME_NUM)&&(j < REMOTEHANSI[i]->Inst_DNum);j++){
		memset(downlink_ip,0,17);
		downlink_mask = 0;
		if((i < MAX_INSTANCE)&&(REMOTEHANSI[i])){
			sprintf(downlink_ip,"%d.%d.%d.%d",
				((REMOTEHANSI[i]->Inst_Downlink[j].vir_ip & 0xff000000) >> 24),
				((REMOTEHANSI[i]->Inst_Downlink[j].vir_ip & 0xff0000) >> 16),
				((REMOTEHANSI[i]->Inst_Downlink[j].vir_ip & 0xff00) >> 8),
				( REMOTEHANSI[i]->Inst_Downlink[j].vir_ip & 0xff));
				downlink_mask = REMOTEHANSI[i]->Inst_Downlink[j].mask;
			if(0 == downlink_mask){
				totalLen += sprintf(cursor,"config downlink %s %s priority %d \n",REMOTEHANSI[i]->Inst_Downlink[j].ifname,downlink_ip,REMOTEHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}else{
				totalLen += sprintf(cursor,"config downlink %s %s/%d priority %d \n",REMOTEHANSI[i]->Inst_Downlink[j].ifname,downlink_ip,downlink_mask,REMOTEHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}
		}
	}
	j = 0;
	for(j=0;(j < MAX_IFNAME_NUM)&&(j < REMOTEHANSI[i]->Inst_UNum);j++){
		memset(uplink_ip,0,17);
		uplink_mask = 0;
		if((i < MAX_INSTANCE)&&(REMOTEHANSI)&&(REMOTEHANSI[i])){
			sprintf(uplink_ip,"%d.%d.%d.%d",
				((REMOTEHANSI[i]->Inst_Uplink[j].vir_ip & 0xff000000) >> 24),
				((REMOTEHANSI[i]->Inst_Uplink[j].vir_ip & 0xff0000) >> 16),
				((REMOTEHANSI[i]->Inst_Uplink[j].vir_ip & 0xff00) >> 8),
				( REMOTEHANSI[i]->Inst_Uplink[j].vir_ip & 0xff));
				uplink_mask = REMOTEHANSI[i]->Inst_Uplink[j].mask;
			if(0 == uplink_mask){
				totalLen += sprintf(cursor,"config uplink %s %s priority %d \n",REMOTEHANSI[i]->Inst_Uplink[j].ifname,uplink_ip,REMOTEHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}else{
				totalLen += sprintf(cursor,"config uplink %s %s/%d priority %d \n",REMOTEHANSI[i]->Inst_Uplink[j].ifname,uplink_ip,uplink_mask,REMOTEHANSI[i]->priority);
				cursor = showStr + totalLen;	
			}
		}
	}
	
	for(j=0;(j < MAX_IFNAME_NUM)&&(j < REMOTEHANSI[i]->Inst_GNum);j++){
		hmd_syslog_info("REMOTEHANSI[i]->Inst_GNum =%d.\n",REMOTEHANSI[i]->Inst_GNum);	//for test
		hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		memset(vgateway_ip,0,17);
		vmask = 0;
		if((i < MAX_INSTANCE)&&(REMOTEHANSI)&&(REMOTEHANSI[i])){
			sprintf(vgateway_ip,"%d.%d.%d.%d",
				((REMOTEHANSI[i]->Inst_Gateway[j].vir_ip & 0xff000000) >> 24),
				((REMOTEHANSI[i]->Inst_Gateway[j].vir_ip & 0xff0000) >> 16),
				((REMOTEHANSI[i]->Inst_Gateway[j].vir_ip & 0xff00) >> 8),
				( REMOTEHANSI[i]->Inst_Gateway[j].vir_ip & 0xff));
				vmask = REMOTEHANSI[i]->Inst_Gateway[j].mask;
			if(0 == vmask){
				hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				totalLen += sprintf(cursor,"config vgateway %s %s \n",REMOTEHANSI[i]->Inst_Gateway[j].ifname,vgateway_ip);
				cursor = showStr + totalLen;	
			}else{
				hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				totalLen += sprintf(cursor,"config vgateway %s %s/%d \n",REMOTEHANSI[i]->Inst_Gateway[j].ifname,vgateway_ip,vmask);
				cursor = showStr + totalLen;	
			}
		}
	}
	if((i < MAX_INSTANCE)&&REMOTEHANSI&&REMOTEHANSI[i]&&(REMOTEHANSI[i]->HmdBakForever == 1)){
		totalLen += sprintf(cursor,"config hansi bak forever enable\n");
		cursor = showStr + totalLen;	
	}
	if((i < MAX_INSTANCE)&&REMOTEHANSI&&REMOTEHANSI[i]&&(REMOTEHANSI[i]->InstState == 1)){
		totalLen += sprintf(cursor,"config service enable\n");
		cursor = showStr + totalLen;	
	}


	totalLen += sprintf(cursor,"exit\n");
	cursor = showStr + totalLen;	

	*totalLen_T = totalLen;
	*str_len_T = str_len;
	return;
}

DBusMessage * hmd_dbus_license_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	int i = 0;
	int j = 0;
	int k = 0;
	int str_len = 0;
	int totalLen = 0;
	char *showStr = NULL,*cursor = NULL;
	
	dbus_error_init(&err);
	str_len = MAX_SLOT_NUM*1024;
	showStr = (char*)malloc(str_len);
	if(NULL == showStr) {
		hmd_syslog_debug_debug(HMD_DBUS,"hmd alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(showStr,0,str_len);
	cursor = showStr;
	if(isMaster){
		for(i = 0; i < LicenseCount; i++){
			for(j = 0; j < MAX_SLOT_NUM; j++){
				for(k = 0; k < MAX_INSTANCE; k++){
					if(LICENSE_MGMT[i].l_assigned_num[j][k] != 0){
						totalLen += sprintf(cursor,"license assign type %d num %d local-hansi %d-%d\n",i+1, LICENSE_MGMT[i].l_assigned_num[j][k], j ,k);
						cursor = showStr + totalLen;
					}
					if(LICENSE_MGMT[i].r_assigned_num[j][k] != 0){
						totalLen += sprintf(cursor,"license assign type %d num %d hansi %d-%d\n",i+1, LICENSE_MGMT[i].r_assigned_num[j][k], j, k);
						cursor = showStr + totalLen;
					}
				}
			}
		}
	}	
	if(service_tftp_state != 0)
	{
        totalLen += sprintf(cursor,"service tftp enable \n");
		cursor = showStr + totalLen; 
	}
	if(service_ftp_state != 0)
	{
        totalLen += sprintf(cursor,"service ftp enable \n");
		cursor = showStr + totalLen; 
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &showStr);
	free(showStr);
	showStr = NULL;
	return reply;
}

DBusMessage * hmd_dbus_hansi_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	int str_len = 0;
	int totalLen = 0;
	char *showStr = NULL,*cursor = NULL;
	int slot_id = 0,instID=0,islocal=0;
	int num = 0;
	int num2 = 0;
	struct Hmd_L_Inst_Mgmt *LOCALHANSI[MAX_SLOT_NUM*MAX_INSTANCE];
	int slotID[MAX_SLOT_NUM*MAX_INSTANCE] = {0};
	struct Hmd_Inst_Mgmt * REMOTEHANSI[MAX_SLOT_NUM*MAX_INSTANCE];	
	dbus_error_init(&err);
	str_len = MAX_SLOT_NUM*1024;
	showStr = (char*)malloc(str_len);
	if(NULL == showStr) {
		hmd_syslog_debug_debug(HMD_DBUS,"hmd alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(showStr,0,str_len);
	cursor = showStr;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_UINT32,&instID,
								DBUS_TYPE_UINT32,&islocal,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(showStr){
			free(showStr);
			showStr = NULL;
			cursor = NULL;
		}
		return NULL;
	}
	if(HMD_BOARD[slot_id]){
		if(islocal){
			if((HMD_BOARD[slot_id]->Hmd_Local_Inst[instID])){
				LOCALHANSI[num] = HMD_BOARD[slot_id]->Hmd_Local_Inst[instID];
				slotID[num] = slot_id;
				num ++;
			}
		}else{
			if((HMD_BOARD[slot_id]->Hmd_Inst[instID])){
				hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				REMOTEHANSI[num2] = HMD_BOARD[slot_id]->Hmd_Inst[instID];
				num2 ++;
			}
		}
	}
	if(0 == islocal){
		if(num2 > 0){
			if(1 == REMOTEHANSI[0]->HmdBakForever){
				totalLen += sprintf(cursor," config hansi bak forever enable\n");
				cursor = showStr + totalLen; 
			}
			/*fengwenchao add 20130415 for hmd timer config save*/
			if((isMaster)&&(isActive))
			{
				if(HANSI_TIMER != 300){
					totalLen += sprintf(cursor," set hmd-timer-config-save time %d\n",HANSI_TIMER);
					cursor = showStr + totalLen; 
				}
				if(HANSI_TIMER_CONFIG_SAVE != 1){
					totalLen += sprintf(cursor," set hansi-check disable\n");
					cursor = showStr + totalLen; 
				}		
			}
			/*fengwenchao add end*/
		}
	}else{
		if(num > 0){
			if(1 == LOCALHANSI[0]->HmdBakForever){
				totalLen += sprintf(cursor," config localhansi bak forever enable\n");
				cursor = showStr + totalLen; 
			}
			/*fengwenchao add 20130415 for hmd timer config save*/
			if((isMaster)&&(isActive))
			{
				if(HANSI_TIMER != 300){
					totalLen += sprintf(cursor," set hmd-timer-config-save time %d\n",HANSI_TIMER);
					cursor = showStr + totalLen; 
				}
				if(HANSI_TIMER_CONFIG_SAVE != 1){
					totalLen += sprintf(cursor," set hansi-check disable\n");
					cursor = showStr + totalLen; 
				}		
			}			
			/*fengwenchao add end*/
		}
	}
	if((num == 0)&&(num2 == 0)){
		hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		totalLen += sprintf(cursor,"          \n");
		cursor = showStr + totalLen;
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &showStr);
	free(showStr);
	showStr = NULL;
	return reply;
}

DBusMessage * hmd_dbus_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	int i = 0;
	int j = 0;
	int l_instID = 0;
	int r_instID = 0;
	int num = 0;
	int num2 = 0;
	int slot_id = 0;
	int instID = 0;
	int str_len = 0;
	int totalLen = 0;
	char *showStr = NULL,*cursor = NULL;
	char *showStr_new = NULL;
	int islocal = 1;
	struct Hmd_L_Inst_Mgmt *LOCALHANSI[MAX_SLOT_NUM*MAX_INSTANCE];
	int slotID[MAX_SLOT_NUM*MAX_INSTANCE] = {0};
	struct Hmd_Inst_Mgmt * REMOTEHANSI[MAX_SLOT_NUM*MAX_INSTANCE];	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_UINT32,&instID,
								DBUS_TYPE_UINT32,&islocal,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	//hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
	if(0 == instID){
		for(i=1;i<MAX_SLOT_NUM;i++){
			if(HMD_BOARD[i]){
				for(j=1;j<MAX_INSTANCE;j++){
					if((HMD_BOARD[i]->Hmd_Local_Inst[j])){
						LOCALHANSI[num] = HMD_BOARD[i]->Hmd_Local_Inst[j];
						slotID[num] = i;
						num ++;
					}
					if((HMD_BOARD[i]->Hmd_Inst[j])){
						//hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
						REMOTEHANSI[num2] = HMD_BOARD[i]->Hmd_Inst[j];
						num2 ++;
					}
				}
			}
		}
	}else if(HMD_BOARD[slot_id]){
		if(islocal){
			if((HMD_BOARD[slot_id]->Hmd_Local_Inst[instID])){
				LOCALHANSI[num] = HMD_BOARD[slot_id]->Hmd_Local_Inst[instID];
				slotID[num] = slot_id;
				num ++;
			}
		}else{
			if((HMD_BOARD[slot_id]->Hmd_Inst[instID])){
				//hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
				REMOTEHANSI[num2] = HMD_BOARD[slot_id]->Hmd_Inst[instID];
				num2 ++;
			}
		}
	}
	str_len = 2*MAX_INSTANCE*MAX_SLOT_NUM*1024;
	showStr = (char*)malloc(str_len);
	if(NULL == showStr) {
		hmd_syslog_debug_debug(HMD_DBUS,"hmd alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(showStr,0,str_len);
	cursor = showStr;
	totalLen += sprintf(cursor," \n");
	cursor = showStr + totalLen;
	
	//hmd_syslog_info("%s,line=%d,num=%d,num2=%d.\n",__func__,__LINE__,num,num2);
	if(num > 0){
	//	hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		hmd_syslog_info("LOCALHANSI[%d]->InstState = %d\n",l_instID,LOCALHANSI[l_instID]->isActive);
		for(l_instID=0;(l_instID<MAX_SLOT_NUM*MAX_INSTANCE)&&(l_instID<num);l_instID++){
			show_running_config_hmd_local(LOCALHANSI,l_instID,cursor,&showStr,showStr_new,&totalLen,&str_len,slotID);
		}
	}
	if(num2 > 0){
		
	//	hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		for(r_instID=0;(r_instID<MAX_SLOT_NUM*MAX_INSTANCE)&&(r_instID < num2);r_instID++){
			
		//	hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
			show_running_config_hmd_remote(REMOTEHANSI,r_instID,cursor,&showStr,showStr_new,&totalLen,&str_len);
		}
	}
	if((num == 0)&&(num2 == 0)){
		hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		totalLen += sprintf(cursor,"          \n");
		cursor = showStr + totalLen;
	}
	//hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &showStr);
	//hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
	free(showStr);
	showStr = NULL;
	//hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
	return reply;
}
DBusMessage * hmd_dbus_interface_config_local_hansi(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned int  ID;
	DBusError err;
	int ret = HMD_DBUS_SUCCESS;
	int slot_num = 0;
	int slot_no = -1;
	int slot_no1 = -1;
	unsigned int slot_id = 0;
	int local_id = 1;	
	HMDThreadArg *arg = NULL;
	char command[128] = {0};
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&ID,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ID == 0){
		return NULL;
	}
	if(isDistributed){
		if(isMaster&&isActive){
			ret = HMD_DBUS_SUCCESS;	
		}else{
			ret = HMD_DBUS_PERMISSION_DENIAL;
		}
	}
	if((ret == HMD_DBUS_SUCCESS)&&(slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL)){
		if(HMD_L_HANSI[ID] == NULL){
			HMD_L_HANSI[ID] = (struct Hmd_L_Inst_Mgmt_Summary *)malloc(sizeof(struct Hmd_L_Inst_Mgmt_Summary));
			if(HMD_L_HANSI[ID] == NULL){
				hmd_syslog_err("HMD_L_HANSI[%d] malloc fail.\n",ID);
				ret = HMD_DBUS_ERROR;
			}else{
				memset(HMD_L_HANSI[ID], 0, sizeof(struct Hmd_L_Inst_Mgmt_Summary));
				HMD_L_HANSI[ID]->slot_no = slot_id;
				HMD_L_HANSI[ID]->slot_no1 = -1;
				HMD_L_HANSI[ID]->InstState = 0;
				HMD_L_HANSI[ID]->slot_num = 1;
				HMD_L_HANSI[ID]->Inst_ID = ID;
				slot_num = HMD_L_HANSI[ID]->slot_num;
				slot_no = HMD_L_HANSI[ID]->slot_no;
				hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
				slot_no1 = HMD_L_HANSI[ID]->slot_no1;
				hmd_pid_write_v3(slot_id,ID,1);
				if(slot_id != HOST_SLOT_NO){
					HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
					memset(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID],0,sizeof(struct Hmd_L_Inst_Mgmt));
					HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->Inst_ID = ID;
					HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->slot_no = slot_id;
					HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->slot_no1 = -1;
					HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->slot_num = 1;
					HmdNoticeToClient(slot_id,ID,local_id,HMD_CREATE);
				}else{
					HOST_BOARD->Hmd_Local_Inst[ID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
					memset(HOST_BOARD->Hmd_Local_Inst[ID],0,sizeof(struct Hmd_L_Inst_Mgmt));
					HOST_BOARD->Hmd_Local_Inst[ID]->Inst_ID = ID;
					HOST_BOARD->Hmd_Local_Inst[ID]->slot_no = HOST_SLOT_NO; 						
					HOST_BOARD->Hmd_Local_Inst[ID]->slot_no1 = -1;
					HOST_BOARD->Hmd_Local_Inst[ID]->slot_num = 1;
					sprintf(command,"sudo /etc/init.d/wcpss start 1 %d",ID);
					system(command);
					/*add for eag  shaojunwu 20110620*/
					memset(command, 0, 128);
					sprintf(command,"sudo /etc/init.d/eag_modules start 1 %d &",ID);
					system(command);
					/*end for eag*/					

					/* add for pppoe lixiang 20120817 */
					memset(command, 0, 128);
					sprintf(command,"sudo /etc/init.d/pppoe start 1 %d &",ID);
					system(command);
					/*end for pppoe*/

					arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
					arg->QID = MAX_INSTANCE+ID;
					arg->islocaled = 1;
					arg->InstID = ID;
					HOST_BOARD->Hmd_Local_Inst[ID]->wid_check = 0;
					HOST_BOARD->Hmd_Local_Inst[ID]->asd_check = 0;
					HOST_BOARD->Hmd_Local_Inst[ID]->wsm_check = 0;
					HmdCreateThread(&(HOST_BOARD->Hmd_Local_Inst[ID]->threadID), HMDHansiMonitor, arg, 0);					
					HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Local_Inst[ID]->HmdTimerID), HMD_CHECKING, ID, 1);
				}
				if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slot_id)){
					syn_hansi_info_to_backup(slot_id, ID, MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_ADD,0);
				}
				sleep(5);
			}
		}
		else if(HMD_L_HANSI[ID]->slot_num < 2){			
			if(HMD_BOARD[slot_id]){
				if(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] == NULL){
					if(HMD_L_HANSI[ID]->slot_no == -1){
						HMD_L_HANSI[ID]->slot_no = slot_id;
						slot_no = HMD_L_HANSI[ID]->slot_no;
						slot_no1 = HMD_L_HANSI[ID]->slot_no1;
						hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
					}else if(HMD_L_HANSI[ID]->slot_no1 == -1){
						HMD_L_HANSI[ID]->slot_no1 = slot_id;
						slot_no = HMD_L_HANSI[ID]->slot_no1;
						slot_no1 = HMD_L_HANSI[ID]->slot_no;
						hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
					}
					if((slot_no1>0)&&(slot_no1<MAX_SLOT_NUM)){
					if((HMD_BOARD[slot_no1] != NULL)&&(HMD_BOARD[slot_no1]->Hmd_Local_Inst[ID] != NULL)){
						HMD_BOARD[slot_no1]->Hmd_Local_Inst[ID]->slot_no1 = slot_no;
						HMD_BOARD[slot_no1]->Hmd_Local_Inst[ID]->slot_num = 2;
					}
					hmd_pid_write_v3(slot_id,ID,1);
					if(slot_id != HOST_SLOT_NO){
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
						memset(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID],0,sizeof(struct Hmd_L_Inst_Mgmt));
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->Inst_ID = ID;
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->slot_no = slot_id;						
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->slot_no1 = slot_no1;
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->slot_num = 2;
						HmdNoticeToClient(slot_id,ID,local_id,HMD_CREATE);
					}else{				
						HOST_BOARD->Hmd_Local_Inst[ID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
						memset(HOST_BOARD->Hmd_Local_Inst[ID],0,sizeof(struct Hmd_L_Inst_Mgmt));
						
						HOST_BOARD->Hmd_Local_Inst[ID]->Inst_ID = ID;
						HOST_BOARD->Hmd_Local_Inst[ID]->slot_no = HOST_SLOT_NO; 						
						HOST_BOARD->Hmd_Local_Inst[ID]->slot_no1 = slot_no1;
						HOST_BOARD->Hmd_Local_Inst[ID]->slot_num = 2;
						sprintf(command,"sudo /etc/init.d/wcpss start 1 %d",ID);
						system(command);
						/*add for eag  shaojunwu 20110620*/
						memset(command, 0, 128);
						sprintf(command,"sudo /etc/init.d/eag_modules start 1 %d &",ID);
						system(command);
						/*end for eag*/	

						/* add for pppoe lixiang 20120817 */
						memset(command, 0, 128);
						sprintf(command,"sudo /etc/init.d/pppoe start 1 %d &",ID);
						system(command);
						/*end for pppoe*/
						
						arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
						arg->QID = MAX_INSTANCE+ID;
						arg->islocaled = 1;
						arg->InstID = ID;
						HOST_BOARD->Hmd_Local_Inst[ID]->wid_check = 0;
						HOST_BOARD->Hmd_Local_Inst[ID]->asd_check = 0;
						HOST_BOARD->Hmd_Local_Inst[ID]->wsm_check = 0;
						HmdCreateThread(&(HOST_BOARD->Hmd_Local_Inst[ID]->threadID), HMDHansiMonitor, arg, 0);						
						HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Local_Inst[ID]->HmdTimerID), HMD_CHECKING, ID, 1);
					}
					HMD_L_HANSI[ID]->slot_num = 2;					
					if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slot_id)){
						syn_hansi_info_to_backup(slot_id, ID, MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_ADD,0);
					}
					sleep(5);
					}else{
						hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
					}
				}else{
					slot_no = slot_id;
					slot_no1 = -1;
					hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
				}
			}
			slot_num = HMD_L_HANSI[ID]->slot_num;
		}else{
			if(HMD_L_HANSI[ID]->slot_no == slot_id){
				slot_no = HMD_L_HANSI[ID]->slot_no;
				slot_no1 = HMD_L_HANSI[ID]->slot_no1;
				slot_num = HMD_L_HANSI[ID]->slot_num;
				hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
			}else if(HMD_L_HANSI[ID]->slot_no1 == slot_id){
				slot_no = HMD_L_HANSI[ID]->slot_no1;
				slot_no1 = HMD_L_HANSI[ID]->slot_no;
				slot_num = HMD_L_HANSI[ID]->slot_num;
				hmd_syslog_info("%s,line=%d slot_no %d slot_no1 %d.\n",__func__,__LINE__,slot_no, slot_no1);
			}else{
				ret = HMD_DBUS_ID_NO_EXIST; 
			}

		}
	}
	else if(ret == HMD_DBUS_SUCCESS){
		ret = HMD_DBUS_SLOT_ID_NOT_EXIST;		
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
					  		 DBUS_TYPE_UINT32,&slot_num,
							 DBUS_TYPE_INT32,&slot_no,
							 DBUS_TYPE_INT32,&slot_no1,
							 DBUS_TYPE_INVALID);
	return reply;
	
}

DBusMessage * hmd_dbus_interface_config_remote_hansi(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned int ID = 0;
	unsigned int slot_id = 0;
	DBusError err;
	int ret = HMD_DBUS_SUCCESS;
	int local_id = 0;	
	HMDThreadArg *arg = NULL;
	char command[128] = {0};
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_INVALID))) 
	{
	    hmd_syslog_err("set hansi profile:Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("set hansi profile %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	if(ID == 0){
		return NULL;
	}
	if(isDistributed){
		if(isMaster&&isActive){
			ret = HMD_DBUS_SUCCESS;	
		}else{
			ret = HMD_DBUS_PERMISSION_DENIAL;
		}
	}
	if((ret == HMD_DBUS_SUCCESS)&&(slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL)){
		if(HMD_BOARD[slot_id]->Hmd_Inst[ID] == NULL){
			HMD_BOARD[slot_id]->Hmd_Inst[ID] = (struct Hmd_Inst_Mgmt *)malloc(sizeof(struct Hmd_Inst_Mgmt));
			if(HMD_BOARD[slot_id]->Hmd_Inst[ID] == NULL){
				ret = HMD_DBUS_ERROR;
			}else{
				memset(HMD_BOARD[slot_id]->Hmd_Inst[ID], 0, sizeof(struct Hmd_Inst_Mgmt));

				HMD_BOARD[slot_id]->Hmd_Inst[ID]->slot_no = slot_id;
				HMD_BOARD[slot_id]->Hmd_Inst[ID]->InstState = 0;
				HMD_BOARD[slot_id]->Hmd_Inst[ID]->Inst_ID = ID;
				hmd_pid_write_v3(slot_id,ID,0);
				if(slot_id != HOST_SLOT_NO){
					HmdNoticeToClient(slot_id,ID,local_id,HMD_CREATE);
				}else{
					sprintf(command,"sudo /etc/init.d/wcpss start 0 %d",ID);
					hmd_syslog_info("%s\n",command);
            		if(system(command))
            		    hmd_syslog_info("777777777\n");
					/*add for eag  shaojunwu 20110620*/
					memset(command, 0, 128);
					sprintf(command,"sudo /etc/init.d/eag_modules start 0 %d &",ID);
					system(command);
					/*end for eag*/					

					/* add for pppoe lixiang 20120817 */
					memset(command, 0, 128);
					sprintf(command,"sudo /etc/init.d/pppoe start 0 %d &",ID);
					system(command);
					/*end for pppoe*/
					
            		memset(command, 0, 128);
            		sprintf(command,"sudo /etc/init.d/had start %d &",ID);
					system(command);
					
					arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
					arg->QID = ID;
					arg->islocaled = 0;
					arg->InstID = ID;
					HOST_BOARD->Hmd_Inst[ID]->wid_check = 0;
					HOST_BOARD->Hmd_Inst[ID]->asd_check = 0;
					HOST_BOARD->Hmd_Inst[ID]->wsm_check = 0;
					HmdCreateThread(&(HOST_BOARD->Hmd_Inst[ID]->threadID), HMDHansiMonitor, arg, 0);					
					HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Inst[ID]->HmdTimerID), HMD_CHECKING, ID, 0);
				}
				sleep(5);
			}			
			if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slot_id)){
				syn_hansi_info_to_backup(slot_id, ID, MASTER_BACKUP_SLOT_NO,0,HMD_HANSI_INFO_SYN_ADD,0);
			}
		}
	}
	else if(ret == HMD_DBUS_SUCCESS){
		ret = HMD_DBUS_ID_NO_EXIST;		
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL == reply){		
		hmd_syslog_err("vrrp set hansi profile dbus reply null!\n");
		return reply;
	}
		
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
	
}

DBusMessage * hmd_dbus_interface_set_hansi_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int ID = 0;
	unsigned int slot_id = 0;
	unsigned int state = 0;
	DBusError err;
	int ret = HMD_DBUS_SUCCESS;
	hmd_syslog_info("%s,into.\n",__func__);
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID))) 
	{
	    hmd_syslog_err("set hansi profile:Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("set hansi profile %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	if(ret == 0){
		if((HMD_BOARD[slot_id]!=NULL)&&(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]!=NULL)){
			HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]->isActive = state;
		}
		
		if(HMD_L_HANSI[ID]){
			if(HMD_L_HANSI[ID]->slot_no == slot_id){
				HMD_L_HANSI[ID]->isActive = state;
				if(HMD_L_HANSI[ID]->slot_num == 2){
					HMD_BOARD[HMD_L_HANSI[ID]->slot_no1]->Hmd_Local_Inst[ID]->isActive1 = state;
				}
			}else if(HMD_L_HANSI[ID]->slot_no1 == slot_id){
				HMD_L_HANSI[ID]->InstState1 = state;
				if(HMD_L_HANSI[ID]->slot_num == 2){
					HMD_BOARD[HMD_L_HANSI[ID]->slot_no]->Hmd_Local_Inst[ID]->isActive1 = state;
				}
			}
		}
 	}
 	reply = dbus_message_new_method_return(msg);
	if(NULL == reply){		
		hmd_syslog_err("vrrp set hansi profile dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;
	
}


DBusMessage * hmd_dbus_interface_set_hansi_check_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int state = 0;
	DBusError err;
	int ret = HMD_DBUS_SUCCESS;
	hmd_syslog_info("%s,into.\n",__func__);
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID))) 
	{
	    hmd_syslog_err("set hansi profile:Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("set hansi profile %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	if((HANSI_CHECK_OP != state)&&(1 == state)){
		int i = 0,j = 0;
		for(i=1;i<MAX_SLOT_NUM;i++){
			if(HMD_BOARD[i]){
				for(j=1;j<MAX_INSTANCE;j++){
					if((HMD_BOARD[i]->Hmd_Local_Inst[j])){
						HmdTimerCancel(&(HMD_BOARD[i]->Hmd_Local_Inst[j]->HmdTimerID),1);
						HMDTimerRequest(HMD_CHECKING_TIMER,&(HMD_BOARD[i]->Hmd_Local_Inst[j]->HmdTimerID), HMD_CHECKING, j, 1);
					}
					if((HMD_BOARD[i]->Hmd_Inst[j])){
						HmdTimerCancel(&(HMD_BOARD[i]->Hmd_Inst[j]->HmdTimerID),1);
						HMDTimerRequest(HMD_CHECKING_TIMER,&(HMD_BOARD[i]->Hmd_Inst[j]->HmdTimerID), HMD_CHECKING, j, 0);
					}
				}
			}
		}
	}
	HANSI_CHECK_OP = state;
 	reply = dbus_message_new_method_return(msg);
	if(NULL == reply){		
		hmd_syslog_err("vrrp set hansi profile dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;
	
}


DBusMessage * hmd_dbus_interface_set_hansi_uplinkif(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};

	unsigned int ret = HMD_DBUS_SUCCESS;
	unsigned int InstID = 0;
	unsigned int priority = 0;
	char *uplink_ifname = NULL;
	unsigned long uplink_ip = 0;
	unsigned int uplink_mask = 0;
	int slot_id = 0;
	int i = 0;
	int len1, len2;
	struct Hmd_L_Inst_Mgmt * InstInfo;
	hmd_syslog_info("local hansi uplink:\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &slot_id,
					DBUS_TYPE_UINT32, &InstID,
					DBUS_TYPE_STRING, &uplink_ifname,
					DBUS_TYPE_UINT32, &uplink_ip,\
					DBUS_TYPE_UINT32, &uplink_mask,\
					DBUS_TYPE_UINT32, &priority,\
					DBUS_TYPE_INVALID)))
	{
		hmd_syslog_err("hmd unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("hmd %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	char *ifname_tmp = NULL;
	int name_ret = 0;
	ifname_tmp = (char *)malloc(strlen(uplink_ifname)+5);
	memset(ifname_tmp,0,strlen(uplink_ifname)+5);
	name_ret = check_ve_interface(uplink_ifname,ifname_tmp);
	if(name_ret == 0)
	{
		if((InstID > 0)&&(InstID < MAX_INSTANCE)){
			if(HMD_BOARD[slot_id] != NULL){
				if(HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID] != NULL){
					InstInfo = HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID];
					if(InstInfo->Inst_UNum < MAX_IFNAME_NUM){
						for(i=0;i<InstInfo->Inst_UNum;i++){
							len1  = strlen(InstInfo->Inst_Uplink[i].ifname);
							len2  = strlen(ifname_tmp);
							if((len1 == len2)&&(strncmp(InstInfo->Inst_Uplink[i].ifname,ifname_tmp,len1)==0)){
								break;
							}
						}
						if(i == InstInfo->Inst_UNum){
							InstInfo->Inst_UNum ++;
							memset(InstInfo->Inst_Uplink[i].ifname,0,MAX_IFNAME_LEN);
							memcpy(InstInfo->Inst_Uplink[i].ifname,ifname_tmp,len2);
							InstInfo->Inst_Uplink[i].vir_ip = uplink_ip;
							InstInfo->Inst_Uplink[i].mask = uplink_mask;
							InstInfo->priority = priority;
						}
					}else{
						ret = HMD_DBUS_NUM_OVER_MAX;
					}
				}else{
					ret = HMD_DBUS_ID_NO_EXIST; 
				}
			}else{
				ret = HMD_DBUS_ID_NO_EXIST;
			}
		}
	}
	else
		ret = HMD_DBUS_ERROR;
	if(ifname_tmp)
	{
		free(ifname_tmp);
		ifname_tmp = NULL;
	}
	//NoticeWIDUpLinkIf();//when server enable sent to wid
	hmd_syslog_err("config hmd start ret %x.\n", ret);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		hmd_syslog_err("hmd dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
	
}
/*fengwenchao add 20120228 for AXSSZFI-680*/
DBusMessage * hmd_dbus_interface_delete_slotid_img(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	hmd_syslog_info("####accessinto %s \n",__func__);
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err;

	int ret = 0;
	unsigned int slot_id_target = 0;
	int sysState = 0;
	int sysErrorCode = 0;
	char *ap_img = NULL;
	

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT32,&slot_id_target,
							  DBUS_TYPE_STRING,&ap_img,
							 DBUS_TYPE_INVALID))) 
	{
	    hmd_syslog_err("set hansi profile:Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("set hansi profile %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	
	char *ap_img_string = NULL;
	
	ap_img_string = (char *)malloc(DEFAULT_LEN);
	if(NULL == ap_img_string){
		hmd_syslog_err("%s %d,ap_img_string malloc fail.\n",__func__,__LINE__);
		return NULL;
	}else{
		memset(ap_img_string,0,DEFAULT_LEN);
	}
	if(ap_img)
	{	
		if(strlen(ap_img) >= DEFAULT_LEN){
			ret = HMD_DBUS_WARNNING;
		}else{
		memset(ap_img_string,0,DEFAULT_LEN);
		memcpy(ap_img_string,ap_img,strlen(ap_img));
			hmd_syslog_info("ap_img_string	=  %s \n",ap_img_string);
	}
	}
	
	if(0 == ret){
	if((slot_id_target < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id_target] != NULL))
	{
			hmd_syslog_info("slot_id_target  =	%d \n",slot_id_target);
		hmd_syslog_info("HOST_SLOT_NO  =  %d \n",HOST_SLOT_NO);
		if(slot_id_target != HOST_SLOT_NO)
		{
			struct sockaddr_tipc * addr = &(HMD_BOARD[slot_id_target]->tipcaddr);
			int fd = HMD_BOARD[slot_id_target]->tipcfd;
			
			struct HmdMsg msg;
			memset(&msg,0,sizeof(struct HmdMsg));
			msg.op = HMD_DELETE_SLOTID_AP_UPDATA_IMG;
			msg.S_SlotID = slot_id_target;
			/*if(msg.slot_ap_updata_img)
			{
				free(msg.slot_ap_updata_img);
				msg.slot_ap_updata_img = NULL;
			}
			msg.slot_ap_updata_img = (char *)malloc(DEFAULT_LEN);*/
			if(ap_img_string)
			{
				memset(msg.slot_ap_updata_img,0,DEFAULT_LEN);
				memcpy(msg.slot_ap_updata_img,ap_img_string,strlen(ap_img_string));
			}
			char buf[4096];
			memset(buf,0,4096);
			int len = sizeof(msg);
			memcpy(buf,(char*)&msg,len);
				hmd_syslog_info("msg.slot_ap_updata_img  =	%s \n",msg.slot_ap_updata_img);
			if (0 > sendto(fd,buf,sizeof(msg)+1,0,
						   (struct sockaddr*)addr,
						   sizeof(struct sockaddr))){
					perror("Server: Failed to send");
					hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
					//exit(1);
			}else{
				hmd_syslog_info("Server: %s,success to send.\n",__func__);
			}
			ret = HMD_DBUS_WARNNING;
		}
		else
		{
			char *dir = "/mnt/wtp";
			char command[DEFAULT_LEN] = {0};
			sprintf(command,"rm /mnt/wtp/%s",ap_img_string);
				hmd_syslog_info("command =	%s \n",command);
			DIR *dp = NULL;
			struct dirent *dirp;
			dp = opendir(dir);
			if(dp != NULL)
			{	
				hmd_syslog_info("dp  ! =  NULL \n");
				while((dirp = readdir(dp)) != NULL)
				{
					ret = HMD_DBUS_ERROR;
						hmd_syslog_info("dirp  ! =	NULL \n");
					hmd_syslog_info("dirp->d_name  =  %s  ap_img_string =  %s \n",dirp->d_name,ap_img_string);
					if((memcmp(dirp->d_name,ap_img_string,strlen(ap_img_string))) ==  0)
					{
						hmd_syslog_info("dirp->d_name  ==  ap_img_string \n");
						sysState = system(command);
						sysErrorCode = WEXITSTATUS(sysState);
						hmd_syslog_info("sysErrorCode  ==  %d \n",sysErrorCode);
						if(sysErrorCode != 0)
							ret = SYSTEM_CMD_ERROR;
						else
							ret = 0;
						break;
					}
				}
				closedir(dp);
			}
		}
	}
	else
	{
		ret = HMD_DBUS_SLOT_ID_NOT_EXIST;
	}
	}else{
	}
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		hmd_syslog_err("hmd dbus set error!\n");
		if(ap_img_string)
		{
			free(ap_img_string);
			ap_img_string = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(ap_img_string)
	{
		free(ap_img_string);
		ap_img_string = NULL;
	}
	hmd_syslog_info("####over %s !!!!!!!!!!\n",__func__);
	return reply;
}
/*fengwenchao add end*/

DBusMessage * hmd_dbus_interface_set_hansi_downlinkif(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	char *ifname = NULL;
	unsigned int	ret = 0;
	int slot_id = 0;
	int InstID;
	int i = 0;
	int downlink_ip = 0;
	int downlink_mask = 0;
	int len1, len2;
	int priority = 0;
	struct Hmd_L_Inst_Mgmt * InstInfo;
    hmd_syslog_info("start local hansi downlink... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, 
					DBUS_TYPE_UINT32,&slot_id, 	
					DBUS_TYPE_UINT32,&InstID, 
					DBUS_TYPE_STRING,&ifname,
					DBUS_TYPE_UINT32,&downlink_ip,
					DBUS_TYPE_UINT32,&downlink_mask,
					DBUS_TYPE_UINT32,&priority,
					DBUS_TYPE_INVALID)))
	{
		hmd_syslog_err("hmd unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			hmd_syslog_err("hmd start downlink %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	char *ifname_tmp = NULL;
	int name_ret = 0;
	ifname_tmp = (char *)malloc(strlen(ifname)+5);
	memset(ifname_tmp,0,strlen(ifname)+5);
	name_ret = check_ve_interface(ifname,ifname_tmp);
	
	if(name_ret == 0)
	{
		if((InstID > 0)&&(InstID < MAX_INSTANCE)){
			if(HMD_BOARD[slot_id] != NULL){
				if(HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID] != NULL){
					InstInfo = HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID];
					if(InstInfo->Inst_DNum < MAX_IFNAME_NUM){
						for(i=0;i<InstInfo->Inst_DNum;i++){
							len1  = strlen(InstInfo->Inst_Downlink[i].ifname);
							len2  = strlen(ifname_tmp);
							if((len1 == len2)&&(strncmp(InstInfo->Inst_Downlink[i].ifname,ifname_tmp,len1)==0)){
								break;
							}
						}
						if(i == InstInfo->Inst_DNum){
							InstInfo->Inst_DNum ++;
							memset(InstInfo->Inst_Downlink[i].ifname,0,MAX_IFNAME_LEN);
							memcpy(InstInfo->Inst_Downlink[i].ifname,ifname_tmp,len2);
							InstInfo->Inst_Downlink[i].vir_ip = downlink_ip;
							InstInfo->Inst_Downlink[i].mask = downlink_mask;
							InstInfo->priority = priority;
						}
					}else{
						ret = HMD_DBUS_NUM_OVER_MAX;
					}
				}else{
					ret = HMD_DBUS_ID_NO_EXIST; 
				}
			}else{
				ret = HMD_DBUS_ID_NO_EXIST;
			}
		}else{
			hmd_syslog_info("%s,line=%d.\n",__func__,__LINE__);
			ret = HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;
		}
	}
	else
		ret = HMD_DBUS_ERROR;
	if(ifname_tmp)
	{
		free(ifname_tmp);
		ifname_tmp = NULL;
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		hmd_syslog_err("hmd start downlink dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
	
}


DBusMessage * hmd_dbus_interface_set_hansi_gateway(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	char *ifname = NULL;
	unsigned int	ret = 0;
	int slot_id = 0;
	int InstID;
	int i = 0;
	int vgateway_ip = 0;
	int vgateway_mask = 0;
	int len1, len2;
	struct Hmd_L_Inst_Mgmt * InstInfo;
    hmd_syslog_info("start local hansi gateway... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, 
					DBUS_TYPE_UINT32,&slot_id, 
					DBUS_TYPE_UINT32,&InstID, 
					DBUS_TYPE_STRING,&ifname,
					DBUS_TYPE_UINT32,&vgateway_ip,
					DBUS_TYPE_UINT32,&vgateway_mask,
					DBUS_TYPE_INVALID)))
	{
		hmd_syslog_err("hmd unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			hmd_syslog_err("hmd start downlink %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	char *ifname_tmp = NULL;
	int name_ret = 0;
	ifname_tmp = (char *)malloc(strlen(ifname)+5);
	memset(ifname_tmp,0,strlen(ifname)+5);
	name_ret = check_ve_interface(ifname,ifname_tmp);	
	
	if(name_ret == 0)
	{
		if((InstID > 0)&&(InstID < MAX_INSTANCE)){
			if(HMD_BOARD[slot_id] != NULL){
				if(HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID] != NULL){
					InstInfo = HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID];
					if(InstInfo->Inst_GNum < MAX_IFNAME_NUM){
						for(i=0;i<InstInfo->Inst_GNum;i++){
							len1  = strlen(InstInfo->Inst_Gateway[i].ifname);
							len2  = strlen(ifname_tmp);
							if((len1 == len2)&&(strncmp(InstInfo->Inst_Gateway[i].ifname,ifname_tmp,len1)==0)){
								break;
							}
						}
						if(i == InstInfo->Inst_GNum){
							InstInfo->Inst_GNum ++;
							memset(InstInfo->Inst_Gateway[i].ifname,0,MAX_IFNAME_LEN);
							memcpy(InstInfo->Inst_Gateway[i].ifname,ifname_tmp,len2);
							InstInfo->Inst_Gateway[i].vir_ip = vgateway_ip;
							InstInfo->Inst_Gateway[i].mask = vgateway_mask;
						}
					}else{
						ret = HMD_DBUS_NUM_OVER_MAX;
					}
				}else{
					ret = HMD_DBUS_ID_NO_EXIST; 
				}
			}else{
				ret = HMD_DBUS_ID_NO_EXIST;
			}
		}
	}
	else
		ret = HMD_DBUS_ERROR;
	if(ifname_tmp)
	{
		free(ifname_tmp);
		ifname_tmp = NULL;
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		hmd_syslog_err("hmd start downlink dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
	
}

DBusMessage * hmd_dbus_interface_delete_link_iface(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	char *ifname = NULL;
	unsigned int	ret = 0;
	int slot_id = 0;
	int InstID;
	int i = 0,j = 0;
	int len1, len2;
	int type = 0;
	struct Hmd_L_Inst_Mgmt * InstInfo;
	hmd_syslog_info("start delete hansi iface config... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, 
					DBUS_TYPE_UINT32,&slot_id, 	
					DBUS_TYPE_UINT32,&InstID, 
					DBUS_TYPE_UINT32,&type, 
					DBUS_TYPE_STRING,&ifname,
					DBUS_TYPE_INVALID)))
	{
		hmd_syslog_err("hmd unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			hmd_syslog_err("hmd start downlink %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	char *ifname_tmp = NULL;
	int name_ret = 0;
	ifname_tmp = (char *)malloc(strlen(ifname)+5);
	memset(ifname_tmp,0,strlen(ifname)+5);
	name_ret = check_ve_interface(ifname,ifname_tmp);	

	if(name_ret == 0)
	{
		if((InstID > 0)&&(InstID < MAX_INSTANCE)){
			if(HMD_BOARD[slot_id] != NULL){
				if(HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID] != NULL){
					InstInfo = HMD_BOARD[slot_id]->Hmd_Local_Inst[InstID];
					if((type == 1)&&(InstInfo->Inst_UNum < MAX_IFNAME_NUM)){
						hmd_syslog_info("Inst_UNum:%d \n",InstInfo->Inst_UNum);
						for(i=0;i<InstInfo->Inst_UNum;i++){
							len1  = strlen(InstInfo->Inst_Uplink[i].ifname);
							len2  = strlen(ifname_tmp);
							if((len1 == len2)&&(strncmp(InstInfo->Inst_Uplink[i].ifname,ifname_tmp,len1)==0)){
								memset(InstInfo->Inst_Uplink[i].ifname,0,MAX_IFNAME_LEN);
								memset(InstInfo->Inst_Uplink[i].mac,0,MAC_LEN);
								InstInfo->Inst_Uplink[i].real_ip = 0;
								InstInfo->Inst_Uplink[i].vir_ip = 0;
								InstInfo->Inst_Uplink[i].remote_r_ip = 0;
								InstInfo->Inst_Uplink[i].mask = 0;
								InstInfo->Inst_UNum--;
								hmd_syslog_info("Inst_UNum111:%d \n",InstInfo->Inst_UNum);

								for(j=i;j<MAX_IFNAME_LEN-1;j++){
									InstInfo->Inst_Uplink[j] = InstInfo->Inst_Uplink[j+1];
									memset(InstInfo->Inst_Uplink[MAX_IFNAME_LEN-1].ifname,0,MAX_IFNAME_LEN);
									memset(InstInfo->Inst_Uplink[MAX_IFNAME_LEN-1].mac,0,MAC_LEN);
									InstInfo->Inst_Uplink[MAX_IFNAME_LEN-1].real_ip = 0;
									InstInfo->Inst_Uplink[MAX_IFNAME_LEN-1].vir_ip = 0;
									InstInfo->Inst_Uplink[MAX_IFNAME_LEN-1].remote_r_ip = 0;
									InstInfo->Inst_Uplink[MAX_IFNAME_LEN-1].mask = 0;
								}
								break;
							}
						}
						if(i == InstInfo->Inst_UNum){
							hmd_syslog_info("i111:%d \n",i);
							ret = HMD_DBUS_NOCONFIG_UPLINK; 
						}
					}else if((type == 2)&&(InstInfo->Inst_DNum < MAX_IFNAME_NUM)){
						hmd_syslog_info("Inst_DNum:%d \n",InstInfo->Inst_DNum);
						for(i=0;i<InstInfo->Inst_DNum;i++){
							len1  = strlen(InstInfo->Inst_Downlink[i].ifname);
							len2  = strlen(ifname_tmp);
							if((len1 == len2)&&(strncmp(InstInfo->Inst_Downlink[i].ifname,ifname_tmp,len1)==0)){
								memset(InstInfo->Inst_Downlink[i].ifname,0,MAX_IFNAME_LEN);
								memset(InstInfo->Inst_Downlink[i].mac,0,MAC_LEN);
								InstInfo->Inst_Downlink[i].real_ip = 0;
								InstInfo->Inst_Downlink[i].vir_ip = 0;
								InstInfo->Inst_Downlink[i].remote_r_ip = 0;
								InstInfo->Inst_Downlink[i].mask = 0;
								InstInfo->Inst_DNum--;
								hmd_syslog_info("Inst_DNum222:%d \n",InstInfo->Inst_DNum);

								for(j=i;j<MAX_IFNAME_LEN-1;j++){
									InstInfo->Inst_Downlink[j] = InstInfo->Inst_Downlink[j+1];
									memset(InstInfo->Inst_Downlink[MAX_IFNAME_LEN-1].ifname,0,MAX_IFNAME_LEN);
									memset(InstInfo->Inst_Downlink[MAX_IFNAME_LEN-1].mac,0,MAC_LEN);
									InstInfo->Inst_Downlink[MAX_IFNAME_LEN-1].real_ip = 0;
									InstInfo->Inst_Downlink[MAX_IFNAME_LEN-1].vir_ip = 0;
									InstInfo->Inst_Downlink[MAX_IFNAME_LEN-1].remote_r_ip = 0;
									InstInfo->Inst_Downlink[MAX_IFNAME_LEN-1].mask = 0;
								}
								break;
							}
						}
						if(i == InstInfo->Inst_DNum){
							hmd_syslog_info("i222:%d \n",i);
							ret = HMD_DBUS_NOCONFIG_UPLINK; 
						}
					}else if((type == 3)&&(InstInfo->Inst_GNum < MAX_IFNAME_NUM)){
						hmd_syslog_info("Inst_GNum:%d \n",InstInfo->Inst_GNum);
						for(i=0;i<InstInfo->Inst_GNum;i++){
							len1  = strlen(InstInfo->Inst_Gateway[i].ifname);
							len2  = strlen(ifname_tmp);
							if((len1 == len2)&&(strncmp(InstInfo->Inst_Gateway[i].ifname,ifname_tmp,len1)==0)){
								memset(InstInfo->Inst_Gateway[i].ifname,0,MAX_IFNAME_LEN);
								memset(InstInfo->Inst_Gateway[i].mac,0,MAC_LEN);
								InstInfo->Inst_Gateway[i].real_ip = 0;
								InstInfo->Inst_Gateway[i].vir_ip = 0;
								InstInfo->Inst_Gateway[i].remote_r_ip = 0;
								InstInfo->Inst_Gateway[i].mask = 0;
								InstInfo->Inst_GNum--;
								hmd_syslog_info("Inst_GNum333:%d \n",InstInfo->Inst_GNum);

								for(j=i;j<MAX_IFNAME_LEN-1;j++){
									InstInfo->Inst_Gateway[j] = InstInfo->Inst_Gateway[j+1];
									memset(InstInfo->Inst_Gateway[MAX_IFNAME_LEN-1].ifname,0,MAX_IFNAME_LEN);
									memset(InstInfo->Inst_Gateway[MAX_IFNAME_LEN-1].mac,0,MAC_LEN);
									InstInfo->Inst_Gateway[MAX_IFNAME_LEN-1].real_ip = 0;
									InstInfo->Inst_Gateway[MAX_IFNAME_LEN-1].vir_ip = 0;
									InstInfo->Inst_Gateway[MAX_IFNAME_LEN-1].remote_r_ip = 0;
									InstInfo->Inst_Gateway[MAX_IFNAME_LEN-1].mask = 0;
								}
								break;
							}
						}
						if(i == InstInfo->Inst_GNum){
							hmd_syslog_info("i333:%d \n",i);
							ret = HMD_DBUS_NOCONFIG_UPLINK; 
						}
					}else{
						ret = HMD_DBUS_NUM_OVER_MAX;
					}
				}else{
					ret = HMD_DBUS_ID_NO_EXIST; 
				}
			}else{
				ret = HMD_DBUS_ID_NO_EXIST;
			}
		}
	}
	else
		ret = HMD_DBUS_ERROR;
	if(ifname_tmp)
	{
		free(ifname_tmp);
		ifname_tmp = NULL;
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		hmd_syslog_err("hmd delete link iface dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
	
}

/*fengwenchao add 20110616*/
DBusMessage * hmd_dbus_interface_show_hmd_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	//DBusMessageIter iter = {0};
	DBusError err;	
	unsigned int	ret = HMD_DBUS_SUCCESS;
	dbus_error_init(&err);

	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_board_struct;
	DBusMessageIter iter_inst_array;
	DBusMessageIter iter_inst_struct;
	DBusMessageIter iter_l_inst_array;
	DBusMessageIter iter_l_inst_struct;
	
	DBusMessageIter inst_Uplink_array;  
	DBusMessageIter inst_Uplink_struct;
	DBusMessageIter inst_downlink_array;  
	DBusMessageIter inst_downlink_struct;
	DBusMessageIter inst_gateway_array;  
	DBusMessageIter inst_gateway_struct;

	DBusMessageIter l_inst_Uplink_array;  
	DBusMessageIter l_inst_Uplink_struct;
	DBusMessageIter l_inst_downlink_array;  
	DBusMessageIter l_inst_downlink_struct;
	DBusMessageIter l_inst_gateway_array;  
	DBusMessageIter l_inst_gateway_struct;


	int i = 0;
	int j = 0;
	int k = 0;
	int board_num = 0;
	//char * null_ifname = "none";
	char * ifname = NULL;
	ifname = (char*)malloc(MAX_IFNAME_LEN+1);
	struct Hmd_Board_Info **board;
	board = malloc(MAX_SLOT_NUM*(sizeof(struct Hmd_Board_Info *)));
	for(i =1; i < MAX_SLOT_NUM; i++)
	{
		if(HMD_BOARD[i])
		{
			board[board_num] = HMD_BOARD[i];
			board_num++;
		}
	}

	if(board_num == 0)
	{	
		ret = HMD_DBUS_ID_NO_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&board_num); 	
	//printf("board_num  =  %d  \n",board_num);

	dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING  //slot_no
	                                            					DBUS_TYPE_UINT32_AS_STRING  //Inst_num	
												DBUS_TYPE_UINT32_AS_STRING  //Local_Inst_num
												
												    DBUS_TYPE_ARRAY_AS_STRING  //Hmd_Inst
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING  //Inst_ID
														DBUS_TYPE_UINT32_AS_STRING  //slot_no
														DBUS_TYPE_UINT32_AS_STRING  //InstState
														DBUS_TYPE_UINT32_AS_STRING  //Inst_UNum
														DBUS_TYPE_UINT32_AS_STRING  //Inst_DNum
														DBUS_TYPE_UINT32_AS_STRING  //Inst_GNum
														DBUS_TYPE_UINT32_AS_STRING  //priority
														DBUS_TYPE_UINT32_AS_STRING  //isActive
														
														/*Inst_Hb begin*/
														DBUS_TYPE_STRING_AS_STRING	//ifname
														DBUS_TYPE_UINT32_AS_STRING	 //real_ip
														DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
														DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
											//			DBUS_TYPE_UINT32_AS_STRING	 //umask
											//			DBUS_TYPE_UINT32_AS_STRING	 //dmask
														/*Inst_Hb  end*/
	
														DBUS_TYPE_ARRAY_AS_STRING     //Inst_Uplink
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_STRING_AS_STRING  //ifname
															DBUS_TYPE_UINT32_AS_STRING   //real_ip
															DBUS_TYPE_UINT32_AS_STRING   //vir_ip
															DBUS_TYPE_UINT32_AS_STRING   //remote_r_ip
															DBUS_TYPE_UINT32_AS_STRING   //mask
											//				DBUS_TYPE_UINT32_AS_STRING   //dmask
															DBUS_STRUCT_END_CHAR_AS_STRING
														DBUS_TYPE_ARRAY_AS_STRING     //Inst_Downlink
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_STRING_AS_STRING	//ifname
															DBUS_TYPE_UINT32_AS_STRING	 //real_ip
															DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
															DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
															DBUS_TYPE_UINT32_AS_STRING	 //mask
											//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
															DBUS_STRUCT_END_CHAR_AS_STRING
														DBUS_TYPE_ARRAY_AS_STRING     //Inst_Gateway
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_STRING_AS_STRING	//ifname
															DBUS_TYPE_UINT32_AS_STRING	 //real_ip
															DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
															DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
															DBUS_TYPE_UINT32_AS_STRING	 //mask
											//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
															DBUS_STRUCT_END_CHAR_AS_STRING	
															
												        DBUS_STRUCT_END_CHAR_AS_STRING

	
												     DBUS_TYPE_ARRAY_AS_STRING //Hmd_Local_Inst
													  DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING	//Inst_ID
														DBUS_TYPE_UINT32_AS_STRING	//slot_no
														DBUS_TYPE_UINT32_AS_STRING	//InstState
														DBUS_TYPE_UINT32_AS_STRING	//Inst_UNum
														DBUS_TYPE_UINT32_AS_STRING	//Inst_DNum
														DBUS_TYPE_UINT32_AS_STRING	//Inst_GNum
														DBUS_TYPE_UINT32_AS_STRING	//priority
														DBUS_TYPE_UINT32_AS_STRING	//isActive

														/*Inst_Hb begin*/
														//DBUS_TYPE_STRING_AS_STRING	//ifname
														//DBUS_TYPE_UINT32_AS_STRING	 //real_ip
														//DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
														//DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
														//DBUS_TYPE_UINT32_AS_STRING	 //umask
														//DBUS_TYPE_UINT32_AS_STRING	 //dmask
														/*Inst_Hb  end*/
	
														DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Uplink
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_STRING_AS_STRING	//ifname
															DBUS_TYPE_UINT32_AS_STRING	 //real_ip
															DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
															DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
															DBUS_TYPE_UINT32_AS_STRING	 //mask
											//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
															DBUS_STRUCT_END_CHAR_AS_STRING
														DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Downlink
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_STRING_AS_STRING	//ifname
															DBUS_TYPE_UINT32_AS_STRING	 //real_ip
															DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
															DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
															DBUS_TYPE_UINT32_AS_STRING	 //mask
											//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
															DBUS_STRUCT_END_CHAR_AS_STRING
														DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Gateway
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_STRING_AS_STRING	//ifname
															DBUS_TYPE_UINT32_AS_STRING	 //real_ip
															DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
															DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
															DBUS_TYPE_UINT32_AS_STRING	 //mask
											//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
															DBUS_STRUCT_END_CHAR_AS_STRING															

													 DBUS_STRUCT_END_CHAR_AS_STRING
	                                      				 DBUS_STRUCT_END_CHAR_AS_STRING,
                                   					 &iter_array);	

	for(i = 0; i < board_num; i++)
	{
		int Inst_num = 0;
		int Local_Inst_num = 0;
		dbus_message_iter_open_container (&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_board_struct);
		
		dbus_message_iter_append_basic(&iter_board_struct,DBUS_TYPE_UINT32,&(board[i]->slot_no));
		//printf("  board[%d]->board_func_type  =  %d  \n",i,board[i]->slot_no);

		struct Hmd_Inst_Mgmt **hmd_inst = NULL;
		hmd_inst = malloc(MAX_INSTANCE*(sizeof(struct Hmd_Inst_Mgmt *)));

		struct Hmd_L_Inst_Mgmt **hmd_local_inst = NULL;
		hmd_local_inst = malloc(MAX_INSTANCE*(sizeof(struct Hmd_L_Inst_Mgmt *)));
		for(j = 0; j < MAX_INSTANCE; j++)
		{
			if(board[i]->Hmd_Inst[j])
			{
				hmd_inst[Inst_num] = board[i]->Hmd_Inst[j];
				Inst_num++;
			}
			if(board[i]->Hmd_Local_Inst[j])
			{
				hmd_local_inst[Local_Inst_num] = board[i]->Hmd_Local_Inst[j];
				Local_Inst_num++;
			}
		}
										
		dbus_message_iter_append_basic(&iter_board_struct,DBUS_TYPE_UINT32,&(Inst_num));	
		//printf("  Inst_num  =  %d  \n",Inst_num);
		
		dbus_message_iter_append_basic(&iter_board_struct,DBUS_TYPE_UINT32,&(Local_Inst_num));	
		//printf("  Local_Inst_num  =  %d  \n",Local_Inst_num);
		
		dbus_message_iter_open_container (&iter_board_struct,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
	
											DBUS_TYPE_UINT32_AS_STRING	//Inst_ID
											DBUS_TYPE_UINT32_AS_STRING	//slot_no
											DBUS_TYPE_UINT32_AS_STRING	//InstState
											DBUS_TYPE_UINT32_AS_STRING	//Inst_UNum
											DBUS_TYPE_UINT32_AS_STRING	//Inst_DNum
											DBUS_TYPE_UINT32_AS_STRING	//Inst_GNum
											DBUS_TYPE_UINT32_AS_STRING	//priority
											DBUS_TYPE_UINT32_AS_STRING	//isActive

											/*Inst_Hb begin*/
											DBUS_TYPE_STRING_AS_STRING	//ifname
											DBUS_TYPE_UINT32_AS_STRING	 //real_ip
											DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
											DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
								//			DBUS_TYPE_UINT32_AS_STRING	 //umask
								//			DBUS_TYPE_UINT32_AS_STRING	 //dmask
											/*Inst_Hb  end*/
		
											DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Uplink
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING	//ifname
												DBUS_TYPE_UINT32_AS_STRING	 //real_ip
												DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
												DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
												DBUS_TYPE_UINT32_AS_STRING	 //mask
								//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Downlink
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING	//ifname
												DBUS_TYPE_UINT32_AS_STRING	 //real_ip
												DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
												DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
												DBUS_TYPE_UINT32_AS_STRING	 //mask
								//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Gateway
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING	//ifname
												DBUS_TYPE_UINT32_AS_STRING	 //real_ip
												DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
												DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
												DBUS_TYPE_UINT32_AS_STRING	 //mask
								//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING	
											

	
										 DBUS_STRUCT_END_CHAR_AS_STRING,
								  		&iter_inst_array);

		for(j = 0; j < Inst_num; j++)
		{
			if(hmd_inst[j] != NULL)
			{
				
				dbus_message_iter_open_container (&iter_inst_array, DBUS_TYPE_STRUCT,NULL, &iter_inst_struct);												 
												  												 
				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_ID));
				//printf("  hmd_inst[%d]->Inst_ID  =  %d  \n",i,j,hmd_inst->Inst_ID);

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->slot_no));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->InstState));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_UNum));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_DNum));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_GNum));	

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->priority));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->isActive));
				//printf("  board[%d]->Hmd_Inst[%d]->isActive  =  %d  \n",i,j,board[i]->Hmd_Inst[j]->isActive);
				
				memset(ifname,0,(MAX_IFNAME_LEN+1));
				memcpy(ifname,hmd_inst[j]->Inst_Hb.ifname,MAX_IFNAME_LEN);
				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_STRING ,&(ifname));
				//printf("  board[%d]->Hmd_Inst[%d]->Inst_Hb.ifname  =  %s  \n",i,j,ifname);
				//printf("  board[%d]->Hmd_Inst[%d]->Inst_Hb.ifname =  %p  \n",i,j,board[i]->Hmd_Inst[j]->Inst_Hb.ifname);
				//printf("  board[%d]->Hmd_Inst[%d]->Inst_Hb.ifname =  %s  \n",i,j,board[i]->Hmd_Inst[j]->Inst_Hb.ifname);
									

				
				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Hb.real_ip));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Hb.vir_ip));

				dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Hb.remote_r_ip));

		//		dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Hb.umask));

		//		dbus_message_iter_append_basic (&iter_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Hb.dmask));
				
				dbus_message_iter_open_container (&iter_inst_struct,
												DBUS_TYPE_ARRAY,   //Inst_Uplink
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_STRING_AS_STRING	//ifname
													DBUS_TYPE_UINT32_AS_STRING	 //real_ip
													DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
													DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
													DBUS_TYPE_UINT32_AS_STRING	 //mask
											//		DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&inst_Uplink_array);

				for(k = 0; k < hmd_inst[j]->Inst_UNum; k++)
				{

						dbus_message_iter_open_container (&inst_Uplink_array, DBUS_TYPE_STRUCT,NULL, &inst_Uplink_struct);

						memset(ifname,0,(MAX_IFNAME_LEN+1));
						memcpy(ifname,hmd_inst[j]->Inst_Uplink[k].ifname,MAX_IFNAME_LEN);
						dbus_message_iter_append_basic (&inst_Uplink_struct,DBUS_TYPE_STRING ,&(ifname));
						//printf("  board[%d]->Hmd_Inst[%d]->Inst_Uplink[%d].ifname  =  %s  \n",i,j,k,ifname);
						//printf("  board[%d]->Hmd_Inst[%d]->Inst_Uplink[%d].ifname  =  %p  \n",i,j,k,board[i]->Hmd_Inst[j]->Inst_Uplink[k].ifname);
						
						dbus_message_iter_append_basic (&inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Uplink[k].real_ip));

						dbus_message_iter_append_basic (&inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Uplink[k].vir_ip));

						dbus_message_iter_append_basic (&inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Uplink[k].remote_r_ip));

						dbus_message_iter_append_basic (&inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Uplink[k].mask));

				//		dbus_message_iter_append_basic (&inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Uplink[k].dmask));

						dbus_message_iter_close_container (&inst_Uplink_array, &inst_Uplink_struct);
					
				}
				
				dbus_message_iter_close_container (&iter_inst_struct, &inst_Uplink_array);

				dbus_message_iter_open_container (&iter_inst_struct,
												DBUS_TYPE_ARRAY,   //Inst_Downlink
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_STRING_AS_STRING	//ifname
													DBUS_TYPE_UINT32_AS_STRING	 //real_ip
													DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
													DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
													DBUS_TYPE_UINT32_AS_STRING	 //mask
											//		DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&inst_downlink_array);

				for(k = 0; k < hmd_inst[j]->Inst_DNum; k++)
				{

						dbus_message_iter_open_container (&inst_downlink_array, DBUS_TYPE_STRUCT,NULL, &inst_downlink_struct);	
						
						memset(ifname,0,(MAX_IFNAME_LEN+1));
						memcpy(ifname,hmd_inst[j]->Inst_Downlink[k].ifname,MAX_IFNAME_LEN);
						dbus_message_iter_append_basic (&inst_downlink_struct,DBUS_TYPE_STRING ,&(ifname));
						//printf("  board[%d]->Hmd_Inst[%d]->Inst_Downlink[%d].ifname  =  %s  \n",i,j,k,ifname);
						
						dbus_message_iter_append_basic (&inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Downlink[k].real_ip));

						dbus_message_iter_append_basic (&inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Downlink[k].vir_ip));

						dbus_message_iter_append_basic (&inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Downlink[k].remote_r_ip));

						dbus_message_iter_append_basic (&inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Downlink[k].mask));

					//	dbus_message_iter_append_basic (&inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Downlink[k].dmask));

						dbus_message_iter_close_container (&inst_downlink_array, &inst_downlink_struct);
			
					
				}
				dbus_message_iter_close_container (&iter_inst_struct, &inst_downlink_array);

				dbus_message_iter_open_container (&iter_inst_struct,
												DBUS_TYPE_ARRAY,   //Inst_Gateway
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_STRING_AS_STRING	//ifname
													DBUS_TYPE_UINT32_AS_STRING	 //real_ip
													DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
													DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
													DBUS_TYPE_UINT32_AS_STRING	 //mask
										//			DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&inst_gateway_array);

				for(k = 0; k < hmd_inst[j]->Inst_GNum; k++)
				{

						dbus_message_iter_open_container (&inst_gateway_array, DBUS_TYPE_STRUCT,NULL, &inst_gateway_struct);
						

						memset(ifname,0,(MAX_IFNAME_LEN+1));
						memcpy(ifname,hmd_inst[j]->Inst_Gateway[k].ifname,MAX_IFNAME_LEN);
						dbus_message_iter_append_basic (&inst_gateway_struct,DBUS_TYPE_STRING ,&(ifname));
						//printf("  board[%d]->Hmd_Inst[%d]->Inst_Gateway[%d].ifname  =  %s  \n",i,j,k,ifname);
								
						dbus_message_iter_append_basic (&inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Gateway[k].real_ip));

						dbus_message_iter_append_basic (&inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Gateway[k].vir_ip));

						dbus_message_iter_append_basic (&inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Gateway[k].remote_r_ip));

						dbus_message_iter_append_basic (&inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Gateway[k].mask));

					//	dbus_message_iter_append_basic (&inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_inst[j]->Inst_Gateway[k].dmask));

						dbus_message_iter_close_container (&inst_gateway_array, &inst_gateway_struct);
			
				}
				dbus_message_iter_close_container (&iter_inst_struct, &inst_gateway_array);

				
				dbus_message_iter_close_container (&iter_inst_array, &iter_inst_struct);

			}

		}

		
		dbus_message_iter_close_container (&iter_board_struct, &iter_inst_array);



	

		/*Hmd_Local_Inst*/	
		dbus_message_iter_open_container (&iter_board_struct,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
	
											DBUS_TYPE_UINT32_AS_STRING	//Inst_ID
											DBUS_TYPE_UINT32_AS_STRING	//slot_no
											DBUS_TYPE_UINT32_AS_STRING	//InstState
											DBUS_TYPE_UINT32_AS_STRING	//Inst_UNum
											DBUS_TYPE_UINT32_AS_STRING	//Inst_DNum
											DBUS_TYPE_UINT32_AS_STRING	//Inst_GNum
											DBUS_TYPE_UINT32_AS_STRING	//priority
											DBUS_TYPE_UINT32_AS_STRING	//isActive

											/*Inst_Hb begin*/
											//DBUS_TYPE_STRING_AS_STRING	//ifname
											//DBUS_TYPE_UINT32_AS_STRING	 //real_ip
											//DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
											//DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
											//DBUS_TYPE_UINT32_AS_STRING	 //umask
											//DBUS_TYPE_UINT32_AS_STRING	 //dmask
											/*Inst_Hb  end*/
		
											DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Uplink
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING	//ifname
												DBUS_TYPE_UINT32_AS_STRING	 //real_ip
												DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
												DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
												DBUS_TYPE_UINT32_AS_STRING	 //mask
								//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Downlink
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING	//ifname
												DBUS_TYPE_UINT32_AS_STRING	 //real_ip
												DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
												DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
												DBUS_TYPE_UINT32_AS_STRING	 //mask
								//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING	  //Inst_Gateway
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING	//ifname
												DBUS_TYPE_UINT32_AS_STRING	 //real_ip
												DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
												DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
												DBUS_TYPE_UINT32_AS_STRING	 //mask
								//				DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING	
											

	
										 DBUS_STRUCT_END_CHAR_AS_STRING,
								  		&iter_l_inst_array);


		for(j = 0; j < Local_Inst_num; j++)
		{
			if(hmd_local_inst[j] != NULL)
			{
				
				dbus_message_iter_open_container (&iter_l_inst_array, DBUS_TYPE_STRUCT,NULL, &iter_l_inst_struct);												 
												  												 
				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_ID));
				//printf("  board[%d]->Hmd_Local_Inst[%d]->Inst_ID  =  %d  \n",i,j,board[i]->Hmd_Local_Inst[j]->Inst_ID);

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->slot_no));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->InstState));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_UNum));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_DNum));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_GNum));	

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->priority));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->isActive));
				/*
				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_STRING ,&(board[i]->Hmd_Local_Inst[j]->Inst_Hb->ifname));
				printf("  board[%d]->Hmd_Local_Inst[%d]->Inst_Hb->ifname  =  %s  \n",i,j,board[i]->Hmd_Local_Inst[j]->Inst_Hb->ifname);

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(board[i]->Hmd_Local_Inst[j]->Inst_Hb->real_ip));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(board[i]->Hmd_Local_Inst[j]->Inst_Hb->vir_ip));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(board[i]->Hmd_Local_Inst[j]->Inst_Hb->remote_r_ip));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(board[i]->Hmd_Local_Inst[j]->Inst_Hb->umask));

				dbus_message_iter_append_basic (&iter_l_inst_struct,DBUS_TYPE_UINT32 ,&(board[i]->Hmd_Local_Inst[j]->Inst_Hb->dmask));*/
				
				dbus_message_iter_open_container (&iter_l_inst_struct,
												DBUS_TYPE_ARRAY,   //Inst_Uplink
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_STRING_AS_STRING	//ifname
													DBUS_TYPE_UINT32_AS_STRING	 //real_ip
													DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
													DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
													DBUS_TYPE_UINT32_AS_STRING	 //mask
										//			DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&l_inst_Uplink_array);

				for(k = 0; k < hmd_local_inst[j]->Inst_UNum;k++)
				{

						dbus_message_iter_open_container (&l_inst_Uplink_array, DBUS_TYPE_STRUCT,NULL, &l_inst_Uplink_struct);	

						memset(ifname,0,(MAX_IFNAME_LEN+1));
						memcpy(ifname,hmd_local_inst[j]->Inst_Uplink[k].ifname,MAX_IFNAME_LEN);
						dbus_message_iter_append_basic (&l_inst_Uplink_struct,DBUS_TYPE_STRING ,&(ifname));

						dbus_message_iter_append_basic (&l_inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Uplink[k].real_ip));

						dbus_message_iter_append_basic (&l_inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Uplink[k].vir_ip));

						dbus_message_iter_append_basic (&l_inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Uplink[k].remote_r_ip));

						dbus_message_iter_append_basic (&l_inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Uplink[k].mask));

					//	dbus_message_iter_append_basic (&l_inst_Uplink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Uplink[k].dmask));

						dbus_message_iter_close_container (&l_inst_Uplink_array, &l_inst_Uplink_struct);
					
				}
				
				dbus_message_iter_close_container (&iter_l_inst_struct, &l_inst_Uplink_array);

				dbus_message_iter_open_container (&iter_l_inst_struct,
												DBUS_TYPE_ARRAY,   //Inst_Downlink
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_STRING_AS_STRING	//ifname
													DBUS_TYPE_UINT32_AS_STRING	 //real_ip
													DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
													DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
													DBUS_TYPE_UINT32_AS_STRING	 //mask
										//			DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&l_inst_downlink_array);

				for(k = 0; k < hmd_local_inst[j]->Inst_DNum;k++)
				{

						dbus_message_iter_open_container (&l_inst_downlink_array, DBUS_TYPE_STRUCT,NULL, &l_inst_downlink_struct);	

						memset(ifname,0,(MAX_IFNAME_LEN+1));
						memcpy(ifname,hmd_local_inst[j]->Inst_Downlink[k].ifname,MAX_IFNAME_LEN);
						dbus_message_iter_append_basic (&l_inst_downlink_struct,DBUS_TYPE_STRING ,&(ifname));

						dbus_message_iter_append_basic (&l_inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Downlink[k].real_ip));

						dbus_message_iter_append_basic (&l_inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Downlink[k].vir_ip));

						dbus_message_iter_append_basic (&l_inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Downlink[k].remote_r_ip));

						dbus_message_iter_append_basic (&l_inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Downlink[k].mask));

				//		dbus_message_iter_append_basic (&l_inst_downlink_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Downlink[k].dmask));

						dbus_message_iter_close_container (&l_inst_downlink_array, &l_inst_downlink_struct);
				
					
				}
				dbus_message_iter_close_container (&iter_l_inst_struct, &l_inst_downlink_array);

				dbus_message_iter_open_container (&iter_l_inst_struct,
												DBUS_TYPE_ARRAY,   //Inst_Gateway
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_STRING_AS_STRING	//ifname
													DBUS_TYPE_UINT32_AS_STRING	 //real_ip
													DBUS_TYPE_UINT32_AS_STRING	 //vir_ip
													DBUS_TYPE_UINT32_AS_STRING	 //remote_r_ip
													DBUS_TYPE_UINT32_AS_STRING	 //mask
											//		DBUS_TYPE_UINT32_AS_STRING	 //dmask
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&l_inst_gateway_array);

				for(k = 0; k < hmd_local_inst[j]->Inst_GNum;k++)
				{

						dbus_message_iter_open_container (&l_inst_gateway_array, DBUS_TYPE_STRUCT,NULL, &l_inst_gateway_struct);	

						memset(ifname,0,(MAX_IFNAME_LEN+1));
						memcpy(ifname,hmd_local_inst[j]->Inst_Gateway[k].ifname,MAX_IFNAME_LEN);
						dbus_message_iter_append_basic (&l_inst_gateway_struct,DBUS_TYPE_STRING ,&(ifname));

						dbus_message_iter_append_basic (&l_inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Gateway[k].real_ip));

						dbus_message_iter_append_basic (&l_inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Gateway[k].vir_ip));

						dbus_message_iter_append_basic (&l_inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Gateway[k].remote_r_ip));

						dbus_message_iter_append_basic (&l_inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Gateway[k].mask));

					//	dbus_message_iter_append_basic (&l_inst_gateway_struct,DBUS_TYPE_UINT32 ,&(hmd_local_inst[j]->Inst_Gateway[k].dmask));

						dbus_message_iter_close_container (&l_inst_gateway_array, &l_inst_gateway_struct);
					
				}
				dbus_message_iter_close_container (&iter_l_inst_struct, &l_inst_gateway_array);

				
				dbus_message_iter_close_container (&iter_l_inst_array, &iter_l_inst_struct);

			}
		}

		if(hmd_inst)
		{
			free(hmd_inst);
			hmd_inst = NULL;
		}

		if(hmd_local_inst)
		{
			free(hmd_local_inst);
			hmd_local_inst = NULL;

		}
		dbus_message_iter_close_container (&iter_board_struct, &iter_l_inst_array);
		dbus_message_iter_close_container (&iter_array, &iter_board_struct);
	}
	dbus_message_iter_close_container (&iter, &iter_array);

	if(board)
	{
		free(board);
		board = NULL;
	}
	if(ifname)
	{
		free(ifname);
		ifname = NULL;
	}
	printf("   hmd  is over  \n");
	return reply;	
}
/*fengwenchao add end*/
DBusMessage * hmd_dbus_interface_set_hansi_disable_enable(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;	
	int i = 0;
	char *ifname = NULL;
	int vip = 0;
	int mask = 0;
	char mac[MAC_LEN];
    unsigned int profile = 0;
    unsigned int slotid = 0;
	int enable = -1;
	unsigned int	ret = HMD_DBUS_SUCCESS;
	unsigned int neighbor_slotid = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&slotid, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&enable,  \
					DBUS_TYPE_INVALID)))
	{
		hmd_syslog_err("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			hmd_syslog_err("vrrp service %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	if(slotid != HOST_SLOT_NO){
		
		hmd_syslog_info("%s 1\n",__func__);
		if((HMD_BOARD[slotid] != NULL)&&(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]!= NULL)){
			hmd_syslog_info("%s 2\n",__func__);
			HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->InstState = enable;			
			if(HMD_L_HANSI[profile]){
				hmd_syslog_info("%s 3\n",__func__);
				if(HMD_L_HANSI[profile]->slot_no == slotid){
					hmd_syslog_info("%s 4\n",__func__);
					HMD_L_HANSI[profile]->InstState = enable;
					if(HMD_L_HANSI[profile]->slot_num == 2){
						hmd_syslog_info("%s 5\n",__func__);
						HMD_BOARD[HMD_L_HANSI[profile]->slot_no1]->Hmd_Local_Inst[profile]->InstState1 = enable;
					}
				}else if(HMD_L_HANSI[profile]->slot_no1 == slotid){
					hmd_syslog_info("%s 6\n",__func__);
					HMD_L_HANSI[profile]->InstState1 = enable;
					if(HMD_L_HANSI[profile]->slot_num == 2){
						hmd_syslog_info("%s 7\n",__func__);
						HMD_BOARD[HMD_L_HANSI[profile]->slot_no]->Hmd_Local_Inst[profile]->InstState1= enable;
					}
				}
			}
			if(enable){
				hmd_syslog_info("%s 8\n",__func__);
				if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num == 2){
					hmd_syslog_info("%s 9\n",__func__);
					neighbor_slotid = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no1;
					if((HMD_BOARD[neighbor_slotid] != NULL)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]!= NULL)){
						hmd_syslog_info("%s 10\n",__func__);
						if((HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->InstState == 1)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive == INST_ACTIVE)){
							hmd_syslog_info("%s 11\n",__func__);
							HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_BACKUP;
							HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive1 = INST_BACKUP;
						}else{
							hmd_syslog_info("%s 12\n",__func__);
							HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
							HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive1 = INST_ACTIVE; 
						}
					}else{
						hmd_syslog_info("%s 13\n",__func__);
						HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
					}
					ret = configuration_server_to_client(slotid, profile,neighbor_slotid);
				}else{
					hmd_syslog_info("%s 14\n",__func__);
					HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
					ret = configuration_server_to_client(slotid, profile,neighbor_slotid);
				}
			}else{
				hmd_syslog_info("%s 15\n",__func__);
				if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num == 2){
					neighbor_slotid = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no1;
					hmd_syslog_info("%s 16\n",__func__);
					if((HMD_BOARD[neighbor_slotid] != NULL)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]!= NULL)){
						hmd_syslog_info("%s 17\n",__func__);
						if((HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->InstState == 1)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive == INST_BACKUP)){
							hmd_syslog_info("%s 18\n",__func__);
							HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
							HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive1 = INST_DISABLE;
							HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive1 = INST_ACTIVE;
							if(neighbor_slotid != HOST_SLOT_NO){
								hmd_syslog_info("%s 19\n",__func__);
								ret = configuration_server_to_client(neighbor_slotid, profile,slotid);
							}
							else{								
								hmd_syslog_info("%s 20\n",__func__);
								
								for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum; i++){
									ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].ifname;
									vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].vir_ip;
									mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mask;
									hmd_ifname_to_mac(ifname,mac);
									memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mac,mac,MAC_LEN);
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}
								for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum; i++){
									ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].ifname;
									vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].vir_ip;
									mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mask;
									hmd_ifname_to_mac(ifname,mac);
									memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mac,mac,MAC_LEN);
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}								
								for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_GNum; i++){
									ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].ifname;
									vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].vir_ip;
									mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mask;
									hmd_ifname_to_mac(ifname,mac);
									memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mac,mac,MAC_LEN);
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}								
								notice_wid_local_hansi_service_change_state(profile, slotid);
								notice_eag_local_hansi_service_change_state(profile, slotid);
								hmd_syslog_info("%s 21\n",__func__);
							}
						}						
					}
				}
				hmd_syslog_info("%s 22\n",__func__);
				HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_DISABLE;											
				ret = configuration_server_to_client(slotid, profile,neighbor_slotid);
			}
		}else
			ret = HMD_DBUS_SLOT_ID_NOT_EXIST;
	}else{
		if(HOST_BOARD->Hmd_Local_Inst[profile] != NULL){
			hmd_syslog_info("%s 23\n",__func__);
			HOST_BOARD->Hmd_Local_Inst[profile]->InstState = enable;			
			if((HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum == 0) && (HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum == 0)){
				ret = HMD_DBUS_NOCONFIG_DOWNLINK;
			}else{
				hmd_syslog_info("%s 24\n",__func__);
				if(HMD_L_HANSI[profile]){
					if(HMD_L_HANSI[profile]->slot_no == slotid){
						HMD_L_HANSI[profile]->InstState = enable;
						if(HMD_L_HANSI[profile]->slot_num == 2){
							HMD_BOARD[HMD_L_HANSI[profile]->slot_no1]->Hmd_Local_Inst[profile]->InstState1 = enable;
						}
					}else if(HMD_L_HANSI[profile]->slot_no1 == slotid){
						HMD_L_HANSI[profile]->InstState1 = enable;
						if(HMD_L_HANSI[profile]->slot_num == 2){
							HMD_BOARD[HMD_L_HANSI[profile]->slot_no]->Hmd_Local_Inst[profile]->InstState1= enable;
						}
					}
				}
				hmd_syslog_info("%s 25\n",__func__);
				
				if(enable){
					hmd_syslog_info("%s 26\n",__func__);
					if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num == 2){
						hmd_syslog_info("%s 27\n",__func__);
						neighbor_slotid = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no1;
						if((HMD_BOARD[neighbor_slotid] != NULL)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]!= NULL)){
							if((HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->InstState == 1)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive == INST_ACTIVE)){
								HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_BACKUP;
							}else{
								HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
							}
						}else{
							HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
						}
						hmd_syslog_info("%s 28\n",__func__);
						
						for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum; i++){
							ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].ifname;
							vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].vir_ip;
							mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mask;
							hmd_ifname_to_mac(ifname,mac);
							memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mac,mac,MAC_LEN);
							if(HOST_BOARD->Hmd_Local_Inst[profile]->isActive == INST_ACTIVE){
								hmd_ipaddr_op_withmask(ifname,vip,mask,1);
								send_tunnel_interface_arp(mac,vip,ifname);	
							}else{
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
						}
						for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum; i++){
							ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].ifname;
							vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].vir_ip;
							mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mask;
							hmd_ifname_to_mac(ifname,mac);
							memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mac,mac,MAC_LEN);
							if(HOST_BOARD->Hmd_Local_Inst[profile]->isActive == INST_ACTIVE){
								hmd_ipaddr_op_withmask(ifname,vip,mask,1);
								send_tunnel_interface_arp(mac,vip,ifname);	
							}else{
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
						}
						for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_GNum; i++){
							ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].ifname;
							vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].vir_ip;
							mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mask;
							hmd_ifname_to_mac(ifname,mac);
							memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mac,mac,MAC_LEN);
							if(HOST_BOARD->Hmd_Local_Inst[profile]->isActive == INST_ACTIVE){
								hmd_ipaddr_op_withmask(ifname,vip,mask,1);
								send_tunnel_interface_arp(mac,vip,ifname);	
							}else{
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
						}						notice_wid_local_hansi_service_change_state(profile, neighbor_slotid);
						notice_eag_local_hansi_service_change_state(profile, neighbor_slotid);
						hmd_syslog_info("%s 29\n",__func__);
					}else{
						HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
						hmd_syslog_info("%s 30\n",__func__);						
						for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum; i++){
							ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].ifname;
							vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].vir_ip;
							mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mask;
							hmd_ifname_to_mac(ifname,mac);
							memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mac,mac,MAC_LEN);
							hmd_ipaddr_op_withmask(ifname,vip,mask,1);
							send_tunnel_interface_arp(mac,vip,ifname);	
						}						
						for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum; i++){
							ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].ifname;
							vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].vir_ip;
							mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mask;
							hmd_ifname_to_mac(ifname,mac);
							memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mac,mac,MAC_LEN);
							hmd_ipaddr_op_withmask(ifname,vip,mask,1);
							send_tunnel_interface_arp(mac,vip,ifname);	
						}
						for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_GNum; i++){
							ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].ifname;
							vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].vir_ip;
							mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mask;
							hmd_ifname_to_mac(ifname,mac);
							memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mac,mac,MAC_LEN);
							hmd_ipaddr_op_withmask(ifname,vip,mask,1);
							send_tunnel_interface_arp(mac,vip,ifname);	
						}
						notice_wid_local_hansi_service_change_state(profile, neighbor_slotid);
						notice_eag_local_hansi_service_change_state(profile, neighbor_slotid);
						hmd_syslog_info("%s 31\n",__func__);
					}
				}else{
					if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num == 2){
						neighbor_slotid = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no1;
						if((HMD_BOARD[neighbor_slotid] != NULL)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]!= NULL)){
							if((HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->InstState == 1)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive == INST_BACKUP)){
								HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
								ret = configuration_server_to_client(neighbor_slotid, profile,slotid);
							}						
						}
					}
					HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive = INST_DISABLE;											
					hmd_syslog_info("%s 32\n",__func__);
					
					for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum; i++){
						ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].ifname;
						vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].vir_ip;
						mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mask;
						hmd_ifname_to_mac(ifname,mac);
						memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mac,mac,MAC_LEN);
						hmd_ipaddr_op_withmask(ifname,vip,mask,0);
					}
					for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum; i++){
						ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].ifname;
						vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].vir_ip;
						mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mask;
						hmd_ifname_to_mac(ifname,mac);
						memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mac,mac,MAC_LEN);
						hmd_ipaddr_op_withmask(ifname,vip,mask,0);
					}
					for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_GNum; i++){
						ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].ifname;
						vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].vir_ip;
						mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mask;
						hmd_ifname_to_mac(ifname,mac);
						memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mac,mac,MAC_LEN);
						hmd_ipaddr_op_withmask(ifname,vip,mask,0);
					}

					notice_wid_local_hansi_service_change_state(profile, neighbor_slotid);
					notice_eag_local_hansi_service_change_state(profile, neighbor_slotid);
					hmd_syslog_info("%s 33\n",__func__);
				}
			}
		}
		else
			ret = HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;
	}
	if(ret == HMD_DBUS_SUCCESS){
		if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slotid)){
			syn_hansi_info_to_backup(slotid, profile, MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_MODIFY,0);
		}
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		hmd_syslog_err("hmd service dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;	
}


DBusMessage * hmd_dbus_interface_delete_remote_hansi(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int ID = 0;
    unsigned int slot_id = 0;
    DBusError err;
    int ret = HMD_DBUS_SUCCESS;
    int local_id = 0;   
    char command[128] = {0};
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,
                             DBUS_TYPE_UINT32,&ID,
                             DBUS_TYPE_UINT32,&slot_id,
                             DBUS_TYPE_INVALID))) 
    {
        hmd_syslog_err("set hansi profile:Unable to get input args ");
        if (dbus_error_is_set(&err)) {
            hmd_syslog_err("set hansi profile %s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }   
    if((slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL)){
        if(HMD_BOARD[slot_id]->Hmd_Inst[ID] != NULL){
			if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slot_id)){
				syn_hansi_info_to_backup(slot_id, ID, MASTER_BACKUP_SLOT_NO,0,HMD_HANSI_INFO_SYN_DEL,0);
			}			
			sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
						0,slot_id,ID, ".pid");
			system(command);
            if(slot_id != HOST_SLOT_NO){
                HmdNoticeToClient(slot_id,ID,local_id,HMD_DELETE);				
				free(HMD_BOARD[slot_id]->Hmd_Inst[ID]);
				HMD_BOARD[slot_id]->Hmd_Inst[ID] = NULL;
            }else{
                HmdSetAssambleHansiMsg(slot_id,ID,local_id,HMD_DELETE);
            }
        }else{
			sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
						0,slot_id,ID, ".pid");
			system(command);
		}
    }else{
        ret = HMD_DBUS_ID_NO_EXIST;
    }
    reply = dbus_message_new_method_return(msg);
    if(NULL == reply){      
        hmd_syslog_err("vrrp set hansi profile dbus reply null!\n");
        return reply;
    }
        
    dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    
    return reply;
    
}

int hmd_notice_vrrp_config_service_change_state(unsigned int InstID, unsigned int enable)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
//#define PATH_LEN 64
	char OBJPATH[PATH_LEN] = {0};
	char BUSNAME[PATH_LEN] = {0};
	int ret = 0;	
	DBusConnection * connection = NULL;
	if((InstID >0)&&(InstID < MAX_INSTANCE)&&(HOST_BOARD->Hmd_Inst[InstID]))
		connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
	else{
		hmd_syslog_err("%s %d.\n",__func__,__LINE__);
		return -1;
	}
	HMDReInitHadDbusPath(InstID,VRRP_DBUS_BUSNAME,BUSNAME,0);
	HMDReInitHadDbusPath(InstID,VRRP_DBUS_OBJPATH,OBJPATH,0);
	query = dbus_message_new_method_call(BUSNAME,
										OBJPATH,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &InstID,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, 150000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		hmd_syslog_err("%s %d\n",__func__,__LINE__);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}

		return -1;
	}
	else if (!(dbus_message_get_args (reply, &err, DBUS_TYPE_UINT32,&ret, DBUS_TYPE_INVALID))){		
		hmd_syslog_info("%s %d\n",__func__,__LINE__);
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	
	hmd_syslog_info("%s %d\n",__func__,__LINE__);
	dbus_message_unref(reply);

	return 0;
}

DBusMessage * hmd_dbus_get_wid_update_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;

	int state = -1,vrid = 0;
	int ret = 0;
	int localed = 0;
	time_t	*r_time = NULL;
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&localed,
		                        DBUS_TYPE_UINT32,&vrid,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		
		return NULL;	
	}
	hmd_syslog_info("%s %d,get vrid(%d) state(%d) from wid.\n",__func__,__LINE__,vrid,state);

	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		hmd_syslog_err("hmd dbus set error!\n");
		return reply;
	}
	if((vrid > 0)&&(vrid < MAX_INSTANCE)){
		if(0 == localed){
			if((HOST_BOARD->Hmd_Inst[vrid])&&(1 == HOST_BOARD->Hmd_Inst[vrid]->HmdBakForever)){
				HOST_BOARD->Hmd_Inst[vrid]->HmdWidUpOk = 1;
				HMDTimerRequest(HMD_CHECKING_UPDATE_TIMER,&(HOST_BOARD->Hmd_Inst[vrid]->HmdCheckUpdateTimerID), HMD_CHECKING_UPDATE, vrid, 1);
			}else{
				hmd_syslog_info("%s %d,get vrid(%d) state(%d) from wid,but hmdbakforever sw is disable.\n",__func__,__LINE__,vrid,state);
			}
			if(HOST_BOARD->Hmd_Inst[vrid]){
				HOST_BOARD->Hmd_Inst[vrid]->WidReadyTimes ++;
				
				r_time = (time_t *)malloc(sizeof(time_t));
				time(r_time);
				HOST_BOARD->Hmd_Inst[vrid]->WidLastReadyTime = *r_time;
				free(r_time);
				r_time = NULL;
			}
		}else{
			if((HOST_BOARD->Hmd_Local_Inst[vrid])&&(1 == HOST_BOARD->Hmd_Local_Inst[vrid]->HmdBakForever)){
				HOST_BOARD->Hmd_Local_Inst[vrid]->HmdWidUpOk = 1;
				HMDTimerRequest(HMD_CHECKING_UPDATE_TIMER,&(HOST_BOARD->Hmd_Local_Inst[vrid]->HmdCheckUpdateTimerID), HMD_CHECKING_UPDATE, vrid, 1);
			}else{
				hmd_syslog_info("%s %d,get vrid(%d) state(%d) from wid,but hmdbakforever sw is disable.\n",__func__,__LINE__,vrid,state);
			}
			if(HOST_BOARD->Hmd_Local_Inst[vrid]){
				HOST_BOARD->Hmd_Local_Inst[vrid]->WidReadyTimes ++;
				
				r_time = (time_t *)malloc(sizeof(time_t));
				time(r_time);
				HOST_BOARD->Hmd_Local_Inst[vrid]->WidLastReadyTime = *r_time;
				free(r_time);
				r_time = NULL;
			}
		}
	}
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,&ret);
	return reply;

}

DBusMessage * hmd_dbus_get_asd_update_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;

	int state = -1,vrid = 0;
	int ret = 0;
	int localed = 0;
	dbus_error_init( &err );
	time_t	*r_time = NULL;
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&localed,
		                        DBUS_TYPE_UINT32,&vrid,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		
		return NULL;	
	}
	hmd_syslog_info("%s %d,get vrid(%d) state(%d) from asd.\n",__func__,__LINE__,vrid,state);

	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		hmd_syslog_err("hmd dbus set error!\n");
		return reply;
	}
	if((vrid > 0)&&(vrid < MAX_INSTANCE)){
		if(0 == localed){
			if((HOST_BOARD->Hmd_Inst[vrid])&&(1 == HOST_BOARD->Hmd_Inst[vrid]->HmdBakForever)){
				if((HOST_BOARD->Hmd_Inst[vrid])&&(1 == HOST_BOARD->Hmd_Inst[vrid]->HmdWidUpOk)){
					HmdTimerCancel(&(HOST_BOARD->Hmd_Inst[vrid]->HmdCheckUpdateTimerID),1);
					int h_times = 0;
					while(h_times < 3){
						if(-1 == hmd_notice_vrrp_config_service_change_state(vrid,0)){
							h_times ++;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),sleep(2),send(disable) again.\n",__func__,__LINE__,vrid,h_times);
							sleep(2);
						}else{
							h_times = 4;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),send(disable) successfull,break.\n",__func__,__LINE__,vrid,h_times);
							break;
						}
					}
					h_times = 0;
					while(h_times < 3){
						if(-1 == hmd_notice_vrrp_config_service_change_state(vrid,1)){
							h_times ++;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),sleep(2),send(enable) again.\n",__func__,__LINE__,vrid,h_times);
							sleep(2);
						}else{
							h_times = 4;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),send(enable) successfull,break.\n",__func__,__LINE__,vrid,h_times);
							break;
						}
					}
					HOST_BOARD->Hmd_Inst[vrid]->HmdWidUpOk = 0;
				}else{
					hmd_syslog_warning("hmd not receive from wid,but receive from asd,not change hansi state!\n");
				}
			}else{
				hmd_syslog_info("%s %d,get vrid(%d) state(%d) from asd,but hmdbakforever sw is disable.\n",__func__,__LINE__,vrid,state);
			}
			if(HOST_BOARD->Hmd_Inst[vrid]){
				HOST_BOARD->Hmd_Inst[vrid]->AsdReadyTimes ++;
				r_time = (time_t *)malloc(sizeof(time_t));
				time(r_time);
				HOST_BOARD->Hmd_Inst[vrid]->AsdLastReadyTime = *r_time;
				free(r_time);
				r_time = NULL;
			}
		}else{
			if((HOST_BOARD->Hmd_Local_Inst[vrid])&&(1 == HOST_BOARD->Hmd_Local_Inst[vrid]->HmdBakForever)){
				if((HOST_BOARD->Hmd_Local_Inst[vrid])&&(1 == HOST_BOARD->Hmd_Local_Inst[vrid]->HmdWidUpOk)){
					HmdTimerCancel(&(HOST_BOARD->Hmd_Local_Inst[vrid]->HmdCheckUpdateTimerID),1);
					int h_times = 0;
					while(h_times < 3){
						if(-1 == hmd_notice_vrrp_config_service_change_state(vrid,0)){
							h_times ++;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),sleep(2),send(disable) again.\n",__func__,__LINE__,vrid,h_times);
							sleep(2);
						}else{
							h_times = 4;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),send(disable) successfull,break.\n",__func__,__LINE__,vrid,h_times);
							break;
						}
					}
					h_times = 0;
					while(h_times < 3){
						if(-1 == hmd_notice_vrrp_config_service_change_state(vrid,1)){
							h_times ++;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),sleep(2),send(enable) again.\n",__func__,__LINE__,vrid,h_times);
							sleep(2);
						}else{
							h_times = 4;
							hmd_syslog_info("%s %d,vrid(%d) h_times(%d),send(enable) successfull,break.\n",__func__,__LINE__,vrid,h_times);
							break;
						}
					}
					HOST_BOARD->Hmd_Local_Inst[vrid]->HmdWidUpOk = 0;
				}else{
					hmd_syslog_warning("hmd not receive from wid,but receive from asd,not change hansi state!\n");
				}
			}else{
				hmd_syslog_info("%s %d,get vrid(%d) state(%d) from asd,but hmdbakforever sw is disable.\n",__func__,__LINE__,vrid,state);
			}
			if(HOST_BOARD->Hmd_Local_Inst[vrid]){
				HOST_BOARD->Hmd_Local_Inst[vrid]->AsdReadyTimes ++;
				r_time = (time_t *)malloc(sizeof(time_t));
				time(r_time);
				HOST_BOARD->Hmd_Local_Inst[vrid]->AsdLastReadyTime = *r_time;
				free(r_time);
				r_time = NULL;
			}
		}
	}

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,&ret);
	return reply;

}


/*book add */
static DBusMessage *hmd_ha_set_state( DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	hmd_syslog_debug_debug(HMD_DEFAULT,"had_ha_set_state");
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusMessageIter	 iter_array, iter_array1, iter_array2;	
	DBusError		err;
	unsigned int ID = 0;
	struct Hmd_Inst_Mgmt *hmd_rmt_hansi = NULL;
	if(HOST_BOARD == NULL){
	    hmd_syslog_err("HOST_BOARD = NULL\n");
	    return NULL;
	}
	
	char master_ip_char[32]="";
	char backup_ip_char[32]="";
	char virtual_ip_char[32]="";
	int j = 0;
	char * interface_name;

	dbus_error_init( &err );
	hmd_syslog_info("%s inst %d\n",__func__, ID);

	//memset( &params, 0, sizeof(params));

	hmd_syslog_debug_debug(HMD_DEFAULT,"--------------- HA instance info ---------------");
	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&(ID));

	if(HOST_BOARD->Hmd_Inst[ID] == NULL){
		hmd_syslog_info("%s inst %d == NULL\n",__func__, ID);
	    HOST_BOARD->Hmd_Inst[ID] = (struct Hmd_Inst_Mgmt *)malloc(sizeof(struct Hmd_Inst_Mgmt));
	}

    hmd_rmt_hansi = HOST_BOARD->Hmd_Inst[ID];
 
    hmd_rmt_hansi->Inst_ID = ID;
    hmd_rmt_hansi->vrrid = ID;
	
	dbus_message_iter_next(&iter);	
	//dbus_message_iter_get_basic(&iter,&(params.state));
	dbus_message_iter_get_basic(&iter,&(hmd_rmt_hansi->InstState));
	if(hmd_rmt_hansi->InstState == 3)
		hmd_rmt_hansi->isActive = 1;
	else
		hmd_rmt_hansi->isActive = 0;		
	hmd_syslog_debug_debug(HMD_DEFAULT,"...vrid %d state %d", hmd_rmt_hansi->vrrid, hmd_rmt_hansi->InstState);

	dbus_message_iter_next(&iter);	
	//dbus_message_iter_get_basic(&iter,&(params.uplink_cnt));
	dbus_message_iter_get_basic(&iter,&(hmd_rmt_hansi->Inst_UNum));
	
	hmd_syslog_debug_debug(HMD_DEFAULT,"...uplink num %d", hmd_rmt_hansi->Inst_UNum);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for (j = 0; j < hmd_rmt_hansi->Inst_UNum; j++)
	{
		if(j >= MAX_IFNAME_NUM)
		{
			hmd_syslog_err("error!params.uplink_cnt=%d > MAX_PHA_IF_NUM=%d",hmd_rmt_hansi->Inst_UNum, MAX_IFNAME_NUM);
			break;
		}
		DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);	
		//dbus_message_iter_get_basic(&iter_struct,&(params.uplink_interface[j].master_ip));
		dbus_message_iter_get_basic(&iter_struct,&(hmd_rmt_hansi->Inst_Uplink[j].real_ip));

		dbus_message_iter_next(&iter_struct);
		//dbus_message_iter_get_basic(&iter_struct,&(params.uplink_interface[j].backp_ip));
		dbus_message_iter_get_basic(&iter_struct,&(hmd_rmt_hansi->Inst_Uplink[j].remote_r_ip));

		dbus_message_iter_next(&iter_struct);
		//dbus_message_iter_get_basic(&iter_struct,&(params.uplink_interface[j].virtual_ip));
		dbus_message_iter_get_basic(&iter_struct,&(hmd_rmt_hansi->Inst_Uplink[j].vir_ip));
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(interface_name));

		//strncpy( params.uplink_interface[j].if_name, interface_name, sizeof(params.uplink_interface[j].if_name)-1);
		strncpy( hmd_rmt_hansi->Inst_Uplink[j].ifname, interface_name, sizeof(hmd_rmt_hansi->Inst_Uplink[j].ifname)-1);
		
		dbus_message_iter_next(&iter_array); 
		
		hmd_syslog_debug_debug(HMD_DEFAULT,"...uplink %d: master %s backup %s virtual %s ifname %s",
					j,
					inet_int2str(hmd_rmt_hansi->Inst_Uplink[j].real_ip, master_ip_char, sizeof(master_ip_char) ),
					inet_int2str(hmd_rmt_hansi->Inst_Uplink[j].remote_r_ip, backup_ip_char, sizeof(backup_ip_char) ),
					inet_int2str(hmd_rmt_hansi->Inst_Uplink[j].vir_ip, virtual_ip_char, sizeof(virtual_ip_char) ),
					hmd_rmt_hansi->Inst_Uplink[j].ifname );
	}

	dbus_message_iter_next(&iter);	
	//dbus_message_iter_get_basic(&iter,&(params.downlink_cnt));
	dbus_message_iter_get_basic(&iter,&(hmd_rmt_hansi->Inst_DNum));
	
	hmd_syslog_debug_debug(HMD_DEFAULT,"...downlink num %d", hmd_rmt_hansi->Inst_DNum);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array1);
	for (j = 0; j < hmd_rmt_hansi->Inst_DNum; j++)
	{
		if(j >= MAX_IFNAME_NUM)
		{
			hmd_syslog_err("error!params.downlink_cnt=%d > MAX_PHA_IF_NUM=%d",hmd_rmt_hansi->Inst_DNum, MAX_IFNAME_NUM);
			break;
		}
		DBusMessageIter iter_struct1;
		dbus_message_iter_recurse(&iter_array1,&iter_struct1);	
		//dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_interface[j].master_ip));		
		dbus_message_iter_get_basic(&iter_struct1,&(hmd_rmt_hansi->Inst_Downlink[j].real_ip));

		dbus_message_iter_next(&iter_struct1);
		//dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_interface[j].backp_ip));
		dbus_message_iter_get_basic(&iter_struct1,&(hmd_rmt_hansi->Inst_Downlink[j].remote_r_ip));

		dbus_message_iter_next(&iter_struct1);
		//dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_interface[j].virtual_ip));
		dbus_message_iter_get_basic(&iter_struct1,&(hmd_rmt_hansi->Inst_Downlink[j].vir_ip));
		
		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(interface_name));

		//strncpy( params.downlink_interface[j].if_name, interface_name, sizeof(params.downlink_interface[j].if_name)-1);
		strncpy( hmd_rmt_hansi->Inst_Downlink[j].ifname, interface_name, sizeof(hmd_rmt_hansi->Inst_Downlink[j].ifname)-1);
		
		dbus_message_iter_next(&iter_array1); 
		
		hmd_syslog_debug_debug(HMD_DEFAULT,"...downlink %d: master %s backup %s virtual %s ifname %s",
					j,
					inet_int2str(hmd_rmt_hansi->Inst_Downlink[j].real_ip, master_ip_char,sizeof(master_ip_char)),
					inet_int2str(hmd_rmt_hansi->Inst_Downlink[j].remote_r_ip, backup_ip_char,sizeof(backup_ip_char)),
					inet_int2str(hmd_rmt_hansi->Inst_Downlink[j].vir_ip, virtual_ip_char,sizeof(virtual_ip_char)),
					hmd_rmt_hansi->Inst_Downlink[j].ifname );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(interface_name));
	//strncpy( params.heartlink_if_name, interface_name, sizeof(params.heartlink_if_name)-1);
	strncpy( hmd_rmt_hansi->Inst_Hb.ifname, interface_name, sizeof(hmd_rmt_hansi->Inst_Hb.ifname)-1);
	
	dbus_message_iter_next(&iter);
	//dbus_message_iter_get_basic(&iter,&(params.heartlink_local_ip));
	dbus_message_iter_get_basic(&iter,&(hmd_rmt_hansi->Inst_Hb.real_ip));
	
	dbus_message_iter_next(&iter);
	//dbus_message_iter_get_basic(&iter,&(params.heartlink_opposite_ip));	
	dbus_message_iter_get_basic(&iter,&(hmd_rmt_hansi->Inst_Hb.remote_r_ip));

	hmd_syslog_debug_debug(HMD_DEFAULT,"...heartlink: name %s local_ip %s opposite_ip %s",
					hmd_rmt_hansi->Inst_Hb.ifname,
					inet_int2str(hmd_rmt_hansi->Inst_Hb.real_ip,master_ip_char,sizeof(master_ip_char)),
					inet_int2str(hmd_rmt_hansi->Inst_Hb.remote_r_ip,backup_ip_char,sizeof(backup_ip_char)) );
	
	dbus_message_iter_next(&iter);	
	//dbus_message_iter_get_basic(&iter,&(params.vgateway_cnt));
	dbus_message_iter_get_basic(&iter,&(hmd_rmt_hansi->Inst_GNum));
	
	hmd_syslog_debug_debug(HMD_DEFAULT,"...vgateway num %d", hmd_rmt_hansi->Inst_GNum);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array2);
	
	for (j = 0; j < hmd_rmt_hansi->Inst_GNum; j++)
	{
		if(j >= MAX_IFNAME_NUM)
		{
			hmd_syslog_err("error!params.vgateway_cnt=%d > MAX_PHA_IF_NUM=%d",hmd_rmt_hansi->Inst_GNum, MAX_IFNAME_NUM);
			break;
		}
		DBusMessageIter iter_struct2;
		dbus_message_iter_recurse(&iter_array2,&iter_struct2);	
		dbus_message_iter_get_basic(&iter_struct2,&(interface_name));
		
		dbus_message_iter_next(&iter_struct2);
		//dbus_message_iter_get_basic(&iter_struct2,&(params.vgateway_interface[j].virtual_ip));
		dbus_message_iter_get_basic(&iter_struct2,&(hmd_rmt_hansi->Inst_Gateway[j].vir_ip));
		
		//strncpy( params.vgateway_interface[j].if_name, interface_name, sizeof(params.vgateway_interface[j].if_name)-1);
		strncpy( hmd_rmt_hansi->Inst_Gateway[j].ifname, interface_name, sizeof(hmd_rmt_hansi->Inst_Gateway[j].ifname)-1);
		
		dbus_message_iter_next(&iter_array2); 
	
		hmd_syslog_debug_debug(HMD_DEFAULT,"...vgateway %d: virtual %s ifname %s",
					j,
					inet_int2str(hmd_rmt_hansi->Inst_Gateway[j].vir_ip,virtual_ip_char,sizeof(virtual_ip_char)),
					hmd_rmt_hansi->Inst_Gateway[j].ifname);
	}

	/* test */
	/*
	params.master_uplinkip = params.uplink_interface[0].master_ip;
	params.master_downlinkip = params.downlink_interface[0].master_ip;
	params.back_uplinkip = params.uplink_interface[0].backp_ip;
	params.back_downlinkip = params.downlink_interface[0].backp_ip;
	params.virtual_uplink_ip = params.uplink_interface[0].virtual_ip;
	params.virtual_downlink_ip = params.downlink_interface[0].virtual_ip;
	params.heartlink_master_ip = params.heartlink_opposite_ip;	
	strncpy(params.uplink_if, params.uplink_interface[0].if_name, sizeof(params.uplink_if));
	strncpy(params.downlink_if, params.downlink_interface[0].if_name, sizeof(params.downlink_if));
	strncpy(params.heartlink_if, params.heartlink_if_name, sizeof(params.heartlink_if));

	if( NULL != onstate_change_callback )
	{
		onstate_change_callback( &params );
		hmd_syslog_debug_debug(HMD_DEFAULT,"call the portal_ha ins done");
	}
	*/
    
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		hmd_syslog_err("portal HA dbus set state get reply message null error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(hmd_rmt_hansi->vrrid));
	if(HOST_SLOT_NO != MASTER_SLOT_NO)
		notice_hmd_server_hansi_info_update(ID,hmd_rmt_hansi);
	return reply;
} 

DBusMessage * hmd_dbus_ha_get_alive_slots(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;
	int i = 0;
	unsigned int alive_slot_cnt = 0;
	
	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);
	if(NULL == reply){		
		hmd_syslog_err("vrrp set hansi profile dbus reply null!\n");
		return reply;
	}

    /* count alive slot numbers */
    for(i = 0; i < MAX_SLOT_NUM; i++){
    	if(HMD_BOARD[i] != NULL){
    		alive_slot_cnt++;
    	}
	}
    		
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&alive_slot_cnt);

	for(i = 0; i < MAX_SLOT_NUM; i++){
    	if(HMD_BOARD[i] != NULL){
    		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(HMD_BOARD[i]->slot_no));
    	}
	}
	
	return reply;
	
}

int hmd_license_synchronize(int slotid,int licensetype, int licensenum){
	if(HMD_BOARD[slotid] == NULL){
		return 0;
	}
	struct sockaddr_tipc * addr = &(HMD_BOARD[slotid]->tipcaddr);
	int fd = HMD_BOARD[slotid]->tipcfd;
	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	msg.op = HMD_LICENSE_SYNCHRONIZE;
	msg.D_SlotID = slotid;
	msg.S_SlotID = HOST_SLOT_NO;
	msg.u.LicenseInfo.licenseType = licensetype;
	msg.u.LicenseInfo.licenseNum = licensenum;	
	memcpy(msg.u.LicenseInfo.licreq, LICENSE_MGMT[licensetype].licreq, LICREQ_LEN);	
	char buf[4096];
	memset(buf,0,4096);
	int len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	if (0 > sendto(fd,buf,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	return 0;
}

int notice_hmd_client_license_change(int slotid,int licensetype, int inst_id, int islocaled){
	struct sockaddr_tipc * addr = &(HMD_BOARD[slotid]->tipcaddr);
	int fd = HMD_BOARD[slotid]->tipcfd;
	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	msg.op = HMD_LICENSE_UPDATE;
	msg.D_SlotID = slotid;
	msg.S_SlotID = HOST_SLOT_NO;
	msg.InstID = inst_id;
	msg.local = islocaled;
	msg.u.LicenseInfo.licenseType = licensetype;
	if(islocaled)
		msg.u.LicenseInfo.licenseNum = HMD_BOARD[slotid]->L_LicenseNum[inst_id][licensetype];	
	else
		msg.u.LicenseInfo.licenseNum = HMD_BOARD[slotid]->R_LicenseNum[inst_id][licensetype];	
		
	char buf[4096];
	memset(buf,0,4096);
	int len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	if (0 > sendto(fd,buf,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	return 0;
}


DBusMessage * hmd_dbus_interface_config_license_assign(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusError err;
	int ret = HMD_DBUS_SUCCESS;
	int type = 0;
	int num = 0;
	int real_type = 0;;
	unsigned int slot_id = 0;
	unsigned int inst_id = 0;
	int num1 = 0;
	int islocaled = 0;
	int LicenseNum = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_UINT32,&num,
								DBUS_TYPE_UINT32,&islocaled,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_UINT32,&inst_id,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(isDistributed){
		if(isMaster){
			if(type <= LicenseCount && type > 0){
				real_type = type - 1;
				if(slot_id < MAX_SLOT_NUM){
					if(HMD_BOARD[slot_id] != NULL){
						if(islocaled){
							LicenseNum = HMD_BOARD[slot_id]->L_LicenseNum[inst_id][real_type];
						}else{
							LicenseNum = HMD_BOARD[slot_id]->R_LicenseNum[inst_id][real_type];
						}
						if(LicenseNum < num){
							num1 = num - LicenseNum;
							if(LICENSE_MGMT[real_type].free_num >= num1){
								LICENSE_MGMT[real_type].free_num = LICENSE_MGMT[real_type].free_num - num1;
								if(islocaled){
									LICENSE_MGMT[real_type].l_assigned_num[slot_id][inst_id] = LICENSE_MGMT[real_type].l_assigned_num[slot_id][inst_id] + num1;
									HMD_BOARD[slot_id]->L_LicenseNum[inst_id][real_type] = num;
								}else{
									LICENSE_MGMT[real_type].r_assigned_num[slot_id][inst_id] = LICENSE_MGMT[real_type].r_assigned_num[slot_id][inst_id] + num1;
									HMD_BOARD[slot_id]->R_LicenseNum[inst_id][real_type] = num;
								}
								if(slot_id == HOST_SLOT_NO){
									struct HmdMsgQ qmsg;
									char command[128];
									memset(command, 0, 128);
									sprintf(command,"sudo echo %d > /var/run/wcpss/wtplicense%d-%d-%d",num,islocaled, inst_id,type);
									system(command);
									memset(&qmsg,0,sizeof(struct HmdMsgQ));
									qmsg.mqinfo.op = HMD_LICENSE_UPDATE;
									qmsg.mqinfo.D_SlotID = slot_id;
									qmsg.mqinfo.S_SlotID = HOST_SLOT_NO;
									qmsg.mqinfo.InstID = inst_id;
									qmsg.mqinfo.local = islocaled;
									qmsg.mqinfo.u.LicenseInfo.licenseType = real_type;
									qmsg.mqinfo.u.LicenseInfo.licenseNum = num;	
									if(islocaled == 0){
										if(HOST_BOARD->Hmd_Inst[inst_id] != NULL){
											qmsg.mqid = inst_id;
											if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
												perror("msgsnd");
										}
									}
									else{
										if(HOST_BOARD->Hmd_Local_Inst[inst_id] != NULL){
											qmsg.mqid = MAX_INSTANCE + inst_id;
											if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
												perror("msgsnd");
										}
									}
								}else
									notice_hmd_client_license_change(slot_id, real_type, inst_id, islocaled);
							}else{
								ret = HMD_DBUS_LICENSE_NUM_NOT_ENOUGH;
							}
						}
						else if(LicenseNum > num){
							num1 = LicenseNum - num;							
							LICENSE_MGMT[real_type].free_num = LICENSE_MGMT[real_type].free_num + num1;
							if(islocaled){
								LICENSE_MGMT[real_type].l_assigned_num[slot_id][inst_id] = LICENSE_MGMT[real_type].l_assigned_num[slot_id][inst_id] - num1;
								HMD_BOARD[slot_id]->L_LicenseNum[inst_id][real_type] = num;
							}else{
								LICENSE_MGMT[real_type].r_assigned_num[slot_id][inst_id] = LICENSE_MGMT[real_type].r_assigned_num[slot_id][inst_id] - num1;
								HMD_BOARD[slot_id]->R_LicenseNum[inst_id][real_type] = num;
							}
							if(slot_id == HOST_SLOT_NO){
								struct HmdMsgQ qmsg;
								char command[128];
								memset(command, 0, 128);
								sprintf(command,"sudo echo %d > /var/run/wcpss/wtplicense%d-%d-%d",num,islocaled, inst_id,type);
								system(command);
								memset(&qmsg,0,sizeof(struct HmdMsgQ));
								qmsg.mqinfo.op = HMD_LICENSE_UPDATE;
								qmsg.mqinfo.D_SlotID = slot_id;
								qmsg.mqinfo.S_SlotID = HOST_SLOT_NO;
								qmsg.mqinfo.local = islocaled;
								qmsg.mqinfo.u.LicenseInfo.licenseType = real_type;
								qmsg.mqinfo.u.LicenseInfo.licenseNum = num;	
								if(islocaled == 0){
									if(HOST_BOARD->Hmd_Inst[inst_id] != NULL){
										qmsg.mqid = inst_id;
										if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
											perror("msgsnd");
									}
								}else{
									if(HOST_BOARD->Hmd_Local_Inst[inst_id] != NULL){
										qmsg.mqid = MAX_INSTANCE + inst_id;
										if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
											perror("msgsnd");
									}
								}
							}else
								notice_hmd_client_license_change(slot_id, real_type, inst_id, islocaled);
						}else{
							ret = HMD_DBUS_SUCCESS;
						}
						if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slot_id)){
							syn_hansi_info_to_backup(slot_id, inst_id, MASTER_BACKUP_SLOT_NO,islocaled,HMD_HANSI_INFO_SYN_LICENSE,real_type);
						}
					}else{
						ret = HMD_DBUS_SLOT_ID_NOT_EXIST;
					}
				}else{
					ret = HMD_DBUS_SLOT_ID_NOT_EXIST; 
				}
			}else{
				ret = HMD_DBUS_LICENSE_TYPE_NOT_EXIST;
			}
		}else{
			ret = HMD_DBUS_COMMAND_NOT_SUPPORT;
		}
	}else{
		ret = HMD_DBUS_COMMAND_NOT_SUPPORT;
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
	
}

DBusMessage * hmd_dbus_license_assign_show(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;	
	DBusMessageIter	 iter_sub_array;	
	DBusMessageIter iter_sub_struct;
	DBusMessageIter	 iter_sub_sub_array;	
	DBusMessageIter iter_sub_sub_struct;
	DBusError err;
	int licenseC = LicenseCount;
	struct Hmd_Board_Info *BOARD[MAX_SLOT_NUM];
	int board_count = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	dbus_error_init(&err);	
	for(i = 0; i < MAX_SLOT_NUM; i++){
		if(HMD_BOARD[i] != NULL){
			BOARD[board_count] = HMD_BOARD[i];
			board_count++;
		}
	}
	if(!isMaster){
		licenseC = 0;
	}
	printf("111111\n");
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&licenseC);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING	//total
									DBUS_TYPE_UINT32_AS_STRING	//free
									DBUS_TYPE_UINT32_AS_STRING	//slot count
										DBUS_TYPE_ARRAY_AS_STRING
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING	//slot
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	//assign_num
													DBUS_TYPE_UINT32_AS_STRING	//assign_num
												DBUS_STRUCT_END_CHAR_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);		
		for(i = 0; i < licenseC; i++)
		{	
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(LICENSE_MGMT[i].total_num));
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(LICENSE_MGMT[i].free_num));
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(board_count));		
			
    		dbus_message_iter_open_container (&iter_struct,
    											   DBUS_TYPE_ARRAY,
    											  	 	DBUS_STRUCT_BEGIN_CHAR_AS_STRING
    													    DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_ARRAY_AS_STRING
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING	//assign_num
																DBUS_TYPE_UINT32_AS_STRING	//assign_num
															DBUS_STRUCT_END_CHAR_AS_STRING
    												DBUS_STRUCT_END_CHAR_AS_STRING,
    											   &iter_sub_array);
			
    		for (j=0;j<board_count;j++){
    			dbus_message_iter_open_container (&iter_sub_array,DBUS_TYPE_STRUCT,NULL,&iter_sub_struct);

    			dbus_message_iter_append_basic(&iter_sub_struct, DBUS_TYPE_UINT32,&(BOARD[j]->slot_no));
				
				dbus_message_iter_open_container (&iter_sub_struct,
													   DBUS_TYPE_ARRAY,
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																	DBUS_TYPE_UINT32_AS_STRING	//assign_num
																	DBUS_TYPE_UINT32_AS_STRING	//assign_num
															DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_sub_sub_array);
				for (k=0;k<MAX_INSTANCE;k++){
					dbus_message_iter_open_container (&iter_sub_sub_array,DBUS_TYPE_STRUCT,NULL,&iter_sub_sub_struct);					
    				dbus_message_iter_append_basic(&iter_sub_sub_struct, DBUS_TYPE_UINT32,&(BOARD[j]->L_LicenseNum[k][i]));
    				dbus_message_iter_append_basic(&iter_sub_sub_struct, DBUS_TYPE_UINT32,&(BOARD[j]->R_LicenseNum[k][i]));
					dbus_message_iter_close_container(&iter_sub_sub_array, &iter_sub_sub_struct);

				}
				dbus_message_iter_close_container (&iter_sub_struct, &iter_sub_sub_array);
    			dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}			
    		dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	return reply;	
}

DBusMessage * hmd_dbus_interface_delete_local_hansi(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned char ID;
	DBusError err;
	int ret = HMD_DBUS_SUCCESS;
	unsigned int slot_id = 0;
	int local_id = 1;	
	unsigned int slot_no = 0;
	char command[128] = {0};
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&ID,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL)){
		if(HMD_L_HANSI[ID] == NULL){
			ret = HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;
		}else{			
			if(HMD_BOARD[slot_id]){
				if(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] != NULL){					
					if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slot_id)){
						syn_hansi_info_to_backup(slot_id, ID, MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_DEL,0);
					}
					sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
								1,slot_id,ID, ".pid");
					system(command);
					if(HMD_L_HANSI[ID]->slot_no == slot_id){
						HMD_L_HANSI[ID]->slot_no = -1;
						slot_no = HMD_L_HANSI[ID]->slot_no1;
					}else if(HMD_L_HANSI[ID]->slot_no1 == slot_id){
						HMD_L_HANSI[ID]->slot_no1 = -1;
						slot_no = HMD_L_HANSI[ID]->slot_no;						
					}				
					if(slot_id != HOST_SLOT_NO){
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] = NULL;
						HmdNoticeToClient(slot_id,ID,local_id,HMD_DELETE);
					}else{				
						HmdSetAssambleHansiMsg(slot_id,ID,local_id,HMD_DELETE);
					}
					if((HMD_BOARD[slot_id] != NULL)&&(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] != NULL)){
						free(HMD_BOARD[slot_id]->Hmd_Local_Inst[ID]);
						HMD_BOARD[slot_id]->Hmd_Local_Inst[ID] = NULL;
					}

					if(HMD_L_HANSI[ID]->slot_num == 2){
						HMD_L_HANSI[ID]->slot_num = 1;
						if((HMD_BOARD[slot_no] != NULL)&&(HMD_BOARD[slot_no]->Hmd_Local_Inst[ID] != NULL)){
							HMD_BOARD[slot_no]->Hmd_Local_Inst[ID]->slot_no1 = -1;
							HMD_BOARD[slot_no]->Hmd_Local_Inst[ID]->slot_num = 1;
						}
					}else if(HMD_L_HANSI[ID]->slot_num == 1){
						HMD_L_HANSI[ID]->slot_num = 0;
						free(HMD_L_HANSI[ID]);
						HMD_L_HANSI[ID] = NULL;
					}
				}else{
					ret = HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;					
				}
			}
		}
	}else{
		ret = HMD_DBUS_ID_NO_EXIST;		
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	sleep(1);//wait for wcpps start,when ac restart add configuration
	return reply;
	
}

DBusMessage * hmd_dbus_service_tftp_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char policy[6] = {0};
    char buff[128] = {0};
	int sysState=0;
	int sysErrorCode = 0;
	dbus_error_init(&err);
	int ret = WID_DBUS_SUCCESS;
    unsigned int ap_auto_update_service_tftp = 0;
 
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&ap_auto_update_service_tftp,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
  
	if(ap_auto_update_service_tftp == 1){
		
        memcpy(policy,"start",5);
   	    ret = WID_DBUS_SUCCESS;
	}
	else if(ap_auto_update_service_tftp == 0){

        memcpy(policy,"stop",4);
	    ret = WID_DBUS_SUCCESS;
		
	}
	else{
		memcpy(policy,"stop",4);
		ret = WID_DBUS_ERROR;
	}
    hmd_syslog_info("service tftp %s\n",policy);

	sprintf(buff, "sudo /etc/init.d/tftpd-hpa %s &", policy);

    sysState = system(buff);
    sysErrorCode = WEXITSTATUS(sysState);
	if(0==sysErrorCode)
	{
        if(ap_auto_update_service_tftp == 1){
          service_tftp_state = 1;
		}
		else if(ap_auto_update_service_tftp == 0){
          service_tftp_state  = 0;
		}
		else{
          service_tftp_state  = 0;
		}
	}
   else
	{
		
		hmd_syslog_info("System cmd error,error code %d\n",sysErrorCode);
		
	}

	if(sysState != 0 )
    {
		
        hmd_syslog_info("System sysState error,error code %d\n",sysState);
		
	}
   
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}
DBusMessage * hmd_dbus_service_ftp_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

    DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char policy[6] = {0};
    char buff[128] = {0};
	int sysState=0;
	int sysErrorCode = 0;
	dbus_error_init(&err);
	int ret = WID_DBUS_SUCCESS;
    unsigned int ftp_state  = 0;
 
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&ftp_state,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
  
	if(ftp_state == 1){
		
        memcpy(policy,"start",5);
   	    ret = WID_DBUS_SUCCESS;
	}
	else if(ftp_state == 0){

        memcpy(policy,"stop",4);
	    ret = WID_DBUS_SUCCESS;
		
	}
	else{
		memcpy(policy,"stop",4);
		ret = WID_DBUS_ERROR;
	}
    hmd_syslog_info("service ftp %s\n",policy);

	sprintf(buff, "sudo /etc/init.d/pure-ftpd %s &", policy);

    sysState = system(buff);
    sysErrorCode = WEXITSTATUS(sysState);
	if(0==sysErrorCode)
	{
        if(ftp_state == 1){
          service_ftp_state  = 1;
		}
		else if(ftp_state == 0){
          service_ftp_state  = 0;
		}
		else{
          service_ftp_state  = 0;
		}
	}
   else
	{
		
		hmd_syslog_info("System cmd error,error code %d\n",sysErrorCode);
		
	}

	if(sysState != 0 )
    {
		
        hmd_syslog_info("System sysState error,error code %d\n",sysState);
		
	}
   
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}


DBusMessage * hmd_dbus_show_service_tftp_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
    DBusMessage * reply;
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&service_tftp_state);

	return reply;

}

DBusMessage * hmd_dbus_show_service_ftp_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
    DBusMessage * reply;
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&service_ftp_state);

	return reply;

}

DBusMessage * hmd_dbus_show_ap_img_file(DBusConnection *conn, DBusMessage *msg, void *user_data){
    DBusMessage * reply;
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);	
	char *buf = (char *)malloc(1024);
	char command[128] = "cd /mnt/wtp/ \n ls -1t *.img *.bin *.tar *.IMG *.BIN *.TAR> /mnt/apimg.txt";
	memset(buf, 0, 256);
	reply = dbus_message_new_method_return(msg);
	system(command);
	read_ac_file("/mnt/apimg.txt", buf,1024);
	dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_STRING,&buf);
	free(buf);
	buf = NULL;
	return reply;

}

DBusMessage * hmd_dbus_show_hmd_bak_forever_info(DBusConnection *conn, DBusMessage *msg, void *user_data){
    DBusMessage * reply;
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);	
	int ret = 0;
	int slot_id = 0,instid = 0,local = 0;
	int sw_state = 0,WidReadyTimes = 0,AsdReadyTimes = 0;
	time_t WidLastReadyTime = 0;
	time_t AsdLastReadyTime = 0;
	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_UINT32,&slot_id,
							DBUS_TYPE_UINT32,&instid,
							DBUS_TYPE_UINT32,&local,
							DBUS_TYPE_INVALID))) 
	{
	    hmd_syslog_err("set hansi profile:Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("set hansi profile %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL))
	{		
		if(HMD_BOARD[slot_id])
		{		
			if(slot_id != HOST_SLOT_NO)
			{
				ret = HMD_DBUS_INVALID_SLOT_NUM; 	
			}
			else
			{				
				if((instid > 0)&&(instid < MAX_INSTANCE)){
					if(0 == local){
						if(HOST_BOARD->Hmd_Inst[instid])
						{		
							sw_state = HOST_BOARD->Hmd_Inst[instid]->HmdBakForever;
							WidReadyTimes = HOST_BOARD->Hmd_Inst[instid]->WidReadyTimes;
							AsdReadyTimes = HOST_BOARD->Hmd_Inst[instid]->AsdReadyTimes;
							WidLastReadyTime = HOST_BOARD->Hmd_Inst[instid]->WidLastReadyTime;
							AsdLastReadyTime = HOST_BOARD->Hmd_Inst[instid]->AsdLastReadyTime;
							hmd_syslog_info("%s,%d,remote hansi HmdBakForever sw %s. \n",__func__,__LINE__,(sw_state == 1)?"enable":"disable");
						}
						else
						{
							ret = HMD_DBUS_HANSI_ID_NOT_EXIST; 	
						}
					}else{
						if(HOST_BOARD->Hmd_Local_Inst[instid])
						{		
							sw_state = HOST_BOARD->Hmd_Local_Inst[instid]->HmdBakForever;
							WidReadyTimes = HOST_BOARD->Hmd_Local_Inst[instid]->WidReadyTimes;
							AsdReadyTimes = HOST_BOARD->Hmd_Local_Inst[instid]->AsdReadyTimes;
							WidLastReadyTime = HOST_BOARD->Hmd_Local_Inst[instid]->WidLastReadyTime;
							AsdLastReadyTime = HOST_BOARD->Hmd_Local_Inst[instid]->AsdLastReadyTime;
							hmd_syslog_info("%s,%d,localhansi HmdBakForever sw %s. \n",__func__,__LINE__,(sw_state == 1)?"enable":"disable");
						}
						else
						{
							ret = HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST; 	
						}
					}
				}
			}
		}
	}
	else
	{
		ret = HMD_DBUS_ID_NO_EXIST; 	
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
								&ret);

	if(ret == 0){
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&sw_state);
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&WidReadyTimes);
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&AsdReadyTimes);
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&WidLastReadyTime);
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&AsdLastReadyTime);
	}




	return reply;
}

/*fengwenchao copy from 1318 for AXSSZFI-839*/
DBusMessage * hmd_dbus_set_wid_clear_binding_interface_flag(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    	DBusMessage * reply = NULL;
	DBusError err;
	dbus_error_init(&err);	
	//hmd_syslog_info(" accessinto %s \n",__func__);
	char *ifname = NULL;
	unsigned int slot_id = 0;
	unsigned int ret =0;
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_STRING,&ifname,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	//hmd_syslog_info(" %s  slot_id = %d, ifname= %s  HOST_SLOT_NO = %d\n",__func__,slot_id,ifname,HOST_SLOT_NO);
	if((slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL))
	{		
		if(HMD_BOARD[slot_id])
		{		
			if(slot_id != HOST_SLOT_NO)
			{
				HmdNoticeToClient_ForClearIfname(slot_id,ifname,HMD_CLEAR_APPLY_IFNAME_FLAG);
			}
			else
			{				
				Set_Interface_binding_Info(ifname,0);
			}
		}
	}
	else
	{
		ret = HMD_DBUS_ID_NO_EXIST; 	
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

DBusMessage * hmd_dbus_set_vrrp_bakup_forever(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    	DBusMessage * reply = NULL;
	DBusError err;
	dbus_error_init(&err);	
	hmd_syslog_info(" accessinto %s \n",__func__);
	unsigned int instid = 0;
	unsigned int ret =0;
	unsigned int sw_state = 0,h_type = 0,slot_id=0;
	struct HmdMsg hmdmsg;
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&slot_id,
									DBUS_TYPE_UINT32,&instid,
									DBUS_TYPE_UINT32,&h_type,
									DBUS_TYPE_UINT32,&sw_state,
									DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	hmdmsg.local = h_type;
	hmdmsg.InstID = instid;
	hmdmsg.D_SlotID = slot_id;
	hmdmsg.u.HANSI.hmdforevesw = sw_state;
	hmdmsg.u.HANSI.slotid = slot_id;
	hmdmsg.u.HANSI.vrrid = instid;
	if((slot_id < MAX_SLOT_NUM)&&(HMD_BOARD[slot_id] != NULL))
	{		
		if(HMD_BOARD[slot_id])
		{		
			if(slot_id != HOST_SLOT_NO)
			{
				HmdNoticeToClient_ForBakForeverConfig(&hmdmsg,HMD_BAKUP_FOREVER_CONFIG);
			}
			else
			{				
				Set_hmd_bakup_foreve_config(&hmdmsg);
			}
		}
	}
	else
	{
		ret = HMD_DBUS_ID_NO_EXIST; 	
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	hmd_syslog_info("%s,%d,set HmdBakForever sw %s,ret:%d,h_type:%d. \n",__func__,__LINE__,(sw_state == 1)?"enable":"disable",ret,h_type);
	return reply;
}

/*fengwenchao copy end*/
/*fengwenchao add 20130412 for hmd timer config save*/
DBusMessage *hmd_dbus_set_hmd_timer_config_save_state(DBusConnection *conn, DBusMessage *msg,void *user_data)
{
    	DBusMessage * reply = NULL;
	DBusError err;
	dbus_error_init(&err);
	hmd_syslog_info(" accessinto %s \n",__func__);
	int state = 0;
	unsigned int ret =0;
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&state,
									DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((isMaster)&&(isActive))
	{
		if(HANSI_TIMER_CONFIG_SAVE != state)
		{
			if(state == 1)
			{
				HMDTimerRequest(HANSI_TIMER,&(HOST_BOARD->HMDTimer_ConfigSave), HMD_TIMER_CONFIG_SAVE, 0, 0);
			}
			else
			{
				HmdTimerCancel(&(HOST_BOARD->HMDTimer_ConfigSave),1);
			}		
			HANSI_TIMER_CONFIG_SAVE = state;
		}
	}
	else
		ret = HMD_DBUS_IS_NOT_MASTER;

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;

}
DBusMessage *hmd_dbus_set_hmd_timer_config_save_timer(DBusConnection *conn, DBusMessage *msg,void *user_data)
{
    	DBusMessage * reply = NULL;
	DBusError err;
	dbus_error_init(&err);
	hmd_syslog_info(" accessinto %s \n",__func__);
	int timer = 0;
	unsigned int ret =0;

	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&timer,
									DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((isMaster)&&(isActive))
	{
		if(HANSI_TIMER_CONFIG_SAVE == 1)
		{
			ret = HMD_DBUS_TIMER_CONFIG_SAVE_STATE_ENABLE;		
		}
		else
			HANSI_TIMER = timer;
	}
	else
		ret = HMD_DBUS_IS_NOT_MASTER;

	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;

}

/*fengwenchao add end*/
DBusMessage *hmd_dbus_set_fast_forwarding_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned int able;
	int ret = 0; 

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&able,
								DBUS_TYPE_INVALID)))
	{
		hmd_syslog_info("%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			hmd_syslog_info("%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	if (able) 
	{
		if (getuid()) 
			system("sudo insmod /lib/modules/2.6.16.26-Cavium-Octeon/misc/ip-fast-forwarding.ko");
		else
			system("insmod /lib/modules/2.6.16.26-Cavium-Octeon/misc/ip-fast-forwarding.ko");
	} 
	else
	{
		if (getuid())
			system("sudo rmmod ip_fast_forwarding");
		else
			system("rmmod ip_fast_forwarding");
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}

DBusMessage *hmd_dbus_set_tunnel_qos_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	int ret = 0; 
	int module = 0;
	int policy = 0;
	int rval = 0, stat = 0, stat1 = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&module,
								DBUS_TYPE_UINT32,&policy,
								DBUS_TYPE_INVALID)))
	{
		hmd_syslog_info("%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			hmd_syslog_info("%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	if(module == 0){
		rval = system("test -d /sys/module/wifi_ethernet");
		stat = WEXITSTATUS(rval);

		if (stat){
			ret = 0;
		}else if (policy){
			system("sudo echo 1 > /sys/module/wifi_ethernet/parameters/wifi_QoS_open");
		}
		else{
			system("sudo echo 0 > /sys/module/wifi_ethernet/parameters/wifi_QoS_open");
		}

	}else if(module == 1){

		rval = system("test -d /sys/module/ip_fast_forwarding");
		stat1 = WEXITSTATUS(rval);

		if (stat1){
			ret = 0;
		}else if (policy){
			system("sudo echo 1 > /sys/module/ip_fast_forwarding/parameters/qos_enable");
		}
		else{
			system("sudo echo 0 > /sys/module/ip_fast_forwarding/parameters/qos_enable");
		}

	}
	else if (module == 2)
	{
		rval = system("test -d /sys/module/wifi_ethernet");
		stat1 = WEXITSTATUS(rval);
		if (stat1)
		{
			ret = 0;
		}
		else if (policy)
		{
			system("sudo echo 1 > /sys/module/wifi_ethernet/parameters/wifi_ipv6_dr_sw");
		}
		else
		{
			system("sudo echo 0 > /sys/module/wifi_ethernet/parameters/wifi_ipv6_dr_sw");
		}
	}
	else
	{
		hmd_syslog_err("%s %d parameter error", __func__, __LINE__);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}

DBusMessage *hmd_dbus_set_tunnel_mm(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	int ret = 0; 
	int wifi_mm = 0;
	char buf[128];
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wifi_mm,
								DBUS_TYPE_INVALID)))
	{
		hmd_syslog_info("%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			hmd_syslog_info("%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	memset(buf,0, 128);
	sprintf(buf,"sudo echo %d > /sys/module/wifi_ethernet/parameters/wifi_mm",wifi_mm);
	system(buf);
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}



DBusMessage *hmd_dbus_set_ipfrag_ignoredf_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned int able;
	int ret = 0; 
	char buff[128] = {0};

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&able,
								DBUS_TYPE_INVALID)))
	{
		hmd_syslog_info("%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			hmd_syslog_info("%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	sprintf(buff, "sudo sysctl -w net.ipv4.ipfrag_ignoredf=%d", able);
	system(buff);
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}
DBusMessage *hmd_dbus_set_ipfrag_inform_nhmtu_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned int able;
	int ret = 0; 
	char buff[128] = {0};

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&able,
								DBUS_TYPE_INVALID)))
	{
		hmd_syslog_info("%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			hmd_syslog_info("%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	sprintf(buff, "sudo sysctl -w net.ipv4.ipfrag_inform_nhmtu_instead=%d", able);
	system(buff);
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}
DBusMessage *hmd_dbus_set_ipfrag_ingress_pmtu_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned int able;
	int ret = 0; 
	char buff[128] = {0};

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&able,
								DBUS_TYPE_INVALID)))
	{
		hmd_syslog_info("%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			hmd_syslog_info("%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	sprintf(buff, "sudo sysctl -w net.ipv4.ipfrag_ingress_pmtu=%d", able);
	system(buff);
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}
DBusMessage * hmd_dbus_femto_service_switch(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = HMD_DBUS_SUCCESS;
	unsigned int service_type = 0;
	unsigned int service_switch = 0;
	unsigned int insID = 0;
	int islocal = 1;
	int slot_id;
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&service_type,
								DBUS_TYPE_UINT32,&service_switch,
						             DBUS_TYPE_UINT32,&insID,
						             DBUS_TYPE_UINT32,&islocal,
						             DBUS_TYPE_UINT32,&slot_id,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(slot_id<MAX_SLOT_NUM && HMD_BOARD[slot_id] != NULL)
	{
		ret = femto_service_switch_check(service_type, slot_id, insID, islocal, service_switch);
		if(ret == HMD_DBUS_SUCCESS)
			ret = femto_service_switch(service_type, slot_id, insID, islocal, service_switch);
	}
	else
		ret = HMD_DBUS_SLOT_ID_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);		
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}
DBusMessage * hmd_dbus_femto_service_state_check(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = HMD_DBUS_SUCCESS;
	unsigned int service_type;
	unsigned int slot_id;
	unsigned int insid;
	int islocal = 0;
	
	if (!(dbus_message_get_args ( msg, &err,
				                          DBUS_TYPE_UINT32,&service_type,
				                          DBUS_TYPE_UINT32,&slot_id,
				                          DBUS_TYPE_UINT32,&insid,
				                          DBUS_TYPE_UINT32,&islocal,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(slot_id<MAX_SLOT_NUM && HMD_BOARD[slot_id] != NULL)
	{
		ret = femto_service_state_check(service_type, slot_id, insid, islocal);
	}
	else
		ret = HMD_DBUS_SLOT_ID_NOT_EXIST;
			
	reply = dbus_message_new_method_return(msg);		
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}

static DBusHandlerResult hmd_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data){

	DBusMessage		*reply = NULL;

	if(message == NULL)
		return DBUS_HANDLER_RESULT_HANDLED;
	hmd_syslog_info("message path %s\n",dbus_message_get_path(message));
	hmd_syslog_info("message interface %s\n",dbus_message_get_interface(message));
	hmd_syslog_info("message member %s\n",dbus_message_get_member(message));
	hmd_syslog_info("message destination %s\n",dbus_message_get_destination(message));	
	hmd_syslog_info("message type %d\n",dbus_message_get_type(message));
	if	(strcmp(dbus_message_get_path(message),HMD_DBUS_OBJPATH) == 0)	{		
		if (dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LOCAL_HANSI)) {
			reply = hmd_dbus_interface_config_local_hansi(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_REMOTE_HANSI)){
			reply = hmd_dbus_interface_config_remote_hansi(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_STATE)){
			reply = hmd_dbus_interface_set_hansi_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_UPLINK_IF)){
			reply = hmd_dbus_interface_set_hansi_uplinkif(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_DOWNLINK_IF)){
			reply = hmd_dbus_interface_set_hansi_downlinkif(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_SLOTID_IMG)){/*fengwenchao add 20120228 for AXSSZFI-680*/
			reply = hmd_dbus_interface_delete_slotid_img(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_GATEWAY_IF)){
			reply = hmd_dbus_interface_set_hansi_gateway(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_LOCAL_HANSI_LINK_IF)){
			reply = hmd_dbus_interface_delete_link_iface(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_LOCAL_HANSI_DISABLE_ENABLE)){
			reply = hmd_dbus_interface_set_hansi_disable_enable(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SHOW_HMD_INFO)){    /*fengwenchao add 20110616*/
			reply = hmd_dbus_interface_show_hmd_info(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SHOW_RUNNING)){
			reply = hmd_dbus_show_running(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_HANSI_SHOW_RUNNING)){
			reply = hmd_dbus_hansi_show_running(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_HAD_NOTIFY_HMD_STATE)){
			reply = hmd_ha_set_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_WID_NOTIFY_HMD_UPDATE_STATE)){
			reply = hmd_dbus_get_wid_update_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_ASD_NOTIFY_HMD_UPDATE_STATE)){
			reply = hmd_dbus_get_asd_update_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_HAD_GET_ALIVE_SLOT_NO)){
			reply = hmd_dbus_ha_get_alive_slots(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_REMOTE_HANSI)){
			reply = hmd_dbus_interface_delete_remote_hansi(connection,message,user_data);
		}else if (dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN)) {
			reply = hmd_dbus_interface_config_license_assign(connection,message,user_data);
		}else if (dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN_SHOW)) {
			reply = hmd_dbus_license_assign_show(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_LICENSE_SHOW_RUNNING)){
			reply = hmd_dbus_license_show_running(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_LOCAL_HANSI)){
			reply = hmd_dbus_interface_delete_local_hansi(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_HANSI_CHECK_STATE)){
			reply = hmd_dbus_interface_set_hansi_check_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SERVICE_TFTP_SWITCH)){
			reply = hmd_dbus_service_tftp_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_SERVICE_TFTP_SWITCH)){
            reply = hmd_dbus_show_service_tftp_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SERVICE_FTP_SWITCH)){
			reply = hmd_dbus_service_ftp_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_SERVICE_FTP_SWITCH)){
            reply = hmd_dbus_show_service_ftp_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_FAST_FORWARDING)){
            reply = hmd_dbus_set_fast_forwarding_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_TUNNEL_QOS_MAP)){
            reply = hmd_dbus_set_tunnel_qos_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_TUNNEL_MM)){
            reply = hmd_dbus_set_tunnel_mm(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_IPFRAG_IGNOREDF)){
            reply = hmd_dbus_set_ipfrag_ignoredf_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_IPFRAG_INFORM_NHMTU_INSTEAD)){
            reply = hmd_dbus_set_ipfrag_inform_nhmtu_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_IPFRAG_INGRESS_PMTU)){
            reply = hmd_dbus_set_ipfrag_ingress_pmtu_state(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_AP_IMG_FILES)){
            reply = hmd_dbus_show_ap_img_file(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SHOW_HMD_BAK_FOREVER_INFO)){
            reply = hmd_dbus_show_hmd_bak_forever_info(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_SWITCH)){
		reply = hmd_dbus_femto_service_switch(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_CHECK)){
		reply = hmd_dbus_femto_service_state_check(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_WID_CLEAR_BINDING_INTERFACE_FLAG)){ /*fengwenchao copy from 1318 for AXSSZFI-839*/
		reply = hmd_dbus_set_wid_clear_binding_interface_flag(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_SET_VRRP_BAKUP_FOREVER)){ /*fengwenchao copy from 1318 for AXSSZFI-839*/
		reply = hmd_dbus_set_vrrp_bakup_forever(connection,message,user_data);
		}else if (dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_HMD_TIMER_CONFIG_SAVE_STATE)){/*fengwenchao add 20130412 for hmd timer config save*/
		reply = hmd_dbus_set_hmd_timer_config_save_state(connection,message,user_data);
		}else if (dbus_message_is_method_call(message,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_HMD_TIMER_CONFIG_SAVE_TIMER)){/*fengwenchao add 20130412 for hmd timer config save*/
		reply = hmd_dbus_set_hmd_timer_config_save_timer(connection,message,user_data);
		}
	}
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); 
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED ;
}



DBusHandlerResult
hmd_dbus_filter_function (DBusConnection * connection,
					   DBusMessage * message, void *user_data)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		hmd_syslog_err("Got disconnected from the system message bus; "
				"retrying to reconnect every 3000 ms\n");
		//dbus_connection_close(hmd_dbus_connection);
		hmd_dbus_connection = NULL;
		HmdThread thread_dbus; 
		if(!(HmdCreateThread(&thread_dbus, hmd_dbus_thread_restart, NULL,0))) {
			hmd_syslog_crit("Error starting Dbus Thread");
			exit(1);
		}

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

	} else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}

int hmd_dbus_reinit(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	hmd_vtable = {NULL, &hmd_dbus_message_handler, NULL, NULL, NULL, NULL};	

//	printf("npd dbus init\n");

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	hmd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	if (hmd_dbus_connection == NULL) {
		hmd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	// Use npd to handle subsection of NPD_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (hmd_dbus_connection, HMD_DBUS_OBJPATH, &hmd_vtable, NULL)) {
		hmd_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	
	i = dbus_bus_request_name (hmd_dbus_connection, HMD_DBUS_BUSNAME,
			       0, &dbus_error);
		
	hmd_syslog_debug_debug(HMD_DBUS,"dbus_bus_request_name:%d",i);
	
	if (dbus_error_is_set (&dbus_error)) {
		hmd_syslog_debug_debug(HMD_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (hmd_dbus_connection, hmd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (hmd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
	return TRUE;
  
}

int hmd_dbus_init(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	
	DBusObjectPathVTable	hmd_vtable = {NULL, &hmd_dbus_message_handler, NULL, NULL, NULL, NULL};	

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	hmd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	hmd_dbus_connection2 = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);

	if (hmd_dbus_connection == NULL) {
		hmd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	if (!dbus_connection_register_fallback (hmd_dbus_connection, HMD_DBUS_OBJPATH, &hmd_vtable, NULL)) {
		hmd_syslog_err("can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	i = dbus_bus_request_name (hmd_dbus_connection, HMD_DBUS_BUSNAME,
			       0, &dbus_error);
	dbus_bus_request_name (hmd_dbus_connection2, "aw.hmd2",
			       0, &dbus_error);

	if (dbus_error_is_set (&dbus_error)) {
		hmd_syslog_debug_debug(HMD_DBUS,"dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (hmd_dbus_connection, hmd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (hmd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
//	printf("init finished\n");
	return TRUE;
  
}

void *HMDDbus()
{
	hmd_pid_write_v2("HMDDbus",HOST_SLOT_NO);	
	if(hmd_dbus_init()
	){
		while (dbus_connection_read_write_dispatch(hmd_dbus_connection,500)) {

		}
	}
	hmd_syslog_err("there is something wrong in dbus handler\n");	

	return 0;
}

void *hmd_dbus_thread_restart()
{
	hmd_pid_write_v2("HMDDbus Reinit",HOST_SLOT_NO);	
	if(hmd_dbus_reinit()
	){
		while (dbus_connection_read_write_dispatch(hmd_dbus_connection,500)) {

		}
	}
	hmd_syslog_err("there is something wrong in dbus handler\n");	
	return 0;
}


