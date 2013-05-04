#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/wait.h>
#include "command.h"
#include "../dcli_main.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "wid_ac.h"
#include "dcli_wtp.h"
#include "../dcli_vrrp.h"
#include "hmd/hmdpub.h"

struct LicenseData{
	int total_num;
	int free_num;
	int slot_id[MAX_SLOT_NUM];
	int l_assign_num[MAX_SLOT_NUM][MAX_INSTANCE];
	int r_assign_num[MAX_SLOT_NUM][MAX_INSTANCE];
};


DEFUN(license_request_cmd_func,
	license_request_cmd,
	"license request",
	"license request\n"
	)
{
	unsigned char cmd[DEFAULT_LEN];
	sprintf(cmd,"/etc/lic/licreq_gen.sh");
	system(cmd);
	return CMD_SUCCESS;
}
DEFUN(license_text_cmd_func,
	license_text_cmd,
	"license text .ID",
	"display license text\n"
	"license ID like 1,2,3 or all display all license\n"
	)
{
	unsigned char channel = 0;
	int ret = 0;
	if (!strcmp(argv[0],"all"))
	{
    	channel = 0;
	}
	else
	{
		ret = parse_char_ID((char *)argv[0],&channel);
		if (ret != WID_DBUS_SUCCESS)
		{	
            if(ret == WID_ILLEGAL_INPUT){
              vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> input parameter %s error expect like 1 2 3 or all\n",argv[0]);
			}
			return CMD_SUCCESS;
		}
	}
		
	unsigned char cmd[DEFAULT_LEN];
	sprintf(cmd,"/etc/lic/lic_dump.sh %d",channel);
	system(cmd);
	return CMD_SUCCESS;
}
DEFUN(license_install_cmd_func,
	license_install_cmd,
	"license install .LINE",
	"install license\n"
	)
{
	DBusMessage *query, *reply;	
	DBusMessage *query2, *reply2;
	DBusError err;
	DBusError err2;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter2;
	int ret;
	int ret2;
	int stat = 0;
	int reason = 0;
	int instRun = 0;
	unsigned char *cmd,*license;	
	license = WID_parse_ap_extension_command(argv, argc);
	cmd = (unsigned char*)malloc(strlen(license)+64);
	memset(cmd,0,strlen(license)+64);
	sprintf(cmd,"/etc/lic/lic_inst.sh %s",license);
	stat = system(cmd);
	reason = WEXITSTATUS(stat);
	if(reason != 0)
	{
		vty_out(vty,"License fail(%d),please check your license or contact vendor.\n",reason);
		//vty_out(vty,"please contact vendor.\n");
		if(cmd)
		{
			free(cmd);
			cmd = NULL;
		}
		return CMD_SUCCESS;
	}else{
		vty_out(vty,"License install successfully , ac need reboot\n");		
	}
	if(cmd)
	{
		free(cmd);
		cmd = NULL;
	}
	if(license){
		free(license);
		license = NULL;
	}
#if 0
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	for(index = 0; index < 17; index++){
		instRun = dcli_vrrp_hansi_is_running(vty, index);
		if((instRun == DCLI_VRRP_INSTANCE_CREATED)||(index == 0)){
			memset(BUSNAME,0,PATH_LEN);
			memset(OBJPATH,0,PATH_LEN);
			memset(INTERFACE,0,PATH_LEN);	
			DBusConnection *dcli_dbus_connection = NULL;
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
			ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
								INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTP_COUNT);
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
			dbus_message_unref(reply);
			
			if(ret == WID_DBUS_SUCCESS){
				vty_out(vty,"instance %d license install success.\n",index); 	
			
			}else{
				vty_out(vty,"<error>instance %d license install failed, AC need reboot for license install.\n",index);		
			}
		}
		
	}
#endif
#if 0

	if(ret == WID_DBUS_SUCCESS){
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);
		ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT);
		dbus_error_init(&err2);
		reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
		dbus_message_unref(query2);
		if (NULL == reply2) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err2)) {
				cli_syslog_info("%s raised: %s",err2.name,err2.message);
				dbus_error_free_for_dcli(&err2);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply2,&iter2);
		dbus_message_iter_get_basic(&iter2,&ret2);
		dbus_message_unref(reply2);
		if(ret2 == ASD_DBUS_SUCCESS){
			DcliWReInit();	
		}
	}else{
		vty_out(vty,"<error> AC need reboot for license install.\n");		
	}
#endif

	return CMD_SUCCESS;
}

DEFUN(license_assign_cmd_func,
	  license_assign_cmd,
	  "license assign type TYPE num NUM (local-hansi|hansi) HANSIID",
	  CONFIG_STR
	  "assign license to slot\n"
	  "license type\n"
	  "license num\n"
	  "slot	num\n"
	 )
{	int ret;
	unsigned int type;
	unsigned int num;
	unsigned int slot_id;
	unsigned int inst_id;
	unsigned int islocaled = 0;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;

	if(distributFag == 0){
		vty_out(vty,"Command Not Supported\n");		
		return CMD_SUCCESS;
	}	
	if(is_active_master == 0){
		vty_out(vty,"Slot Is Not Active Master,Command Not Supported\n");		
		return CMD_SUCCESS;
	}
	ret = parse_int(argv[0], &type);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown type format\n");
		return CMD_SUCCESS;
	}		
	ret = parse_int(argv[1], &num);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown num format\n");
		return CMD_SUCCESS;
	}	
	if(num > 1024){
		vty_out(vty,"<error> num should be less than 1024\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[2],"hansi")||(tolower(argv[2][0]) == 'h')){
		islocaled = 0;
	}else if (!strcmp(argv[2],"local-hansi")||(tolower(argv[2][0]) == 'l')){
		islocaled = 1;
	}
	
	ret = parse_slot_hansi_id((char*)argv[3],&slot_id,&inst_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown hansi id format\n");
		return CMD_SUCCESS;
	}	
	if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
		vty_out(vty,"<error> local hansi slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
		return CMD_SUCCESS;
	}	
	if(inst_id >= MAX_INSTANCE|| inst_id < 0){
		vty_out(vty,"<error> local hansi instance id should be 1 to %d\n",MAX_INSTANCE-1);
		return CMD_SUCCESS;
	}	

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&num,
							 DBUS_TYPE_UINT32,&islocaled,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&inst_id,
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
				vty_out(vty,"license assign type %d num %d sucessfully.\n",type, num);				
			}
			else if(ret == HMD_DBUS_LICENSE_NUM_NOT_ENOUGH){
				vty_out(vty,"<error> no enough license left\n");
			}
			else if(ret == HMD_DBUS_LICENSE_TYPE_NOT_EXIST){
				vty_out(vty,"<error> license type not exist\n");
			}
			else if(ret == HMD_DBUS_SLOT_ID_NOT_EXIST){
				vty_out(vty,"<error> slot id not exist\n");
			}
			else if(ret == HMD_DBUS_COMMAND_NOT_SUPPORT){
				vty_out(vty,"<error> the board is no active master\n");
			}
			else{				
				vty_out(vty,"<error> code num %d.\n", ret);
			}
	}
	else
		vty_out(vty,"<error> %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(license_assign_show_cmd_func,
	  license_assign_show_cmd,
	  "license assign show",
	  CONFIG_STR
	  "assign license to slot\n"
	  "license info show\n"
	 )
{	int ret;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array;	
	DBusMessageIter	 iter_sub_struct;
	DBusMessageIter  iter_sub_sub_array;	
	DBusMessageIter	 iter_sub_sub_struct;
	struct LicenseData *LicenseInfo = NULL;
	int LicenseCount;
	int SlotCount;
	int i = 0;
	int j = 0;
	int k = 0;
	int num = 0;
	char buf[128] = {0};
	char *tmp = buf;
	int len = 0;
	
	if(distributFag == 0){
		vty_out(vty,"Command Not Supported\n");		
		return CMD_SUCCESS;
	}

	if(is_master == 0){
		vty_out(vty,"Slot Is Not Master,Command Not Supported\n");		
		return CMD_SUCCESS;
	}

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN_SHOW);

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
	{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&LicenseCount);
		LicenseInfo = malloc(LicenseCount*(sizeof(struct LicenseData)));
		memset(LicenseInfo, 0, LicenseCount*(sizeof(struct LicenseData)));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		for (i = 0; i < LicenseCount; i++)
		{
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(LicenseInfo[i].total_num));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LicenseInfo[i].free_num));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&SlotCount);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0;j<SlotCount;j++){
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(LicenseInfo[i].slot_id[j]));
				dbus_message_iter_next(&iter_sub_struct);					
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);
				for(k = 0; k < MAX_INSTANCE; k++){
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(LicenseInfo[i].l_assign_num[j][k]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(LicenseInfo[i].r_assign_num[j][k]));
					dbus_message_iter_next(&iter_sub_sub_array);
				}
				dbus_message_iter_next(&iter_sub_array);
			}

			
			dbus_message_iter_next(&iter_array);	/*xiaodawei add for bindingflag, 20101108*/
				
		}
	}
	if(LicenseCount != 0){
		vty_out(vty,"############################################################################\n");
		vty_out(vty,"License Count %d\n",LicenseCount);
		len += sprintf(tmp,"%14s","LicenseType:");
		tmp = buf + len;
		for(i = 1; i <= LicenseCount; i++){
			len += sprintf(tmp,"%6d",i);
			tmp = buf + len;
		}
		vty_out(vty,"%s\n",buf);
		for(i = 0; i < SlotCount; i++){
			for(k = 0; k < MAX_INSTANCE; k++){
				memset(buf, 0, 128);
				tmp = buf;
				len = 0;
				num = 0;
				for(j = 0; j < LicenseCount; j++){
					num += LicenseInfo[j].l_assign_num[i][k];
				}
				if(num != 0){
					
					len += sprintf(tmp,"%8s%2d-%2d:","L-hansi",LicenseInfo[0].slot_id[i],k);
					tmp = buf + len;
					for(j = 0; j < LicenseCount; j++){
						len += sprintf(tmp,"%6d",LicenseInfo[j].l_assign_num[i][k]);
						tmp = buf + len;
					}
					vty_out(vty,"%s\n",buf);
				}
			}			
			for(k = 0; k < MAX_INSTANCE; k++){
				memset(buf, 0, 128);
				tmp = buf;
				len = 0;
				num = 0;
				for(j = 0; j < LicenseCount; j++){
					num += LicenseInfo[j].r_assign_num[i][k];
				}
				if(num != 0){				
					len += sprintf(tmp,"%8s%2d-%2d:","Hansi",LicenseInfo[0].slot_id[i],k);
					tmp = buf + len;
					for(j = 0; j < LicenseCount; j++){
						len += sprintf(tmp,"%6d",LicenseInfo[j].r_assign_num[i][k]);
						tmp = buf + len;
					}
					vty_out(vty,"%s\n",buf);
				}
			}			

		}
		memset(buf, 0 ,128);
		vty_out(vty,"----------------------------------------------------------------------------\n");
		memset(buf, 0 ,128);
		tmp = buf;
		len = 0;
		len += sprintf(tmp,"%14s","Total:");
		tmp = buf + len;
		for(i = 0; i < LicenseCount; i++){
			len += sprintf(tmp,"%6d",LicenseInfo[i].total_num);
			tmp = buf + len;
		}
		vty_out(vty,"%s\n",buf);
		
		memset(buf, 0 ,128);
		tmp = buf;
		len = 0;
		len += sprintf(tmp,"%14s","Free:");
		tmp = buf + len;
		for(i = 0; i < LicenseCount; i++){
			len += sprintf(tmp,"%6d",LicenseInfo[i].free_num);
			tmp = buf + len;
		}
		vty_out(vty,"%s\n",buf);
		
		vty_out(vty,"############################################################################\n");
	}
	if(LicenseInfo != NULL){
		free(LicenseInfo);
		LicenseInfo = NULL;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


void dcli_license_init(void) {	
	install_element(ENABLE_NODE,&license_request_cmd);
	install_element(ENABLE_NODE,&license_install_cmd);
	install_element(ENABLE_NODE,&license_text_cmd);	
	install_element(ENABLE_NODE,&license_assign_cmd);
	install_element(ENABLE_NODE,&license_assign_show_cmd);
	install_element(CONFIG_NODE,&license_assign_cmd);
	install_element(CONFIG_NODE,&license_assign_show_cmd);
	return;
}
#endif

