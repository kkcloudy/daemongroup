/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_dcli_ebr.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif

#include "ws_dcli_ebr.h"


/*dcli_ebr.c V1.19*/
/*author qiaojie*/
/*update time 10-01-20*/


int create_ethereal_bridge_cmd(dbus_parameter parameter, DBusConnection *connection,char *id,char *brname)  
																			/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																			/*返回-2表示ebr id should be 1 to EBR_NUM-1，返回-3表示ebr name is too long,it should be 1 to 15*/
																			/*返回-4表示ebr id exist，返回-5表示ebr  is already exist，返回-6表示system cmd error，返回-7表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == id)||(NULL == brname))
		return 0;
	
	int ret=0,len=0,retu;
	unsigned int isAdd = 1;	
	unsigned int EBRID = 0;
	char *name = NULL;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 1;			

	ret = parse_int_ID((char*)id, &EBRID);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}
	
	if(EBRID >= EBR_NUM || EBRID == 0){
		return -2;
	}
	
	len = strlen(brname);
	if(len > 15){
		return -3;
	}
	name = (char*)malloc(strlen(brname)+1);
	if(NULL == name)
		return 0;
	memset(name, 0, strlen(brname)+1);
	memcpy(name, brname, strlen(brname));		
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&EBRID,
						DBUS_TYPE_STRING,&name,							 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		FREE_OBJECT(name);

		return SNMPD_CONNECTION_ERROR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			retu=1;
		else if(ret == WID_EBR_BE_USED)
			retu=-4;
		else if(ret == WID_EBR_ERROR)
			retu=-5;
		else if(ret == SYSTEM_CMD_ERROR)
			retu=-6;
		else
			retu=-7;

	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return retu;	
}


int delete_ethereal_bridge_cmd(dbus_parameter parameter, DBusConnection *connection,char *id)   
																/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示ebr id should be 1 to EBR_NUM-1*/
																/*返回-3表示ebr id does not exist，返回-4表示system cmd error，返回-5表示ebr is enable,please disable it first，返回-6表示error*/		
																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == id)
		return 0;
	
	int ret,retu;
	int isAdd = 1;	
	unsigned int EBRID = 0;
	char *name = NULL;
	char *name_d = "0";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 0;	
	ret = parse_int_ID((char*)id, &EBRID);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(EBRID >= EBR_NUM || EBRID == 0){
		return -2;
	}

	name = name_d;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&EBRID,
						DBUS_TYPE_STRING,&name,							 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			retu=1;
		else if(ret == WID_EBR_NOT_EXIST)
			retu=-3;
		else if(ret == SYSTEM_CMD_ERROR)
			retu=-4;
		else if(ret == WID_EBR_SHOULD_BE_DISABLE)	
			retu=-5;
		else
			retu=-6;

	dbus_message_unref(reply);
	return retu;	
}


void Free_ethereal_bridge_one_head(DCLI_EBR_API_GROUP *EBRINFO)
{
	void (*dcli_init_free_func)(char *,DCLI_EBR_API_GROUP *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ebr_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_EBR_METHOD_SHOW_EBR,EBRINFO);
		}
	}
}

/*返回1时，调用Free_ethereal_bridge_one_head()释放空间*/
int show_ethereal_bridge_one(dbus_parameter parameter, DBusConnection *connection,char *id,DCLI_EBR_API_GROUP **EBRINFO )
																						 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																						 /*返回-2表示ebr id should be 1 to EBR_NUM-1，返回-3表示ebr id does not exist*/
																						 /*返回-4表示error*/		
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	if(NULL == id)
	{
		*EBRINFO = NULL;
		return 0;
	}
		
	int ret,retu;
	int EBRID = 0;
	unsigned char localid_pub = 1;

	localid_pub = (unsigned char)parameter.local_id;
	
	ret = parse_int_ID((char*)id, &EBRID);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(EBRID >= EBR_NUM || EBRID == 0){
		return -2;
	}
	
	void *(*dcli_init_func)(
                        	int ,
                        	unsigned char ,
                        	unsigned int ,
                        	unsigned int* ,
                        	unsigned char *,
                        	DBusConnection *,
                        	char *
                        	);

	*EBRINFO = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ebr_show_api_group");
		if(NULL != dcli_init_func)
		{
			*EBRINFO = (*dcli_init_func)(
                        				parameter.instance_id,
                        				0,
                        				EBRID,
                        				&ret,
                        				&localid_pub,
                        				connection,
                        				WID_DBUS_EBR_METHOD_SHOW_EBR
                        				);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	if(ret == -1)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if((ret == 0)&&(*EBRINFO))
	{
	    retu = 1;		
	}
	else if(ret == WID_EBR_NOT_EXIST)
	{
		retu = -3;
	}
	else 
	{	
		retu = -4;
	}
	
	return retu;
}

void Free_ethereal_bridge_head(DCLI_EBR_API_GROUP *EBRINFO)
{
	void (*dcli_init_free_func)(char *,DCLI_EBR_API_GROUP *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ebr_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_EBR_METHOD_SHOW_EBR_LIST,EBRINFO);
		}
	}
	
}

/*返回1时，调用Free_ethereal_bridge_head()释放空间*/
int show_ethereal_bridge_list(dbus_parameter parameter, DBusConnection *connection,DCLI_EBR_API_GROUP **EBRINFO)/*返回0表示失败，返回1表示成功，返回-1表示no ebr exist*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;

    int ret,retu = 0;
	unsigned char localid_pub = 1;

	localid_pub = (unsigned char)parameter.local_id;
	
	void*(*dcli_init_func)(
                        	int ,
                        	unsigned char ,
                        	unsigned int ,
                        	unsigned int* ,
                        	unsigned char *,
                        	DBusConnection *,
                        	char *
                        	);

    *EBRINFO = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ebr_show_api_group");		
		if(NULL != dcli_init_func)
		{
			*EBRINFO = (*dcli_init_func)(
                        				parameter.instance_id,
                        				0,
                        				0,
                        				&ret,
                        				&localid_pub,
                        				connection,
                        				WID_DBUS_EBR_METHOD_SHOW_EBR_LIST
                        				);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	
	if(ret == -1)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if((ret == 0)&&(*EBRINFO))
	{
	    retu = 1;		
	}
	else if(ret == WID_EBR_NOT_EXIST)
	{
		retu = -1;
	}
	
	return retu;
}


int config_ethereal_bridge_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *ebr_state)
																						/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																						/*返回-2表示ebr id does not exist，返回-3表示ebr if error，返回-4表示system cmd process error*/
																						/*返回-5表示error，返回-6示EBR ID非法*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
	
	if(NULL == ebr_state)
		return 0;
	
	int ret,retu;
	unsigned int ebrid = 0;
	unsigned char state = 0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = ebr_id;
	
	if (!strcmp(ebr_state,"enable")){
		state=1;
	}
	else if (!strcmp(ebr_state,"disable")){
		state=0;
	}
	else
	{
		return -1;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ebrid = ebr_id;
	if(ebrid >= EBR_NUM || ebrid == 0){
		syslog(LOG_DEBUG,"ebr id in config_ethereal_bridge_enable_cmd is %d\n",ebrid);
		return -6;
	}

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_CONFIG_EBR_ENABLE);

	/*ery = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_CONFIG_EBR_ENABLE);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu=1;
	else if(ret == WID_EBR_NOT_EXIST)
		retu=-2;
	else if(ret == WID_EBR_ERROR)
		retu=-3;
	else if(ret == SYSTEM_CMD_ERROR)
		retu=-4;
	else
		retu=-5;

	dbus_message_unref(reply);
	return retu;
}


int ebr_set_bridge_isolation_func(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *isolate_state)   
																						/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																						/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																						/*返回-5表示system cmd process error，返回-6表示sameportswitch and isolation are conflict,disable sameportswitch first*/
																						/*返回-7表示error，返回-8示EBR ID非法，返回-9表示apply security in this wlan first*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == isolate_state)
		return 0;
	
	int ret,retu;
	unsigned int ebrid = 0;
	unsigned char state=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = ebr_id;
	
	if (!strcmp(isolate_state,"enable")){
		state=1;
	}
	else if (!strcmp(isolate_state,"disable")){
		state=0;
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ebrid = ebr_id;
	if(ebrid >= EBR_NUM || ebrid == 0){
		syslog(LOG_DEBUG,"ebr id in ebr_set_bridge_isolation_func is %d\n",ebrid);
		return -8;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_ISOLATION);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu=1;
	else if(ret == WID_EBR_NOT_EXIST)
		retu=-2;
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		retu=-3;
	else if(ret == WID_EBR_ERROR)
		retu=-4;
	else if(ret == SYSTEM_CMD_ERROR)
		retu=-5;
	else if(ret == ISOLATION_CONFLICT)
		retu=-6;
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
		retu=-9;
	else
		retu=-7;

	dbus_message_unref(reply);
	return retu;
}



int ebr_set_bridge_multicast_isolation_func(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *mult_isolate_state)   
																										/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																										/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																										/*返回-5表示system cmd process error，返回-6表示sameportswitch and isolation are conflict,disable sameportswitch first*/
																										/*返回-7表示error，返回-8示EBR ID非法，返回-9表示apply security in this wlan first*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == mult_isolate_state)
		return 0;
	
	int ret,retu;
	unsigned int ebrid = 0;
	unsigned char state=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//ebrid = ebr_id;
	
	if (!strcmp(mult_isolate_state,"enable")){
		state=1;
	}
	else if (!strcmp(mult_isolate_state,"disable")){
		state=0;
	}
	else
	{
		return -1;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ebrid = ebr_id;
	if(ebrid >= EBR_NUM || ebrid == 0){
		syslog(LOG_DEBUG,"ebr id in ebr_set_bridge_multicast_isolation_func is %d\n",ebrid);
		return -8;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_MULTICAST_ISOLATION);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_MULTICAST_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu=1;
	else if(ret == WID_EBR_NOT_EXIST)
		retu=-2;
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		retu=-3;
	else if(ret == WID_EBR_ERROR)
		retu=-4;
	else if(ret == SYSTEM_CMD_ERROR)
		retu=-5;	
	else if(ret == ISOLATION_CONFLICT)
		retu=-6;
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
		retu=-9;
	else
		retu=-7;

	dbus_message_unref(reply);
	return retu;
}


int set_ebr_add_del_if_cmd(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *if_state,char *if_name)
																						/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
																						/*返回-2表示if name too long，返回-3表示ebr id does not exist，返回-4表示ebr should be disable first*/
																						/*返回-5表示if_name already exist/remove some br,or system cmd process error，返回-6表示input ifname error*/
																						/*返回-7表示ebr if error，返回-8表示error，返回-9示EBR ID非法*/
																						/*返回-10表示you want to delete wlan, please do not operate like this*/
																						/*返回-11表示please check the interface's wlanid, you maybe have delete this wlan*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == if_state)||(NULL == if_name))
		return 0;
	
	int ret = WID_DBUS_SUCCESS;;
	int retu;
	unsigned int EBRID = 0;
	char *name;
	unsigned int state = 0;
	int lenth = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	
	//EBRID = ebr_id;

	if (!strcmp(if_state,"add")){
		state = 1;
	}
	else if (!strcmp(if_state,"delete")){
		state = 0;
	}
	else
	{
		return -1;
	}

	lenth = strlen((char *)if_name);

	if(lenth > 15)
	{		
		return -2;
	}
	
	name = (char *)malloc(sizeof(char)*20);
	if(name == NULL)
	{
		return MALLOC_ERROR;
	}
	memset(name,0,20);
	memcpy(name,if_name,strlen(if_name));
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	EBRID = ebr_id;
	if(EBRID >= EBR_NUM || EBRID == 0){
		syslog(LOG_DEBUG,"ebr id in set_ebr_add_del_if_cmd is %d\n",EBRID);
		return -9;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_IF);
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_IF);*/

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EBRID,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		FREE_OBJECT(name);

		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0)
		retu=1;
	else if(ret == WID_EBR_NOT_EXIST)
		retu=-3;
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		retu=-4;
	else if(ret == SYSTEM_CMD_ERROR)
		retu=-5;
	else if(ret == APPLY_IF_FAIL)
		retu=-6;
	else if(ret == WID_EBR_ERROR)
		retu=-7;
	else if (ret == WID_WANT_TO_DELETE_WLAN)
		retu=-10;
	else if (ret == WLAN_ID_NOT_EXIST)
		retu=-11;
	else
		retu=-8;

	
	if(name)
	{
		free(name);
		name = NULL;
	}
	dbus_message_unref(reply);
	return retu;
}


int set_ebr_add_del_uplink_cmd(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *addordel,char *ifnamez) 
																							  /*返回0表示失败，返回1表示成功，返回-1表示error*/
																							  /*返回-2表示input parameter should only be 'add' or 'delete'，返回-3表示if name too long*/
																							  /*返回-4表示malloc error，返回-5表示ebr should be disable first*/
																							  /*返回-6表示already exist/remove some br,or system cmd process error，返回-7表示input ifname error*/
																							  /*返回-8表示ebr if error，返回-9表示interface does not add to br or br uplink，返回-10表示ebr id does not exist*/
																							  /*返回-11示EBR ID非法*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == addordel)||(NULL == ifnamez))
		return 0;
		
	int ret = WID_DBUS_SUCCESS;
	int retu;
	unsigned int EBRID = 0;
	char *name;
	unsigned int state = 0;
	int lenth = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);

	if (!strcmp(addordel,"add")){
		state = 1;
	}
	else if (!strcmp(addordel,"delete")){
		state = 0;
	}
	else
	{
		//vty_out(vty,"<error> input parameter should only be 'add' or 'delete'\n");
		retu = -2;
		return retu;
	}

	lenth = strlen((char *)ifnamez);

	if(lenth > 15)
	{		
		//vty_out(vty,"<error> if name too long\n");
		retu = -3;
		return retu;
	}
	
	name = (char *)malloc(sizeof(char)*20);
	if(name == NULL)
	{
		//return MALLOC_ERROR;
		return -4;
	}
	memset(name,0,20);
	memcpy(name,ifnamez,strlen(ifnamez));
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	EBRID = ebr_id;
	if(EBRID >= EBR_NUM || EBRID == 0){
		syslog(LOG_DEBUG,"ebr id in set_ebr_add_del_uplink_cmd is %d\n",EBRID);
		return -11;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_UPLINK);
		

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EBRID,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		FREE_OBJECT(name);

		return SNMPD_CONNECTION_ERROR ;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0)
	{
		//vty_out(vty,"set ebr %s uplink %s successfully.\n",argv[0],argv[1]);
		retu = 1;
	}
	else if(ret == WID_EBR_NOT_EXIST)
	{
		//vty_out(vty,"<error> ebr id does not exist\n");
		retu = -10;
	}
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
	{
		//vty_out(vty,"<error> ebr should be disable first\n");
		retu = -5;
	}
	else if(ret == SYSTEM_CMD_ERROR)
	{
		//vty_out(vty,"<error> %s already exist/remove some br,or system cmd process error\n",argv[1]);
		retu = -6;
	}
	else if(ret == APPLY_IF_FAIL)
	{
		//vty_out(vty,"<error> input ifname error\n");
		retu = -7;
	}
	else if(ret == WID_EBR_ERROR)
	{
		//vty_out(vty,"<error> ebr if error\n");
		retu = -8;
	}
	else if(ret == WID_EBR_IF_NOEXIT)
	{
		//vty_out(vty,"<error> interface does not add to br or br uplink\n");	
		retu  =-9;
	}
	else
	{
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -1;
	}
	
	if(name)
	{
		free(name);
		name = NULL;
	}
	
	dbus_message_unref(reply);
	
	return retu;
}


int ebr_set_bridge_sameportswitch_func(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *spswitch_state)  /*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																								/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																								/*返回-5表示system cmd process error，返回-6表示isolation or multicast are enable,disable isolation and multicast first*/
																								/*返回-7表示error，返回-8示EBR ID非法*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == spswitch_state)
		return 0;
	
	int ret,retu;
	unsigned int ebrid = 0;
	unsigned char state=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
		
	if (!strcmp(spswitch_state,"enable")){
		state=1;
	}
	else if (!strcmp(spswitch_state,"disable")){
		state=0;
	}
	else
	{
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ebrid = ebr_id;
	if(ebrid >= EBR_NUM || ebrid == 0){
		syslog(LOG_DEBUG,"ebr id in ebr_set_bridge_sameportswitch_func is %d\n",ebrid);
		return -8;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_SAMEPORTSWITCH);

	/*ery = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_SET_BRIDGE_SAMEPORTSWITCH);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ebrid,
							 DBUS_TYPE_BYTE,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu=1;
	else if(ret == WID_EBR_NOT_EXIST)
		retu=-2;
	else if(ret == WID_EBR_SHOULD_BE_DISABLE)
		retu=-3;
	else if(ret == WID_EBR_ERROR)
		retu=-4;
	else if(ret == SYSTEM_CMD_ERROR)
		retu=-5;
	else if(ret == ISOLATION_CONFLICT)
		retu=-6;
	else
		retu=-7;

	dbus_message_unref(reply);
	return retu;
}


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

