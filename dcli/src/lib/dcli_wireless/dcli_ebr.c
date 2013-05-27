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
#include "vtysh/vtysh.h"
#include <lib/vty.h>
#include "memory.h"
#include "command.h"
#include "if.h"
#include "../dcli_main.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dcli_ebr.h"  
#include "bsd/bsdpub.h"


struct cmd_node ebr_node =
{
	EBR_NODE,
	"%s(config-ebr %d)# "
};
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
struct cmd_node ebr_node1 =
{
	EBR_NODE1,
	" "
};


DEFUN(wid_create_ethereal_bridge_cmd_func,
		wid_create_ethereal_bridge_cmd,
		"create ebr ID BRNAME",
		CONFIG_STR
		"ethereal bridge\n"
		"ethereal bridge\n"
		"ethereal bridge ID\n"
		"ethereal bridge NAME\n"
	)
{
	int ret=0,len=0;
	unsigned int isAdd = 1;	
	unsigned int EBRID = 0;
	char *name = NULL;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 1;			

	ret = parse_int_ID((char*)argv[0], &EBRID);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	
	if(EBRID >= EBR_NUM || EBRID == 0){
		vty_out(vty,"<error> ebr id should be 1 to %d\n",EBR_NUM-1);
		return CMD_SUCCESS;
	}
	
	len = strlen(argv[1]);
	if(len > 15){
		vty_out(vty,"<error> ebr name is too long,it should be 1 to 15 \n");
		return CMD_SUCCESS;
	}
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));	
	
	
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
	else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
   }
	
	DBusConnection *dcli_dbus_connection = NULL;

	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&EBRID,
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
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"ebr %d was successfully created.\n",EBRID);
		else if(ret == WID_EBR_BE_USED)
			vty_out(vty,"<error> ebr id exist\n");
		else if(ret == WID_EBR_ERROR)
			vty_out(vty,"<error> ebr%d is already exist\n",EBRID);
		else if(ret == SYSTEM_CMD_ERROR)
			vty_out(vty,"<error> system cmd error\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return CMD_SUCCESS;	
}
DEFUN(wid_delete_ethereal_bridge_cmd_func,
		wid_delete_ethereal_bridge_cmd,
		"delete ebr ID",
		"Delete Configuration\n"
		"ethereal bridge\n"
		"ethereal bridge\n"
		"ethereal bridge ID\n"
	)
{
	int ret;
	int isAdd = 1;	
	unsigned int EBRID = 0;
	char *name = NULL;
	char *name_d = "0";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 0;	
	ret = parse_int_ID((char*)argv[0], &EBRID);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(EBRID >= EBR_NUM || EBRID == 0){
		vty_out(vty,"<error> ebr id should be 1 to %d\n",EBR_NUM-1);
		return CMD_SUCCESS;
	}

	name = name_d;

	int localid = 1;
    int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&EBRID,
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
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"ebr %d was successfully deleted.\n",EBRID);
		else if(ret == WID_EBR_NOT_EXIST)
			vty_out(vty,"<error> ebr id does not exist\n");
		else if(ret == SYSTEM_CMD_ERROR)
			vty_out(vty,"<error> system cmd error\n");
		else if(ret == WID_EBR_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> ebr is enable,please disable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}


DEFUN(wid_show_ethereal_bridge_cmd_func,
	  wid_show_ethereal_bridge_cmd,
	  "show ebr ID [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "ethereal bridge\n"
	  "ID of ethereal bridge\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret;
	int EBRID = 0;
	int i = 0;
	unsigned int num = 0;
	unsigned int num2 = 0;
	EBR_IF_LIST	**iflist;
	EBR_IF_LIST	**uplinklist;	
	ETHEREAL_BRIDGE *ebr;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	char en[] = "enable";
	char dis[] = "disable";
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	ret = parse_int_ID((char*)argv[0], &EBRID);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(EBRID >= EBR_NUM || EBRID == 0){
		vty_out(vty,"<error> ebr id should be 1 to %d\n",EBR_NUM-1);
		return CMD_SUCCESS;
	}
	
	int localid = 1;
    int slot_id = HostSlotId;
	int index = 0;
	unsigned char localid_pub = 1;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SHOW_EBR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SHOW_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EBRID,
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
	
	if(ret == 0 )
	{	
		ebr = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ebr->EBRID);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ebr->name);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ebr->state);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ebr->isolation_policy);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ebr->multicast_isolation_policy);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ebr->sameportswitch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
		
		iflist = malloc(num* sizeof(EBR_IF_LIST *));
		memset(iflist, 0, num*sizeof(EBR_IF_LIST*));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			iflist[i] = (EBR_IF_LIST *)malloc(sizeof(EBR_IF_LIST));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(iflist[i]->ifname));
		
			dbus_message_iter_next(&iter_array);
		}
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num2);
		
		uplinklist = malloc(num2* sizeof(EBR_IF_LIST *));
		memset(uplinklist, 0, num2*sizeof(EBR_IF_LIST*));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num2; i++) {
			DBusMessageIter iter_struct;
			
			uplinklist[i] = (EBR_IF_LIST *)malloc(sizeof(EBR_IF_LIST));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(uplinklist[i]->ifname));
		
			dbus_message_iter_next(&iter_array);
		}

		
	}
		
	dbus_message_unref(reply);
#endif	
	DCLI_EBR_API_GROUP *EBRINFO = NULL;

	DBusConnection *dcli_dbus_connection = NULL;
	
	localid_pub = (unsigned char)localid;
	
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
		localid_pub = (unsigned char)localid;
		
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
		EBRINFO = dcli_ebr_show_api_group(
			index,
			0,
			EBRID,
			&ret,
			&localid_pub,
			dcli_dbus_connection,
			WID_DBUS_EBR_METHOD_SHOW_EBR
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0)
		{
			vty_out(vty,"EBR %d infomation\n",EBRINFO->EBR[0]->EBRID);
			vty_out(vty,"================================================================================\n");
			vty_out(vty,"EBR id:	%d\n",EBRINFO->EBR[0]->EBRID);
			vty_out(vty,"EBR name:	%s\n",EBRINFO->EBR[0]->name);
			vty_out(vty,"EBR state:	%s\n",(EBRINFO->EBR[0]->state == 1)?en:dis);
			vty_out(vty,"EBR isolation policy:		%s\n",(EBRINFO->EBR[0]->isolation_policy == 1)?en:dis);
			vty_out(vty,"EBR multicast isolation policy:	%s\n",(EBRINFO->EBR[0]->multicast_isolation_policy == 1)?en:dis);
			vty_out(vty,"EBR sameportswitch policy:	%s\n",(EBRINFO->EBR[0]->sameportswitch== 1)?en:dis);
			vty_out(vty,"EBR unicast solicit policy:\t%s\n", (EBRINFO->EBR[0]->bridge_ucast_solicit_stat == 1) ? en : dis);
			vty_out(vty,"EBR multicast solicit policy:\t%s\n", (EBRINFO->EBR[0]->bridge_mcast_solicit_stat == 1) ? en : dis);
			vty_out(vty,"EBR multicast fdb learn policy:	%s\n",(EBRINFO->EBR[0]->multicast_fdb_learn == 1) ? en:dis);
			if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->iflist))
			{	
			
				EBR_IF_LIST *head = NULL;
				head = EBRINFO->EBR[0]->iflist;
				while(head != NULL)
				{
					vty_out(vty,"EBR interface: %s\n",head->ifname);
					head = head->ifnext;
					//free(iflist[i]);
				}
				//free(iflist);
			}
			
			if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->uplinklist))
			{
				EBR_IF_LIST *head = NULL;
				head = EBRINFO->EBR[0]->uplinklist;
				while(head != NULL)
				{
					vty_out(vty,"EBR uplink port interface: %s\n",head->ifname);
					head = head->ifnext;
					//free(uplinklist[i]);
					//uplinklist[i] = NULL;
				}
				//free(uplinklist);
				//uplinklist = NULL;
			}	
			/*if(ebr)
			{
				free(ebr);
				ebr = NULL;
			}*/
			dcli_ebr_free_fun(WID_DBUS_EBR_METHOD_SHOW_EBR,EBRINFO);
			vty_out(vty,"================================================================================\n");
			
		}
		else if(ret == WID_EBR_NOT_EXIST)
		{
			vty_out(vty,"<error> ebr id does not exist\n");
		}
		else 
		{	
			vty_out(vty,"<error>  %d\n",ret);
		}
		//dbus_message_unref(reply);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			localid_pub = (unsigned char)localid;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				EBRINFO = dcli_ebr_show_api_group(
					profile,
					0,
					EBRID,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_EBR_METHOD_SHOW_EBR
					);
				
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"--------------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0)
				{
					vty_out(vty,"EBR %d infomation\n",EBRINFO->EBR[0]->EBRID);
					vty_out(vty,"--------------------------------------------------------------------------------\n");
					vty_out(vty,"EBR id:	%d\n",EBRINFO->EBR[0]->EBRID);
					vty_out(vty,"EBR name:	%s\n",EBRINFO->EBR[0]->name);
					vty_out(vty,"EBR state:	%s\n",(EBRINFO->EBR[0]->state == 1)?en:dis);
					vty_out(vty,"EBR isolation policy:		%s\n",(EBRINFO->EBR[0]->isolation_policy == 1)?en:dis);
					vty_out(vty,"EBR multicast isolation policy:	%s\n",(EBRINFO->EBR[0]->multicast_isolation_policy == 1)?en:dis);
					vty_out(vty,"EBR sameportswitch policy:	%s\n",(EBRINFO->EBR[0]->sameportswitch== 1)?en:dis);
					vty_out(vty,"EBR unicast solicit policy:\t%s\n", (EBRINFO->EBR[0]->bridge_ucast_solicit_stat == 1) ? en : dis);
					vty_out(vty,"EBR multicast solicit policy:\t%s\n", (EBRINFO->EBR[0]->bridge_mcast_solicit_stat == 1) ? en : dis);
					vty_out(vty,"EBR multicast fdb learn policy:	%s\n",(EBRINFO->EBR[0]->multicast_fdb_learn == 1) ? en:dis);
					if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->iflist))
					{	
					
						EBR_IF_LIST *head = NULL;
						head = EBRINFO->EBR[0]->iflist;
						while(head != NULL)
						{
							vty_out(vty,"EBR interface: %s\n",head->ifname);
							head = head->ifnext;
						}
					}
					
					if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->uplinklist))
					{
						EBR_IF_LIST *head = NULL;
						head = EBRINFO->EBR[0]->uplinklist;
						while(head != NULL)
						{
							vty_out(vty,"EBR uplink port interface: %s\n",head->ifname);
							head = head->ifnext;
						}
					}	
					dcli_ebr_free_fun(WID_DBUS_EBR_METHOD_SHOW_EBR,EBRINFO);
					
				}
				else if(ret == WID_EBR_NOT_EXIST){
					vty_out(vty,"<error> ebr id does not exist\n");
				}
				else{	
					vty_out(vty,"<error>  %d\n",ret);
				}
				vty_out(vty,"================================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			localid_pub = (unsigned char)localid;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		local_hansi_parameter:
				EBRINFO = dcli_ebr_show_api_group(
					profile,
					0,
					EBRID,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_EBR_METHOD_SHOW_EBR
					);
				
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"--------------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0)
				{
					vty_out(vty,"EBR %d infomation\n",EBRINFO->EBR[0]->EBRID);
					vty_out(vty,"--------------------------------------------------------------------------------\n");
					vty_out(vty,"EBR id:	%d\n",EBRINFO->EBR[0]->EBRID);
					vty_out(vty,"EBR name:	%s\n",EBRINFO->EBR[0]->name);
					vty_out(vty,"EBR state:	%s\n",(EBRINFO->EBR[0]->state == 1)?en:dis);
					vty_out(vty,"EBR isolation policy:		%s\n",(EBRINFO->EBR[0]->isolation_policy == 1)?en:dis);
					vty_out(vty,"EBR multicast isolation policy:	%s\n",(EBRINFO->EBR[0]->multicast_isolation_policy == 1)?en:dis);
					vty_out(vty,"EBR sameportswitch policy:	%s\n",(EBRINFO->EBR[0]->sameportswitch== 1)?en:dis);
					vty_out(vty,"EBR unicast solicit policy:\t%s\n", (EBRINFO->EBR[0]->bridge_ucast_solicit_stat == 1) ? en : dis);
					vty_out(vty,"EBR multicast solicit policy:\t%s\n", (EBRINFO->EBR[0]->bridge_mcast_solicit_stat == 1) ? en : dis);
					vty_out(vty,"EBR multicast fdb learn policy:	%s\n",(EBRINFO->EBR[0]->multicast_fdb_learn == 1) ? en:dis);
					if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->iflist))
					{	
					
						EBR_IF_LIST *head = NULL;
						head = EBRINFO->EBR[0]->iflist;
						while(head != NULL)
						{
							vty_out(vty,"EBR interface: %s\n",head->ifname);
							head = head->ifnext;
						}
					}
					
					if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->uplinklist))
					{
						EBR_IF_LIST *head = NULL;
						head = EBRINFO->EBR[0]->uplinklist;
						while(head != NULL)
						{
							vty_out(vty,"EBR uplink port interface: %s\n",head->ifname);
							head = head->ifnext;
						}
					}	
					dcli_ebr_free_fun(WID_DBUS_EBR_METHOD_SHOW_EBR,EBRINFO);
					
				}
				else if(ret == WID_EBR_NOT_EXIST){
					vty_out(vty,"<error> ebr id does not exist\n");
				}
				else{	
					vty_out(vty,"<error>  %d\n",ret);
				}
				vty_out(vty,"================================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;
}
DEFUN(wid_show_ethereal_bridge_list_cmd_func,
	  wid_show_ethereal_bridge_list_cmd,
	  "show ebr (all|list) [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "ethereal bridge\n"
	  "all of ethereal bridge\n"
	  "all of ethereal bridge\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret;
	int i = 0;
	int num = 0;
	ETHEREAL_BRIDGE *ebr[EBR_NUM];
	char en[] = "enable";
	char dis[] = "disable";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int index = 0;
	int localid = 1;
	unsigned char localid_pub = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}

#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SHOW_EBR_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SHOW_EBR_LIST);*/
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
	
	if(ret == 0 )
	{	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			ebr[i] = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ebr[i]->EBRID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ebr[i]->name));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ebr[i]->state));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ebr[i]->isolation_policy));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ebr[i]->multicast_isolation_policy));

			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
#endif	
	DCLI_EBR_API_GROUP *EBRINFO = NULL;
	DBusConnection *dcli_dbus_connection = NULL;
	localid_pub = (unsigned char)localid;
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
		localid_pub = (unsigned char)localid;
		
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
		EBRINFO = dcli_ebr_show_api_group(
			index,
			0,
			0,
			&ret,
			&localid_pub,
			dcli_dbus_connection,
			WID_DBUS_EBR_METHOD_SHOW_EBR_LIST
			);
	    vty_out(vty,"ebr list summary:\n");	
	    vty_out(vty,"%d ebr exist\n",((ret == 0)?EBRINFO->ebr_num:0));
	    vty_out(vty,"================================================================================\n");
		vty_out(vty,"%-5s %-16s %-7s %-9s %-9s %-8s %-8s %-11s\n",
					"EBRID",
					"EBRName",
					"state",
					"isolation",
					"multicast",
					"usolicit", 
					"msolicit",
					"mfdb_learn");
		
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0)
		{
			for (i = 0; i < EBRINFO->ebr_num; i++) 
			{	
				vty_out(vty,"%-5d %-16s %-7s %-9s %-9s %-8s %-8s %-11s\n",
						EBRINFO->EBR[i]->EBRID,
						EBRINFO->EBR[i]->name,
						(EBRINFO->EBR[i]->state == 1)?en:dis,
						(EBRINFO->EBR[i]->isolation_policy == 1)?en:dis,
						(EBRINFO->EBR[i]->multicast_isolation_policy == 1)?en:dis,
						(EBRINFO->EBR[i]->bridge_ucast_solicit_stat == 1) ? en : dis,
						(EBRINFO->EBR[i]->bridge_mcast_solicit_stat == 1) ? en : dis,
						(EBRINFO->EBR[i]->multicast_fdb_learn == 1)?en:dis);
				
			}
			dcli_ebr_free_fun(WID_DBUS_EBR_METHOD_SHOW_EBR_LIST,EBRINFO);
		}
		vty_out(vty,"================================================================================\n");
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			localid_pub = (unsigned char)localid;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				EBRINFO = dcli_ebr_show_api_group(
					profile,
					0,
					0,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_EBR_METHOD_SHOW_EBR_LIST
					);
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"--------------------------------------------------------------------------------\n");
			    vty_out(vty,"ebr list summary:\n");	
			    vty_out(vty,"%d ebr exist\n",((ret == 0)?EBRINFO->ebr_num:0));
			    vty_out(vty,"================================================================================\n");
				vty_out(vty,"%-5s %-16s %-7s %-9s %-9s %-8s %-8s %-11s\n",
							"EBRID",
							"EBRName",
							"state",
							"isolation",
							"multicast",
							"usolicit", 
							"msolicit",
					"mfdb_learn");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0)
				{
					for (i = 0; i < EBRINFO->ebr_num; i++) 
					{	
						vty_out(vty,"%-5d %-16s %-7s %-9s %-9s %-8s %-8s %-11s\n",
							EBRINFO->EBR[i]->EBRID,
							EBRINFO->EBR[i]->name,
							(EBRINFO->EBR[i]->state == 1)?en:dis,
							(EBRINFO->EBR[i]->isolation_policy == 1)?en:dis,
							(EBRINFO->EBR[i]->multicast_isolation_policy == 1)?en:dis,
							(EBRINFO->EBR[i]->bridge_ucast_solicit_stat == 1) ? en : dis,
							(EBRINFO->EBR[i]->bridge_mcast_solicit_stat == 1) ? en : dis,
							(EBRINFO->EBR[i]->multicast_fdb_learn == 1)?en:dis);
					}
					dcli_ebr_free_fun(WID_DBUS_EBR_METHOD_SHOW_EBR_LIST,EBRINFO);
				}
				vty_out(vty,"================================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		localid_pub = (unsigned char)localid;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			EBRINFO = dcli_ebr_show_api_group(
				profile,
				0,
				0,
				&ret,
				&localid_pub,
				dcli_dbus_connection,
				WID_DBUS_EBR_METHOD_SHOW_EBR_LIST
				);
			vty_out(vty,"================================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"--------------------------------------------------------------------------------\n");
		    vty_out(vty,"ebr list summary:\n");	
		    vty_out(vty,"%d ebr exist\n",((ret == 0)?EBRINFO->ebr_num:0));
		    vty_out(vty,"================================================================================\n");
			vty_out(vty,"%-5s %-16s %-7s %-9s %-9s %-8s %-8s %-11s\n",
						"EBRID",
						"EBRName",
						"state",
						"isolation",
						"multicast",
						"usolicit", 
						"msolicit",
						"mfdb_learn");

			if(ret == -1){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else if(ret == 0)
			{
				for (i = 0; i < EBRINFO->ebr_num; i++) 
				{	
					vty_out(vty,"%-5d %-16s %-7s %-9s %-9s %-8s %-8s %-11s\n",
							EBRINFO->EBR[i]->EBRID,
							EBRINFO->EBR[i]->name,
							(EBRINFO->EBR[i]->state == 1)?en:dis,
							(EBRINFO->EBR[i]->isolation_policy == 1)?en:dis,
							(EBRINFO->EBR[i]->multicast_isolation_policy == 1)?en:dis,
							(EBRINFO->EBR[i]->bridge_ucast_solicit_stat == 1) ? en : dis,
							(EBRINFO->EBR[i]->bridge_mcast_solicit_stat == 1) ? en : dis,
							(EBRINFO->EBR[i]->multicast_fdb_learn == 1)?en:dis);

				}
				dcli_ebr_free_fun(WID_DBUS_EBR_METHOD_SHOW_EBR_LIST,EBRINFO);
			}
			vty_out(vty,"================================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}


DEFUN(wid_config_ethereal_bridge_cmd_func,
	  wid_config_ethereal_bridge_cmd,
	  "config ebr ID",
	  CONFIG_STR
	  "ethereal bridge\n"
	  "ethereal bridge\n"
	  "ethereal bridge ID\n"
	 )
{	int ret = 0;
	int ebrid = 0;
	int status = 0;
	int wlanid = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = parse_int_ID((char*)argv[0], &ebrid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_WARNING;
	}	
	if(ebrid >= EBR_NUM || ebrid == 0){
		vty_out(vty,"<error> ebr id should be 1 to %d\n",EBR_NUM-1);
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
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_CONFIG_EBR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_CONFIG_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret); 

	if(ret == 0)
	{
		if(vty->node == CONFIG_NODE){
			vty->node = EBR_NODE;
			vty->index = (void *)ebrid;
		}else if(vty->node == HANSI_NODE){
			vty->node = HANSI_EBR_NODE;
			vty->index_sub = (void *)ebrid;
			index = vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if (vty->node == LOCAL_HANSI_NODE){
		    vty->node = LOCAL_HANSI_EBR_NODE;
			vty->index_sub = (void *)ebrid;
		    index = vty->index;
		    localid = vty->local;
		    slot_id = vty->slotindex;
	  }
	}
	else if(ret == WID_EBR_NOT_EXIST)
	{
		vty_out(vty,"<error> ebr id does not exist\n");
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_ethereal_bridge_enable_cmd_func,
	  config_ethereal_bridge_enable_cmd,
	  "service (enable|disable)",
	  SERVICE_STR
	  "Make service enable/disable\n"
	 )
{	
	int ret;
	unsigned int ebrid = 0;
	unsigned char state = 0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = (unsigned int)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		ebrid = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		ebrid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		ebrid = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_CONFIG_EBR_ENABLE);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_CONFIG_EBR_ENABLE);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
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

	if(ret == 0)
		vty_out(vty,"set ebr %s successfully.\n",argv[0]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");	
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(ebr_set_bridge_isolation_func,
	  ebr_set_bridge_isolation_cmd,
	  "set bridge_isolation (enable|disable)",
	  CONFIG_STR
	  "ebr bridge isolation\n"
	  "ebr bridge isolation enable|disable\n" 

	 )
{
	int ret;
	unsigned int ebrid = 0;
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = (unsigned int)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		ebrid = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		ebrid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		ebrid = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_ISOLATION);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
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

	if(ret == 0)
		vty_out(vty,"set ebr isolation %s successfully\n",argv[0]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		vty_out(vty,"<error> ebr should be disable first\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");	
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> sameportswitch and isolation are conflict,disable sameportswitch first\n");	
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
	{
		vty_out(vty, "<error> apply security in this wlan first\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(ebr_set_bridge_multicast_isolation_func,
	  ebr_set_bridge_multicast_isolation_cmd,
	  "set bridge_multicast_isolation (enable|disable)",
	  CONFIG_STR
	  "ebr bridge multicast isolation\n"
	  "ebr bridge multicast isolation enable|disable\n" 
	 )
{
	int ret;
	unsigned int ebrid = 0;
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = (unsigned int)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		ebrid = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		ebrid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		ebrid = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_MULTICAST_ISOLATION);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_MULTICAST_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
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

	if(ret == 0)
		vty_out(vty,"set ebr multicast isolation %s successfully\n",argv[0]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		vty_out(vty,"<error> ebr should be disable first\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");	
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> sameportswitch and isolation are conflict,disable sameportswitch first\n");
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
	{
		vty_out(vty, "<error> apply security in this wlan first\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}














/*
  *****************************************************************************
  *  
  * NOTES:	 
  * INPUT:	   
  * OUTPUT:	  
  * return:	  
  *  
  * author: 		Huang Leilei 
  * begin time:	2012-11-12 9:00  
  * finish time:		2012-11-15 11:00 
  * history:	
  * 
  **************************************************************************** 
  */
DEFUN(ebr_set_bridge_ucast_solicit_func,
	  ebr_set_bridge_ucast_solicit_cmd,
	  "set bridge_ucast_solicit (enable|disable)",
	  CONFIG_STR
	  "set bridge unicast solicit\n"
	  "enable bridge unicast solicit\n"
	  "disable bridge unicast solicit\n" 
	 )
{
	int ret;
	unsigned int Wlan_Ebr_ID = 0;
	unsigned char Wlan_Ebr_flag = 0;		/* Wlan_Ebr_ID flag, which is unsed to identifying in wlan config mode or ebr config mode: 0-wlan config mode, 1-ebr config mode */
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	dbus_error_init(&err);
	if (!strcmp(argv[0],"enable"))
	{
		state=1;
	}
	else if (!strcmp(argv[0],"disable"))
	{
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_WARNING;
	}
	
	switch (vty->node)
	{
		
		case WLAN_NODE:
			index = 0;
			Wlan_Ebr_ID = (int)vty->index;
			Wlan_Ebr_flag = 0;
			break;
		case HANSI_WLAN_NODE:
			index = vty->index;			
			Wlan_Ebr_ID = (int)vty->index_sub;	
			localid = vty->local;
			slot_id = vty->slotindex;
			Wlan_Ebr_flag = 0;
			break;
		case LOCAL_HANSI_WLAN_NODE:
			index = vty->index;
			Wlan_Ebr_ID = (int)vty->index_sub;	
			localid = vty->local;
			slot_id = vty->slotindex;
			Wlan_Ebr_flag = 0;
			break;
		case EBR_NODE:
			index = 0;			
			Wlan_Ebr_ID = (int)vty->index;
			Wlan_Ebr_flag = 1;
			break;
		case HANSI_EBR_NODE:
			index = vty->index; 		
			Wlan_Ebr_ID = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
			Wlan_Ebr_flag = 1;
			break;
		case LOCAL_HANSI_EBR_NODE:
			index = vty->index;
			Wlan_Ebr_ID = (int)vty->index_sub;
		    localid = vty->local;
	  		slot_id = vty->slotindex;
			Wlan_Ebr_flag = 1;
			break;
		default :
			cli_syslog_info("<error> %s unknow node:%d\n", __func__, vty->node);
			vty_out(vty, "<error> unknow node:%d\n", vty->node);
			return CMD_WARNING;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);

	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_EBR_METHOD_SET_BRIDGE_UCAST_SOLICT);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &Wlan_Ebr_ID,
							 DBUS_TYPE_BYTE, &state,
							 DBUS_TYPE_BYTE, &Wlan_Ebr_flag,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		cli_syslog_info("%s %d <error> failed get reply.\n", __func__, __LINE__);
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	switch (ret)
	{
		case WID_DBUS_SUCCESS:
			vty_out(vty,"set  bridge unicast solicit %s successfully.\n",(state==1) ? en : di);
			break;
		case WID_DBUS_ERROR:
			vty_out(vty, "wid dbus error\n");
			break;
		case WLAN_CREATE_BR_FAIL:
			vty_out(vty,"<error> ebr bridge error\n");
			break;
		case WID_EBR_NOT_EXIST:
			vty_out(vty,"<error> ebr id does not exist\n");
			break;
		case WLAN_ID_NOT_EXIST:
			vty_out(vty, "<error> wlan id does not exist\n");
			break;
		case IF_POLICY_CONFLICT:
			vty_out(vty, "<error> wlan is not wlan if policy\n");
			break;
		case WLAN_APPLY_SECURITY_FIRST:
			vty_out(vty, "<error> apply security in this wlan first\n");
			break;
		case WID_WANT_TO_DELETE_WLAN:		/* Huangleilei add for ASXXZFI-1622 */
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			break;
		default:
			vty_out(vty,"<error> set set bridge_ucast_solicit unknow return value %d\n",ret);
			break;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*
  *****************************************************************************
  *  
  * NOTES:	 
  * INPUT:	   
  * OUTPUT:	  
  * return:	  
  *  
  * author: 		Huang Leilei 
  * begin time:	2012-11-12 9:00  
  * finish time:		2012-11-15 11:00 
  * history:	
  * 
  **************************************************************************** 
  */
DEFUN(ebr_set_bridge_multicast_solicit_func,
	  ebr_set_bridge_multicast_solicit_cmd,
	  "set bridge_mcast_solicit (enable|disable)",
	  CONFIG_STR
  	  "set bridge multicast solicit\n"
	  "enable bridge multicast solicit\n"
	  "disable bridge multicast solicit\n" 
	 )
{
	int ret;
	unsigned int Wlan_Ebr_ID = 0;
	unsigned char Wlan_Ebr_flag = 0;		/* Wlan_Ebr_ID flag, which is unsed to identifying in wlan config mode or ebr config mode: 0-wlan config mode, 1-ebr config mode */
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	dbus_error_init(&err);
	if (!strcmp(argv[0],"enable"))
	{
		state=1;
	}
	else if (!strcmp(argv[0],"disable"))
	{
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_WARNING;
	}

	switch (vty->node)
	{
		case WLAN_NODE:
			index = 0;
			Wlan_Ebr_ID = (int)vty->index;
			Wlan_Ebr_flag = 0;
			break;
		case HANSI_WLAN_NODE:
			index = vty->index;			
			Wlan_Ebr_ID = (int)vty->index_sub;	
			localid = vty->local;
			slot_id = vty->slotindex;
			Wlan_Ebr_flag = 0;
			break;
		case LOCAL_HANSI_WLAN_NODE:
			index = vty->index;
			Wlan_Ebr_ID = (int)vty->index_sub;	
			localid = vty->local;
			slot_id = vty->slotindex;
			Wlan_Ebr_flag = 0;
			break;
		case EBR_NODE:
			index = 0;			
			Wlan_Ebr_ID = (int)vty->index;
			Wlan_Ebr_flag = 1;
			break;
		case HANSI_EBR_NODE:
			index = vty->index; 		
			Wlan_Ebr_ID = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
			Wlan_Ebr_flag = 1;
			break;
		case LOCAL_HANSI_EBR_NODE:
			index = vty->index;
			Wlan_Ebr_ID = (int)vty->index_sub;
		    localid = vty->local;
	  		slot_id = vty->slotindex;
			Wlan_Ebr_flag = 1;
			break;
		default :
			cli_syslog_info("<error> %s unknow node:%d\n", __func__, vty->node);
			vty_out(vty, "<error> unknow node:%d\n", vty->node);
			return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);

	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_EBR_METHOD_SET_BRIDGE_MCAST_SOLICT);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &Wlan_Ebr_ID,
							 DBUS_TYPE_BYTE, &state,
							 DBUS_TYPE_BYTE, &Wlan_Ebr_flag,
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
		return CMD_WARNING;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	switch (ret)
	{
		case WID_DBUS_SUCCESS:
			vty_out(vty,"set bridge multicast solicit %s successfully.\n",(state==1) ? en : di);
			break;
		case WID_DBUS_ERROR:
			vty_out(vty, "wid dbus error\n");
			break;
		case WLAN_CREATE_BR_FAIL:
			vty_out(vty,"<error> ebr bridge error\n");
			break;
		case WID_EBR_NOT_EXIST:
			vty_out(vty,"<error> ebr id does not exist\n");
			break;	
		case WLAN_ID_NOT_EXIST:
			vty_out(vty, "<error> wlan id does not exist\n");
			break;
		case IF_POLICY_CONFLICT:
			vty_out(vty, "<error> wlan is not wlan if policy\n");
			break;
		case WLAN_APPLY_SECURITY_FIRST:
			vty_out(vty, "<error> apply security in this wlan first\n");
			break;
		case WID_WANT_TO_DELETE_WLAN:		/* Huangleilei add for ASXXZFI-1622 */
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			break;
		default:
			vty_out(vty,"<error> set bridge_mcast_solicit unknow return value %d\n",ret);
			break;
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}























DEFUN(ebr_set_bridge_sameportswitch_func,
	  ebr_set_bridge_sameportswitch_cmd,
	  "set bridge sameportswitch (enable|disable)",
	  CONFIG_STR
	  "ebr bridge sameportswitch\n"
	  "ebr bridge sameportswitch enable|disable\n" 
	 )
{
	int ret;
	unsigned int ebrid = 0;
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = (unsigned int)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		ebrid = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		ebrid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		ebrid = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_SAMEPORTSWITCH);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_SAMEPORTSWITCH);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
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

	if(ret == 0)
		vty_out(vty,"set ebr sameportswitch %s successfully\n",argv[0]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		vty_out(vty,"<error> ebr should be disable first\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");	
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> isolation or multicast are enable,disable isolation and multicast first\n");	
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_ebr_add_del_if_cmd_func,
	  set_ebr_add_del_if_cmd,
	  "set ebr (add|delete) interface IFNAME",
	  CONFIG_STR
	  "ethereal bridge\n"
	  "add if to br / remove if from br\n"
	  "add/delete interface"
	  "IFNAME like eth1-1.100\n" 
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned int EBRID = 0;
	char *name;
	unsigned int state = 0;
	int lenth = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	
	int wtpid = 0;
	int wlanid = 0;
	int radioid = 0;
	int vrid =0;
	int slotID =0;
	//EBRID = (unsigned int)vty->index;

	if (!strcmp(argv[0],"add")){
		state = 1;
	}
	else if (!strcmp(argv[0],"delete")){
		state = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'add' or 'delete'\n");
		return CMD_SUCCESS;
	}

	lenth = strlen((char *)argv[1]);

	if(lenth > 15)
	{		
		vty_out(vty,"<error> if name too long\n");
		return CMD_SUCCESS;
	}
	
	name = (char *)malloc(sizeof(char)*20);
	if(name == NULL)
	{
		return MALLOC_ERROR;
	}
	memset(name,0,20);
	memcpy(name,argv[1],strlen(argv[1]));
	
	if ((!strncasecmp(name,"radio",5))||(!strncasecmp(name,"r",1))){
		if(!strncasecmp(name,"radio",5)){
			if(parse_radio_ifname(name+5, &wtpid,&radioid,&wlanid) == 0){
			}else{
				if(parse_radio_ifname_v2(name+5, &wtpid,&radioid,&wlanid,&vrid) == 0){
				}else{
					if(parse_radio_ifname_v3(name+5, &wtpid,&radioid,&wlanid,&vrid,&slotID) == 0){

					}else
						ret = WID_UNKNOWN_ID;
				}
			}
		}else{
			if(parse_radio_ifname(name+1, &wtpid,&radioid,&wlanid) == 0){
			}else{
				if(parse_radio_ifname_v2(name+1, &wtpid,&radioid,&wlanid,&vrid) == 0){
				}else{
					if(parse_radio_ifname_v3(name+1, &wtpid,&radioid,&wlanid,&vrid,&slotID) == 0){

					}else
						ret = WID_UNKNOWN_ID;
				}
			}
		}
	}
	if (ret != WID_DBUS_SUCCESS) 
	{
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		EBRID = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		EBRID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		EBRID = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_IF);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_IF);*/

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EBRID,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_STRING,&name,
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
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0)
		vty_out(vty,"set ebr %s %s successfully.\n",argv[0],argv[1]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		vty_out(vty,"<error> ebr should be disable first\n");
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> %s already exist/remove some br,or system cmd process error\n",argv[1]);
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> input ifname error\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	if(name)
	{
		free(name);
		name = NULL;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_ebr_summary_detail_cmd_func,
	show_ebr_summary_detail_cmd,
	"show fdb (summary|detail)",
	CONFIG_STR
	"show fdb (summary |detail)\n"
	)
{
	int ret=WID_DBUS_SUCCESS;
	int is_detail=0;
	unsigned int ebrid;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);

	int fdb_count=0;

	if(!strcmp(argv[0],"summary")){
		is_detail=0;
		}
	else if(!strcmp(argv[0],"detail")){
		is_detail=1;
		}
	else{
		vty_out(vty,"<error> input parameter must be 'summary' or 'detail' \n");
		return CMD_SUCCESS;
		}
	
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;	
		ebrid = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		ebrid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		ebrid = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SHOW_FDB_SUMMARY_DETAIL);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ebrid,
						DBUS_TYPE_UINT32,&is_detail,
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
	
	
	if(ret==WID_DBUS_SUCCESS){
		
		dbus_message_iter_next(&iter); 
		dbus_message_iter_get_basic(&iter,&fdb_count);
		
		vty_out(vty,"\n\tTotal items of fdb: %d\n",fdb_count);
		if(is_detail && fdb_count>0){
			vty_out(vty,"==================================================\n");
			vty_out(vty,"Port_no\tMAC Addr\t\tIs_local\n");
			vty_out(vty,"--------------------------------------------------\n");
			int port_no=0;
			unsigned char is_local;
			unsigned char mac[6]={0};
			
			DBusMessageIter iter_fdb_array;
			DBusMessageIter iter_fdb;

			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_fdb_array);

			int i=0;
			for(i=0;i<fdb_count;i++){
				dbus_message_iter_recurse(&iter_fdb_array,&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&port_no);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&is_local);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&mac[0]);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&mac[1]);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&mac[2]);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&mac[3]);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&mac[4]);
				
				dbus_message_iter_next(&iter_fdb);
				dbus_message_iter_get_basic(&iter_fdb,&mac[5]);
				
				vty_out(vty,"%3i\t", port_no);
				vty_out(vty,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\t",
					mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				vty_out(vty,"%s\t\t",(is_local ? "yes":"no"));
				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_fdb_array);
				}			
			vty_out(vty,"==================================================\n");
		}
	}
	else if(ret==WID_EBR_NOT_EXIST){
		vty_out(vty,"<error> ebr id does not exist\n");
	}
	else if(ret==WID_EBR_ERROR){
		vty_out(vty,"<error> ebr if error\n");
	}
	else{
		vty_out(vty,"<error> %d\n",ret);
	}
	
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
	
}

DEFUN(set_ebr_add_del_uplink_cmd_func,
	  set_ebr_add_del_uplink_cmd,
	  "set ebr (add|delete) uplink IFNAME",
	  CONFIG_STR
	  "ethereal bridge\n"
	  "add uplink to br / remove uplink from br\n"
	  "add/delete uplink"
	  "IFNAME like eth1-1.100\n" 
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned int EBRID = 0;
	char *name;
	unsigned int state = 0;
	int lenth = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);

	if (!strcmp(argv[0],"add")){
		state = 1;
	}
	else if (!strcmp(argv[0],"delete")){
		state = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'add' or 'delete'\n");
		return CMD_SUCCESS;
	}

	lenth = strlen((char *)argv[1]);

	if(lenth > 15)
	{		
		vty_out(vty,"<error> if name too long\n");
		return CMD_SUCCESS;
	}
	
	name = (char *)malloc(sizeof(char)*20);
	if(name == NULL)
	{
		return MALLOC_ERROR;
	}
	memset(name,0,20);
	memcpy(name,argv[1],strlen(argv[1]));
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		EBRID = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		EBRID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		
		index = vty->index;
		EBRID = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_UPLINK);
		

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EBRID,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_STRING,&name,
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
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0)
		vty_out(vty,"set ebr %s uplink %s successfully.\n",argv[0],argv[1]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		vty_out(vty,"<error> ebr should be disable first\n");
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> %s already exist/remove some br,or system cmd process error\n",argv[1]);
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> input ifname error\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");
	else if(ret == WID_EBR_IF_NOEXIT)
		vty_out(vty,"<error> interface does not add to br or br uplink\n");	
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	if(name)
	{
		free(name);
		name = NULL;
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

DEFUN(ebr_set_multicast_fdb_learn_cmd_func,
	  ebr_set_multicast_fdb_learn_cmd,
	  "set ebr multicast fdb learn (enable|disable)",
	  CONFIG_STR
	  "ebr multicast fdb learn\n"
	  "ebr multicast fdb learn enable|disable\n" 
	 )
{
	int ret;
	unsigned int ebrid = 0;
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
    int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == EBR_NODE){
		index = 0;			
		ebrid = (int)vty->index;
	}else if(vty->node == HANSI_EBR_NODE){
		index = vty->index; 		
		ebrid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_EBR_NODE){
		index = vty->index;
		ebrid = (int)vty->index_sub;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_MULTICAST_FDB_LEARN);
		
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set ebr multicast fdb learn %s successfully\n",argv[0]);
	else if(ret == WID_EBR_NOT_EXIST)
		vty_out(vty,"<error> ebr id does not exist\n");
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		vty_out(vty,"<error> ebr should be disable first\n");
	else if(ret == WID_EBR_ERROR)
		vty_out(vty,"<error> ebr if error\n");	
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> isolation or multicast are enable,disable isolation and multicast first\n");	
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_ebr_show_running_config_start(struct vty *vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	char *tmp = NULL;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(
							BUSNAME,		\
							OBJPATH , \
							INTERFACE ,		\
							WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_START);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);//fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show ebr start config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"EBR1");
		vtysh_add_show_string(_tmpstr);	
		vtysh_add_show_string(showStr);
		dbus_message_unref(reply);
		
		tmp = vtysh_client_config_wireless_interface(&vtysh_client[0], "show interface ebr_config local 0 0");
		if(tmp != NULL){
			//vtysh_add_show_string(tmp);			
			dcli_config_write(tmp,1,slot_id,index,1,0);
			XFREE(MTYPE_TMP,tmp);
			tmp = NULL;
		}
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

int dcli_ebr_show_running_config_end(struct vty* vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(
							BUSNAME,		\
							OBJPATH , \
							INTERFACE , 	\
							WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_END);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);//fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show ebr end config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"EBR2");
		vtysh_add_show_string(_tmpstr);	
	
		vtysh_add_show_string(showStr);		
		dcli_config_write(showStr,1,slot_id,index,0,1);
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


void dcli_ebr_init(void) {
	install_element(VIEW_NODE,&wid_show_ethereal_bridge_cmd);												/*a2*/
	install_element(VIEW_NODE,&wid_show_ethereal_bridge_list_cmd);
#if 0
	install_node(&ebr_node,dcli_ebr_show_running_config_start);
	install_node(&ebr_node1,dcli_ebr_show_running_config_end);
	install_default(EBR_NODE);
	
	/*------------------------------VIEW_NODE-----------------------------*/
	install_element(VIEW_NODE,&wid_show_ethereal_bridge_list_cmd);										/*a1*/
	install_element(VIEW_NODE,&wid_show_ethereal_bridge_cmd);												/*a2*/
	/*------------------------------ENABLE_NODE-----------------------------*/
	install_element(ENABLE_NODE,&wid_show_ethereal_bridge_list_cmd);										/*a1*/
	install_element(ENABLE_NODE,&wid_show_ethereal_bridge_cmd);												/*a2*/
	
	/*------------------------------CONFIG_NODE-----------------------------*/
	install_element(CONFIG_NODE,&wid_show_ethereal_bridge_list_cmd);										/*a1*/
	install_element(CONFIG_NODE,&wid_show_ethereal_bridge_cmd);												/*a2*/
	install_element(CONFIG_NODE,&wid_config_ethereal_bridge_cmd);	
	install_element(CONFIG_NODE,&wid_create_ethereal_bridge_cmd);		
	install_element(CONFIG_NODE,&wid_delete_ethereal_bridge_cmd);	
	
	/*------------------------------EBR_NODE-----------------------------*/
	install_element(EBR_NODE,&ebr_set_bridge_isolation_cmd);
	install_element(EBR_NODE,&ebr_set_bridge_multicast_isolation_cmd);
	install_element(EBR_NODE,&ebr_set_bridge_sameportswitch_cmd);
	install_element(EBR_NODE,&set_ebr_add_del_if_cmd);
	install_element(EBR_NODE,&config_ethereal_bridge_enable_cmd);
	install_element(EBR_NODE,&set_ebr_add_del_uplink_cmd);
	install_element(EBR_NODE,&show_ebr_summary_detail_cmd);
	install_element(EBR_NODE,&ebr_set_multicast_fdb_learn_cmd);
#endif
	/********************************************************************************/
		install_node(&hansi_ebr_node,NULL,"HANSI_EBR_NODE");		
		install_default(HANSI_EBR_NODE);

		install_node(&local_hansi_ebr_node,NULL,"LOCAL_HANSI_EBR_NODE");		
		install_default(LOCAL_HANSI_EBR_NODE);
		
		/*------------------------------HANSI_NODE-----------------------------*/
		install_element(HANSI_NODE,&wid_show_ethereal_bridge_list_cmd);	
		install_element(HANSI_NODE,&wid_show_ethereal_bridge_cmd); 
		install_element(HANSI_NODE,&wid_config_ethereal_bridge_cmd);	
		install_element(HANSI_NODE,&wid_create_ethereal_bridge_cmd);		
		install_element(HANSI_NODE,&wid_delete_ethereal_bridge_cmd);	

		install_element(LOCAL_HANSI_NODE,&wid_show_ethereal_bridge_list_cmd);	
		install_element(LOCAL_HANSI_NODE,&wid_show_ethereal_bridge_cmd); 
		install_element(LOCAL_HANSI_NODE,&wid_config_ethereal_bridge_cmd);	
		install_element(LOCAL_HANSI_NODE,&wid_create_ethereal_bridge_cmd);		
		install_element(LOCAL_HANSI_NODE,&wid_delete_ethereal_bridge_cmd);	
		
		/*------------------------------HANSI_EBR_NODE-----------------------------*/
		install_element(HANSI_EBR_NODE,&ebr_set_bridge_isolation_cmd);
		install_element(HANSI_EBR_NODE,&ebr_set_bridge_multicast_isolation_cmd);

		install_element(HANSI_EBR_NODE, &ebr_set_bridge_ucast_solicit_cmd);
		install_element(HANSI_WLAN_NODE, &ebr_set_bridge_ucast_solicit_cmd);
		install_element(HANSI_EBR_NODE, &ebr_set_bridge_multicast_solicit_cmd);
		install_element(HANSI_WLAN_NODE, &ebr_set_bridge_multicast_solicit_cmd);
		
		install_element(HANSI_EBR_NODE,&ebr_set_bridge_sameportswitch_cmd);
		install_element(HANSI_EBR_NODE,&set_ebr_add_del_if_cmd);
		install_element(HANSI_EBR_NODE,&config_ethereal_bridge_enable_cmd);
		install_element(HANSI_EBR_NODE,&set_ebr_add_del_uplink_cmd);
		install_element(HANSI_EBR_NODE,&show_ebr_summary_detail_cmd);
		install_element(HANSI_EBR_NODE,&ebr_set_multicast_fdb_learn_cmd);
		
		install_element(LOCAL_HANSI_EBR_NODE,&ebr_set_bridge_isolation_cmd);
		install_element(LOCAL_HANSI_EBR_NODE,&ebr_set_bridge_multicast_isolation_cmd);

		install_element(LOCAL_HANSI_EBR_NODE, &ebr_set_bridge_ucast_solicit_cmd);
		install_element(LOCAL_HANSI_WLAN_NODE, &ebr_set_bridge_ucast_solicit_cmd);
		install_element(LOCAL_HANSI_EBR_NODE, &ebr_set_bridge_multicast_solicit_cmd);
		install_element(LOCAL_HANSI_WLAN_NODE, &ebr_set_bridge_multicast_solicit_cmd);
		
		install_element(LOCAL_HANSI_EBR_NODE,&ebr_set_bridge_sameportswitch_cmd);
		install_element(LOCAL_HANSI_EBR_NODE,&set_ebr_add_del_if_cmd);
		install_element(LOCAL_HANSI_EBR_NODE,&config_ethereal_bridge_enable_cmd);
		install_element(LOCAL_HANSI_EBR_NODE,&set_ebr_add_del_uplink_cmd);
		install_element(LOCAL_HANSI_EBR_NODE,&show_ebr_summary_detail_cmd);
		install_element(LOCAL_HANSI_EBR_NODE,&ebr_set_multicast_fdb_learn_cmd);

	return;
}

#endif

