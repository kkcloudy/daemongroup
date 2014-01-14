#ifdef _D_WCPSS_

#ifndef HAVE_SOCKLEN_T
#define HAVE_SOCKLEN_T
#endif

#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <time.h>     /*xm add*/
#include <sys/time.h> /*xm add*/

#include "command.h"
//#include "dcli_ac.h"
//#include "dcli_acl.h"
#include "../dcli_main.h"
#include "dcli_wlan.h"
#include "vtysh/vtysh.h"

#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "wid_ac.h"
#include "asd_sta.h"
#include "sysdef/npd_sysdef.h"
#include "bsd/bsdpub.h"




/** 
  * @brief  generate return string with return value
  * @param  iReturnValue   
  * @param  pReturnStr   
  * @return  
  * @author  zhangshu
  * @date  2012/06/04
  */
char * dcli_bsd_get_return_string(int iReturnValue, char pReturnStr[BSD_COMMAND_BUF_LEN])
{
    if(pReturnStr == NULL)
        return "Copy file error.\n";
    memset(pReturnStr, 0, BSD_COMMAND_BUF_LEN);
    switch(iReturnValue) {
        case BSD_SUCCESS:
            memcpy(pReturnStr, "Copy file successful.\n", strlen("Copy file successful.\n"));
            break;
        case BSD_GET_SLOT_INFORMATION_ERROR:
            memcpy(pReturnStr, "Get slot information error.\n", strlen("Get slot information error\n"));
            break;
        case BSD_INIT_SOCKET_ERROR:
        case BSD_ESTABLISH_CONNECTION_FAILED:
            memcpy(pReturnStr, "Failed to connect with peer.\n", strlen("Failed to connect with peer.\n"));
            break;
        case BSD_ILLEGAL_SOURCE_FILE_PATH:
            memcpy(pReturnStr, "Illegal source file path.\n", strlen("Illegal source file path.\n"));
            break;
        case BSD_ILLEGAL_DESTINATION_FILE_PATH:
            memcpy(pReturnStr, "Illegal target file path.\n", strlen("Illegal target file path.\n"));
            break;
        case BSD_GET_FILE_SIZE_ERROR:
            memcpy(pReturnStr, "Failed to get file size.\n", strlen("Failed to get file size.\n"));
            break;
        case BSD_NOT_ENOUGH_MEMERY:
            memcpy(pReturnStr, "Not enough free memery on peer.\n", strlen("Not enough free memery on peer.\n"));
            break;
        case BSD_SEND_MESSAGE_ERROR:
            memcpy(pReturnStr, "Failed to send message to peer.\n", strlen("Failed to send message to peer.\n"));
            break;
        case BSD_RECEIVE_MESSAGE_ERROR:
            memcpy(pReturnStr, "Failed to receive message from peer.\n", strlen("Failed to receive message from peer.\n"));
            break;
        case BSD_EVENTID_NOT_MATCH:
            memcpy(pReturnStr, "Sending event ID not match.\n", strlen("Sending event ID not match.\n"));
            break;
        case BSD_PEER_SAVE_FILE_ERROR:
            memcpy(pReturnStr, "Failed to save file on target device.\n", strlen("Failed to save file on target device.\n"));
            break;
        case BSD_SERVER_NOT_CATCH:
            memcpy(pReturnStr, "Can not catch peer.\n", strlen("Can not catch peer.\n"));
            break;
        case BSD_MALLOC_MEMERY_ERROR:
            memcpy(pReturnStr, "Malloc memery error.\n", strlen("Malloc memery error.\n"));
            break;
        case BSD_MD5_ERROR:
            memcpy(pReturnStr, "MD5 check error.\n", strlen("MD5 check error.\n"));
            break;
		case BSD_MD5_ERROR_THIRD:
            memcpy(pReturnStr, "MD5 check error third,you can try again.\n", strlen("MD5 check error third,you can try again.\n"));
            break;
        case BSD_WAIT_THREAD_CONDITION_TIMEOUT:
            memcpy(pReturnStr, "Wait for peer response over time.\n", strlen("Wait for peer response over time.\n"));
            break;
        case BSD_ADD_TO_MESSAGE_QUEUE_ERROR:
            memcpy(pReturnStr, "Failed to add message into msgq.\n", strlen("Failed to add message into msgq.\n"));
            break;
        case BSD_UNKNOWN_ERROR:
        default:
            memcpy(pReturnStr, "Copy file failed with unknow error.\n", strlen("Copy file failed with unknow error.\n"));
            break;
    }
    return (char*)pReturnStr;
}



char* dcli_hansi_wtp_show_running_config_start(int localid ,int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int res1 = 0, res2 = 0;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_START);
	/*query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_START);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("show wtp config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
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
//		dcli_config_write(tmp,localid,slot_id,index,0,0);
		dbus_message_unref(reply);		
		return tmp;	
	} 
	else 
	{
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return NULL;
	}
	
	return NULL;	
}




char* dcli_hansi_wtp_show_running_config_end(int localid, int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_END);
	/*query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_END);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("show wtp config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp; 
	} 
	else 
	{
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return NULL;	
	}

}

void ReInitDbusConnection(DBusConnection **dcli_dbus_connection,int slot_id,int distributFag){
	//DBusConnection *dcli_dbus_connection = NULL;
	if((distributFag)&&(dbus_connection_dcli[slot_id])&&(NULL != dbus_connection_dcli[slot_id]->dcli_dbus_connection))
	{
		*dcli_dbus_connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}else{
		//*dcli_dbus_connection = dcli_dbus_connection_local;
		vty_out(vty,"the slot %d has not connected\n", slot_id);
	}
}

//qiuchen
char *dcli_auth_type_to_text(unsigned int type)
{
	switch (type)
	{
		case asd_auth_type_unkown:			/* 1 */
			return "UNKOWN AUTH TYPE";
		case asd_auth_type_authfree:		/* 2 */
			return "AUTH FREE";
		case asd_auth_type_weppsk:			/* 3*/
			return "WEP/PSK AUTH";
		case asd_auth_type_autoauth:		/* 4*/
			return "SIM/PEAP AUTH";
		default:
			return "UNKOWN AUTH TYPE";
	}
}



/*******wangchao moved from dcli_security.c*******/
char *dcli_hansi_security_show_running_config(int localid, int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_SECURITY_OBJPATH , \
							ASD_DBUS_SECURITY_INTERFACE ,		\
							ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("hansi show security config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("security: %s raised: %s",err.name,err.message);
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
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



int dcli_wlan_list_show_running_config(struct vty*vty) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	//index = vty->index;
	//localid = vty->local;
	//slot_id = vty->slotindex;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID))	{
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"WLAN LIST");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return 1;	
	}
}

int dcli_wtp_list_show_running_config(struct vty*vty) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	//index = vty->index;
	//localid = vty->local;
	//slot_id = vty->slotindex;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp list config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"WTP LIST");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
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
		return 1;	
	}

}

int dcli_bss_list_show_running_config(struct vty*vty) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	//index = vty->index;
	//localid = vty->local;
	//slot_id = vty->slotindex;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show bss list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"BSS LIST");
		vtysh_add_show_string(_tmpstr);					
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
	} else 	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 	{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);

			dbus_message_unref(reply);
			return 1;	
		}
	}

}


char* dcli_hansi_wlan_list_show_running_config(int localid, int slot_id,int index) 
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

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID))	{
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));	
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return NULL;	
	}
}

char* dcli_hansi_wtp_list_show_running_config(int localid, int slot_id,int index) 
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

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp list config failed get reply.\n");
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
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

char *dcli_hansi_bss_list_show_running_config(int localid, int slot_id, int index) 
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

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show bss list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));	
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} else 	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 	{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);

			dbus_message_unref(reply);
			return NULL;	
		}
	}

}

#endif



/***from dcli_wlan.c*****/
char* dcli_hansi_wlan_show_running_config_start(int localid ,int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int res = 1;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START);
/*	query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan config failed get reply.\n");
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
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
	
	return NULL;	
}

int dcli_hansi_wlan_show_running_config_end(int localid, int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END);
/*	query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan config failed get reply.\n");
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
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
  return NULL;
}



/*****from dcli_ebr.c*****/

char* dcli_hansi_ebr_show_running_config_start(int localid, int slot_id ,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(
							BUSNAME,		\
							OBJPATH , \
							INTERFACE ,		\
							WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_START);

	dbus_error_init(&err);

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
	//	dcli_config_write(showStr,localid,slot_id,index,1,0);
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

char *dcli_hansi_ebr_show_running_config_end(int localid,int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(
							BUSNAME,		\
							OBJPATH , \
							INTERFACE , 	\
							WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_END);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);//fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show hansi_ebr end config failed get reply.\n");
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
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


/***moved from dcli_ac_ip_list.c**/

char* dcli_hansi_ac_ip_list_show_running_config(int localid, int slot_id, int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);//fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show hansi_ac_ip_list config failed get reply.\n");
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
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
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

	return NULL;	
}

/******wangchao moved frome dcli_wtp.c******/
struct cmd_node wtp_node =
{
	WTP_NODE,
	"%s(config-wtp %d)# ",
	1
};
struct cmd_node hansi_wtp_node =
{
	HANSI_WTP_NODE,
	"%s(hansi-wtp %d-%d-%d)# ",
	1
};
struct cmd_node local_hansi_wtp_node =
{
	LOCAL_HANSI_WTP_NODE,
	"%s(local_hansi-wtp %d-%d-%d)# ",
	1
};

struct cmd_node wtp_node1 =
{
	WTP_NODE1,
	" ",
	1
};


/**********moved from dcli_radio.c*****/
struct cmd_node radio_node =
{
	RADIO_NODE,
	"%s(config-radio %d-%d)# "
};
struct cmd_node hansi_radio_node =
{
	HANSI_RADIO_NODE,
	"%s(hansi-radio %d-%d-%d-%d)# ",
	1
};
struct cmd_node local_hansi_radio_node =
{
	LOCAL_HANSI_RADIO_NODE,
	"%s(local-hansi-radio %d-%d-%d-%d)# ",
	1
};

/*****moved from dcli_wlan.c**/
struct cmd_node hansi_wlan_node =
{
	HANSI_WLAN_NODE,
	"%s(hansi-wlan %d-%d-%d)# ",
	1
};
/*fengwenchao add 20110507*/
struct cmd_node local_hansi_wlan_node =
{
	LOCAL_HANSI_WLAN_NODE,
	"%s(local-hansi-wlan %d-%d-%d)# ",
	1
};

/***moved from dcli_ebr.c***/
struct cmd_node hansi_ebr_node =
{
	HANSI_EBR_NODE,
	"%s(hansi-ebr %d-%d-%d)# ",
	1
};
struct cmd_node local_hansi_ebr_node =
{
	LOCAL_HANSI_EBR_NODE,
	"%s(local-hansi-ebr %d-%d-%d)# ",
	1
};

/***moved from dcli_ap_group.c***/
struct cmd_node hansi_ap_group_node =
{
	HANSI_AP_GROUP_NODE,
	"%s(hansi-ap-group %d-%d-%d)# ",
	1
};

struct cmd_node local_hansi_ap_group_node =
{
	LOCAL_HANSI_AP_GROUP_NODE,
	"%s(local-hansi-ap-group %d-%d-%d)# ",
	1
};

struct cmd_node hansi_ap_group_wtp_node =
{
	HANSI_AP_GROUP_WTP_NODE,
	"%s(hansi-ap-group-wtp %d-%d-%d)# ",
	1
};

struct cmd_node local_hansi_ap_group_wtp_node =
{
	LOCAL_HANSI_AP_GROUP_WTP_NODE,
	"%s(local-hansi-ap-group-wtp %d-%d-%d)# ",
	1
};

struct cmd_node hansi_ap_group_radio_node =
{
	HANSI_AP_GROUP_RADIO_NODE,
	"%s(hansi-ap-group-radio %d-%d-%d-%d)# ",
	1
};

struct cmd_node local_hansi_ap_group_radio_node =
{
	LOCAL_HANSI_AP_GROUP_RADIO_NODE,
	"%s(local-hansi-ap-group-radio %d-%d-%d)# ",
	1
};


dcli_wireless_init(void)
{
	/****moved from dcli_stp.c****/
	install_node(&hansi_wtp_node,NULL,"HANSI_WTP_NODE");
	install_default(HANSI_WTP_NODE);
	install_node(&local_hansi_wtp_node,NULL,"LOCAL_HANSI_WTP_NODE");
	install_default(LOCAL_HANSI_WTP_NODE);

    /****moved from dcli_radio.c***/
	install_node(&hansi_radio_node,NULL,"HANSI_RADIO_NODE");
	install_default(HANSI_RADIO_NODE);

	install_node(&local_hansi_radio_node,NULL,"LOCAL_HANSI_RADIO_NODE");
	install_default(LOCAL_HANSI_RADIO_NODE);

	/***moved from dcli_ebr.c***/
	install_node(&hansi_ebr_node,NULL,"HANSI_EBR_NODE");		
	install_default(HANSI_EBR_NODE);
	
	install_node(&local_hansi_ebr_node,NULL,"LOCAL_HANSI_EBR_NODE");		
	install_default(LOCAL_HANSI_EBR_NODE);	

	/***moved from dcli_wlan.c**/
	install_node(&hansi_wlan_node,NULL,"HANSI_WLAN_NODE");
	install_default(HANSI_WLAN_NODE);

    install_node(&local_hansi_wlan_node,NULL,"LOCAL_HANSI_WLAN_NODE");
    install_default(LOCAL_HANSI_WLAN_NODE);

	/**moved from dcli_wlan.c**/
	install_node(&hansi_ap_group_node,NULL,"HANSI_AP_GROUP_NODE");
	install_default(HANSI_AP_GROUP_NODE);

	install_node(&local_hansi_ap_group_node,NULL,"LOCAL_HANSI_AP_GROUP_NODE");
	install_default(LOCAL_HANSI_AP_GROUP_NODE);

	install_node(&hansi_ap_group_wtp_node,NULL,"HANSI_AP_GROUP_WTP_NODE");
	install_default(HANSI_AP_GROUP_WTP_NODE);	

	install_node(&local_hansi_ap_group_wtp_node,NULL,"LOCAL_HANSI_AP_GROUP_WTP_NODE");
	install_default(LOCAL_HANSI_AP_GROUP_WTP_NODE);

	install_node(&hansi_ap_group_radio_node,NULL,"HANSI_AP_GROUP_RADIO_NODE");
	install_default(HANSI_AP_GROUP_RADIO_NODE); 

	install_node(&local_hansi_ap_group_radio_node,NULL,"LOCAL_HANSI_AP_GROUP_RADIO_NODE");
	install_default(LOCAL_HANSI_AP_GROUP_RADIO_NODE);
			
}





