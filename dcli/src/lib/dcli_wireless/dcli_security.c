#ifdef _D_WCPSS_
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"

#include "../dcli_main.h"
#include "dcli_wlan.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dcli_security.h"   /*xm 08/09/01*/
#include "asd_security.h"
#include "asd_sta.h"
#include "bsd/bsdpub.h"
#include "math.h"//qiuchen
/*#include <linux/if.h>*/

struct cmd_node security_node =
{
	SECURITY_NODE,
	"%s(config-security %d)# "
};

struct cmd_node hansi_security_node =
{
	HANSI_SECURITY_NODE,
	"%s(hansi-security %d-%d-%d)# "
};

struct cmd_node local_hansi_security_node =
{
	LOCAL_HANSI_SECURITY_NODE,
	"%s(local-hansi-security %d-%d-%d)# "
};


/*xm0701*/
DEFUN(set_wapi_rekey_para_cmd_func,
	  set_wapi_rekey_para_cmd,
	  "set wapi (unicast|multicast) rekey (time|packet) parameter  <0-400000000>",
	  "wapi rekey parameter\n"
	  "wapi time_based or packet_based parameter\n"
	  "parameter value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned int value=0;
	unsigned char uorm,torp;

	//security_id = (int)vty->index;

	str2lower(&argv[0]);
	if (!strcmp(argv[0],"unicast")||(tolower(argv[0][0]) == 'u'))
		uorm=0;
	else if (!strcmp(argv[0],"multicast")||(tolower(argv[0][0]) == 'm'))
		uorm=1;
	else	{
		vty_out(vty,"<error> unknown command format\n");
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	if (!strcmp(argv[1],"time")||(tolower(argv[1][0]) == 't'))
		torp=0;
	else if (!strcmp(argv[1],"packet")||(tolower(argv[1][0]) == 'p'))
		torp=1;
	else	{
		vty_out(vty,"<error> unknown command format\n");
		return CMD_SUCCESS;
	}
	
	value= atoi(argv[2]);

	if(value>400000000){
		vty_out(vty,"<error> input value should be 0 to 400000000.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_REKEY_PARA);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_REKEY_PARA);*/ 
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&uorm,
						 DBUS_TYPE_BYTE,&torp,
						 DBUS_TYPE_UINT32,&value,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security type set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
 	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set wapi rekey parameter under current config.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*	xm0701*/
DEFUN(set_wapi_ucast_rekey_method_cmd_func,
	  set_wapi_ucast_rekey_method_cmd,
	  "set wapi (unicast|multicast) rekey method (time_based|packet_based|both_based|disable)",
	  "wapi unicast or multicast key update method.\n"
	  "4 update method :time_based	packet_based	both_based	disable\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char method=1;	/*	0--disable	1--time_based		2--packet_based	3--both_based*/
	unsigned char uorm=0;	/*	0--unicast	1--multicast*/

	//security_id = (int)vty->index;

	str2lower(&argv[0]);
	if (!strcmp(argv[0],"unicast")||(tolower(argv[0][0]) == 'u'))
		uorm=0;
	else if (!strcmp(argv[0],"multicast")||(tolower(argv[0][0]) == 'm'))
		uorm=1;
	else	{
		vty_out(vty,"<error> unknown command format\n");
		return CMD_SUCCESS;
	}
	
	str2lower(&argv[1]);
	if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd'))
		method=0;
	else if (!strcmp(argv[1],"time_based")||(tolower(argv[1][0]) == 't'))
		method=1;
	else if (!strcmp(argv[1],"packet_based")||(tolower(argv[1][0]) == 'p'))
		method=2;
	else if (!strcmp(argv[1],"both_based")||(tolower(argv[1][0]) == 'b'))
		method=3;
	else	{
		vty_out(vty,"<error> unknown command format\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_UCAST_REKEY_METHOD);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_UCAST_REKEY_METHOD);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&uorm,
						 DBUS_TYPE_BYTE,&method,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security type set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
 	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set wapi unicast rekey method under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*xm 08/09/02*/
DEFUN(config_secondary_acct_cmd_func,
	  config_secondary_acct_cmd,
	  "secondary radius acct IP PORT SHARED_SECRET",
	  "set the second time.\n"
	  "Configuring service.\n"
	  "Make security account server\n"
	  "IP addr of acct server\n"
	  "PORT of IP of acct server\n"
	  "share secret of acct server\n"
	 )
{	
	int ret;
	unsigned char security_id;
	unsigned int type, port;
	char *ip, *shared_secret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
//	security_id = (int)vty->index;

	/*get parameter for argv[ ]*/

	ret = Check_IP_Format((char*)argv[0]);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	ip = (char *)malloc(strlen(argv[0])+1);     /*don't forget free ip;*/
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));

	ret = parse_int_ID((char*)argv[1], &port);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown port format\n");
		free(ip);
		ip = NULL;
		return CMD_SUCCESS;
	}	
	if(port > 65535){
		vty_out(vty,"<error> unknown port id\n");
		free(ip);
		ip = NULL;
		return CMD_SUCCESS;
	}	

	shared_secret = (char *)malloc(strlen(argv[2])+1);
	memset(shared_secret, 0, strlen(argv[2])+1);
	memcpy(shared_secret, argv[2], strlen(argv[2]));

	/*I want to know whether we get the right parameter/////////////////////////////
	//printf("dcli: get ip %s\n",ip);
	//printf("dcli: get port %d\n",port);
	//printf("dcli: get shared secret %s\n",shared_secret);
	///////////////////////////////////////////////////////////////////////*/

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_ACCT);

	/*send message to asd*/
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_ACCT);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&port,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&shared_secret,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	/*after send message ,free() some thing*/
	free(ip);
	ip = NULL;
	free(shared_secret);
	shared_secret = NULL;
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	/*get reply*/

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security %d successfully set %s server.\n",security_id,argv[0]);
		else if(ret == ASD_SECURITY_TYPE_WITHOUT_8021X)			
			vty_out(vty,"<error> security type which you chose does not support 802.1X.\n");
		else if((ret == ASD_SECURITY_ACCT_BE_USED)||(ret == ASD_SECURITY_AUTH_BE_USED))
			vty_out(vty,"<error> changing radius info is not permited\n");		
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_AUTH_NOT_EXIST)			
			vty_out(vty,"<error> please use radius acct ip port shared_secret first.\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)//qiuchen
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}




DEFUN(set_eap_reauth_period_cmd_func,
	  set_eap_reauth_period_cmd,
	  "set eap reauth period <0-32767>",
	  "eap\n"
	  "auth the second time\n"
	  "eap reauth period\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int period=3600;

//	security_id = (int)vty->index;
	period= atoi(argv[0]);

	if(period<0||period>32767)
		{
			vty_out(vty,"<error> input period value should be 0 to 32767.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD); */
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&period,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security type set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
 	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set eap reauth period under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*ht add,090205*/
DEFUN(set_acct_interim_interval_cmd_func,
	  set_acct_interim_interval_cmd,
	  "set acct interim interval <0-32767>",
	  "radius\n"
	  "acct interim interval\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int interval=0;

//	security_id = (int)vty->index;
	interval= atoi(argv[0]);

	if(interval<0||interval>32767)
		{
			vty_out(vty,"<error> input time value should be 0 to 32767.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCT_INTERIM_INTERVAL);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCT_INTERIM_INTERVAL);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&interval,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set acct interim interval %d successfully!\n",interval);
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set acct interim interval under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


DEFUN(config_secondary_auth_cmd_func,
	  config_secondary_auth_cmd,
	  "secondary radius auth IP PORT SHARED_SECRET",
	  "set the second time.\n"
	  "Configuring service.\n"
	  "Make security authentication server\n"
	  "IP addr of auth server\n"
	  "PORT of IP of auth server\n"
	  "share secret of auth server\n"
	 )
{	
	int ret;
	unsigned char security_id;
	unsigned int type, port;
	char *ip, *shared_secret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//security_id = (int)vty->index;

	/*get parameter for argv[ ]*/

	ret = Check_IP_Format((char*)argv[0]);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	ip = (char *)malloc(strlen(argv[0])+1);     /*don't forget free ip;*/
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));

	ret = parse_int_ID((char*)argv[1], &port);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown port format\n");
		free(ip);
		ip = NULL;
		return CMD_SUCCESS;
	}	
	if(port > 65535){
		vty_out(vty,"<error> unknown port d\n");
		free(ip);
		ip = NULL;
		return CMD_SUCCESS;
	}	

	shared_secret = (char *)malloc(strlen(argv[2])+1);
	memset(shared_secret, 0, strlen(argv[2])+1);
	memcpy(shared_secret, argv[2], strlen(argv[2]));

	/*I want to know whether we get the right parameter/////////////////////////////
	//printf("dcli: get ip %s\n",ip);
	//printf("dcli: get port %d\n",port);
	//printf("dcli: get shared secret %s\n",shared_secret);
	///////////////////////////////////////////////////////////////////////*/

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AUTH);

	/*send message to asd*/
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AUTH);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&port,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&shared_secret,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	/*after send message ,free() some thing*/
	free(ip);
	ip = NULL;
	free(shared_secret);
	shared_secret = NULL;
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	/*get reply*/

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security %d successfully set %s server.\n",security_id,argv[0]);
		else if(ret == ASD_SECURITY_TYPE_WITHOUT_8021X)			
			vty_out(vty,"<error> security type which you chose does not support 802.1X.\n");
		else if((ret == ASD_SECURITY_ACCT_BE_USED)||(ret == ASD_SECURITY_AUTH_BE_USED))
			vty_out(vty,"<error> changing radius info is not permited\n");		
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some Wlans,please disable these Wlans first\n");
		else if(ret == ASD_SECURITY_AUTH_NOT_EXIST)			
			vty_out(vty,"<error> please use radius auth ip port shared_secret first.\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)//qiuchen
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
/*xm 08/09/02
/*****************************************************************************************/

/*add by sz20080827*/
DEFUN(	config_vlan_list_enable_cmd_func,
	  		config_vlan_list_enable_cmd,
		    "config vlan VLANLIST dot1x (enable|disable)",
		    "config vlan enable\n"
			"maybe include list of vlans\n"
	)
{	/*printf("start\n");*/
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusError err;

	int i=0;
	int list[4095];
	int n,num,stat=0;
	int num_from_npd=0;
	
	VLAN_ENABLE vlano[MAX_VLAN_NUM];
	memset(vlano,0,sizeof(VLAN_ENABLE)*MAX_VLAN_NUM);
	
	VLAN_PORT_ENABLE rcv_npd[MAX_VLAN_NUM];
	memset(rcv_npd,0,sizeof(VLAN_PORT_ENABLE)*MAX_VLAN_NUM);
	
	/*////////////////////////////////////////////////////////////////////////
	//printf("============================== cmd ====================================\n");*/


	dbus_error_init(&err);
	/*printf("start2\n");*/
	ret = parse_vlan_list((char*)argv[0],&n,list);
	/*parse if there is repeat vlanid*/
	num = RemoveListRepId(list,n);
	/*printf("%d\n",num);*/
	for(i=0;i<num;i++){
		vlano[i].vlanid=list[i];
		/*printf("%d vlanid: %d\n",i,vlano[i].vlanid);///////////////////////*/
		}
	if (-1 == ret) {
    	vty_out(vty,"<error> input parameter is illegal\n");
		return CMD_FAILURE;
	}
	str2lower(&argv[1]);
	if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e'))
		stat=1;
	else
		stat=0;
	
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WTP_OBJPATH,\
									WID_DBUS_WTP_INTERFACE,WID_DBUS_WTP_METHOD_SET_VLAN_LIST_ENABLE);					
	
	dbus_message_iter_init_append (query, &iter);

		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										& num);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(vlano[i].vlanid));
		
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &stat);

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	/*printf(" dcli: send to npd.\n");/////////////////////////////////*/
	
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

	/*receive message from npd*/
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&num_from_npd);
	
	if(num_from_npd> 0 ){
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		

		for (i = 0; i < num_from_npd; i++) {
			DBusMessageIter iter_struct;
					
			dbus_message_iter_recurse(&iter_array,&iter_struct);
				
			dbus_message_iter_get_basic(&iter_struct,&(rcv_npd[i].vlanid));
				
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(rcv_npd[i].port));
	
			dbus_message_iter_next(&iter_array);

			rcv_npd[i].stat=stat;

			/*printf("vlanid: %d with port: %d with stat %d.\n",vlani[i].vlanid,vlani[i].port,vlani[i].stat);
			//printf("dcli rceive from npd: %d. vlanid-portindex-stat  %d-%d-%d\n",i+1,rcv_npd[i].vlanid,rcv_npd[i].port,rcv_npd[i].stat);
			/////////////////////////////////////////////////////////////////////////*/
			
		}
	
	}
		


	/*send this to the asd(take wid as example first)*/

	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
			ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_VLAN_LIST_APPEND_ENABLE);
														
				
	dbus_message_iter_init_append (query, &iter);
		
				
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&num_from_npd);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);


	for(i = 0; i <num_from_npd; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(rcv_npd[i].vlanid));
		
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &rcv_npd[i].port);
		
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &rcv_npd[i].stat);

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			
			
	/*printf("dcli: send to npd %d items.\n ",num_from_npd);////////////////////////////// */

		
	dbus_message_unref(query);
			
	if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		}

	/*printf("cmd success\n");*/
			
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);
	
	if(ret == 0)
		vty_out(vty,"set successfully\n");
		
	else
		vty_out(vty,"<error>  %d\n",ret);
	return CMD_SUCCESS;


}



DEFUN(config_vlan_list_security_cmd_func,
	config_vlan_list_security_cmd,
	"config vlan VLANLIST dot1x security SECURITYID",
	"config vlan security\n"
	"maybe include list of vlans\n"
)
{	
	int ret,ret1,ret2,i,j;
	int n,num,num1,sendnum;
	int list[4095];
	 
	VLAN_PORT_SECURITY vlani[MAX_VLAN_NUM],vlano[MAX_VLAN_NUM];
	memset(vlani,0,sizeof(VLAN_PORT_SECURITY)*MAX_VLAN_NUM);
	memset(vlano,0,sizeof(VLAN_PORT_SECURITY)*MAX_VLAN_NUM);
	

	unsigned char security_id;
	DBusMessage *query,*query1, *reply;
	DBusError err,err1;
	DBusMessageIter  iter,iiter,iiiter;
	DBusMessageIter	 iter_array,iiter_array,iiiter_array;
	
	/*get vlan ID*/
	ret2 = parse_vlan_list((char*)argv[0],&n,list);

	/*parse if there is repeat vlanid*/
	num = RemoveListRepId(list,n);

	for(i=0;i<num;i++){
		vlano[i].vlanid=list[i];
		/*printf("%d vlanid: %d\n",i,vlano[i].vlanid);///////////////////////////////////////////*/
		}
	if (-1 == ret2) {
    	vty_out(vty,"<error> input parameter illegal!\n");
		return CMD_FAILURE;
	}


	/*parse security id*/
	ret1 = parse_char_ID((char*)argv[1], &security_id);	
	if(ret1 != WID_DBUS_SUCCESS){
            if(ret1== WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}
	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	for(i=0;i<num;i++){
		vlano[i].securityid=security_id;
		/*printf("%d securityid: %d\n",i,vlano[i].securityid);//////////////////////////////////*/
	}
	
		dbus_error_init(&err);
	
		query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WTP_OBJPATH,\
									WID_DBUS_WTP_INTERFACE,WID_DBUS_WTP_METHOD_SET_SECURITY);
			
		dbus_message_iter_init_append (query, &iter);
	
			
		dbus_message_iter_append_basic (	&iter,
											DBUS_TYPE_UINT32,
											&num);
			
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING					
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);
	
		for(i = 0; i < num; i++){			
			DBusMessageIter iter_struct;
				
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
	
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(vlano[i].vlanid));
			
	
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		/*printf("send out vlanid: %d\n",vlano[i].vlanid);///////////////////////////////////////////////*/
		}
					
		dbus_message_iter_close_container (&iter, &iter_array);
	
	
		
	
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
		dbus_message_iter_init(reply,&iiter);

		dbus_message_iter_get_basic(&iiter,&ret);
		
		if (ret==0){
		dbus_message_iter_next(&iiter);	
		dbus_message_iter_get_basic(&iiter,&num1);
		/*printf("\nthe total number is %2d\n ",num1);//////////////////////////////////////////////////*/
		dbus_message_iter_next(&iiter);
		
		dbus_message_iter_recurse(&iiter,&iiter_array);
		
		for (i = 0; i <num1; i++) {
			
			DBusMessageIter iiter_struct;
			
		
			dbus_message_iter_recurse(&iiter_array,&iiter_struct);
	
			dbus_message_iter_get_basic(&iiter_struct,&(vlani[i].vlanid));
		
			dbus_message_iter_next(&iiter_struct);
		
			dbus_message_iter_get_basic(&iiter_struct,&(vlani[i].port));
				
			dbus_message_iter_next(&iiter_array);
		

			vlani[i].securityid = security_id;
			
			/*//////////////////////////////////////////////////////////////////////////////
			//printf("vlanid: %d with port: %d with security %d.\n",vlani[i].vlanid,vlani[i].port,vlani[i].securityid);
			/////////////////////////////////////////////////////////////////////////////////*/
		}
				sendnum = i;
		}
	 	dbus_message_unref(reply);
		

	
		/*send this to the asd(take wid as example first)*/

			query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
										ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_VLAN_APPEND_SECURITY);
																	
				
			dbus_message_iter_init_append (query, &iiiter);
		
				
			dbus_message_iter_append_basic (	&iiiter,
												DBUS_TYPE_UINT32,
												& sendnum);
		
			
			
			/*printf("send %d with security information.\n ",sendnum);*/
			
				
			dbus_message_iter_open_container (&iiiter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iiiter_array);
		
			for(i = 0; i < sendnum; i++){			
				DBusMessageIter iiiter_struct;
					
				dbus_message_iter_open_container (&iiiter_array,
												DBUS_TYPE_STRUCT,
												NULL,
												&iiiter_struct);
				dbus_message_iter_append_basic
							(&iiiter_struct,
							   DBUS_TYPE_UINT32,
							  &(vlani[i].vlanid));
				dbus_message_iter_append_basic
							(&iiiter_struct,
							  DBUS_TYPE_UINT32,
							  &(vlani[i].port));
				dbus_message_iter_append_basic
							(&iiiter_struct,
							  DBUS_TYPE_BYTE,
							  &(vlani[i].securityid));
		
				dbus_message_iter_close_container (&iiiter_array, &iiiter_struct);
		
			}
						
			dbus_message_iter_close_container (&iiiter, &iiiter_array);
		
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
			
			dbus_message_unref(reply);
			
			if(ret == 0)
				vty_out(vty,"set successfully\n");
				
			else
				vty_out(vty,"<error>  %d\n",ret);
			return CMD_SUCCESS;

		/*printf("cmd success\n");*/
		return CMD_SUCCESS;

}




DEFUN(config_port_vlan_security_cmd_func,
		  config_port_vlan_security_cmd,
		 "config port PORTID vlan VLANID dot1x security SECURITYID",
		 "config port vlan security\n"
		 "portid: slot/port\n"
		 "vlanid: <1-4094>\n"
		 "securityid: <wlannum-1>\n"
)
{	
	int vlanid;/*,slot,port=0;*/
	unsigned char security_id;
	int ret,ret1,ret2;
	
	SLOT_PORT_VLAN_SECURITY vlan1,vlan2;
	
		
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	
	
	
	ret = parse_port((char*)argv[0],&vlan1);

	/*printf("%d/%d\n",vlan1.slot,vlan1.port);

	//printf("%d/%d\n",vlan1.slot,vlan1.port);*/
	if (-1 == ret) {
    	vty_out(vty,"<error> input parameter illegal!\n");
		return -1;
	}
	
	ret1 = parse_int_ID((char*)argv[1], &vlanid);
	
	vlan1.vlanid = vlanid;
	if(ret1 != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return 0;
	}
	if(vlanid<1||vlanid>4094){
		vty_out(vty,"<error> vlan id should be 1 to 4094\n");
		return 0;
	}

	/*parse security id*/

	ret2 = parse_char_ID((char*)argv[2], &security_id);
	vlan1.securityid = security_id;
		/*printf("securityid: %d\n",vlan1.securityid);*/
		
	if(ret2 != WID_DBUS_SUCCESS){
            if(ret2 == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return 0;
	}
	
	/*send slot port vlanid to the acdbus*/
	dbus_error_init(&err);
	/*
		query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WTP_OBJPATH,\
									WID_DBUS_WTP_INTERFACE,WID_DBUS_WTP_METHOD_SET_PORT_VLAN_SECURITY);
			
		dbus_message_iter_init_append (query, &iter);			
				
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);
				
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			
		dbus_message_iter_append_basic(&iter_struct,
					  					DBUS_TYPE_UINT32,
					  					&(vlan1.slot));

		dbus_message_iter_append_basic(&iter_struct,
					  					DBUS_TYPE_UINT32,
					  					&(vlan1.port));

		dbus_message_iter_append_basic(&iter_struct,
					  					DBUS_TYPE_UINT32,
					  					&(vlan1.vlanid));
		

		dbus_message_iter_close_container (&iter_array, &iter_struct);

		///////////////////////////////////////////////////////////////////////////////////////
		printf("\n===========================================CMD====================================\n");
		printf("dcli :send vlanid-slot-port to npd: vlanid: %d slot: %d port: %d\n",vlan1.vlanid,vlan1.slot,vlan1.port);
		////////////////////////////////////////////////////////////////////////////////////////
					
		dbus_message_iter_close_container (&iter, &iter_array);
	
	
		
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	


			 	
					
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		dbus_message_iter_init(reply,&iter);

		dbus_message_iter_get_basic(&iter,&ret);
		
		if (ret==0){
				
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_recurse(&iter,&iter_array);
		
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(vlan2.vlanid));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(vlan2.portindex));
				
			dbus_message_iter_next(&iter_array);

			vlan2.slot = vlan1.slot;
			vlan2.port = vlan1.port;
			vlan2.securityid = security_id;

			/////////////////////////////////////////////////////////////////////////////////////////////
			printf("\nfrom simulated npd: slot: %d port: %d vlanid: %d with portindex: %d with security %d.\n",vlan2.slot,vlan2.port,vlan2.vlanid,vlan2.portindex,vlan2.securityid);
			/////////////////////////////////////////////////////////////////////////////////////////////
		}
	 	dbus_message_unref(reply);
		
		*/
	
		/*send this to the asd(take wid as example first)*/
		/*zhanglei change for debug*/	
			vlan2.slot = vlan1.slot;
			vlan2.port = vlan1.port;
			vlan2.vlanid = vlan1.vlanid;
			vlan2.portindex = vlan1.port;
			vlan2.securityid = security_id;
		/*end*/
		query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
										ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_SECURITY);
					

			dbus_message_iter_init_append (query, &iter);
						
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array);
					
			DBusMessageIter iiter_struct;
				
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iiter_struct);
			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.slot));

			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.port));
			
			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.vlanid));
			
			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.portindex));

			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_BYTE,
						  &(vlan2.securityid));
	
			dbus_message_iter_close_container (&iter_array, &iiter_struct);
					
			dbus_message_iter_close_container (&iter, &iter_array);
		
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
				return 0;
			}
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			
			dbus_message_unref(reply);
			
			if(ret == 0)
				vty_out(vty,"set successfully\n");
				
			else
				vty_out(vty,"<error>  %d\n",ret);
			return CMD_SUCCESS;

}


DEFUN(config_port_vlan_enable_cmd_func,
		 config_port_vlan_enable_cmd,
		 "config port PORTID vlan VLANID dot1x (enable|disable)",
		 "config port vlan enable|disable\n"
		 "portid: slot/port\n"
		 "vlanid: <1-4094>\n"
		 "securityid: <wlannum-1>\n"
	)
{	int vlanid,slot,port=0;
	int stat;
	unsigned char security_id;
	int ret,ret1,ret2;
	SLOT_PORT_VLAN_ENABLE vlan1,vlan2;

		
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;


	ret = _parse_port((char*)argv[0],&vlan1);

	if (-1 == ret) {
    	vty_out(vty,"<error> nput parameter illegal!\n");
		return CMD_FAILURE;
	}

	ret1 = parse_int_ID((char*)argv[1], &vlanid);
	vlan1.vlanid = vlanid;
	if(ret1 != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(vlanid<1||vlanid>4094){
		vty_out(vty,"<error> vlan id should be 1 to 4094\n");
		return CMD_SUCCESS;
	}

	str2lower(&argv[2]);
	if (!strcmp(argv[2],"enable")||(tolower(argv[2][0]) == 'e'))
		stat=1;
	else if (!strcmp(argv[2],"disable")||(tolower(argv[2][0]) == 'd'))
		stat=0;
	else	{
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}

	
	/*send slot port vlanid to the acdbus*/
	dbus_error_init(&err);
	
		query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WTP_OBJPATH,\
											WID_DBUS_WTP_INTERFACE,WID_DBUS_WTP_METHOD_SET_PORT_VLAN_ENABLE);
		



			
		dbus_message_iter_init_append (query, &iter);			
				
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);
				
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			
		dbus_message_iter_append_basic(&iter_struct,
					  					DBUS_TYPE_UINT32,
					  					&(vlan1.slot));

		dbus_message_iter_append_basic(&iter_struct,
					  					DBUS_TYPE_UINT32,
					  					&(vlan1.port));

		dbus_message_iter_append_basic(&iter_struct,
					  					DBUS_TYPE_UINT32,
					  					&(vlan1.vlanid));
		

		dbus_message_iter_close_container (&iter_array, &iter_struct);

		/*/////////////////////////////////////////////////////////////////////////
		//printf("send out vlanid: %d slot: %d port: %d\n",vlan1.vlanid,vlan1.slot,vlan1.port);
		///////////////////////////////////////////////////////////////////////////*/
					
		dbus_message_iter_close_container (&iter, &iter_array);
	
	
		
	
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
		dbus_message_iter_init(reply,&iter);

		dbus_message_iter_get_basic(&iter,&ret2);
		
		if (ret2==0){
				
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_recurse(&iter,&iter_array);
		
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(vlan2.vlanid));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(vlan2.portindex));
				
			dbus_message_iter_next(&iter_array);

			vlan2.slot = vlan1.slot;
			vlan2.port = vlan1.port;;
			vlan2.stat = stat;
			/*//////////////////////////////////////////////////////////////////////////////////////
			//printf("slot: %d port: %d vlanid: %d with portindex: %d with state %d.\n",vlan2.slot,vlan2.port,vlan2.vlanid,vlan2.portindex,vlan2.stat);
			////////////////////////////////////////////////////////////////////////////////////////*/
		}
	 	dbus_message_unref(reply);
		

	
		/*send this to the asd(take wid as example first)*/

			query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
										ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_ENABLE);
				
			dbus_message_iter_init_append (query, &iter);
						
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array);
					
			DBusMessageIter iiter_struct;
				
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iiter_struct);
			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.slot));

			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.port));
			
			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.vlanid));
			
			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.portindex));

			dbus_message_iter_append_basic
						(&iiter_struct,
						   DBUS_TYPE_UINT32,
						  &(vlan2.stat));
	
			dbus_message_iter_close_container (&iter_array, &iiter_struct);
					
			dbus_message_iter_close_container (&iter, &iter_array);
		
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

		/*printf("cmd success\n");*/
		
	 	dbus_message_unref(reply);
		return CMD_SUCCESS;


}

//weichao add 20110921

DEFUN(set_eap_alive_period_cmd_func,
	  set_eap_alive_period_cmd,
	  "set eap alive period <60-32767>",
	  "eap\n"
	  "auth the second time\n"
	  "eap alive period\n"
	  "alive value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int period=3600;

//	security_id = (int)vty->index;
	period= atoi(argv[0]);

	if(period<60||period>32767)
	{
		vty_out(vty,"<error> input period value should be 60 to 32767.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_ALIVE_PERIOD);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD); */
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&period,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security type set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
 	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set eap alive period under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
//weichao add  2011.11.29
DEFUN(set_sta_account_cmd_func,
	  set_sta_account_cmd,
	  "set account start after authorized (enable|disable)",
	  "set account messge time.\n"
	  "default disable"
	 )
{

	int ret = ASD_DBUS_SUCCESS;
	int   type = 0;
	unsigned char security_id = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_AUTHORIZE);
		
		dbus_error_init(&err);
		
	
		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&security_id,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");	
		else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
			vty_out(vty,"<error> Can't set account message start after authorize under current security type.\n");
		else if(ret == ASD_DBUS_SUCCESS)
			{
				if(1==type)
					vty_out(vty,"set account message start after authorize enable successfully!\n");
				
				else
					vty_out(vty,"set account messge start after authorize disable successfully!\n");
			}	
						
			else
				vty_out(vty,"<error!>%d\n",ret);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

//weichao add 
DEFUN(set_dhcp_account_cmd_func,
	  set_dhcp_account_cmd,
	  "set account start after dhcp (enable|disable)",
	  "differentiate the accounting method.\n"
	  "default disable"
	 )
{

	int ret = ASD_DBUS_SUCCESS;
	int  type = 0;
	unsigned char security_id = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_DHCP);
		
		dbus_error_init(&err);
		
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&security_id,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		
		dbus_message_iter_get_basic(&iter,&ret);
		
		if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");	
		else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
				vty_out(vty,"<error> Can't set  account messge start after dchp under current security type.\n");
		else if(ret == ASD_ACCOUNT_AFTER_AUTHORIZE)
				vty_out(vty,"<error> you have set account start after authorize,please disable it first\n");
		else if(ret == ASD_DBUS_SUCCESS)
			{
				if(1==type)
					vty_out(vty,"set account messge start after dhcp enable successfully!\n");
				
				else
					vty_out(vty,"set account messge start after dhcp disable successfully!\n");
			}	
						
		else
			vty_out(vty,"<error!>%d\n",ret);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

DEFUN(set_ap_max_detect_interval_cmd_func,
	  set_ap_max_detect_interval_cmd,
	  "set ap max detect interval <30-86400>",
	  "ap detect if sta is online.\n"
	  "VALUE,default is 300 seconds"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned int interval = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	ret = parse_int_ID((char*)argv[0], &interval);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown format,please input number\n");
		return CMD_SUCCESS;
	}
	if( 0 != interval%15){
		vty_out(vty,"<error> the number is not be multiple of 15\n");
		return CMD_SUCCESS;

	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AP_DETECT_INTERVAL);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&interval,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set ap max detect interval %s successfully\n",argv[0]);
	else if (ret == ASD_SECURITY_NOT_EXIST) 		
		vty_out(vty,"<error> security profile does not exist.\n");			
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#if DCLI_NEW
DEFUN(show_radius_cmd_func,
	  show_radius_cmd,
	  "show radius RADIUSID",
	  CONFIG_STR
	  "Radius profile information\n"
	  "radius id\n"
	 )
{	
	 
	struct dcli_security	*sec = NULL;
	unsigned int 	ret = 0;
	unsigned char 	radius_id = 0;
	int  i;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;

	ret = parse_char_ID((char*)argv[0], &radius_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(radius_id >= WLAN_NUM || radius_id == 0){
		vty_out(vty,"<error> radius id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	sec = show_radius_id(dcli_dbus_connection, index, radius_id, localid, &ret);	
	if ((ret == 0) && (sec != NULL)) {			
			vty_out(vty,"RADIUS ID : %u\n",radius_id);			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"AUTH IP : %s:%d\n",sec->auth.auth_ip,sec->auth.auth_port);			
			vty_out(vty,"AUTH SHARED SECRET: %s\n",sec->auth.auth_shared_secret);
			vty_out(vty,"ACCT IP : %s:%d\n",sec->acct.acct_ip,sec->acct.acct_port);
			vty_out(vty,"ACCT SHARED SECRET : %s\n\n",sec->acct.acct_shared_secret);

			vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",sec->auth.secondary_auth_ip,sec->auth.secondary_auth_port); 
			vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",sec->auth.secondary_auth_shared_secret);
			vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",sec->acct.secondary_acct_ip,sec->acct.secondary_acct_port);
			vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n",sec->acct.secondary_acct_shared_secret);
			vty_out(vty,"==============================================================================\n");
			dcli_free_security(sec);
	}
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> security id does not exist\n");
	else if (ret == ASD_DBUS_ERROR)		
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		
	return CMD_SUCCESS;

}
#else
DEFUN(show_radius_cmd_func,
	  show_radius_cmd,
	  "show radius RADIUSID",
	  CONFIG_STR
	  "Radius profile information\n"
	  "radius id\n"
	 )
{	
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	int ret;
	unsigned char radius_id;
	auth_server auth;
	acct_server acct;

	ret = parse_char_ID((char*)argv[0], &radius_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(radius_id >= WLAN_NUM || radius_id == 0){
		vty_out(vty,"<error> radius id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&radius_id,
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
					
					DBUS_TYPE_UINT32,&(auth.auth_port),
					DBUS_TYPE_STRING,&(auth.auth_ip),
					DBUS_TYPE_STRING,&(auth.auth_shared_secret),
					DBUS_TYPE_UINT32,&(auth.secondary_auth_port),
					DBUS_TYPE_STRING,&(auth.secondary_auth_ip),
					DBUS_TYPE_STRING,&(auth.secondary_auth_shared_secret),	
	
					DBUS_TYPE_UINT32,&(acct.acct_port),
					DBUS_TYPE_STRING,&(acct.acct_ip),
					DBUS_TYPE_STRING,&(acct.acct_shared_secret),					
					DBUS_TYPE_UINT32,&(acct.secondary_acct_port),
					DBUS_TYPE_STRING,&(acct.secondary_acct_ip),
					DBUS_TYPE_STRING,&(acct.secondary_acct_shared_secret),	
					DBUS_TYPE_INVALID)) {
		if(ret == 0){			
			vty_out(vty,"RADIUS ID : %u\n",radius_id);			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"AUTH IP : %s:%d\n",auth.auth_ip,auth.auth_port);			
			vty_out(vty,"AUTH SHARED SECRET: %s\n",auth.auth_shared_secret);
			vty_out(vty,"ACCT IP : %s:%d\n",acct.acct_ip,acct.acct_port);
			vty_out(vty,"ACCT SHARED SECRET : %s\n\n",acct.acct_shared_secret);

			vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",auth.secondary_auth_ip,auth.secondary_auth_port); 
			vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",auth.secondary_auth_shared_secret);
			vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",acct.secondary_acct_ip,acct.secondary_acct_port);
			vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n",acct.secondary_acct_shared_secret);
			vty_out(vty,"==============================================================================\n");

		}else if (ret == ASD_SECURITY_NOT_EXIST) {
			vty_out(vty,"<error> radius %u does not exist\n",radius_id);
		}else
			vty_out(vty,"<error> %d\n",ret);
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;
	
}
#endif

#if DCLI_NEW
DEFUN(show_security_list_cmd_func,
	  show_security_list_cmd,
	  "show security (list|all) [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "Display security profile information\n"
	  "List security profile summary\n"
	  "List security profile summary\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
	 
	struct dcli_security	*sec = NULL;
	struct dcli_security	*tmp = NULL;
	unsigned int 	ret = 0;
	unsigned char 	num = 0;
	char SecurityType[20];
	char EncryptionType[20];
	int i;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
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
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
	
	if(vty->node != VIEW_NODE){
		sec = show_security_list(dcli_dbus_connection, index, &num, localid, &ret);

		if((ret == 0) || (ret == ASD_SECURITY_NOT_EXIST) ){
			tmp = sec;
			vty_out(vty,"Security profile list summary\n");
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"%-10s  %-12s  %-8s  %-15s  %-12s %s\n","SecurityID","SecurityName","RadiusID","HostIP","SecurityType","EncryptionType");
			for (i = 0; i < num; i++) {
				if(tmp == NULL)
					break;

				
				memset(SecurityType, 0, 20);
				memset(EncryptionType, 0, 20);
				CheckSecurityType(SecurityType, tmp->SecurityType);
				CheckEncryptionType(EncryptionType, tmp->EncryptionType);
				vty_out(vty,"%-10d  %-12s  %-8d  %-15s  %-12s %s\n",tmp->SecurityID, tmp->name, tmp->RadiusID, tmp->host_ip,SecurityType,EncryptionType);
				tmp = tmp->next;
			}
			vty_out(vty,"==============================================================================\n");
			dcli_free_security_list(sec);
		}
		else if (ret == ASD_DBUS_ERROR){
			cli_syslog_info("<error> failed get reply.\n");
			return CMD_FAILURE;	
		}
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				sec = show_security_list(dcli_dbus_connection, profile, &num, localid, &ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) || (ret == ASD_SECURITY_NOT_EXIST) ){
					tmp = sec;
					vty_out(vty,"Security profile list summary\n");
					vty_out(vty,"=========================================================================\n");
					vty_out(vty,"%-10s  %-12s  %-8s  %-15s  %-12s %s\n","SecurityID","SecurityName","RadiusID","HostIP","SecurityType","EncryptionType");
					for (i = 0; i < num; i++) {
						if(tmp == NULL)
							break;

						
						memset(SecurityType, 0, 20);
						memset(EncryptionType, 0, 20);
						CheckSecurityType(SecurityType, tmp->SecurityType);
						CheckEncryptionType(EncryptionType, tmp->EncryptionType);
						vty_out(vty,"%-10d  %-12s  %-8d  %-15s  %-12s %s\n",tmp->SecurityID, tmp->name, tmp->RadiusID, tmp->host_ip,SecurityType,EncryptionType);
						tmp = tmp->next;
					}
					dcli_free_security_list(sec);
				}
				else if (ret == ASD_DBUS_ERROR){
					cli_syslog_info("<error> failed get reply.\n");
					return CMD_FAILURE;	
				}
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			sec = show_security_list(dcli_dbus_connection, profile, &num, localid, &ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) || (ret == ASD_SECURITY_NOT_EXIST) ){
				tmp = sec;
				vty_out(vty,"Security profile list summary\n");
				vty_out(vty,"=========================================================================\n");
				vty_out(vty,"%-10s  %-12s  %-8s  %-15s  %-12s %s\n","SecurityID","SecurityName","RadiusID","HostIP","SecurityType","EncryptionType");
				for (i = 0; i < num; i++) {
					if(tmp == NULL)
						break;

					
					memset(SecurityType, 0, 20);
					memset(EncryptionType, 0, 20);
					CheckSecurityType(SecurityType, tmp->SecurityType);
					CheckEncryptionType(EncryptionType, tmp->EncryptionType);
					vty_out(vty,"%-10d  %-12s  %-8d  %-15s  %-12s %s\n",tmp->SecurityID, tmp->name, tmp->RadiusID, tmp->host_ip,SecurityType,EncryptionType);
					tmp = tmp->next;
				}
				dcli_free_security_list(sec);
			}
			else if (ret == ASD_DBUS_ERROR){
				cli_syslog_info("<error> failed get reply.\n");
				return CMD_FAILURE;	
			}
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}
#else
DEFUN(show_security_list_cmd_func,
	  show_security_list_cmd,
	  "show security (list|all)",
	  SHOW_STR
		"Display security profile information\n"
		"List security profile summary\n"
	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	security_profile *Security[WLAN_NUM];	
	int ret,i=0;
	unsigned char num;
	char SecurityType[20];
	char EncryptionType[20];
	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST);*/
	dbus_error_init(&err);
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0 ){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			Security[i] = (security_profile*)malloc(sizeof(security_profile));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(Security[i]->SecurityID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(Security[i]->RadiusID));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(Security[i]->name));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(Security[i]->host_ip));
					
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(Security[i]->securityType));
					
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(Security[i]->encryptionType));
					
			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
	
	
	
    vty_out(vty,"Security profile list summary\n");
    vty_out(vty,"================================================================================\n");
	vty_out(vty,"%-10s  %-12s  %-8s  %-15s  %-12s %s\n","SecurityID","SecurityName","RadiusID","HostIP","SecurityType","EncryptionType");
	if(ret == 0)
		for (i = 0; i < num; i++) {
			memset(SecurityType, 0, 20);
			memset(EncryptionType, 0, 20);
			CheckSecurityType(SecurityType, Security[i]->securityType);
			CheckEncryptionType(EncryptionType, Security[i]->encryptionType);
			vty_out(vty,"%-10d  %-12s  %-8d  %-15s  %-12s %s\n",Security[i]->SecurityID, Security[i]->name,Security[i]->RadiusID,Security[i]->host_ip,SecurityType,EncryptionType);
			free(Security[i]);
			Security[i]=NULL;
	}
	vty_out(vty,"================================================================================\n");
	return CMD_SUCCESS;
	
}
#endif

#if DCLI_NEW
DEFUN(show_security_cmd_func,
	  show_security_cmd,
	  "show security SECURITYID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "Security profile information\n"
	  "security id\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_security	*sec = NULL;
	unsigned int 	ret = 0;
	unsigned char 	security_id = 0;
	char SecurityType[20];
	char RekeyMethod[30];	
	char EncryptionType[20];
	char asc[]="ASCII";
	char hex[]="HEX";
	char unk[]="Unknown";
	int  i;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	//qiuchen
	char enable[] = "enable";
	char disable[] = "disable";
	char auth[] = "auth";
	char acct[] = "acct";
	char both[] = "both";
	char on[] = "on";
	char off[] = "off";
	char master[] = "master";
	char bak[] = "bak";
	ret = parse_char_ID((char*)argv[0], &security_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
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
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		sec = show_security_id(dcli_dbus_connection, index, security_id, localid, &ret);	
		if ((ret == 0) && (sec != NULL)) {
				memset(SecurityType, 0, 20);
				memset(EncryptionType, 0, 20);
				CheckSecurityType(SecurityType, sec->SecurityType);
				CheckEncryptionType(EncryptionType, sec->EncryptionType);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"Name : %s\n",sec->name);
				vty_out(vty,"ID : %d\n",sec->SecurityID);
				vty_out(vty,"IP : %s\n",sec->host_ip);
				vty_out(vty,"SecurityType : %s\n",SecurityType);
				vty_out(vty,"EncryptionType : %s\n",EncryptionType);
				vty_out(vty,"SecurityKEY(%s): %s\n",(sec->keyInputType==0)?unk:((sec->keyInputType==1)?asc:hex),sec->SecurityKey);
				vty_out(vty,"SECURITY INDEX : %d\n\n",sec->security_index);
				vty_out(vty,"Extensible Authentication state: %s\n",(sec->extensible_auth == 1)?"open":"close");
				vty_out(vty,"Radius Server type: %s\n",(sec->wired_radius == 1)?"wired":"wireless");

				vty_out(vty,"Hybrid Authentication state: %s\n",(sec->hybrid_auth == 1)?"open":"close");	//mahz add 2011.2.28
				vty_out(vty,"MAC AUTH: %s\n",(sec->mac_auth == 1)?"enable":"disable");		//weichao add
				vty_out(vty,"WAPI RADIUS AUTH: %s\n",(sec->wapi_radius_auth == 1)?"enable":"disable");		//mahz add 2010.11.24
				vty_out(vty,"WAPI RADIUS AUTH USER PASSWD: %s\n",sec->user_passwd);			//mahz add 2010.12.9
				
				vty_out(vty,"EAP REAUTH PERIOD: %d\n",sec->eap_reauth_period);
				vty_out(vty,"ACCT INTERIM INTERVAL: %d\n",sec->acct_interim_interval);
				vty_out(vty,"1X QUIET PERIOD: %d\n",sec->quiet_period);
				vty_out(vty,"AUTH IP : %s:%d\n",sec->auth.auth_ip,sec->auth.auth_port);			
				vty_out(vty,"AUTH SHARED SECRET: %s\n",sec->auth.auth_shared_secret);
				vty_out(vty,"ACCT IP : %s:%d\n",sec->acct.acct_ip,sec->acct.acct_port);
				vty_out(vty,"ACCT SHARED SECRET : %s\n\n",sec->acct.acct_shared_secret);

				vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",sec->auth.secondary_auth_ip,sec->auth.secondary_auth_port);	
				vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",sec->auth.secondary_auth_shared_secret);
				vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",sec->acct.secondary_acct_ip,sec->acct.secondary_acct_port);
				vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n\n",sec->acct.secondary_acct_shared_secret);
																					
				vty_out(vty,"WAPI AUTH IP : %s:%d\n",sec->wapi_as.as_ip,3810);	
				vty_out(vty,"WAPI MULTI CERT : %s\n",sec->wapi_as.multi_cert==1?"enable":"disable");	
				vty_out(vty,"WAPI CERTIFICATION TPYE : %s\n",(sec->wapi_as.certification_type==WAPI_X509)?"X.509":((sec->wapi_as.certification_type==WAPI_GBW)?"GBW":" "));
				vty_out(vty,"WAPI AS CERTIFICATION PATH : %s\n",sec->wapi_as.certification_path);
				vty_out(vty,"WAPI AE CERTIFICATION PATH : %s\n",sec->wapi_as.ae_cert_path);
				vty_out(vty,"WAPI CA CERTIFICATION PATH : %s\n\n",sec->wapi_as.ca_cert_path);

				if(sec->SecurityType==WAPI_AUTH||sec->SecurityType==WAPI_PSK){
					CheckRekeyMethod(RekeyMethod,sec->wapi_ucast_rekey_method);
					vty_out(vty,"WAPI UNICAST REKEY METHOD : %s\n",RekeyMethod);
					if((sec->wapi_ucast_rekey_method==1)||(sec->wapi_ucast_rekey_method==3)){
						vty_out(vty,"WAPI UNICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_ucast_rekey_para_t);
					}
					if((sec->wapi_ucast_rekey_method==2)||(sec->wapi_ucast_rekey_method==3)){
						vty_out(vty,"WAPI UNICAST REKEY PACKET BASED PARA : %5u\n\n",sec->wapi_ucast_rekey_para_p);	
					}
					CheckRekeyMethod(RekeyMethod,sec->wapi_mcast_rekey_method);
					vty_out(vty,"WAPI MULTICAST REKEY METHOD : %s\n",RekeyMethod);
					if((sec->wapi_mcast_rekey_method==1)||(sec->wapi_mcast_rekey_method==3)){
						vty_out(vty,"WAPI MULTICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_t);
					}
					if((sec->wapi_mcast_rekey_method==2)||(sec->wapi_mcast_rekey_method==3)){
						vty_out(vty,"WAPI MULTICAST REKEY PACKET BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_p);
					}
				}
				vty_out(vty,"EAP EXTENTION FUNCTION ACTIVATED: %s\n",(sec->eap_sm_run_activated==1)?"enable":"disable");//Qc
				vty_out(vty,"------------------------------------------------------------------------\n");
				//qiuchen add it for master_bak radius server configuration 2012.12.11
				if((sec->SecurityType == IEEE8021X)||(sec->SecurityType == WPA_E)||(sec->SecurityType == WPA2_E)||(sec->SecurityType == MD5)){
					vty_out(vty,"Radius server binding is %s\n",(sec->radius_server_binding_type == 1)?enable:disable);
					vty_out(vty,"Radius heart test type is %s\n",(sec->radius_heart_test_type == 0)?auth:((sec->radius_heart_test_type == 1)?acct:both));
					vty_out(vty,"Radius response fail percent is %lf\n",sec->radius_res_fail_percent);
					vty_out(vty,"Radius response success percent is %lf\n",sec->radius_res_suc_percent);
					vty_out(vty,"Radius access test interval is %d\n",sec->radius_access_test_interval);
					vty_out(vty,"Radius server change test timer is %d(s)\n",sec->radius_server_change_test_timer);
					vty_out(vty,"Radius server reuse test timer is %d(s)\n",sec->radius_server_reuse_test_timer);
					vty_out(vty,"Radius server ac_radius_name is %s\n",sec->ac_radius_name);
					vty_out(vty,"Radius server heart test activated is %s\n",(sec->heart_test_on == 1)?on:off);
					vty_out(vty,"Radius server acct current use is %s\n",(sec->acct_server_current_use == 0)?master:((sec->acct_server_current_use == 1)?bak:disable));
					vty_out(vty,"Radius server auth current use is %s\n",(sec->auth_server_current_use == 0)?master:((sec->auth_server_current_use == 1)?bak:disable));
				}
				//end
				vty_out(vty,"==============================================================================\n");
				dcli_free_security(sec);
		}
		else if (ret == ASD_SECURITY_NOT_EXIST)
			vty_out(vty,"<error> security id does not exist\n");
		else if (ret == ASD_DBUS_ERROR)		
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				sec = show_security_id(dcli_dbus_connection, profile, security_id, localid, &ret);	
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if ((ret == 0) && (sec != NULL)) {
						memset(SecurityType, 0, 20);
						memset(EncryptionType, 0, 20);
						CheckSecurityType(SecurityType, sec->SecurityType);
						CheckEncryptionType(EncryptionType, sec->EncryptionType);

						vty_out(vty,"Name : %s\n",sec->name);
						vty_out(vty,"ID : %d\n",sec->SecurityID);
						vty_out(vty,"IP : %s\n",sec->host_ip);
						vty_out(vty,"SecurityType : %s\n",SecurityType);
						vty_out(vty,"EncryptionType : %s\n",EncryptionType);
						vty_out(vty,"SecurityKEY(%s): %s\n",(sec->keyInputType==0)?unk:((sec->keyInputType==1)?asc:hex),sec->SecurityKey);
						vty_out(vty,"SECURITY INDEX : %d\n\n",sec->security_index);
						vty_out(vty,"Extensible Authentication state: %s\n",(sec->extensible_auth == 1)?"open":"close");
						vty_out(vty,"Radius Server type: %s\n",(sec->wired_radius == 1)?"wired":"wireless");

						vty_out(vty,"Hybrid Authentication state: %s\n",(sec->hybrid_auth == 1)?"open":"close");	//mahz add 2011.2.28
						vty_out(vty,"MAC AUTH: %s\n",(sec->mac_auth == 1)?"enable":"disable");		//weichao add
						vty_out(vty,"WAPI RADIUS AUTH: %s\n",(sec->wapi_radius_auth == 1)?"enable":"disable");		//mahz add 2010.11.24
						vty_out(vty,"WAPI RADIUS AUTH USER PASSWD: %s\n",sec->user_passwd);			//mahz add 2010.12.9
						
						vty_out(vty,"EAP REAUTH PERIOD: %d\n",sec->eap_reauth_period);
						vty_out(vty,"ACCT INTERIM INTERVAL: %d\n",sec->acct_interim_interval);
						vty_out(vty,"1X QUIET PERIOD: %d\n",sec->quiet_period);
						vty_out(vty,"AUTH IP : %s:%d\n",sec->auth.auth_ip,sec->auth.auth_port);			
						vty_out(vty,"AUTH SHARED SECRET: %s\n",sec->auth.auth_shared_secret);
						vty_out(vty,"ACCT IP : %s:%d\n",sec->acct.acct_ip,sec->acct.acct_port);
						vty_out(vty,"ACCT SHARED SECRET : %s\n\n",sec->acct.acct_shared_secret);

						vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",sec->auth.secondary_auth_ip,sec->auth.secondary_auth_port);	
						vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",sec->auth.secondary_auth_shared_secret);
						vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",sec->acct.secondary_acct_ip,sec->acct.secondary_acct_port);
						vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n\n",sec->acct.secondary_acct_shared_secret);
																							
						vty_out(vty,"WAPI AUTH IP : %s:%d\n",sec->wapi_as.as_ip,3810);	
						vty_out(vty,"WAPI MULTI CERT : %s\n",sec->wapi_as.multi_cert==1?"enable":"disable");	
						vty_out(vty,"WAPI CERTIFICATION TPYE : %s\n",(sec->wapi_as.certification_type==WAPI_X509)?"X.509":((sec->wapi_as.certification_type==WAPI_GBW)?"GBW":" "));
						vty_out(vty,"WAPI AS CERTIFICATION PATH : %s\n",sec->wapi_as.certification_path);
						vty_out(vty,"WAPI AE CERTIFICATION PATH : %s\n",sec->wapi_as.ae_cert_path);
						vty_out(vty,"WAPI CA CERTIFICATION PATH : %s\n\n",sec->wapi_as.ca_cert_path);

						if(sec->SecurityType==WAPI_AUTH||sec->SecurityType==WAPI_PSK){
							CheckRekeyMethod(RekeyMethod,sec->wapi_ucast_rekey_method);
							vty_out(vty,"WAPI UNICAST REKEY METHOD : %s\n",RekeyMethod);
							if((sec->wapi_ucast_rekey_method==1)||(sec->wapi_ucast_rekey_method==3)){
								vty_out(vty,"WAPI UNICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_ucast_rekey_para_t);
							}
							if((sec->wapi_ucast_rekey_method==2)||(sec->wapi_ucast_rekey_method==3)){
								vty_out(vty,"WAPI UNICAST REKEY PACKET BASED PARA : %5u\n\n",sec->wapi_ucast_rekey_para_p);	
							}
							CheckRekeyMethod(RekeyMethod,sec->wapi_mcast_rekey_method);
							vty_out(vty,"WAPI MULTICAST REKEY METHOD : %s\n",RekeyMethod);
							if((sec->wapi_mcast_rekey_method==1)||(sec->wapi_mcast_rekey_method==3)){
								vty_out(vty,"WAPI MULTICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_t);
							}
							if((sec->wapi_mcast_rekey_method==2)||(sec->wapi_mcast_rekey_method==3)){
								vty_out(vty,"WAPI MULTICAST REKEY PACKET BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_p);
							}
						}

						dcli_free_security(sec);
				}
				else if (ret == ASD_SECURITY_NOT_EXIST)
					vty_out(vty,"<error> security id does not exist\n");
				else if (ret == ASD_DBUS_ERROR)		
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error>  %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			sec = show_security_id(dcli_dbus_connection, profile, security_id, localid, &ret);	
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if ((ret == 0) && (sec != NULL)) {
					memset(SecurityType, 0, 20);
					memset(EncryptionType, 0, 20);
					CheckSecurityType(SecurityType, sec->SecurityType);
					CheckEncryptionType(EncryptionType, sec->EncryptionType);

					vty_out(vty,"Name : %s\n",sec->name);
					vty_out(vty,"ID : %d\n",sec->SecurityID);
					vty_out(vty,"IP : %s\n",sec->host_ip);
					vty_out(vty,"SecurityType : %s\n",SecurityType);
					vty_out(vty,"EncryptionType : %s\n",EncryptionType);
					vty_out(vty,"SecurityKEY(%s): %s\n",(sec->keyInputType==0)?unk:((sec->keyInputType==1)?asc:hex),sec->SecurityKey);
					vty_out(vty,"SECURITY INDEX : %d\n\n",sec->security_index);
					vty_out(vty,"Extensible Authentication state: %s\n",(sec->extensible_auth == 1)?"open":"close");
					vty_out(vty,"Radius Server type: %s\n",(sec->wired_radius == 1)?"wired":"wireless");

					vty_out(vty,"Hybrid Authentication state: %s\n",(sec->hybrid_auth == 1)?"open":"close");	//mahz add 2011.2.28
					vty_out(vty,"MAC AUTH: %s\n",(sec->mac_auth == 1)?"enable":"disable");		//weichao add
					vty_out(vty,"WAPI RADIUS AUTH: %s\n",(sec->wapi_radius_auth == 1)?"enable":"disable");		//mahz add 2010.11.24
					vty_out(vty,"WAPI RADIUS AUTH USER PASSWD: %s\n",sec->user_passwd);			//mahz add 2010.12.9
					
					vty_out(vty,"EAP REAUTH PERIOD: %d\n",sec->eap_reauth_period);
					vty_out(vty,"ACCT INTERIM INTERVAL: %d\n",sec->acct_interim_interval);
					vty_out(vty,"1X QUIET PERIOD: %d\n",sec->quiet_period);
					vty_out(vty,"AUTH IP : %s:%d\n",sec->auth.auth_ip,sec->auth.auth_port);			
					vty_out(vty,"AUTH SHARED SECRET: %s\n",sec->auth.auth_shared_secret);
					vty_out(vty,"ACCT IP : %s:%d\n",sec->acct.acct_ip,sec->acct.acct_port);
					vty_out(vty,"ACCT SHARED SECRET : %s\n\n",sec->acct.acct_shared_secret);

					vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",sec->auth.secondary_auth_ip,sec->auth.secondary_auth_port);	
					vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",sec->auth.secondary_auth_shared_secret);
					vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",sec->acct.secondary_acct_ip,sec->acct.secondary_acct_port);
					vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n\n",sec->acct.secondary_acct_shared_secret);
																						
					vty_out(vty,"WAPI AUTH IP : %s:%d\n",sec->wapi_as.as_ip,3810);	
					vty_out(vty,"WAPI MULTI CERT : %s\n",sec->wapi_as.multi_cert==1?"enable":"disable");	
					vty_out(vty,"WAPI CERTIFICATION TPYE : %s\n",(sec->wapi_as.certification_type==WAPI_X509)?"X.509":((sec->wapi_as.certification_type==WAPI_GBW)?"GBW":" "));
					vty_out(vty,"WAPI AS CERTIFICATION PATH : %s\n",sec->wapi_as.certification_path);
					vty_out(vty,"WAPI AE CERTIFICATION PATH : %s\n",sec->wapi_as.ae_cert_path);
					vty_out(vty,"WAPI CA CERTIFICATION PATH : %s\n\n",sec->wapi_as.ca_cert_path);

					if(sec->SecurityType==WAPI_AUTH||sec->SecurityType==WAPI_PSK){
						CheckRekeyMethod(RekeyMethod,sec->wapi_ucast_rekey_method);
						vty_out(vty,"WAPI UNICAST REKEY METHOD : %s\n",RekeyMethod);
						if((sec->wapi_ucast_rekey_method==1)||(sec->wapi_ucast_rekey_method==3)){
							vty_out(vty,"WAPI UNICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_ucast_rekey_para_t);
						}
						if((sec->wapi_ucast_rekey_method==2)||(sec->wapi_ucast_rekey_method==3)){
							vty_out(vty,"WAPI UNICAST REKEY PACKET BASED PARA : %5u\n\n",sec->wapi_ucast_rekey_para_p);	
						}
						CheckRekeyMethod(RekeyMethod,sec->wapi_mcast_rekey_method);
						vty_out(vty,"WAPI MULTICAST REKEY METHOD : %s\n",RekeyMethod);
						if((sec->wapi_mcast_rekey_method==1)||(sec->wapi_mcast_rekey_method==3)){
							vty_out(vty,"WAPI MULTICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_t);
						}
						if((sec->wapi_mcast_rekey_method==2)||(sec->wapi_mcast_rekey_method==3)){
							vty_out(vty,"WAPI MULTICAST REKEY PACKET BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_p);
						}
					}

					dcli_free_security(sec);
			}
			else if (ret == ASD_SECURITY_NOT_EXIST)
				vty_out(vty,"<error> security id does not exist\n");
			else if (ret == ASD_DBUS_ERROR)		
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
		
	return CMD_SUCCESS;
}
#else
DEFUN(show_security_cmd_func,
	  show_security_cmd,
	  "show security SECURITYID",
	  CONFIG_STR
	  "Security profile information\n"
	  "security id\n"
	 )
{	int ret;
	unsigned char security_id, ID;
	char * Name;
	char * IP;
	auth_server auth;
	acct_server acct;

	WAPI_AS wapi;
	
	unsigned int Stype, Etype;
	char * key;
	char SecurityType[20];
	char RekeyMethod[30];	/*	xm0701*/
	char EncryptionType[20];
	int eap_reauth_period;
	int acct_interim_interval;
	int input_type=-1;
	int exten_auth,wired;
	char asc[]="ASCII";
	char hex[]="HEX";
	char unk[]="Unknown";

	unsigned int quitePeriod;
	unsigned char wapi_ucast_rekey_method;
	unsigned int wapi_ucast_rekey_para_t;
 	unsigned int wapi_ucast_rekey_para_p;

 	unsigned char wapi_mcast_rekey_method;
 	unsigned int wapi_mcast_rekey_para_t;
 	unsigned int wapi_mcast_rekey_para_p;
	
	ret = parse_char_ID((char*)argv[0], &security_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
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
					DBUS_TYPE_BYTE,&wapi_ucast_rekey_method,
					DBUS_TYPE_UINT32,&wapi_ucast_rekey_para_t,
					DBUS_TYPE_UINT32,&wapi_ucast_rekey_para_p,

					DBUS_TYPE_BYTE,&wapi_mcast_rekey_method,
					DBUS_TYPE_UINT32,&wapi_mcast_rekey_para_t,
					DBUS_TYPE_UINT32,&wapi_mcast_rekey_para_p,	/*	xm0701*/
					
					DBUS_TYPE_STRING,&Name,
					DBUS_TYPE_BYTE,&ID,
					DBUS_TYPE_STRING,&IP,
					DBUS_TYPE_UINT32,&(auth.auth_port),
					DBUS_TYPE_STRING,&(auth.auth_ip),
					DBUS_TYPE_STRING,&(auth.auth_shared_secret),
					/*////////////////////////////////////////*/
					DBUS_TYPE_UINT32,&(auth.secondary_auth_port),
					DBUS_TYPE_STRING,&(auth.secondary_auth_ip),
					DBUS_TYPE_STRING,&(auth.secondary_auth_shared_secret),	
					DBUS_TYPE_UINT32,&(acct.secondary_acct_port),
					DBUS_TYPE_STRING,&(acct.secondary_acct_ip),
					DBUS_TYPE_STRING,&(acct.secondary_acct_shared_secret),	
					DBUS_TYPE_UINT32,&eap_reauth_period,			/*xm 08/09/03	*/	
					DBUS_TYPE_UINT32,&acct_interim_interval,			/*ht 090206	*/
					DBUS_TYPE_UINT32,&quitePeriod,			/*ht 090727 */
					/*//////////////////////////////////////////*/
					DBUS_TYPE_UINT32,&(acct.acct_port),
					DBUS_TYPE_STRING,&(acct.acct_ip),
					DBUS_TYPE_STRING,&(acct.acct_shared_secret),					
					DBUS_TYPE_UINT32,&Stype,
					DBUS_TYPE_UINT32,&Etype,
					DBUS_TYPE_STRING,&key,
					DBUS_TYPE_UINT32,&input_type,    /*xm add 08/11/25*/
					DBUS_TYPE_UINT32,&exten_auth,
					DBUS_TYPE_UINT32,&wired,

					DBUS_TYPE_STRING,&(wapi.as_ip),
					DBUS_TYPE_BYTE,&(wapi.multi_cert),
					DBUS_TYPE_STRING,&(wapi.certification_path),
					DBUS_TYPE_STRING,&(wapi.ae_cert_path),
					DBUS_TYPE_STRING,&(wapi.ca_cert_path),
					DBUS_TYPE_UINT32,&(wapi.certification_type),
					DBUS_TYPE_INVALID)) {
		if(ret == 0){			
			memset(SecurityType, 0, 20);
			memset(EncryptionType, 0, 20);
			CheckSecurityType(SecurityType, Stype);
			CheckEncryptionType(EncryptionType, Etype);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"NAME : %s\n",Name);
			vty_out(vty,"ID : %d\n",ID);
			vty_out(vty,"IP : %s\n",IP);
			vty_out(vty,"SecurityType : %s\n",SecurityType);
			vty_out(vty,"EncryptionType : %s\n",EncryptionType);
			vty_out(vty,"SecurityKEY(%s): %s\n\n",(input_type==0)?unk:((input_type==1)?asc:hex),key);
			vty_out(vty,"Extensible Authentication state: %s\n",(exten_auth == 1)?"open":"close");
			vty_out(vty,"Radius Server type: %s\n",(wired == 1)?"wired":"wireless");
			vty_out(vty,"EAP REAUTH PERIOD: %d\n",eap_reauth_period);
			vty_out(vty,"ACCT INTERIM INTERVAL: %d\n",acct_interim_interval);
			vty_out(vty,"1X QUITE PERIOD: %d\n",quitePeriod);
			vty_out(vty,"AUTH IP : %s:%d\n",auth.auth_ip,auth.auth_port);			
			vty_out(vty,"AUTH SHARED SECRET: %s\n",auth.auth_shared_secret);
			vty_out(vty,"ACCT IP : %s:%d\n",acct.acct_ip,acct.acct_port);
			vty_out(vty,"ACCT SHARED SECRET : %s\n\n",acct.acct_shared_secret);
			/*////////////////////////////////////////////////////////*/
			vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",auth.secondary_auth_ip,auth.secondary_auth_port);	
			vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",auth.secondary_auth_shared_secret);
			vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",acct.secondary_acct_ip,acct.secondary_acct_port);
			vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n\n",acct.secondary_acct_shared_secret);/*xm 08/09/03*/
																				/*xm 08/09/03	*/
			vty_out(vty,"WAPI AUTH IP : %s:%d\n",wapi.as_ip,3810);	
			vty_out(vty,"WAPI MULTI CERT : %s\n",wapi.multi_cert==1?"enable":"disable");	
			vty_out(vty,"WAPI CERTIFICATION TPYE : %s\n",(wapi.certification_type==WAPI_X509)?"X.509":((wapi.certification_type==WAPI_GBW)?"GBW":" "));
			vty_out(vty,"WAPI AS CERTIFICATION PATH : %s\n",wapi.certification_path);
			vty_out(vty,"WAPI AE CERTIFICATION PATH : %s\n",wapi.ae_cert_path);
			vty_out(vty,"WAPI CA CERTIFICATION PATH : %s\n\n",wapi.ca_cert_path);

			if(Stype==WAPI_AUTH||Stype==WAPI_PSK){

				CheckRekeyMethod(RekeyMethod,wapi_ucast_rekey_method);
				vty_out(vty,"WAPI UNICAST REKEY METHOD : %s\n",RekeyMethod);
				if((wapi_ucast_rekey_method==1)||(wapi_ucast_rekey_method==3)){
					vty_out(vty,"WAPI UNICAST REKEY TIME BASED PARA : %5u\n",wapi_ucast_rekey_para_t);
					/*	xm0701*/
				}
				if((wapi_ucast_rekey_method==2)||(wapi_ucast_rekey_method==3)){
					vty_out(vty,"WAPI UNICAST REKEY PACKET BASED PARA : %5u\n\n",wapi_ucast_rekey_para_p);	
					/*	xm0701*/
				}
				CheckRekeyMethod(RekeyMethod,wapi_mcast_rekey_method);
				vty_out(vty,"WAPI MULTICAST REKEY METHOD : %s\n",RekeyMethod);
				if((wapi_mcast_rekey_method==1)||(wapi_mcast_rekey_method==3)){
					vty_out(vty,"WAPI MULTICAST REKEY TIME BASED PARA : %5u\n",wapi_mcast_rekey_para_t);
				}
				if((wapi_mcast_rekey_method==2)||(wapi_mcast_rekey_method==3)){
					vty_out(vty,"WAPI MULTICAST REKEY PACKET BASED PARA : %5u\n",wapi_mcast_rekey_para_p);
				}
			}
			/*//////////////////////////////////////////////////////*/
			vty_out(vty,"==============================================================================\n");

		}
	}
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> security id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif


//mahz add for mib_request , 2011.1.12
DEFUN(show_radius_all_cmd_func,
	  show_radius_all_cmd,
	  "show radius all",
	  CONFIG_STR
	  "Radius profile information\n"
	  "all radius id\n"
	 )
{	
	 
	struct dcli_security	*sec = NULL;
	struct dcli_security	*tmp = NULL;
	unsigned int 	ret = 0;
	unsigned char 	num = 0;

	int  i;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	sec = show_radius_all(dcli_dbus_connection, index, &num, localid, &ret);
	
	if (ret == 0) {
		tmp = sec;
		for (i = 0; i < num; i++) {
			if(tmp == NULL)
				break;			
			vty_out(vty,"\n");
			vty_out(vty,"RADIUS ID : %u\n",tmp->RadiusID);			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"AUTH IP : %s:%d\n",tmp->auth.auth_ip,tmp->auth.auth_port);			
			vty_out(vty,"AUTH SHARED SECRET: %s\n",tmp->auth.auth_shared_secret);
			vty_out(vty,"ACCT IP : %s:%d\n",tmp->acct.acct_ip,tmp->acct.acct_port);
			vty_out(vty,"ACCT SHARED SECRET : %s\n\n",tmp->acct.acct_shared_secret);

			vty_out(vty,"SECONDARY AUTH IP : %s:%d\n",tmp->auth.secondary_auth_ip,tmp->auth.secondary_auth_port); 
			vty_out(vty,"SECONDARY AUTH SHARED SECRET: %s\n",tmp->auth.secondary_auth_shared_secret);
			vty_out(vty,"SECONDARY ACCT IP : %s:%d\n",tmp->acct.secondary_acct_ip,tmp->acct.secondary_acct_port);
			vty_out(vty,"SECONDARY ACCT SHARED SECRET : %s\n",tmp->acct.secondary_acct_shared_secret);
			vty_out(vty,"==============================================================================\n");
			tmp = tmp->next;
		}
			dcli_free_security_list(sec);
	}
	
	else if (ret == ASD_DBUS_ERROR){		
		cli_syslog_info("<error> failed get reply.\n");
		return CMD_FAILURE;
	}
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> no security support radius.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		
	return CMD_SUCCESS;
}

DEFUN(config_security_cmd_func,
	  config_security_cmd,
	  "config security SECURITYID",
	  CONFIG_STR
	  "Security information\n"
	  "security id that you want to config\n"
	 )
{	int ret;
	unsigned char security_id,status,wlanid;
/*    security_id = atoi(argv[0]);	*/	
	ret = parse_char_ID((char*)argv[0], &security_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_WARNING;
	}	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_WARNING;
	}
	DBusMessage *query, *reply;
	DBusError err;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIG_SECURITY);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIG_SECURITY);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
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
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			if(vty->node == CONFIG_NODE){
				vty->node = SECURITY_NODE;
				vty->index = (void *)security_id;
			}else if(vty->node == HANSI_NODE){
				vty->node = HANSI_SECURITY_NODE;
				vty->index_sub = (void *)security_id;
				vty->local = 0;
			}else if(vty->node == LOCAL_HANSI_NODE){
				vty->node = LOCAL_HANSI_SECURITY_NODE;
				vty->index_sub = (void *)security_id;
				vty->local = 1;
			}
		}else if (ret == ASD_SECURITY_NOT_EXIST){
			vty_out(vty,"<error> security id does not exist\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
			}
		else{
			vty_out(vty,"<error>  %d\n",ret);
			dbus_message_unref(reply);
			return CMD_WARNING;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(set_security_quiet_period_cmd_func,
	  set_security_quiet_period_cmd,
	  "security quiet period <0-65535>",
	  "config security\n"
	  "1X restart time\n"
	  "period value\n"
	  )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned int quietPeriod=0;

//	security_id = (int)vty->index;
	quietPeriod= atoi(argv[0]);

	if(quietPeriod<0||quietPeriod>65535)
		{
			vty_out(vty,"<error> input time value should be 0 to 65535.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD);
/*
	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&quietPeriod,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security type set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set 1x quiet period under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}



/*ht add,081105*/
DEFUN(config_security_host_ip_cmd_func,
	  config_security_host_ip_cmd,
	  "security host_ip IP",
	  "Configuring local ip\n"
	  "ip addr which connects with radius server\n"
	 )
{	int ret, ret1, ret2;
	unsigned char security_id;
	char *ip;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;

	ret1 = Check_IP_Format((char*)argv[0]);
	if(ret1 != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	#if 0 
	ret2 = Check_Local_IP((char*)argv[0]);
	if (ret2 == -1){
		vty_out(vty,"<error> check_local_ip error\n");
		return CMD_SUCCESS;
	}else if(ret2 == 0){
		vty_out(vty,"<error> not local ip \n");
		return CMD_SUCCESS;
	}
	#endif
	ip = (char *)malloc(strlen(argv[0])+1);
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_HOST_IP);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_HOST_IP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	free(ip);
	ip = NULL;
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security host ip set successfully \n");
		else if(ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");	
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)//qiuchen
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_security_type_cmd_func,
	  config_security_type_cmd,
	  /*"security type TYPE",*/
	  "security type (open|shared|802.1x|wpa_p|wpa2_p|wpa_e|wpa2_e|md5|wapi_psk|wapi_auth)",
	  "Configure security service\n"
	  "type of security\n"
	  /*"OPEN/SHARED/802.1X/WPA_P/WPA2_P/WPA_E/WPA2_E supported"*/
	 )
{	int ret;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"OPEN"))||(!strcmp(argv[0],"open")))
		type = OPEN;			
	else if ((!strcmp(argv[0],"SHARED"))||(!strcmp(argv[0],"shared")))
		type = SHARED;	
	else if ((!strcmp(argv[0],"802.1X"))||(!strcmp(argv[0],"802.1x")))
		type = IEEE8021X;
	else if ((!strcmp(argv[0],"WPA_P"))||(!strcmp(argv[0],"wpa_p")))
		type = WPA_P;
	else if ((!strcmp(argv[0],"WPA2_P"))||(!strcmp(argv[0],"wpa2_p")))
		type = WPA2_P;
	else if ((!strcmp(argv[0],"WPA_E"))||(!strcmp(argv[0],"wpa_e")))
		type = WPA_E;
	else if ((!strcmp(argv[0],"WPA2_E"))||(!strcmp(argv[0],"wpa2_e")))
		type = WPA2_E;	
	else if ((!strcmp(argv[0],"MD5"))||(!strcmp(argv[0],"md5")))
		type = MD5;	
	else if ((!strcmp(argv[0],"WAPI_PSK"))||(!strcmp(argv[0],"wapi_psk")))
		type = WAPI_PSK;	
	else if ((!strcmp(argv[0],"WAPI_AUTH"))||(!strcmp(argv[0],"wapi_auth")))
		type = WAPI_AUTH;	
	else 
	{		
		vty_out(vty,"<error> unknown security type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security type set successfully\n");
		else if (ret==ASD_SECURITY_TYPE_HAS_CHANGED)
		{
		vty_out(vty,"security type set successfully\n");
		vty_out(vty,"security type changes,remember setting security %d's other property.\n",security_id);// nl add 091028
		}
		else if(ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");	
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)//qiuchen
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
//weichao add 2011.10.31

DEFUN(config_mac_auth_cmd_func,
	  config_mac_auth_cmd,
	  "mac auth (enable|disable)",
	  "mac auth is enable.\n"
	  "portal auth is disable\n"
	  "default disable"
	 )
{	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int slot_id = HostSlotId;
	int localid = 1;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = (int)vty->index;			
		security_id = (int)vty->index_sub;	
	}
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_MAC_AUTH);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"mac auth %s successfully\n",argv[0]);
	else if (ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");			
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
		vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
	else if(ret == ASD_MAC_AUTH_NOT_SUPPORT)
		vty_out(vty,"<error> the security type or other is not right!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_encryption_type_cmd_func,
	  config_encryption_type_cmd,
	  /*"encryption type TYPE",*/
	  "encryption type (none|wep|aes|tkip|sms4)",
	  "Configuring service.\n"
	  "type of encryption\n"
	  "NONE/WEP/AES/TKIP/SMS4 supported"
	 )
{	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"NONE"))||(!strcmp(argv[0],"none")))
		type = NONE;			
	else if ((!strcmp(argv[0],"WEP"))||(!strcmp(argv[0],"wep")))
		type = WEP;	
	else if ((!strcmp(argv[0],"AES"))||(!strcmp(argv[0],"aes")))
		type = AES;
	else if ((!strcmp(argv[0],"TKIP"))||(!strcmp(argv[0],"tkip")))
		type = TKIP;
	else if ((!strcmp(argv[0],"SMS4"))||(!strcmp(argv[0],"sms4")))
		type = SMS4;
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"encryption type successfully set.\n");
		else if(ret==ASD_SECURITY_TYPE_HAS_CHANGED)
		{
			vty_out(vty,"encryption type successfully set.\n");
			vty_out(vty,"encrytion type changes,security %d's formal set including key has cleared.\n",security_id);
		}
		else if (ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE)
			vty_out(vty,"<error> encryption type does not match security type\n");
		else if (ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)//qiuchen
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_extensible_auth_cmd_func,
	  config_extensible_auth_cmd,
	  "extensible authentication (enable|disable)",
	  "extensible service.\n"
	  "extensible authentication\n"
	  "default disable"
	 )
{	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_EXTENSIBLE_AUTH);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_EXTENSIBLE_AUTH);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"extensible authentication %s successfully\n",argv[0]);
		else if (ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE)
			vty_out(vty,"<error> encryption type does not match security type\n");
		else if (ret == ASD_EXTENSIBLE_AUTH_NOT_SUPPORT)
			vty_out(vty,"<error> extensible auth is supported open or shared\n");
		else if (ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_radius_server_select_cmd_func,
	  config_radius_server_select_cmd,
	  "connect with (wired|wireless) radius server",
	  "extensible service.\n"
	  "wired/wireless radius server\n"
	  "default wireless"
	 )
{	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"wired")))
		type = 1;			
	else if ((!strcmp(argv[0],"wireless")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_SERVER_SELECT);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_SERVER_SELECT);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"connect with %s radius server successfully.\n",argv[0]);
		else if (ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE)
			vty_out(vty,"<error> encryption type does not match security type\n");
		else if (ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



/*xm add 08/11/25*/
DEFUN(config_security_key_cmd_func,
	  config_security_key_cmd,
	  "security (ascii|hex) key KEY",
	  "Configuring service.\n"
	  "Input type of key\n"
	  "key of security\n"
	  "KEY"
	 ){
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	char *key;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char input_type_of_key;	/*1--ASCII        2--Hex*/
	//security_id = (int)vty->index;

	if ((!strcmp(argv[0],"ASCII"))||(!strcmp(argv[0],"ascii")))
		input_type_of_key=1;			
	else if ((!strcmp(argv[0],"HEX"))||(!strcmp(argv[0],"hex")))
		input_type_of_key= 2;	
	else 
	{		
		vty_out(vty,"<error> unknown input type.\n");
		return CMD_SUCCESS;
	}
	
	
	key = (char*)malloc(strlen(argv[1])+1);
	memset(key, 0, strlen(argv[1])+1);
	memcpy(key, argv[1], strlen(argv[1]));

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_KEY);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_KEY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_STRING,&key,
							 DBUS_TYPE_BYTE,&input_type_of_key,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	free(key);/*don't forget it.*/
	key = NULL;
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security key set successful\n");
		else if(ret == ASD_SECURITY_NOT_EXIST)
			vty_out(vty,"<error> asd security profile does not exist\n");
		else if(ret == ASD_SECURITY_KEY_NOT_PERMIT)
			vty_out(vty,"<error> asd security key does not permit setting\n");
		else if(ret == ASD_SECURITY_KEY_LEN_NOT_PERMIT)
			vty_out(vty,"<error> asd security key length is incorrect(WPA should be 8 to 63 ,WEP should be 5 or 13 or 16,WAPI_PSK should be great than 8.)\n ");
		else if(ret == ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX)
			vty_out(vty,"<error> asd security key length is incorrect(WPA should be 64 ,WEP should be 10 or 26 or 32,WAPI_PSK should be great than 16)\n ");
		else if(ret == ASD_SECURITY_KEY_HEX_FORMAT)
			vty_out(vty,"<error> key format is incorrect(key should be '0' to '9' or 'a' to 'f')\n ");
		else if(ret == ASD_SECURITY_KEY_HAS_BEEN_SET)
			vty_out(vty,"<error> asd security key has been setted\n");		
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else 
			vty_out(vty,"<error>  %d\n",ret);
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}




DEFUN(config_acct_auth_cmd_func,
	  config_acct_auth_cmd,
	  "radius (acct|auth) IP PORT SHARED_SECRET",
	  "Configuring service.\n"
	  "Make security account / authentication server\n"
	  "IP addr of acct/auth server\n"
	  "PORT of IP of acct/auth server\n"
	  "share secret of acct/auth server\n"
	 )
{	int ret;
	unsigned char security_id;
	unsigned int type, port;
	char *ip, *shared_secret;			
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if (!strncmp("acct",argv[0],strlen(argv[0])))
		type = 0;			
	else if (!strncmp("auth",argv[0],strlen(argv[0])))
		type = 1;
	ret = Check_IP_Format((char*)argv[1]);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	ip = (char *)malloc(strlen(argv[1])+1);
	memset(ip, 0, strlen(argv[1])+1);
	memcpy(ip, argv[1], strlen(argv[1]));
	
/*	port = atoi(argv[2]);*/	
	ret = parse_int_ID((char*)argv[2], &port);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown port format\n");
		free(ip);
		ip = NULL;
		return CMD_SUCCESS;
	}	
	if(port > 65535){
		vty_out(vty,"<error> unknown port id\n");
		free(ip);
		ip = NULL;
		return CMD_SUCCESS;
	}		
	shared_secret = (char *)malloc(strlen(argv[3])+1);
	memset(shared_secret, 0, strlen(argv[3])+1);
	memcpy(shared_secret, argv[3], strlen(argv[3]));
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	if(type == 0){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ACCT);
		/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ACCT);*/
	}else if(type == 1)	{	
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTH);
		/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTH);*/
	}
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&port,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&shared_secret,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	free(ip);
	ip = NULL;
	free(shared_secret);
	shared_secret = NULL;

	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security %d successfully set %s server.\n",security_id,argv[0]);
		//qiuchen add 2012.12.11
		else if(ret == ASD_SECURITY_RADIUS_HEARTTEST_DEFAULT){
			vty_out(vty,"security %d successfully set %s server.\n",security_id,argv[0]);
			vty_out(vty,"<Attention> The radius heart test type change into default (auth).\n");
		}
		//end
		else if(ret == ASD_SECURITY_TYPE_WITHOUT_8021X)			
			vty_out(vty,"<error> security type which you chose does not support 802.1X.\n");
		else if((ret == ASD_SECURITY_ACCT_BE_USED)||(ret == ASD_SECURITY_AUTH_BE_USED))
			vty_out(vty,"<error> changing radius info is not permited\n");		
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2010.12.9

DEFUN(config_wapi_radius_auth_set_user_passwd_cmd_func,
	  config_wapi_radius_auth_set_user_passwd_cmd,
 	  "wapi radius auth set passwd USER_PASSWD",
	  "Configuring service.\n"
	  "wapi radius auth set user passwd.\n"
	  "passwd of user\n"
	 )
{	int ret;
	unsigned char security_id;
	char *user_passwd;			
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
				
	user_passwd = (char *)malloc(strlen(argv[0])+1);
	memset(user_passwd, 0, strlen(argv[0])+1);
	memcpy(user_passwd, argv[0], strlen(argv[0]));
		
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_USER_PASSWD);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_STRING,&user_passwd,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	free(user_passwd);
	user_passwd = NULL;
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security %d successfully set user_passwd %s.\n",security_id,argv[0]);
		else if(ret == ASD_SECURITY_TYPE_WITHOUT_8021X)			
			vty_out(vty,"<error> security type which you chose does not support 802.1X.\n");
		else if((ret == ASD_SECURITY_ACCT_BE_USED)||(ret == ASD_SECURITY_AUTH_BE_USED))
			vty_out(vty,"<error> changing radius info is not permited\n");		
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
//

DEFUN(create_security_cmd_func,
		create_security_cmd,
		"create security SECURITYID SECURITYNAME",
		"Configuring service.\n"
		"security service\n"
		"assign security ID for security profile\n"
		"assign security NAME\n"
	/*	"assign security IP\n"*/
	)
{
	int ret,len;
	unsigned char isAdd = 1;	
	unsigned char security_id;
	unsigned int security_int_id;
	char *name = NULL;
	//char *security_ip = NULL;
	char *name_d = "0";
	char *security_ip_d = "0";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 1;			
/*	security_id = atoi(argv[0]);*/	
	ret = parse_int_ID((unsigned char*)argv[0], &security_int_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(security_int_id >= WLAN_NUM || security_int_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	security_id = security_int_id;
	len = strlen(argv[1]);
	if(len > 15){
		vty_out(vty,"<error> security name is too long,it should be 1 to 15 \n");
		return CMD_SUCCESS;
	}
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));		
/*	ret = Check_IP_Format((char*)argv[2]);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}*/
/*	security_ip = (char*)malloc(strlen(argv[2])+1);
//	memset(security_ip, 0, strlen(argv[2])+1);
//	memcpy(security_ip, argv[2], strlen(argv[2]));*/
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isAdd,								
						DBUS_TYPE_BYTE,&security_id,
						DBUS_TYPE_STRING,&name,
					/*	DBUS_TYPE_STRING,&security_ip,	*/						 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);//qiuchen
		name = NULL;
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"security profile %d was successfully created.\n",security_id);
		else if(ret == ASD_SECURITY_BE_USED)
			vty_out(vty,"<error> security id exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(name);
	name = NULL;
/* 	free(security_ip);*/
	return CMD_SUCCESS;	
}

DEFUN(delete_security_cmd_func,
		delete_security_cmd,
		"delete security SECURITYID",
		"Configuring service.\n"
		"security service\n"
		"assign security ID for security profile\n"
	)
{
	int ret;
	unsigned char isAdd = 1;	
	unsigned char security_id;
	char *name = NULL;
	char *security_ip = NULL;
	char *name_d = "0";
	char *security_ip_d = "0";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 0;
/*	security_id = atoi(argv[0]);*/		
	ret = parse_char_ID((char*)argv[0], &security_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	name = name_d;
	security_ip = security_ip_d;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isAdd,								
						DBUS_TYPE_BYTE,&security_id,
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_STRING,&security_ip,							 
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
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"Security profile %d was successfully deleted.\n",security_id);
		else if(ret == ASD_SECURITY_NOT_EXIST)
			vty_out(vty,"<error> security id does not exist\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_HEART_TEST_ON)
			vty_out(vty,"The radius heart test is on,turn it off first!\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

/*
DEFUN(apply_security_cmd_func,
	  apply_security_cmd,
	  "apply wlanID WLANIDS",
	  SERVICE_STR
	  "security profile bind with wlan\n"
	  "wlanID of wlan you bind\n"
	 )
{	int ret;
	unsigned char security_id;
	unsigned char WlanID;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	security_id = (int)vty->index;	
	WlanID = atoi(argv[0]);	
	if(WlanID >= WLAN_NUM || WlanID == 0){
		vty_out(vty,"WLAN ID should be 1-%d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_APPLY_WLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"Wlan %d was successfully binded.\n",WlanID);
		else if(ret == ASD_WLAN_NOT_EXIST)
			vty_out(vty,"WLAN %d NOT EXIT\n",WlanID);
		else if(ret == ASD_SECURITY_NOT_EXIST)
			vty_out(vty,"ASD SECURITY PROFILE NOT EXIST\n");
		else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
			vty_out(vty,"ASD SECURITY PROFILE NOT INTEGRITY\n");
		else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE)
			vty_out(vty,"encryption type dosen't match with security type\n");
		else if(ret == ASD_WLAN_HAS_BEEN_BINDED)
			vty_out(vty,"WLANID has been bind other security profile\n");
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

*/

/***************************************************************************************/
/*xm 08/08/27*/
DEFUN(	config_port_cmd_func,
	  		config_port_cmd,
	    "config port PORTLIST dot1x security SECURITYID",/*suppose that security ID is within <1-255>*/
	    "config\n"
	    "port\n"
		"port list\n"
		"dot1x is a fix format\n"
		"receive security id\n"
		"security id\n"
	 )
{

	DBusMessage* reply;	
	DBusMessageIter	 iter,iiter,iiiter;
	DBusMessageIter	 iter_array,iiter_array,iiiter_array;

	DBusError err;
	int ret,ret1 ;
    int i=0;
	DBusMessage *query; 

	unsigned char 		security=0;

	unsigned int 	    p_count = 0  ;  /*number of list from _parse_port_list*/
	unsigned int 		rcv_num=0;     /*number of slot-port-vlanid items,receive from wid*/

	SLOT_PORT_S slotport[MAX_NUM_OF_VLANID] ;
	memset(slotport,0,sizeof(SLOT_PORT_S)*MAX_NUM_OF_VLANID);

	PORTINDEX_VLANID_S pvs[MAX_NUM_OF_VLANID];
	memset(pvs,0,sizeof(PORTINDEX_VLANID_S)*MAX_NUM_OF_VLANID);

	ret1 = parse_char_ID((char*)argv[1], &security);
	if(ret1 != WID_DBUS_SUCCESS){
            if(ret1 == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(security >= WLAN_NUM || security == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return 0;
	}

	/*here I want to know weather I have received the message correctly.
//	printf("\n==================cmd start=====================\n");
//	printf("Get security id %d\n",security);*/

	if (-1 == _parse_port_list((char *)argv[0],&p_count,slotport)) {
    	vty_out(vty,"<error> unknown port format.\n");
		return CMD_FAILURE;
	}

	if(p_count<=0||p_count>24){

		vty_out(vty,"<error> unknown port format.\n");
		return CMD_FAILURE;
	}
	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WTP_OBJPATH,\
							WID_DBUS_WTP_INTERFACE,WID_DBUS_WTP_METHOD_CONFIG_PORT );
		
	dbus_message_iter_init_append (query, &iter);

		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										& p_count);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING							
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < p_count; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(slotport[i].slot));
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(slotport[i].port));

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);


	

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

	/*code receive module*/
	
	dbus_message_iter_init(reply,&iiter);
	dbus_message_iter_get_basic(&iiter,&ret);
	if(ret == 0 ){
		dbus_message_iter_next(&iiter);	
		dbus_message_iter_get_basic(&iiter,&rcv_num);
		
	/*here I want to know weather I have received the message correctly.
	//printf("\nthe totle number of slot-port-vlan is %2d\n ",rcv_num);*/			
	
	
		dbus_message_iter_next(&iiter);	
		dbus_message_iter_recurse(&iiter,&iiter_array);
		
		for (i = 0; i < rcv_num; i++) {
			DBusMessageIter iiter_struct;
			

			dbus_message_iter_recurse(&iiter_array,&iiter_struct);
		
			dbus_message_iter_get_basic(&iiter_struct,&(pvs[i].port_index));
		
			dbus_message_iter_next(&iiter_struct);
			
			dbus_message_iter_get_basic(&iiter_struct,&(pvs[i].vlan_id));
					
			dbus_message_iter_next(&iiter_array);

			pvs[i].security=security;

		    /*here I want to know weather I have received the message correctly.
			//printf("\nreceive %d.port_index-vlan_id:\t%2d-%2d\n ",i+1,pvs[i].port_index,pvs[i].vlan_id);
			//////////////////////////////////////////////////////////////////////////////////*/
			
		}
	}
	 
	dbus_message_unref(reply);
	

	/*now we  send port_index-vlan_id-security to asd .*/

	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT);
		
	dbus_message_iter_init_append (query, &iiiter);

		
	dbus_message_iter_append_basic (	&iiiter,
										DBUS_TYPE_UINT32,
										& rcv_num);

	
	/*here I want to know weather I have received the message correctly.
	//printf("\nWe send %d items with security information.\n ",rcv_num);	*/
	
		
	dbus_message_iter_open_container (&iiiter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iiiter_array);

	for(i = 0; i < rcv_num; i++){			
		DBusMessageIter iiiter_struct;
			
		dbus_message_iter_open_container (&iiiter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iiiter_struct);
		dbus_message_iter_append_basic
					(&iiiter_struct,
					   DBUS_TYPE_UINT32,
					  &(pvs[i].port_index));
		dbus_message_iter_append_basic
					(&iiiter_struct,
					  DBUS_TYPE_UINT32,
					  &(pvs[i].vlan_id));
		dbus_message_iter_append_basic
					(&iiiter_struct,
					  DBUS_TYPE_BYTE,
					  &(pvs[i].security));

		dbus_message_iter_close_container (&iiiter_array, &iiiter_struct);

	}
				
	dbus_message_iter_close_container (&iiiter, &iiiter_array);

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

	
	/*here I want to know weather I have send the message correctly.
	//printf("\nsend complete.\n ");	
	////////////////////////////////////////////////////*/
	
	return CMD_SUCCESS; 
	
}

DEFUN(	config_port_enable_cmd_func,
	  		config_port_enable_cmd,
	    "config port PORTLIST dot1x (enable|disable)",
	    "config\n"
	    "port\n"
		"port list\n"
		"dot1x is a fix format\n"
		"describe the state\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusError err;

	int i=0;
	unsigned int p_count=0;
	unsigned char stat=0;
	SLOT_PORT_S slotport[MAX_NUM_OF_VLANID] ;
	memset(slotport,0,sizeof(SLOT_PORT_S)*MAX_NUM_OF_VLANID);

	dbus_error_init(&err);
		
	/*receive argv[1]*/
	str2lower(&argv[1]);
	if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e'))
		stat=1;
	else
		stat=0;

	/*receive port list in slotport array*/
	if (-1 == _parse_port_list((char *)argv[0],&p_count,slotport)) {
			vty_out(vty,"<error> unknow port format.\n");
			return CMD_FAILURE;
		}

	/*now send  slot-port-stat to npd */
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WTP_OBJPATH,\
							WID_DBUS_WTP_INTERFACE,WID_DBUS_WTP_METHOD_CONFIG_PORT_ENABLE );

	dbus_message_iter_init_append (query, &iter);

		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										& p_count);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < p_count; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(slotport[i].slot));
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(slotport[i].port));
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &stat);

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	/*//////////////////////////////////////////////////////////////////////////
	//printf("\n==================cmd start=====================\n");
	//printf("dcli send to npd: set %d items %s\n",p_count,((stat==0)?"disable":"enable"));
	////////////////////////////////////////////////////////////////////////////*/
	
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

	dbus_message_unref(reply);
	
	if(ret == 0)
		vty_out(vty,"set successfully\n");
		
	else
		vty_out(vty,"<error>  %d\n",ret);

	
		
	/*now send  slot-port-stat to asd */
	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT_ENABLE);
	
	dbus_message_iter_init_append (query, &iter);

		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										& p_count);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < p_count; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);

			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(slotport[i].slot));
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(slotport[i].port));
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &stat);

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	/*/////////////////////////////////////////////////////////////////////////////
	//printf("\ndcli send to asd: set %d items %s\n",p_count,((stat==0)?"disable":"enable"));
	///////////////////////////////////////////////////////////////////////////////*/
	
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

	dbus_message_unref(reply);
	
	if(ret == 0)
		vty_out(vty,"set successfully\n");
		
	else
		vty_out(vty,"<error>  %d\n",ret);
	return CMD_SUCCESS;

}

/*xm 08/08/27*/
/***************************************************************************************/
DEFUN(config_wapi_auth_cmd_func,
		  config_wapi_auth_cmd,
		  "wapi as IP certification type (X.509|GBW) ",
		  "Configuring service.\n"
		  "set AS ip port and certification type\n"
		  "IP addr of AS\n"
		  "CERTIPATH certification path\n"
	 )
{		int ret;
		unsigned char security_id;
		unsigned int type;
		char *ip;			
		DBusMessage *query, *reply; 
		DBusMessageIter  iter;
		DBusError err;
		
		//security_id = (int)vty->index;
		if (!strcmp(argv[1],"X.509"))
			type = 0;			
		else if (!strcmp(argv[1],"GBW"))
			type = 1;
		else {
			vty_out(vty,"<error> unknown certification type\n");
			return CMD_SUCCESS;
		}
		
		ret = Check_IP_Format((char*)argv[0]);
		if(ret != ASD_DBUS_SUCCESS){
			vty_out(vty,"<error> unknown ip format\n");
			return CMD_SUCCESS;
		}
		ip = (char *)malloc(strlen(argv[0])+1);
		memset(ip, 0, strlen(argv[0])+1);
		memcpy(ip, argv[0], strlen(argv[0]));
		
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_AUTH);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_AUTH);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	free(ip);
	ip=NULL;
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
	
			if(ret == ASD_DBUS_SUCCESS)
				vty_out(vty,"security %d successfully set wapi auth server.\n",security_id);
			else if(ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH) 		
				vty_out(vty,"<error> security type which you chose does not support wapi authentication.\n");		
			else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
				vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
}


DEFUN(config_wapi_auth_path_cmd_func,
	  config_wapi_auth_path_cmd,
	  "wapi (as|ae|ca) certification path CERTIPATH",
	  "Configuring wapi.\n"
	  "set AS or AE or CA certification path\n"
	  "CERTIPATH certification path\n"
	 )
{	int ret;
	unsigned char security_id;
	unsigned int type;
	char *cert_path;			
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//security_id = (int)vty->index;
	if (!strcmp(argv[0],"as"))                 /*as--1 ; ae--2; ca--3*/
		type = 1;			
	else if (!strcmp(argv[0],"ae"))
		type = 2;
	else if (!strcmp(argv[0],"ca"))
		type = 3;

	if(access(argv[1],R_OK) != 0){
		vty_out(vty,"<error> certification %s isn't exit or can't be read.\n",argv[1]);
		return CMD_SUCCESS;
	}
	cert_path = (char *)malloc(strlen(argv[1])+1);
	memset(cert_path, 0, strlen(argv[1])+1);
	memcpy(cert_path, argv[1], strlen(argv[1]));
	

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_PATH);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_WAPI_PATH);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&cert_path,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);


	free(cert_path);
	cert_path=NULL;
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set wapi certification path.\n",security_id);
	else if(ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)			
		vty_out(vty,"<error> security type which you chose does not support wapi authentication.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
		vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> this security profile isn't integrity\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//weichao add 20110801
DEFUN(delete_wapi_certification_cmd_func,
	  delete_wapi_certification_cmd,
	  "del wapi (as|ae|ca) certification ",
	  "Configuring wapi.\n"
	  "delete AS or AE or CA certification \n"
	 )

{	int ret;
	unsigned char security_id;
	unsigned int type;
	char *cert_path;			
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//security_id = (int)vty->index;
	if (!strcmp(argv[0],"as"))                 /*as--1 ; ae--2; ca--3*/
		type = 1;			
	else if (!strcmp(argv[0],"ae"))
		type = 2;
	else if (!strcmp(argv[0],"ca"))
		type = 3;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DEL_WAPI_CER);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully delete wapi certification path.\n",security_id);
	else if(ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)			
		vty_out(vty,"<error> security type which you chose does not support wapi authentication.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
		vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> this security profile isn't integrity\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		

		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_wapi_p12_cert_auth_path_cmd_func,
	  config_wapi_p12_cert_auth_path_cmd,
	  "wapi (as|ae|ca) p12 certification path CERTIPATH PASSWORD",
	  "Configuring wapi.\n"
	  "set AS or AE or CA certification path\n"
	  "CERTIPATH certification path\n"
	  "CERTIPATH certification password\n"
	 )
{	
	 		
	unsigned char security_id;
	unsigned int type;
	char *cert_path;
	char *pass_word;
	int   ret;
	
	if (!strcmp(argv[0],"as"))				   /*as--1 ; ae--2; ca--3*/
		type = 1;			
	else if (!strcmp(argv[0],"ae"))
		type = 2;
	else if (!strcmp(argv[0],"ca"))
		type = 3;

	
	if(access(argv[1],R_OK) != 0){
		vty_out(vty,"<error> certification %s isn't exit or can't be read.\n",argv[1]);
		return CMD_SUCCESS;
	}
	cert_path = (char *)malloc(strlen(argv[1])+1);
	memset(cert_path, 0, strlen(argv[1])+1);
	memcpy(cert_path, argv[1], strlen(argv[1]));
	
	pass_word = (char *)malloc(strlen(argv[2])+1);
	memset(pass_word, 0, strlen(argv[2])+1);
	memcpy(pass_word, argv[2], strlen(argv[2]));

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	set_wapi_p12_cert_path(dcli_dbus_connection, index, security_id, type, cert_path, pass_word, localid, &ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set wapi p12 cert certification path.\n",security_id);
	else if(ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH) 		
		vty_out(vty,"<error> security type which you chose does not support wapi authentication.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> this security profile isn't integrity\n");
	else if(ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> p12 cert password error.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		
	return CMD_SUCCESS;
}



DEFUN(config_pre_auth_cmd_func,
	  config_pre_auth_cmd_func_cmd,
	  "pre-authentication (enable|disable)",
	  "pre-authentication service.\n"
	  "default disable"
	 )
{	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_PRE_AUTHENTICATION);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_PRE_AUTHENTICATION);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"extensible authentication %s successfully\n",argv[0]);
		else if (ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE)
			vty_out(vty,"<error> encryption type does not match security type\n");
		else if (ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*ht add ,090827*/
DEFUN(config_accounting_on_cmd_func,
	  config_accounting_on_cmd,
	  "accounting on (enable|disable)",
	  "accounting service.\n"
	  "default enable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	//security_id = (int)vty->index;
	
	if ((!strcmp(argv[0],"enable")))
		type = 0;			
	else if ((!strcmp(argv[0],"disable")))
		type = 1;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"accounting on %s successfully\n",argv[0]);
		else if (ret == ASD_SECURITY_TYPE_WITHOUT_8021X)
			vty_out(vty,"<error> security type is needn't 802.1X\n");
		else if (ret == ASD_SECURITY_NOT_EXIST) 		
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2011.10.25
DEFUN(set_asd_distribute_cmd_func,
	  set_asd_distribute_cmd,
	  "set asd distribute (enable|disable)",
	  "asd distribute service.\n"
	  "default enable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	if ((!strcmp(argv[0],"enable")))
		type = 0;			
	else if ((!strcmp(argv[0],"disable")))
		type = 1;	
	else {		
		vty_out(vty,"<error> unknown input parameter.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DISTRIBUTE_ON);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd distribute %s successfully\n",argv[0]);
	else if (ret == ASD_SECURITY_TYPE_WITHOUT_8021X)
		vty_out(vty,"<error> security type should be 802.1X, wpa_e or wpa2_e\n");
	else if (ret == ASD_SECURITY_NOT_EXIST) 		
		vty_out(vty,"<error> security profile does not exist.\n");			
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_HEART_TEST_ON)
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2011.10.26
DEFUN(set_asd_rdc_para_cmd_func,
	  set_asd_rdc_para_cmd,
	  "set asd rdc slotid SLOTID instid INSTID",
	  "asd rdc parameter.\n"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int slot_value;
	unsigned int inst_value;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	slot_value = atoi(argv[0]);
	inst_value = atoi(argv[1]);
	if(slot_value < 0 || slot_value > 16){
		vty_out(vty,"<error> slotid should be 0 to 16.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RDC_PARA);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&slot_value,
							 DBUS_TYPE_UINT32,&inst_value,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd rdc slotid %s instid %s successfully\n",argv[0],argv[1]);
	else if (ret == ASD_SECURITY_TYPE_WITHOUT_8021X)
		vty_out(vty,"<error> security type should be 802.1X, wpa_e or wpa2_e\n");
	else if (ret == ASD_SECURITY_NOT_EXIST) 		
		vty_out(vty,"<error> security profile does not exist.\n");			
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_HEART_TEST_ON)
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2011.11.18
DEFUN(show_asd_rdc_info_cmd_func,
	  show_asd_rdc_info_cmd,
	  "show asd rdc info",
	  "show asd rdc parameter.\n"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char 	num = 0;
	struct dcli_security	*sec = NULL;
	struct dcli_security	*tmp = NULL;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	int  i;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	sec = show_asd_rdc_info(dcli_dbus_connection, index, &num, localid, &ret);
	
	if (ret == 0) {
		tmp = sec;
		for (i = 0; i < num; i++) {
			if(tmp == NULL)
				break;			
			vty_out(vty,"\n");
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"SECURITY ID : %u\n",tmp->SecurityID);			
			vty_out(vty,"RDC SLOTID : %d\n",tmp->slot_value);			
			vty_out(vty,"RDC INSTID : %d\n",tmp->inst_value);			
			vty_out(vty,"==============================================================================\n");
			tmp = tmp->next;
		}
			dcli_free_security_list(sec);
	}
	else if (ret == ASD_DBUS_ERROR){		
		cli_syslog_info("<error> failed get reply.\n");
		return CMD_FAILURE;
	}
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> no distribution info is configed.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
		
	return CMD_SUCCESS;
}

/*nl 1010*/
DEFUN(set_mobile_open_cmd_func,
	  set_mobile_open_cmd,
	  "set wtp send response to mobile (enable|disable)",
	  "wtp send response to mobile\n"
	  "default disable\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char wtp_send_response_to_mobilephone=0;
	if(!strcmp(argv[0],"enable")){
		wtp_send_response_to_mobilephone=1;
	}
	else if(!strcmp(argv[0],"disable")){
	wtp_send_response_to_mobilephone=0;
	}
		else 
		{vty_out(vty,"<error> input should be enable or disable.\n");
		return CMD_SUCCESS;
		}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_MOBILE_OPEN);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_MOBILE_OPEN);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&wtp_send_response_to_mobilephone,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security type set successfully\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*ht add ,090827*/
DEFUN(config_radius_extend_attr_cmd_func,
	  config_radius_extend_attr_cmd,
	  "radius extend attribute (enable|disable)",
	  "accounting service.\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned int type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	//security_id = (int)vty->index;
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"radius extend attribute %s successfully\n",argv[0]);
		else if (ret == ASD_SECURITY_TYPE_WITHOUT_8021X)
			vty_out(vty,"<error> security type is needn't 802.1X\n");
		else if (ret == ASD_SECURITY_NOT_EXIST) 		
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2010.11.24

DEFUN(config_wapi_radius_auth_cmd_func,
	  config_wapi_radius_auth_cmd,
	  "wapi radius auth (enable|disable)",
	  "wapi radius auth.\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned int type = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown security type.\n");
		return CMD_SUCCESS;
	}

	int  index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_RADIUS_AUTH);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"wapi radius auth %s successfully\n",argv[0]);
		else if (ret == ASD_SECURITY_TYPE_WITHOUT_8021X)
			vty_out(vty,"<error> security type is needn't 802.1X\n");
		else if (ret == ASD_SECURITY_NOT_EXIST) 		
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}



//mahz add 2011.2.18 for Hybrid Authentication
DEFUN(config_hybrid_auth_cmd_func,
	  config_hybrid_auth_cmd,
	  "hybrid auth (enable|disable)",
	  "hybrid auth.\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned int type = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown security type.\n");
		return CMD_SUCCESS;
	}

	int  index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_HYBRID_AUTH);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"hybrid auth %s successfully\n",argv[0]);
		else if (ret == ASD_SECURITY_NOT_EXIST) 		
			vty_out(vty,"<error> security profile does not exist.\n");			
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

//mahz add 2011.7.8 for Fast Authentication Control in security about radius
DEFUN(config_fast_auth_cmd_func,
	  config_fast_auth_cmd,
	  "fast auth (enable|disable)",
	  "fast auth.\n"
	  "enable or disable fast auth, default enable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned int type = 1;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else {		
		vty_out(vty,"<error> unknown security type.\n");
		return CMD_SUCCESS;
	}

	int  index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = (int)vty->index;			
		security_id = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_FAST_AUTH);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"fast auth %s successfully\n",argv[0]);
	else if (ret == ASD_SECURITY_NOT_EXIST) 		
		vty_out(vty,"<error> security profile does not exist.\n");			
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> this security profile is used by some wlans,please disable them first\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//mahz add 2011.3.17
DEFUN(show_asd_global_variable_cmd_func,
	  show_asd_global_variable_cmd,
	  "show asd global variable",
	  "global variable\n"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int  asd_notice_sta_info_to_portal;
	int  asd_notice_sta_info_to_portal_timer;
	int  wtp_send_response_to_mobile;
	int  asd_dbus_count_switch;
	unsigned int  sta_static_fdb_able;
	unsigned int  asd_switch;
	unsigned char  asd_station_arp_listen;
	unsigned char  asd_station_static_arp;
	unsigned int  asd_sta_idle_time = 0;
	unsigned char asd_sta_idle_time_switch = 0;
	unsigned int asd_bak_sta_update_time =0;
	unsigned char  asd_ipset_switch = 0;
	unsigned char asd_getip_from_dhcpsnp = 0;
	unsigned char asd_syslog_debug = 0;
	unsigned char asd_radius_format = 0;
	int  index = 0;
	int  localid = 1;
	int  slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_ASD_GLOBAL_VARIABLE);

	dbus_error_init(&err);

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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_notice_sta_info_to_portal);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_notice_sta_info_to_portal_timer);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtp_send_response_to_mobile);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_dbus_count_switch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sta_static_fdb_able);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_switch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_station_arp_listen);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_station_static_arp);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_sta_idle_time_switch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_sta_idle_time);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_bak_sta_update_time);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_ipset_switch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_getip_from_dhcpsnp);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_syslog_debug);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&asd_radius_format);		

		vty_out(vty,"================================================================================\n");
		vty_out(vty,"ASD global variable list summary\n");
		vty_out(vty,"======================================================\n");
				vty_out(vty,"asd notice sta info to portal timer:	  %ds\n",asd_notice_sta_info_to_portal_timer);
				vty_out(vty,"asd notice sta info to portal:           %s\n",(asd_notice_sta_info_to_portal == 1)?"enable":"disable");
				vty_out(vty,"wtp send response to mobile:             %s\n",(wtp_send_response_to_mobile == 1)?"enable":"disable");
				vty_out(vty,"sta static fdb:                          %s\n",(sta_static_fdb_able == 1)?"enable":"disable");
				vty_out(vty,"asd switch:                              %s\n",(asd_switch == 1)?"enable":"disable");
				vty_out(vty,"dbus count switch:                       %s\n",(asd_dbus_count_switch == 1)?"enable":"disable");
				vty_out(vty,"asd sta static arp:                      %s\n",(asd_station_static_arp == 1)?"enable":"disable");
				if(asd_station_arp_listen == 0)
				    vty_out(vty,"asd sta arp listen:                      %s\n","disable");
				else if(asd_station_arp_listen == 1)
				    vty_out(vty,"asd sta arp listen:                      %s\n","enable");
				else if(asd_station_arp_listen == 2)
				    vty_out(vty,"asd sta arp listen and set:              %s\n","enable");
				vty_out(vty,"asd sta idle time switch:                %s\n",(asd_sta_idle_time_switch == 1)?"enable":"disable");
				vty_out(vty,"asd sta idle time:                        %dh\n",asd_sta_idle_time);
				vty_out(vty,"asd bak sta update time:                 %dmin\n",asd_bak_sta_update_time);
				vty_out(vty,"asd ipset switch:                        %s\n",(asd_ipset_switch == 1)?"enable":"disable");
				vty_out(vty,"asd get ip from dhcp-snooping:           %s\n",(asd_getip_from_dhcpsnp == 1)?"enable":"disable");
				vty_out(vty,"asd log format(bit0:hn-mobile bit1:mobile):   0x%x\n",asd_syslog_debug);
				vty_out(vty,"asd radius format(0:default 1:indonesia):   %d\n",asd_radius_format);
		vty_out(vty,"================================================================================\n");
	}
	
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}


/*nl 091119*/
DEFUN(set_notice_sta_info_to_portal_open_cmd_func,
	  set_notice_sta_info_to_portal_open_cmd,
	  "set notice sta info to portal (enable|disable)",
	  "asd send notice station info to portal\n"
	  "default disable\n"
	 )
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char asd_send_notice_sta_info_to_proto=0;
	if(!strcmp(argv[0],"enable")){
		asd_send_notice_sta_info_to_proto=1;
	}
	else if(!strcmp(argv[0],"disable")){
	asd_send_notice_sta_info_to_proto=0;
	}
		else 
		{vty_out(vty,"<error> input should be enable or disable.\n");
		return CMD_SUCCESS;
		}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_NOTICE_STA_INFO_TO_PROTO_OPEN);*/
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_NOTICE_STA_INFO_TO_PROTO_OPEN);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&asd_send_notice_sta_info_to_proto,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"notice sta info to portal set successfully.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}

DEFUN(set_notice_sta_info_to_portal_timer_cmd_func,
	  set_notice_sta_info_to_portal_timer_cmd,
	  "set notice sta info to portal timer <0-60>",
	  "asd send notice station info to portal timer\n"
	  "default 10s\n"
	 )
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned int timer = 10;
	timer = atoi(argv[0]);
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_NOTICE_STA_INFO_TO_PORTAL_TIMER);

/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_NOTICE_STA_INFO_TO_PORTAL_TIMER);
*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&timer,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"notice sta info to portal timer set successfully.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_wapipreauth_cmd_func,
	  set_wapi_sub_attr_wapipreauth_cmd,
	  "set wapi wapipreauth (enable|disable)",
	  "Configuring service.\n"
	  "wapi_sub_attr_WapiPreauth\n"
	  "default disable\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char WapiPreauth=0;

	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		WapiPreauth = 1;			
	else if ((!strcmp(argv[0],"disable")))
		WapiPreauth = 0;	
	else 
	{		
		vty_out(vty,"<error> WapiPreauth should be enable or disable.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE);
	

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&WapiPreauth,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security WapiPreauth set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set WapiPreauth under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_multicaserekeystrict_cmd_func,
	  set_wapi_sub_attr_multicaserekeystrict_cmd,
	  "set wapi multicaserekeystrict (enable|disable)",
	  "Configuring service.\n"
	  "wapi_sub_attr_MulticaseRekeyStrict\n"
	  "default disable\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char MulticaseRekeyStrict=0;

	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		MulticaseRekeyStrict = 1;			
	else if ((!strcmp(argv[0],"disable")))
		MulticaseRekeyStrict = 0;	
	else 
	{		
		vty_out(vty,"<error> MulticaseRekeyStrict should be enable or disable.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MUTICASEREKEYSTRICT_UPDATE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&MulticaseRekeyStrict,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security MulticaseRekeyStrict set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set MulticaseRekeyStrict under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}



/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_unicastcipherenabled_cmd_func,
	  set_wapi_sub_attr_unicastcipherenabled_cmd,
	  "set wapi unicastcipherenabled (enable|disable)",
	  "Configuring service.\n"
	  "wapi_sub_attr_UnicastCipherEnabled\n"
	  "default enable\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char UnicastCipherEnabled=0;

	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		UnicastCipherEnabled = 1;			
	else if ((!strcmp(argv[0],"disable")))
		UnicastCipherEnabled = 0;	
	else 
	{		
		vty_out(vty,"<error> UnicastCipherEnabled should be enable or disable.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICASTCIPHERENABLED_UPDATE);//qiuchen change it 2012.11.22

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&UnicastCipherEnabled,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security UnicastCipherEnabled set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set UnicastCipherEnabled under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}





/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_authenticationsuiteenable_cmd_func,
	  set_wapi_sub_attr_authenticationsuiteenable_cmd,
	  "set wapi authenticationsuiteenable (enable|disable)",
	  "Configuring service.\n"
	  "wapi_sub_attr_authenticationsuiteenable\n"
	  "default enable\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char AuthenticationSuiteEnable=0;

	//security_id = (int)vty->index;
	if ((!strcmp(argv[0],"enable")))
		AuthenticationSuiteEnable = 1;			
	else if ((!strcmp(argv[0],"disable")))
		AuthenticationSuiteEnable = 0;	
	else 
	{		
		vty_out(vty,"<error> AuthenticationSuiteEnable should be enable or disable.\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_AUTHENTICATIONSUITEENABLE_UPDATE);//qiuchen change it 2012.11.22


	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&AuthenticationSuiteEnable,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security AuthenticationSuiteEnable set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set AuthenticationSuiteEnable under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_certificateupdatecount_cmd_func,
	  set_wapi_sub_attr_certificateupdatecount_cmd,
	  "set wapi certificateupdatecount <0-64>",
	  "Configuring service.\n"
	  "wapi_sub_attr_CertificateUpdateCount\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int CertificateUpdateCount=0;

	//security_id = (int)vty->index;
	CertificateUpdateCount= atoi(argv[0]);

	if(CertificateUpdateCount<0||CertificateUpdateCount>64)
		{
			vty_out(vty,"<error> input retry value should be 0 to 64.\n");
			return CMD_SUCCESS;
		}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_CERTIFICATE_UPDATE);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_CERTIFICATE_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&CertificateUpdateCount,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security CertificateUpdateCount set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set CertificateUpdateCount under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_multicastupdatecount_cmd_func,
	  set_wapi_sub_attr_multicastupdatecount_cmd,
	  "set wapi multicastupdatecount <0-64>",
	  "Configuring service.\n"
	  "wapi_sub_attr_MulticastUpdateCount\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int MulticastUpdateCount=0;

	security_id = (int)vty->index;
	MulticastUpdateCount= atoi(argv[0]);

	if(MulticastUpdateCount<0||MulticastUpdateCount>64)
		{
			vty_out(vty,"<error> input retry value should be 0 to 64.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_UPDATE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&MulticastUpdateCount,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security MulticastUpdateCount set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set MulticastUpdateCount under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_unicastupdatecount_cmd_func,
	  set_wapi_sub_attr_unicastupdatecount_cmd,
	  "set wapi unicastupdatecount <0-64>",
	  "Configuring service.\n"
	  "wapi_sub_attr_UnicastUpdateCount\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int UnicastUpdateCount=0;

	security_id = (int)vty->index;
	UnicastUpdateCount= atoi(argv[0]);

	if(UnicastUpdateCount<0||UnicastUpdateCount>64)
		{
			vty_out(vty,"<error> input  value should be 0 to 64.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICAST_COUNT_UPDATE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICAST_COUNT_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&UnicastUpdateCount,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security UnicastUpdateCount set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set UnicastUpdateCount under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_bklifetime_cmd_func,
	  set_wapi_sub_attr_bklifetime_cmd,
	  "set wapi bklifetime <0-86400>",
	  "Configuring service.\n"
	  "wapi_sub_attr_UBKLifetime\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int BKLifetime=0;

	security_id = (int)vty->index;
	BKLifetime= atoi(argv[0]);

	if(BKLifetime<0||BKLifetime>86400)
		{
			vty_out(vty,"<error> input  value should be 0 to 86400.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKLIFETIME_UPDATE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKLIFETIME_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&BKLifetime,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security BKLifetime set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set BKLifetime under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}




/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_bkreauththreshold_cmd_func,
	  set_wapi_sub_attr_bkreauththreshold_cmd,
	  "set wapi bkreauththreshold <0-99>",
	  "Configuring service.\n"
	  "wapi_sub_attr_bkreauththreshold\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int BKReauthThreshold=0;

	//security_id = (int)vty->index;
	BKReauthThreshold= atoi(argv[0]);

	if(BKReauthThreshold<0||BKReauthThreshold>99)
		{
			vty_out(vty,"<error> input  value should be 0 to 99.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKREAUTH_THREASHOLD_UPDATE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKREAUTH_THREASHOLD_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&BKReauthThreshold,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security BKReauthThreshold set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set BKReauthThreshold under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


/*nl add 09/11/02*/
DEFUN(set_wapi_sub_attr_satimeout_cmd_func,
	  set_wapi_sub_attr_satimeout_cmd,
	  "set wapi satimeout <0-120>",
	  "Configuring service.\n"
	  "wapi_sub_attr_SATimeout\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int SATimeout=0;

	//security_id = (int)vty->index;
	SATimeout= atoi(argv[0]);

	if(SATimeout<0||SATimeout>120)
		{
			vty_out(vty,"<error> input retry value should be 0 to 120.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_SA_TIMEOUT_UPDATE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_SA_TIMEOUT_UPDATE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&SATimeout,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security SATimeout set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set SATimeout under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
DEFUN(set_wapi_sub_attr_multicast_cipher_cmd_func,
	  set_wapi_sub_attr_multicast_cipher_cmd,
	  "set wapi multicast cipher 00-14-72- <1-255>",
	  "Configuring service.\n"
	  "wapi_sub_attr_multicast_sipher\n"
	  )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	unsigned char type = 0;

	type = atoi(argv[0]);

	if(type<1 || type>255)
	{
			vty_out(vty,"<error> input retry value should be 1 to 255.\n");
			return CMD_SUCCESS;
	}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_CIPHER);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&type,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security multicast sipher set successfully\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set multicast sipher under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
/*nl add 20100307*/
DEFUN(set_security_wep_index_func,
	  set_security_wep_index_cmd,
	  "set security index <1-4>",
	  "index\n"
	  "security index\n"
	  "security index\n"
	 )
{
	int ret,ret1;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char security_index=0;

	security_id = (int)vty->index;
	ret1 = parse_char_ID((char*)argv[0], &security_index);
	
	if(ret1 != WID_DBUS_SUCCESS){
            if(ret1 == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}
	//security_index= atoi(argv[0]);

	if(security_index<1||security_index>4)
		{
			vty_out(vty,"<error> input security index should be 0 to 3.\n");
			return CMD_SUCCESS;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WEP_INDEX_PERIOD);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_BYTE,&security_index,
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

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"security index set successfully\n");
		else if(ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error> security profile does not exist.\n");	
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	 	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
			vty_out(vty,"<error> the encryption type of the security should be wep.\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
		
		dbus_message_unref(reply);
				
		return CMD_SUCCESS;

}

#if DCLI_NEW
DEFUN(show_security_wapi_info_cmd_func,
	  show_security_wapi_info_cmd,
	  "show security SECURITYID wapi info",	  
	  "ID security id\n"
	  "Display security wapi information\n"
	  
	 )
{
	 		
	struct dcli_security	*sec = NULL;
	unsigned char 	security_id;	
	int ret;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;

	ret = parse_char_ID((char*)argv[0], &security_id);
	
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	sec = show_security_wapi_info(dcli_dbus_connection, index, security_id, localid, &ret);	
					
	if((ret == 0 ) && (sec != NULL)){
				
		vty_out(vty,"==============================================================================\n");		
		vty_out(vty,"CertificateUpdateCount:%15d\n",sec->CertificateUpdateCount);		
		vty_out(vty,"MulticastUpdateCount:  %15d\n",sec->MulticastUpdateCount);
		vty_out(vty,"UnicastUpdateCount:    %15d\n",sec->UnicastUpdateCount);

		
		vty_out(vty,"BKLifetime:            %15d\n",sec->BKLifetime);
		vty_out(vty,"BKReauthThreshold:     %15d\n",sec->BKReauthThreshold);
		vty_out(vty,"SATimeout:             %15d\n",sec->SATimeout);

		if(sec->WapiPreauth==0)
		vty_out(vty,"WapiPreauth:                   disable\n");
		else if(sec->WapiPreauth==1)
		vty_out(vty,"WapiPreauth:                    enable\n");

		if(sec->MulticaseRekeyStrict==0)
		vty_out(vty,"MulticaseRekeyStrict:          disable\n");
		else if(sec->MulticaseRekeyStrict==1)
		vty_out(vty,"MulticaseRekeyStrict:           enable\n");
		
		if(sec->UnicastCipherEnabled==0)
		vty_out(vty,"UnicastCipherEnabled:          disable\n");
		else if(sec->UnicastCipherEnabled==1) 
		vty_out(vty,"UnicastCipherEnabled:           enable\n");
		
		if(sec->AuthenticationSuiteEnable==0)
		vty_out(vty,"AuthenticationSuiteEnable:     disable\n");
		else if(sec->AuthenticationSuiteEnable==1)
		vty_out(vty,"AuthenticationSuiteEnable:      enable\n");
		vty_out(vty,"MulticastCipher:           %02x-%02x-%02x-%02x\n",sec->MulticastCipher[0],sec->MulticastCipher[1],sec->MulticastCipher[2],sec->MulticastCipher[3]);
		vty_out(vty,"==============================================================================\n");
				
		}
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> security id does not exist\n");
	else if (ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)
		vty_out(vty,"<error> security id should be wapi_psk or wapi_auth\n");
	else if (ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> security's wapi config  is not intergrity\n");		
	else if (ret == ASD_DBUS_ERROR)		
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dcli_free_security(sec);	
	return CMD_SUCCESS;
		
}

#else
/*nl add /09/11/03 for wapi*/
DEFUN(show_security_wapi_info_cmd_func,
	  show_security_wapi_info_cmd,
	  "show security SECURITYID wapi info",	  
	  "ID security id\n"
	  "Display security wapi information\n"
	  
	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;		
	int ret;
	
	unsigned char security_id;
	unsigned int CertificateUpdateCount;
	unsigned int MulticastUpdateCount;
	unsigned int UnicastUpdateCount;
	unsigned int BKLifetime;
	unsigned int BKReauthThreshold;
	unsigned int SATimeout;
	
	unsigned char WapiPreauth;			/*default 0,no*/
	unsigned char MulticaseRekeyStrict;   /*default 0,no*/
	unsigned char UnicastCipherEnabled;			/*default 1,yes*/
	unsigned char AuthenticationSuiteEnable;	 /*default 1,yes*/
	unsigned char MulticastCipher[4];	 /*default 00-14-72-01*/

	ret = parse_char_ID((char*)argv[0], &security_id);
	
	if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
		vty_out(vty,"<error> unknown id format\n");
				}
		return CMD_SUCCESS;
	}	
	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO);
	
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 ){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&CertificateUpdateCount);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&MulticastUpdateCount);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&UnicastUpdateCount);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&BKLifetime);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&BKReauthThreshold);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&SATimeout);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&WapiPreauth);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&MulticaseRekeyStrict);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&UnicastCipherEnabled);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&AuthenticationSuiteEnable);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&MulticastCipher[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&MulticastCipher[1]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&MulticastCipher[2]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&MulticastCipher[3]);
		
		vty_out(vty,"==============================================================================\n");		
		vty_out(vty,"CertificateUpdateCount:%15d\n",CertificateUpdateCount);		
		vty_out(vty,"MulticastUpdateCount:  %15d\n",MulticastUpdateCount);
		vty_out(vty,"UnicastUpdateCount:    %15d\n",UnicastUpdateCount);

		
		vty_out(vty,"BKLifetime:            %15d\n",BKLifetime);
		vty_out(vty,"BKReauthThreshold:     %15d\n",BKReauthThreshold);
		vty_out(vty,"SATimeout:             %15d\n",SATimeout);

		if(WapiPreauth==0)
		vty_out(vty,"WapiPreauth:                   disable\n");
		else if(WapiPreauth==1)
		vty_out(vty,"WapiPreauth:                    enable\n");

		if(MulticaseRekeyStrict==0)
		vty_out(vty,"MulticaseRekeyStrict:          disable\n");
		else if(MulticaseRekeyStrict==1)
		vty_out(vty,"MulticaseRekeyStrict:           enable\n");
		
		if(UnicastCipherEnabled==0)
		vty_out(vty,"UnicastCipherEnabled:          disable\n");
		else if(UnicastCipherEnabled==1) 
		vty_out(vty,"UnicastCipherEnabled:           enable\n");
		
		if(AuthenticationSuiteEnable==0)
		vty_out(vty,"AuthenticationSuiteEnable:     disable\n");
		else if(AuthenticationSuiteEnable==1)
		vty_out(vty,"AuthenticationSuiteEnable:      enable\n");
		vty_out(vty,"MulticastCipher:           %02x-%02x-%02x-%02x\n",MulticastCipher[0],MulticastCipher[1],MulticastCipher[2],MulticastCipher[3]);
		vty_out(vty,"==============================================================================\n");
		
		
		}
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> security id does not exist\n");
	else if (ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)
		vty_out(vty,"<error> security id should be wapi_psk or wapi_auth\n");
	else if (ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> security's wapi config  is not intergrity\n");
	
	
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
		
}
#endif

/*ht add ,091111*/
DEFUN(config_asd_get_sta_info_able_func,
	  config_asd_get_sta_info_able_cmd,
	  "asd get sta info (enable|disable)",
	  "set asd get sta info from wsm\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_ASD_GET_STA_INFO_ABLE);
	
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_ASD_GET_STA_INFO_ABLE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"asd get sta info %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*ht add ,091111*/
DEFUN(config_asd_get_sta_info_time_func,
	  config_asd_get_sta_info_time_cmd,
	  "set asd get sta info time <5-3600>",
	  "set asd get sta info time\n"
	  "default 10"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int period=20;

	period= atoi(argv[0]);

	if(period<5 || period>3600){
		vty_out(vty,"<error> input time value should be 5 to 3600.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_ASD_GET_STA_INFO_TIME);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_ASD_GET_STA_INFO_TIME); */
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&period,
						 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd get sta info time %d successfully\n",period);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}

/*ht add ,091130*/
DEFUN(config_wapi_multi_cert_func,
	  config_wapi_multi_cert_cmd,
	  "wapi multi cert (enable|disable)",
	  "config wapi use multi cert\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id;
	unsigned char type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_MULTI_CERT);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_MULTI_CERT);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"wapi multi cert %s successfully\n",argv[0]);
	else if(ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)			
		vty_out(vty,"<error> security type which you chose does not support wapi authentication.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)			
		vty_out(vty,"<error> this security profile be used by some wlans,please disable them first\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*ht add ,091111*/
DEFUN(config_asd_ieee_80211n_able_func,
	  config_asd_ieee_80211n_able_cmd,
	  "asd process 80211n (enable|disable)",
	  "set asd process 80211n message\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_ASD_PROCESS_80211N_ABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"asd get sta info %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*ht add ,100618*/
DEFUN(config_asd_set_sta_static_fdb_able_func,
	  config_asd_set_sta_static_fdb_able_cmd,
	  "set sta static fdb (enable|disable)",
	  "set sta static fdb switch\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_STA_STATIC_FDB_ABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set sta static fdb %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


#if 1
/*nl add /09/12/14 for wapi*/
DEFUN(show_wlanid_security_wapi_config_cmd_func,
	  show_wlanid_security_wapi_config_cmd,
	  "show wlan ID wapi config",	  
	  "ID  wlanid\n"
	  "Display wlan wapi information\n"
	  
	 )
{
	 		
	struct dcli_security	*sec = NULL;	
	int ret;
	unsigned char wlan_id;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);	
	
	sec = show_wlan_security_wapi_info(dcli_dbus_connection, index, wlan_id, localid, &ret);

	if((ret == 0) && (sec != NULL))
	{
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"SecurityID:            %15d\t\t",sec->SecurityID);
		vty_out(vty,"CertificateUpdateCount:%15d\n",sec->CertificateUpdateCount);		
		vty_out(vty,"MulticastUpdateCount:  %15d\t\t",sec->MulticastUpdateCount);
		vty_out(vty,"UnicastUpdateCount:    %15d\n",sec->UnicastUpdateCount);

		vty_out(vty,"BKLifetime:            %15d\t\t",sec->BKLifetime);
		vty_out(vty,"BKReauthThreshold:     %15d\n",sec->BKReauthThreshold);
		vty_out(vty,"SATimeout:             %15d\t\t",sec->SATimeout);
		
		if(sec->MulticaseRekeyStrict==0)
			vty_out(vty,"MulticaseRekeyStrict:          disable\n");
		else if(sec->MulticaseRekeyStrict==1)
			vty_out(vty,"MulticaseRekeyStrict:          enable\n");
		
		if(sec->UnicastCipherEnabled==0)
			vty_out(vty,"UnicastCipherEnabled:          disable\t\t");
		else if(sec->UnicastCipherEnabled==1) 
			vty_out(vty,"UnicastCipherEnabled:          enable\t\t");

		if(sec->AuthenticationSuiteEnable==0)
			vty_out(vty,"AuthenticationSuiteEnabled:    disable\n");
		else if(sec->AuthenticationSuiteEnable==1) 
			vty_out(vty,"AuthenticationSuiteEnabled:    enable\n\n");
					
		vty_out(vty,"IsInstalledCer:     	      %s\t\t\t",(sec->IsInstalledCer==1) ? "YES" :"NO" );	
		vty_out(vty,"WAPIASIPAddress:  	 	 %s:%d\n",sec->wapi_as.as_ip,3810);	
		vty_out(vty,"ControlledPortControl:         Auto\t\t");
		
		if (sec->ControlledAuthControl == 1){
			vty_out(vty,"ControlledAuthControl              :     YES\n");		
		}
		else if (sec->ControlledAuthControl== 0){
			vty_out(vty,"ControlledAuthControl              :     NO\n");		
		}

		vty_out(vty,"WAPI UNICAST REKEY METHOD :   %11s\t",sec->RekeyMethod);
		if((sec->wapi_ucast_rekey_method==1)||(sec->wapi_ucast_rekey_method==3)){
			vty_out(vty,"WAPI UNICAST REKEY TIME BASED PARA :   %5u\n",sec->wapi_ucast_rekey_para_t);		
		}
		if((sec->wapi_ucast_rekey_method==2)||(sec->wapi_ucast_rekey_method==3)){
			vty_out(vty,"WAPI UNICAST REKEY PACKET BASED PARA : %5u\n",sec->wapi_ucast_rekey_para_p);			
		}
		
		vty_out(vty,"WAPI MULTICAST REKEY METHOD : %11s\t",sec->RekeyMethod);
		if((sec->wapi_mcast_rekey_method==1)||(sec->wapi_mcast_rekey_method==3)){
			vty_out(vty,"WAPI MULTICAST REKEY TIME BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_t);
		}
		
		if((sec->wapi_mcast_rekey_method==2)||(sec->wapi_mcast_rekey_method==3)){
			vty_out(vty,"WAPI MULTICAST REKEY PACKET BASED PARA : %5u\n",sec->wapi_mcast_rekey_para_p);
		}
		
		vty_out(vty,"\nSupported Latest Version: %6u\t\t",sec->ConfigVersion);	
		vty_out(vty,"UnicastKeysSupported: 	        %u\n",sec->UnicastKeysSupported);	
		vty_out(vty,"WapiSupported:     %15s\t\t",((sec->WapiSupported==1)?"Yes":"No"));			
		vty_out(vty,"WapiEnabled:                    %s\n",((sec->WapiSupported==1)?"Yes":"No"));
		
		if(sec->WapiPreauth==0)
			vty_out(vty,"WapiPreauth:                   disable\t\t");
		else if(sec->WapiPreauth==1)
			vty_out(vty,"WapiPreauth:                   enable\n\t\t");
		if(sec->WapiPreauth==0)
			vty_out(vty,"WapiPreauthEnabled:             disable\n");
		else if(sec->WapiPreauth==1)
			vty_out(vty,"WapiPreauthEnabled:             enable\n");
							
		vty_out(vty,"MulticastRekeyPackets:	%11u\t\t",sec->MulticastRekeyPackets);
		vty_out(vty,"UnicastRekeyPackets: 		%u\n",sec->UnicastRekeyPackets);
		vty_out(vty,"MulticastRekeyStrict:          NO\n");	
		/*
		vty_out(vty,"PSKValue:\t\t%16s\t",sec->SecurityKey);	
		vty_out(vty,"PSKPassPhrase: \t\t%17s\n",sec->SecurityKey);	*/
		
		vty_out(vty,"PSKValue:\t\t%16s\t",sec->SecurityKey);	
		vty_out(vty,"PSKPassPhrase: \t\t%17s\n",sec->SecurityKey);	
		
		vty_out(vty,"MulticastCipherSize:%14d\t\t",sec->MulticastCipherSize);			
		vty_out(vty,"MulticastCipher: \t\t%02X-%02X-%02X-%02X\n",sec->MulticastCipher[0],sec->MulticastCipher[1],sec->MulticastCipher[2],sec->MulticastCipher[3]);
		vty_out(vty,"UnicastCipherSize:  %14d\t\t",sec->UnicastCipherSize);			
		vty_out(vty,"UnicastCipher: \t\t\t%02X-%02X-%02X-%02X\n",sec->UnicastCipher[0],sec->UnicastCipher[1],sec->UnicastCipher[2],sec->UnicastCipher[3]);
		vty_out(vty,"AuthenticationSuite:           %02X-%02X-%02X-%02X\t",sec->AuthenticationSuite[0],sec->AuthenticationSuite[1],sec->AuthenticationSuite[2],sec->AuthenticationSuite[3]);	
		if (sec->SecurityType==WAPI_AUTH){
			vty_out(vty,"AuthSuitSelected:    %02X-%02X-%02X-%02X\n",sec->AuthSuitSelected_Auth[0],sec->AuthSuitSelected_Auth[1],sec->AuthSuitSelected_Auth[2],sec->AuthSuitSelected_Auth[3]);
		}
		else if (sec->SecurityType==WAPI_PSK){
			vty_out(vty,"AuthSuitSelected: \t\t%02X-%02X-%02X-%02X\n",sec->AuthSuitRequested_Psk[0],sec->AuthSuitRequested_Psk[1],sec->AuthSuitRequested_Psk[2],sec->AuthSuitRequested_Psk[3]);
		}
		vty_out(vty,"MulticastCipherSelected:       %02X-%02X-%02X-%02X\t",sec->MulticastCipherSelected[0],sec->MulticastCipherSelected[1],sec->MulticastCipherSelected[2],sec->MulticastCipherSelected[3]);								
		vty_out(vty,"MulticastCipherRequested: \t%02X-%02X-%02X-%02X\n",sec->MulticastCipherRequested[0],sec->MulticastCipherRequested[1],sec->MulticastCipherRequested[2],sec->MulticastCipherRequested[3]);		
		vty_out(vty,"UnicastCipherRequested:        %02X-%02X-%02X-%02X\t",sec->UnicastCipherRequested[0],sec->UnicastCipherRequested[1],sec->UnicastCipherRequested[2],sec->UnicastCipherRequested[3]);
		vty_out(vty,"UnicastCipherSelected:    \t%02X-%02X-%02X-%02X\n",sec->UnicastCipherSelected[0],sec->UnicastCipherSelected[1],sec->UnicastCipherSelected[2],sec->UnicastCipherSelected[3]);
		vty_out(vty,"==============================================================================\n");	
	}	
	
	else if (ret == ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if (ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)
		vty_out(vty,"<error> security id should be wapi_psk or wapi_auth\n");
	else if (ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> security's wapi config  is not intergrity\n");
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> wlan has not apply any security\n");
	else if (ret == ASD_DBUS_ERROR)		
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dcli_free_security(sec);	
	
	return CMD_SUCCESS;		
}

#else
/*nl add /09/12/14 for wapi*/
DEFUN(show_wlanid_security_wapi_config_cmd_func,
	  show_wlanid_security_wapi_config_cmd,
	  "show wlan ID wapi config",	  
	  "ID  wlanid\n"
	  "Display wlan wapi information\n"
	  
	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;		
	int ret;

	unsigned char SecurityID;
	unsigned int Stype, Etype;
	unsigned char wlan_id;
	char RekeyMethod[30];
	WAPI_AS wapi;
	char * key;
	int input_type=-1;

	unsigned char security_id;
	unsigned char IsInstalledCer;
	unsigned int CertificateUpdateCount;
	unsigned int MulticastUpdateCount;
	unsigned int UnicastUpdateCount;
	unsigned int BKLifetime;
	unsigned int BKReauthThreshold;
	unsigned int SATimeout;
	unsigned char WapiPreauth;			/*default 0,no*/
	unsigned char MulticaseRekeyStrict;   /*default 0,no*/
	unsigned char UnicastCipherEnabled;			/*default 1,yes*/
	unsigned char AuthenticationSuiteEnable;	 /*default 1,yes*/
	
	unsigned char wapi_ucast_rekey_method;
	unsigned int wapi_ucast_rekey_para_t;
 	unsigned int wapi_ucast_rekey_para_p;
 	unsigned char wapi_mcast_rekey_method;
 	unsigned int wapi_mcast_rekey_para_t;
 	unsigned int wapi_mcast_rekey_para_p;
	unsigned char ControlledAuthControl; 

	unsigned int  ConfigVersion = 1;
	unsigned char WapiSupported = 1;
	unsigned char WapiPreauthEnabled = 0;
	unsigned char UnicastKeysSupported = 2;
	unsigned char AuthenticationSuiteEnabled = 1;
	unsigned char MulticastRekeyStrict = 0;
	
	unsigned char UnicastCipherSelected[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned char MulticastCipherSelected[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned char UnicastCipherRequested[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned char MulticastCipherRequested[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned int  MulticastCipherSize = 256;
	unsigned char MulticastCipher[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned int  UnicastCipherSize = 512;
	unsigned char UnicastCipher[4] = {0x00, 0x14, 0x72, 0x01};

	unsigned char AuthSuitSelected_Auth[4] = {0x00, 0x14, 0x72, 0x01};	
	unsigned char AuthSuitRequested_Psk[4] = {0x00, 0x14, 0x72, 0x02};
    unsigned char AuthenticationSuite[4] = 	{0x00, 0x14, 0x72, 0x02};	
	unsigned int MulticastRekeyTime = 86400;	
	unsigned int UnicastRekeyTime= 86400;
	unsigned int MulticastRekeyPackets = 1000;	
	unsigned int UnicastRekeyPackets= 1000;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_WLAN_SECURITY_WAPI_CONF);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&wlan_id,
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
		DBUS_TYPE_UINT32,&CertificateUpdateCount,
		DBUS_TYPE_UINT32,&MulticastUpdateCount,
		DBUS_TYPE_UINT32,&UnicastUpdateCount,

		DBUS_TYPE_UINT32,&BKLifetime,
		DBUS_TYPE_UINT32,&BKReauthThreshold,
		DBUS_TYPE_UINT32,&SATimeout,	
					
		DBUS_TYPE_BYTE,&WapiPreauth,
		DBUS_TYPE_BYTE,&MulticaseRekeyStrict,
		DBUS_TYPE_BYTE,&UnicastCipherEnabled,
		DBUS_TYPE_BYTE,&AuthenticationSuiteEnable,
		DBUS_TYPE_BYTE,&MulticastCipher[0],
		DBUS_TYPE_BYTE,&MulticastCipher[1],
		DBUS_TYPE_BYTE,&MulticastCipher[2],
		DBUS_TYPE_BYTE,&MulticastCipher[3],
		DBUS_TYPE_STRING,&wapi.as_ip,

		DBUS_TYPE_BYTE,&wapi_ucast_rekey_method,	
		DBUS_TYPE_UINT32,&wapi_ucast_rekey_para_t,
		DBUS_TYPE_UINT32,&wapi_ucast_rekey_para_p,
		DBUS_TYPE_BYTE,&wapi_mcast_rekey_method,
		DBUS_TYPE_UINT32,&wapi_mcast_rekey_para_t,
		DBUS_TYPE_UINT32,&wapi_mcast_rekey_para_p,
		
		DBUS_TYPE_STRING,&(wapi.certification_path),
		DBUS_TYPE_STRING,&(wapi.ae_cert_path),
		DBUS_TYPE_UINT32,&(wapi.certification_type),
		DBUS_TYPE_UINT32,&Stype,
		DBUS_TYPE_UINT32,&Etype,
		DBUS_TYPE_BYTE,&SecurityID,
		DBUS_TYPE_STRING,&key,
		DBUS_TYPE_UINT32,&input_type,				
		DBUS_TYPE_INVALID)){
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"SecurityID:            %15d\t\t",SecurityID);
		vty_out(vty,"CertificateUpdateCount:%15d\n",CertificateUpdateCount);		
		vty_out(vty,"MulticastUpdateCount:  %15d\t\t",MulticastUpdateCount);
		vty_out(vty,"UnicastUpdateCount:    %15d\n",UnicastUpdateCount);

		vty_out(vty,"BKLifetime:            %15d\t\t",BKLifetime);
		vty_out(vty,"BKReauthThreshold:     %15d\n",BKReauthThreshold);
		vty_out(vty,"SATimeout:             %15d\t\t",SATimeout);
		
		if(MulticaseRekeyStrict==0)
			vty_out(vty,"MulticaseRekeyStrict:          disable\n");
		else if(MulticaseRekeyStrict==1)
			vty_out(vty,"MulticaseRekeyStrict:          enable\n");
		
		if(UnicastCipherEnabled==0)
			vty_out(vty,"UnicastCipherEnabled:          disable\t\t");
		else if(UnicastCipherEnabled==1) 
			vty_out(vty,"UnicastCipherEnabled:          enable\t\t");

		if(AuthenticationSuiteEnabled==0)
			vty_out(vty,"AuthenticationSuiteEnabled:    disable\n");
		else if(AuthenticationSuiteEnabled==1) 
			vty_out(vty,"AuthenticationSuiteEnabled:    enable\n\n");
				
		if(strcmp(wapi.ae_cert_path, " ") == 0){
			IsInstalledCer = 0;
			vty_out(vty,"IsInstalledCer:     	       NO\t\t");
		}
		else {	
			IsInstalledCer = 1;
			vty_out(vty,"IsInstalledCer:     	       YES\t\t");
		}
		vty_out(vty,"WAPIASIPAddress:  	 	 %s:%d\n",wapi.as_ip,3810);	
		vty_out(vty,"ControlledPortControl:         Auto\t\t");
		
		if (AuthenticationSuiteEnable == 1){
			ControlledAuthControl = 1;
			vty_out(vty,"ControlledAuthControl              :     YES\n");		
		}
		else if (AuthenticationSuiteEnable== 0){
			ControlledAuthControl = 0;
			vty_out(vty,"ControlledAuthControl              :     NO\n");		
		}

		CheckRekeyMethod(RekeyMethod,wapi_ucast_rekey_method);
		vty_out(vty,"WAPI UNICAST REKEY METHOD :   %11s\t",RekeyMethod);
		if((wapi_ucast_rekey_method==1)||(wapi_ucast_rekey_method==3)){
			vty_out(vty,"WAPI UNICAST REKEY TIME BASED PARA :   %5u\n",wapi_ucast_rekey_para_t);		
		}
		if((wapi_ucast_rekey_method==2)||(wapi_ucast_rekey_method==3)){
			vty_out(vty,"WAPI UNICAST REKEY PACKET BASED PARA : %5u\n",wapi_ucast_rekey_para_p);			
		}
		
		CheckRekeyMethod(RekeyMethod,wapi_mcast_rekey_method);
		vty_out(vty,"WAPI MULTICAST REKEY METHOD : %11s\t",RekeyMethod);
		if((wapi_mcast_rekey_method==1)||(wapi_mcast_rekey_method==3)){
			vty_out(vty,"WAPI MULTICAST REKEY TIME BASED PARA : %5u\n",wapi_mcast_rekey_para_t);
		}
		if((wapi_mcast_rekey_method==2)||(wapi_mcast_rekey_method==3)){
			vty_out(vty,"WAPI MULTICAST REKEY PACKET BASED PARA : %5u\n",wapi_mcast_rekey_para_p);
		}
		vty_out(vty,"\nSupported Latest Version: %6u\t\t",ConfigVersion);
		
		vty_out(vty,"UnicastKeysSupported: 	        %u\n",UnicastKeysSupported);	
		vty_out(vty,"WapiSupported:     %15s\t\t",((WapiSupported==1)?"Yes":"No"));			
		vty_out(vty,"WapiEnabled:                    %s\n",((WapiSupported==1)?"Yes":"No"));
		if(WapiPreauth==0)
			vty_out(vty,"WapiPreauth:                   disable\t\t");
		else if(WapiPreauth==1)
			vty_out(vty,"WapiPreauth:                   enable\n\t\t");
		if(WapiPreauth==0)
			vty_out(vty,"WapiPreauthEnabled:             disable\n");
		else if(WapiPreauth==1)
			vty_out(vty,"WapiPreauthEnabled:             enable\n");

						
		//vty_out(vty,"MulticastRekeyTime:  %15u\t\t",MulticastRekeyTime);
		//vty_out(vty,"UnicastRekeyTime: 	  	%u\n",UnicastRekeyTime);
		vty_out(vty,"MulticastRekeyPackets:	%11u\t\t",MulticastRekeyPackets);
		vty_out(vty,"UnicastRekeyPackets: 		%u\n",UnicastRekeyPackets);
		vty_out(vty,"MulticastRekeyStrict:          NO\n");				
		vty_out(vty,"PSKValue:\t\t%16s\t",key);	
		vty_out(vty,"PSKPassPhrase: \t\t%17s\n",key);				
		vty_out(vty,"MulticastCipherSize:%14d\t\t",MulticastCipherSize);			
		vty_out(vty,"MulticastCipher: \t\t%02X-%02X-%02X-%02X\n",MulticastCipher[0],MulticastCipher[1],MulticastCipher[2],MulticastCipher[3]);
		vty_out(vty,"UnicastCipherSize:  %14d\t\t",UnicastCipherSize);			
		vty_out(vty,"UnicastCipher: \t\t\t%02X-%02X-%02X-%02X\n",UnicastCipher[0],UnicastCipher[1],UnicastCipher[2],UnicastCipher[3]);
		vty_out(vty,"AuthenticationSuite:           %02X-%02X-%02X-%02X\t",AuthenticationSuite[0],AuthenticationSuite[1],AuthenticationSuite[2],AuthenticationSuite[3]);	
		if (Stype==WAPI_AUTH){
			vty_out(vty,"AuthSuitSelected:    %02X-%02X-%02X-%02X\n",AuthSuitSelected_Auth[0],AuthSuitSelected_Auth[1],AuthSuitSelected_Auth[2],AuthSuitSelected_Auth[3]);
		}
		else if (Stype==WAPI_PSK){
			vty_out(vty,"AuthSuitSelected: \t\t%02X-%02X-%02X-%02X\n",AuthSuitRequested_Psk[0],AuthSuitRequested_Psk[1],AuthSuitRequested_Psk[2],AuthSuitRequested_Psk[3]);
		}
		vty_out(vty,"MulticastCipherSelected:       %02X-%02X-%02X-%02X\t",MulticastCipherSelected[0],MulticastCipherSelected[1],MulticastCipherSelected[2],MulticastCipherSelected[3]);								
		vty_out(vty,"MulticastCipherRequested: \t%02X-%02X-%02X-%02X\n",MulticastCipherRequested[0],MulticastCipherRequested[1],MulticastCipherRequested[2],MulticastCipherRequested[3]);		
		vty_out(vty,"UnicastCipherRequested:        %02X-%02X-%02X-%02X\t",UnicastCipherRequested[0],UnicastCipherRequested[1],UnicastCipherRequested[2],UnicastCipherRequested[3]);
		vty_out(vty,"UnicastCipherSelected:    \t%02X-%02X-%02X-%02X\n",UnicastCipherSelected[0],UnicastCipherSelected[1],UnicastCipherSelected[2],UnicastCipherSelected[3]);
		vty_out(vty,"==============================================================================\n");	
	}	
	
	else if (ret == ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if (ret == ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH)
		vty_out(vty,"<error> security id should be wapi_psk or wapi_auth\n");
	else if (ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> security's wapi config  is not intergrity\n");
	else if (ret == ASD_SECURITY_NOT_EXIST)
		vty_out(vty,"<error> wlan has not apply any security\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
}
#endif


//mahz add for mib request , 2011.1.27 , dot11AKMConfigTable
DEFUN(show_security_wapi_config_of_all_wlan_cmd_func,
	  show_security_wapi_config_of_all_wlan_cmd,
	  "show security wapi config of all wlan",	  
	  "all wlan\n"
	  "Display wlan wapi information\n"
	 )
{
	 		
	struct dcli_security	*sec = NULL;
	struct dcli_security	*tmp = NULL;	
	int ret;
	unsigned int wlan_num;
	int i = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);	
	
	tmp = show_security_wapi_config_of_all_wlan(dcli_dbus_connection, index, &wlan_num , localid, &ret);

	if((ret == 0) && (tmp != NULL) && (wlan_num != 0)){
		sec = tmp;
		for(i=0;i<wlan_num;i++){			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WlanId:				%d\n",sec->SecurityID);
		
			if(sec->AuthenticationSuiteEnable==0)
				vty_out(vty,"AuthenticationSuiteEnabled:    disable\n");
			else if(sec->AuthenticationSuiteEnable==1) 
				vty_out(vty,"AuthenticationSuiteEnabled:    enable\n");
		
			vty_out(vty,"AuthenticationSuite:           %02X-%02X-%02X-%02X\n",sec->AuthenticationSuite[0],sec->AuthenticationSuite[1],sec->AuthenticationSuite[2],sec->AuthenticationSuite[3]);	
			vty_out(vty,"==============================================================================\n\n");	
			sec = sec->next;
		}	
	}
	else if (ret == ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> no wlan applied wapi security exist\n");
	else if (ret == ASD_DBUS_ERROR)		
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dcli_free_security_list(tmp);	
	
	return CMD_SUCCESS;		
}


/*ht add ,01.03.05*/
DEFUN(config_set_asd_trap_able_func,
	  config_set_asd_trap_able_cmd,
	  "set asd trap (wtp_deny_sta|sta_verify_failed|sta_assoc_failed|wapi_invalid_cert"
	  "|wapi_challenge_replay|wapi_mic_juggle|wapi_low_safe|wapi_addr_redirection) (enable|disable)",
	  "set asd asd trap switch\n"
	  "default disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type = 0;
	unsigned char flag = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	if ((!strcmp(argv[0],"wtp_deny_sta")))
		flag = 1;			
	else if ((!strcmp(argv[0],"sta_verify_failed")))
		flag = 2;	
	else if ((!strcmp(argv[0],"sta_assoc_failed")))
		flag = 3;	
	else if ((!strcmp(argv[0],"wapi_invalid_cert")))
		flag = 4;	
	else if ((!strcmp(argv[0],"wapi_challenge_replay")))
		flag = 5;	
	else if ((!strcmp(argv[0],"wapi_mic_juggle")))
		flag = 6;	
	else if ((!strcmp(argv[0],"wapi_low_safe")))
		flag = 7;	
	else if ((!strcmp(argv[0],"wapi_addr_redirection")))
		flag = 8;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		return CMD_SUCCESS;
	}

	if ((!strcmp(argv[1],"enable")))
		type = 1;			
	else if ((!strcmp(argv[1],"disable")))
		type = 0;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);	
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_TRAP_ABLE);
	//query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
	//					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_TRAP_ABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&flag,
							 DBUS_TYPE_BYTE,&type,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd trap %s %s successfully\n",argv[0],argv[1]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*ht add ,01.03.05*/
DEFUN(show_asd_trap_state_func,
	  show_asd_trap_state_cmd,
	  "show asd trap state [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "show trap state\n"
	  "asd trap state\n"
	  "asd trap state\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type[8] = {0};
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
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
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SHOW_TRAP_STATE);
		//query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
		//					ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SHOW_TRAP_STATE);
		dbus_error_init(&err);

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
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[0]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[1]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[2]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[3]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[4]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[5]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[6]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[7]));	

		if(ret == ASD_DBUS_SUCCESS){
			vty_out(vty,"asd trap state\n");
			vty_out(vty,"======================================================\n");
			vty_out(vty,"wtp_deny_sta:          %s\n",(type[0]==1)?"enable":"disable");
			vty_out(vty,"sta_verify_failed:     %s\n",(type[1]==1)?"enable":"disable");
			vty_out(vty,"sta_assoc_failed:      %s\n",(type[2]==1)?"enable":"disable");
			vty_out(vty,"wapi_invalid_cert:     %s\n",(type[3]==1)?"enable":"disable");
			vty_out(vty,"wapi_challenge_replay: %s\n",(type[4]==1)?"enable":"disable");
			vty_out(vty,"wapi_mic_juggle:       %s\n",(type[5]==1)?"enable":"disable");
			vty_out(vty,"wapi_low_safe:         %s\n",(type[6]==1)?"enable":"disable");
			vty_out(vty,"wapi_addr_redirection: %s\n",(type[7]==1)?"enable":"disable");
			vty_out(vty,"======================================================\n");
		}else
			vty_out(vty,"<error>  %d\n",ret);
		
		dbus_message_unref(reply);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				ReInitDbusPath_V2(localid,profile,ASD_DBUS_BUSNAME,BUSNAME);
				ReInitDbusPath_V2(localid,profile,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
				ReInitDbusPath_V2(localid,profile,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
				query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SHOW_TRAP_STATE);
				dbus_error_init(&err);

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
				
				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[0]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[1]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[2]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[3]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[4]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[5]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[6]));	

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(type[7]));	

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == ASD_DBUS_SUCCESS){
					vty_out(vty,"asd trap state\n");
					vty_out(vty,"======================================================\n");
					vty_out(vty,"wtp_deny_sta:          %s\n",(type[0]==1)?"enable":"disable");
					vty_out(vty,"sta_verify_failed:     %s\n",(type[1]==1)?"enable":"disable");
					vty_out(vty,"sta_assoc_failed:      %s\n",(type[2]==1)?"enable":"disable");
					vty_out(vty,"wapi_invalid_cert:     %s\n",(type[3]==1)?"enable":"disable");
					vty_out(vty,"wapi_challenge_replay: %s\n",(type[4]==1)?"enable":"disable");
					vty_out(vty,"wapi_mic_juggle:       %s\n",(type[5]==1)?"enable":"disable");
					vty_out(vty,"wapi_low_safe:         %s\n",(type[6]==1)?"enable":"disable");
					vty_out(vty,"wapi_addr_redirection: %s\n",(type[7]==1)?"enable":"disable");
				}else
					vty_out(vty,"<error>  %d\n",ret);
				
				dbus_message_unref(reply);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
		local_hansi_parameter:
			ReInitDbusPath_V2(localid,profile,ASD_DBUS_BUSNAME,BUSNAME);
			ReInitDbusPath_V2(localid,profile,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,profile,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SHOW_TRAP_STATE);
			dbus_error_init(&err);

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
			
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[0]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[1]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[2]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[3]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[4]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[5]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[6]));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(type[7]));	

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if(ret == ASD_DBUS_SUCCESS){
				vty_out(vty,"asd trap state\n");
				vty_out(vty,"======================================================\n");
				vty_out(vty,"wtp_deny_sta:          %s\n",(type[0]==1)?"enable":"disable");
				vty_out(vty,"sta_verify_failed:     %s\n",(type[1]==1)?"enable":"disable");
				vty_out(vty,"sta_assoc_failed:      %s\n",(type[2]==1)?"enable":"disable");
				vty_out(vty,"wapi_invalid_cert:     %s\n",(type[3]==1)?"enable":"disable");
				vty_out(vty,"wapi_challenge_replay: %s\n",(type[4]==1)?"enable":"disable");
				vty_out(vty,"wapi_mic_juggle:       %s\n",(type[5]==1)?"enable":"disable");
				vty_out(vty,"wapi_low_safe:         %s\n",(type[6]==1)?"enable":"disable");
				vty_out(vty,"wapi_addr_redirection: %s\n",(type[7]==1)?"enable":"disable");
			}else
				vty_out(vty,"<error>  %d\n",ret);
			
			dbus_message_unref(reply);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_wpa_group_rekey_period_cmd_func,
	  set_wpa_group_rekey_period_cmd,
	  "set wpa group rekey period <0-86400>",
	  "wpa group rekey period\n"
	  "wpa group rekey time\n"
	  "wpa group rekey period\n"
	  "period value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int period=3600;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	period= atoi(argv[0]);
	if(period<0||period>86400)
	{
		vty_out(vty,"<error> input period value should be 0 to 86400.\n");
		return CMD_SUCCESS;
	}

	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WPA_GROUP_REKEY_PERIOD);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&period,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set wpa group rekey period %s successfully\n",argv[0]);
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set wpa group rekey period under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}

//mahz add 2011.1.3 
//用于主备切换之后，设置进行组播密钥更新的时间

DEFUN(set_once_wpa_group_rekey_time_cmd_func,
	  set_once_wpa_group_rekey_time_cmd,
	  "set once wpa group rekey time <0-20>",
	  "wpa group rekey period\n"
	  "wpa group rekey time\n"
	  "time value\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int period=3600;
	int time = 6;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	time= atoi(argv[0]);
	if(time<0||time>20)
	{
		vty_out(vty,"<error> input time value should be 0 to 20.\n");
		return CMD_SUCCESS;
	}

	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WPA_ONCE_GROUP_REKEY_TIME);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&time,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set once wpa group rekey time %s successfully\n",argv[0]);
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set wpa once group rekey time under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}


DEFUN(set_wpa_keyupdate_timeout_period_cmd_func,
	  set_wpa_keyupdate_timeout_period_cmd,
	  "set wpa keyupdate timeout period <100-10000>",
	  "wpa keyupdate timeout period\n"
	  "wpa keyupdate timeout time\n"
	  "wpa keyupdate timeout period\n"
	  "period value,default 1000ms\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	int period=1000;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	period= atoi(argv[0]);

	if(period<100||period>10000)
	{
		vty_out(vty,"<error> input period value should be 100 to 10000.\n");
		return CMD_SUCCESS;
	}

	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_WPA_KEYUPDATE_TIMEOUT_PERIOD);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_UINT32,&period,
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

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set wpa keyupdate timeout period %s successfully\n",argv[0]);
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error> security profile does not exist.\n");	
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
		vty_out(vty,"<error> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set wpa keyupdate timeout period under current security type.\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}

//mahz add for test 2011.10.17
DEFUN(set_asd_socket_operate_cmd_func,
	  set_asd_socket_operate_cmd,
	  "set asd socket (TableSock|DataSock|TableSend|DataSend|NetLink) (close|start|restart)",
	  "asd recv sock SOCK\n"
	  "asd send sock SEND\n"
	  "operate on socket\n"
	 )
{
	int ret;
	unsigned char security_id;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	unsigned char sock,oper;
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if (!strcmp(argv[0],"TableSock"))
		sock = 1;			
	if (!strcmp(argv[0],"DataSock"))
		sock = 2;			
	if (!strcmp(argv[0],"TableSend"))
		sock = 3;			
	if (!strcmp(argv[0],"DataSend"))
		sock = 4;			
	if(!strcmp(argv[0],"NetLink"))
		sock = 5;
	if (!strcmp(argv[1],"close"))
		oper = 1;			
	if (!strcmp(argv[1],"start"))
		oper = 2;			
	if (!strcmp(argv[1],"restart"))
		oper = 3;			

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ASD_SOCKET_OPERATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&sock,
						 DBUS_TYPE_BYTE,&oper,
						 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply){
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd socket %s %s successfully\n",argv[0],argv[1]);
	else if(ret == ASD_SOCK_NOT_EXIST)
		vty_out(vty,"the socket dosen't exist or should be closed first\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
DEFUN(config_eap_sm_run_activated_cmd_func,
		config_eap_sm_run_activated_cmd,
		"set eap_extention_function (enable|disable)",
		"config eap extention function activated\n"
		"config eap extention function activated\n"
		"extention fuction enable\n"
		"extention fuction disable\n"
)
{
	unsigned int ret;
	unsigned char security_id;
	unsigned char type = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	if (!strcmp(argv[0],"enable"))
		type = 1;			
	else if (!strcmp(argv[0],"disable"))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown input parameter.\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_EAP_SM_RUN_ACTIVATED);

	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"Successfully set eap_sm_run_activated %s\n",argv[0]);
	else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)		
		vty_out(vty,"<error!> This Security Profile be used by some Wlans,please disable them first\n");
	else if(ret == ASD_SECURITY_NOT_EXIST)			
		vty_out(vty,"<error!> security profile does not exist.\n"); 
 	else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
		vty_out(vty,"<error> Can't set eap_extention_function activated under current security type.\n");
	else
		vty_out(vty,"<error!>%d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
//weichao add 
DEFUN(set_traffic_limit_radius_cmd_func,
	  set_traffic_limit_radius_cmd,
	  "traffic limit from radius (first|second)", 
	  "differentiate the accounting method.\n"
	  "limit\n"
	  "from\n"
	  "radius\n"
	  "default second"
	 )
{

		int ret = ASD_DBUS_SUCCESS;
		int  type = 0;
		unsigned char security_id = 0;	
		DBusMessage *query, *reply;	
		DBusMessageIter	 iter;
		DBusError err;

		if ((!strcmp(argv[0],"first")))
			type = 1;			
		else if ((!strcmp(argv[0],"second")))
			type = 0;	
		else 
		{		
			vty_out(vty,"<error> unknown encryption type.\n");
			return CMD_SUCCESS;
		}
		int index = 0;
		int localid = 1;
		int slot_id = HostSlotId;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == SECURITY_NODE){
			index = 0;
			security_id = (int)vty->index;	
		}else if(vty->node == HANSI_SECURITY_NODE){
			index = vty->index;  
			security_id = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if (vty->node == LOCAL_HANSI_NODE){
			index = vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
			index = vty->index;
			security_id = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_TRAFFIC_LIMIT_FROM_RADIUS);
		
		dbus_error_init(&err);
		
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&security_id,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		
		dbus_message_iter_get_basic(&iter,&ret);
		
		if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE) 		
			vty_out(vty,"<error!> This Security Profile be used by some Wlans,please disable them first\n");
		else if(ret == ASD_SECURITY_NOT_EXIST)			
			vty_out(vty,"<error!> security profile does not exist.\n");	
		else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE) 		
				vty_out(vty,"<error!> Can't set  account messge start after dchp under current security type.\n");
		else if(ret == ASD_ACCOUNT_AFTER_AUTHORIZE)
				vty_out(vty,"<error!> you have set account start after authorize,please disable it first.\n");
		else if(ret == ASD_DBUS_SUCCESS)
			{
				if(1==type)
					vty_out(vty,"traffic limit from radius first successfully!\n");
				
				else
					vty_out(vty,"traffic limit from radius second successfully!\n");
			}	
						
		else
			vty_out(vty,"<error!>%d\n",ret);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
//qiuchen add it for master_back radius server 2012.12.11
DEFUN(config_ac_radius_name_cmd_func,
		config_ac_radius_name_cmd,
		"set ac_radius_name RADIUSNAME",
		"config master radius server heartbeat msg user name.\n"
		"USER NAME\n"
		""
)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	char *name = NULL;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	if(strlen(argv[0]) > 253){
		vty_out(vty,"<error> the name is too long!\n");
		return CMD_SUCCESS;
	}
	name = malloc(strlen(argv[0])+1);
	if(name == NULL){
		vty_out(vty,"<error> malloc fail\n");
		return CMD_SUCCESS;
	}
	memset(name,0,strlen(argv[0])+1);
	memcpy(name,argv[0],strlen(argv[0]));

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AC_RADIUS_NAME);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(name){
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set ac_radius_name %s.\n",security_id,argv[0]);
	else if(ret == ASD_SECURITY_HEART_TEST_ON) 		
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	if(name){
		free(name);
		name = NULL;
	}
	return CMD_SUCCESS;
}
DEFUN(config_radius_res_fail_suc_percent_cmd_func,
		config_radius_res_fail_suc_percent_cmd,
		"set (radius_res_fail|radius_res_suc) percent PERCENT",
		"config the percentage to decide whether or not to change server\n"
		"above this change to the bak\n"
		"above this change to the master\n"
		"percentage\n"
)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	double percent;
	unsigned char type = 0;
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);

	
	if (!strncmp(argv[0],"radius_res_fail",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"radius_res_suc",strlen(argv[0]))){
		type = 2;
	} 

	ret = parse_double_PERCENT(argv[1],&percent);
	if(ret != WID_DBUS_SUCCESS){
        vty_out(vty,"<error> illegal input:the percentage should be like 0.9 or 90% \n");
		return CMD_SUCCESS;
	}	
	if(percent > 1 || percent <= 0){
        vty_out(vty,"<error> illegal input:the percentage shoule be between 0 and 1\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_RES_FAIL_SUC_PERCENT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_DOUBLE,&percent,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS){
		vty_out(vty,"security %d successfully set %s %s.\n",security_id,argv[0],argv[1]);
	}
	else if(ret == ASD_SECURITY_HEART_TEST_ON) 		
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}
DEFUN(config_radius_server_access_test_interval_cmd_func,
		config_radius_server_access_test_interval_cmd,
		"set radius_server access_test_interval INTERVAL",
		"config the heart test parameters\n"
		"config the heart test parameters\n"
		"heart test interval\n"
		"test interval\n"
)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	unsigned int inter = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	ret = parse_int_ID(argv[0],&inter);
	if(ret != WID_DBUS_SUCCESS){
        if(ret == WID_ILLEGAL_INPUT){
         	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
        }
		else{
			vty_out(vty,"<error> unknown id format\n");
		}
		return CMD_SUCCESS;
	}
	if(inter > 65535 || inter == 0){
		vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_ACCESS_TEST_INTERVAL);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&inter,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS){
		vty_out(vty,"security %d successfully set radius access test interval %s .\n",security_id,argv[0]);
	}
	else if(ret == ASD_SECURITY_HEART_TEST_ON) 		
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	
}
DEFUN(config_radius_server_change_test_timer_cmd_func,
		config_radius_server_change_test_timer_cmd,
		"set radius_server (change_test|reuse_test) timer TIMER",
		"config the heart test parameters\n"
		"config the heart test parameters\n"
		"master to bak test timer\n"
		"bak to master test timer\n"
		"TIMER\n"
)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	unsigned int time = 0;
	unsigned char type = 0;
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);

	
	if (!strncmp(argv[0],"change_test",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"reuse_test",strlen(argv[0]))){
		type = 2;
	} 

	ret = parse_int_ID(argv[1],&time);
	if(ret != WID_DBUS_SUCCESS){
        if(ret == WID_ILLEGAL_INPUT){
         	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
        }
		else{
			vty_out(vty,"<error> unknown id format\n");
		}
		return CMD_SUCCESS;
	}
	if(time > 65535 || time == 0){
		vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_CHANGE_REUSE_TEST_TIMER);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_UINT32,&time,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set %s timer %s.\n",security_id,argv[0],argv[1]);
	else if(ret == ASD_SECURITY_HEART_TEST_ON) 		
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}
DEFUN(config_radius_server_heart_test_type_cmd_func,
		config_radius_server_heart_test_type_cmd,
		"set radius_server heart_test_type (auth|acct|both)",
		"config the heart test parameters\n"
		"config the heart test parameters\n"
		"radius heart test type\n"
		"auth server test\n"
		"acct server test\n"
		"both servers test\n"

)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	unsigned char type = 0;
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);

	
	if (!strncmp(argv[0],"auth",strlen(argv[0]))){
		type = 0;
	}
	else if (!strncmp(argv[0],"acct",strlen(argv[0]))){
		type = 1;
	} 
	else if (!strncmp(argv[0],"both",strlen(argv[0]))){
		type = 2;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_HEART_TEST_TYPE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set radius server heart test type %s.\n",security_id,argv[0]);
	else if(ret == ASD_SECURITY_HEART_TEST_ON) 		
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else if(ret == ASD_DBUS_RADIUS_HEART_TEST_UNCHANGE)
		vty_out(vty,"<!> the radius server heart test type should not be both because of the same auth/acct ip addr\n");
	else if(ret == ASD_DBUS_RADIUS_BOTH_UNBINED)
		vty_out(vty,"Both test type should set the radius server binding disable!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
DEFUN(set_radius_heart_test_on_cmd_func,
		set_radius_heart_test_on_cmd,
		"set radius heart test (on|off)",
		"heart test switch\n"
		"heart test switch\n"
		"on\n"
		"off\n"
)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	unsigned char type = 0;
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);

	
	if (!strncmp(argv[0],"on",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"off",strlen(argv[0]))){
		type = 0;
	} 
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_HEART_TEST_ON_OFF);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set radius heart test %s.\n",security_id,argv[0]);
	else if(ret == ASD_SECURITY_RADIUSINIT_FAIL)
		vty_out(vty,"<error> Heart test radius init fail!\n");
	else if(ret == ASD_SECURITY_NONEED_RADIUS_TEST)
		vty_out(vty,"There is no bak server (either acct nor auth)!\n");
	else if(ret == ASD_SECURITY_TYPE_WITHOUT_8021X)
		vty_out(vty,"The security type is not right for this function!\n");
	else if(ret == ASD_SECURITY_HEART_TEST_TYPE_WRONG)
		vty_out(vty,"The heart test type with not integrity bak server configuration!\n");
	else if(ret == ASD_SECURITY_HEART_TEST_ON)
		vty_out(vty,"The radius heart test is already on!\n");
	else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> this security profile isn't integrity\n");
	else if(ret == ASD_SECURIT_HEART_PARAMETER_WRONG)
		vty_out(vty,"<error!>One of this:(change_test|reuse_test)timer/access_test_interval is less than 5!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	

}
DEFUN(config_radius_server_binding_type_cmd_func,
		config_radius_server_binding_type_cmd,
		"set radius_server binding (enable|disable)",
		"config the heart test parameters\n"
		"config the heart test parameters\n"
		"servers binding or not\n"
		"servers binding or not\n"
		"binded\n"
		"unbinded\n"
)
{
	unsigned int ret = 0;
	unsigned char security_id = 0;
	unsigned char type = 0;
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);

	
	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 2;
	} 
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == SECURITY_NODE){
		index = 0;
		security_id = (int)vty->index;	
	}else if(vty->node == HANSI_SECURITY_NODE){
		index = vty->index;  
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_SECURITY_NODE){
		index = vty->index;
		security_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_BINDING_ENABLE_DISABLE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&type,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"security %d successfully set radius server binding %s.\n",security_id,argv[0]);
	else if(ret == ASD_SECURITY_HEART_TEST_ON) 		
		vty_out(vty,"The radius heart test is on,turn it off first!\n");
	else if(ret == ASD_DBUS_RADIUS_BOTH_UNBINED)
		vty_out(vty,"Binding enable failed because of heart test type is both!\n");
	else if(ret == ASD_SECURITY_RADIUSINIT_FAIL)
		vty_out(vty,"It should not be unbinded because of the servers have the same ip\n");
	else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
		vty_out(vty,"<error> config the radius server first!\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	

}
int parse_double_PERCENT(char *str,double *percent){
	char *endptr = NULL;
	char c;
	char *mstr = NULL;
	unsigned long int t_int = 0;
	unsigned long int p_int = 0;
	unsigned int num = 0;
	c = str[0];
	if (c>='0'&&c<='9'){
		t_int = strtoul(str,&endptr,10);
		if(endptr[0] == '.')
		{
			mstr = endptr+1;
			while(mstr){
				if((mstr[0])>='0'&&(mstr[0])<='9'){
					num++;
					mstr++;
				}
				else
					break;
			}
			mstr = endptr+1;
			endptr = NULL;
			if(num)
				p_int = strtoul(mstr,&endptr,10);
			else
				return WID_ILLEGAL_INPUT;
			if(endptr[0] != '\0')
				return WID_ILLEGAL_INPUT;
			*percent = t_int + ((double)p_int)/pow(10,num);
			return WID_DBUS_SUCCESS;
		}
		else if(endptr[0] == '%' && endptr[1] == '\0'){
			*percent = ((double)t_int)/100;
			return WID_DBUS_SUCCESS;
		}
		else if(endptr[0] == '\0'){
			*percent = (double)t_int;
			return WID_DBUS_SUCCESS;
		}
		else
			return WID_ILLEGAL_INPUT;
	}
	else
		return WID_UNKNOWN_ID;

}
//end

int dcli_security_show_running_config(struct vty* vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//index = vty->index;
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
		printf("show security config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("security: %s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID))) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return 1;	
	}else{

		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"WLAN SECURITY");
		vtysh_add_show_string(_tmpstr);	
		vtysh_add_show_string(showStr);		
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
	} 

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STATIC_STA_RUNNING_CONFIG);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show static sta config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("security: %s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID))) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return 1;	
	}else{

		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"STATIC STA");
		vtysh_add_show_string(_tmpstr); 
		vtysh_add_show_string(showStr);
		
		dbus_message_unref(reply);
	} 

	return 0;

}


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


void dcli_security_init(void) {
	install_element(VIEW_NODE,&show_security_cmd);
	install_element(VIEW_NODE,&show_security_list_cmd);													/*a1*/
	install_element(VIEW_NODE,&show_asd_trap_state_cmd);
#if 0
	install_node(&security_node,dcli_security_show_running_config);
	install_default(SECURITY_NODE);

	/*-----------------------------------------VIEW_NODE----------------------------------------------------*/
	install_element(VIEW_NODE,&show_security_list_cmd);													/*a1*/
	install_element(VIEW_NODE,&show_security_cmd);														/*a2*/
	install_element(VIEW_NODE,&show_wlanid_security_wapi_config_cmd);/*nl 091214*/							/*a3*/
	install_element(VIEW_NODE,&show_security_wapi_info_cmd);/*nl add 09/11/03*/								/*a4*/
	install_element(VIEW_NODE,&show_radius_cmd);															/*a5*/
	install_element(VIEW_NODE,&show_asd_trap_state_cmd);	/*ht 091111*/									/*a6*/
	install_element(VIEW_NODE,&show_security_wapi_config_of_all_wlan_cmd);	//mahz add 2011.1.27	
	
	/*-----------------------------------------ENABLE_NODE---------------------------------------------------*/
/*	install_element(ENABLE_NODE,&create_security_cmd);	*/
	install_element(ENABLE_NODE,&show_security_list_cmd);													/*a1*/
	install_element(ENABLE_NODE,&show_security_cmd);														/*a2*/
	install_element(ENABLE_NODE,&show_wlanid_security_wapi_config_cmd);/*nl 091214*/							/*a3*/
	install_element(ENABLE_NODE,&show_security_wapi_info_cmd);/*nl add 09/11/03*/								/*a4*/
	install_element(ENABLE_NODE,&show_radius_cmd);															/*a5*/
	install_element(ENABLE_NODE,&show_asd_trap_state_cmd);	/*ht 091111*/									/*a6*/
	install_element(ENABLE_NODE,&show_radius_all_cmd);		//mahz add 2011.1.12
	install_element(ENABLE_NODE,&show_security_wapi_config_of_all_wlan_cmd);	//mahz add 2011.1.27	
	/*-----------------------------------------CONFIG_NODE--------------------------------------------------*/
	install_element(CONFIG_NODE,&show_security_list_cmd);													/*a1*/
	install_element(CONFIG_NODE,&show_security_cmd);														/*a2*/
	install_element(CONFIG_NODE,&show_wlanid_security_wapi_config_cmd);/*nl 091214*/							/*a3*/
	install_element(CONFIG_NODE,&show_security_wapi_info_cmd);/*nl add 09/11/03*/								/*a4*/
	install_element(CONFIG_NODE,&show_radius_cmd);															/*a5*/
	install_element(CONFIG_NODE,&show_asd_trap_state_cmd);	/*ht 091111*/									/*a6*/
	install_element(CONFIG_NODE,&show_radius_all_cmd);					//mahz add 2011.1.12
	install_element(CONFIG_NODE,&show_security_wapi_config_of_all_wlan_cmd);	//mahz add 2011.1.27	
	install_element(CONFIG_NODE,&show_asd_global_variable_cmd); 				//mahz add 2011.3.17
	install_element(CONFIG_NODE,&set_asd_socket_operate_cmd); 				//mahz add 2011.10.17
	install_element(CONFIG_NODE,&show_asd_rdc_info_cmd);		//mahz add 2011.11.18

	install_element(CONFIG_NODE,&create_security_cmd);		
	install_element(CONFIG_NODE,&delete_security_cmd);	
	install_element(CONFIG_NODE,&config_security_cmd);	
/*	install_element(CONFIG_NODE,&config_asd_get_sta_info_able_cmd);	*//*ht 091111*/
	install_element(CONFIG_NODE,&config_asd_get_sta_info_time_cmd);	
	install_element(CONFIG_NODE,&config_asd_ieee_80211n_able_cmd);
	install_element(CONFIG_NODE,&config_asd_set_sta_static_fdb_able_cmd);
	install_element(CONFIG_NODE,&config_set_asd_trap_able_cmd);	/*ht 091111*/
	install_element(CONFIG_NODE,&set_mobile_open_cmd);	/*nl add 091010*/	
	install_element(CONFIG_NODE,&set_notice_sta_info_to_portal_open_cmd);/*nl add 091119*/
	install_element(CONFIG_NODE,&set_notice_sta_info_to_portal_timer_cmd);	
	#ifdef _D_WIRED_
	install_element(CONFIG_NODE,&config_port_enable_cmd);  /*xm  08/09/01*/
	install_element(CONFIG_NODE,&config_port_cmd);	/*xm  08/09/01*/
	install_element(CONFIG_NODE,&config_port_vlan_security_cmd);/*sz20080827*/
	install_element(CONFIG_NODE,&config_port_vlan_enable_cmd);/*sz20080827*/
	install_element(CONFIG_NODE,&config_vlan_list_security_cmd);/*sz20080827*/
	install_element(CONFIG_NODE,&config_vlan_list_enable_cmd);/*sz20080827*/
	#endif
	install_element(SECURITY_NODE,&config_wapi_auth_cmd);
	install_element(SECURITY_NODE,&config_wapi_multi_cert_cmd);
	install_element(SECURITY_NODE,&config_wapi_auth_path_cmd);
	install_element(SECURITY_NODE,&delete_wapi_certification_cmd);//weichao add 20110801
	install_element(SECURITY_NODE,&config_wapi_p12_cert_auth_path_cmd);
	install_element(SECURITY_NODE,&config_acct_auth_cmd);	
/*	install_element(SECURITY_NODE,&apply_security_cmd);*/
	install_element(SECURITY_NODE,&config_security_host_ip_cmd);	/*ht add,081105	*/
	install_element(SECURITY_NODE,&config_encryption_type_cmd);
	install_element(SECURITY_NODE,&config_security_type_cmd);	
	install_element(SECURITY_NODE,&config_extensible_auth_cmd);	
	install_element(SECURITY_NODE,&config_radius_server_select_cmd);/*zhanglei add*/
	install_element(SECURITY_NODE,&config_security_key_cmd);
	install_element(SECURITY_NODE,&config_secondary_auth_cmd);  /*xm 08/09/02*/
	install_element(SECURITY_NODE,&set_eap_reauth_period_cmd);  /*xm 08/09/02*/
	install_element(SECURITY_NODE,&config_secondary_acct_cmd);  /*xm 08/09/03*/

	install_element(SECURITY_NODE,&set_acct_interim_interval_cmd);  /*ht 090205*/
	install_element(SECURITY_NODE,&set_security_quiet_period_cmd);  /*ht 090727*/
	install_element(SECURITY_NODE,&config_pre_auth_cmd_func_cmd);	
	install_element(SECURITY_NODE,&config_accounting_on_cmd);	
	install_element(SECURITY_NODE,&config_radius_extend_attr_cmd);	
	install_element(SECURITY_NODE,&set_wapi_ucast_rekey_method_cmd);	/*	xm0701*/	
	install_element(SECURITY_NODE,&set_wapi_rekey_para_cmd);	/*xm0701	*/
	install_element(SECURITY_NODE,&set_security_wep_index_cmd);	/*nl 09/11/02*/	
	
	install_element(SECURITY_NODE,&set_wapi_sub_attr_certificateupdatecount_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_multicastupdatecount_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_unicastupdatecount_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_bklifetime_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_bkreauththreshold_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_satimeout_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_multicast_cipher_cmd);/*ht 100112*/
	
	install_element(SECURITY_NODE,&set_wapi_sub_attr_wapipreauth_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_multicaserekeystrict_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_unicastcipherenabled_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wapi_sub_attr_authenticationsuiteenable_cmd);/*nl 09/11/02*/
	install_element(SECURITY_NODE,&set_wpa_group_rekey_period_cmd);  /*ht 100611*/
	install_element(SECURITY_NODE,&set_wpa_keyupdate_timeout_period_cmd);  /*ht 100611*/
	install_element(SECURITY_NODE,&config_wapi_radius_auth_cmd); 		//mahz add 2010.11.24
	install_element(SECURITY_NODE,&config_wapi_radius_auth_set_user_passwd_cmd); 		//mahz add 2010.12.9
	install_element(SECURITY_NODE,&set_once_wpa_group_rekey_time_cmd);					//mahz add 2011.1.3
	install_element(SECURITY_NODE,&show_security_wapi_config_of_all_wlan_cmd);	//mahz add 2011.1.27	
	install_element(SECURITY_NODE,&config_hybrid_auth_cmd);						//mahz add 2011.2.18
	install_element(SECURITY_NODE,&set_asd_distribute_cmd); 					//mahz add 2011.10.26
	install_element(SECURITY_NODE,&set_asd_rdc_para_cmd); 					//mahz add 2011.10.26
#endif
/***************************************HANSI_EREA**************************************************/	
/*-----------------------------------------HANSI_NODE--------------------------------------------------*/
	install_node(&hansi_security_node,NULL,"HANSI_SECURITY_NODE");
	install_default(HANSI_SECURITY_NODE);
	
	install_element(HANSI_NODE,&set_mobile_open_cmd);	/*nl add 091010*/
	install_element(HANSI_NODE,&show_security_cmd);	
	install_element(HANSI_NODE,&show_security_list_cmd);	
	install_element(HANSI_NODE,&create_security_cmd);		
	install_element(HANSI_NODE,&delete_security_cmd);	
	install_element(HANSI_NODE,&config_security_cmd);	
	install_element(HANSI_NODE,&set_notice_sta_info_to_portal_open_cmd);	
	install_element(HANSI_NODE,&set_notice_sta_info_to_portal_timer_cmd);	
	install_element(HANSI_NODE,&show_radius_all_cmd);					//mahz add 2011.1.12
	install_element(HANSI_NODE,&show_security_wapi_config_of_all_wlan_cmd);	//mahz add 2011.1.27	
	install_element(HANSI_NODE,&show_asd_global_variable_cmd); 				//mahz add 2011.3.17
	install_element(HANSI_NODE,&show_wlanid_security_wapi_config_cmd);/*nl 091214*/							/*a3*/
	install_element(HANSI_NODE,&show_radius_cmd);															/*a5*/
	install_element(HANSI_NODE,&show_asd_trap_state_cmd);	/*ht 091111*/									/*a6*/
	install_element(HANSI_NODE,&config_asd_ieee_80211n_able_cmd);
	install_element(HANSI_NODE,&config_asd_set_sta_static_fdb_able_cmd);
	install_element(HANSI_NODE,&config_set_asd_trap_able_cmd);	/*ht 091111*/
	install_element(HANSI_NODE,&set_asd_socket_operate_cmd); 				//mahz add 2011.10.17
	install_element(HANSI_NODE,&show_asd_rdc_info_cmd);		//mahz add 2011.11.18
#ifdef _D_WIRED_
	install_element(HANSI_NODE,&config_port_enable_cmd);  /*xm  08/09/01*/
	install_element(HANSI_NODE,&config_port_cmd);	/*xm  08/09/01*/
	install_element(HANSI_NODE,&config_port_vlan_security_cmd);/*sz20080827*/
	install_element(HANSI_NODE,&config_port_vlan_enable_cmd);/*sz20080827*/
	install_element(HANSI_NODE,&config_vlan_list_security_cmd);/*sz20080827*/
	install_element(HANSI_NODE,&config_vlan_list_enable_cmd);/*sz20080827*/
	install_element(HANSI_NODE,&show_security_wapi_info_cmd);/*nl add 09/11/03*/
/*	install_element(HANSI_NODE,&config_asd_get_sta_info_able_cmd);	*//*ht 091111*/
	install_element(HANSI_NODE,&config_asd_get_sta_info_time_cmd);	
#endif
	/*-----------------------------------------HANSI_SECURITY_NODE--------------------------------------------------*/
	install_element(HANSI_SECURITY_NODE,&config_wapi_auth_cmd);
	install_element(HANSI_SECURITY_NODE,&config_wapi_multi_cert_cmd);
	install_element(HANSI_SECURITY_NODE,&config_wapi_auth_path_cmd);	
	install_element(HANSI_SECURITY_NODE,&delete_wapi_certification_cmd);//weichao add 20110801
	install_element(HANSI_SECURITY_NODE,&config_wapi_p12_cert_auth_path_cmd);
	install_element(HANSI_SECURITY_NODE,&config_acct_auth_cmd);	
/*	install_element(SECURITY_NODE,&apply_security_cmd);*/
	install_element(HANSI_SECURITY_NODE,&config_security_host_ip_cmd);	/*ht add,081105 */
	install_element(HANSI_SECURITY_NODE,&config_encryption_type_cmd);
	install_element(HANSI_SECURITY_NODE,&config_security_type_cmd);	
	install_element(HANSI_SECURITY_NODE,&config_extensible_auth_cmd); 
	install_element(HANSI_SECURITY_NODE,&config_radius_server_select_cmd);/*zhanglei add*/
	install_element(HANSI_SECURITY_NODE,&config_security_key_cmd);
	install_element(HANSI_SECURITY_NODE,&config_secondary_auth_cmd);	/*xm 08/09/02*/
	install_element(HANSI_SECURITY_NODE,&set_eap_reauth_period_cmd);	/*xm 08/09/02*/
	install_element(HANSI_SECURITY_NODE,&config_secondary_acct_cmd);	/*xm 08/09/03*/
	install_element(HANSI_SECURITY_NODE,&config_mac_auth_cmd);

	install_element(HANSI_SECURITY_NODE,&set_acct_interim_interval_cmd);	/*ht 090205*/
	install_element(HANSI_SECURITY_NODE,&set_security_quiet_period_cmd);	/*ht 090727*/
	install_element(HANSI_SECURITY_NODE,&config_pre_auth_cmd_func_cmd);	
	install_element(HANSI_SECURITY_NODE,&config_accounting_on_cmd);	
	install_element(HANSI_SECURITY_NODE,&set_wapi_ucast_rekey_method_cmd);	/*	xm0701*/	
	install_element(HANSI_SECURITY_NODE,&set_wapi_rekey_para_cmd);	/*xm0701	*/
	install_element(HANSI_SECURITY_NODE,&config_radius_extend_attr_cmd);	
	
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_certificateupdatecount_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_multicastupdatecount_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_unicastupdatecount_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_bklifetime_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_bkreauththreshold_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_satimeout_cmd);/*nl 09/11/02*/

	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_wapipreauth_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_multicaserekeystrict_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_unicastcipherenabled_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_authenticationsuiteenable_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_security_wep_index_cmd);/*nl 09/11/02*/
	install_element(HANSI_SECURITY_NODE,&set_wpa_group_rekey_period_cmd);  /*ht 100611*/
	install_element(HANSI_SECURITY_NODE,&set_wpa_keyupdate_timeout_period_cmd);  /*ht 100611*/
	install_element(HANSI_SECURITY_NODE,&config_wapi_radius_auth_cmd); 			//mahz add 2010.11.24
	install_element(HANSI_SECURITY_NODE,&config_wapi_radius_auth_set_user_passwd_cmd); 		//mahz add 2010.12.9
	install_element(HANSI_SECURITY_NODE,&set_once_wpa_group_rekey_time_cmd);					//mahz add 2011.1.3
	install_element(HANSI_SECURITY_NODE,&config_hybrid_auth_cmd);						//mahz add 2011.2.18
	install_element(HANSI_SECURITY_NODE,&config_fast_auth_cmd); 					//mahz add 2011.7.8
	install_element(HANSI_SECURITY_NODE,&set_asd_distribute_cmd); 					//mahz add 2011.10.26
	install_element(HANSI_SECURITY_NODE,&set_asd_rdc_para_cmd); 					//mahz add 2011.10.26
	install_element(HANSI_SECURITY_NODE,&set_wapi_sub_attr_multicast_cipher_cmd);/*ht 100112*/
	install_element(HANSI_SECURITY_NODE,&set_eap_alive_period_cmd);	//weichao add20110902
	install_element(HANSI_SECURITY_NODE,&set_sta_account_cmd);	//weichao add20111201
	install_element(HANSI_SECURITY_NODE,&set_dhcp_account_cmd);	//weichao add20111201
	install_element(HANSI_SECURITY_NODE,&set_ap_max_detect_interval_cmd);	//weichao add
	//install_element(HANSI_SECURITY_NODE,&set_traffic_limit_radius_cmd);	
	install_element(HANSI_SECURITY_NODE,&config_eap_sm_run_activated_cmd);//Qc
	//qiuchen add it for master_bak radius server 2012.12.11
	install_element(HANSI_SECURITY_NODE,&config_ac_radius_name_cmd);
	install_element(HANSI_SECURITY_NODE,&config_radius_res_fail_suc_percent_cmd);
	install_element(HANSI_SECURITY_NODE,&config_radius_server_access_test_interval_cmd);
	install_element(HANSI_SECURITY_NODE,&config_radius_server_change_test_timer_cmd);
	install_element(HANSI_SECURITY_NODE,&config_radius_server_heart_test_type_cmd);
	install_element(HANSI_SECURITY_NODE,&config_radius_server_binding_type_cmd);
	install_element(HANSI_SECURITY_NODE,&set_radius_heart_test_on_cmd);
	//end
	/*-----------------------------------------LOCAL_HANSI_NODE--------------------------------------------------*/
	install_node(&local_hansi_security_node,NULL,"LOCAL_HANSI_SECURITY_NODE");
	install_default(LOCAL_HANSI_SECURITY_NODE);

	install_element(LOCAL_HANSI_NODE,&set_mobile_open_cmd);	/*nl add 091010*/
	install_element(LOCAL_HANSI_NODE,&show_security_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_security_list_cmd);	
	install_element(LOCAL_HANSI_NODE,&create_security_cmd);		
	install_element(LOCAL_HANSI_NODE,&delete_security_cmd);	
	install_element(LOCAL_HANSI_NODE,&config_security_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_notice_sta_info_to_portal_open_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_notice_sta_info_to_portal_timer_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_radius_all_cmd);					//mahz add 2011.1.12
	install_element(LOCAL_HANSI_NODE,&show_security_wapi_config_of_all_wlan_cmd);	//mahz add 2011.1.27	
	install_element(LOCAL_HANSI_NODE,&show_asd_global_variable_cmd); 				//mahz add 2011.3.17
	install_element(LOCAL_HANSI_NODE,&show_wlanid_security_wapi_config_cmd);/*nl 091214*/							/*a3*/
	install_element(LOCAL_HANSI_NODE,&show_radius_cmd);															/*a5*/
	install_element(LOCAL_HANSI_NODE,&show_asd_trap_state_cmd);	/*ht 091111*/									/*a6*/
	install_element(LOCAL_HANSI_NODE,&config_asd_ieee_80211n_able_cmd);
	install_element(LOCAL_HANSI_NODE,&config_asd_set_sta_static_fdb_able_cmd);
	install_element(LOCAL_HANSI_NODE,&config_set_asd_trap_able_cmd);	/*ht 091111*/
	install_element(LOCAL_HANSI_NODE,&set_asd_socket_operate_cmd); 		//mahz add 2011.10.17
	install_element(LOCAL_HANSI_NODE,&show_asd_rdc_info_cmd);		//mahz add 2011.11.18
#ifdef _D_WIRED_
	install_element(LOCAL_HANSI_NODE,&config_port_enable_cmd);  /*xm  08/09/01*/
	install_element(LOCAL_HANSI_NODE,&config_port_cmd);	/*xm  08/09/01*/
	install_element(LOCAL_HANSI_NODE,&config_port_vlan_security_cmd);/*sz20080827*/
	install_element(LOCAL_HANSI_NODE,&config_port_vlan_enable_cmd);/*sz20080827*/
	install_element(LOCAL_HANSI_NODE,&config_vlan_list_security_cmd);/*sz20080827*/
	install_element(LOCAL_HANSI_NODE,&config_vlan_list_enable_cmd);/*sz20080827*/
	install_element(LOCAL_HANSI_NODE,&show_security_wapi_info_cmd);/*nl add 09/11/03*/
/*	install_element(HANSI_NODE,&config_asd_get_sta_info_able_cmd);	*//*ht 091111*/
	install_element(LOCAL_HANSI_NODE,&config_asd_get_sta_info_time_cmd);	
#endif
	/*-----------------------------------------LOCAL_HANSI_SECURITY_NODE--------------------------------------------------*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_wapi_auth_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_wapi_multi_cert_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_wapi_auth_path_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&delete_wapi_certification_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_wapi_p12_cert_auth_path_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_acct_auth_cmd);	
/*	install_element(SECURITY_NODE,&apply_security_cmd);*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_security_host_ip_cmd);	/*ht add,081105 */
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_encryption_type_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_security_type_cmd);	
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_extensible_auth_cmd); 
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_mac_auth_cmd); 
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_server_select_cmd);/*zhanglei add*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_security_key_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_secondary_auth_cmd);	/*xm 08/09/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_eap_reauth_period_cmd);	/*xm 08/09/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_secondary_acct_cmd);	/*xm 08/09/03*/

	install_element(LOCAL_HANSI_SECURITY_NODE,&set_acct_interim_interval_cmd);	/*ht 090205*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_security_quiet_period_cmd);	/*ht 090727*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_pre_auth_cmd_func_cmd);	
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_accounting_on_cmd);	
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_ucast_rekey_method_cmd);	/*	xm0701*/	
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_rekey_para_cmd);	/*xm0701	*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_extend_attr_cmd);	
	
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_certificateupdatecount_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_multicastupdatecount_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_unicastupdatecount_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_bklifetime_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_bkreauththreshold_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_satimeout_cmd);/*nl 09/11/02*/

	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_wapipreauth_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_multicaserekeystrict_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_unicastcipherenabled_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_authenticationsuiteenable_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_security_wep_index_cmd);/*nl 09/11/02*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wpa_group_rekey_period_cmd);  /*ht 100611*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wpa_keyupdate_timeout_period_cmd);  /*ht 100611*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_wapi_radius_auth_cmd); 			//mahz add 2010.11.24
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_wapi_radius_auth_set_user_passwd_cmd); 		//mahz add 2010.12.9
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_once_wpa_group_rekey_time_cmd);					//mahz add 2011.1.3
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_hybrid_auth_cmd);						//mahz add 2011.2.18
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_fast_auth_cmd); 					//mahz add 2011.7.8
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_asd_distribute_cmd); 					//mahz add 2011.10.26
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_asd_rdc_para_cmd); 					//mahz add 2011.10.26
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_wapi_sub_attr_multicast_cipher_cmd);/*ht 100112*/
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_eap_alive_period_cmd);		//weichao add
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_sta_account_cmd); 		//weichao add
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_dhcp_account_cmd); 		//weichao add
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_ap_max_detect_interval_cmd); 		//weichao add
	//install_element(LOCAL_HANSI_SECURITY_NODE,&set_traffic_limit_radius_cmd);		//weichao add
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_eap_sm_run_activated_cmd);//Qc
	//qiuchen add it for master_bak radius server 2012.12.11
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_ac_radius_name_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_res_fail_suc_percent_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_server_access_test_interval_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_server_change_test_timer_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_server_heart_test_type_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&config_radius_server_binding_type_cmd);
	install_element(LOCAL_HANSI_SECURITY_NODE,&set_radius_heart_test_on_cmd);
	//end
	return;
}



#endif
