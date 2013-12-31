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


dcli_wireless_init(void)
{
	/****moved from dcli_stp.c****/
	install_node(&hansi_wtp_node,NULL,"HANSI_WTP_NODE");
	install_default(HANSI_WTP_NODE);
	install_node(&local_hansi_wtp_node,NULL,"LOCAL_HANSI_WTP_NODE");
	install_default(LOCAL_HANSI_WTP_NODE);

    /****moved frome dcli_radio.c***/
	install_node(&hansi_radio_node,NULL,"HANSI_RADIO_NODE");
	install_default(HANSI_RADIO_NODE);

	install_node(&local_hansi_radio_node,NULL,"LOCAL_HANSI_RADIO_NODE");
	install_default(LOCAL_HANSI_RADIO_NODE);
			
}





