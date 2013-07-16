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
* ws_sta.c
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

#include "ws_sta.h"

/*dcli_sta.c V1.34*/
/*author qiaojie*/
/*update time 09-2-19*/

/*dcli_sta.c V1.90*/
/*author qiaojie*/
/*update time 09-12-10*/

/*dcli_sta.c V1.114*/
/*author zhouym*/
/*update time 10-01-21*/

/*dcli_sta.c V1.133*/
/*author qiaojie*/
/*update time 10-03-31*/


int parse2_char_ID(char* str,unsigned char* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='1'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
			return ASD_DBUS_SUCCESS;
		else
			return ASD_UNKNOWN_ID;
	}
	else
		return ASD_UNKNOWN_ID;
	
}

void asd_state_check(unsigned char *ieee80211_state, unsigned int sta_flags, unsigned char *PAE, unsigned int pae_state, unsigned char *BACKEND, unsigned int backend_state){
	if((sta_flags&1) != 0){
		if((sta_flags&2) != 0){
			if((sta_flags&32) != 0){
				memcpy(ieee80211_state,"Autherized",strlen("Autherized"));
		}else{
				if((sta_flags&1024) != 0)
					memcpy(ieee80211_state,"Roaming",strlen("Roaming"));
				else
				memcpy(ieee80211_state,"Assoc",strlen("Assoc"));
			}

		}else
			memcpy(ieee80211_state,"Auth",strlen("Auth"));
	}else
		memcpy(ieee80211_state,"Unconnected",strlen("Unconnected"));
	
	switch(pae_state){
		case PAE_INITIALIZE :
			memcpy(PAE,"PAE_INITIALIZE",strlen("PAE_INITIALIZE"));
			break;
		case PAE_DISCONNECTED :			
			memcpy(PAE,"PAE_DISCONNECTED",strlen("PAE_DISCONNECTED"));
			break;
		case PAE_CONNECTING :
			memcpy(PAE,"PAE_CONNECTING",strlen("PAE_CONNECTING"));
			break;
		case PAE_AUTHENTICATING :
			memcpy(PAE,"PAE_AUTHENTICATING",strlen("PAE_AUTHENTICATING"));
			break;
		case PAE_AUTHENTICATED :
			memcpy(PAE,"PAE_AUTHENTICATED",strlen("PAE_AUTHENTICATED"));
			break;
		case PAE_ABORTING :
			memcpy(PAE,"PAE_ABORTING",strlen("PAE_ABORTING"));
			break;
		case PAE_HELD :
			memcpy(PAE,"PAE_HELD",strlen("PAE_HELD"));
			break;
		case PAE_FORCE_AUTH :
			memcpy(PAE,"PAE_FORCE_AUTH",strlen("PAE_FORCE_AUTH"));
			break;
		case PAE_FORCE_UNAUTH :
			memcpy(PAE,"PAE_FORCE_UNAUTH",strlen("PAE_FORCE_UNAUTH"));
			break;
		case PAE_RESTART :
			memcpy(PAE,"PAE_RESTART",strlen("PAE_RESTART"));
			break;
		default:
			memcpy(PAE,"NONE",strlen("NONE"));
			break;
	}

	switch(backend_state){
		case AUTH_REQUEST:
			memcpy(BACKEND,"AUTH_REQUEST",strlen("AUTH_REQUEST"));
			break;
		case AUTH_RESPONSE:
			memcpy(BACKEND,"AUTH_RESPONSE",strlen("AUTH_RESPONSE"));
			break;
		case AUTH_SUCCESS:
			memcpy(BACKEND,"AUTH_SUCCESS",strlen("AUTH_SUCCESS"));
			break;
		case AUTH_FAIL:
			memcpy(BACKEND,"AUTH_FAIL",strlen("AUTH_FAIL"));
			break;
		case AUTH_TIMEOUT:
			memcpy(BACKEND,"AUTH_TIMEOUT",strlen("AUTH_TIMEOUT"));
			break;
		case AUTH_IDLE:
			memcpy(BACKEND,"AUTH_IDLE",strlen("AUTH_IDLE"));
			break;
		case AUTH_INITIALIZE:
			memcpy(BACKEND,"AUTH_INITIALIZE",strlen("AUTH_INITIALIZE"));
			break;
		case AUTH_IGNORE:
			memcpy(BACKEND,"AUTH_IGNORE",strlen("AUTH_IGNORE"));
			break;
		default:
			memcpy(BACKEND,"NONE",strlen("NONE"));
			break;

	}

}


void Free_bss_bymac(struct dcli_sta_info *sta)
{
	void (*dcli_init_free_func)(struct dcli_sta_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_sta");	
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(sta);
		}
	}
}

/*只要调用函数，就调用Free_bss_bymac()释放空间*/
int show_sta_bymac(dbus_parameter parameter, DBusConnection *connection,char *arg_mac,struct dcli_sta_info **sta)/*返回0表示失败，返回1表示成功，返回-1表示station does not exist,返回-2表示error*/
																													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if(NULL == arg_mac)
	{
		*sta = NULL;
		return 0;
	}
	
	unsigned char   mac1[6] ={'\0','\0','\0','\0','\0','\0'};

	unsigned int mac[6];
	unsigned int ret;
	int retu;

	memset(mac,0,MAC_LEN);
	sscanf(arg_mac,"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
   
	
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	


	
	void *(*dcli_init_func)
		(DBusConnection *,
		int , 
		unsigned char *, 
		int ,
		unsigned int *
		);

	*sta = NULL;		
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"get_sta_info_by_mac");
		if(NULL != dcli_init_func)
		{
			*sta = (*dcli_init_func)(connection,
			                         parameter.instance_id,
			                         mac1,
			                         parameter.local_id,
			                         &ret);
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

	if((ret == 0) && (*sta != NULL))
	{
		(*sta)->radio_l_id = (*sta)->radio_g_id%L_RADIO_NUM;
		(*sta)->wtp_id = (*sta)->radio_g_id/L_RADIO_NUM;
		retu = 1;
	}
	else if (ret == ASD_STA_NOT_EXIST)
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -2;
	}
	return retu ;	
}


int kick_sta_MAC(dbus_parameter parameter, DBusConnection *connection,char * mac_addr)
															/*返回0表示失败，返回1表示成功，返回-1表示station does not exist*/
															/*返回-2表示Unknown mac addr format*/
															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == mac_addr)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac1[MAC_LEN];
	//unsigned int mac[MAC_LEN];
	unsigned int ret;
	int retu = 0;

	ret = wid_parse_mac_addr((char *)mac_addr,&mac1);
	if (CMD_FAILURE == ret) {
			return -2;
	}

	/*memset(mac,0,6);
	sscanf(mac_addr,"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];*/	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_KICKSTA);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_KICKSTA);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac1[0],
							 DBUS_TYPE_BYTE,&mac1[1],
							 DBUS_TYPE_BYTE,&mac1[2],
							 DBUS_TYPE_BYTE,&mac1[3],
							 DBUS_TYPE_BYTE,&mac1[4],
						   	 DBUS_TYPE_BYTE,&mac1[5],
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
	
	if(ret == ASD_STA_NOT_EXIST)
		retu = -1;
	else 
		retu = 1;
	
	dbus_message_unref(reply);
	return retu;
}

void Free_sta_summary(struct dcli_ac_info *ac)
{
	void (*dcli_init_free_func)(struct dcli_ac_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_ac"); 
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(ac);
		}
	}
}

/*返回1时，调用Free_sta_summary()释放空间*/
/*sort_type为可选参数，参数范围是"ascend"或"descend"，默认为"descend"*/
int show_sta_summary(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac,char *sort_type)
																										 /*返回0表示失败，返回1表示成功，返回-1表示error*/
																										 /*返回-2表示input patameter should only be 'ascend' or 'descend'*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{ 

    if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	int retu = 0;
	unsigned char type = 0;

	if(sort_type)
	{
		if (!strcmp(sort_type,"ascend")){
			type=1;		
		}else if (!strcmp(sort_type,"descend")){
			type=0;		
		}else{
			return -2;
		}
	}

	void *(*dcli_init_func1)
		(DBusConnection *,
		int , 
		int ,
		unsigned int *
		);

    *ac = NULL; 
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func1 = dlsym(ccgi_dl_handle,"show_sta_summary");
		if(NULL != dcli_init_func1)
		{
			*ac = (*dcli_init_func1)(
				connection,
				parameter.instance_id,
				parameter.local_id,
				&ret
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
	
	if((ret == 0) && (*ac != NULL)) 
	{
		if(type == 1)
		{
			struct dcli_wtp_info *(*dcli_init_func2)(struct dcli_wtp_info *, unsigned int);
			if(NULL != ccgi_dl_handle)
			{
				dcli_init_func2 = dlsym(ccgi_dl_handle,"sort_sta_ascending");
				if(NULL != dcli_init_func2)
				{
					(*ac)->wtp_list = (*dcli_init_func2)((*ac)->wtp_list,(*ac)->num_wtp);
				}
			}
		}
		else
		{
			struct dcli_wtp_info *(*dcli_init_func2)(struct dcli_wtp_info *, unsigned int);
			if(NULL != ccgi_dl_handle)
			{
				dcli_init_func2 = dlsym(ccgi_dl_handle,"sort_sta_descending");
				if(NULL != dcli_init_func2)
				{
					(*ac)->wtp_list = (*dcli_init_func2)((*ac)->wtp_list,(*ac)->num_wtp);
				}
			}
		}
		retu = 1;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -1;
	}

	return retu;
}

/*返回1时，调用Free_sta_summary()释放空间*/
int show_station_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	int retu;

	void *(*dcli_init_func)
		(DBusConnection *,
		int , 
		int ,
		unsigned int *
		);
		
    *ac = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_list");
		if(NULL != dcli_init_func)
		{
			*ac = (*dcli_init_func)
				(
			    connection,
				parameter.instance_id,
				parameter.local_id,
				&ret
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

	if((ret == 0) && (*ac != NULL))
	{
		retu = 1;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -1;	
	}
	
	return retu;
}

/*返回1时，调用Free_sta_summary()释放空间*/
int show_station_list_by_group(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac)/*失败返回0，成功返回1，返回-1表示error*/
																												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	int retu;

	void *(*dcli_init_func)
		(DBusConnection *,
		int ,
		int ,
		unsigned int *
		);

    *ac = NULL; 
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_list");
		if(NULL != dcli_init_func)
		{
			*ac = (*dcli_init_func)(
				connection,
				parameter.instance_id,
				parameter.local_id,
				&ret
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

	if((ret == 0) && (*ac != NULL)){
		retu = 1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -1;
	}
	
	return retu;
}

void Free_sta_bywlanid(struct dcli_wlan_info *wlan)
{
	void (*dcli_init_free_func)(struct dcli_wlan_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_wlan");	
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wlan);
		}
	}
}

/*返回1时，调用Free_sta_bywlanid()释放空间*/
int show_sta_bywlanid(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wlan_info **wlan)
																				/*返回0表示失败，返回1表示成功*/
																				/*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																				/*返回-2表示wlan does not exist，返回-3表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned char wlan_id = 0;
	unsigned int ret = 0;
	int retu;
	wlan_id = id;
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -1;
	}	

    void *(*dcli_init_func)(DBusConnection *, 
                            int , 
                            unsigned char , 
                            int ,
                            unsigned int *);

    *wlan = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_bywlan");
		if(NULL != dcli_init_func)
		{
			*wlan = (*dcli_init_func)(connection, 
		                              parameter.instance_id, 
		                              wlan_id, 
		                              parameter.local_id,
		                              &ret);
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
	
	if((ret == 0) && (*wlan != NULL))
	{
		retu=1;
	}
	else if (ret == ASD_WLAN_NOT_EXIST)	
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}
	
	return retu;
}

void Free_sta_bywtpid(struct dcli_wtp_info *wtp)
{
	void (*dcli_init_free_func)(struct dcli_wtp_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_wtp");	
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wtp);
		}
	}
}

/*返回1时，调用Free_sta_bywtpid()释放空间*/
int show_sta_bywtpid(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wtp_info **wtp) 
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																			/*返回-2表示wtp does not provide service or it maybe does not exist*/
																			/*返回-3表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	unsigned int wtp_id = 0;
	int retu;
	wtp_id = id;
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}

    void *(*dcli_init_func)(DBusConnection *, 
                            int , 
                            unsigned int , 
                            int ,
                            unsigned int *);

    *wtp = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_bywtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)(connection, 
			                        parameter.instance_id, 
			                        wtp_id, 
			                        parameter.local_id,
			                        &ret);
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
	
	if((ret == 0) && (*wtp != NULL))
	{
		retu = 1;
		
	}else if (ret == ASD_WTP_NOT_EXIST)	
	{
		retu  = -2;
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}
	
	return retu ;
}

/*stat为"black"或"white"*/
int wlan_add_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac) 
																			   /*返回0表示失败，返回1表示成功*/
																			   /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																			   /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																			   /*返回-3表示Unknown mac addr format*/
																			   /*返回-4表示wlan isn't existed，返回-5表示mac add already*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == stat)||(NULL == arg_mac))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret,retu;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   //1--black list
								// 2--white list
	
	wlan_id = id;
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -1;
	}

	str2lower(&stat);	
	
	if (!strcmp(stat,"black")){
		list_type=1;		
	}
	else if (!strcmp(stat,"white")){
		list_type=2;		
	}else{
		return -2;
	}

	ret = parse_mac_addr(arg_mac,&mac);
	if (CMD_FAILURE == ret) {
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_ADD_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_ADD_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		retu = -4;
	else if(ret == ASD_MAC_ADD_ALREADY)
		retu = -5;
	else 
		retu = 1;
	
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"black"或"white"*/
int wlan_delete_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac)
																				/*返回0表示失败，返回1表示成功*/
																				/*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																				/*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																				/*返回-3表示Unknown mac addr format*/
																				/*返回-4表示wlan isn't existed，返回-5表示mac is not in the list*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == stat)||(NULL == arg_mac))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret,retu;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   //1--black list
								// 2--white list
	wlan_id = id;
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -1;
	}

	str2lower(&stat);
	
	if (!strcmp(stat,"black")){
		list_type=1;		
	}
	else if (!strcmp(stat,"white")){
		list_type=2;		
	}else{
		return -2;
	}
	
	ret = parse_mac_addr(arg_mac,&mac);
	if (CMD_FAILURE == ret) {
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		retu = -4;
	else if(ret==ASD_UNKNOWN_ID)
		retu = -5;
	else 
		retu = 1;
	
	dbus_message_unref(reply);

	return retu; 
}


/*stat为"none"/"black"/"white"*/
int wlan_use_none_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat)
																	  /*返回0表示失败，返回1表示成功*/
																	  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																	  /*返回-2表示input patameter should only be 'black/white/none' or 'b/w/n'*/
																	  /*返回-3表示wlan isn't existed，返回-4表示wids has open,can't be set other except blank list*/
																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
        return 0;
	
	if(NULL == stat)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret,retu;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   //0--none,1--black list, 2--white list
	
	wlan_id = id;
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -1;
	}

	str2lower(&stat);
	
	if (!strcmp(stat,"none")){
		list_type=0;			
	}
	else if (!strcmp(stat,"black")){
		list_type=1;
	}
	else if (!strcmp(stat,"white")){
		list_type=2;
	}else{
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		retu = -3;	
	else if(ret==ASD_WIDS_OPEN)
		retu = -4;
	else 
		retu = 1;
	
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"black"或"white"*/
int wtp_add_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac)
																			 /*返回0表示失败，返回1表示成功*/
																			 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																			 /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																			 /*返回-3表示Unknown mac addr format*/
																			 /*返回-4表示wtp is not existed，返回-5表示mac add already*/
																			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == stat)||(NULL == arg_mac))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret,retu;
	unsigned int wtp_id = 0;
	unsigned char list_type=0;   //1--black list
								// 2--white list
	
	wtp_id = id;
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}

	str2lower(&stat);
	
	if (!strcmp(stat,"black")){
		list_type=1;		
	}
	else if (!strcmp(stat,"white")){
		list_type=2;		
	}else{
		return -2;
	}
	
	ret = parse_mac_addr(arg_mac,&mac);
	if (CMD_FAILURE == ret) {
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_WTP_NOT_EXIST)
		retu = -4;	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
		retu = -1;
	else if(ret == ASD_MAC_ADD_ALREADY)
		retu = -5;
	else 
		retu = 1;
	
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"black"或"white"*/
int wtp_delete_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac)  
																				 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																				 /*返回-2返回input patameter should only be 'black/white' or 'b/w'*/
																				 /*返回-3表示Unknown mac addr format，返回-4表示wtp is not existed*/
																				 /*返回-5表示mac is not in the list*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == stat)||(NULL == arg_mac))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret,retu;
	unsigned int wtp_id = 0;
	unsigned char list_type=0;   //1--black list
								// 2--white list
	
	wtp_id = id;
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}

	str2lower(&stat);
	
	if (!strcmp(stat,"black")){
		list_type=1;		
	}
	else if (!strcmp(stat,"white")){
		list_type=2;		
	}else{
		return -2;
	}
	
	ret = parse_mac_addr(arg_mac,&mac);
	if (CMD_FAILURE == ret) {
		return -3;
	}			
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_WTP_NOT_EXIST)
		retu = -4;
	else if(ret==ASD_UNKNOWN_ID)
		retu = -5;	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
		retu = -1;
	else 
		retu = 1;
	
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"none"/"black"/"white"*/
int wtp_use_none_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat) 
																	  /*返回0表示失败，返回1表示成功*/
																	  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																	  /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																	  /*返回-3表示wtp isn't existed*/
																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == stat)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret,retu;
	unsigned int wtp_id = 0;
	unsigned char list_type=0;   //0--none,1--black list, 2--white list
	
	wtp_id = id;
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}

	str2lower(&stat);
	
	if (!strcmp(stat,"none")){
		list_type=0;			
	}
	else if (!strcmp(stat,"black")){
		list_type=1;
	}
	else if (!strcmp(stat,"white")){
		list_type=2;	
	}else{
		return -2;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
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
	
	if(ret==ASD_WTP_NOT_EXIST)
		retu = -3;	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
		retu = -1;
	else 
		retu = 1;
	
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"black"或"white"*/
int radio_bss_add_black_white(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,char *stat,char *arg_mac)
																								 /*返回0表示失败，返回1表示成功*/
																								 /*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																								 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																								 /*返回-3表示Unknown mac addr format*/
																								 /*返回-4表示bss is not exist，返回-5表示mac add already*/
																								 /*返回-6表示unknown id format，返回-7表示wlan is not exist*/
																								 /*返回-8表示radio has not apply wlan，返回-9表示radio id is not exist*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == wlanID)||(NULL == stat)||(NULL == arg_mac))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret,retu;
	unsigned int radio_id = 0;
	unsigned char list_type=0;   /*1--black list*/ /* 2--white list*/
	unsigned char wlan_id = 0;
	
	
	radio_id = rid;
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		return -1;
	}
	
	ret = parse2_char_ID((char*)wlanID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -6;
	}	
	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}
	

	if (!strcmp(stat,"black")){
		list_type=1;		
	}
	else if (!strcmp(stat,"white")){
		list_type=2;		
	}
	
	ret = parse_mac_addr(arg_mac,&mac);
	if (CMD_FAILURE == ret) {
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BSS_ADD_MAC_LIST);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_BYTE,&list_type,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
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

	if(ret==BSS_NOT_EXIST)
	{
		return -8;
	}
	else if(ret == BSS_NOT_ENABLE)
	{
		//vty_out(vty,"bss add mac list successfully(in wid)!\n");
		return 1;
	}	
	else if(ret == WID_MAC_ADD_ALREADY)
	{
		return -5;
	}	
	else if(ret == RADIO_ID_NOT_EXIST)
	{
		return -9;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		return -7;
	}

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_BSS_NOT_EXIST)
		retu = -4;	
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 
		retu = -1;
	else if(ret == ASD_MAC_ADD_ALREADY)
		retu = -5;
	else 
		retu = 1;
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"black"或"white"*/
int radio_bss_delete_black_white(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,char *stat,char *arg_mac) 
																									/*返回0表示失败，返回1表示成功*/
																									/*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																									/*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																									/*返回-3表示Unknown mac addr format*/
																									/*返回-4表示bss is not exist，返回-5表示mac is not in the list*/
																									/*返回-6表示unknown id format，返回-7表示wlan is not exist*/
																									/*返回-8表示radio has not apply wlan，返回-9表示radio id is not exist*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == wlanID)||(NULL == stat)||(NULL == arg_mac))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret,retu;
	unsigned int radio_id = 0;
	unsigned char list_type=0;   /*1--black list*/	/* 2--white list*/
	unsigned char wlan_id = 0;
	
	
	radio_id = rid;
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		return -1;
	}
	
	ret = parse2_char_ID((char*)wlanID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -6;
	}	
	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){	
		return -2;
	}
	
	
	if (!strcmp(stat,"black")){
		list_type=1;		
	}
	else if (!strcmp(stat,"white")){
		list_type=2;		
	}
	
	ret = parse_mac_addr(arg_mac,&mac);
	if (CMD_FAILURE == ret) {
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BSS_DEL_MAC_LIST);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_BYTE,&list_type,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
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

	if(ret==BSS_NOT_EXIST)
	{
		return -8;
	}
	else if(ret== WID_UNKNOWN_ID)
	{
		return -5;
	}	
	else if(ret == BSS_NOT_ENABLE)
	{
		//vty_out(vty,"bss add mac list successfully(int wid)!\n");
		return 1;
	}	
	else if(ret == RADIO_ID_NOT_EXIST)
	{
		return -9;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		return -7;
	}
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_BSS_NOT_EXIST)
		retu = -4;
	else if(ret==ASD_UNKNOWN_ID)
		retu = -5;	
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
		retu = -1;
	else 
		retu = 1;
	dbus_message_unref(reply);

	return retu; 
}

/*stat为"none"/"black"/"white"*/
int radio_bss_use_none_black_white(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,char *stat) 
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																						  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																						  /*返回-3表示bss is not exist*/
																						  /*返回-4表示unknown id format，返回-5表示wlan is not exist*/
																						  /*返回-6表示radio has not apply wlan，返回-7表示mac add already*/
																						  /*返回-8表示radio id is not exist*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == wlanID)||(NULL == stat))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret,retu;
	unsigned int radio_id = 0;
	unsigned char list_type=0;   //0--none,1--black list, 2--white list	
	unsigned char wlan_id = 0;
	
	radio_id = rid;
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		return -1;
	}
	
	ret = parse2_char_ID((char*)wlanID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -4;
	}	

	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}
	
	
	if (!strcmp(stat,"none")){
		list_type=0;			
	}
	else if (!strcmp(stat,"black")){
		list_type=1;
	}
	else if (!strcmp(stat,"white")){
		list_type=2;			
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BSS_USE_MAC_LIST);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_BYTE,&list_type,
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

	if(ret==BSS_NOT_EXIST)
	{
		return -6;
	}
	else if(ret == BSS_NOT_ENABLE)
	{
		//vty_out(vty,"bss use mac list successfully(in wid)!\n");
		return 1;
	}	
	else if(ret == WID_MAC_ADD_ALREADY)
	{
		return -7;
	}	
	else if(ret == RADIO_ID_NOT_EXIST)
	{
		return -8;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		return -5;
	}

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
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
	
	if(ret==ASD_BSS_NOT_EXIST)
		retu = -3;
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
		retu = -1;
	else 
		retu = 1;
	dbus_message_unref(reply);

	return retu; 
}

/*返回1时，调用Free_sta_bywlanid()释放函数*/
int show_wlan_mac_list(dbus_parameter parameter, DBusConnection *connection,char* id,struct dcli_wlan_info **wlan)
																				  /*返回0表示失败，返回1表示成功*/
																				  /*返回-1表示unknown id format*/
																				  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																				  /*返回-3表示wlan isn't existed，返回-4表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	if(NULL == id)
	{
		*wlan = NULL;
		return 0;
	}
	
	unsigned int ret;
	unsigned char wlan_id = 0;	
	int retu;
	
	ret = parse2_char_ID((char*)id, &wlan_id);
	
	if(ret != ASD_DBUS_SUCCESS){
		return -1;
	}	
	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}
		
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned char , 
	                        int ,
	                        unsigned int *);

    *wlan = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wlan_maclist");
		if(NULL != dcli_init_func)
		{
			*wlan = (*dcli_init_func)(connection, 
			                          parameter.instance_id, 
			                          wlan_id,
			                          parameter.local_id,
			                          &ret);
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
 	
	if((ret == 0) && (*wlan != NULL))
	{		
	    retu = 1;		
	}
	else if(ret==ASD_WLAN_NOT_EXIST) 
	{
		retu =-3;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu  = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -4;
	}

	return retu;	
}

/*返回1时，调用Free_sta_bywtpid()释放空间*/
int show_wtp_mac_list(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wtp_info **wtp)
																			 /*返回0表示失败，返回1表示成功*/
																			 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																			 /*返回-2表示wtp isn't existed，返回-3表示error*/
																			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	unsigned int wtp_id = 0;
	int retu;
	
	wtp_id = id;
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}
	

	void *(*dcli_init_func)(DBusConnection *,
		                    int , 
		                    unsigned int , 
		                    int ,
		                    unsigned int *);

    *wtp = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wtp_maclist");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)(connection, 
			                        parameter.instance_id, 
			                        wtp_id, 
			                        parameter.local_id,
			                        &ret);
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

	if((ret == 0) && (*wtp != NULL))
	{			
		retu = 1;
	}
	else if(ret==ASD_WTP_NOT_EXIST) 
	{
		retu = -2;
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu  = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}

	return retu ;	
}

void Free_mac_head(struct dcli_bss_info *bss)
{
	void (*dcli_init_free_func)(struct dcli_bss_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_bss");	
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(bss);
		}
	}
}

/*返回1时，调用Free_mac_head() 释放空间*/
int show_radio_bss_mac_list(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,struct dcli_bss_info **bss)
																								 /*返回0表示失败，返回1表示成功*/
																								 /*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																								 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																								 /*返回-3表示bss isn't existed，返回-4表示error*/
																								 /*返回-5表示unknown id format*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	if(NULL == wlanID)
	{
		*bss = NULL;
		return 0;
	}
	
	int retu = 0;
	unsigned int ret;
	unsigned int radio_id = 0;
	unsigned char wlan_id = 0;
	
    radio_id = rid;	
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		return -1;
	}	
	
	ret = parse2_char_ID((char*)wlanID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -5;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int , 
	                        unsigned char , 
	                        int ,
	                        unsigned int *);

    *bss = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_radio_bss_maclist");
		if(NULL != dcli_init_func)
		{
			*bss = (*dcli_init_func)(connection, 
			                        parameter.instance_id, 
			                        radio_id, 
			                        wlan_id,
			                        parameter.local_id,
			                        &ret);
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
		
	if((ret == 0) && (*bss != NULL))
	{
		retu =1;
	}
	else if((ret==ASD_BSS_NOT_EXIST)||(ret==BSS_NOT_EXIST))
	{
		retu = -3;
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
	{
		retu = -1;
	}
	else if ((ret == ASD_DBUS_ERROR)||(ret == WID_DBUS_ERROR))
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -4;
	}

	return retu;	
}

/*未使用*/
/*返回1时，调用Free_sta_summary()释放空间*/
int show_all_wlan_mac_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac)/*返回0表示失败，返回1表示成功，返回-1表示wlan isn't existed，返回-2表示error*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
            
	unsigned int ret = 0;
	int retu;	 
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        int ,
	                        unsigned int *);

    *ac = NULL; 
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_all_wlan_maclist");
		if(NULL != dcli_init_func)
		{
			*ac = (*dcli_init_func)(connection, 
			                        parameter.instance_id,
			                        parameter.local_id,
			                        &ret);
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
			
	if((ret == 0) && (*ac != NULL))
	{
		retu = 1;
	}
	else if(ret == ASD_WLAN_NOT_EXIST) 
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -2;
	}

	return retu; 
}

/*未使用*/
/*返回1时，调用Free_sta_summary()释放空间*/
int show_all_wtp_mac_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac) 
																	    /*返回0表示失败，返回1表示成功，返回-1表示wtp isn't existed*/
																		/*返回-2表示input wtp id should be 1 to WTP_NUM-1，返回-3表示error*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	int retu;
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int ,
	                        int ,
	                        unsigned int *);

    *ac = NULL; 
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_all_wtp_maclist");
		if(NULL != dcli_init_func)
		{
			*ac = (*dcli_init_func)(connection, 
			                        parameter.instance_id,
			                        parameter.local_id,
			                        &ret);
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

	if((ret == 0) && (*ac != NULL))
	{
		retu = 1;
	}
	else if(ret == ASD_WTP_NOT_EXIST) 
	{
		retu = -1;
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{		
		retu = -3;
	}

	return retu; 
}

/*未使用*/
/*返回1时，调用Free_sta_summary()释放空间*/
int show_all_bss_mac_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	int retu;
	
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        int ,
	                        unsigned int *);

    *ac = NULL; 
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_all_bss_maclist");
		if(NULL != dcli_init_func)
		{
			*ac = (*dcli_init_func)(connection, 
			                        parameter.instance_id,
			                        parameter.local_id,
			                        &ret);
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

	if((ret == 0) && (*ac != NULL))
	{
		retu = 1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;	
	}
	else
	{
		retu = -1;
	}

	return retu; 
}

struct extend_sta_info *get_extend_sta_by_mac( struct extend_sta_profile *psta_profile, char mac[6]  )
{
	struct extend_sta_info *p_ret;

	if( NULL == psta_profile )
		return;

	for( p_ret=psta_profile->head.next;p_ret;p_ret=p_ret->next )
	{
		if( memcmp(p_ret->mac,mac,6) == 0 )
			break;
	}
	return p_ret;
}

int extend_show_sta_mac(dbus_parameter parameter, DBusConnection *connection,char *MAC,struct extend_sta_info *sta_info)  /*返回0表示失败，返回1表示成功，返回-1表示station does not exist*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	if(NULL == MAC)
	{
		sta_info = NULL;
		return 0;
	}
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN],ret,snr=0;
	unsigned long long rr=0,tr=0,tp=0,rx_bytes=0,tx_bytes=0;
	unsigned long long int rx_pkts=0, tx_pkts=0,rtx=0,rtx_pkts=0,err_pkts=0;	
	unsigned char ip[16];
	unsigned char *in_addr = ip;
	int retu=1;
	time_t StaTime,sta_online_time,sta_access_time,online_time;

	
	memset(mac,0,MAC_LEN);
	sscanf(MAC,"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac1[0],
							 DBUS_TYPE_BYTE,&mac1[1],
							 DBUS_TYPE_BYTE,&mac1[2],
							 DBUS_TYPE_BYTE,&mac1[3],
							 DBUS_TYPE_BYTE,&mac1[4],
						   	 DBUS_TYPE_BYTE,&mac1[5],
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
	if(ret == 0){
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(in_addr));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(snr));	
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(rr));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tr));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tp));	
				
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_bytes));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_bytes));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_pkts));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_pkts));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rtx));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rtx_pkts));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(err_pkts));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(StaTime));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_online_time));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_access_time));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(online_time));
	}		
	dbus_message_unref(reply);
	if(ret == 0){	
		sta_info->mac[0]=mac[0];
		sta_info->mac[1]=mac[1];
		sta_info->mac[2]=mac[2];
		sta_info->mac[3]=mac[3];
		sta_info->mac[4]=mac[4];
		sta_info->mac[5]=mac[5];
		memset(sta_info->in_addr,0,20);
		strcpy((char *)sta_info->in_addr,(char *)in_addr);
		sta_info->snr=snr;
		sta_info->rr=rr;
		sta_info->tr=tr;
		sta_info->tp=tp;
		sta_info->rx_bytes = rx_bytes;
		sta_info->tx_pkts=tx_pkts;
		sta_info->rx_pkts=rx_pkts;
		sta_info->tx_bytes = tx_bytes;
		sta_info->rtx_pkts = rtx_pkts;
		sta_info->err_pkts = err_pkts;

		if( rtx < OKB ){ 
			sta_info->rtx = (double)rtx;				
		} else if((rtx >= OKB) && (rtx < OMB)){ 
			sta_info->rtx = (double)rtx/OKB; 
		} else if((rtx >= OMB) && (rtx < OGB)){ 
			sta_info->rtx = (double)rtx/OMB; 
		} else{ \
			sta_info->rtx= (double)rtx/OGB; 
		} 
		
		if( rr < OKB )
			sta_info->flux_rr=rr;
		else if((rr >= OKB) && (rr < OMB))
			sta_info->flux_rr=(double)rr/OKB;
		else if((rr >= OMB) && (rr < OGB))
			sta_info->flux_rr=(double)rr/OMB;
		else
			sta_info->flux_rr=(double)rr/OGB;

			
		if( tr < OKB )
			sta_info->flux_tr=tr;
		else if((tr >= OKB) && (tr < OMB))
			sta_info->flux_tr=(double)tr/OKB;
		else if((tr >= OMB) && (tr < OGB))
			sta_info->flux_tr=(double)tr/OMB;
		else
			sta_info->flux_tr=(double)tr/OGB;
			
			
		if( tp < OKB )
			sta_info->flux_tp=tp;
		else if((tp >= OKB) && (tp < OMB))
			sta_info->flux_tp=(double)tp/OKB;
		else if((tp >= OMB) && (tp < OGB))
			sta_info->flux_tp=(double)tp/OMB;
		else
			sta_info->flux_tp=(double)tp/OGB;		
		
		time_t now,online_time,now_sysrun,statime;
		get_sysruntime(&now_sysrun);
		time(&now);
		
		online_time=now_sysrun-StaTime+sta_online_time;
		statime = now - online_time;
		sta_info->online_t = online_time;
		
		memset(sta_info->StaccTime,0,60);
		strcpy(sta_info->StaccTime,ctime(&statime));
		
	}
	else if (ret == ASD_STA_NOT_EXIST)
		retu=-1;

	return retu;
}

/*只要调用函数，就调用Free_sta_bywtpid()释放空间*/
int extend_show_sta_bywtpid(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wtp_info **wtp) 
																					 /*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					 /*返回-2表示WTP does not provide service*/
																					 /*返回-3表示wtp does not provide service or it maybe does not exist*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	unsigned int wtp_id = 0;
	int retu;

	wtp_id = id;
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}

	struct dcli_wtp_info*(*dcli_init_func)(
        									DBusConnection *, 
        									int , 
        									unsigned int , 
        									int ,
        									unsigned int *);

	*wtp = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"extend_show_sta_bywtp");
		if(NULL != dcli_init_func)
		{
			*wtp =(*dcli_init_func)
				  (
					  connection, 
					  parameter.instance_id, 
					  wtp_id, 
					  parameter.local_id,
					  &ret
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
	
	if((ret == 0)&&((*wtp) != NULL))
	{
		retu = 1;
		
		if((*wtp)->num_bss == 0)
			retu = -2;
	}
	else if (ret == ASD_WTP_NOT_EXIST)	
	{
		retu = -3;
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else if(ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}

	return retu;
}

/*只要调用函数，就调用Free_sta_bywtpid(struct dcli_wtp_info *wtp)释放空间*/
int show_bss_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *WTPID,struct dcli_wtp_info **wtp)
																				  /*返回0表示失败，返回1表示成功*/
																				  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																				  /*返回-2表示wtp does not provide service or it maybe does not exist*/
																				  /*返回-3表示error，返回-4表示unknown id format*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	if(NULL == WTPID)
	{
		*wtp = NULL;
		return 0;
	}
	
	unsigned int wtp_id = 0;
	unsigned int ret = 0;
	int retu ;

	*wtp  = NULL;	
	int(*parse2_int_ID_func)(char* ,unsigned int* );
	if(NULL != ccgi_dl_handle)
	{
		parse2_int_ID_func = dlsym(ccgi_dl_handle,"parse2_int_ID");
		if(NULL != parse2_int_ID_func)
		{
			ret  = (*parse2_int_ID_func)((char*)WTPID, &wtp_id);
			if(ret != ASD_DBUS_SUCCESS){
				return -4;
			}	
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
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}

	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int , 
	                        int , 
	                        unsigned int *);

	*wtp  = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_info_bywtp");
		if(NULL != dcli_init_func)
		{
			*wtp  = (*dcli_init_func)(connection, 
			                            parameter.instance_id, 
			                            wtp_id,
			                            parameter.local_id,
			                            &ret);
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
	
	if((ret == 0) && (*wtp != NULL))
	{
	   retu = 1;
	}
	else if (ret == ASD_WTP_NOT_EXIST)
	{
		retu = -2;
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}
	
	return retu;
}

/*只要调用函数，就调用Free_sta_bywtpid(struct dcli_wtp_info *wtp)释放空间*/
int show_radio_info_cmd(dbus_parameter parameter, DBusConnection *connection,int WTPID,struct dcli_wtp_info **wtp)
																				  /*返回0表示失败，返回1表示成功*/
																				  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																				  /*返回-2表示wtp does not provide service or it maybe does not exist*/
																				  /*返回-3表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;

	unsigned int wtp_id = 0;
	unsigned int ret = 0;
	int retu;

	wtp_id = WTPID;
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -1;
	}


	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int , 
	                        int ,
	                        unsigned int *);

	*wtp = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_radio_info_bywtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)(connection, 
			                         parameter.instance_id, 
			                         wtp_id, 
			                         parameter.local_id,
			                         &ret);
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
	
	if((ret==0)&&(*wtp!=NULL))
	{
       retu = 1;
	}
	else if (ret == ASD_WTP_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}
	
	return retu;
}

void Free_channel_access_time_head( struct dcli_channel_info *channel)
{
	void (*dcli_init_free_func)(struct dcli_channel_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_channel");	
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(channel);
		}
	}
}

/*只要调用函数，就调用Free_channel_access_time_head()释放空间*/
int show_channel_access_time_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_channel_info **channel)
																				/*返回0表示失败，返回1表示成功，返回-1表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
        return 0;
        
	int ret;
	
	unsigned char num ;
	int retu;

	
	void *(*dcli_init_func)(DBusConnection *, 
	                         int ,
	                         unsigned char *,
	                         int ,
	                         unsigned int *);

	*channel = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_channel_access_time");
		if(NULL != dcli_init_func)
		{
			*channel = (*dcli_init_func)(connection, 
			                                parameter.instance_id, 
			                                &num,
			                                parameter.local_id,
			                                &ret);
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
    
	if((ret == 0 )&&(*channel))
	{
	   retu = 1;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -1;
	}	

	return retu;	
}

void Free_mib_info_head( struct dcli_radio_info *radio)
{
	void (*dcli_init_free_func)(struct dcli_radio_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_radio");	
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(radio);
		}
	}
}

/*只要调用函数，就调用Free_mib_info_head()释放空间*/
int show_mib_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,int Rid,struct dcli_radio_info **radio)
																						 /*返回0表示失败，返回1表示成功，返回-1表示radio id invalid*/
																						 /*返回-2表示radio does not provide service or it maybe does not exist，返回-3表示error*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	unsigned int radio_id = 0;
	unsigned int ret = 0;
	int retu;
	
	radio_id = Rid;
	if(radio_id >= G_RADIO_NUM || radio_id < 1*L_RADIO_NUM){
		return -1;
	}



	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int ,
	                        int ,
	                        unsigned int *);

	*radio = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_mib_info_byradio");
		if(NULL != dcli_init_func)
		{
			*radio = (*dcli_init_func)(connection, 
			                            parameter.instance_id, 
			                            radio_id, 
			                            parameter.local_id,
			                            &ret);
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
	
	if((ret==0)&&(*radio!=NULL))
	{
		retu  = 1;
	}
	else if (ret ==ASD_WTP_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}	
	else
	{
		retu  = -3;
	}
	
	return retu;
}

/*返回1时，调用Free_sta_bywlanid()释放空间*/
int show_wlan_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WLANID,struct dcli_wlan_info **wlan)
																						   /*返回0表示失败，返回1表示成功，返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																						   /*返回-2表示wlan does not provide service or it maybe does not exist，返回-3表示error*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
        
	unsigned char wlan_id = 0;
	unsigned int ret = 0;
	int retu;

    wlan_id = WLANID;
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -1;
	}
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned char ,
	                        int ,
	                        unsigned int *);

    *wlan = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_info_bywlan");
		if(NULL != dcli_init_func)
		{
			*wlan= (*dcli_init_func)(connection, 
			                         parameter.instance_id, 
			                         wlan_id,
			                         parameter.local_id,
			                         &ret);
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
	
	if((ret == 0) && (*wlan != NULL))
	{
		retu = 1;
	}	
	else if (ret == ASD_WLAN_NOT_EXIST) 	
	{
		retu = -2;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}

	return retu;
}

/*只要调用函数，就调用Free_sta_bywtpid()释放空间*/
int show_wapi_mib_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,struct dcli_wtp_info **wtp)
																								/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示wtpid id invalid*/
																								/*返回-3表示wtp does not provide service or it maybe does not exist，返回-4表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	if(NULL == wtp_id)
	{
		*wtp = NULL;
		return 0;
	}
	
	unsigned int ret = 0;
	unsigned int wtpid=0 ;	
	int retu;
		
	int(*parse2_int_ID_func)(char* ,unsigned int* );
	if(NULL != ccgi_dl_handle)
	{
		parse2_int_ID_func = dlsym(ccgi_dl_handle,"parse2_int_ID");
		if(NULL != parse2_int_ID_func)
		{
			ret  = (*parse2_int_ID_func)((char*)wtp_id, &wtpid);
			if(ret != ASD_DBUS_SUCCESS){
				return -1;
			}	
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
	
	if(wtpid >= WTP_NUM ){
		return -2;
	}
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int , 
	                        int ,
	                        unsigned int *);

	*wtp = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wapi_mib_info_bywtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)(connection, 
			                         parameter.instance_id, 
			                         wtpid, 
			                         parameter.local_id,
			                         &ret);
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

	if((ret == 0)&&(*wtp != NULL))
	{
		retu = 1;
	}
	else if (ret ==ASD_WTP_NOT_EXIST) 
	{
		retu = -3;
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -4;
	}
	
	return retu;
}


int show_wapi_protocol_info_cmd_func(struct wapi_protocol_info_profile *wapi_protocol_info)
{
	if(NULL == wapi_protocol_info)
		return 0;
	
	unsigned int  ConfigVersion = 1;
	unsigned char WapiSupported = 1;
	unsigned char WapiPreAuth = 0;
	unsigned char WapiPreauthEnabled = 0;
	unsigned char UnicastKeysSupported = 2;
	unsigned char UnicastCipherEnabled = 1;
	unsigned char AuthenticationSuiteEnabled = 1;
	unsigned char MulticastRekeyStrict = 0;
	
	unsigned int  BKLifetime = 43200;
	unsigned int  BKReauthThreshold = 70;
	unsigned int  SATimeout = 60;
	unsigned char UnicastCipherSelected[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned char MulticastCipherSelected[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned char UnicastCipherRequested[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned char MulticastCipherRequested[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned int  MulticastCipherSize = 256;
	unsigned char MulticastCipher[4] = {0x00, 0x14, 0x72, 0x01};
	unsigned int  UnicastCipherSize = 512;
	unsigned char UnicastCipher[4] = {0x00, 0x14, 0x72, 0x01};

	/*vty_out(vty,"==============================================================================\n");
	vty_out(vty,"Supported Latest Version: %u\n",ConfigVersion);
	vty_out(vty,"WapiSupported: %s\n",((WapiSupported==1)?"Yes":"No"));
	vty_out(vty,"WapiPreAuth: %s\n",((WapiPreAuth==1)?"Yes":"No"));
	vty_out(vty,"WapiPreauthEnabled: %s\n",((WapiPreauthEnabled==1)?"Yes":"No"));
	vty_out(vty,"UnicastCipherEnabled: %s\n",((UnicastCipherEnabled==1)?"Yes":"No"));
	vty_out(vty,"AuthenticationSuiteEnabled: %s\n",((AuthenticationSuiteEnabled==1)?"Yes":"No"));
	vty_out(vty,"MulticastRekeyStrict: %s\n",((MulticastRekeyStrict==1)?"Yes":"No"));
	vty_out(vty,"UnicastKeysSupported: %u\n",UnicastKeysSupported);

	vty_out(vty,"BKLifetime: %u(seconds)\n",BKLifetime);
	vty_out(vty,"BKReauthThreshold: %u%%\n",BKReauthThreshold);
	vty_out(vty,"SATimeout: %u(seconds)\n",SATimeout);
	
	vty_out(vty,"UnicastCipherSelected: %02X-%02X-%02X-%02X\n",UnicastCipherSelected[0],UnicastCipherSelected[1],UnicastCipherSelected[2],UnicastCipherSelected[3]);
	vty_out(vty,"MulticastCipherSelected: %02X-%02X-%02X-%02X\n",MulticastCipherSelected[0],MulticastCipherSelected[1],MulticastCipherSelected[2],MulticastCipherSelected[3]);
	vty_out(vty,"UnicastCipherRequested: %02X-%02X-%02X-%02X\n",UnicastCipherRequested[0],UnicastCipherRequested[1],UnicastCipherRequested[2],UnicastCipherRequested[3]);
	vty_out(vty,"MulticastCipherRequested: %02X-%02X-%02X-%02X\n",MulticastCipherRequested[0],MulticastCipherRequested[1],MulticastCipherRequested[2],MulticastCipherRequested[3]);

	vty_out(vty,"MulticastCipherSize: %u(bits)\n",MulticastCipherSize);
	vty_out(vty,"MulticastCipher: %02X-%02X-%02X-%02X\n",MulticastCipher[0],MulticastCipher[1],MulticastCipher[2],MulticastCipher[3]);
	vty_out(vty,"UnicastCipherSize: %u(bits)\n",UnicastCipherSize);
	vty_out(vty,"UnicastCipher: %02X-%02X-%02X-%02X\n",UnicastCipher[0],UnicastCipher[1],UnicastCipher[2],UnicastCipher[3]);
	vty_out(vty,"==============================================================================\n");*/

	wapi_protocol_info->ConfigVersion=ConfigVersion;
	wapi_protocol_info->WapiSupported=WapiSupported;
	wapi_protocol_info->WapiPreAuth=WapiPreAuth;
	wapi_protocol_info->WapiPreauthEnabled=WapiPreauthEnabled;
	wapi_protocol_info->UnicastCipherEnabled=UnicastCipherEnabled;
	wapi_protocol_info->AuthenticationSuiteEnabled=AuthenticationSuiteEnabled;
	wapi_protocol_info->MulticastRekeyStrict=MulticastRekeyStrict;
	wapi_protocol_info->UnicastKeysSupported=UnicastKeysSupported;
	wapi_protocol_info->BKLifetime=BKLifetime;
	wapi_protocol_info->BKReauthThreshold=BKReauthThreshold;
	wapi_protocol_info->SATimeout=SATimeout;
	wapi_protocol_info->UnicastCipherSelected[0]=UnicastCipherSelected[0];
	wapi_protocol_info->UnicastCipherSelected[1]=UnicastCipherSelected[1];
	wapi_protocol_info->UnicastCipherSelected[2]=UnicastCipherSelected[2];
	wapi_protocol_info->UnicastCipherSelected[3]=UnicastCipherSelected[3];
	wapi_protocol_info->MulticastCipherSelected[0]=MulticastCipherSelected[0];
	wapi_protocol_info->MulticastCipherSelected[1]=MulticastCipherSelected[1];
	wapi_protocol_info->MulticastCipherSelected[2]=MulticastCipherSelected[2];
	wapi_protocol_info->MulticastCipherSelected[3]=MulticastCipherSelected[3];
	wapi_protocol_info->UnicastCipherRequested[0]=UnicastCipherRequested[0];
	wapi_protocol_info->UnicastCipherRequested[1]=UnicastCipherRequested[1];
	wapi_protocol_info->UnicastCipherRequested[2]=UnicastCipherRequested[2];
	wapi_protocol_info->UnicastCipherRequested[3]=UnicastCipherRequested[3];
	wapi_protocol_info->MulticastCipherRequested[0]=MulticastCipherRequested[0];
	wapi_protocol_info->MulticastCipherRequested[1]=MulticastCipherRequested[1];
	wapi_protocol_info->MulticastCipherRequested[2]=MulticastCipherRequested[2];
	wapi_protocol_info->MulticastCipherRequested[3]=MulticastCipherRequested[3];
	wapi_protocol_info->MulticastCipherSize=MulticastCipherSize;
	wapi_protocol_info->MulticastCipher[0]=MulticastCipher[0];
	wapi_protocol_info->MulticastCipher[1]=MulticastCipher[1];
	wapi_protocol_info->MulticastCipher[2]=MulticastCipher[2];
	wapi_protocol_info->MulticastCipher[3]=MulticastCipher[3];
	wapi_protocol_info->UnicastCipherSize=UnicastCipherSize;
	wapi_protocol_info->UnicastCipher[0]=UnicastCipher[0];
	wapi_protocol_info->UnicastCipher[1]=UnicastCipher[1];
	wapi_protocol_info->UnicastCipher[2]=UnicastCipher[2];
	wapi_protocol_info->UnicastCipher[3]=UnicastCipher[3];
	
	return 1;
}

/*未使用*/
/*只要调用函数，就调用Free_sta_bywtpid()释放空间*/
int show_wapi_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID,struct dcli_wtp_info **wtp) 
																						   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																						   /*返回-2表示wtp id should be 1 to WTP_NUM-1，返回-3表示WTP does not provide service*/
																						   /*返回-4表示wtp does not provide service or it maybe does not exist，返回-5表示error*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if(NULL == WtpID)
	{
		*wtp = NULL;
		return 0;
	}
	
	unsigned int wtp_id = 0;
	unsigned int ret = 0;
	int retu;	

	int(*parse2_int_ID_func)(char* ,unsigned int* );
	if(NULL != ccgi_dl_handle)
	{
		parse2_int_ID_func = dlsym(ccgi_dl_handle,"parse2_int_ID");
		if(NULL != parse2_int_ID_func)
		{
			ret  = (*parse2_int_ID_func)((char*)WtpID, &wtp_id);
			if(ret != ASD_DBUS_SUCCESS){
				return -1;
			}	
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
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		return -2;
	}
	
	void *(*dcli_init_func)(DBusConnection *, 
                    		int , 
                    		unsigned int , 
                    		int ,
                    		unsigned int *);

	*wtp = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wapi_info_bywtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)(connection, 
			                        parameter.instance_id, 
			                        wtp_id, 
			                        parameter.local_id,
			                        &ret);
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

	if((ret==0)&&((*wtp)!=NULL))
	{
		retu =1;
		if((*wtp)->num_bss == 0)
		{
			retu = -3;
		}
	}
	else if (ret == ASD_WTP_NOT_EXIST)
	{
		retu = -4;
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu  = -5;
	}		

	return retu;
}

/*未使用*/
/*state的范围是"enable"或"disable"*/
int set_ac_flow_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WlanID,char *state)
																	   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																	   /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																	   /*返回-3表示input patameter only with 'enable' or 'disable'，返回-4表示wlan isn't existed*/
																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == WlanID)||(NULL == state))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/
	int retu;
	unsigned char wlan_id = 0;
	
	ret = parse2_char_ID((char*)WlanID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -1;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}	
	
	str2lower(&state);
	
	if (!strcmp(state,"enable")){
		type=1;		
	}else if (!strcmp(state,"disable")){
		type=2;		
	}else{
		return -3;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_AC_FLOW);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_AC_FLOW);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&type,
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		retu=-4;
	else 
		retu=1;

	dbus_message_unref(reply);

	return retu; 
}

/*未使用*/
/*返回1时，调用Free_sta_summary(释放空间*/
int show_traffic_limit_info_rd_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *RID,struct dcli_ac_info **ac)
																								/*返回0表示失败，返回1表示成功*/
																								/*返回-1表示unknown id format，返回-2表示radio id invalid*/
																								/*返回-3表示radio does not provide service or it maybe does not exist*/
																								/*返回-4表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
            
	if(NULL == RID)
	{
		*ac = NULL;
		return 0;
	}
	
	unsigned int radioid = 0;
	unsigned int ret = 0;
	int retu;
	
	int(*parse2_int_ID_func)(char* ,unsigned int* );
	if(NULL != ccgi_dl_handle)
	{
		parse2_int_ID_func = dlsym(ccgi_dl_handle,"parse2_int_ID");
		if(NULL != parse2_int_ID_func)
		{
			ret  = (*parse2_int_ID_func)((char*)RID, &radioid);
			if(ret != ASD_DBUS_SUCCESS){
				return -1;
			}	
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
	
	if(radioid >= G_RADIO_NUM || radioid < 1*L_RADIO_NUM){
		return -2;
	}


	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int , 
                            int ,
	                        unsigned int *);

    *ac = NULL; 
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_traffic_limit_byradio");
		if(NULL != dcli_init_func)
		{
			*ac = (*dcli_init_func)(connection, 
			                        parameter.instance_id, 
			                        radioid, 
			                        parameter.local_id,
			                        &ret);
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

	if((ret == 0) && (*ac != NULL)) 
	{
		retu = 1;
	}
	else if (ret ==ASD_WTP_NOT_EXIST) 
	{
		retu = -3;
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -4;
	}

	return retu;
}

/*未使用*/
/*返回1时，调用Free_mac_head()释放空间*/
int show_traffic_limit_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *BssIndex,struct dcli_bss_info **bss)
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown bssindex format*/
																									/*返回-2表示bssindex invalid，返回-3表示bssindex BssIndex does not provide service or it maybe does not exist*/
																								    /*返回-4表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
            
	if(NULL == BssIndex)
	{
		*bss = NULL;
		return 0;
	}
	
	unsigned int bssindex = 0;
	unsigned int ret = 0;
	int retu;

	int(*parse2_int_ID_func)(char* ,unsigned int* );
	if(NULL != ccgi_dl_handle)
	{
		parse2_int_ID_func = dlsym(ccgi_dl_handle,"parse2_int_ID");
		if(NULL != parse2_int_ID_func)
		{
			ret  = (*parse2_int_ID_func)((char*)BssIndex, &bssindex);
			if(ret != ASD_DBUS_SUCCESS){
				return -1;
			}	
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
	
	if(bssindex >= (G_RADIO_NUM+1)*L_BSS_NUM|| bssindex <1* L_RADIO_NUM*L_BSS_NUM){
		return -2;
	}
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned int ,
	                        int ,
	                        unsigned int *);

    *bss = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_traffic_limit_bybss");
		if(NULL != dcli_init_func)
		{
			*bss = (*dcli_init_func)(connection, 
			                        parameter.instance_id, 
			                        bssindex,
			                        parameter.local_id,
			                        &ret);
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

	if((ret == 0) && (*bss != NULL))
	{
		retu = 1;
	}
	else if (ret ==ASD_WTP_NOT_EXIST) 
	{
		retu = -3;
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -4;
	}
	
	return retu ;
}

/*未使用*/
/*返回1时，调用Free_sta_bywlanid()释放空间*/
int show_wlan_wids_MAC_list_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WLAN_ID,struct dcli_wlan_info **wlan)
																										  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																										  /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wlan isn't existed*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;

	if(NULL == WLAN_ID)
	{
		*wlan = NULL;
		return 0;
	}
	
	unsigned int ret = 0;
	unsigned char wlan_id = 0;
	int retu;
	
	ret = parse2_char_ID((char*)WLAN_ID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -1;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}
	
	void *(*dcli_init_func)(DBusConnection *, 
	                        int , 
	                        unsigned char , 
	                        int ,
	                        unsigned int *);

    *wlan = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wlan_wids_maclist");
		if(NULL != dcli_init_func)
		{
			*wlan = (*dcli_init_func)(connection, 
			                          parameter.instance_id, 
			                          wlan_id,
			                          parameter.local_id,
			                          &ret);
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

	if((ret == 0) && (*wlan != NULL))
	{
		retu = 1;
	}
	else if(ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}

	return retu; 
}

/*state为"enable"或"disable"*/
int set_wlan_extern_balance_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WlanID,char *state)
																					  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																					  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																					  /*返回-3表示input patameter should only be 'enable' or 'disable'*/
																					  /*返回-4表示wlan isn't existed*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == WlanID)||(NULL == state))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/
	unsigned char wlan_id = 0;
    int retu;							
	
	ret = parse2_char_ID((char*)WlanID, &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		return -1;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		return -2;
	}	
	
	str2lower(&state);
	
	if (!strcmp(state,"enable")){
		type=1;		
	}else if (!strcmp(state,"disable")){
		type=0;		
	}else{
		return -3;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&type,
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
	
	if(ret==ASD_WLAN_NOT_EXIST)
		retu=-4;
	else 
		retu=1;

	dbus_message_unref(reply);

	return retu; 
}

void Free_sta_static_head(struct sta_static_info *tab)
{
	void (*dcli_init_free_func)(struct sta_static_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_static_sta_tab"); 
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(tab);
		}
	}
}

/*未使用*/
/*返回1时，调用Free_sta_static_head( )释放空间*/
int show_all_sta_static_info_cmd(dbus_parameter parameter, DBusConnection *connection,struct sta_static_info **tab)/*返回0表示失败，返回1表示成功，返回-1表示there is no static sta，返回-2表示station does not exist，返回-3表示error*/
																														/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned int 	num=0;
	unsigned int 	ret=0;
	int retu;
	
	void *(*dcli_init_func)(DBusConnection *,
	                        int , 
	                        unsigned int *, 
                            int ,
	                        unsigned int *);

    *tab = NULL;	
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_static_info");
		if(NULL != dcli_init_func)
		{
			*tab = (*dcli_init_func)(connection,
			                        parameter.instance_id,
			                        &num,
			                        parameter.local_id,
			                        &ret);
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

	if((ret == 0) && (*tab != NULL))
	{
		retu = 1;
	}
	else if((ret == 0) && (num == 0))
	{
		retu = -1;
	}
	else if (ret == ASD_STA_NOT_EXIST)
	{
		retu = -2;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}

	return retu ;
}


/*未使用*/
/*Type为(vlanid | limit | send_limit)*/
/*当Type为vlanid时，Value的范围是0-4095*/
/*wlanID为可选参数，不配置wlanID时，请输入NULL*/
int set_sta_static_info_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC,char *Type,char *Value,char *wlanID)
																							 /*返回0表示失败，返回1表示成功*/
																						     /*返回-1表示input parameter should only be 'vlanid' or 'limit' or 'send_limit'*/
																						     /*返回-2表示vlan id should be 0 to 4095，返回-3表示wtp isn't existed*/
																						     /*返回-4表示radio isn't existed*/
																							 /*返回-5表示station traffic limit send value is more than bss traffic limit send value*/
																							 /*返回-6表示radio doesn't bing wlan，返回-7表示wlan isn't existed*/
																							 /*返回-8表示check sta set invalid value，返回-9表示error*/
																							 /*返回-10表示wlanid is not legal，返回-11表示wlanid should be 1 to WLAN_NUM-1*/
																							 /*返回-12表示wtp doesn't bing wlan，返回-13表示input wlanid is not sta accessed wlan*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if((NULL == MAC)||(NULL == Type)||(NULL == Value))
		return 0;
	
	struct dcli_sta_info *sta = NULL;
	unsigned int 	value = 0;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned char   type = 0;
	unsigned int	ret=0;
	unsigned char 	wlan_id=0;
	int retu;
	
	sscanf(MAC,"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	

	if (!strcmp(Type,"vlanid")) {
		type = 0;	
	}else if (!strcmp(Type,"limit")) {
		type = 1;	
	}else if(!strcmp(Type,"send_limit")) {
		type = 2;
	}else {
		return -1;
	}

	value = atoi(Value);
	int(*parse2_int_ID_func)(char* ,unsigned int* );
	if(NULL != ccgi_dl_handle)
	{
		parse2_int_ID_func = dlsym(ccgi_dl_handle,"parse2_int_ID");
		if(NULL != parse2_int_ID_func)
		{
			ret  = (*parse2_int_ID_func)((char*)Value, &value);
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
		
	if((type == 0) && (value > 4095 )){
		return -2;
	}
	
	if(wlanID)
	{
		ret = parse2_char_ID((char *)wlanID,&wlan_id);
		if(ret != WID_DBUS_SUCCESS){
			return -10;
		}
		if(wlan_id < 0 || wlan_id > 128){
			return -11;
		}
	}

	struct dcli_sta_info*(*dcli_init_func)(
								DBusConnection *,
								int , 
								unsigned char *, 
								unsigned char , 
								unsigned int , 
								int ,
								unsigned int *
							);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"check_sta_by_mac");
		if(NULL != dcli_init_func)
		{
			sta =(*dcli_init_func)
				  (
					 connection,
					 parameter.instance_id,
					 mac1,
					 type,
					 value,
					 parameter.local_id,
					 &ret
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

	if((ret == 0) && (sta != NULL)){
		if((wlanID)&&(wlan_id == sta->wlan_id)){
			void (*dcli_init_func)(
						DBusConnection *,
						int , 
						unsigned char *, 
						unsigned char , 
						unsigned int , 
						unsigned char , 
						unsigned int , 
						int ,
						unsigned int *
					);
			
			if(NULL != ccgi_dl_handle)
			{
				dcli_init_func = dlsym(ccgi_dl_handle,"wid_set_sta_info");
				if(NULL != dcli_init_func)
				{
					(*dcli_init_func)
						  (
							 connection,
							 parameter.instance_id,
							 mac1,
							 sta->wlan_id,
							 sta->radio_g_id,
							 type,
							 value,
							 parameter.local_id,
							 &ret
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
			
			if(ret == 0){
				void (*dcli_init_func)(
							DBusConnection *,
							int , 
							unsigned char *, 
							unsigned char ,
							unsigned int , 
							unsigned char , 
							unsigned int , 
							int ,
							unsigned int *
						);
			
				if(NULL != ccgi_dl_handle)
				{
					dcli_init_func = dlsym(ccgi_dl_handle,"set_sta_static_info");
					if(NULL != dcli_init_func)
					{
						(*dcli_init_func)
							  (
								 connection,
								 parameter.instance_id,
								 mac1,
								 sta->wlan_id,
								 sta->radio_g_id,
								 type,
								 value,
								 parameter.local_id,
								 &ret
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
			
				if(ret == 0){
					retu = 1;
				}else if (ret == ASD_DBUS_ERROR)
					retu = SNMPD_CONNECTION_ERROR;
				else
					retu = -9;
			}
			else if(ret == WTP_ID_NOT_EXIST)
				retu = -3;
			else if(ret == RADIO_ID_NOT_EXIST)
				retu = -4;
			else if(ret == IF_POLICY_CONFLICT)
				retu = -5;
			else if(ret==RADIO_NO_BINDING_WLAN)
				retu = -6;
			else if(ret==WLAN_ID_NOT_EXIST)
				retu = -7;
			else
				retu = -9;
			
			void (*dcli_init_free_func)(struct dcli_sta_info *);
			if(NULL != ccgi_dl_handle)
			{
				dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_sta");
				if(NULL != dcli_init_free_func)
				{
					dcli_init_free_func(sta);
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
		else if((NULL == wlanID)||((wlanID)&&(wlan_id != sta->wlan_id))){
			if(NULL == wlanID)
			{
				void (*dcli_init_func)(
							DBusConnection *,
							int , 
							unsigned char *, 
							unsigned char , 
							unsigned int , 
							unsigned char , 
							unsigned int , 
							int ,
							unsigned int *
						);
			
				if(NULL != ccgi_dl_handle)
				{
					dcli_init_func = dlsym(ccgi_dl_handle,"wid_set_sta_info");
					if(NULL != dcli_init_func)
					{
						(*dcli_init_func)
							  (
								 connection,
								 parameter.instance_id,
								 mac1,
								 sta->wlan_id,
								 sta->radio_g_id,
								 type,
								 value,
								 parameter.local_id,
								 &ret
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
			}
			else
			{
				void (*dcli_init_func)(
							DBusConnection *,
							int , 
							unsigned char *, 
							unsigned char , 
							unsigned int , 
							unsigned char , 
							unsigned int , 
							int ,
							unsigned int *
						);
			
				if(NULL != ccgi_dl_handle)
				{
					dcli_init_func = dlsym(ccgi_dl_handle,"wid_set_sta_info");
					if(NULL != dcli_init_func)
					{
						(*dcli_init_func)
							  (
								 connection,
								 parameter.instance_id,
								 mac1,
								 wlan_id,
								 sta->radio_g_id,
								 type,
								 value,
								 parameter.local_id,
								 &ret
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
			}
			if(ret == 0){
				void (*dcli_init_func)(
							DBusConnection *,
							int , 
							unsigned char *, 
							unsigned char , 
							unsigned int , 
							unsigned char , 
							unsigned int ,
							int ,
							unsigned int *
						);
			
				if(NULL != ccgi_dl_handle)
				{
					dcli_init_func = dlsym(ccgi_dl_handle,"set_sta_static_info");
					if(NULL != dcli_init_func)
					{
						(*dcli_init_func)
							  (
								 connection,
								 parameter.instance_id,
								 mac1,
								 wlan_id,
								 sta->radio_g_id,
								 type,
								 value,
								 parameter.local_id,
								 &ret
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
				if(ret == 0)
					retu = 1;
				else if (ret == ASD_DBUS_ERROR)
					retu = SNMPD_CONNECTION_ERROR;
				else
					retu = -9;
			}
			else if(ret == WTP_ID_NOT_EXIST)
				retu = -3;
			else if(ret == RADIO_ID_NOT_EXIST)
				retu = -4;
			else if(ret == IF_POLICY_CONFLICT)
				retu = -5;
			else if(ret==RADIO_NO_BINDING_WLAN)
				retu = -6;
			else if(ret==WLAN_ID_NOT_EXIST)
				retu = -7;
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				retu = -12;
			else
				retu = -9;

			void (*dcli_init_free_func)(struct dcli_sta_info *);
			if(NULL != ccgi_dl_handle)
			{
				dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_sta");
				if(NULL != dcli_init_free_func)
				{
					dcli_init_free_func(sta);
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
		else
			retu = -13;
	}
	else if (ret == ASD_STA_NOT_EXIST){
		void (*dcli_init_func)(
						DBusConnection *,
						int , 
						unsigned char *, 
						unsigned char , 
						unsigned int , 
						unsigned char , 
						unsigned int ,
						int ,
						unsigned int *
					);

		if(NULL != ccgi_dl_handle)
		{
			dcli_init_func = dlsym(ccgi_dl_handle,"set_sta_static_info");
			if(NULL != dcli_init_func)
			{
				(*dcli_init_func)
					  (
						 connection,
						 parameter.instance_id,
						 mac1,
						 wlan_id,
						 0,
						 type,
						 value,
						 parameter.local_id,
						 &ret
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

		if(ret == 0){
			retu = 1;
		}else if (ret == ASD_DBUS_ERROR)
			retu = SNMPD_CONNECTION_ERROR;
		else
			retu = -9;
	}
	else if (ret == ASD_DBUS_SET_ERROR){
		retu = -8;
	}else if (ret == ASD_DBUS_ERROR)
		retu = SNMPD_CONNECTION_ERROR;
	else
		retu = -9;

	return retu;
}

/*未使用*/
/*wlanID为可选参数，不配置wlanID时，请输入NULL*/
int del_sta_static_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC,char *wlanID)
														 /*返回0表示失败，返回1表示成功*/
													     /*返回-1表示station does not exist，返回-2表示error*/
														 /*返回-3表示wlanid is not legal，返回-4表示wlanid should be 0 to WLAN_NUM-1*/
														 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	if(NULL == MAC)
		return 0;
	
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned int	ret=0;
	unsigned char 	wlan_id=0;
	char c = 0;
	int retu = 0;
	
	sscanf(MAC,"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	
	
	if(wlanID)
	{
		ret = parse2_char_ID((char *)wlanID,&wlan_id);
		c = (char *)wlanID[0];
		if(c == '0'){
			ret = WID_DBUS_SUCCESS;
			wlan_id = 0;
		}
		if(ret != WID_DBUS_SUCCESS){
			return -3;
		}
		if(wlan_id > 128){
			return -4;
		}
	}


	void (*dcli_init_func)(
				DBusConnection *,
				int , 
				unsigned char *, 
				unsigned char ,
				int ,
				unsigned int *
			);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"del_sta_static_info");
		if(NULL != dcli_init_func)
		{
			(*dcli_init_func)
				  (
					 connection,
					 parameter.instance_id,
					 mac1,
					 wlan_id,
					 parameter.local_id,
					 &ret
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
		
	if(ret == 0) {	
		retu = 1;
	}else if (ret == ASD_STA_NOT_EXIST)
		retu = -1;
	else if (ret == ASD_DBUS_ERROR)
		retu = SNMPD_CONNECTION_ERROR;
	else
		retu = -2;

	return retu;
}

/*未使用*/
/* 返回1时，调用Free_sta_static_head( )释放空间*/
/*wlanID为可选参数，不配置wlanID时，请输入NULL*/
int show_sta_mac_static_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC,struct sta_static_info **sta,char *wlanID)
																							   /*返回0表示失败，返回1表示成功*/
																							   /*返回-1表示station does not exist，返回-2表示error*/
																							   /*返回-3表示wlanid is not legal，返回-4表示wlanid should be 1 to WLAN_NUM-1*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	if(NULL == MAC)
	{
		*sta = NULL;
		return 0;
	}
	
	struct sta_static_info *tab = NULL;
	unsigned int 	mac[MAC_LEN]={0};
	unsigned char 	mac1[MAC_LEN]={0};
	unsigned char 	wlan_id=0;
	unsigned int 	ret=0;
	int retu = 0;
	
	sscanf(MAC,"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	
	
	if(wlanID)
	{
		ret = parse2_char_ID((char *)wlanID,&wlan_id);
		if(ret != WID_DBUS_SUCCESS){
			return -3;
		}
		if(wlan_id > 128){
			return -4;
		}
	}


	struct sta_static_info*(*dcli_init_func)(
									DBusConnection *,
									int , 
									unsigned char *, 
									unsigned char ,
									int ,
									unsigned int *
								);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_static_info_bymac");
		if(NULL != dcli_init_func)
		{
			tab = NULL;
			tab =(*dcli_init_func)
				  (
					connection,
					parameter.instance_id,
					mac1,
					wlan_id,
					parameter.local_id,
					&ret
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

	if((ret == 0) && (tab != NULL))	
		retu = 1;
	else if (ret == ASD_STA_NOT_EXIST)
		retu = -1;
	else if (ret == ASD_DBUS_ERROR)
		retu = SNMPD_CONNECTION_ERROR;
	else
		retu = -2;

	return retu;
}

/*未使用*/
/*type为(add|del)*/
int sta_arp_set_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *IP,char *MAC,char *if_name)
																					/*返回0表示失败，返回1表示成功*/
																				    /*返回-1表示Unkown Command*/
																				    /*返回-2表示unknown ip format*/
																					/*返回-3表示Unknow mac addr format*/
																					/*返回-4表示set sta arp failed*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if((NULL == type)||(NULL == IP)||(NULL == MAC)||(NULL == if_name))
		return 0;
	
	int ret;
	int op_ret;
	unsigned char macAddr[MAC_LEN];
	char *ip;
	char *ifname;
	char *mac;
	int is_add = 0;
	int retu;
	
	int len = strlen(type);
	if(len <= 3 && strncmp(type,"add",len) == 0){
		is_add = 1;
	}else if(len <= 3 && strncmp(type,"del",len) == 0){
		is_add = 2;
	}else if(len <= 6 && strncmp(type,"change",len) == 0){
		is_add = 3;
	}else if(len <= 7 && strncmp(type,"replace",len) == 0){
		is_add = 4;
	}else{
		return -1;		
	}
	ret = Check_IP_Format((char*)IP);
	if(ret != ASD_DBUS_SUCCESS){
		return -2;
	}
	ip = malloc(strlen(IP)+1);
	if(NULL == ip)
        return 0;
	memset(ip, 0, strlen(IP)+1);
	memcpy(ip, IP,strlen(IP));
	
	op_ret = parse_mac_addr(MAC,&macAddr);

	if (NPD_FAIL == op_ret) {
		FREE_OBJECT(ip);
		return -3;
	}
	
	mac = malloc(strlen(MAC)+1);
	if(NULL == mac)
        return 0;
	memset(mac, 0, strlen(MAC)+1);
	memcpy(mac, MAC, strlen(MAC));

	ifname = malloc(strlen(if_name)+1);
	if(NULL == ifname)
        return 0;
	memset(ifname, 0, strlen(if_name)+1);
	memcpy(ifname, if_name, strlen(if_name));

	int(*dcli_init_func)(
							int , 
							int ,
							int , 
							char *, 
							char*, 
							char*,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_asd_set_sta_arp");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
					 parameter.instance_id,
					 parameter.local_id,
					 is_add, 
					 ip, 
					 mac, 
					 ifname, 
					 connection
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
	
	FREE_OBJECT(ip);
	FREE_OBJECT(ifname);
	FREE_OBJECT(mac);

	if(ret == -1)
		retu = SNMPD_CONNECTION_ERROR;
	else if(ret == ASD_DBUS_SUCCESS)
		retu = 1;
	else 
		retu = -4;

	return retu;
}

/*type为"black"或"white"*/
int ac_add_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *MAC)
																   /*返回0表示失败，返回1表示成功*/
																   /*返回-1表示input patameter should only be 'black' or 'white'*/
																   /*返回-2表示Unknown mac addr format，返回-3表示error*/
																   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == type)||(NULL == MAC))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN] = {0};
	unsigned int ret;
	unsigned char list_type=0;   /*1--black list, 2--white list*/
	int retu;

	str2lower(&type);
	
	if (!strcmp(type,"black")){
		list_type=1;		
	}
	else if (!strcmp(type,"white")){
		list_type=2;		
	}
	else{
		return -1;
	}
	
	ret = wid_parse_mac_addr((char *)MAC,&mac);
	if (CMD_FAILURE == ret) {
		return -2;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_AC_ADD_MAC_LIST);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_DBUS_SUCCESS)
	{
		retu = 1;
	}	
	else 
	{
		retu = -3;
	}
	
	dbus_message_unref(reply);
	return retu; 
}

/*type为"black"或"white"*/
int ac_del_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *MAC)
																   /*返回0表示失败，返回1表示成功*/
																   /*返回-1表示input patameter should only be 'black' or 'white'*/
																   /*返回-2表示Unknown mac addr format，返回-3表示error*/
																   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																   /*返回-4表示mac is not in the list*/
{
    if(NULL == connection)
        return 0;
	
	if((NULL == type)||(NULL == MAC))
		return 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN]={0};
	unsigned int ret;
	unsigned char list_type=0;   /*1--black list, 2--white list*/
	int retu;

	str2lower(&type);
	
	if (!strcmp(type,"black")){
		list_type=1;		
	}
	else if (!strcmp(type,"white")){
		list_type=2;		
	}
	else{
		return -1;
	}
	
	ret = wid_parse_mac_addr((char *)MAC,&mac);
	if (CMD_FAILURE == ret) {
		return -2;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_AC_DEL_MAC_LIST);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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
	
	if(ret==ASD_DBUS_SUCCESS)
	{
		retu = 1;
	}		
	else if(ret == ASD_UNKNOWN_ID)
	{
		retu = -4;
	}
	else 
	{
		retu = -3; 
	}
	
	dbus_message_unref(reply);
	return retu; 
}

/*type为"none"、"black"或"white"*/
int ac_use_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,char *type)
														/*返回0表示失败，返回1表示成功*/
													    /*返回-1表示input patameter should only be 'none','black' or 'white'*/
													    /*返回-2表示Wids is enable,ac can only use black list!，返回-3表示error*/
														/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == type)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char list_type=0;   /*0--none,1--black list, 2--white list*/
	int retu;
	
	str2lower(&type);
	
	if (!strcmp(type,"none")){
		list_type=0;			
	}
	else if (!strcmp(type,"black")){
		list_type=1;
	}
	else if (!strcmp(type,"white")){
		list_type=2;	
	}
	else{
		return -1;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_AC_USE_MAC_LIST);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&list_type,
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
	
	if(ret==ASD_DBUS_SUCCESS)
		retu = 1;
	else if(ret==ASD_WIDS_OPEN)
		retu = -2;
	else 
		retu = -3;
	
	dbus_message_unref(reply);
	return retu; 
}

void Free_ac_mac_head(struct ac_mac_list_profile *info)
{
	int i = 0;
    struct mac_list_profile *f1=NULL,*f2=NULL;

	if((info != NULL)&&(info->black_mac_list != NULL)&&(info->black_mac_list->next != NULL))
	{
		f1=info->black_mac_list->next;		
		for(i = 0; i < info->black_mac_num; i++)
		{
		  if(NULL == f1)
		    break;
		    
		  f2=f1->next;
		  FREE_OBJECT(f1);
		  f1=f2;
		}
		FREE_OBJECT(info->black_mac_list);
	}

	if((info != NULL)&&(info->white_mac_list != NULL)&&(info->white_mac_list->next != NULL))
	{
		f1=info->white_mac_list->next;		
		for(i = 0; i < info->white_mac_num; i++)
		{
		    if(NULL == f1)
		        break;
		    
            f2=f1->next;
            FREE_OBJECT(f1);
            f1=f2;
		}
		FREE_OBJECT(info->white_mac_list);
	}
}

/*返回1时，调用Free_ac_mac_head()释放空间*/
int show_ac_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,struct ac_mac_list_profile *info)/*返回0表示失败，返回1表示成功*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
    if(NULL == connection)
        return 0;
	
	if(NULL == info)
		return 0;
	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
	DBusMessageIter iter_sub_struct;
	DBusError err;
	unsigned int ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_AC_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
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
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	{		
		unsigned int maclist_acl;
		unsigned int num[2];
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		info->maclist_acl = maclist_acl;
        
		for(i=0; i<2; i++) {
			struct mac_list_profile *tail = NULL;
			
			if(i == 0)
			{
				info->black_mac_num = num[i];
				info->black_mac_list = (struct mac_list_profile *)malloc(sizeof(struct mac_list_profile));
				if(info->black_mac_list != NULL) {
					info->black_mac_list->next = NULL;
					tail = info->black_mac_list;
				}
				else {
				    return -1;
				}    
			}
			else if(i == 1)
			{
				info->white_mac_num = num[i];
				info->white_mac_list = (struct mac_list_profile *)malloc(sizeof(struct mac_list_profile));
				if(info->white_mac_list != NULL) {
					info->white_mac_list->next = NULL;
					tail = info->white_mac_list;
				}
				else {
				    return -1;
				}
			}
			
			for(j=0; j<num[i]; j++){	
				unsigned char mac[6];

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(mac[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(mac[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(mac[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(mac[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(mac[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(mac[5]));	
				
				dbus_message_iter_next(&iter_sub_array);
				struct mac_list_profile *q = NULL;
				if(NULL == (q = (struct mac_list_profile*)malloc(sizeof(struct mac_list_profile)+1))) {
				    continue;
				}    
				memset(q,0,sizeof(struct mac_list_profile)+1);
				memcpy(q->mac, mac, 6);

				if(tail)
				{
					q->next = tail->next;
					tail->next = q;
				}
			}
		}
		dbus_message_iter_next(&iter_array);
	}
	dbus_message_unref(reply);

	return 1;	
}


void Free_show_wlan_sta_of_all_cmd(struct dcli_wlan_info *wlan)
{
	void (*dcli_init_free_func)(struct dcli_wlan_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_wlanlist");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wlan);
		}
	}
}

/*返回1时，调用Free_show_wlan_sta_of_all_cmd()释放空间*/
int show_wlan_sta_of_all_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wlan_info **wlan)
																				/*返回0表示失败，返回1表示成功*/
																				/*返回-1表示wlan does not exist，返回-2表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{		
    if(NULL == connection)
        return 0;
	
	unsigned int ret = 0;
	int wlan_num = 0;
	int retu = 0;
	

	void*(*dcli_init_func)(
						DBusConnection *, 
						int , 
						int *, 
						int ,
						unsigned int *
						);

    *wlan = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_of_allwlan");
		if(NULL != dcli_init_func)
		{
			*wlan = (*dcli_init_func)
				(
					connection, 
					parameter.instance_id, 
					&wlan_num,
					parameter.local_id,
					&ret
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
	
	if((ret == 0) && (*wlan != NULL))
		retu = 1;		
	else if (ret == ASD_WLAN_NOT_EXIST)	
		retu = -1;
	else if (ret == ASD_DBUS_ERROR)
		retu = SNMPD_CONNECTION_ERROR;
	else
		retu = -2;

	return retu;
}

void Free_show_distinguish_information_of_all_wtp_cmd(struct dcli_wtp_info *wtp)
{
	void (*dcli_init_free_func)(struct dcli_wtp_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_wtp_list");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wtp);
		}
	}
}

/*只要调用函数，就调用Free_show_distinguish_information_of_all_wtp_cmd()释放空间*/
int show_distinguish_information_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wtp_info **wtp)
																									/*返回0表示失败，返回1表示成功*/
																								    /*返回-1表示wtp does not provide service or it maybe does not exist*/
																								    /*返回-2表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
        return 0;
	
	unsigned int wtp_num;
	unsigned int ret = 0;
	int retu = 0;
	
	void*(*dcli_init_func)(
						int ,
						DBusConnection *, 
						unsigned int *,
						int ,
						unsigned int *
						);

	*wtp = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_distinguish_info_of_all_wtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)
				(
					parameter.instance_id,
					connection,
					&wtp_num,
					parameter.local_id,
					&ret
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
	
	if((ret == 0) && (*wtp != NULL))
	{
		retu = 1;
	}
	else if (ret == ASD_WTP_NOT_EXIST)
	{
		retu = -1;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -2;
	}
	
	return retu;	
}


void Free_show_wapi_mib_info_of_all_wtp_cmd(struct dcli_wtp_info *wtp)
{
	void (*dcli_init_free_func)(struct dcli_wtp_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_wtp_list");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wtp);
		}
	}
}

/*只要调用函数，就调用Free_show_wapi_mib_info_of_all_wtp_cmd()释放空间*/
int show_wapi_mib_info_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wtp_info **wtp)
																						 /*返回0表示失败，返回1表示成功*/
																					     /*返回-1表示wtp does not provide service or it maybe does not exist*/
																					     /*返回-2表示error*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
	
	unsigned int ret = 0;
	unsigned int wtp_num;
	int retu = 0;	
			
	void*(*dcli_init_func)(
							DBusConnection *, 
							int , 
							unsigned int *, 
							int ,
							unsigned int *
						);

	*wtp = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wapi_mib_info_of_all_wtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)
				(
					connection,
					parameter.instance_id,
					&wtp_num,
					parameter.local_id,
					&ret
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

	if((ret == 0)&&(*wtp != NULL))
	{
		retu = 1;
	}
	else if (ret ==ASD_WTP_NOT_EXIST) 
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -2;
	}
	
	return retu;		
}

void Free_show_sta_wapi_mib_info_of_all_wtp_cmd(struct wapi_mib_wtp_info *wtp)
{
	void (*dcli_init_free_func)(struct wapi_mib_wtp_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_sta_wapi_mib_info_free_wtp_list");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wtp);
		}
	}
}

/*只要调用函数，就调用Free_show_sta_wapi_mib_info_of_all_wtp_cmd()释放空间*/
int show_sta_wapi_mib_info_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct wapi_mib_wtp_info **wtp)
																								 /*返回0表示失败，返回1表示成功*/
																							     /*返回-1表示wtp does not provide service or it maybe does not exist*/
																							     /*返回-2表示error*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
	
	unsigned int ret = 0;
	unsigned int wtp_num;
	int retu = 0;
	
	void*(*dcli_init_func)(
							DBusConnection *, 
							int , 
							unsigned int *, 
							int ,
							unsigned int *
						);

	*wtp = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_wapi_mib_info_of_all_wtp");
		if(NULL != dcli_init_func)
		{
			*wtp = (*dcli_init_func)
				(
					connection,
					parameter.instance_id,
					&wtp_num,
					parameter.local_id,
					&ret
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

	if((ret == 0)&&(*wtp != NULL))
	{
		retu = 1;
	}
	else if (ret ==ASD_WTP_NOT_EXIST) 
	{
		retu = -1;
	}
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -2;
	}
	
	return retu;
}

void Free_show_wlan_info_allwlan_cmd(struct dcli_wlan_info *wlan)
{
	void (*dcli_init_free_func)(struct dcli_wlan_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_allwlan");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(wlan);
		}
	}
}

/*只要调用函数，就调用Free_show_wlan_info_allwlan_cmd()释放空间*/
int show_wlan_info_allwlan_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wlan_info **wlan)
																				  /*返回0表示失败，返回1表示成功*/
																			      /*返回-1表示wlan does not provide service or it maybe does not exist*/
																			      /*返回-2表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
        
	unsigned int ret = 0;
	int wlan_num = 0;
	int retu = 0;

	void*(*dcli_init_func)(
							DBusConnection *, 
							int ,
							int ,
							unsigned int *,
							int *
						);

	*wlan = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_info_allwlan");
		if(NULL != dcli_init_func)
		{
			*wlan = (*dcli_init_func)
				(
					connection,
					parameter.instance_id,
					parameter.local_id,
					&ret,
					&wlan_num
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
	
	if((ret == 0) && (*wlan != NULL))
	{
		retu = 1;
	}
	else if (ret == ASD_WLAN_NOT_EXIST)
	{
		retu = -1;
	}	
	else if (ret == ASD_DBUS_ERROR)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -2;
	}

	return retu;
}

void Free_show_all_sta_base_info_cmd(struct dcli_base_bss_info *bsshead)
{
	void (*dcli_init_free_func)(struct dcli_base_bss_info *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_base_bss");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(bsshead);
		}
	}
}

/*返回1时，调用Free_show_all_sta_base_info_cmd()释放空间*/
int show_all_sta_base_info_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_base_bss_info **bsshead)
																				  /*返回0表示失败，返回1表示成功*/
																			      /*返回-1表示there is no station，返回-2表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
        return 0;
	
	unsigned int ret = 0;
	unsigned int bss_num = 0;
	int retu = 0;
	
	void*(*dcli_init_func)(
							DBusConnection *,
							int , 
							int , 
							unsigned int *,
							unsigned int *
						);

	*bsshead = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_sta_base_info");
		if(NULL != dcli_init_func)
		{
			*bsshead = (*dcli_init_func)
				(
					connection,
					parameter.instance_id,
					parameter.local_id,
					&ret,
					&bss_num
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

	if((ret == 0) && ((*bsshead) != NULL))
		retu = 1;
	else if(bss_num == 0)
		retu = -1;
	else if (ret == ASD_DBUS_ERROR)
		retu = SNMPD_CONNECTION_ERROR;
	else
		retu = -2;
	
	return retu;
}


/*Type为"listen"或"listen_and_set"*/
/*state为"enable"或"disable"*/
int set_asd_sta_arp_listen_cmd(dbus_parameter parameter, DBusConnection *connection,char *Type,char *state)
														 /*返回0表示失败，返回1表示成功*/
													     /*返回-1表示input para error*/
														 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
        return 0;
        
	if((NULL == Type) || (NULL == state))
		return 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char op = 0;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/	
	int retu = 0;
	
	str2lower(&Type);
	
	if (!strcmp(Type,"listen")){
		op=1; 	
	}else if (!strcmp(Type,"listen_and_set")){
		op=2; 	
	}else{
		return -1;
	}
	
	str2lower(&state);
	
	if (!strcmp(state,"enable")){
		type=1;		
	}else if (!strcmp(state,"disable")){
		type=0;		
	}else{
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&op,
							DBUS_TYPE_BYTE,&type,
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
	
	if(ret==0)
	{
		retu = 1;
	}

	dbus_message_unref(reply);

	return retu; 
}


#ifdef __cplusplus
}
#endif

