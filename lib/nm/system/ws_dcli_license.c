/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* ws_dcli_license.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/wait.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_license.h"


/*dcli_license.c V1.2*/
/*author qiaojie*/
/*update time 09-6-23*/

int license_request_cmd_func()
{
	unsigned char cmd[DEFAULT_LEN];
	sprintf(cmd,"sudo /etc/lic/licreq_gen.sh > /var/run/license.tmp");
	system(cmd);
	return 0;
}

/*
	author zhangwl;
	update time 11-9-5;
	dcli_license.c V1.3
*/
int license_text_cmd_func()
{	
		
	unsigned char cmd[DEFAULT_LEN];
	sprintf(cmd,"/etc/lic/lic_dump.sh 0 > /var/run/maxwtpcount.lic ");
	system(cmd);
	return 0;

}

int license_install_cmd_func(char *SN)/*返回0表示失败，返回1表示成功，返回-1表示License fail,please contact vendor*/
{
	if(NULL == SN)
		return 0;
	
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
	unsigned char *cmd;	
	cmd = (unsigned char*)malloc(strlen(SN)+64);
	memset(cmd,0,strlen(SN)+64);
	sprintf(cmd,"/etc/lic/lic_inst.sh %s",SN);
	stat = system(cmd);
	reason = WEXITSTATUS(stat);	
	if(reason != 0)
	{
		if(cmd)
		{
			free(cmd);
			cmd = NULL;
		}
		return -1;
	}
	else
	{
		return 1;
	}
	if(cmd)
	{
		free(cmd);
		cmd = NULL;
	}

	#if 0
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	ccgi_ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTP_COUNT);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	if(ret == WID_DBUS_SUCCESS){
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);
		ccgi_ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
		ccgi_ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ccgi_ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT);
		
		dbus_error_init(&err2);
		reply2 = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query2,-1, &err2);	
		dbus_message_unref(query2);
		if (NULL == reply2) {
			if (dbus_error_is_set(&err2)) {
				dbus_error_free(&err2);
			}
			return 0;
		}
		
		dbus_message_iter_init(reply2,&iter2);
		dbus_message_iter_get_basic(&iter2,&ret2);
		dbus_message_unref(reply2);
		
		/*delete by qiaojie*/
		/*由于在web页面中，install license之后，不需要通过DcliWReInit函数重新计算WTP_NUM等全局变量*/
		/*如果需要在snmp中支持install license功能，就必须调用DcliWReInit函数*/
		
		/*if(ret2 == ASD_DBUS_SUCCESS){
			DcliWReInit();	
		}*/
	}
	return 1;
	#endif
}

int parse_slot_hansi_id(char* ptr,int *firstid,int *secondid)
{
    id_state state = c_first_id;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case c_first_id: 
			*firstid = strtoul(str,&str,10);
			if(*firstid >= 0 && *firstid < 4095){
        		state=c_sub;
			}
			else state=c_fail;
			break;
		case c_sub: 
			if (PARSE_ID_IFNAME_SUB== str[0]){
				if ('\0' == str[1]){
					state = c_fail;
				}
				else{		
					state = c_second_id;
				}
				}
			else
				state = c_fail;
			break;
		case c_second_id: 
			*secondid = strtoul((char *)&(str[1]),&str,10);
			if(*secondid >= 0 && *secondid <= 16){/*radioid range 0 1 2 3*/
        		state=c_end;
			}
			else state=c_fail;
			break;
		case c_fail:
            return -1;
			break;
		case c_end: 
			if ('\0' == str[0]) {
				state = c_success;
			}
			else
				state = c_fail;
				break;
		case c_success: 
			return 0;
			break;
		default: break;
		}
	}
		
}


int license_assign_cmd_web(char *type_p,char *num_p,char *hansi,int slotid,char *hansi_id,DBusConnection *connection)
	//-2:command not supported;-3:slot is not active master;-4:unknow type format;-5:unknow num format;-6:hansi_id format error
	//-7:slot_id is out range;-8:instance_id is out range;-9:failed get reply;-10:no enough license left;-11:license type no exist;
	//-12:slot_id is not exist;-13:the board is not active master;-14:num should be more than specefication,should be 1024 or 2048;-15 or -16 is other error

{	
	int ret;
	int retu;
	unsigned int type;
	unsigned int num;
	unsigned int slot_id =slotid;
	unsigned int inst_id;
	unsigned int islocaled = 0;
	DBusMessage *query, *reply;
	DBusError err;
	FILE *fd = NULL;
	int is_active_master = 0;

	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		return -1;
	}
	fscanf(fd, "%d", &is_active_master);
	fclose(fd);

	if(distributed_flag == 0){
		return -2;
	}	
	if(is_active_master == 0){
		return -3;
	}
	ret = parse_int(type_p, &type);
	if(ret != WID_DBUS_SUCCESS){
		return -4;
	}		
	ret = parse_int(num_p, &num);
	if(ret != WID_DBUS_SUCCESS){
		return -5;
	}	

	if (!strcmp(hansi,"hansi")){
		islocaled = 0;
	}else if (!strcmp(hansi,"local-hansi")){
		islocaled = 1;
	}
	inst_id=  (unsigned int)strtoul(hansi_id,0,10);
	if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
		return -7;
	}	
	if(inst_id >= MAX_INSTANCE|| inst_id < 0){
		return -8;
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

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -9;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
			if(ret == 0){
				retu=0;
			}
			else if(ret == HMD_DBUS_LICENSE_NUM_NOT_ENOUGH){
				retu=-10;
			}
			else if(ret == HMD_DBUS_LICENSE_TYPE_NOT_EXIST){
				retu=-11;
			}
			else if(ret == HMD_DBUS_SLOT_ID_NOT_EXIST){
				retu=-12;
			}
			else if(ret == HMD_DBUS_COMMAND_NOT_SUPPORT){
				retu=-13;
			}
			else if(ret == HMD_DBUS_SET_NUM_MORE_THAN_SPECEFICATION){
				retu=-14;
			}
			else{				
				retu=-15;
			}
	}
	else
		retu=-16;
	dbus_message_unref(reply);
	return retu;
}

int license_assign_show_cmd_web(DBusConnection *connection,int *license_count,struct LicenseData **LicInfo,int *slot_c)
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
	//char buf[128] = {0};
	//char *tmp = buf;
	int len = 0;
	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN_SHOW);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return -1;
	}
	{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&LicenseCount);
		*license_count=LicenseCount;
		LicenseInfo = malloc(LicenseCount*(sizeof(struct LicenseData)));
		if(NULL == LicenseInfo){
			return -2;
		}
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
			*slot_c = SlotCount;
			
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

			
			dbus_message_iter_next(&iter_array);	
				
		}
	}
	dbus_message_unref(reply);
	*LicInfo = LicenseInfo;
	return 0;
}	




#ifdef __cplusplus
}
#endif


