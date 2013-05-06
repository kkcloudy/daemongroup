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
* ws_dcli_wqos.c
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

#include "ws_dcli_wqos.h"

int wid_wqos_parse_char_ID(char* str,unsigned char* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			return WID_DBUS_SUCCESS;
		}
		else
		{
			return WID_UNKNOWN_ID;
		}
	}
	else
	{
		return WID_UNKNOWN_ID;
	}
}
int wid_wqos_parse_short_ID(char* str,unsigned short* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			return WID_DBUS_SUCCESS;
		}
		else
		{
			return WID_UNKNOWN_ID;
		}
	}
	else
	{
		return WID_UNKNOWN_ID;
	}
}


int parse_dot1p_list(char* ptr,unsigned char* count,unsigned char dot1plist[])
{
	char* endPtr = NULL;
	int   i=0;
	endPtr = ptr;
	dot1p_list_state state = qos_check_dot1p_state;

	while(1){
		switch(state){
			
		case qos_check_dot1p_state: 
				dot1plist[i] = strtoul(endPtr,&endPtr,10);
				
				if((dot1plist[i])<8){
            		state=qos_check_comma_state;
				}
				else
					state=qos_check_fail_state;
				
				break;
		
		case qos_check_comma_state:
			if (QOS_DOT1P_COMMA == endPtr[0]){
				endPtr = (char*)endPtr + 1;
				state = qos_check_dot1p_state;
				
				i++;
				}
			else
				state = qos_check_end_state;
			break;
				
		
		case qos_check_fail_state:
			
			return -1;
			break;

		case qos_check_end_state: 
			
			if ('\0' == endPtr[0]) {
				state = qos_check_success_state;
				i++;
				}
			else
				state = qos_check_fail_state;
				break;
			
		case qos_check_success_state: 
			
			
			*count = i;
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

unsigned char QosRemoveListRepId(unsigned char list[],unsigned char num)

{
	int i,j,k;
	for(i=0;i<num;i++){ 
        for(j=i+1;j<num;j++)  { 
              if(list[i]==list[j])  { 
                  num--;
                  for(k=j;k<num;k++) { 
                       list[k]=list[k+1]; 
                  } 
                  j--; 
               } 
         } 
     } 
     for(i=0;i<num;i++){
	 //printf("%d",list[i]);
     //printf("\n");
	 }
     return num; 

}


/*dcli_wqos.c V1.25*/
/*author qiaojie*/
/*update time 10-01-13*/

int create_qos(dbus_parameter parameter, DBusConnection *connection,char *id, char *qos_name)
														  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
                                             			  /*返回-3表示qos name is too long,it should be 1 to 15，返回-4表示qos id exist，返回-5表示error*/
														  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == id)||(NULL == qos_name))
		return 0;
	
	int ret,len,retu;
	int isAdd = 1;	
	int qos_id = 0;
	char *name = NULL;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 1;			

	ret = parse_int_ID((char*)id, &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}
	
	if(qos_id >= QOS_NUM || qos_id <= 0){
		return -2;
	}
	
	len = strlen(qos_name);
	if(len > 15){
		return -3;
	}
	name = (char*)malloc(strlen(qos_name)+1);
	if(NULL == name)
        return 0;
	memset(name, 0, strlen(qos_name)+1);
	memcpy(name, qos_name, strlen(qos_name));	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&qos_id,
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
		return SNMPD_CONNECTION_ERROR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			retu=1;
		else if(ret == WID_QOS_BE_USED)
			retu=-4;
		else
			retu=-5;

	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}	
	return retu;	
}


int delete_qos(dbus_parameter parameter, DBusConnection *connection,char *id)
										  /*返回0表示 删除失败，返回1表示删除成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
                            			  /*返回-3表示qos id does not exist，返回-4表示this qos profile be used by some radios,please disable them first，返回-5表示error*/
										  /*返回-6表示this qos now be used by some radios,please delete them*/
										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == id)
		return 0;
	
	int ret,retu;
	int isAdd = 1;	
	int qos_id = 0;
	char *name = NULL;
	char *name_d = "0";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 0;	
	ret = parse_int_ID((char*)id, &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		return -2;
	}

	name = name_d;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&qos_id,
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
		else if(ret == WID_QOS_NOT_EXIST)
			retu=-3;
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)	
			retu=-4;
		else if(ret == WID_QOS_BE_USED_BY_RADIO)
			retu=-6;
		else
			retu=-5;

	dbus_message_unref(reply);
	return retu;	
}

void Free_qos_one(DCLI_WQOS *WQOS)
{
	void (*dcli_init_free_func)(char *,DCLI_WQOS *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_wqos_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_QOS_METHOD_SHOW_QOS,WQOS);
		}
	}
}


/*返回1时，调用Free_qos_one()释放空间*/
int show_qos_one(dbus_parameter parameter, DBusConnection *connection,char *id,DCLI_WQOS **WQOS)
																/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
                                                                /*返回-3表示qos id does not exist，返回-4表示error*/
																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if(NULL == id)
	{
		*WQOS = NULL;
		return 0;
	}
	
	int ret;
	int qos_id = 0;
	int retu;
	
	ret = parse_int_ID((char*)id, &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		return -2;
	}


	void*(*dcli_init_func)(
						int ,
						unsigned int* ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						DBusConnection *,
						char *
						);

    *WQOS = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_wqos_wireless_qos_show_config_info");
		if(NULL != dcli_init_func)
		{
			*WQOS =(*dcli_init_func)
				  (
					  parameter.instance_id,
					  &ret,
					  parameter.local_id,
					  qos_id,		  
					  0,
					  connection,
					  WID_DBUS_QOS_METHOD_SHOW_QOS
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
	else if((ret == 0)&&(*WQOS))
	{
		retu = 1;
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		retu = -3;
	}
	else 
	{	
		retu = -4;
	}

	return retu;
}



void Free_qos_head(DCLI_WQOS *WQOS)
{
	void (*dcli_init_free_func)(char *,DCLI_WQOS *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_wqos_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_QOS_METHOD_SHOW_QOS_LIST,WQOS);
		}
	}
}


/*返回1时，调用Free_qos_head()释放空间*/
int show_wireless_qos_profile_list(dbus_parameter parameter, DBusConnection *connection,DCLI_WQOS **WQOS)/*返回0表示失败，返回1表示成功，返回-1表示qos not exsit*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	int ret = 0;
	int retu;
	
	void*(*dcli_init_func)(
						int ,
						unsigned int* ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						DBusConnection *,
						char *
						);

    *WQOS = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_wqos_wireless_qos_show_config_info");
		if(NULL != dcli_init_func)
		{
			*WQOS = (*dcli_init_func)
				(
					parameter.instance_id,/*"show wireless qos profile (all|list)"*/
					&ret,
					parameter.local_id,
					0,
					0,
					connection,
					WID_DBUS_QOS_METHOD_SHOW_QOS_LIST
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

	if(ret == -1){
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if((ret == 0)&&(*WQOS)){
		retu = 1;
	}
	else if(ret == 5){
		retu = -1;
	}
	
	return retu;

}


/*qos_stream的范围是"voice","video","besteffort"或"background"*/
/*ACK的范围是"ack"或"noack"*/
int config_radio_qos_service(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *qos_stream,char *CWMIN,char *CWMAX,char *AIFS,char *TXOPLIMIT,char *ACK)  
																							/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
																				 			/*返回-3表示qos cwmin should be 0 to QOS_CWMIN_NUM-1，返回-4表示qos cwmax should be 0 to QOS_CWMAX_NUM-1*/
																							/*返回-5表示qos aifs should be 0 to QOS_AIFS_NUM-1，返回-6表示qos txoplimit should be 0 to QOS_TXOPLIMIT_NUM*/
																						    /*返回-7表示qos profile does not exist，返回-8表示this qos profile is used by some radios,please disable them first*/
																						    /*返回-9表示error，返回-10表示WQOS ID非法，返回-11表示cwmin is not allow larger than cwmax*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	if((NULL == qos_stream)||(NULL == CWMIN)||(NULL == CWMAX)||(NULL == AIFS)||(NULL == TXOPLIMIT)||(NULL == ACK))
		return 0;
	
	int ret = 0,retu;
	int ID = 0;
	int qos_stream_id = 5;
	unsigned short cwmin = 0;
	unsigned short cwmax = 0;
	unsigned char aifs = 0;
	unsigned char ack = 2;
	unsigned short txoplimit = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)qos_id;

	if (!strcmp(qos_stream,"voice"))
		qos_stream_id = 3;			
	else if (!strcmp(qos_stream,"video"))
		qos_stream_id = 2;	
	else if (!strcmp(qos_stream,"besteffort"))
		qos_stream_id = 0;
	else if (!strcmp(qos_stream,"background"))
		qos_stream_id = 1;
	else 
	{		
		return -1;
	}
	//cwmin
	
	ret = wid_wqos_parse_short_ID((char*)CWMIN, &cwmin);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(cwmin >= QOS_CWMIN_NUM ){
		return -3;
	}
	//cwmax
	ret = wid_wqos_parse_short_ID((char*)CWMAX, &cwmax);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(cwmax >= QOS_CWMAX_NUM){
		return -4;
	}
	if(cwmin > cwmax)
	{
		return -11;		
	}
	//aifs
	ret = wid_wqos_parse_char_ID((char*)AIFS, &aifs);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(aifs >= QOS_AIFS_NUM ){
		return -5;
	}
	//txoplimit
	ret = wid_wqos_parse_short_ID((char*)TXOPLIMIT, &txoplimit);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(txoplimit > QOS_TXOPLIMIT_NUM ){
		return -6;
	}
	//ack
	if ((!strcmp(ACK,"ack")))
		ack = 1;
	else if ((!strcmp(ACK,"noack")))
		ack = 0;
	else 
	{		
		
		return -1;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = qos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in config_radio_qos_service is %d\n",ID);
		return -10;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_UINT32,&qos_stream_id,
							 DBUS_TYPE_UINT16,&cwmin,
							 DBUS_TYPE_UINT16,&cwmax,
							 DBUS_TYPE_BYTE,&aifs,
							 DBUS_TYPE_BYTE,&ack,
							 DBUS_TYPE_UINT16,&txoplimit,
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

	if(ret == WID_DBUS_SUCCESS)
		retu=1;
	else if(ret == WID_QOS_NOT_EXIST)			
		retu=-7;
	else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)	
		retu=-8;
	else
		retu=-9;

	dbus_message_unref(reply);
	return retu;
}


/*qos_stream的范围是"voice","video","besteffort"或"background"*/
int config_client_qos_service(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *qos_stream,char *CWMIN,char *CWMAX,char *AIFS,char *TXOPLIMIT) 
																								  /*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
																								  /*返回-3表示qos cwmin should be 0 to QOS_CWMIN_NUM-1，返回-4表示qos cwmax should be 0 to QOS_CWMAX_NUM-1*/
																								  /*返回-5表示qos aifs should be 0 to QOS_AIFS_NUM-1，返回-6表示qos txoplimit should be 0 to QOS_TXOPLIMIT_NUM*/
																				     			  /*返回-7表示qos profile does not exist，返回-8表示this qos profile is used by some radios,please disable them first*/
																				     			  /*返回-9表示error，返回-10表示WQOS ID非法，返回-11表示cwmin is not allow larger than cwmax*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
	
	if((NULL == qos_stream)||(NULL == CWMIN)||(NULL == CWMAX)||(NULL == AIFS)||(NULL == TXOPLIMIT))
		return 0;
	
	int ret = 0,retu;
	int ID = 0;
	int qos_stream_id = 5;
	unsigned short cwmin = 0;
	unsigned short cwmax = 0;
	unsigned char aifs = 0;
	unsigned short txoplimit = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int) qos_id;

	if (!strcmp(qos_stream,"voice"))
		qos_stream_id = 3;			
	else if (!strcmp(qos_stream,"video"))
		qos_stream_id = 2;	
	else if (!strcmp(qos_stream,"besteffort"))
		qos_stream_id = 0;
	else if (!strcmp(qos_stream,"background"))
		qos_stream_id = 1;
	else 
	{		
		return -1;
	}
	//cwmin
	ret = wid_wqos_parse_short_ID((char*)CWMIN, &cwmin);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(cwmin >= QOS_CWMIN_NUM ){
		return -3;
	}
	//cwmax
	ret = wid_wqos_parse_short_ID((char*)CWMAX, &cwmax);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(cwmax >= QOS_CWMAX_NUM){
		return -4;
	}
	if(cwmin > cwmax)
	{
		return -11;		
	}
	//aifs
	ret = wid_wqos_parse_char_ID((char*)AIFS, &aifs);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(aifs >= QOS_AIFS_NUM ){
		return -5;
	}
	//txoplimit
	ret = wid_wqos_parse_short_ID((char*)TXOPLIMIT, &txoplimit);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(txoplimit > QOS_TXOPLIMIT_NUM ){
		return -6;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = qos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in config_client_qos_service is %d\n",ID);
		return -10;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO_CLIENT);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO_CLIENT);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_UINT32,&qos_stream_id,
							 DBUS_TYPE_UINT16,&cwmin,
							 DBUS_TYPE_UINT16,&cwmax,
							 DBUS_TYPE_BYTE,&aifs,
							 DBUS_TYPE_UINT16,&txoplimit,
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

	if(ret == WID_DBUS_SUCCESS)
		retu=1;
	else if(ret == WID_QOS_NOT_EXIST)	
		retu=-7;
	else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)		
		retu=-8;
	else
		retu=-9;

	dbus_message_unref(reply);
	return retu;
}

/*state的范围是"enable"或"disable"*/
int config_wmm_service(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *state)  
																	 /*返回0表示失败，返回1表示成功，返回-1表示input parameter should be only 'enable' or 'disable'*/
																	 /*返回-2表示qos id does not exist，返回-3表示this qos profile be used by some radios,please disable them first*/
																	 /*返回-4表示error，返回-5表示WQOS ID非法*/
																	 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == state)
		return 0;
	
	int ret,retu;
	int isAdd = 2;	
	int ID = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)qos_id;
	
	if (!strcmp(state,"enable"))
		isAdd = 1;
	else if (!strcmp(state,"disable"))
		isAdd = 0;
	else 
	{		
		return -1;
	}


	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = qos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in config_wmm_service is %d\n",ID);
		return -5;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MAP);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MAP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_UINT32,&isAdd,						 
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-2;
	else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)	
		retu=-3;
	else
		retu=-4;

	dbus_message_unref(reply);
	return retu;	
}

/*WMM_ORDER的范围是"voice","video","besteffort"或"background"*/
int config_wmm_map_dotlp(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *WMM_ORDER,char *DOTLP)  
																						 /*返回0表示失败，返回1表示成功，返回-1表示input parameter should be only 'voice' 'video' 'besteffort' or 'background'*/
																						 /*返回-2表示unknown id format，返回-3表示qos dot1p should be 0 to 7，返回-4表示qos id does not exist*/
																						 /*返回-5表示this qos profile be used by some radios,please disable them first，返回-6表示this qos map is disable,please enable it first*/
																						 /*返回-7表示error，返回-8表示WQOS ID非法*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == WMM_ORDER)||(NULL == DOTLP))
		return 0;
	
	int ret=0,retu;
	int wmm_order = 4;	
	int ID = 0;
	unsigned char dot1p = 8;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)qos_id;
	
	
	if (!strcmp(WMM_ORDER,"voice"))
		wmm_order = 3;			
	else if (!strcmp(WMM_ORDER,"video"))
		wmm_order = 2;	
	else if (!strcmp(WMM_ORDER,"besteffort"))
		wmm_order = 0;
	else if (!strcmp(WMM_ORDER,"background"))
		wmm_order = 1;
	else 
	{		
		return -1;
	}
	
	
	//ret = parse_char_ID((char*)DOTLP, &dot1p);	
	dot1p = atoi(DOTLP);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}	
	if(dot1p >= 8 ){
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = qos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in config_wmm_map_dotlp is %d\n",ID);
		return -8;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_WMM_MAP_DOT1P);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_WMM_MAP_DOT1P);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_UINT32,&wmm_order,
						DBUS_TYPE_BYTE,&dot1p,
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-4;
	else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)	
		retu=-5;
	else if(ret == WID_QOS_WMM_MAP_DISABLE)	
		retu=-6;
	else
		retu=-7;

	dbus_message_unref(reply);
	return retu;	
}

/*WMM_ORDER的范围是"voice","video","besteffort"或"background"*/
int config_dotlp_map_wmm(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *DOTLP,char *WMM_ORDER) 
																						 /*返回0表示失败，返回1表示成功，返回-1表示input parameter is illegal*/
																						 /*返回-2表示input parameter should be only 'voice' 'video' 'besteffort' or 'background'，返回-3表示qos id does not exist*/
																						 /*返回-4表示this qos profile be used by some radios,please disable them first，返回-5表示this qos map is disable,please enable it first*/
																						 /*返回-6表示error，返回-7表示WQOS ID非法*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if((NULL == DOTLP)||(NULL == WMM_ORDER))
		return 0;
	
	int ret,retu;
	int wmm_order = 4;	
	int ID = 0;
	unsigned char dot1p[DCLIWQOS_DOT1P_LIST_NUM];
	unsigned char n = 0;
	unsigned char num = 0;
	int i = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)qos_id;
	
	ret = parse_dot1p_list((char*)DOTLP,&n,dot1p);

	num = QosRemoveListRepId(dot1p,n);
	
	if (-1 == ret) {
		return -1;
	}

	
	if (!strcmp(WMM_ORDER,"voice"))
		wmm_order = 3;			
	else if (!strcmp(WMM_ORDER,"video"))
		wmm_order = 2;	
	else if (!strcmp(WMM_ORDER,"besteffort"))
		wmm_order = 0;
	else if (!strcmp(WMM_ORDER,"background"))
		wmm_order = 1;
	else 
	{		
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = qos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in config_dotlp_map_wmm is %d\n",ID);
		return -7;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_DOT1P_MAP_WMM);
	
	/*ery = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_DOT1P_MAP_WMM);*/
	dbus_error_init(&err);

	
	dbus_message_iter_init_append (query, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ID);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&num);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wmm_order);
			
	for(i = 0; i < num; i++)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&dot1p[i]);
	}
	
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-3;
	else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)
		retu=-4;
	else if(ret == WID_QOS_WMM_MAP_DISABLE)			
		retu=-5;
	else
		retu=-6;

	dbus_message_unref(reply);
	return retu;	
}


void Free_qos_extension_info(DCLI_WQOS *WQOS)
{
	void (*dcli_init_free_func)(char *,DCLI_WQOS *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_wqos_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO,WQOS);
		}
	}
}


/*返回1时，调用Free_qos_extension_info()释放空间*/
int show_qos_extension_info(dbus_parameter parameter, DBusConnection *connection,char *wqos_id,DCLI_WQOS **WQOS) 
																				  /*返回0表示失败，返回1表示成功*/
																				  /*返回-1表示unknown id format*/
																				  /*返回-2表示qos id should be 1 to QOS_NUM-1*/
																				  /*返回-3表示qos id does not exist，返回-4表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if(NULL == wqos_id)
	{
		*WQOS = NULL;
		return 0;
	}
	
	int ret = 0;
	int qos_id = 0;
	int retu;

	ret = parse_int_ID((char*)wqos_id, &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		return -2;
	}

	void*(*dcli_init_func)(
						int ,
						unsigned int* ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						DBusConnection *,
						char *
						);

    *WQOS = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_wqos_wireless_qos_show_config_info");
		if(NULL != dcli_init_func)
		{
			*WQOS =(*dcli_init_func)
					(
						parameter.instance_id,
						&ret,
						parameter.local_id,
						qos_id, 		
						0,
						connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO
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

	if(ret == -1){
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if((ret == 0)&&(*WQOS))
	{
		retu = 1;
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		retu = -3;
	}
	else 
	{	
		retu = -4;
	}
	
	return retu;
}

int set_qos_total_bandwidth(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *value)
																		/*返回0表示失败，返回1表示成功*/
																		/*返回-1表示unknown id format，返回-2表示qos dot1p should be 1 to 25*/
																		/*返回-3表示qos id does not exist，返回-4表示error，返回-5表示WQOS ID非法*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == value)
		return 0;
	
	int ret,retu;
	unsigned char bandwidth = 0;	
	int ID = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)wqos_id;
	
	ret = wid_wqos_parse_char_ID((char*)value, &bandwidth);
	
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(bandwidth > 25){
		return -2;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ID = wqos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in set_qos_total_bandwidth is %d\n",ID);
		return -5;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_TOTAL_BANDWIDTH);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_TOTAL_BANDWIDTH);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&bandwidth,						 
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-3;
	else
		retu=-4;

	dbus_message_unref(reply);
	return retu;	
}

/*flow_type的范围是"besteffort","background","video"或"voice"*/
/*par_type的范围是"averagerate","maxburstiness","managepriority","shovepriority","grabpriority","maxparallel","bandwidth"或"bandwidthpercentage"*/
int wid_config_set_qos_flow_parameter(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *flow_type,char *par_type,char *para_value)  
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown qos flow type*/
																									/*返回-2表示unknown qos parameter type，返回-3表示unknown value format*/
																									/*返回-4表示qos id does not exist，返回-5表示error，返回-6表示WQOS ID非法*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == flow_type)||(NULL == par_type)||(NULL == para_value))
		return 0;
	
	int ret,retu;
	unsigned int value = 0;	
	int ID = 0;
	unsigned char qos_stream_id = 0;
	unsigned int paramater_type = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)wqos_id;

	if ((!strcmp(flow_type,"BESTEFFORT"))||(!strcmp(flow_type,"besteffort")))
	{
		qos_stream_id = 0;
	}
	else if ((!strcmp(flow_type,"BACKGROUND"))||(!strcmp(flow_type,"background")))
	{
		qos_stream_id = 1;
	}
	else if ((!strcmp(flow_type,"VIDEO"))||(!strcmp(flow_type,"video")))
	{
		qos_stream_id = 2;	
	}
	else if ((!strcmp(flow_type,"VOICE"))||(!strcmp(flow_type,"voice")))
	{	
		qos_stream_id = 3;			
	}
	else 
	{		
		return -1;
	}

	if (!strcmp(par_type,"averagerate"))
	{
		paramater_type = 1;
	}
	else if (!strcmp(par_type,"maxburstiness"))
	{
		paramater_type = 2;
	}
	else if (!strcmp(par_type,"managepriority"))
	{
		paramater_type = 3;	
	}
	else if (!strcmp(par_type,"shovepriority"))
	{	
		paramater_type = 4;			
	}
	else if (!strcmp(par_type,"grabpriority"))
	{	
		paramater_type = 5;			
	}	
	else if (!strcmp(par_type,"maxparallel"))
	{
		paramater_type = 6;	
	}
	else if (!strcmp(par_type,"bandwidth"))
	{	
		paramater_type = 7;			
	}
	else if (!strcmp(par_type,"bandwidthpercentage"))
	{	
		paramater_type = 8;			
	}
	else if (!strcmp(par_type,"flowqueuelenth"))
	{	
		paramater_type = 9;			
	}
	else if (!strcmp(par_type,"flowaveragerate"))
	{	
		paramater_type = 10;			
	}
	else if (!strcmp(par_type,"flowmaxburstiness"))
	{	
		paramater_type = 11;			
	}
	else 
	{		
		return -2;
	}

	ret = parse_int_ID((char*)para_value, &value);
	
	if(ret != WID_DBUS_SUCCESS){
		return -3;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ID = wqos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in wid_config_set_qos_flow_parameter is %d\n",ID);
		return -6;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&qos_stream_id,	
						DBUS_TYPE_UINT32,&paramater_type,
						DBUS_TYPE_UINT32,&value,
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-4;
	else
		retu=-5;

	dbus_message_unref(reply);
	return retu;	
}


/*par_type的范围是"totalbandwidth","resourcescale","sharebandwidth"或"resourcesharescale"*/
int wid_config_set_qos_parameter(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *par_type,char *par_value) 
																									/*返回0表示失败，返回1表示成功*/
																									/*返回-1表示unknown qos parameter type，返回-2表示unknown value format*/
																									/*返回-3表示qos id does not exist，返回-4表示error，返回-5表示WQOS ID非法*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == par_type)||(NULL == par_value))
		return 0;
	
	int ret,retu;
	unsigned int value = 0;	
	int ID = 0;
	unsigned int paramater_type = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)wqos_id;


	if (!strcmp(par_type,"totalbandwidth"))
	{
		paramater_type = 1;
	}
	else if (!strcmp(par_type,"resourcescale"))
	{
		paramater_type = 2;
	}
	else if (!strcmp(par_type,"sharebandwidth"))
	{
		paramater_type = 3;	
	}
	else if (!strcmp(par_type,"resourcesharescale"))
	{	
		paramater_type = 4;			
	}
	else 
	{		
		return -1;
	}
	
	ret = parse_int_ID((char*)par_value, &value);
	
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = wqos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in wid_config_set_qos_parameter is %d\n",ID);
		return -5;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_PARAMETER);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_PARAMETER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_UINT32,&paramater_type,
						DBUS_TYPE_UINT32,&value,
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-3;
	else
		retu=-4;

	dbus_message_unref(reply);
	return retu;	
}

/*policy_type的范围是"grab"或"shove"*/
int wid_config_set_qos_policy_used(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *policy_type,char *policy_state) 
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示unknown qos policy type*/
																											/*返回-2表示qos id does not exist，返回-3表示error*/
																											/*返回-4表示WQOS ID非法*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == policy_type)||(NULL == policy_state))
		return 0;
		
	int ret,retu;
	int ID = 0;
	unsigned char policy = 0;
	unsigned char used = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)wqos_id;

	if (!strcmp(policy_type,"grab"))
	{
		policy = 1;	
	}
	else if (!strcmp(policy_type,"shove"))
	{	
		policy = 2;			
	}
	else 
	{		
		return -1;
	}

	if (!strcmp(policy_state,"used"))
	{
		used = 1;	
	}
	else if (!strcmp(policy_state,"unused"))
	{	
		used = 0;			
	}
	else 
	{		
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = wqos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in wid_config_set_qos_policy_used is %d\n",ID);
		return -4;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&policy,	
						DBUS_TYPE_BYTE,&used,
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
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-2;
	else
		retu=-3;

	dbus_message_unref(reply);
	return retu;	
}

/*policy_type的范围是"grab"或"shove"*/
int wid_config_set_qos_policy_name(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *policy_type,char *policy_name) 
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示policy name too long，返回-2表示unknown qos policy type*/
																											/*返回-3表示qos id does not exist，返回-4表示error，返回-5表示WQOS ID非法*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == policy_type)||(NULL == policy_name))
		return 0;
	
	int ret,retu;
	int ID = 0;
	unsigned char policy = 0;
	char *name;
	unsigned int lenth = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)wqos_id;

	lenth = strlen((char *)policy_name);

	if(lenth > 15)
	{		
		return -1;
	}
	
	name = (char *)malloc(lenth+1);
	if(NULL == name)
        return 0;
	memset(name,0,lenth+1);
	memcpy(name,policy_name,lenth);
	
	if (!strcmp(policy_type,"grab"))
	{
		policy = 1;	
	}
	else if (!strcmp(policy_type,"shove"))
	{	
		policy = 2;			
	}
	else 
	{		
		if(name)
		{
			free(name);
			name = NULL;
		}
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = wqos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in wid_config_set_qos_policy_name is %d\n",ID);
		return -5;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY_NAME);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY_NAME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&policy,	
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
		return SNMPD_CONNECTION_ERROR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu=1;
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-3;
	else
		retu=-4;

	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return retu;	
}


int wid_config_set_qos_manage_arithmetic_name(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *arithmetic_name)  
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示arithmetic name too long*/
																											/*返回-2表示qos id does not exist，返回-3表示error*/
																											/*返回-4表示WQOS ID非法*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == arithmetic_name)
		return 0;
	
	int ret,retu;
	int ID = 0;
	char *name;
	unsigned int lenth = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)wqos_id;

	lenth = strlen((char *)arithmetic_name);

	if(lenth > 15)
	{		
		return -1;
	}
	
	name = (char *)malloc(lenth+1);
	if(NULL == name)
        return 0;
	memset(name,0,lenth+1);
	memcpy(name,arithmetic_name,lenth);
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ID = wqos_id;
	if(ID >= QOS_NUM || ID == 0){
		syslog(LOG_DEBUG,"wqos id in wid_config_set_qos_manage_arithmetic_name is %d\n",ID);
		return -4;
	}
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MANAGE_ARITHMETIC_NAME);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MANAGE_ARITHMETIC_NAME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,	
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
		return SNMPD_CONNECTION_ERROR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		retu=1;
	else if(ret == WID_QOS_NOT_EXIST)
		retu=-2;
	else
		retu=-3;

	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return retu;	
}



/* type为"besteffort"|"background"|"video"|"voice" */
/*R_L_ID的范围是0-3*/
/*返回1时，调用Free_qos_one()释放空间*/
int wid_show_qos_radio_cmd(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,char *R_L_ID,char *type,DCLI_WQOS **WQOS)
																										/*返回0表示失败，返回1表示成功，返回-1表示unknown wtpid format*/
																									    /*返回-2表示unknown local radio id format，返回-3表示wtp id should be 1 to WTP_NUM-1*/
																									    /*返回-4表示local radio id should be 0 to 3，返回-5表示unknown qos flow type*/
																									    /*返回-6表示wtp id not exist，返回-7表示radio id not exist，返回-8表示qos id not exist*/
																									    /*返回-9表示wtp WTPID radio radio_l_id didn't bind qos，返回-10表示qos id should be 1 to QOS_NUM-1*/
																									    /*返回-11表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	if((NULL == wtp_id)||(NULL == R_L_ID)||(NULL == type))
	{
		*WQOS = NULL;
		return 0;
	}
	
	int ret,ret2;
	int radio_l_id = 0;
	int	WTPID = 0;
	int qostype = 0;
	int qos_id = 0;
	int retu;
	
	ret = parse_int_ID((char*)wtp_id, &WTPID);
	ret2 = parse_int_ID((char*)R_L_ID, &radio_l_id);
	if(ret != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(ret2 != WID_DBUS_SUCCESS){
		return -2;
	}
	if(WTPID >= WTP_NUM || WTPID == 0){
		return -3;
	}
	if(radio_l_id >= 4){
		return -4;
	}
	
	if ((!strcmp(type,"BESTEFFORT"))||(!strcmp(type,"besteffort")))
	{
		qostype = 0;
	}
	else if ((!strcmp(type,"BACKGROUND"))||(!strcmp(type,"background")))
	{
		qostype = 1;
	}
	else if ((!strcmp(type,"VIDEO"))||(!strcmp(type,"video")))
	{
		qostype = 2;	
	}
	else if ((!strcmp(type,"VOICE"))||(!strcmp(type,"voice")))
	{	
		qostype = 3;			
	}
	else 
	{		
		return -5;
	}
	
	void*(*dcli_init_func1)(
						int ,
						unsigned int* ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						DBusConnection *,
						char *
						);

    *WQOS = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func1 = dlsym(ccgi_dl_handle,"dcli_wqos_wireless_qos_show_config_info");
		if(NULL != dcli_init_func1)
		{
			*WQOS =(*dcli_init_func1)
				(
					parameter.instance_id,
					&ret,
					parameter.local_id,
					WTPID,			
					radio_l_id,
					connection,
					WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO
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
	else if(ret == 0 )
	{	
		qos_id = (*WQOS)->qos[0]->QosID;
		
		void (*dcli_init_free_func)(char *,DCLI_WQOS *);
		if(NULL != ccgi_dl_handle)
		{
			dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_wqos_free_fun");
			if(NULL != dcli_init_free_func)
			{
				dcli_init_free_func(WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO,*WQOS); 
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
	}
	else {
		
		
		if(ret == WTP_ID_NOT_EXIST){
			retu = -6;
		}
		else if(ret == RADIO_ID_NOT_EXIST){
			retu = -7;
		}
		else if(ret == WID_QOS_NOT_EXIST){
			retu = -8;
		}
		else if(ret == RADIO_NO_BINDING_WQOS){
			retu = -9;
		}
		else if(qos_id >= QOS_NUM || qos_id == 0){
			retu = -10;
		}
		else{
			retu = -11;
		}
		//dbus_message_unref(reply);
		return retu;	
	}

	void*(*dcli_init_func2)(
						int ,
						unsigned int* ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						DBusConnection *,
						char *
						);

    *WQOS = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func2 = dlsym(ccgi_dl_handle,"dcli_wqos_wireless_qos_show_config_info");
		if(NULL != dcli_init_func2)
		{
			*WQOS =(*dcli_init_func2)
				(
					parameter.instance_id,
					&ret,
					parameter.local_id,
					qos_id, 		
					0,
					connection,
					WID_DBUS_QOS_METHOD_SHOW_QOS
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
	

	if(ret == -1){
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if(ret == 0){
		retu = 1;
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		retu = -8;
	}
	else 
	{
		retu = -11;
	}
	
	//dbus_message_unref(reply);
	return retu;
}


#ifdef __cplusplus
}
#endif

