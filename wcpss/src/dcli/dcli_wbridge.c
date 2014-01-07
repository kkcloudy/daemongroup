#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/stat.h>

#include "command.h"
#include "vtysh/vtysh.h"
#include <sys/wait.h>

#include "dcli_main.h"
#include "dcli_vrrp.h"
#include "wcpss/waw.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "wbmd/wbmdpub.h"
#include "dbus/wbmd/WbmdDbusDef.h"

DEFUN(config_wbridge_create_cmd_func,
	config_wbridge_create_cmd,
	"create wbridge ID A.B.C.D",
	CONFIG_STR
	"create wbridge id ip\n"
	"wbridge id\n"
	"wbridge ip\n"
)
{
	int ret = 0;
	unsigned int wbid = 0;
	int wbip = 0;
	int split = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
			if(ret == WID_ILLEGAL_INPUT){
				vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
			}
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wbid >= WBRIDGE_NUM || wbid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WBRIDGE_NUM-1);
		return CMD_SUCCESS;
	}
	ret = dcli_vrrp_check_ip_format((char*)argv[1], &split);
	if (CMD_SUCCESS == ret) {
		if (0 == split) {
			wbip = (unsigned long)inet_addr((char*)argv[1]);
		}else{
			vty_out(vty, "%%Illegal IP address %s!", argv[1]);
			return CMD_WARNING;
		}
	}
	else {
		vty_out(vty, "%%Illegal IP address %s!", argv[1]);
		return CMD_WARNING;		
	}
	int index = 0;
	int localid = 1;  
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;		
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 			
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_CREATE_WBRIDGE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&wbid,								
						DBUS_TYPE_UINT32,&wbip,
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

		if(ret == 0){
			vty_out(vty,"wbridge %d was successfully created.\n",wbid);
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(delete_wbridge_cmd_func,
	  delete_wbridge_cmd,
	  "delete wbridge ID",
	  CONFIG_STR
	  "Wireless bridge information\n"
	  "bridge id that you want to config\n"
	 )
{	int ret;
	unsigned int wbid,ip;
	
	int localid = 1; 
	int slot_id = HostSlotId; 
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_WARNING;
	}	
	if(wbid >= WBRIDGE_NUM || wbid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WBRIDGE_NUM-1);
		return CMD_WARNING;
	}
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;    
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_DELETE_WBRIDGE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wbid,
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
			vty_out(vty,"delete wbridge %d successfully.\n",wbid);
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(show_wbridge_list_cmd_func,
	show_wbridge_list_cmd,
	"show wbridge ID",
	SHOW_STR
	"Display wireless bridge information\n"
	"List wireless bridge summary\n"
)
{
	char  quit[] = "quit";
	char  run[] = "run";
	int ret,i=0;
	unsigned char num;
	
	int localid = 1; 
	int slot_id = HostSlotId; 
	int wbid = 0;
	int index = 0;
	
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
		if((strcmp(argv[0],"l")==0)||(strcmp(argv[0],"list")==0)){
			wbid = 0;
		}else{
			if(ret == WID_ILLEGAL_INPUT){
				vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
			}
			else{
				vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
		}
	}	
	if(wbid >= WBRIDGE_NUM || wbid < 0){
		vty_out(vty,"<error> wlan id should be 1 to %d or list\n",WBRIDGE_NUM-1);
		return CMD_WARNING;
	}
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	DCLI_WBRIDGE_API_GROUP * WBLIST = NULL;
	WBLIST = (DCLI_WBRIDGE_API_GROUP *) dcli_show_wbridge_list(
		index,
		localid,
		&ret,
		wbid,
		dcli_dbus_connection
		);
    vty_out(vty,"wbridge list summary\n");
	vty_out(vty,"wbridge num:	%d\n",(ret != 0)?0:WBLIST->wb_num);
    vty_out(vty,"==============================================================================\n");
	vty_out(vty,"%-15s %-20s %-15s %s\n","WBridgeID","WBridgeIP","WBridgeState","WBridgeOnline");
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		for (i = 0; i < WBLIST->wb_num; i++) {
			unsigned char *ip = (char *)&(WBLIST->wbridge[i]->IP);
			char ip_str[32] = {0};
			time_t now;
			int online = 0;
			if(WBLIST->wbridge[i]->access_time != 0){
				time(&now);
				online = now - WBLIST->wbridge[i]->access_time;
			}
			sprintf(ip_str,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
			vty_out(vty,"%-15d %-20s %-15s %02d:%02d:%02d\n",WBLIST->wbridge[i]->WBID,ip_str,(WBLIST->wbridge[i]->WBState==0)?quit:run,online/3600,(online%3600)/60,(online%3600)%60);
			if(wbid != 0){
				vty_out(vty,"%-15s %s","AccessTime:",(WBLIST->wbridge[i]->access_time!=0)?ctime(&(WBLIST->wbridge[i]->access_time)):" \n");
				vty_out(vty,"%-15s %s","LastAccessTime:",(WBLIST->wbridge[i]->last_access_time!=0)?ctime(&(WBLIST->wbridge[i]->last_access_time)):" \n");
				vty_out(vty,"%-15s %s","LastLeaveTime:",(WBLIST->wbridge[i]->leave_time!=0)?ctime(&(WBLIST->wbridge[i]->leave_time)):" \n");
			}
		}
	}
	vty_out(vty,"==============================================================================\n");
	dcli_wbridge_list_free(WBLIST);
	return CMD_SUCCESS;
	
}

DEFUN(config_wbridge_cmd_func,
	  config_wbridge_cmd,
	  "config wbridge ID",
	  CONFIG_STR
	  "Wireless bridge information\n"
	  "bridge id that you want to config\n"
	 )
{	int ret;
	unsigned int wbid,ip;
	
	int localid = 1; 
	int slot_id = HostSlotId; 
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_WARNING;
	}	
	if(wbid >= WBRIDGE_NUM || wbid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WBRIDGE_NUM-1);
		return CMD_WARNING;
	}
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;    
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_WBRIDGE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wbid,
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
					DBUS_TYPE_UINT32,&ip,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			char buf[DEFAULT_LEN] = {0};
			struct sockaddr_in src;
			src.sin_addr.s_addr = ip;
			sprintf(buf,"telnet %s \n",inet_ntoa(src.sin_addr));
			system(buf);
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

DEFUN(set_wbridge_snmp_cmd_func,
	set_wbridge_snmp_cmd,
	"set wbridge ID snmp .SNMP",
	CONFIG_STR
	"set wbridge id snmp\n"
	"wbridge id or default\n"
	"wbridge snmp\n"
)
{
	int ret = 0;
	unsigned int wbid = 0;
	int wbip = 0;
	int split = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int num = argc - 1;
	int i = 0;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
		if((strcmp(argv[0],"de")==0)||(strcmp(argv[0],"default")==0)){
			wbid = 0;
		}else{
			if(ret == WID_ILLEGAL_INPUT){
				vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
			}
			else{
				vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
		}
	}	
	if(wbid >= WBRIDGE_NUM || wbid < 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WBRIDGE_NUM-1);
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;  
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;		
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 			
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_SET_WBRIDGE_SNMP);
	dbus_error_init(&err);
	dbus_message_iter_init_append (query, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wbid);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	for(i = 1; (i < num+1); i++){			
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING,
					  &(argv[i]));
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
				
	dbus_message_iter_close_container (&iter, &iter_array);

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

		if(ret == 0){
			vty_out(vty,"wbridge %d was successfully setting.\n",wbid);
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


DEFUN(show_wbridge_if_info_cmd_func,
	show_wbridge_if_info_cmd,
	"show wbridge ID basic info",
	SHOW_STR
	"Display wireless bridge information\n"
	"List wireless bridge summary\n"
)
{
	char  quit[] = "quit";
	char  run[] = "run";
	int ret,i=0;
	unsigned char num;
	int j = 0;
	int localid = 1; 
	int slot_id = HostSlotId; 
	int wbid = 0;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
		if((strcmp(argv[0],"l")==0)||(strcmp(argv[0],"list")==0)){
			wbid = 0;
		}else{
			if(ret == WID_ILLEGAL_INPUT){
				vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
			}
			else{
				vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
		}
	}	
	if(wbid >= WBRIDGE_NUM || wbid < 0){
		vty_out(vty,"<error> wlan id should be 1 to %d or list\n",WBRIDGE_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	DCLI_WBRIDGE_API_GROUP * WBLIST = NULL;
	WBLIST = (DCLI_WBRIDGE_API_GROUP *) dcli_show_wbridge_basic_info(
		index,
		localid,
		&ret,
		wbid,
		dcli_dbus_connection
		);
    vty_out(vty,"wbridge list summary\n");
	vty_out(vty,"wbridge num:	%d\n",(ret != 0)?0:WBLIST->wb_num);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		for (i = 0; i < WBLIST->wb_num; i++) {
			unsigned char *ip = (char *)&(WBLIST->wbridge[i]->IP);
			char ip_str[32] = {0};
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"%-15s %-20s %-15s\n","WBridgeID","WBridgeIP","WBridgeState");
			sprintf(ip_str,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
			vty_out(vty,"%-15d %-20s %-15s\n",WBLIST->wbridge[i]->WBID,ip_str,(WBLIST->wbridge[i]->WBState==0)?quit:run);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			for(j = 0; j < WBLIST->wbridge[i]->if_num; j++){
				vty_out(vty,"%-15s %-9s %-6d %-4s %02X:%02X:%02X:%02X:%02X:%02X\n",WBLIST->wbridge[i]->WBIF[j].ifDescr,"Index:",WBLIST->wbridge[i]->WBIF[j].ifIndex,"MAC",WBLIST->wbridge[i]->WBIF[j].ifPhysAddress[0],
					WBLIST->wbridge[i]->WBIF[j].ifPhysAddress[1],WBLIST->wbridge[i]->WBIF[j].ifPhysAddress[2],WBLIST->wbridge[i]->WBIF[j].ifPhysAddress[3],WBLIST->wbridge[i]->WBIF[j].ifPhysAddress[4],WBLIST->wbridge[i]->WBIF[j].ifPhysAddress[5]);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","MTU:",WBLIST->wbridge[i]->WBIF[j].ifMtu,"SPEED:",WBLIST->wbridge[i]->WBIF[j].ifSpeed);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","RxOctets:",WBLIST->wbridge[i]->WBIF[j].ifInOctets,"RxUcastPkts:",WBLIST->wbridge[i]->WBIF[j].ifInUcastPkts);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","RxMcastPkts:",WBLIST->wbridge[i]->WBIF[j].ifInNUcastPkts,"RxDiscards:",WBLIST->wbridge[i]->WBIF[j].ifInDiscards);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","RxErrors:",WBLIST->wbridge[i]->WBIF[j].ifInErrors,"RxUnknownProtos:",WBLIST->wbridge[i]->WBIF[j].ifInUnknownProtos);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","TxOctets:",WBLIST->wbridge[i]->WBIF[j].ifOutOctets,"TxUcastPkts:",WBLIST->wbridge[i]->WBIF[j].ifOutUcastPkts);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","TxMcastPkts:",WBLIST->wbridge[i]->WBIF[j].ifOutNUcastPkts,"TxDiscards:",WBLIST->wbridge[i]->WBIF[j].ifOutDiscards);
				vty_out(vty,"%-15s %-16d %-15s %-10d\n","TxErrors:",WBLIST->wbridge[i]->WBIF[j].ifOutErrors,"TxUnknownProtos:",WBLIST->wbridge[i]->WBIF[j].ifOutQLen);
				vty_out(vty,"------------------------------------------------------------------------------\n");
			}
			vty_out(vty,"==============================================================================\n");
		}
	}
	dcli_wbridge_list_free(WBLIST);
	return CMD_SUCCESS;
	
}

DEFUN(show_wbridge_mint_info_cmd_func,
	show_wbridge_mint_info_cmd,
	"show wbridge ID mint info",
	SHOW_STR
	"Display wireless bridge information\n"
	"List wireless bridge summary\n"
)
{
	char  quit[] = "quit";
	char  run[] = "run";
	int ret,i=0;
	unsigned char num;
	int j = 0;
	int localid = 1; 
	int slot_id = HostSlotId; 
	int wbid = 0;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
		if((strcmp(argv[0],"l")==0)||(strcmp(argv[0],"list")==0)){
			wbid = 0;
		}else{
			if(ret == WID_ILLEGAL_INPUT){
				vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
			}
			else{
				vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
		}
	}	
	if(wbid >= WBRIDGE_NUM || wbid < 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WBRIDGE_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	DCLI_WBRIDGE_API_GROUP * WBLIST = NULL;
	WBLIST = (DCLI_WBRIDGE_API_GROUP *) dcli_show_wbridge_mint_info(
		index,
		localid,
		&ret,
		wbid,
		dcli_dbus_connection
		);
    vty_out(vty,"wbridge list summary\n");
	vty_out(vty,"wbridge num:	%d\n",(ret != 0)?0:WBLIST->wb_num);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		for (i = 0; i < WBLIST->wb_num; i++) {
			unsigned char *ip = (char *)&(WBLIST->wbridge[i]->IP);
			char ip_str[32] = {0};
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"%-15s %-20s %-15s\n","WBridgeID","WBridgeIP","WBridgeState");
			sprintf(ip_str,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
			vty_out(vty,"%-15d %-20s %-15s\n",WBLIST->wbridge[i]->WBID,ip_str,(WBLIST->wbridge[i]->WBState==0)?quit:run);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			for(j = 0; j < 1; j++){
				vty_out(vty,"%-15s %-6s %-3d %-4s %02X:%02X:%02X:%02X:%02X:%02X\n",WBLIST->wbridge[i]->WBMintNode.nodeName,"Index:",WBLIST->wbridge[i]->WBMintNode.nodeInterfaceId,"MAC",WBLIST->wbridge[i]->WBMintNode.netAddress[0],
					WBLIST->wbridge[i]->WBMintNode.netAddress[1],WBLIST->wbridge[i]->WBMintNode.netAddress[2],WBLIST->wbridge[i]->WBMintNode.netAddress[3],WBLIST->wbridge[i]->WBMintNode.netAddress[4],WBLIST->wbridge[i]->WBMintNode.netAddress[5]);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","nodeType:",WBLIST->wbridge[i]->WBMintNode.nodeType,"nodeMode:",WBLIST->wbridge[i]->WBMintNode.nodeMode);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","linksCount:",WBLIST->wbridge[i]->WBMintNode.linksCount,"nodesCount:",WBLIST->wbridge[i]->WBMintNode.nodesCount);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","protoState:",WBLIST->wbridge[i]->WBMintNode.protocolEnabled,"extraCost:",WBLIST->wbridge[i]->WBMintNode.extraCost);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","fixedCost:",WBLIST->wbridge[i]->WBMintNode.fixedCost,"nodeID:",WBLIST->wbridge[i]->WBMintNode.nodeID);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","ampLow:",WBLIST->wbridge[i]->WBMintNode.ampLow,"ampHigh:",WBLIST->wbridge[i]->WBMintNode.ampHigh);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","authMode:",WBLIST->wbridge[i]->WBMintNode.authMode,"authRelay:",WBLIST->wbridge[i]->WBMintNode.authRelay);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","crypt:",WBLIST->wbridge[i]->WBMintNode.crypt,"compress:",WBLIST->wbridge[i]->WBMintNode.compress);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","roaming:",WBLIST->wbridge[i]->WBMintNode.roaming,"polling:",WBLIST->wbridge[i]->WBMintNode.polling);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","BroadcastRT:",WBLIST->wbridge[i]->WBMintNode.mintBroadcastRate,"noiseFloor:",WBLIST->wbridge[i]->WBMintNode.noiseFloor);
				vty_out(vty,"%-15s %-10s \n","secretKey:",WBLIST->wbridge[i]->WBMintNode.secretKey);
				vty_out(vty,"------------------------------------------------------------------------------\n");
			}
			vty_out(vty,"==============================================================================\n");
		}
	}
	dcli_wbridge_list_free(WBLIST);
	return CMD_SUCCESS;
	
}


DEFUN(show_wbridge_rf_info_cmd_func,
	show_wbridge_rf_info_cmd,
	"show wbridge ID rf info",
	SHOW_STR
	"Display wireless bridge information\n"
	"List wireless bridge summary\n"
)
{
	char  quit[] = "quit";
	char  run[] = "run";
	int ret,i=0;
	unsigned char num;
	int j = 0;
	int localid = 1; 
	int slot_id = HostSlotId; 
	int wbid = 0;
	ret = parse_int_ID((char*)argv[0], &wbid);
	if(ret != WID_DBUS_SUCCESS){
		if((strcmp(argv[0],"l")==0)||(strcmp(argv[0],"list")==0)){
			wbid = 0;
		}else{
			if(ret == WID_ILLEGAL_INPUT){
				vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
			}
			else{
				vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
		}
	}	
	if(wbid >= WBRIDGE_NUM || wbid < 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WBRIDGE_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	DCLI_WBRIDGE_API_GROUP * WBLIST = NULL;
	WBLIST = (DCLI_WBRIDGE_API_GROUP *) dcli_show_wbridge_rf_info(
		index,
		localid,
		&ret,
		wbid,
		dcli_dbus_connection
		);
    vty_out(vty,"wbridge list summary\n");
	vty_out(vty,"wbridge num:	%d\n",(ret != 0)?0:WBLIST->wb_num);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		for (i = 0; i < WBLIST->wb_num; i++) {
			unsigned char *ip = (char *)&(WBLIST->wbridge[i]->IP);
			char ip_str[32] = {0};
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"%-15s %-20s %-15s\n","WBridgeID","WBridgeIP","WBridgeState");
			sprintf(ip_str,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
			vty_out(vty,"%-15d %-20s %-15s\n",WBLIST->wbridge[i]->WBID,ip_str,(WBLIST->wbridge[i]->WBState==0)?quit:run);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			for(j = 0; j < 1; j++){
				vty_out(vty,"%-15s %-10s %-15s %-10d\n","Type:",WBLIST->wbridge[i]->WBRmProperty.rmType,"Index:",WBLIST->wbridge[i]->WBRmProperty.rmPropertiesIfIndex);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","Frequency:",WBLIST->wbridge[i]->WBRmProperty.rmFrequency,"BitRate:",WBLIST->wbridge[i]->WBRmProperty.rmBitRate);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","Sid:",WBLIST->wbridge[i]->WBRmProperty.rmSid,"CurPowerLevel:",WBLIST->wbridge[i]->WBRmProperty.rmCurPowerLevel);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","Modulation:",WBLIST->wbridge[i]->WBRmProperty.rmModulation,"Antenna:",WBLIST->wbridge[i]->WBRmProperty.rmAntenna);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","Distance:",WBLIST->wbridge[i]->WBRmProperty.rmDistance,"Burst:",WBLIST->wbridge[i]->WBRmProperty.rmBurst);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","LongRange:",WBLIST->wbridge[i]->WBRmProperty.rmLongRange,"PowerCtl:",WBLIST->wbridge[i]->WBRmProperty.rmPowerCtl);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","TXRT:",WBLIST->wbridge[i]->WBRmProperty.rmTXRT,"TXVRT:",WBLIST->wbridge[i]->WBRmProperty.rmTXVRT);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","PTP:",WBLIST->wbridge[i]->WBRmProperty.rmPTP,"WOCD:",WBLIST->wbridge[i]->WBRmProperty.rmWOCD);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","BCsid:",WBLIST->wbridge[i]->WBRmProperty.rmBCsid,"DistanceAuto:",WBLIST->wbridge[i]->WBRmProperty.rmDistanceAuto);
				vty_out(vty,"%-15s %-10d %-15s %-10d\n","NoiseFloor:",WBLIST->wbridge[i]->WBRmProperty.rmNoiseFloor,"Bandwidth:",WBLIST->wbridge[i]->WBRmProperty.rmBandwidth);
				vty_out(vty,"%-15s %-10d \n","ChainMode:",WBLIST->wbridge[i]->WBRmProperty.rmChainMode);
				vty_out(vty,"------------------------------------------------------------------------------\n");
			}
			vty_out(vty,"==============================================================================\n");
		}
	}
	dcli_wbridge_list_free(WBLIST);
	return CMD_SUCCESS;
	
}


char* dcli_wbridge_show_running_config(int localid, int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WBMD_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); 

	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
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
		dbus_message_unref(reply);
		return NULL;	
	}

}


void dcli_wbridge_init(void) {		
	install_element(CONFIG_NODE,&config_wbridge_create_cmd);
	install_element(CONFIG_NODE,&show_wbridge_list_cmd);	
	install_element(CONFIG_NODE,&config_wbridge_cmd);
	install_element(CONFIG_NODE,&set_wbridge_snmp_cmd);	
	install_element(CONFIG_NODE,&show_wbridge_if_info_cmd);	
	install_element(CONFIG_NODE,&show_wbridge_mint_info_cmd);	
	install_element(CONFIG_NODE,&show_wbridge_rf_info_cmd);	
	install_element(CONFIG_NODE,&delete_wbridge_cmd);
	return;
}



