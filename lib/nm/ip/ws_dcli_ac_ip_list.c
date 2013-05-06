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
* ws_dcli_ac_ip_list.c
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


#include "ws_dcli_ac_ip_list.h"


/*dcli_ac_ip_list.c V1.10*/
/*author qiaojie*/
/*update time 10-04-15*/


void Free_show_ac_ip_list_one(DCLI_AC_IP_LIST_API_GROUP *IPLIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_IP_LIST_API_GROUP *);
	dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_ip_list_free_fun");
	dcli_init_free_func(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE,IPLIST);
}

/*返回1时，调用Free_show_ac_ip_list_one()释放空间*/
/*id的范围是1至ACIPLIST_NUM-1*/
int show_ac_ip_list_one_cmd(int instance_id,char *id, DCLI_AC_IP_LIST_API_GROUP **IPLIST)/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																							   /*返回-2表示AC IP LIST id should be 1 to ACIPLIST_NUM-1*/
									                                             			   /*返回-3表示id does not exist，返回-4表示error*/
{	
	if(NULL == id)
	{
		*IPLIST = NULL;
		return 0;
	}
	
	int ret;
	unsigned char ID = 0;
#if 0	
	int i = 0;
	unsigned int num = 0;
	struct wid_ac_ip **iplist;
	
	char en[] = "enable";
	char dis[] = "disable";
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
#endif	
	int retu;
	
	ret = parse_char_ID((char*)id, &ID);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(ID >= ACIPLIST_NUM || ID == 0){
		return -2;
	}

	int index;
#if 0	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
#endif

	/*if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}*/
	index = instance_id;
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
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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

	void*(*dcli_init_func)(
						int ,
						unsigned char ,
						unsigned int ,
						unsigned int* ,
						unsigned char *,
						DBusConnection *,
						char *
						);

	*IPLIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_ip_list_show_api_group");
		if(NULL != dcli_init_func)
		{
			*IPLIST =(*dcli_init_func)
				  (
					  index,
					  ID,
					  0,
					  &ret,
					  0,
					  ccgi_dbus_connection,
					  WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE
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
		retu = 0;
	}
	if((ret == 0)&&(*IPLIST))
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


void Free_show_ac_ip_list(DCLI_AC_IP_LIST_API_GROUP *IPLIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_IP_LIST_API_GROUP *);
	dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_ip_list_free_fun");
	dcli_init_free_func(WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST,IPLIST);
}

/*返回1时，调用Free_show_ac_ip_list()释放空间*/
int show_ac_ip_list_cmd(int instance_id,DCLI_AC_IP_LIST_API_GROUP **IPLIST)/*返回0表示失败，返回1表示成功*/
{	
	int ret;
#if 0
	int i = 0;
	int num = 0;
	wid_ac_ip_group *ACIPLIST[ACIPLIST_NUM];
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	char en[] = "enable";
	char dis[] = "disable";	
#endif

	int retu;
	
	
	int index;
#if 0
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
#endif

	/*if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}*/
	index = instance_id;
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
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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

	void*(*dcli_init_func)(
						int ,
						unsigned char ,
						unsigned int ,
						unsigned int* ,
						unsigned char *,
						DBusConnection *,
						char *
						);

	*IPLIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_ip_list_show_api_group");
		if(NULL != dcli_init_func)
		{
			*IPLIST =(*dcli_init_func)
				  (
					  index,
					  0,
					  0,
					  &ret,
					  0,
					  ccgi_dbus_connection,
					  WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST
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
		retu = 0;
	}
	if((ret == 0)&&(*IPLIST))
	{
		retu = 1;
	}

	return retu;
}

/*ID的范围是1至ACIPLIST_NUM-1*/
/*ifname的长度要小于16 */
int create_ac_ip_list_cmd(int instance_id,char *ID,char *ifname)/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																   /*返回-2表示id should be 1 to ACIPLIST_NUM-1，返回-3表示ifname is too long,out of the limit of 16*/
																   /*返回-4表示id exist，返回-5表示error，返回-6表示interface has be binded in other hansi*/
{
	if((NULL == ID)||(NULL == ifname))
		return 0;
	
	int ret,len;
	unsigned char isAdd;	
	unsigned char id;
	char *name;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
	int retu;
	
	ret = parse_char_ID((char*)ID, &id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(id >= ACIPLIST_NUM || id == 0){
		return -2;
	}
	len = strlen(ifname);
	if(len > 16){		
		return -3;
	}
	name = (char*)malloc(strlen(ifname)+1);
	if(NULL == name)
		return 0;
	memset(name, 0, strlen(ifname)+1);
	memcpy(name, ifname, strlen(ifname));		
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}*/
	index = instance_id;
	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP_LIST_GROUP);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP_LIST_GROUP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&id,
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		if(name)
		{
			free(name);
			name = NULL;
		}
		return 0;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_BE_USED)
		retu = -4;
	else if(ret == IF_BINDING_FLAG)
		retu = -6;
	else
		retu = -5;

	dbus_message_unref(reply);
	FREE_OBJECT(name);
	return retu;	
}

/*ID的范围是1至ACIPLIST_NUM-1*/
int del_ac_ip_list_cmd(int instance_id,char *ID)/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
												   /*返回-2表示id should be 1 to ACIPLIST_NUM-1，返回-3表示wlan id does not exist*/
												   /*返回-4表示wlan is enable,please disable it first，返回-5表示error*/
{
	if(NULL == ID)
		return 0;
	
	int ret;
	unsigned char id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	int retu;
		
	ret = parse_char_ID((char*)ID, &id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(id >= ACIPLIST_NUM || id == 0){
		return -2;
	}
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}*/
	index = instance_id;
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP_LIST_GROUP);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP_LIST_GROUP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&id,
						DBUS_TYPE_INVALID);

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

		if(ret == 0)
			retu = 1;
		else if(ret == WLAN_ID_NOT_EXIST)
			retu = -3;
		else if(ret == WLAN_BE_ENABLE)
			retu = -4;
		else
			retu = -5;

	dbus_message_unref(reply);
	return retu;	
}

/*Priority的范围是1-100*/
int add_ac_ip_cmd(int instance_id,char *id,char *IP,char *Priority)/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																	   /*返回-2表示unknown id format，返回-3表示wlan id does not exist*/
																	   /*返回-4表示interface does not exist，返回-5表示wlan is enable,please disable it first*/
																	   /*返回-6表示error*/
{
	if((NULL == id)||(NULL == IP)||(NULL == Priority))
		return 0;
	
	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	unsigned char priority = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = WID_Check_IP_Format((char*)IP);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}
	
	ret = parse_char_ID((char*)Priority, &priority);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	//ID = (int)vty->index;
	
	ip = (char*)malloc(strlen(IP)+1);
	if(NULL == ip)
		return 0;
	memset(ip, 0, strlen(IP)+1);
	memcpy(ip, IP, strlen(IP));	

	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
	}*/
	index = instance_id;			
	ID = id;
	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_BYTE,&priority,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_NOT_EXIST)
		retu = -3;
	else if(ret == APPLY_IF_FAIL)
		retu = -4;
	else if(ret == WLAN_BE_ENABLE)
		retu = -5;
	else
		retu = -6;
		
	dbus_message_unref(reply);
	FREE_OBJECT(ip);
	
	return retu;
}

int del_ac_ip_cmd(int instance_id,char *id,char *IP)/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
													   /*返回-2表示wlan id does not exist，返回-3表示interface does not exist*/
													   /*返回-4表示wlan is enable,please disable it first，返回-5表示error*/
{
	if((NULL == id)||(NULL == IP))
		return 0;
	
	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = WID_Check_IP_Format((char*)IP);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}
		
	ip = (char*)malloc(strlen(IP)+1);
	if(NULL == ip)
		return 0;
	memset(ip, 0, strlen(IP)+1);
	memcpy(ip, IP, strlen(IP));	

	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
	}*/
	index = instance_id;			
	ID = id;
		
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_NOT_EXIST)
		retu = -2;
	else if(ret == APPLY_IF_FAIL)
		retu = -3;
	else if(ret == WLAN_BE_ENABLE)
		retu = -4;
	else
		retu = -5;
		
	dbus_message_unref(reply);
	FREE_OBJECT(ip);

	return retu;
}

/*Priority的范围是1-100*/
int modify_ac_ip_priority_cmd(int instance_id,char *id,char *IP,char *Priority)/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																				   /*返回-2表示unknown id format，返回-3表示wlan id does not exist*/
																				   /*返回-4表示interface does not exist，返回-5表示wlan is enable,please disable it first*/
																				   /*返回-6表示error*/
{
	if((NULL == id)||(NULL == IP)||(NULL == Priority))
		return 0;
	
	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	unsigned char priority = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = WID_Check_IP_Format((char*)IP);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}
	
	ret = parse_char_ID((char*)Priority, &priority);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	
	ip = (char*)malloc(strlen(IP)+1);
	if(NULL == ip)
		return 0;
	memset(ip, 0, strlen(IP)+1);
	memcpy(ip, IP, strlen(IP));	

	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
	}*/
	index = instance_id;			
	ID = id;
		
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_PRIORITY);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_ACIPLIST_OBJPATH,\
						WID_DBUS_ACIPLIST_INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_PRIORITY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_BYTE,&priority,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_NOT_EXIST)
		retu = -3;
	else if(ret == APPLY_IF_FAIL)
		retu = -4;
	else if(ret == WLAN_BE_ENABLE)
		retu = -5;
	else
		retu = -6;
		
	dbus_message_unref(reply);
	FREE_OBJECT(ip);

	return retu;
}

/*state为"enable"或"disable"*/
int set_ac_ip_list_banlance_cmd(int instance_id,char *id,char *state)/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist*/
																	      /*返回-2表示interface does not exist，返回-3表示wlan is enable,please disable it first*/
																	      /*返回-4表示error*/
{
	if((NULL == id)||(NULL == state))
		return 0;
	
	int ret;
	unsigned char ID = 0;
	unsigned char banlanceflag = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	if (!strcmp(state,"enable"))
	{
		banlanceflag = 1;	
	}		

	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
	}*/
	index = instance_id;			
	ID = id;
		
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_BANLANCE_FLAG);
		

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_BYTE,&banlanceflag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_NOT_EXIST)
		retu = -1;
	else if(ret == APPLY_IF_FAIL)
		retu = -2;
	else if(ret == WLAN_BE_ENABLE)
		retu = -3;
	else
		retu = -4;
		
	dbus_message_unref(reply);
	return retu;
}

int set_ac_ip_threshold_cmd(int instance_id,char *id,char *IP,char *value)/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																			   /*返回-2表示unknown id format，返回-3表示wlan id does not exist*/
																		       /*返回-4表示interface does not exist，返回-5表示wlan is enable,please disable it first*/
																		       /*返回-6表示ip addr no exit*/
{
	if((NULL == id)||(NULL == IP)||(NULL == value))
		return 0;
	
	int ret;
	unsigned char ID = 0;
	unsigned char * ip;
	unsigned int threshold = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = WID_Check_IP_Format((char*)IP);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}
	
	ret = parse_int_ID((char*)value, &threshold);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	
	ip = (char*)malloc(strlen(IP)+1);
	if(NULL == ip)
		return 0;
	memset(ip, 0, strlen(IP)+1);
	memcpy(ip, IP, strlen(IP));	

	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
	}*/
	index = instance_id;			
	ID = id;
	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_THRESHOLD);
		
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_UINT32,&threshold,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		if(ip)
		{
			free(ip);
			ip = NULL;
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_NOT_EXIST)
		retu = -3;
	else if(ret == APPLY_IF_FAIL)
		retu = -4;
	else if(ret == WLAN_BE_ENABLE)
		retu = -5;
	else
		retu = -6;
		
	dbus_message_unref(reply);
	FREE_OBJECT(ip);

	return retu;
}

int set_ac_ip_diffnum_banlance_cmd(int instance_id,char *id,char *value)/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																			   /*返回-2表示wlan id does not exist，返回-3表示interface does not exist*/
																			   /*返回-4表示wlan is enable,please disable it first，返回-5表示error*/
{
	if((NULL == id)||(NULL == value))
		return 0;
	
	int ret;
	unsigned char ID = 0;
	unsigned int diffnum = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = parse_int_ID((char*)value, &diffnum);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	

	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == ACIPLIST_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_ACIPLIST_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
	}*/
	index = instance_id;			
	ID = id;
	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_DIFF_BANLANCE);
		

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_UINT32,&diffnum,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu = 1;
	else if(ret == WLAN_ID_NOT_EXIST)
		retu = -2;
	else if(ret == APPLY_IF_FAIL)
		retu = -3;
	else if(ret == WLAN_BE_ENABLE)
		retu = -4;
	else
		retu = -5;
		
	dbus_message_unref(reply);
	return retu;
}



