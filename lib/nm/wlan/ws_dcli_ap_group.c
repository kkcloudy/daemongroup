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
* ws_dcli_ap_group.c
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

#include "ws_dcli_ap_group.h"
#include <syslog.h>
#include <sys/wait.h>
#include <dirent.h>


int ccgi_create_ap_group_cmd(dbus_parameter  parameter, DBusConnection *connection, char *ap_g_id, char *ap_g_name)
								/*返回1表示失败，返回0表示成功，返回2表示unknown id format*/
								/*返回3表示ap_g_id should be 1 to WTP_GROUP_NUM-1，返回4表示name is too long,out of the limit of 64*/
								/*返回5表示ap-group id exist，返回6表示error*/
{
	if((NULL == ap_g_id)||(NULL == ap_g_name))
		return 1;
	
	int ret,len;
	unsigned char isAdd;	
	unsigned int id;
	char *name = NULL;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
	int retu = 0;
	
	ret = parse_int_ID((char*)ap_g_id, &id);
	if(ret != WID_DBUS_SUCCESS){
		return 2;
	}	
	if(id >= WTP_GROUP_NUM || id == 0){
		return 3;
	}
	len = strlen(ap_g_name);
	if(len > WTP_AP_GROUP_NAME_MAX_LEN){		
		return 4;
	}
	name = (char*)malloc(strlen(ap_g_name)+1);
	if(NULL == name)
		return 1;
	memset(name, 0, strlen(ap_g_name)+1);
	memcpy(name, ap_g_name, strlen(ap_g_name));		
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_CREATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&id,
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
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
		return 1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		retu = 0;
	}
	else if(ret == GROUP_ID_EXIST)
	{
		retu = 5;
	}
	else
	{
		retu = 6;
	}

	dbus_message_unref(reply);
	FREE_OBJECT(name);
	return retu;	
}

int ccgi_del_ap_group_cmd(dbus_parameter  parameter, DBusConnection *connection, char *ap_g_id)
										/*返回0表示成功，返回-1表示失败，返回-2表示unknown id format*/
										/*返回-3表示ap_g_id should be 1 to WTP_GROUP_NUM-1，返回-4表示(NULL == reply)*/
										/*返回-5表示ap group id does not exist，返回-6表示error*/
{
	if((NULL == ap_g_id)||(NULL == connection))
		return -1;
	
	int ret;
	unsigned int id;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	int retu = 0;
	
	ret = parse_int_ID((char*)ap_g_id, &id);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(id >= WTP_GROUP_NUM || id == 0){
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_DEL);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&id,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -4;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		retu = -0;
	}
	else if(ret == GROUP_ID_NOT_EXIST)
	{
		retu = -5;
	}
	else
	{
		retu = -6;
	}

	dbus_message_unref(reply);

	return retu;	
}

/*oper为"add"或"delete"*/
/*wtp_id_list为"all"或AP ID列表，格式为1,8,9-20,33*/
int ccgi_add_del_ap_group_member_cmd(dbus_parameter  parameter, DBusConnection *connection,char *ap_g_id,char *oper,char *wtp_id_list)
									/*返回0表示成功，返回1表示一部分ap 没有删除成功，*/
									/*返回-1表示指针为空返回-2表示操作只能是add和delete;*/
									/*-3表示set wtp list error,like 1,8,9-20,33，*/
									/*返回-4表示ap group id  不正确，返回-5表示ap group id  不存在，返回-6表示error*/
{
	int index = 0;
	if((NULL == connection)||(NULL == ap_g_id)||(NULL == oper)||(NULL == wtp_id_list))
		return -1;
	
	int ret;
	unsigned int isadd = 1;	
	unsigned int GROUPID = 0;	
	update_wtp_list *wtplist = NULL;
	unsigned int *wtp_list = NULL;
	unsigned int apcount = 0;
	int retu = 0;
	
	if(strncmp("add",oper,(strlen(oper)>3)?3:strlen(oper))==0){
		isadd = 1;
	}else if(strncmp("delete",oper,(strlen(oper)>6)?6:strlen(oper))==0){
		isadd = 0;
	}else{
		return -2;
	}
	
	wtplist = (struct tag_wtpid_list*)malloc(sizeof(struct tag_wtpid_list));
	if(NULL == wtplist)
		return -1;
	wtplist->wtpidlist = NULL ; 	
	wtplist->count = 0;
	
	if (!strcmp(wtp_id_list,"all"))
	{
		wtplist->wtpidlist = NULL ; 
		wtplist->count = 0;	
	}else{
		ret = parse_wtpid_list((char*)wtp_id_list,&wtplist);
		if(ret != 0)
		{
			struct tag_wtpid * tmp = wtplist->wtpidlist;
			while(tmp){
				tmp = tmp->next;
			}
			destroy_input_wtp_list(wtplist);
			return -3;
		}
		else
		{
			delsame(wtplist);			
		}
	}
	index = parameter.instance_id;
	ret = parse_int_ID((char*)ap_g_id, &GROUPID);
	if(ret != WID_DBUS_SUCCESS){
		return -4;
	}	
	if(GROUPID >= WTP_GROUP_NUM || GROUPID == 0){
		return -4;
	}
		
	int(*dcli_init_func)(
						int ,
						int ,
						unsigned int ,
						int ,
						struct tag_wtpid_list * ,
						unsigned int **,
						unsigned int *,
						DBusConnection *
						);

	dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ap_group_add_del_member");

	ret =(*dcli_init_func)
		  (
		  	  0,
			  index,
			  GROUPID,
			  isadd,
			  wtplist,
			  &wtp_list,
			  &apcount,
			  connection
		  );

	if(ret == 0)
	{
		retu = 0;
		if(apcount != 0){
			retu = 1;
			FREE_OBJECT(wtp_list);
		}
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		retu = -5;
	}
	else
	{
		retu = -6;
	}

	return retu;	
}

void Free_ccgi_show_group_member_cmd(unsigned int *wtp_list)
{	
	if(wtp_list){
		free(wtp_list);
		wtp_list = NULL;
	}
}

/*只要调用函数，就调用Free_ccgi_show_group_member_cmd()释放空间*/
int ccgi_show_group_member_cmd(dbus_parameter  parameter, DBusConnection *connection,unsigned int id,unsigned int **wtp_list,unsigned int *count)
									/*返回0表示失败，返回1表示成功，返回-1表示ap group id非法*/
																					   /*返回-2表示ap group id does not exist，返回-3表示error*/
{
	int ret;
	unsigned int groupid = 0;	
	unsigned int apcount = 0;
	int index = 0;
	int retu = 0;

    	index= parameter.instance_id;
	groupid = id;
	if(groupid >= WTP_GROUP_NUM || groupid == 0){
		
		return -2;
	}
		
	int(*dcli_init_func)(			
						int ,
						int ,
						unsigned int ,
						unsigned int **,
						unsigned int *,
						DBusConnection *
						);

	dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ap_group_show_member");

	ret =(*dcli_init_func)
		  (
		  	  0,
			  index,
			  groupid,
			  wtp_list,
			  &apcount,
			  connection
		  );

	if(ret == 0)
	{	
		*count=apcount;
		retu = 0;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		retu = -3;
	}
	else
	{
		retu = -4;
	}

	return retu;	
}
void Free_ccgi_show_ap_group_cmd(struct ap_group_list *head)
{
	struct ap_group_list *f1= NULL, *f2= NULL;
	if(head)
	{	
		f1= head->next;
		if(f1)
		{
			f2=f1->next;
			while(f2!=NULL)
			{
				FREE_OBJECT(f1->test_name);
				free(f1);
				f1=f2;
				f2=f2->next;
			}
			FREE_OBJECT(f1->test_name);
			free(f1);
			f1=f2;
			f2=f2->next;
		}
	}
}

/*只要调用函数，就调用Free_ccgi_show_ap_group_cmd()释放空间*/
int ccgi_show_ap_group_cmd(dbus_parameter  parameter, DBusConnection *connection, struct ap_group_list *head, unsigned int *ap_count_ptr)
											/*返回-1表示失败，返回0表示成功，返回-2表示(NULL == reply)*/
{
	if((NULL == connection)||(NULL == head))
		return  -1;

	int ret;
	char  i = 0,len=0;
	unsigned int ap_group_count = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	unsigned int test_id;
	unsigned char *test_name;
	struct ap_group_list *q=NULL, *tail=NULL;


	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_SHOW_ALL);

	dbus_error_init(&err);
    	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -2;
	}

    	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ap_group_count);
	*ap_count_ptr = ap_group_count;
	head->next = NULL;
	tail=head;
	for(i=0; i < ap_group_count; i++){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&test_id);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&test_name);
		q=(struct ap_group_list*)malloc(sizeof(struct ap_group_list));
		if(NULL != q)
		{
			q->test_id = test_id;
			len = strlen(test_name)+1;
			q->test_name = (char *)malloc(len);
			if(q->test_name != NULL)
			{
				memset(q->test_name, 0, len);
				memcpy(q->test_name, test_name, len);
			}
			q->next = NULL;
			tail->next = q;
			tail=q;
		}
	}
	dbus_message_unref(reply);

	return 0; 
}


#ifdef __cplusplus
}
#endif


