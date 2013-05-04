#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"

#include "../dcli_main.h"
#include "dcli_ap_group.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dcli_wtp.h"
struct cmd_node ap_group_node =
{
	AP_GROUP_NODE,
	"%s(ap-group %d)# "
};
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
struct cmd_node ap_group_wtp_node =
{
	AP_GROUP_WTP_NODE,
	"%s(ap-group-wtp %d)# "
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
struct cmd_node ap_group_radio_node =
{
	AP_GROUP_RADIO_NODE,
	"%s(ap-group-radio %d)# "
};
struct cmd_node hansi_ap_group_radio_node =
{
	HANSI_AP_GROUP_RADIO_NODE,
	"%s(hansi-ap-group-radio %d-%d-%d)# ",
	1
};
struct cmd_node local_hansi_ap_group_radio_node =
{
	LOCAL_HANSI_AP_GROUP_RADIO_NODE,
	"%s(local-hansi-ap-group-radio %d-%d-%d)# ",
	1
};


DEFUN(config_ap_group_cmd_func,
	  config_ap_group_cmd,
	  "config ap-group ID",
	  CONFIG_STR
	  "Wireless LAN information\n"
	  "id that you want to config\n"
	 )
{	int ret;
	unsigned int id;
	DBusMessage *query, *reply;
	DBusError err;
	ret = parse_int_ID((char*)argv[0], &id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(id >= WTP_GROUP_NUM || id == 0){
		vty_out(vty,"<error> id should be 1 to %d\n",WTP_GROUP_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index;
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
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_CONFIG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&id,
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
			if(vty->node == CONFIG_NODE){
				vty->node = AP_GROUP_NODE;
				vty->index = (void *)id;
			}else if(vty->node == HANSI_NODE){
				vty->node = HANSI_AP_GROUP_NODE;
				vty->index_sub = (void *)id;
			}else if(vty->node == LOCAL_HANSI_NODE){
				vty->node = LOCAL_HANSI_AP_GROUP_NODE;
				vty->index_sub = (void *)id;
			}
		}else if (ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> id does not exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_ap_group_wtp_cmd_func,
	  config_ap_group_wtp_cmd,
	  "config ap-group-wtp",
	  CONFIG_STR
	  "Wireless LAN information\n"
	 )
{	

	if(vty->node == AP_GROUP_NODE){
		vty->node = AP_GROUP_WTP_NODE;
	}else if(vty->node == HANSI_AP_GROUP_NODE){
		vty->node = HANSI_AP_GROUP_WTP_NODE;
	}else if(vty->node == LOCAL_HANSI_AP_GROUP_NODE){
		vty->node = LOCAL_HANSI_AP_GROUP_WTP_NODE;
	}
	
	return CMD_SUCCESS;
}

DEFUN(config_ap_group_radio_cmd_func,
	  config_ap_group_radio_cmd,
	  "config ap-group-radio",
	  CONFIG_STR
	  "Wireless LAN information\n"
	 )
{	
	if(vty->node == AP_GROUP_NODE){
		vty->node = AP_GROUP_RADIO_NODE;
	}else if(vty->node == HANSI_AP_GROUP_NODE){
		vty->node = HANSI_AP_GROUP_RADIO_NODE;
	}else if(vty->node == LOCAL_HANSI_AP_GROUP_NODE){
		vty->node = LOCAL_HANSI_AP_GROUP_RADIO_NODE;
	}	
	return CMD_SUCCESS;
}


DEFUN(create_ap_group_cmd_func,
		create_ap_group_cmd,
		"create ap-group ID NAME",
		"create ap group profile\n"
		"ap group\n"
		"assign ID for group\n"
		"assign NAME\n"
	)
{
	int ret,len;
	unsigned char isAdd;	
	unsigned int id;
	char *name;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
	
	ret = parse_int_ID((char*)argv[0], &id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_WARNING;
	}	
	if(id >= WTP_GROUP_NUM || id == 0){
		vty_out(vty,"<error> id should be 1 to %d\n",WTP_GROUP_NUM-1);
		return CMD_WARNING;
	}
	len = strlen(argv[1]);
	if(len > 128){		
		vty_out(vty,"<error> name is too long,out of the limit of 128\n");
		return CMD_WARNING;
	}
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));		
	
	int index;
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
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_CREATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&id,
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
		return CMD_WARNING;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0){
				vty_out(vty,"ap group %d was successfully created.\n",id);
		}
		else if(ret == WLAN_ID_BE_USED){
			vty_out(vty,"<error> id exist\n");
			dbus_message_unref(reply);
			free(name);
			return CMD_WARNING;	
		}
		else{
			vty_out(vty,"<error>  %d\n",ret);
			dbus_message_unref(reply);
			free(name);
			return CMD_WARNING;	

		}	
	dbus_message_unref(reply);
	free(name);
	return CMD_SUCCESS;	
}

DEFUN(del_ap_group_cmd_func,
		del_ap_group_cmd,
		"delete ap-group ID",
		"delete ap-group profile\n"
		"ap-group profile\n"
		"ID of ap group\n"
	)
{
	int ret;
	unsigned int id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	ret = parse_int_ID((char*)argv[0], &id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(id >= WTP_GROUP_NUM || id == 0){
		vty_out(vty,"<error> id should be 1 to %d\n",WTP_GROUP_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index;
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
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_DEL);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&id,
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
			vty_out(vty,"ap group %d was successfully deleted.\n",id);
		}
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error>ap group id does not exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(add_del_ap_group_member_cmd_func,
		add_del_ap_group_member_cmd,
		"(add|delete) ap-group member WTPIDLIST",
		"add or delete ap-group member\n"
		"ap-group member\n"
		"ap-group member\n"
		"wtp id list :1,2,3,<10-20>,100\n"
	)
{
	int ret;
	unsigned int i = 0;
	unsigned int isadd = 1;	
	unsigned int GROUPID = 0;	
	update_wtp_list *wtplist = NULL;
	unsigned int *wtp_list = NULL;
	unsigned int apcount = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	if(strncmp("add",argv[0],(strlen(argv[0])>3)?3:strlen(argv[0]))==0){
		isadd = 1;
	}else if(strncmp("delete",argv[0],(strlen(argv[0])>6)?6:strlen(argv[0]))==0){
		isadd = 0;
	}else{
		vty_out(vty,"<error>unknown command\n");
		return CMD_SUCCESS;
	}
	
	wtplist = (struct tag_wtpid_list*)malloc(sizeof(struct tag_wtpid_list));
	wtplist->wtpidlist = NULL ; 	
	wtplist->count = 0;
	
	if (!strcmp(argv[1],"all"))
	{
		;	
	}else{
		ret = parse_wtpid_list((char*)argv[1],&wtplist);
		if(ret != 0)
		{
			struct tag_wtpid * tmp = wtplist->wtpidlist;
			while(tmp){
				printf("tmp->wtpid %d\n",tmp->wtpid);
				tmp = tmp->next;
			}
			vty_out(vty, "%% set wtp list error,like 1,8,9-20,33\n");
			destroy_input_wtp_list(wtplist);
			return CMD_WARNING;
		}
		else
		{
			delsame(wtplist);			
		}
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == AP_GROUP_NODE){
		index = 0;
		GROUPID = vty->index;
	}else if(vty->node == HANSI_AP_GROUP_NODE){
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		GROUPID = vty->index_sub;
	}
    else if (vty->node == LOCAL_HANSI_AP_GROUP_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = dcli_ap_group_add_del_member(localid,index,GROUPID,isadd,wtplist,&wtp_list,&apcount,dcli_dbus_connection);

	if(ret == 0){
		vty_out(vty,"%s ap group member successfully.\n",argv[0]);
		if(apcount != 0){
			vty_out(vty,"wtp ",argv[0]);
		}
		for(i = 0; i <apcount; i++){
			vty_out(vty,"%d,",wtp_list[i]);
		}
		if(apcount != 0){
			vty_out(vty,"%s ap group member failed.\n",argv[0]);
			free(wtp_list);
			wtp_list = NULL;
		}
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error>ap group id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;	
}

//zhangshu add,2010-09-16
DEFUN(show_group_member_func,
		show_group_member_cmd,
		"show group member",
		"show all wtps of group\n"
		"show all wtps of group\n"
	)
{
	int ret;
	unsigned int i = 0;
	unsigned int groupid = 0;	
	unsigned int *wtp_list = NULL;
	unsigned int apcount = 0;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
    
	if(vty->node == AP_GROUP_NODE)
	{
	    vty_out(vty,"AP_GROUP_NODE = %d",vty->node);
		index = 0;
		groupid = vty->index;
	}
	else if(vty->node == HANSI_AP_GROUP_NODE)
	{
	    vty_out(vty,"HANSI_AP_GROUP_NODE = %d",vty->node);
		index = vty->index;
		localid = vty->local;
        slot_id = vty->slotindex;
		groupid = vty->index_sub;
	}
    else if (vty->node == LOCAL_HANSI_AP_GROUP_NODE){
        index = vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
    }
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_ap_group_show_member(localid,index,groupid,&wtp_list,&apcount,dcli_dbus_connection);

    vty_out(vty,"ret =  %d\n",ret);
	if(ret == 0)
	{
	    vty_out(vty,"apcount = %d\n",apcount);
	    if(apcount != 0)
	    {
			vty_out(vty,"wtp ");
		}
		for(i = 0; i <apcount; i++)
		{
			vty_out(vty,"%d ",wtp_list[i]);
		}
		vty_out(vty,"\n");
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error>ap group id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	if(wtp_list){
		free(wtp_list);
		wtp_list = NULL;
	}
	return CMD_SUCCESS;	
}


void dcli_ap_group_init(void) {
	//printf("11111111111H####1111111\n");
//	install_node(&ap_group_node,NULL);
	//printf("222222GGGG####\n");
//	install_default(AP_GROUP_NODE);	

	install_node(&hansi_ap_group_node,NULL,"HANSI_AP_GROUP_NODE");
	install_default(HANSI_AP_GROUP_NODE);

	install_node(&local_hansi_ap_group_node,NULL,"LOCAL_HANSI_AP_GROUP_NODE");
	install_default(LOCAL_HANSI_AP_GROUP_NODE);

//	install_node(&ap_group_wtp_node,NULL);
//	install_default(AP_GROUP_WTP_NODE); 

//	install_node(&ap_group_radio_node,NULL);
//	install_default(AP_GROUP_RADIO_NODE);	

	install_node(&hansi_ap_group_wtp_node,NULL,"HANSI_AP_GROUP_WTP_NODE");
	install_default(HANSI_AP_GROUP_WTP_NODE);	

	install_node(&local_hansi_ap_group_wtp_node,NULL,"LOCAL_HANSI_AP_GROUP_WTP_NODE");
	install_default(LOCAL_HANSI_AP_GROUP_WTP_NODE);

	install_node(&hansi_ap_group_radio_node,NULL,"HANSI_AP_GROUP_RADIO_NODE");
	install_default(HANSI_AP_GROUP_RADIO_NODE); 

	install_node(&local_hansi_ap_group_radio_node,NULL,"LOCAL_HANSI_AP_GROUP_RADIO_NODE");
	install_default(LOCAL_HANSI_AP_GROUP_RADIO_NODE);
#if 0	
	install_element(CONFIG_NODE,&create_ap_group_cmd);	
	install_element(CONFIG_NODE,&del_ap_group_cmd);
	install_element(CONFIG_NODE,&config_ap_group_cmd);
	
	install_element(AP_GROUP_NODE,&add_del_ap_group_member_cmd);
	install_element(AP_GROUP_NODE,&show_group_member_cmd);
	install_element(AP_GROUP_NODE,&config_ap_group_wtp_cmd);
	install_element(AP_GROUP_NODE,&config_ap_group_radio_cmd);
#endif	
	/*------------------------------HANSI_NODE----------------------------*/
	
	install_element(HANSI_NODE,&create_ap_group_cmd);	
	install_element(HANSI_NODE,&del_ap_group_cmd);
	install_element(HANSI_NODE,&config_ap_group_cmd);	
	
	install_element(HANSI_AP_GROUP_NODE,&show_group_member_cmd);
	//printf("qqqqqq***wwwww\n");
	install_element(HANSI_AP_GROUP_NODE,&add_del_ap_group_member_cmd);
	//printf("eeee***rrrr\n");
	install_element(HANSI_AP_GROUP_NODE,&config_ap_group_wtp_cmd);
	//intf("ttt***yyyy\n");
	install_element(HANSI_AP_GROUP_NODE,&config_ap_group_radio_cmd);

	install_element(LOCAL_HANSI_NODE,&create_ap_group_cmd);	
	install_element(LOCAL_HANSI_NODE,&del_ap_group_cmd);
	install_element(LOCAL_HANSI_NODE,&config_ap_group_cmd);	
	
	install_element(LOCAL_HANSI_AP_GROUP_NODE,&show_group_member_cmd);
	//printf("qqqqqq***wwwww\n");
	install_element(LOCAL_HANSI_AP_GROUP_NODE,&add_del_ap_group_member_cmd);
	//printf("eeee***rrrr\n");
	install_element(LOCAL_HANSI_AP_GROUP_NODE,&config_ap_group_wtp_cmd);
	//intf("ttt***yyyy\n");
	install_element(LOCAL_HANSI_AP_GROUP_NODE,&config_ap_group_radio_cmd);
	
	return;
}


#endif
