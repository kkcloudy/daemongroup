#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"

#include "dcli_main.h"
#include "dcli_wlan.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"

#include "asd_ac_roaming.h"



struct cmd_node ac_group_node =
{
	AC_GROUP_NODE,
	"%s(config-ac-group)# "
};

struct cmd_node hansi_ac_group_node =
{
	HANSI_AC_GROUP_NODE,
	"%s(hansi-ac-group %d-%d-%d)# "
};

struct cmd_node local_hansi_ac_group_node =
{
	LOCAL_HANSI_AC_GROUP_NODE,
	"%s(local-hansi-ac-group %d-%d-%d)# "
};


DEFUN(config_ac_group_cmd_func,
	  config_ac_group_cmd,
	  "config ac-mobility-group ID",
	  CONFIG_STR
	  "Wireless LAN information\n"
	  "ac group id that you want to config\n"
	 )
{	int ret;
	unsigned char group_id,status,wlanid;
    /*wlan_id = atoi(argv[0]);*/
	DBusMessage *query, *reply;
	DBusError err;
	ret = parse_char_ID((char*)argv[0], &group_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_WARNING;
	}	
	if(group_id >= GROUP_NUM || group_id == 0){
		vty_out(vty,"<error> group id should be 1 to %d\n",GROUP_NUM-1);
		return CMD_WARNING;
	}
	int localid = 1;
	int index;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_CONFIG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&group_id,
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
				vty->node = AC_GROUP_NODE;
				vty->index = (void *)group_id;
			}
			else if(vty->node == HANSI_NODE){	
				vty->node = HANSI_AC_GROUP_NODE;
				vty->index_sub = (void *)group_id;
			}else if(vty->node == LOCAL_HANSI_NODE){	
				vty->node = LOCAL_HANSI_AC_GROUP_NODE;
				vty->index_sub = (void *)group_id;
			}
		}else if (ret == ASD_AC_GROUP_ID_NOT_EXIST){
			vty_out(vty,"<error> group id does not exist\n");
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

#if DCLI_NEW
	
DEFUN(show_ac_moblility_group_one_func,
	  show_ac_moblility_group_one_cmd,
	  "show ac-mobility-group ID",
	  SHOW_STR
	  "ac-mobility-group\n"
	  "ac-mobility-group\n"
	  "ID of ac-mobility-group\n"
	 )
{	
	int ret,index;
	unsigned char ID = 0;
	int i = 0;
	unsigned int num = 0;
	Inter_AC_R_Group_T *ACGROUP ;	
	
	unsigned char *ip;
	char en[] = "enable";
	char dis[] = "disable";
	
	
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
	if(ID >= GROUP_NUM || ID == 0){
		vty_out(vty,"<error> AC MOBILITY GROUP id should be 1 to %d\n",GROUP_NUM-1);
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ACGROUP = show_ac_mobility_group(localid,dcli_dbus_connection, index, ID, &num, &ret);
	
	if((ret == 0)&&(ACGROUP != NULL))
	{
		vty_out(vty,"AC Mobility Group %d infomation\n",ACGROUP->GroupID);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"AC IP LIST id:	%d\n",ACGROUP->GroupID);
		vty_out(vty,"Gname : %s\n",ACGROUP->name);
		vty_out(vty,"ESSID : %s\n",ACGROUP->ESSID);
		ip = (unsigned char *)&(ACGROUP->host_ip);
		vty_out(vty,"Host IP : %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
		if(num != 0)
		{
			for(i=0;i<num;i++)
			{
				ip = (unsigned char *)&(ACGROUP->Mobility_AC[i]->ACIP);
				vty_out(vty,"AC Member: %-2d ip: %d.%d.%d.%d\n",ACGROUP->Mobility_AC[i]->ACID,ip[0],ip[1],ip[2],ip[3]);
				free(ACGROUP->Mobility_AC[i]);
			}
			free(ACGROUP->Mobility_AC);
		}
		if(ACGROUP)
		{
			free(ACGROUP);
			ACGROUP = NULL;
		}
		vty_out(vty,"==============================================================================\n");
		
	}
	else if(ret == ASD_AC_GROUP_ID_NOT_EXIST){
		vty_out(vty,"<error> id %d does not exist\n",ID);
	}
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else{	
		vty_out(vty,"<error>  %d\n",ret);
	}
	return CMD_SUCCESS;
}

#else
DEFUN(show_ac_moblility_group_one_func,
	  show_ac_moblility_group_one_cmd,
	  "show ac-mobility-group ID",
	  SHOW_STR
	  "ac-mobility-group\n"
	  "ac-mobility-group\n"
	  "ID of ac-mobility-group\n"
	 )
{	
	int ret;
	unsigned char ID = 0;
	int i = 0;
	unsigned int num = 0;
	Inter_AC_R_Group_T *ACGROUP;	
	Mobility_AC_Info_T **ACM;
	unsigned char *ip;
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
	if(ID >= GROUP_NUM || ID == 0){
		vty_out(vty,"<error> AC MOBILITY GROUP id should be 1 to %d\n",GROUP_NUM-1);
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP);

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
		ACGROUP = (Inter_AC_R_Group_T *)malloc(sizeof(Inter_AC_R_Group_T));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->GroupID));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->ESSID));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->name));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ACGROUP->host_ip));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
		
		ACM = malloc(num* sizeof(struct Mobility_AC_Info_T *));
		memset(ACM, 0, num*sizeof(struct Mobility_AC_Info_T*));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			ACM[i] = (Mobility_AC_Info_T *)malloc(sizeof(Mobility_AC_Info_T));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ACM[i]->ACID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ACM[i]->ACIP));
		
			dbus_message_iter_next(&iter_array);
		}
	}
		
	dbus_message_unref(reply);
	if(ret == 0)
	{
		vty_out(vty,"AC Mobility Group %d infomation\n",ACGROUP->GroupID);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"AC IP LIST id:	%d\n",ACGROUP->GroupID);
		vty_out(vty,"Gname : %s\n",ACGROUP->name);
		vty_out(vty,"ESSID : %s\n",ACGROUP->ESSID);
		ip = (unsigned char *)&(ACGROUP->host_ip);
		vty_out(vty,"Host IP : %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
		if(num != 0)
		{
			for(i=0;i<num;i++)
			{
				ip = (unsigned char *)&(ACM[i]->ACIP);
				vty_out(vty,"AC Member: %-2d ip: %d.%d.%d.%d\n",ACM[i]->ACID,ip[0],ip[1],ip[2],ip[3]);
				free(ACM[i]);
			}
			free(ACM);
		}
		if(ACGROUP)
		{
			free(ACGROUP);
			ACGROUP = NULL;
		}
		vty_out(vty,"==============================================================================\n");
		
	}
	else if(ret == ASD_AC_GROUP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> id does not exist\n");
	}
	else 
	{	
		vty_out(vty,"<error>  %d\n",ret);
	}
	return CMD_SUCCESS;
}
#endif


#if DCLI_NEW

DEFUN(show_ac_group_list_cmd_func,
	  show_ac_group_list_cmd,
	  "show  ac-mobility-group (all|list)",
	  SHOW_STR
	  "ac-mobility-group\n"
	  "ac-mobility-group\n"
	  "all of ac-mobility-group\n"
	 )
{	
	int ret,index ;
	int i = 0;
	int num = 0;	
	Inter_AC_R_Group_T **ACGROUP;
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			
	ACGROUP = show_ac_mobility_list(localid,dcli_dbus_connection, index, &num, &ret);
				
    vty_out(vty,"AC Mobility Group list summary:\n");	
    vty_out(vty,"%d AC Mobility Group exist\n",num);
    vty_out(vty,"==============================================================================\n");
	vty_out(vty,"%-17s %-16s %s\n","GroupID","ESSID","GroupName");
	if((ret == 0) && (ACGROUP != NULL))
	{
		for (i = 0; i < num; i++) 
		{	
			vty_out(vty,"%-17d %-16s %s\n",ACGROUP[i]->GroupID,ACGROUP[i]->ESSID,ACGROUP[i]->name);
		
			if(ACGROUP[i])
			{
				free(ACGROUP[i]);
				ACGROUP[i] = NULL;	
			}		
		}
				
		vty_out(vty,"==============================================================================\n");
	}	
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error>");

	free(ACGROUP);
	
	
	return CMD_SUCCESS;

}

#else
DEFUN(show_ac_group_list_cmd_func,
	  show_ac_group_list_cmd,
	  "show  ac-mobility-group (all|list)",
	  SHOW_STR
	  "ac-mobility-group\n"
	  "ac-mobility-group\n"
	  "all of ac-mobility-group\n"
	 )
{	
	int ret;
	int i = 0;
	int num = 0;
	Inter_AC_R_Group_T *ACGROUP[GROUP_NUM];
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_LIST);

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
			
			ACGROUP[i] = (Inter_AC_R_Group_T *)malloc(sizeof(Inter_AC_R_Group_T));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ACGROUP[i]->GroupID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ACGROUP[i]->ESSID));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ACGROUP[i]->name));

			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
	
    vty_out(vty,"AC Mobility Group list summary:\n");	
    vty_out(vty,"%d AC Mobility Group exist\n",num);
    vty_out(vty,"==============================================================================\n");
	vty_out(vty,"%-17s %-16s %s\n","GroupID","ESSID","GroupName");
	if(ret == 0)
	{
		for (i = 0; i < num; i++) 
		{	
			vty_out(vty,"%-17d %-16s %s\n",ACGROUP[i]->GroupID,ACGROUP[i]->ESSID,ACGROUP[i]->name);
		
			if(ACGROUP[i])
			{
				free(ACGROUP[i]);
				ACGROUP[i] = NULL;	
			}
		}
	}
	vty_out(vty,"==============================================================================\n");
	return CMD_SUCCESS;

}
#endif

DEFUN(create_ac_group_cmd_func,
		create_ac_group_cmd,
		"create ac-mobility-group GID GNAME base ESSID",
		"create wireless server\n"
		"ac mobility service\n"
		"assign AC GROUP ID\n"
		"assign AC GROUP NAME\n"
		"base ESSID\n"
	)
{
	int ret,len;
	unsigned char isAdd;	
	unsigned char group_id;
	char *name;
	char *ESSID;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
/*	wlan_id = atoi(argv[0]);*/
	
	ret = parse_char_ID((char*)argv[0], &group_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(group_id >= GROUP_NUM || group_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",GROUP_NUM-1);
		return CMD_SUCCESS;
	}
	len = strlen(argv[1]);
	if(len > 15){		
		vty_out(vty,"<error> wlan name is too long,out of the limit of 15\n");
		return CMD_SUCCESS;
	}
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));		
	len = strlen(argv[2]);
	if(len > 32){		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	ESSID = (char*)malloc(strlen(argv[2])+1);
	memset(ESSID, 0, strlen(argv[2])+1);
	memcpy(ESSID, argv[2], strlen(argv[2]));
	int localid = 1;
	int index;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_NODE)
	{
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
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_CREATE_GROUP);
	//query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_AC_GROUP_OBJPATH,\
	//					ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_CREATE_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&group_id,
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_STRING,&ESSID, 
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
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0){//qiuchen fixed it
		vty_out(vty,"group %d was successfully created.\n",group_id);
	}
	else if(ret == ASD_AC_GROUP_ID_USED)
		vty_out(vty,"<error> group id exist\n");
	else if(ret == ASD_AC_GROUP_ESSID_NOT_EXIST)
		vty_out(vty,"<error> essid %s does not exist\n",argv[2]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(name);
	free(ESSID);

	return CMD_SUCCESS;	
}

DEFUN(delete_ac_group_cmd_func,
		delete_ac_group_cmd,
		"delete ac-mobility-group GID",
		"delete wireless server\n"
		"ac mobility service\n"
		"assign AC GROUP ID\n"
	)
{
	int ret,len;
	unsigned char group_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = parse_char_ID((char*)argv[0], &group_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(group_id >= GROUP_NUM || group_id == 0){
		vty_out(vty,"<error> ac id should be 1 to %d\n",GROUP_NUM-1);
		return CMD_SUCCESS;
	}
	int localid = 1;
	int index;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_DELETE_GROUP);
//	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_AC_GROUP_OBJPATH,\
//						ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_DELETE_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&group_id,
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
		vty_out(vty,"group %d was successfully deleted.\n",group_id);
	}
	else if(ret == ASD_AC_GROUP_ID_NOT_EXIST){
		vty_out(vty,"group %d does not exist.\n",group_id);
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}


DEFUN(add_ac_group_member_cmd_func,
		add_ac_group_member_cmd,
		"add ac ACID ACIP as member",
		"add ac member\n"
		"ac mobility service\n"
		"assign AC ID\n"
		"assign AC IP\n"
	)
{
	int ret,len;
	unsigned char isAdd;	
	unsigned char ac_id;
	char *ip;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
/*	wlan_id = atoi(argv[0]);*/
	unsigned char group_id;
	group_id = (unsigned char)vty->index;
	ret = parse_char_ID((char*)argv[0], &ac_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(ac_id >= G_AC_NUM || ac_id == 0){
		vty_out(vty,"<error> ac id should be 1 to %d\n",G_AC_NUM-1);
		return CMD_SUCCESS;
	}
	ret = Check_IP_Format((char*)argv[1]);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}

	ip = (char*)malloc(strlen(argv[1])+1);
	memset(ip, 0, strlen(argv[1])+1);
	memcpy(ip, argv[1], strlen(argv[1]));		
	int localid = 1;
	int index;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == AC_GROUP_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_AC_GROUP_NODE){
		index = vty->index; 		
		group_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_AC_GROUP_NODE){
		index = vty->index; 		
		group_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_ADD_GROUP_MEMBER);

//	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_AC_GROUP_OBJPATH,\
//						ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_ADD_GROUP_MEMBER);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&group_id,								
						DBUS_TYPE_BYTE,&ac_id,
						DBUS_TYPE_STRING,&ip,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
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

	if(ret == 0){
		vty_out(vty,"add ac %s %s as member successfully.\n",argv[0],argv[1]);
	}
	else if(ret == ASD_AC_GROUP_MEMBER_EXIST){
		vty_out(vty,"groupid %s exists already.\n",argv[0]);
	}
	else if(ret == ASD_AC_GROUP_ID_NOT_EXIST){
		vty_out(vty,"groupid %s does not exist.\n",argv[0]);
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(ip);

	return CMD_SUCCESS;	
}

DEFUN(del_ac_group_member_cmd_func,
		del_ac_group_member_cmd,
		"del ac ACID",
		"del ac member\n"
		"ac mobility service\n"
		"assign AC ID\n"
	)
{
	int ret,len;
	unsigned char ac_id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char group_id;
	group_id = (unsigned char)vty->index;
	ret = parse_char_ID((char*)argv[0], &ac_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(ac_id >= G_AC_NUM || ac_id == 0){
		vty_out(vty,"<error> AC id should be 1 to %d\n",G_AC_NUM-1);
		return CMD_SUCCESS;
	}
	int localid = 1;
	int index;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == AC_GROUP_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_AC_GROUP_NODE){
		index = vty->index; 		
		group_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_AC_GROUP_NODE){
		index = vty->index; 		
		group_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_DEL_GROUP_MEMBER);
//	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_AC_GROUP_OBJPATH,\
//						ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_DEL_GROUP_MEMBER);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&group_id,								
						DBUS_TYPE_BYTE,&ac_id,
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
		vty_out(vty,"del ac %s successfully.\n",argv[0]);
	}
	else if(ret == ASD_AC_GROUP_ID_NOT_EXIST){
		vty_out(vty,"groupid %d does not exist.\n",group_id);
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}


DEFUN(set_host_ip_cmd_func,
		set_host_ip_cmd,
		"set host ip ACIP",
		"set host ip\n"
		"assign AC IP\n"
	)
{
	int ret, ret1, ret2;
	unsigned char group_id;
	char *ip;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	group_id = (int)vty->index;

	ret1 = Check_IP_Format((char*)argv[0]);
	if(ret1 != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	
	ret2 = Check_Local_IP((char*)argv[0]);
	if (ret2 == -1){
		vty_out(vty,"<error> check_local_ip error\n");
		return CMD_SUCCESS;
	}else if(ret2 == 0){
		vty_out(vty,"<error> not local ip \n");
		return CMD_SUCCESS;
	}
	
	ip = (char *)malloc(strlen(argv[0])+1);
	memset(ip, 0, strlen(argv[0])+1);
	memcpy(ip, argv[0], strlen(argv[0]));
	int localid = 1;
	int index;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == AC_GROUP_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_AC_GROUP_NODE){
		index = vty->index; 		
		group_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_AC_GROUP_NODE){
		index = vty->index; 		
		group_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_HOST_IP);
//	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_AC_GROUP_OBJPATH,\
//						ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_HOST_IP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&group_id,
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
		vty_out(vty,"host ip set successfully \n");
	else if(ret == ASD_AC_GROUP_ID_NOT_EXIST){
		vty_out(vty,"groupid %d does not exist.\n",group_id);
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_inter_ac_roaming_count_cmd_func,
		show_inter_ac_roaming_count_cmd,
		"show inter_ac_roaming count",
		"show inter_ac_roaming count\n"
		"show inter_ac_roaming count\n"
	)
{
	int ret;
	int in;
	int out;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int localid = 1;
	int index = 0;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_ROAMING_COUNT);
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
	dbus_message_iter_get_basic(&iter,&in);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&out);

	vty_out(vty,"Inter AC Roaming total Count:  %d\n",ret);	
	vty_out(vty,"Inter AC Roaming in Count:  %d\n",in);
	vty_out(vty,"Inter AC Roaming out Count:  %d\n",out);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_ac_mobility_group_show_running_config(int localid,struct vty*vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	int local = 1;
	int slot_id = HostSlotId;
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(local,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show ac_mobility_group config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"AC_MOBILITY_GROUP");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
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

void dcli_ac_group_init(void) {
#if 0
	install_node(&ac_group_node,dcli_ac_mobility_group_show_running_config,"ACIPLIST_NODE");
	install_default(AC_GROUP_NODE);
	install_element(CONFIG_NODE,&show_ac_moblility_group_one_cmd);
	install_element(CONFIG_NODE,&show_ac_group_list_cmd);
	install_element(CONFIG_NODE,&config_ac_group_cmd);
	install_element(CONFIG_NODE,&create_ac_group_cmd);
	install_element(CONFIG_NODE,&delete_ac_group_cmd);	
	install_element(AC_GROUP_NODE,&add_ac_group_member_cmd);	
	install_element(AC_GROUP_NODE,&set_host_ip_cmd);
	install_element(AC_GROUP_NODE,&del_ac_group_member_cmd);
	install_element(CONFIG_NODE,&show_inter_ac_roaming_count_cmd);
#endif
	install_node(&hansi_ac_group_node,NULL,"HANSI_AC_GROUP_NODE");
	install_default(HANSI_AC_GROUP_NODE);

	install_element(HANSI_NODE,&show_ac_moblility_group_one_cmd);
	install_element(HANSI_NODE,&show_ac_group_list_cmd);
	install_element(HANSI_NODE,&config_ac_group_cmd);
	install_element(HANSI_NODE,&create_ac_group_cmd);
	install_element(HANSI_NODE,&delete_ac_group_cmd);	
	install_element(HANSI_NODE,&show_inter_ac_roaming_count_cmd);
	install_element(HANSI_AC_GROUP_NODE,&add_ac_group_member_cmd);	
	install_element(HANSI_AC_GROUP_NODE,&set_host_ip_cmd);
	install_element(HANSI_AC_GROUP_NODE,&del_ac_group_member_cmd);
	
	install_node(&local_hansi_ac_group_node,NULL,"LOCAL_HANSI_AC_GROUP_NODE");
	install_default(LOCAL_HANSI_AC_GROUP_NODE);
	
	install_element(LOCAL_HANSI_NODE,&show_ac_moblility_group_one_cmd);
	install_element(LOCAL_HANSI_NODE,&show_ac_group_list_cmd);
	install_element(LOCAL_HANSI_NODE,&config_ac_group_cmd);
	install_element(LOCAL_HANSI_NODE,&create_ac_group_cmd);
	install_element(LOCAL_HANSI_NODE,&delete_ac_group_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_inter_ac_roaming_count_cmd);
	install_element(LOCAL_HANSI_AC_GROUP_NODE,&add_ac_group_member_cmd);	
	install_element(LOCAL_HANSI_AC_GROUP_NODE,&set_host_ip_cmd);
	install_element(LOCAL_HANSI_AC_GROUP_NODE,&del_ac_group_member_cmd);
	return;
}

#endif
