#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"

#include "dcli_main.h"
#include "dcli_ac_ip_list.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "bsd/bsdpub.h"
struct cmd_node ac_ip_list_node =
{
	ACIPLIST_NODE,
	"%s(config-list %d)# "
};
struct cmd_node hansi_ac_ip_list_node =
{
	HANSI_ACIPLIST_NODE,
	"%s(hansi-ac_ip_list %d-%d-%d)# ",
	1
};

struct cmd_node local_hansi_ac_ip_list_node =
{
	LOCAL_HANSI_ACIPLIST_NODE,
	"%s(local-hansi-ac_ip_list %d-%d-%d)# ",
	1
};




DEFUN(show_ac_ip_list_one_cmd_func,
	  show_ac_ip_list_one_cmd,
	  "show ac-ip-list ID [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "ac-ip-list\n"
	  "ID of ac-ip-list\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret;
	unsigned char ID = 0;
	int i = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	unsigned int num = 0;
	struct wid_ac_ip **iplist;
	//wid_ac_ip_group *AC_IP_LIST;
	
	char en[] = "enable";
	char dis[] = "disable";
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	ret = parse_char_ID((char*)argv[0], &ID);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(ID >= ACIPLIST_NUM || ID == 0){
		vty_out(vty,"<error> AC IP LIST id should be 1 to %d\n",ACIPLIST_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	unsigned char localid_pub = 1;
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
	
	
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
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
		AC_IP_LIST = (wid_ac_ip_group *)malloc(sizeof(wid_ac_ip_group));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&AC_IP_LIST->GroupID);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&AC_IP_LIST->ifname);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
		
		iplist = malloc(num* sizeof(struct wid_ac_ip *));
		memset(iplist, 0, num*sizeof(struct wid_ac_ip*));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			iplist[i] = (struct wid_ac_ip *)malloc(sizeof(struct wid_ac_ip));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(iplist[i]->ip));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(iplist[i]->priority));
		
			dbus_message_iter_next(&iter_array);
		}
	}
		
	dbus_message_unref(reply);
#endif
	DCLI_AC_IP_LIST_API_GROUP * IPLIST = NULL;
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
		IPLIST = dcli_ac_ip_list_show_api_group(
			index,
			ID,
			0,
			&ret,
			&localid_pub,
			dcli_dbus_connection,
			WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0)
		{
			vty_out(vty,"AC IP LIST %d infomation\n",IPLIST->AC_IP_LIST[0]->GroupID);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"AC IP LIST id:	%d\n",IPLIST->AC_IP_LIST[0]->GroupID);
			vty_out(vty,"IFNAME : %s\n",IPLIST->AC_IP_LIST[0]->ifname);
			vty_out(vty,"IFADDR : %s\n",IPLIST->AC_IP_LIST[0]->ipaddr);
			vty_out(vty,"LoadBanlance : %s\n",(IPLIST->AC_IP_LIST[0]->load_banlance == 1 )? en :dis);
			vty_out(vty,"Load diffnum : %d\n",IPLIST->AC_IP_LIST[0]->diff_count);
			if((IPLIST)&&(IPLIST->AC_IP_LIST[0])&&(IPLIST->AC_IP_LIST[0]->ip_list))
			{		
				struct wid_ac_ip *head = NULL;
				head = IPLIST->AC_IP_LIST[0]->ip_list;
				while(head)
				{	
					
					vty_out(vty,"AC ip: %-16s priority: %-5d  threshold: %-5d currentwtpnum: %-5d\n",head->ip, head->priority,head->threshold,head->wtpcount);
					//free(iplist[i]);
					head = head->next;
				}
				//free(iplist);
			}
			vty_out(vty,"==============================================================================\n");
			dcli_ac_ip_list_free_fun(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE,IPLIST);
		}
		else if(ret == WID_EBR_NOT_EXIST)
		{
			vty_out(vty,"<error> id does not exist\n");
		}
		else 
		{	
			vty_out(vty,"<error>  %d\n",ret);
		}
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
				IPLIST = dcli_ac_ip_list_show_api_group(
					profile,
					ID,
					0,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0)
				{
					vty_out(vty,"AC IP LIST %d infomation\n",IPLIST->AC_IP_LIST[0]->GroupID);
					vty_out(vty,"========================================================================\n");
					vty_out(vty,"AC IP LIST id:	%d\n",IPLIST->AC_IP_LIST[0]->GroupID);
					vty_out(vty,"IFNAME : %s\n",IPLIST->AC_IP_LIST[0]->ifname);
					vty_out(vty,"IFADDR : %s\n",IPLIST->AC_IP_LIST[0]->ipaddr);
					vty_out(vty,"LoadBanlance : %s\n",(IPLIST->AC_IP_LIST[0]->load_banlance == 1 )? en :dis);
					vty_out(vty,"Load diffnum : %d\n",IPLIST->AC_IP_LIST[0]->diff_count);
					if((IPLIST)&&(IPLIST->AC_IP_LIST[0])&&(IPLIST->AC_IP_LIST[0]->ip_list))
					{		
						struct wid_ac_ip *head = NULL;
						head = IPLIST->AC_IP_LIST[0]->ip_list;
						while(head)
						{	
							
							vty_out(vty,"AC ip: %-16s priority: %-5d  threshold: %-5d currentwtpnum: %-5d\n",head->ip, head->priority,head->threshold,head->wtpcount);
							head = head->next;
						}
					}
					dcli_ac_ip_list_free_fun(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE,IPLIST);
				}
				else if(ret == WID_EBR_NOT_EXIST)
				{
					vty_out(vty,"<error> id does not exist\n");
				}
				else 
				{	
					vty_out(vty,"<error>  %d\n",ret);
				}
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
			localid_pub = (unsigned char)localid;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				IPLIST = dcli_ac_ip_list_show_api_group(
					profile,
					ID,
					0,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0)
				{
					vty_out(vty,"AC IP LIST %d infomation\n",IPLIST->AC_IP_LIST[0]->GroupID);
					vty_out(vty,"========================================================================\n");
					vty_out(vty,"AC IP LIST id:	%d\n",IPLIST->AC_IP_LIST[0]->GroupID);
					vty_out(vty,"IFNAME : %s\n",IPLIST->AC_IP_LIST[0]->ifname);
					vty_out(vty,"IFADDR : %s\n",IPLIST->AC_IP_LIST[0]->ipaddr);
					vty_out(vty,"LoadBanlance : %s\n",(IPLIST->AC_IP_LIST[0]->load_banlance == 1 )? en :dis);
					vty_out(vty,"Load diffnum : %d\n",IPLIST->AC_IP_LIST[0]->diff_count);
					if((IPLIST)&&(IPLIST->AC_IP_LIST[0])&&(IPLIST->AC_IP_LIST[0]->ip_list))
					{		
						struct wid_ac_ip *head = NULL;
						head = IPLIST->AC_IP_LIST[0]->ip_list;
						while(head)
						{	
							
							vty_out(vty,"AC ip: %-16s priority: %-5d  threshold: %-5d currentwtpnum: %-5d\n",head->ip, head->priority,head->threshold,head->wtpcount);
							head = head->next;
						}
					}
					dcli_ac_ip_list_free_fun(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE,IPLIST);
				}
				else if(ret == WID_EBR_NOT_EXIST)
				{
					vty_out(vty,"<error> id does not exist\n");
				}
				else 
				{	
					vty_out(vty,"<error>  %d\n",ret);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;
}
DEFUN(show_ac_ip_list_cmd_func,
	  show_ac_ip_list_cmd,
	  "show ac-ip-list (all|list) [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "ac-ip-list\n"
	  "all of ac-ip-list\n"
	  "all of ac-ip-list\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret;
	int i = 0;
	int num = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	wid_ac_ip_group *ACIPLIST[ACIPLIST_NUM];
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	char en[] = "enable";
	char dis[] = "disable";	
	
	
	int index = 0;
	int localid = 1;
	unsigned char localid_pub =1;
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
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST);*/
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
			
			ACIPLIST[i] = (wid_ac_ip_group *)malloc(sizeof(wid_ac_ip_group));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ACIPLIST[i]->GroupID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ACIPLIST[i]->ifname));

			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
#endif
	DCLI_AC_IP_LIST_API_GROUP * IPLIST = NULL;
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
		IPLIST = dcli_ac_ip_list_show_api_group(
			index,
			0,
			0,
			&ret,
			&localid_pub,
			dcli_dbus_connection,
			WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST
			);
	    vty_out(vty,"ac-ip-list list summary:\n");	
	    vty_out(vty,"%d ac-ip-list exist\n",(ret != 0)?0:IPLIST->ip_list_num);
	    vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-16s %-15s %-10s\n","GroupID","IFName","LoadDiffNum","LoadBanlance");
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		if(ret == 0)
		{
			for (i = 0; i < IPLIST->ip_list_num; i++) 
			{	
				vty_out(vty,"%-17d %-16s %-15d %-10s\n",IPLIST->AC_IP_LIST[i]->GroupID,IPLIST->AC_IP_LIST[i]->ifname,\
				IPLIST->AC_IP_LIST[i]->diff_count,(IPLIST->AC_IP_LIST[0]->load_banlance == 1 )? en :dis);
			
				/*if(ACIPLIST[i])
				{
					free(ACIPLIST[i]);
					ACIPLIST[i] = NULL;	
				}*/
			}
			dcli_ac_ip_list_free_fun(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST,IPLIST);
		}
		vty_out(vty,"==============================================================================\n");
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
				IPLIST = dcli_ac_ip_list_show_api_group(
					profile,
					0,
					0,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
			    vty_out(vty,"ac-ip-list list summary:\n");	
			    vty_out(vty,"%d ac-ip-list exist\n",(ret != 0)?0:IPLIST->ip_list_num);
			    vty_out(vty,"========================================================================\n");
				vty_out(vty,"%-17s %-16s %-15s %-10s\n","GroupID","IFName","LoadDiffNum","LoadBanlance");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0)
				{
					for (i = 0; i < IPLIST->ip_list_num; i++) 
					{	
						vty_out(vty,"%-17d %-16s %-15d %-10s\n",IPLIST->AC_IP_LIST[i]->GroupID,IPLIST->AC_IP_LIST[i]->ifname,\
						IPLIST->AC_IP_LIST[i]->diff_count,(IPLIST->AC_IP_LIST[0]->load_banlance == 1 )? en :dis);
					}
					dcli_ac_ip_list_free_fun(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST,IPLIST);
				}
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
			localid_pub = (unsigned char)localid;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				IPLIST = dcli_ac_ip_list_show_api_group(
					profile,
					0,
					0,
					&ret,
					&localid_pub,
					dcli_dbus_connection,
					WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
			    vty_out(vty,"ac-ip-list list summary:\n");	
			    vty_out(vty,"%d ac-ip-list exist\n",(ret != 0)?0:IPLIST->ip_list_num);
			    vty_out(vty,"========================================================================\n");
				vty_out(vty,"%-17s %-16s %-15s %-10s\n","GroupID","IFName","LoadDiffNum","LoadBanlance");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				if(ret == 0)
				{
					for (i = 0; i < IPLIST->ip_list_num; i++) 
					{	
						vty_out(vty,"%-17d %-16s %-15d %-10s\n",IPLIST->AC_IP_LIST[i]->GroupID,IPLIST->AC_IP_LIST[i]->ifname,\
						IPLIST->AC_IP_LIST[i]->diff_count,(IPLIST->AC_IP_LIST[0]->load_banlance == 1 )? en :dis);
					}
					dcli_ac_ip_list_free_fun(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST,IPLIST);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	return CMD_SUCCESS;

}



DEFUN(config_ail_cmd_func,
	  config_ail_cmd,
	  "config ac-ip-list ID",
	  CONFIG_STR
	  "Wireless LAN information\n"
	  "id that you want to config\n"
	 )
{	int ret;
	unsigned char id;
	DBusMessage *query, *reply;
	DBusError err;
	ret = parse_char_ID((char*)argv[0], &id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				return CMD_WARNING;
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
		return CMD_WARNING;
			}
			return CMD_WARNING;
	}	
	if(id >= ACIPLIST_NUM || id == 0){
		vty_out(vty,"<error> id should be 1 to %d\n",ACIPLIST_NUM-1);
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
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_CONFIG);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_CONFIG);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&id,
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
				vty->node = ACIPLIST_NODE;
				vty->index = (void *)id;
			}else if(vty->node == HANSI_NODE){
				vty->node = HANSI_ACIPLIST_NODE;
				vty->index_sub = (void *)id;
			}
			else if(vty->node == LOCAL_HANSI_NODE){
				vty->node = LOCAL_HANSI_ACIPLIST_NODE;
				vty->index_sub = (void *)id;
			}
		}else if (ret == WLAN_ID_NOT_EXIST){
			vty_out(vty,"<error> id does not exist\n");
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


DEFUN(create_ac_ip_list_cmd_func,
		create_ac_ip_list_cmd,
		"create ac-ip-list ID base IFNAME",
		"create bak ac ip list server\n"
		"ac ip list group\n"
		"assign ID for group\n"
		"assign IFNAME\n"
	)
{
	int ret,len;
	unsigned char isAdd;	
	unsigned char id;
	char *name;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
	
	ret = parse_char_ID((char*)argv[0], &id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(id >= ACIPLIST_NUM || id == 0){
		vty_out(vty,"<error> id should be 1 to %d\n",ACIPLIST_NUM-1);
		return CMD_SUCCESS;
	}
	len = strlen(argv[1]);
	if(len > 16){		
		vty_out(vty,"<error> ifname is too long,out of the limit of 16\n");
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
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP_LIST_GROUP);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP_LIST_GROUP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&id,
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
		if(name)
		{
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0){
				vty_out(vty," %d was successfully created.\n",id);
		}
		else if(ret == INTERFACE_HAVE_NO_IP_ADDR){
			vty_out(vty,"%s have no ip address,please binding ip address first!!\n",argv[1]);
		}
		else if(ret == WLAN_ID_BE_USED)
			vty_out(vty,"<error> id exist\n");
		else if(ret == IF_BINDING_FLAG){
			vty_out(vty,"<error>interface %s has be binded in other hansi.\n",name);
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(name);
	return CMD_SUCCESS;	
}

DEFUN(del_ac_ip_list_cmd_func,
		del_ac_ip_list_cmd,
		"delete ac-ip-list ID",
		"delete ac-ip-list server\n"
		"ac ip list group service\n"
		"ID of ac ip list group\n"
	)
{
	int ret;
	unsigned char id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	ret = parse_char_ID((char*)argv[0], &id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(id >= ACIPLIST_NUM || id == 0){
		vty_out(vty,"<error> id should be 1 to %d\n",ACIPLIST_NUM-1);
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
		 
	}else if (vty->node == LOCAL_HANSI_NODE){
	
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP_LIST_GROUP);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP_LIST_GROUP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&id,
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

		if(ret == 0){
			vty_out(vty,"ac ip list group %d was successfully deleted.\n",id);
		}
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan id does not exist\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan is enable,please disable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(add_ac_ip_cmd_func,
	  add_ac_ip_cmd,
	  "add ac ip IP priority <1-100>",
	  "add ac ip IP \n"
	  "ip like A.B.C.D\n" 
	  "assign ip priority\n"
	 )
{

	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	unsigned char priority = 0;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	ret = WID_Check_IP_Format((char*)argv[0]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_char_ID((char*)argv[1], &priority);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	//ID = (int)vty->index;
	
	ip = (char*)malloc(strlen(argv[0])+1);
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));	

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_BYTE,&priority,
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
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"successfully.\n");
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface %s dose not exist\n",ip);
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ip);
	ip = NULL;
	return CMD_SUCCESS;
}

DEFUN(del_ac_ip_cmd_func,
	  del_ac_ip_cmd,
	  "del ac ip IP",
	  "del ac ip IP \n"
	  "ip like A.B.C.D\n" 
	 )
{

	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	ret = WID_Check_IP_Format((char*)argv[0]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	
	//ID = (int)vty->index;
	
	ip = (char*)malloc(strlen(argv[0])+1);
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));	

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
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
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"successfully.\n");
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface %s dose not exist\n",ip);
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ip);
	ip = NULL;
	return CMD_SUCCESS;
}


DEFUN(modify_ac_ip_priority_cmd_func,
	  modify_ac_ip_priority_cmd,
	  "modify ac ip IP priority <1-100>",
	  "ac ip IP \n"
	  "ip like A.B.C.D\n" 
	  "assign ip priority\n"
	 )
{

	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	unsigned char priority = 0;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	ret = WID_Check_IP_Format((char*)argv[0]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_char_ID((char*)argv[1], &priority);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	//ID = (int)vty->index;
	
	ip = (char*)malloc(strlen(argv[0])+1);
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));	

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_PRIORITY);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_PRIORITY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_BYTE,&priority,
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
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"successfully.\n");
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface %s dose not exist\n",ip);
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ip);
	ip = NULL;
	return CMD_SUCCESS;
}

DEFUN(set_ac_ip_list_banlance_cmd_func,
	  set_ac_ip_list_banlance_cmd,
	  "set ac_ip_list banlance (enable|disable)",
	  "set ac_ip_list banlance flag\n"
	  "set ac_ip_list banlance flag enable or disable" 
	 )
{

	int ret;
	unsigned char ID = 0;
	unsigned char banlanceflag = 0;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	if (!strcmp(argv[0],"enable"))
	{
		banlanceflag = 1;	
	}		

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_BANLANCE_FLAG);
		

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_BYTE,&banlanceflag,
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
	{
		vty_out(vty,"successfully.\n");
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface dose not exist\n");
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_ac_ip_threshold_cmd_func,
	  set_ac_ip_threshold_cmd,
	  "set ac ip IP threshold VALUE",
	  "set ac ip IP \n"
	  "ip like A.B.C.D\n" 
	  "assign ip threshold value\n"
	 )
{

	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	unsigned int threshold = 0;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	ret = WID_Check_IP_Format((char*)argv[0]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char*)argv[1], &threshold);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	//ID = (int)vty->index;
	
	ip = (char*)malloc(strlen(argv[0])+1);
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));	

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_THRESHOLD);
		
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_UINT32,&threshold,
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
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"successfully.\n");
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface %s dose not exist\n",ip);
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else{
		vty_out(vty,"<error> ip addr no exit %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ip);
	ip = NULL;
	return CMD_SUCCESS;
}


DEFUN(set_ac_ip_diffnum_banlance_cmd_func,
	  set_ac_ip_diffnum_banlance_cmd,
	  "set ac_ip_list diffnum banlance VALUE",
	  "set ac_ip_list banlance value\n"
	  "set ac_ip_list banlance value" 
	 )
{

	int ret;
	unsigned char ID = 0;
	unsigned int diffnum = 0;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = parse_int_ID((char*)argv[0], &diffnum);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_DIFF_BANLANCE);
		

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_UINT32,&diffnum,
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
	{
		vty_out(vty,"successfully.\n");
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface dose not exist\n");
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_ac_ip_list_show_running_config(struct vty*vty) {	
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
	ReInitDbusPath_V2(local,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show ac_ip_list config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"AC_IP_LIST");
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

	return 0;	
}

/**wangchao move to dcli_wireless_main.c***/
#if 0
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

#endif

void dcli_aciplist_init(void) {
	install_element(VIEW_NODE,&show_ac_ip_list_one_cmd);
	install_element(VIEW_NODE,&show_ac_ip_list_cmd);
#if 0
	install_node(&ac_ip_list_node,dcli_ac_ip_list_show_running_config, "ACIPLIST_NODE");
	install_default(ACIPLIST_NODE);	
	/*------------------------------VIEW_NODE------------------------------*/
	install_element(VIEW_NODE,&show_ac_ip_list_one_cmd);											/*a1*/
	install_element(VIEW_NODE,&show_ac_ip_list_cmd);												/*a2*/
	
	/*------------------------------CONFIG_NODE----------------------------*/
	install_element(CONFIG_NODE,&show_ac_ip_list_one_cmd);											/*a1*/
	install_element(CONFIG_NODE,&show_ac_ip_list_cmd);												/*a2*/
	
	install_element(CONFIG_NODE,&create_ac_ip_list_cmd);	
	install_element(CONFIG_NODE,&del_ac_ip_list_cmd);
	install_element(CONFIG_NODE,&config_ail_cmd);
	
	/*------------------------------ACIPLIST_NODE----------------------------*/
	install_element(ACIPLIST_NODE,&add_ac_ip_cmd);	
	install_element(ACIPLIST_NODE,&del_ac_ip_cmd);	
	install_element(ACIPLIST_NODE,&modify_ac_ip_priority_cmd);	
	install_element(ACIPLIST_NODE,&set_ac_ip_list_banlance_cmd);	
	install_element(ACIPLIST_NODE,&set_ac_ip_threshold_cmd);
	install_element(ACIPLIST_NODE,&set_ac_ip_diffnum_banlance_cmd);	
#endif	
	/********************************************************************************/
	install_node(&hansi_ac_ip_list_node,NULL, "HANSI_ACIPLIST_NODE");
	install_default(HANSI_ACIPLIST_NODE);

	install_node(&local_hansi_ac_ip_list_node,NULL, "LOCAL_HANSI_ACIPLIST_NODE");
	install_default(LOCAL_HANSI_ACIPLIST_NODE);
	
	/*------------------------------HANSI_NODE----------------------------*/
	install_element(HANSI_NODE,&show_ac_ip_list_one_cmd);										/*a1*/
	install_element(HANSI_NODE,&show_ac_ip_list_cmd);											/*a2*/
	
	install_element(HANSI_NODE,&create_ac_ip_list_cmd);	
	install_element(HANSI_NODE,&del_ac_ip_list_cmd);
	install_element(HANSI_NODE,&config_ail_cmd);

	install_element(LOCAL_HANSI_NODE,&show_ac_ip_list_one_cmd);										/*a1*/
	install_element(LOCAL_HANSI_NODE,&show_ac_ip_list_cmd);											/*a2*/
	
	install_element(LOCAL_HANSI_NODE,&create_ac_ip_list_cmd);	
	install_element(LOCAL_HANSI_NODE,&del_ac_ip_list_cmd);
	install_element(LOCAL_HANSI_NODE,&config_ail_cmd);
	
	/*------------------------------HANSI_ACIPLIST_NODE----------------------------*/
	install_element(HANSI_ACIPLIST_NODE,&add_ac_ip_cmd);	
	install_element(HANSI_ACIPLIST_NODE,&del_ac_ip_cmd);	
	install_element(HANSI_ACIPLIST_NODE,&modify_ac_ip_priority_cmd);	
	install_element(HANSI_ACIPLIST_NODE,&set_ac_ip_list_banlance_cmd);
	install_element(HANSI_ACIPLIST_NODE,&set_ac_ip_threshold_cmd);
	install_element(HANSI_ACIPLIST_NODE,&set_ac_ip_diffnum_banlance_cmd);		

	install_element(LOCAL_HANSI_ACIPLIST_NODE,&add_ac_ip_cmd);	
	install_element(LOCAL_HANSI_ACIPLIST_NODE,&del_ac_ip_cmd);	
	install_element(LOCAL_HANSI_ACIPLIST_NODE,&modify_ac_ip_priority_cmd);	
	install_element(LOCAL_HANSI_ACIPLIST_NODE,&set_ac_ip_list_banlance_cmd);
	install_element(LOCAL_HANSI_ACIPLIST_NODE,&set_ac_ip_threshold_cmd);
	install_element(LOCAL_HANSI_ACIPLIST_NODE,&set_ac_ip_diffnum_banlance_cmd);		
	

	return;
}

#endif


