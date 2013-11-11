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
* ws_dcli_ac.c
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


#include <syslog.h>
#include "ws_dcli_ac.h"


/*dcli_ac.c V1.30*/
/*author liutao*/
/*update time 09-3-2*/

/*dcli_ac.c V1.31*/
/*author wangpeng*/
/*update time 09-3-13*/
/*related:ws_dcli_bss.c*/

/*dcli_ac.c V1.109*/
/*author qiaojie*/
/*update time 10-03-24*/

int wid_mac_format_check(char* str,int len) 
{
	int i = 0;
	unsigned int result = CMD_SUCCESS;
	char c = 0;
	
	if( 17 != len){
	   return CMD_FAILURE;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
			if((':'!=c)&&('-'!=c)&&('.'!=c))
				return CMD_FAILURE;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = CMD_FAILURE;
			return result;
		}
    }
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
		
        result = CMD_FAILURE;
		return result;
	}
	return result;
}

 int wid_parse_mac_addr(char* input,WIDMACADDR* macAddr) 
 {
 	
	int i = 0;
	char cur = 0,value = 0;
	
	if((NULL == input)||(NULL == macAddr)) return CMD_FAILURE;
	if(CMD_FAILURE == wid_mac_format_check(input,strlen(input))) return CMD_FAILURE;
	
	
	for(i = 0; i <6;i++) {
		cur = *(input++);
		if((cur == ':') ||(cur == '-')||(cur == '.')){
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->macaddr[i] = value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->macaddr[i] = (macAddr->macaddr[i]<< 4)|value;
	}
	
	return CMD_SUCCESS;
} 
int parse_country_code(char *input)
{
	if ((!strcmp(input,"cn"))||(!strcmp(input,"eu"))||(!strcmp(input,"us"))
		||(!strcmp(input,"jp"))||(!strcmp(input,"fr"))||(!strcmp(input,"es")))
	{
		return	COUNTRY_CODE_ERROR_SMALL_LETTERS;
		}
	
	if (!strcmp(input,"CN")) return COUNTRY_CHINA_CN;
	else if (!strcmp(input,"EU")) return COUNTRY_EUROPE_EU;
	else if (!strcmp(input,"US")) return COUNTRY_USA_US;
	else if (!strcmp(input,"JP")) return COUNTRY_JAPAN_JP;
	else if (!strcmp(input,"FR")) return COUNTRY_FRANCE_FR;
	else if (!strcmp(input,"ES")) return COUNTRY_SPAIN_ES;
	else 
	{
		return COUNTRY_CODE_ERROR;
	}
}


int wid_parse_OUI(char* input,OUI_S *ouielem) 
 {
 	
	int i = 0,j=0;
	char cur = 0,value = 0;
	char *p;
	
	if((NULL == input)||(NULL == ouielem)) 
		return -1;
	
	if(strlen(input)!=8)
		return -2;

	p=input;
	
	for(j=0;j<8;j++){
		if(j==2||j==5){
			if(*(p+j)!=':')
				return -3;
			else
				continue;

		}else{
			if(!((*(p+j)>='0'&&*(p+j)<='9')
				||(*(p+j)>='a'&&*(p+j)<='f')
				||(*(p+j)>='A'&&*(p+j)<='F'))){
				return -3;
			}
				
		}
	}

	
	for(i = 0; i <3;i++) {
		cur = *(input++);
		if(cur == ':'){
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		
		ouielem->oui[i]= value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		ouielem->oui[i] = (ouielem->oui[i]<< 4)|value;
	}
	
	return 0;
}

void CheckWIDSType(char *pattacktype, char* pframetype, unsigned char attacktype,unsigned char frametype)
{
	
	switch(attacktype)
	{

		case 1 :
			{
				strcpy(pattacktype, "flood");
				
				switch(frametype)
				{
					case 1 :
						strcpy(pframetype, "probe");
						break;
						
					case 2 :
						strcpy(pframetype, "auth");
						break;
					
					case 3 :
						strcpy(pframetype, "assoc");
						break;
						
					case 4 :
						strcpy(pframetype, "reassoc");
						break;
						
					case 5 :
						strcpy(pframetype, "deauth");
						break;
					
					case 6 :
						strcpy(pframetype, "deassoc");
							break;
					case 7 :
						strcpy(pframetype, "data");
						break;
					
					case 8 :
						strcpy(pframetype, "action");
						break;

					default:
						strcpy(pframetype, "unknown");
						break;
				}
			}
			
			break;
			
		case 2 :
			{
				strcpy(pattacktype, "spoof");
				switch(frametype)
				{
					case 1 :
						strcpy(pframetype, "deauth");
						break;
						
					case 2 :
						strcpy(pframetype, "deassoc");
						break;
						
					default:
						strcpy(pframetype, "unknown");
						break;
				}

			}
			break;
		
		case 3 :
			{
				strcpy(pattacktype, "weakiv");
				
				switch(frametype)
				{
					case 3 :
						strcpy(pframetype, "weakiv");
						break;
						
					default:
						strcpy(pframetype, "unknown");
						break;
				}
			}
			break;
		default:
			strcpy(pattacktype, "unknown");
			break;
			
		
	}

}

void Free_wc_config(DCLI_AC_API_GROUP_FIVE *wirelessconfig)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_WIDCONFIG,wirelessconfig);
			wirelessconfig = NULL;
		}
	}
}


/*返回1时，调用Free_wc_config()释放空间*/
int show_wc_config(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **wirelessconfig)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	int ret = 0;
	int retu;

	void*(*dcli_init_func)(
						int ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						unsigned int *,
						char *,
						char *,
						int ,
						DBusConnection *,
						char *
						);

	*wirelessconfig = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			
			*wirelessconfig = (*dcli_init_func)(
				parameter.instance_id,
				FIRST,/*"show wireless-control config"*/
				0,
				0,
				0,
				&ret,
				0,
				0,
				parameter.local_id,
				connection,
				WID_DBUS_CONF_METHOD_WIDCONFIG
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
	else if((ret == 0)&&(*wirelessconfig))
	{
		retu = 1;
	}
	else 
	{
		retu = -1;
	}

	return retu;	
}

/*未使用*/
int set_wid_hw_version_func(dbus_parameter parameter, DBusConnection *connection,char *para)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	
	if(NULL == connection)
		return 0;
	
	if(NULL == para)
		return 0;

	int ret,retu;
	unsigned char * hwversion;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	
	hwversion = (char*)malloc(strlen(para)+1);
	if(NULL == hwversion)
		return 0;
	memset(hwversion, 0, strlen(para)+1);
	memcpy(hwversion, para, strlen(para));	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_HW_VERSION);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_HW_VERSION);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&hwversion,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		
		FREE_OBJECT(hwversion);

		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);
	
	FREE_OBJECT(hwversion);
	
	return retu;	

}

/*未使用*/
int set_wid_sw_version_func(dbus_parameter parameter, DBusConnection *connection,char *para)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == para)
		return 0;

	int ret,retu;
	unsigned char * swversion;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	
	swversion = (char*)malloc(strlen(para)+1);
	if(NULL == swversion)
		return 0;
	memset(swversion, 0, strlen(para)+1);
	memcpy(swversion, para, strlen(para));	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SW_VERSION);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SW_VERSION);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&swversion,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		
		FREE_OBJECT(swversion);

		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);
	
	FREE_OBJECT(swversion);
	
	return retu;	
}

/*未使用*/
/*protocol_type为IPv4或IPv6*/
int set_wid_lev3_protocol_func(dbus_parameter parameter, DBusConnection *connection,char *protocol_type)
																		/*返回0表示失败，返回1表示成功，返回-1表示This version only surport IPv4,we will update later*/
																		/*返回-2表示input patameter should only be 'IPv4' or 'IPv6'，返回-3表示error*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == protocol_type)
		return 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char lev3protocol;
	int ret,retu;

	if (!strcmp(protocol_type,"IPv4"))
	{
		lev3protocol = 1;	
	}		
	else if (!strcmp(protocol_type,"IPv6"))
	{
		//lev3protocol = 0;
		return -1;
	}
	else
	{
		return -2;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LEV3_PROTOCOL);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LEV3_PROTOCOL);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&lev3protocol,
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
	else
		retu=-3;

	dbus_message_unref(reply);
	return retu;		
	
}

/*未使用*/
/*auth_secu_type为PRESHARED或X509_CERTIFCATE*/
int set_wid_auth_security_func(dbus_parameter parameter, DBusConnection *connection,char *auth_secu_type)/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'X509_CERTIFCATE' or 'PRESHARED'，返回-2表示error*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == auth_secu_type)
		return 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char securitytype;
	int ret,retu;

	if (!strcmp(auth_secu_type,"X509_CERTIFCATE"))
	{
		securitytype = 1;	
	}		
	else if (!strcmp(auth_secu_type,"PRESHARED"))
	{
		securitytype = 0;
	}
	else
	{
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SECURITY_TYPE);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SECURITY_TYPE);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&securitytype,
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
	else
		retu=-2;

	dbus_message_unref(reply);
	return retu;		
	
}

/*未使用*/
int set_wid_name_func(dbus_parameter parameter, DBusConnection *connection,char *para)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == para)
		return 0;

	int ret,retu;
	unsigned char * acname;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	
	acname = (char*)malloc(strlen(para)+1);
	if(NULL == acname)
		return 0;
	memset(acname, 0, strlen(para)+1);
	memcpy(acname, para, strlen(para));	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AC_NAME);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AC_NAME);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&acname,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		
		FREE_OBJECT(acname);

		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);
	
	FREE_OBJECT(acname);
	
	return retu;		
}


/*int	config_wireless_max_wtp(char *num)
{
	int ret;
	int wtpnums;
	int retu = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    wtpnums = atoi(num);
	//make robust code
	
	if((wtpnums < 15)||(wtpnums > WTP_NUM))
	{
		//vty_out(vty,"<error> input parameter should be 15 to 1024\n");
		//return ;
		retu = -3;
		return retu;
	}


	
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_MAX_WTP);
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpnums,
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
	{
		retu = 1;
		//vty_out(vty,"wireless-control set max wtp %d successfully\n",wtpnums);
	}				
	else if(ret == WTP_LESS_THAN_STATIC_WTP)
	{
		retu = -1;
		//vty_out(vty,"<error> wireless-control set max wtp must over current static wtp\n");
	}
	else
	{
		retu = -2;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return retu;		
}*/


int config_wireless_max_mtu(dbus_parameter parameter, DBusConnection *connection,char*mtu)
															/*返回0表示失败，返回1表示成功*/
														    /*返回-1表示input parameter should be 500 to 1500*/
														    /*返回-2表示error*/
														    /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == mtu)
		return 0;	

	int ret;
	int maxmtu;
	int retu = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    maxmtu = atoi(mtu);
	//make robust code

	if((maxmtu < 500)||(maxmtu > 1500))
	{
		retu = -1;
		return retu;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MAX_MTU);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_MAX_MTU);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&maxmtu,
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
	{
		//vty_out(vty,"wireless-control set mtu %d successfully\n",maxmtu);
		retu = 1;
	}				
	else
	{
	//	vty_out(vty,"<error>  %d\n",ret);
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;		
}


int	 set_wid_log_switch_cmd(dbus_parameter parameter, DBusConnection *connection,char*state)
															/*返回0表示失败，返回1表示成功*/
															/*返回-1表示error*/
															/*返回-2表示input patameter should only be 'ON' or 'OFF'*/
															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == state)
		return 0;	

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char logswitch;
	int ret,retu;

	if (!strcmp(state,"ON"))
	{
		logswitch = 1;	
	}		
	else if (!strcmp(state,"OFF"))
	{
		logswitch = 0;
	}
	else
	{
		//vty_out(vty,"<error> input patameter should only be 'ON' or 'OFF'\n");
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOG_SWITCH);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LOG_SWITCH);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&logswitch,
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
		//vty_out(vty,"wireless-control set log switch %s successfully\n",argv[0]);
		retu = 1;
	else
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -1;
	
	dbus_message_unref(reply);
	
	return retu;			
}

/*未使用*/
/*log_lever_type为info或debug或all*/
int set_wid_log_level_func(dbus_parameter parameter, DBusConnection *connection,char *log_lever_type)/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'info' 'debug' or 'all'，返回-2表示error*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == log_lever_type)
		return 0;	

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char widloglevel;

	/*if (!strcmp(argv[0],"none"))
	{
		widloglevel = 0;	
	}		
	else if (!strcmp(argv[0],"warning"))
	{
		widloglevel = 1;
	}
	else */if (!strcmp(log_lever_type,"info"))
	{
		widloglevel = 1;	
	}
	else if (!strcmp(log_lever_type,"debug"))
	{
		widloglevel = 8;	
	}
	else if (!strcmp(log_lever_type,"all"))
	{
		widloglevel = 15;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOG_LEVEL);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LOG_LEVEL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&widloglevel,
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*未使用*/
/*state为open或者close*/
/*type为"default"/"dbus"/"wtp"/"mb"/"all"*/
int set_wid_daemonlog_debug_open_func(dbus_parameter parameter, DBusConnection *connection,char *type,char *state)
																					   /*返回0表示失败，返回1表示成功*/
																					   /*返回-1表示input patameter should only be default|dbus|wtp|mb|all*/
																					   /*返回-2表示input patameter should only be 'open' or 'close'，返回-3表示error*/
																					   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if((NULL == type)||(NULL == state))
		return 0;	

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    
    unsigned int daemonlogtype;
    unsigned int daemonloglevel;

	if (!strcmp(type,"default"))
	{
		daemonlogtype = WID_DEFAULT;	
	}
	else if (!strcmp(type,"dbus"))
	{
		daemonlogtype = WID_DBUS;	
	}
	else if (!strcmp(type,"wtp"))
	{
		daemonlogtype = WID_WTPINFO;	
	}
	else if (!strcmp(type,"mb"))
	{
		daemonlogtype = WID_MB;	
	}	
	else if (!strcmp(type,"all"))
	{
		daemonlogtype = WID_ALL;	
	}
	else
	{
		return -1;
	}
    

	
	if (!strcmp(state,"open"))
	{
		daemonloglevel = 7;	
	}
	else if (!strcmp(state,"close"))
	{
		daemonloglevel = 0;	
	}
	else
	{
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DAEMONLOG_DEBUG_OPEN);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DAEMONLOG_DEBUG_OPEN);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&daemonlogtype,
							 DBUS_TYPE_UINT32,&daemonloglevel,
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
	{
		retu=1;
	}				
	else
	{
		retu=-3;
	}
		
	dbus_message_unref(reply);
	
	return retu;			
}


int  set_wid_log_size_cmd(dbus_parameter parameter, DBusConnection *connection,char *size)
														  /*返回0表示失败，返回1表示成功*/	 
														  /*返回-1表示input parameter should be 1000000 to 500000000*/
														  /*返回-2表示error*/
														  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == size)
		return 0;

	int ret;
	int logsize;
	int retu = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    logsize = atoi(size);
	//make robust code

	if((logsize < 1000000)||(logsize > 500000000))
	{
		//vty_out(vty,"<error> input parameter should be 1000000 to 500000000\n");
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOG_SIZE);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LOG_SIZE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&logsize,
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
	{
		retu = 1;
		//vty_out(vty,"wireless-control set log size %d successfully\n",logsize);
	}				
	else
	{
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*state==0表示"disable"，state==1表示"enable"*/
int set_radio_resource_management(dbus_parameter parameter, DBusConnection *connection,int state)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	policy = state;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -1;
	}
		
	dbus_message_unref(reply);
	
	return retu;			
}


int set_ap_scanning_report_interval_cmd(dbus_parameter parameter, DBusConnection *connection,char* rm_time)
																			 /*返回0表示失败，返回1表示成功*/
																			 /*返回-1表示input patameter should be 30 to 32767*/
																			 /*返回-2表示error*/
																			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == rm_time)
		return 0;
	
	int ret;
    int retu = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int time = 0;

    time = atoi(rm_time);
	
	if (time < 30 || time > 32767)
	{
	
		//vty_out(vty,"<error> input patameter should be 30 to 32767\n");
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING_REPORT_INTERVAL);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING_REPORT_INTERVAL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
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
	{
		//vty_out(vty," set ap scanning report interval %s successfully\n",argv[0]);
		retu = 1;
	}				
	else
	{
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*未使用*/
int update_ap_scanning_info_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int state = 1;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_AP_SCANNING_INFO);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_AP_SCANNING_INFO);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
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
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}


int set_mac_whitelist(dbus_parameter parameter, DBusConnection *connection,char *mac)
												   /*返回0表示失败，返回1 表示成功*/
				                                   /*返回-1表示Unknown mac addr format*/
												   /*返回-2表示error*/
												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == mac)
		return 0;

	int ret,retu;
	WIDMACADDR macaddr; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_mac_addr(mac,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	return retu;			
}

int delete_mac_whitelist(dbus_parameter parameter, DBusConnection *connection,char *mac)
													  /*返回0表示失败，返回1 表示成功*/
			                                          /*返回-1表示Unknown mac addr format*/
													  /*返回-2表示error*/
													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == mac)
		return 0;

	int ret,retu;
	WIDMACADDR macaddr; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_mac_addr(mac,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_DELETE);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_DELETE);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	return retu;			
}


void Free_wireless_control_whitelist(DCLI_AC_API_GROUP_ONE *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_ONE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_WHITELIST_SHOW,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_wireless_control_whitelist()释放空间*/
int show_wireless_control_whitelist(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE  **LIST)
																									/*返回0表示失败，返回1表示成功，返回2表示there is no white list，返回-1表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
			return 0;

	int ret;
	int retu;


	*LIST = NULL;
	CW_CREATE_OBJECT_ERR(*LIST, DCLI_AC_API_GROUP_ONE, return -1;);	
	(*LIST)->dcli_attack_mac_list = NULL;
	(*LIST)->dcli_essid_list = NULL;
	(*LIST)->dcli_oui_list = NULL;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_one");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)
				(
					parameter.instance_id,
					0,/*"show wireless-control whitelist"*/
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_WHITELIST_SHOW
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
	else if((ret == 0)&&(*LIST))
	{
		retu = 1;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		retu = 2;
	}
	else
	{
		retu = -1;
	}
	
	return retu;				
}


int set_mac_blacklist(dbus_parameter parameter, DBusConnection *connection,char *mac)
												  /*返回0表示失败，返回1 表示成功*/
	                                      		  /*返回-1表示Unknown mac addr format*/
												  /*返回-2表示error*/
												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == mac)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    WIDMACADDR macaddr;

	ret = wid_parse_mac_addr(mac,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}


int delete_mac_blacklist(dbus_parameter parameter, DBusConnection *connection,char *mac)
													  /*返回0表示失败，返回1 表示成功*/
                                            		  /*返回-1表示Unknown mac addr format*/
													  /*返回-2表示error*/
													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == mac)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    WIDMACADDR macaddr;

	ret = wid_parse_mac_addr(mac,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}		
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_DELETE);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_DELETE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

void Free_wireless_control_blacklist(DCLI_AC_API_GROUP_ONE *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_ONE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_BLACKLIST_SHOW,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_wireless_control_blacklist()释放空间*/
int show_wireless_control_blacklist(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE  **LIST)
																						/*返回0表示失败，返回1表示成功，返回2表示there is no black list，返回-1表示error*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;
	
	
	*LIST = NULL;
	CW_CREATE_OBJECT_ERR(*LIST, DCLI_AC_API_GROUP_ONE, return -1;);	
	(*LIST)->dcli_attack_mac_list = NULL;
	(*LIST)->dcli_essid_list = NULL;
	(*LIST)->dcli_oui_list = NULL;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_one");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)
				(
					parameter.instance_id,
					0,/*"show wireless-control blacklist"*/
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_BLACKLIST_SHOW
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
	else if((ret == 0)&&(*LIST))
	{
		retu = 1;
	}
	else if(ret == WID_NO_BLACKLIST)
	{
		retu = 2;
	}
	else
	{
		retu = -1;
	}
		
	return retu;					
}


void Free_rogue_ap_head(DCLI_AC_API_GROUP_TWO *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_ap_head()释放空间*/
int show_rogue_ap_list(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **LIST) 
																		  /*返回0表示失败，返回1表示成功，返回2表示there is no rouge ap*/
                                                                          /*返回-1表示radio resource managment is disable please enable first*/ 
																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;


	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)(
				parameter.instance_id,
				FIRST,/*"show rogue ap list"*/
				0,
				0,
				0,
				&ret,
				0,
				&(parameter.local_id),
				connection,
				WID_DBUS_CONF_METHOD_ROGUE_AP_LIST
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
	else if((ret == 0)&&(*LIST))
	{	
		retu = 1;
	}		
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		retu = -1;
	}
	else
	{	
		retu = 2;
	}
	
	return retu;			
}

void Free_rogue_aplist_bywtpid(DCLI_AC_API_GROUP_TWO *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID,LIST);
			LIST = NULL;
		}
	}
}
		
/*返回1时，调用Free_rogue_aplist_bywtpid()释放空间*/
int show_rogue_aplist_bywtpid(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_AC_API_GROUP_TWO **LIST)  
																							  /*返回0表示失败，返回1表示成功，返回2表示there is no rouge ap*/
                                                                                              /*返回-1表示input wtp id should be 1 to WTP_NUM-1，返回-2表示wtp does not exist*/
                                                                                              /*返回-3表示radio resource managment is disable please enable first*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;
	
	int wtpid = 0;

	wtpid = wtp_id;
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
	}

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)
				(
					parameter.instance_id,
					THIRD,/*"show rogue ap list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID
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
	else if((ret == 0)&&(*LIST)){	
		retu = 1;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		retu = -3;
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else
	{
		retu = 2;
	}
	
	return retu;			
}

void Free_neighbor_aplist_bywtpid(DCLI_AC_API_GROUP_TWO *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,LIST);
			LIST = NULL;
		}
	}
}
		
/*返回1时，调用Free_neighbor_aplist_bywtpid()释放空间*/
int show_neighbor_aplist_bywtpid(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_AC_API_GROUP_TWO **LIST)
																							   /*返回0表示失败，返回1表示成功，返回2表示there is no neighbor ap*/
                                                                                               /*返回-1表示input wtp id should be 1 to WTP_NUM-1，返回-2表示wtp does not exist*/
                                                                                               /*返回-3表示radio resource managment is disable please enable first*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;
	int wtpid = 0;
	
	wtpid = wtp_id;
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
	}
	
	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)
				(
					parameter.instance_id,
					FOURTH,/*"show neighbor ap list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID
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
	else if((ret == 0)&&(*LIST))
	{	
		retu = 1;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		retu = -3;
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else
	{
		retu = 2;
	}
	
	return retu;			
}

void Free_show_neighbor_ap_list_bywtpid2_cmd(DCLI_AC_API_GROUP_TWO *dcli_list)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2,dcli_list);
			dcli_list = NULL;
		}
	}
}
		
/*返回1时，调用Free_show_neighbor_ap_list_bywtpid2_cmd()释放空间*/
int show_neighbor_ap_list_bywtpid2_cmd(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,DCLI_AC_API_GROUP_TWO **dcli_list)
																											   /*返回0表示失败，返回1表示成功*/
				                                                                                               /*返回-1表示input wtp id should be 1 to WTP_NUM-1*/
				                                                                                               /*返回-2表示wtp does not existt*/
				                                                                                               /*返回-3表示radio resource managment is disable please enable first*/
																											   /*返回-4表示there is no neighbor ap*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == wtp_id)
	{
		*dcli_list = NULL;
		return 0;
	}

	int ret = -1;
	int wtpid = 0;
	int retu = 0;
	
	wtpid = atoi(wtp_id);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
	}

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					//DCLI_AC_API_GROUP_TWO *,
					DBusConnection *,
					char *
					);

	*dcli_list = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*dcli_list = (*dcli_init_func)
				(
					parameter.instance_id,
					FOURTH,/*"show neighbor ap list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					//dcli_list,
					connection,
					WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2
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
		retu = SNMPD_CONNECTION_ERROR;
	else if((ret == 0)&&(*dcli_list))
		retu = 1;
	else if (ret == WTP_ID_NOT_EXIST)
		retu = -2;
	else if(ret == WID_AP_SCANNING_DISABLE)
		retu = -3;
	else if(ret == WTP_ID_LARGE_THAN_MAX)
		retu = -1;
	else
		retu = -4;
	
	return retu;			
}
int dynamic_channel_selection_cmd(dbus_parameter parameter, DBusConnection *connection,char*state)
																	/*返回0表示失败，返回1表示成功*/
                                                                    /*返回-1表示input patameter only with 'enable' or 'disable'*/
                                                                    /*返回-2表示you should enable radio resource management first*/
																    /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;

	int ret;
	int retu = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int policy = 0;

	
	if (!strcmp(state,"open"))
	{
		policy = 1; 
	}
	else if (!strcmp(state,"close"))
	{
		policy = 0; 
	}
	else
	{
		//vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		//vty_out(vty," %s dynamic channel selection successfully\n",argv[0]);
		retu = 1;
	}				
	else
	{
	//	vty_out(vty,"failure,you should enable radio resource management first\n");
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu; 		
}

/*wtp_id和radio_G_id只需填写一个，分别表示在wtp节点和radio节点下配置*/
/*wtp_id为0时，表示全局配置*/
int set_system_country_code_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,int radio_G_id,char *Country_code) 
																			/*返回0表示失败，返回1表示成功，返回-1表示input country code should be capital letters*/
																			/*返回-2表示input country code error，返回-3表示system country code is already Country_code, no need to change*/
																			/*返回-4表示system country code error，返回-5表示WTP ID非法，返回-6表示Radio ID非法*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == Country_code)
		return 0;

	int ret = 0,retu;	
	int wtpid = 0;
	int radioid = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	
	int country_code = COUNTRY_USA_US;
	
	country_code = parse_country_code((char *) Country_code);
	//printf("country_code %d",country_code);
	
	
	if (country_code == COUNTRY_CODE_ERROR_SMALL_LETTERS)
	{
		return -1;
	}
	if (country_code == COUNTRY_CODE_ERROR)
	{
		return -2;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	wtpid = wtp_id;
	if(wtpid >= WTP_NUM || wtpid < 0){
		syslog(LOG_DEBUG,"wtp id in set_system_country_code_func is %d\n",wtpid);
		return -5;
	}
	radioid = radio_G_id;
	if(radioid > G_RADIO_NUM || radioid < 0){
		syslog(LOG_DEBUG,"radio id in set_system_country_code_func is %d\n",radioid);
		return -6;
   }

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_COUNTRY_CODE);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_COUNTRY_CODE);*/
	


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&country_code,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&radioid,
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


	if(ret == COUNTRY_CODE_SUCCESS)
	{
		retu=1;
	}	
	else if (ret == COUNTRY_CODE_NO_CHANGE)
	{
		retu=-3;
	}
	else
	{
		retu=-4;
	}
		
	dbus_message_unref(reply);	
	
	return retu;			
}

/*未使用*/
int undo_system_country_code_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功，返回-1表示system country code is default, no need to change，返回-2表示system country code error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = 0,retu;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	
	int country_code = COUNTRY_USA_US;//default value
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UNDO_COUNTRY_CODE);	
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_UNDO_COUNTRY_CODE);*/
	


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&country_code,
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


	if(ret == COUNTRY_CODE_SUCCESS)
	{
		retu=1;
	}	
	else if (ret == COUNTRY_CODE_NO_CHANGE)
	{
		retu=-1;
	}
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);	
	
	return retu;			
}

/*未使用*/
/*state为open或close*/
/*type为"default"/"dbus"/"80211"/"1x"/"wpa"/"wapi"/"leave"/"all"*/
int set_asd_daemonlog_debug_open_func(dbus_parameter parameter, DBusConnection *connection,char *type,char *state)
																					   /*返回0表示失败，返回1表示成功*/
																			 		   /*返回-1表示input patameter should only be default|dbus|80211|1x|wpa|wapi|leave|all*/
																					   /*返回-2表示input patameter should only be 'open' or 'close'，返回-3表示error*/
																					   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == type)||(NULL == state))
		return 0;
	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
    
    unsigned int daemonlogtype;
    unsigned int daemonloglevel;
	
	if (!strcmp(type,"default"))
	{
		daemonlogtype = ASD_DEFAULT;	
	}
	else if (!strcmp(type,"dbus"))
	{
		daemonlogtype = ASD_DBUS;	
	}
	else if (!strcmp(type,"80211"))
	{
		daemonlogtype = ASD_80211;	
	}
	else if (!strcmp(type,"1x"))
	{
		daemonlogtype = ASD_1X;	
	}
	else if (!strcmp(type,"wpa"))
	{
		daemonlogtype = ASD_WPA;	
	}
	else if (!strcmp(type,"wapi"))
	{
		daemonlogtype = ASD_WAPI;	
	}
	else if (!strcmp(type,"leave"))
	{
		daemonlogtype = ASD_LEAVE;	
	}
	else if (!strcmp(type,"all"))
	{
		daemonlogtype = ASD_ALL;	
	}
	else
	{
		return -1;
	}
	
	if (!strcmp(state,"open"))
	{
		daemonloglevel = 1;	
	}
	else if (!strcmp(state,"close"))
	{
		daemonloglevel = 0;	
	}
	else
	{
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&daemonlogtype,
							 DBUS_TYPE_UINT32,&daemonloglevel,
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
	{
		retu=1;
	}				
	else
	{
		retu=-3;
	}
		
	dbus_message_unref(reply);

	return retu;			
}

/*未使用*/
/*state为open或close*/
int set_hostapd_logger_printflag_open_func(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'open' or 'close'，返回-2表示error*/
																												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	int ret,retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char printflag;

	
	if (!strcmp(state,"open"))
	{
		printflag = 1;	
	}
	else if (!strcmp(state,"close"))
	{
		printflag = 0;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_LOGGER_PRINTFLAG);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_HOSTAPD_LOGGER_PRINTFLAG);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&printflag,
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

int dynamic_power_selection_cmd(dbus_parameter parameter, DBusConnection *connection,char*state)/*返回0表示失败，返回1表示成功，返回-1表示you should enable radio resource management first*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	int ret,retu;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int policy = 0;

	
	if (!strcmp(state,"open"))
	{
		policy = 1; 
	}
	else if (!strcmp(state,"close"))
	{
		policy = 0; 
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRANSMIT_POWER_CONTROL);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRANSMIT_POWER_CONTROL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);

	
	return retu; 		
}

int set_transmit_power_control_scope(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char scope;
	int ret,retu;

	if (!strcmp(state,"own"))
	{
		scope = 0;	
	}		
	else if (!strcmp(state,"all"))
	{
		scope = 1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CONTROL_SCOPE);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CONTROL_SCOPE);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&scope,
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
	else
		retu=-1;

	dbus_message_unref(reply);
	return retu;		
	
}


int set_wirelesscontrol_auto_ap_switch(dbus_parameter parameter, DBusConnection *connection,char *stat)
																		/*返回0表示失败，返回1表示成功*/
																        /*返回-1表示input patameter should only be 'enable' or 'disable'*/
																		/*返回-2表示auto_ap_login interface has not set yet*/
																		/*返回-3表示error*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == stat)
		return 0;
	
	int ret;
    int retu = 1;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int dynamic_ap_login_switch = 0;

	
	if (!strcmp(stat,"enable"))
	{
		dynamic_ap_login_switch = 1;	
	}
	else if (!strcmp(stat,"disable"))
	{
		dynamic_ap_login_switch = 0;	
	}
	else
	{
		//vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		retu = -1;
		return retu;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}*/

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SWITCH);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SWITCH);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dynamic_ap_login_switch,
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
	{
		//vty_out(vty,"set wireless-control dynamic_ap_login_switch %s successfully\n",argv[0]);
		retu = 1;
	}				
	else if(ret == AUTO_AP_LOGIN_INTERFACE_NOT_SET)
	{
		//vty_out(vty,"<error> auto_ap_login interface has not set yet\n");
		retu = -2;
	}
	else
	{
		retu = -3;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}


int set_wirelesscontrol_auto_ap_binding_l3_interface(dbus_parameter parameter, DBusConnection *connection,char *state,char* inter) 
																									 /*返回0表示失败，返回1表示成功，返回-1表示interface name is too long,should be no more than 15*/
																									 /*返回-2表示input patameter only with 'uplink'or 'downlink'，返回-3表示auto ap login switch is enable,you should disable it first*/
																									 /*返回-4表示interface argv[1] error, no index or interface down，返回-5表示interface argv[1] is down，返回-6表示interface argv[1] is no flags*/
																									 /*返回-7表示interface argv[1] is no index，返回-8表示interface argv[1] error，返回-9表示interface has be binded in other hansi*/
																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == state)||(NULL == inter))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int len = 0;
	int quitreason = 0;
	char *name;
	unsigned char policy = 2;
	int retu;
    
	
	len = strlen(inter);
	
	if(len > 15)
	{		
		return -1;
	}
	
	if (!strcmp(state,"uplink"))
	{
		policy = 1;	
	}
	else if (!strcmp(state,"downlink"))
	{
		policy = 0;	
	}
	else
	{
		return -2;
	}	
		
	
	name = (char*)malloc(strlen(inter)+1);
	if(NULL == name)
		return 0;
	memset(name, 0, strlen(inter)+1);
	memcpy(name, inter, strlen(inter)); 
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
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
	dbus_message_iter_get_basic(&iter,&quitreason);
	if(ret == 0)
	{
		retu = 1;
	}		
	else if(ret == SWITCH_IS_DISABLE)
	{
		retu=-3;
	}
	else if(ret == APPLY_IF_FAIL)
	{
		retu=-4;
	}
	else if(ret == IF_BINDING_FLAG)
	{
		retu=-9;
	}
	else
	{
		if(quitreason == IF_DOWN)
		{
			retu = -5;
		}
		else if(quitreason == IF_NOFLAGS)
		{
			retu = -6;
		}
		else if(quitreason == IF_NOINDEX)
		{
			retu = -7;
		}
		else
		{
			retu = -8;
		}
	}
		
	FREE_OBJECT(name);

	dbus_message_unref(reply);
	return retu;			
}


int set_wirelesscontrol_auto_ap_binding_wlan(dbus_parameter parameter, DBusConnection *connection,char*wlan_id,char *ifname)  
																								/*返回0表示失败，返回1表示成功，返回-1表示interface name is too long,should be no more than 15*/
																								/*返回-2表示wlanid should be 1 to WLAN_NUM-1，返回-3表示wlan not exist，返回-4表示wlan has not bind interface*/
																								/*返回-5表示 interface not in the auto ap login interface，返回-6表示auto_ap_login interface has not set yet*/
																								/*返回-7表示auto_ap_login interface argv[1] wlan num is already L_BSS_NUM*/
																								/*返回-8表示auto ap login switch is enable,you should disable it first*/
																								/*返回-9表示is no local interface, permission denial，返回-10表示interface error, no index or interface down*/
																								/*返回-11表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == wlan_id)||(NULL == ifname))
		return 0;
	
	int ret,ret1;
     int retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int wlanid = 0;
	int len = 0;
	char *name;
	
	len = strlen(ifname);
	if(len > 15)
	{		
		return -1;
	}
	

	ret1 = parse_int_ID((char*)wlan_id, &wlanid);
	if(ret1 != WID_DBUS_SUCCESS){
		return -2;
	}
	
	name = (char*)malloc(strlen(ifname)+1);
	if(NULL == name)
		return 0;
	memset(name, 0, strlen(ifname)+1);
	memcpy(name, ifname, strlen(ifname)); 
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_WLANID);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_WLANID);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
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
	{
		retu = 1;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		retu = -3;
	}
	else if(ret == Wlan_IF_NOT_BE_BINDED)
	{
		retu = -4;
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_ERROR)
	{
		retu = -5;
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_NOT_SET)
	{
		retu = -6;
	}
	else if(ret == WTP_OVER_MAX_BSS_NUM)
	{
		retu = -7;
	}
	else if(ret == SWITCH_IS_DISABLE)
	{
		retu = -8;
	}
	else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD)
	{
		retu = -9;
	}
	else if(ret == APPLY_IF_FAIL)
	{
		retu = -10;
	}
	else
	{
		retu = -11;
	}
		
	dbus_message_unref(reply);

	FREE_OBJECT(name);

	return retu;			
}


int del_wirelesscontrol_auto_ap_binding_wlan_func(dbus_parameter parameter, DBusConnection *connection,char *wlan_id,char *ifname)  
																										/*返回0表示失败，返回1表示成功，返回-1表示interface name is too long,should be no more than 15*/
																										/*返回-2表示wlanid should be 1 to WLAN_NUM-1，返回-3表示wlan not exist，返回-4表示wlan has not bind interface*/
																										/*返回-5表示interface argv[1] not in the auto ap login interface，返回-6表示auto_ap_login interface has not set yet*/
																										/*返回-7表示auto_ap_login interface argv[1] wlan num is 0，返回-8表示auto ap login switch is enable,you should disable it first*/
																										/*返回-9表示input interface dosen't exist!，返回-10表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == wlan_id)||(NULL == ifname))
		return 0;
	
	int ret,ret1;
	int retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int wlanid = 0;
	int len = 0;
	char *name;
	
	len = strlen(ifname);
	if(len > 15)
	{		
		return -1;
	}
	
	ret1 = parse_int_ID((char*)wlan_id, &wlanid);
	if(ret1 != WID_DBUS_SUCCESS){
		return -2;
	}
	
	name = (char*)malloc(strlen(ifname)+1);
	if(NULL == name)
		return 0;
	memset(name, 0, strlen(ifname)+1);
	memcpy(name, ifname, strlen(ifname)); 
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_DYNAMIC_AP_LOGIN_WLANID);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_DYNAMIC_AP_LOGIN_WLANID);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
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
	{
		retu=1;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		retu=-3;
	}
	else if(ret == Wlan_IF_NOT_BE_BINDED)
	{
		retu=-4;
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_ERROR)
	{
		retu=-5;
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_NOT_SET)
	{
		retu=-6;
	}
	else if(ret == WTP_OVER_MAX_BSS_NUM)
	{
		retu=-7;
	}
	else if(ret == SWITCH_IS_DISABLE)
	{
		retu=-8;
	}
	else if(ret == APPLY_IF_FAIL)
	{
		retu=-9;
	}
	else
	{
		retu=-10;
	}
		
	dbus_message_unref(reply);
	
	FREE_OBJECT(name);

	return retu;			
}



int set_wirelesscontrol_auto_ap_save_config_switch(dbus_parameter parameter, DBusConnection *connection,char *stat)
																					  /*返回0表示失败，返回1表示成功*/
																					  /*返回-1表示input patameter should only be 'enable' or 'disable'*/
																					  /*返回-2表示auto ap login switch is enable,you should disable it first*/
																					  /*返回-3表示error*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == stat)
		return 0;
	
	int ret;
	int retu = 1;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int dynamic_ap_login_save_config_switch = 0;

	
	if (!strcmp(stat,"enable"))
	{
		dynamic_ap_login_save_config_switch = 1;	
	}
	else if (!strcmp(stat,"disable"))
	{
		dynamic_ap_login_save_config_switch = 0;	
	}
	else
	{
		//vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG_SWITCH);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG_SWITCH);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dynamic_ap_login_save_config_switch,
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
	{
		//vty_out(vty,"set wireless-control dynamic_ap_login_save_config_switch %s successfully\n",argv[0]);
		retu = 1;
	}	
	else if(ret == SWITCH_IS_DISABLE)
	{
		retu = -2;
		//vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	}
	else
	{
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -3;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

void Free_auto_ap_config(DCLI_AC_API_GROUP_FIVE *auto_ap_login)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG,auto_ap_login);
			auto_ap_login = NULL;
		}
	}
}

/*返回1时，调用Free_auto_ap_config()释放空间*/
int show_auto_ap_config(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **auto_ap_login)/*返回0表示失败，返回1表示成功*/
																														 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = 0;
	int retu;
	

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int,
					DBusConnection *,
					char *
					);

	*auto_ap_login = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*auto_ap_login = (*dcli_init_func)
				(
					parameter.instance_id,
					THIRD,/*"show auto_ap_login config"*/
					0,
					0,
					1,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG
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
	else if(*auto_ap_login)
	{	
		retu = 1;
	}

	return retu;			
}

/*未使用*/
int set_ap_timestamp(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu = 1;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int timestamp = 0;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_TIMESTAMP);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_TIMESTAMP);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&timestamp,
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
	{
		//vty_out(vty," set ap timestamp successfully\n");
		retu = 1;
		
	}				
	else
	{
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -1;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}



//new function add by wangpeng
/*未使用*/
int set_monitor_time(dbus_parameter parameter, DBusConnection *connection,char * mon_time)/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示error*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;
	
	if(NULL == mon_time)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned int time;
	int retu;

	ret = parse_int_ID((char*)mon_time, &time);
	if(ret != WID_DBUS_SUCCESS)
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_TIME);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_TIME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
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
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		retu = 1;
		//vty_out(vty,"set monitor time %s successfully\n",argv[0]);
	}
	else 
	{
		retu = -2;
		//vty_out(vty,"<error> %d\n",ret);
	}
	
	return retu;
}

int set_sample_time(dbus_parameter parameter, DBusConnection *connection,char * sample_time)/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;
	
	if(NULL == sample_time)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned int time;
	int retu;

	ret = parse_int_ID((char*)sample_time, &time);
	if(ret != WID_DBUS_SUCCESS)
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_TIME);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_TIME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		retu = 1;
	}
	else 
	{
		retu = -2;
	}
	return retu;
}

void Free_sample_info(DCLI_AC_API_GROUP_FIVE *sample_info)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO,sample_info);
			sample_info = NULL;
		}
	}
}

/*返回1时，调用Free_qos_head()释放空间*/
int show_sample_info(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **sample_info)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;

	int ret = WID_DBUS_SUCCESS;
	int retu;


	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int,
					DBusConnection *,
					char *
					);

	*sample_info = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*sample_info = (*dcli_init_func)
				(
					parameter.instance_id,
					SIXTH,/*"show sample infomation"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO
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
	else if((ret == 0)&&(*sample_info))
	{
		retu = 1;
	}
	else 
	{
		retu = -1;
	}

	return retu;
}

/*未使用*/
int set_monitor_enable(dbus_parameter parameter, DBusConnection *connection,char * able)
													  /*返回0表示失败，返回1表示成功*/
													  /*返回-1表示input patameter only with 'enable' or 'disable'*/
													  /*返回-2表示error*/
													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == able)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

   int policy = 0;

	
	if (!strcmp(able,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(able,"disable"))
	{
		policy = 0;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_ENABLE);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_ENABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*未使用*/
int set_sample_enable(dbus_parameter parameter, DBusConnection *connection,char * able)/*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示input patameter only with 'enable' or 'disable'*/
																						  /*返回-2表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == able)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

    int policy = 0;

	
	if (!strcmp(able,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(able,"disable"))
	{
		policy = 0;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_ENABLE);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_ENABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

void Free_ap_txpower_control(DCLI_AC_API_GROUP_FIVE *tx_control)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL,tx_control);
			tx_control = NULL;
		}
	}
}

/*返回1时，调用Free_ap_txpower_control()释放空间*/
int show_ap_txpower_control(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **tx_control)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																														   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int,
					DBusConnection *,
					char *
					);

	*tx_control = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*tx_control = (*dcli_init_func)
				(
					parameter.instance_id,
					FOURTH,/*"show ap txpower control"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL
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
	else if((ret == 0)&&(*tx_control))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
	}
	
	return retu;	
}

/*未使用*/
int set_receiver_signal_level(dbus_parameter parameter, DBusConnection *connection,int level_num)/*返回0表示失败，返回1表示成功*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int level = 0;
	int ret = WID_DBUS_SUCCESS;
	int retu;
	
	level = level_num;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_RECEIVER_SIGNAL_LEVEL);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_RECEIVER_SIGNAL_LEVEL);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&level,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if(NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{
		retu = 1;
	}
	return retu;
}

void Free_receiver_signal_level(DCLI_AC_API_GROUP_FIVE *receiver_sig_lev)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");		
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL,receiver_sig_lev);
			receiver_sig_lev = NULL;
		}
	}
}

/*返回1时，调用Free_receiver_signal_level()释放空间*/
int show_receiver_signal_level(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **receiver_sig_lev)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;

	*receiver_sig_lev = NULL;
	CW_CREATE_OBJECT_ERR((*receiver_sig_lev), DCLI_AC_API_GROUP_FIVE, return -1;); 
	(*receiver_sig_lev)->num = 0;
	int ret = 0;
	int retu;
	

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*receiver_sig_lev = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*receiver_sig_lev = (*dcli_init_func)
				(
					parameter.instance_id,
					FIFTH,/*"show receiver signal level"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL
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
	else if((ret == 0)&&(*receiver_sig_lev))
	{
		retu = 1;
	}
	else 
	{
		retu = -1;
	}

	return retu;
}

int clear_auto_ap_config_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = 0,retu;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WID_DYNAMIC_AP_LOGIN_CONFIG);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WID_DYNAMIC_AP_LOGIN_CONFIG);*/
	dbus_error_init(&err);

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

	dbus_message_unref(reply);
	return retu;			
}

/*able==1表示"enable",able == 0表示"disable"*/
int set_ap_statistics(dbus_parameter parameter, DBusConnection *connection,int able)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	int policy = 0;
		
	policy = able;	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if(NULL == reply)
	{
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		//vty_out(vty," set ap statistics %s successfully\n",argv[0]);
		retu = 1;
	}				
	else
	{
		//vty_out(vty,"<error>  %d\n",ret);
		retu = -1;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}


void Free_attack_mac_list_head(DCLI_AC_API_GROUP_ONE  *LIST)
{
  	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_ONE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW,LIST);
			LIST = NULL;
		}
	}
}


/*返回1时，调用Free_attack_mac_list_head()释放空间*/
int show_attack_mac_list(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE  **LIST)/*返回0表示失败，返回1表示成功，返回-1表示there is no attack mac list，返回-2表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;

	void*(*dcli_init_func)(
						int ,
						unsigned int ,
						unsigned int ,
						unsigned int ,
						unsigned int* ,
						unsigned int* ,
						unsigned int* ,
						DBusConnection *,
						char *
						);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_one");
		if(NULL != dcli_init_func)
		{
			*LIST =(*dcli_init_func)	
					(
						parameter.instance_id,
						0,/*"show attack ap mac list"*/
						0,
						0,
						&ret,
						0,
						&(parameter.local_id),
						connection,
						WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW
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
	else if((ret == 0)&&(*LIST)){	
		retu = 1;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		retu = -1;
	}
	else
	{
		retu = -2;
	}
	
	return retu;		
}

void Free_legal_essid_list_head(DCLI_AC_API_GROUP_ONE *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_ONE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_ESSID_SHOW,LIST);
			LIST = NULL;
		}
	}
}


/*返回1时，调用Free_legal_essid_list_head()释放空间*/
int show_legal_essid_list_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE **LIST)/*返回0表示失败，返回1表示成功，返回-1表示there is no legal essid list，返回-2表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;	
	
	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_one");
		if(NULL != dcli_init_func)
		{
			*LIST =(*dcli_init_func)	
					(
						parameter.instance_id,
						0,/*"show legal essid list "*/
						0,
						0,
						&ret,
						0,
						&(parameter.local_id),
						connection,
						WID_DBUS_CONF_METHOD_ESSID_SHOW
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
	else if((ret == 0)&&(*LIST))
	{	
		retu = 1;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		retu = -1;
	}
	else
	{
		retu = -2;
	}		
	
	return retu;				
}


void Free_manufacturer_oui_list_head(DCLI_AC_API_GROUP_ONE *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_ONE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_OUI_SHOW,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_manufacturer_oui_list_head()释放空间*/
int show_manufacturer_oui_list(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE **LIST)/*返回0表示失败，返回1表示成功，返回-1表示there is no legal manufacturer list，返回-2表示error*/
																													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;

	*LIST = NULL;
	CW_CREATE_OBJECT_ERR(*LIST, DCLI_AC_API_GROUP_ONE, return -1;);	
	(*LIST)->dcli_attack_mac_list = NULL;
	(*LIST)->dcli_essid_list = NULL;
	(*LIST)->dcli_oui_list = NULL;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_one");
		if(NULL != dcli_init_func)
		{
			*LIST =(*dcli_init_func)	
					(
						parameter.instance_id,
						0,/*"show legal manufacturer list "*/
						0,
						0,
						&ret,
						0,
						&(parameter.local_id),
						connection,
						WID_DBUS_CONF_METHOD_OUI_SHOW
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
	else if((ret == 0)&&(*LIST))
	{	
		retu = 1;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		retu = -1;
	}
	else
	{
		retu = -2;
	}
	
	return retu;				
}


int add_legal_manufacturer_func(dbus_parameter parameter, DBusConnection *connection,char *OUI)/*返回0表示失败，返回1表示成功，返回-1表示Unknown OUI format，返回-2表示error*/
																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == OUI)
		return 0;
	
	int ret,retu;
	OUI_S ouiElem; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_OUI((char*)OUI,&ouiElem);
	if (ret<0) {
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_OUI);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_OUI);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &ouiElem.oui[0],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[1],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[2],
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);

	return retu;	
}

int add_legal_essid_func(dbus_parameter parameter, DBusConnection *connection,char *ESSID)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																							 /*返回-2表示essid is too long,out of the limit of 32*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == ESSID)
		return 0;

	if (strlen(ESSID) > 32)
	{
		return -2;
	}
	
	int ret,retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	

	char *essid=(char *)malloc(strlen(ESSID)+1);
	if(NULL == essid)
		return 0;
	memset(essid,0,strlen(ESSID)+1);
	memcpy(essid,ESSID,strlen(ESSID));	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_ESSID);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_ESSID );*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

	dbus_message_unref(query);

	FREE_OBJECT(essid);

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
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);

	return retu;	
}

int add_attack_ap_mac_func(dbus_parameter parameter, DBusConnection *connection,char *MAC)/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == MAC)
		return 0;
	
	int ret,retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)MAC,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_ATTACK_MAC);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_ATTACK_MAC);*/

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);

	return retu;			
}


int del_legal_manufacturer_func(dbus_parameter parameter, DBusConnection *connection,char *OUI)/*返回0表示失败，返回1表示成功，返回-1表示Unknown OUI format，返回-2表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == OUI)
		return 0;
	
	int ret,retu;
	OUI_S ouiElem; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_OUI((char*)OUI,&ouiElem);
	if (ret<0) {
		return -1;
	}	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_OUI);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_OUI);*/
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &ouiElem.oui[0],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[1],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[2],
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);

	return retu;	
}


int del_legal_essid_func(dbus_parameter parameter, DBusConnection *connection,char *ESSID)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																							/*返回-2表示essid is too long,out of the limit of 32*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == ESSID)
		return 0;

	if (strlen(ESSID) > 32)
	{
		return -2;
	}
	
	int ret,retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	

	char *essid=(char *)malloc(strlen(ESSID)+1);
	if(NULL == essid)
		return 0;
	memset(essid,0,strlen(ESSID)+1);
	memcpy(essid,ESSID,strlen(ESSID));	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_ESSID);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_ESSID );*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

	dbus_message_unref(query);

	FREE_OBJECT(essid);

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
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);

	return retu;	
}

int del_attack_ap_mac_func(dbus_parameter parameter, DBusConnection *connection,char *MAC)/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == MAC)
		return 0;
	
	int ret,retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)MAC,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_ATTACK_MAC);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_ATTACK_MAC);*/

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);

	return retu;			
}

#if 0
 // Moved to dcli_wsm.c 
int set_ipfwd_func(char *state)/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
{
	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(state,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(state,"disable"))
	{
		policy = 0;	
	}
	else
	{
		return -1;
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
	index = 0;
	ccgi_ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_IPFWD);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_IPFWD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);
	return retu;			
}
#endif


void Free_ApMode_head(DCLI_AC_API_GROUP_FOUR *conf_info)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FOUR *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_four");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST,conf_info);
			conf_info = NULL;
		}
	}
}

/*未使用*/
/*返回1时，调用Free_ApMode_head()释放空间*/
int show_model_list_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FOUR **conf_info)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret=0;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int,
					DBusConnection *,
					char *
					);

	*conf_info = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_four");
		if(NULL != dcli_init_func)
		{
			*conf_info = (*dcli_init_func)
				(
					parameter.instance_id,
					SECOND,/*"show model (list|all)"*/
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST
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
	else if((ret == 0)&&(*conf_info))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
 	}

	return retu;
}

void Free_model_cmd(DCLI_AC_API_GROUP_FOUR *modelinfo)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FOUR *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_four");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_MODEL,modelinfo);
			modelinfo = NULL;
		}
	}
}

/*未使用*/
/*返回1时，调用Free_model_cmd()释放空间*/
int show_model_cmd(dbus_parameter parameter, DBusConnection *connection,char *mode,DCLI_AC_API_GROUP_FOUR **modelinfo)/*返回0表示失败，返回1表示成功，返回-1表示this model doesn't exist*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == mode )
	{
		*modelinfo = NULL;
		return 0;
	}
	
	char *model = NULL;	
	int ret;
	int retu;

	model = (char*)malloc(strlen(mode)+1);
	if(NULL == model)
		return 0;
	memset(model, 0, strlen(mode)+1);
	memcpy(model, mode, strlen(mode));			

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*modelinfo = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_four");
		if(NULL != dcli_init_func)
		{
			*modelinfo = (*dcli_init_func)
				(
					parameter.instance_id,
					THIRD,/*"show model .MODEL"*/
					0,
					0,
					&ret,
					model,/* char *char1 */
					0,/* char *char2  */
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_MODEL
				);
		}
		else
		{
			free(model);
			return 0;
		}
	}
	else
	{
		free(model);
		return 0;
	}
	
	if(ret == -1)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if((ret == 0)&&(*modelinfo))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
	}
	
	return retu;	
}


int set_model_cmd(dbus_parameter parameter, DBusConnection *connection,char *mode,char *new_mode)/*返回0表示失败，返回1表示成功，返回-1表示new model is not configuration please change other name，返回-2表示this model doesn't exist*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == mode)||(NULL == new_mode))
		return 0;
	
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	char *oldmodel = NULL;
	char *newmodel = NULL;
	int ret,retu;

	oldmodel = (char*)malloc(strlen(mode)+1);
	if(NULL == oldmodel)
		return 0;
	memset(oldmodel, 0, strlen(mode)+1);
	memcpy(oldmodel, mode, strlen(mode));	
	
	newmodel = (char*)malloc(strlen(new_mode)+1);
	if(NULL == newmodel){
		if(NULL!=oldmodel){free(oldmodel);oldmodel=NULL;}
		return 0;
	}
	memset(newmodel, 0, strlen(new_mode)+1);
	memcpy(newmodel, new_mode, strlen(new_mode));	


	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_MODEL);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_MODEL);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
						DBUS_TYPE_STRING,&oldmodel,
						DBUS_TYPE_STRING,&newmodel,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		if(oldmodel)
		{	
			free(oldmodel);
			oldmodel = NULL;
		}
		if(newmodel)
		{	
			free(newmodel);
			newmodel = NULL;
		}
		return SNMPD_CONNECTION_ERROR;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0 )
	{
		retu=1;
	}	
	else if(ret == MODEL_NO_CONFIG)
	{
		retu=-1;
	}
	else
	{
		retu=-2;
	}
	
	if(oldmodel)
	{	
		free(oldmodel);
		oldmodel = NULL;
	}
	if(newmodel)
	{	
		free(newmodel);
		newmodel = NULL;
	}
	dbus_message_unref(reply);	

	return retu;

}

void Free_ap_model_code(DCLI_AC_API_GROUP_FOUR *codeinfo)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FOUR *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_four");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION,codeinfo);
			codeinfo = NULL;
		}
	}
}

/*返回1时，调用Free_ap_model_code()释放空间*/
int show_ap_model_code_func(dbus_parameter parameter, DBusConnection *connection,char *mode,DCLI_AC_API_GROUP_FOUR **codeinfo)/*返回0表示失败，返回1表示成功，返回-1表示this model doesn't exist*/
																																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if(NULL == mode)
	{
		*codeinfo = NULL;
		return 0;
	}
	
	char *model = NULL;	
	int ret=0;
	int retu;

	model = (char*)malloc(strlen(mode)+1);
	if(NULL == model)
		return 0;
	memset(model, 0, strlen(mode)+1);
	memcpy(model, mode, strlen(mode));	
	

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

    *codeinfo = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_four");
		if(NULL != dcli_init_func && connection)
		{
			*codeinfo = (*dcli_init_func)
				(
					parameter.instance_id,
					FOURTH,/*"show ap model-code-information .MODEL"*/
					0,
					0,
					&ret,
					model,/*char1*/
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION
				);
		}
		else
		{
			if(NULL!=model){free(model);model=NULL;}
			return 0;
		}
	}
	else
	{
		if(NULL!=model){free(model);model=NULL;}
		return 0;
	}

	if (WID_DBUS_ERROR == ret) {
	    retu = 3;    
    }
	else if((ret == 0)&&(*codeinfo))
	{
		retu = 1;
	}
	else if(ret == -1)
	{
	    retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -1;
	}

	FREE_OBJECT(model);
	return retu;
}


void Free_rogue_ap_list_v1_head(DCLI_AC_API_GROUP_TWO *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_rogue_ap_list_v1_head()释放空间*/
int show_rogue_ap_list_v1_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **LIST)/*返回0表示失败，返回1表示成功，返回-1表示radio resource managment is disable please enable first，返回-2表示good luck there is no rouge ap*/
																													   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)
				(
					parameter.instance_id,
					SECOND,/*"show rogue ap list v1"*/
					0,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1
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
	else if((ret == 0)&&(*LIST))
	{	
		retu = 1;
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		retu = -1;
	}
	else
	{
		retu = -2;
	}
	
	return retu;	
}

int set_txpower_threshold_cmd(dbus_parameter parameter, DBusConnection *connection,char *value)/*返回0表示失败，返回1表示成功，返回-1表示input patameter should be 20 to 35，返回-2表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == value)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int txpower_threshold = 0;
	int ret,retu;
	
	txpower_threshold = atoi(value);
	
	if (txpower_threshold < 20 || txpower_threshold > 35)
	{	
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TXPOWER_THRESHOLD);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TXPOWER_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&txpower_threshold,
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);
	
	return retu;	
	
}


int set_coverage_threshold_cmd(dbus_parameter parameter, DBusConnection *connection,char *value)/*返回0表示失败，返回1表示成功，返回-1表示input patameter should be 5 to 15，返回-2表示error*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == value)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int coverage_threshold = 0;
	int ret,retu;
	
	coverage_threshold = atoi(value);
	
	if (coverage_threshold < 5 || coverage_threshold > 15)
	{	
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_COVERAGE_THRESHOLD);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_COVERAGE_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&coverage_threshold,
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
	{
		retu=1;
	}				
	else
	{
		retu=-2;
	}
		
	dbus_message_unref(reply);
	
	return retu;	
	
}

void Free_neighbor_rssi_info_bywtpid(DCLI_AC_API_GROUP_THREE *RSSI)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_THREE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_three");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO,RSSI);
			RSSI = NULL;
		}
	}
}

/*未使用*/
/*返回1时，调用Free_neighbor_rssi_info_bywtpid()释放空间*/
int show_neighbor_rssi_info_bywtpid_cmd(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_AC_API_GROUP_THREE **RSSI)
																			/*返回0表示失败，返回1表示成功，返回-1表示wtp id should be 1 to WTP_NUM-1*/
																			/*返回-2表示wtp does not exist，返回-3表示radio resource managment is disable please enable first*/
																			/*返回-4表示transmit power control is disable please enable first，返回-5表示there is no neighbor ap*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;
	int wtpid = 0;
	
	wtpid = wtp_id;	
	if ((wtpid <= 0) || (wtpid >= WTP_NUM))
	{
		return -1;
	}	

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*RSSI = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_three");
		if(NULL != dcli_init_func)
		{
			*RSSI =(*dcli_init_func)
				  (
					parameter.instance_id,
					FIFTH,/*"show neighbor rssi bywtpid ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO
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
	else if((ret == 0)&&(*RSSI))
	{	
		retu = 1;			
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		retu = -3;
	}
	else if(ret == WID_TRANSMIT_POWER_CONTROL_DISABLE)
	{
		retu = -4;
	}	
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else
	{
		retu = -5;
	}
		
	return retu;		
}


int set_ap_cm_threshold_func(dbus_parameter parameter, DBusConnection *connection,char *thr_type,char *thr_value)  
																					/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'cpu','temperature'or 'memoryuse'*/
																					/*返回-2表示unknown id format，返回-3表示ap cpu threshold parameters error，返回-4表示ap memory use threshold parameters error*/
																					/*返回-5表示wtp id does not run，返回-6表示error*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == thr_type)||(NULL == thr_value))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int value;
	int ret = WID_DBUS_SUCCESS;
	int retu;
    int policy = 0;

	
	if (!strcmp(thr_type,"cpu"))
	{
		policy = 1;	
	}
	else if (!strcmp(thr_type,"memoryuse"))
	{
		policy = 2;	
	}
	else if (!strcmp(thr_type,"temperature"))
	{
		policy = 3;	
	}
	else
	{
		return -1;
	}

	ret = parse_int_ID((char*)thr_value, &value);
	if(ret != WID_DBUS_SUCCESS){
			return -2;
	}
	if(policy == 1)
	{
		if(value > 9999 || value == 0){
			return -3;
		}
	}
	else if((policy == 2)||(policy == 3))
	{
		if(value > 99 || value == 0){
			return -4;
		}
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_CM_THRESHOLD);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_CM_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_UINT32,&value,
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
	{
		retu=1;
	}	
	else if (ret == WTP_NOT_IN_RUN_STATE)
	{
		retu=-5;
	}
	else
	{
		retu=-6;
	}
		
	dbus_message_unref(reply);
	return retu;			
}

void Free_ap_threshold(DCLI_AC_API_GROUP_FIVE *ap_threshold)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD,ap_threshold);
			ap_threshold = NULL;
		}
	}
}

/*返回1时，调用Free_ap_threshold()释放空间*/
int show_ap_threshold_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **ap_threshold)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;

	int ret = WID_DBUS_SUCCESS;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int,
					DBusConnection *,
					char *
					);

	*ap_threshold = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*ap_threshold = (*dcli_init_func)
				(
					parameter.instance_id,
					SEVENTH,/*"show ap threshold"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD
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
	else if((ret == 0)&&(*ap_threshold))
	{
		retu = 1;
	}
	else if(ret == WID_DBUS_ERROR) 
	{
		retu = -1;
	}

	return retu;
}

void Free_ap_rrm_config(DCLI_AC_API_GROUP_FIVE *resource_mg)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG,resource_mg);
			resource_mg = NULL;
		}
	}
}

/*返回1时，调用Free_ap_rrm_config()释放空间*/
int show_ap_rrm_config_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **resource_mg)/*返回0表示失败，返回1表示成功 ，返回-1表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*resource_mg = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*resource_mg = (*dcli_init_func)
				(
					parameter.instance_id,
					EIGHTH,/*"show radio resource management configuration"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG
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
	else if((ret == 0)&&(*resource_mg))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
	}

	return retu;			
}

#if 0
 // Moved to dcli_wsm.c 
int show_ipfwd_func(int *ipfwd_state)	/*ipfwd_state==0表示disable，ipfwd_state==1表示enable*/
											/*返回0表示失败，返回1表示成功，返回-1表示unexpected flow-based-forwarding state*/
{
	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int state = 0;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}*/
	index = 0;
	ccgi_ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_IPFWD);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_IPFWD);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection, query, -1, &err);
	
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

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&state);
		

	*ipfwd_state=state;

	if(0 == state)
		retu=1;
	else if(1 == state)
		retu=1;
	else
		retu=-1;

	dbus_message_unref(reply);
	return retu;
}
#endif

void Free_wids_device_head(DCLI_AC_API_GROUP_TWO *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_wids_device_head()释放空间*/
int show_wids_device_list_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **LIST)/*返回0表示失败，返回1表示成功，返回-1表示there is no wids device，返回-2表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;
	int wtpid = 0;
	
	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*LIST = (*dcli_init_func)
				(
					parameter.instance_id,
					FIFTH,/*"show wids device list"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST
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
	else if((ret == 0)&&(*LIST))
	{	
		retu = 1;
	}
	else if(ret == NO_WIDS_DEVICE)
	{
		retu = -1;
	}
	else
	{
		retu = -2;
	}

	return retu; 		
}

void Free_wids_device_list_bywtpid_head(DCLI_AC_API_GROUP_TWO *LIST)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_TWO *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_two");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID,LIST);
			LIST = NULL;
		}
	}
}

/*返回1时，调用Free_wids_device_list_bywtpid_head()释放空间*/
int show_wids_device_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID,DCLI_AC_API_GROUP_TWO **LIST)
																		 /*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																		 /*返回-2表示wtp does not exist，返回-3表示there is no wids device，返回-4表示error*/
																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == WtpID)
	{
		*LIST = NULL;
		return 0;
	}
	
	int ret;
	int retu;
	int wtpid = 0;

	
	wtpid = atoi(WtpID);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
	}

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*LIST = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_two");
		if(NULL != dcli_init_func)
		{
			*LIST =(*dcli_init_func)
				  (
					parameter.instance_id,
					SIXTH,/*"show wids device list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID
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
	else if((ret == 0)&&(*LIST))
	{
		retu = 1;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	
	else if(ret == NO_WIDS_DEVICE)
	{
		retu = -3;
	}
	else
	{
		retu = -4;
	}
		
	return retu; 	
}

int clear_wids_device_list_cmd_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
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


	if(ret == 0 )
		retu=1;
	else
		retu=-1;
		
	dbus_message_unref(reply);	
	
	return retu; 		
}

/*未使用*/
int clear_wids_device_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID)
																				/*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																				/*返回-2表示wtp does not exist，返回-3表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == WtpID)
		return 0;
	
	int ret,retu;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int state = 1;
	int wtpid = 0;

	wtpid = atoi(WtpID);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
		}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST_BYWTPID);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
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


	if(ret == 0 )
		retu=1;
	else if (ret == WTP_ID_NOT_EXIST)
		retu=-2;
	else if(ret == WTP_ID_LARGE_THAN_MAX)
		retu=-1;
	else
		retu=-3;
		
	dbus_message_unref(reply);	
	
	return retu; 		
}

void Free_wids_statistics_list_bywtpid_head(DCLI_AC_API_GROUP_THREE *widsstatis)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_THREE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_three");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID,widsstatis);
			widsstatis = NULL;
		}
	}
}

/*返回1时，调用Free_wids_statistics_list_bywtpid_head()释放空间*/
int show_wids_statistics_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID,DCLI_AC_API_GROUP_THREE **widsstatis)
																				/*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																			    /*返回-2表示wtp does not exist，返回-3表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == WtpID)
	{
		*widsstatis = NULL;
		return 0;
	}
	
	int ret;
	int retu;
	int wtpid = 0;
	
	*widsstatis = NULL;
	CW_CREATE_OBJECT_ERR(*widsstatis, DCLI_AC_API_GROUP_THREE, return -1;);	
	(*widsstatis)->WTPIP = NULL;

	wtpid = atoi(WtpID);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
		}


	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_three");
		if(NULL != dcli_init_func)
		{
			*widsstatis =(*dcli_init_func)
				  (
					parameter.instance_id,
					THIRD,/*"show wids statistics bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID
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
	else if((ret == 0)&&(*widsstatis))
	{
		retu = 1;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		retu = -2;
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
	else
	{
		retu = -3;
	}
		
	return retu;			
}

void Free_wids_statistics_list_head(DCLI_AC_API_GROUP_THREE *widsstatis)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_THREE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_three");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST,widsstatis);
			widsstatis = NULL;
		}
	}
}

/*返回1时，调用Free_wids_statistics_list_head()释放空间*/
int show_wids_statistics_list_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_THREE **widsstatis)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;
	int wtpid = 0;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

	*widsstatis = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_three");
		if(NULL != dcli_init_func)
		{
			*widsstatis =(*dcli_init_func)
				  (
					parameter.instance_id,
					FOURTH,/*"show wids statistics"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST
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
	else if((ret == 0)&&(*widsstatis))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
	}
		
	return retu;
}

int clear_wids_statistics_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID)
																				  /*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																				  /*返回-2表示wtp does not exist，返回-3表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == WtpID)
		return 0;
	
	int ret,retu;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int state = 1;
	int wtpid = 0;

	wtpid = atoi(WtpID);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
		}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST_BYWTPID);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
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


	if(ret == 0 )
		retu=1;
	else if (ret == WTP_ID_NOT_EXIST)
		retu=-2;
	else if(ret == WTP_ID_LARGE_THAN_MAX)
		retu=-1;
	else
		retu=-3;
		
	dbus_message_unref(reply);	
	
	return retu; 		
}

int clear_wids_statistics_list_cmd_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
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


	if(ret == 0 )
		retu=1;
	else
		retu=-1;
		
	dbus_message_unref(reply);	
	
	return retu; 		
}


int set_neighbordead_interval_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *value)
																		  /*返回0表示失败，返回1表示成功*/
																		  /*返回-1表示input patameter should be 20 to 2000，返回-2表示error*/
																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == value)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int neighbordead_interval = 0;
	int ret,retu;
	
	neighbordead_interval = atoi(value);
	
	if (neighbordead_interval < 20 || neighbordead_interval > 2000)
	{	
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_NEIGHBORDEAD_INTERVAL);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_NEIGHBORDEAD_INTERVAL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&neighbordead_interval,
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
	else
		retu=-2;
		
	dbus_message_unref(reply);
	
	return retu;	
	
}

void Free_neighbordead_interval(DCLI_AC_API_GROUP_FIVE *interval)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL,interval);
			interval = NULL;
		}
	}
}

/*返回1时，调用Free_neighbordead_interval()释放空间*/
int show_neighbordead_interval_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **interval)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = -1;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int,
					DBusConnection *,
					char *
					);

	*interval = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*interval = (*dcli_init_func)
				(
					parameter.instance_id,
					NINTH,/*"show neighbordead interval"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL
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
	else if((ret == 0)&&(*interval))
	{
		retu = 1;
	}
	else 
	{
		retu = -1;
	}

	return retu;	
}

/*未使用*/
int update_bak_ac_config_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_BAK_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_UPDATE_BAK_INFO);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_UPDATE_BAK_INFO);*/
	dbus_error_init(&err);

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
	dbus_message_unref(reply);	
	if(ret == 0)
		retu=1;

	return retu;			
}

/*未使用*/
int synchronize_wsm_table_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_BAK_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_SYNCHRONIZE_INFO);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_SYNCHRONIZE_INFO);*/
	dbus_error_init(&err);

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
	dbus_message_unref(reply);	
	if(ret == 0)
		retu=1;

	return retu;			
}

/*未使用*/
int notice_vrrp_state_func(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_BAK_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_NOTICE_INFO);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_NOTICE_INFO);*/
	dbus_error_init(&err);

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
	dbus_message_unref(reply);	
	if(ret == 0)
		retu=1;

	return retu;			
}

/*para的范围是1-5，单位是second*/
int set_wtp_wids_interval_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *para)
																	/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																	/*返回-2表示wtp wids interval error,should be 1 to 5 second*/
																	/*返回-3表示wids switch is enable，返回-4表示error*/
																	/*返回-5表示illegal input:Input exceeds the maximum value of the parameter type*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == para)
		return 0;
	
	int ret = 0,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char interval = 0;
	
	ret = parse_char_ID((char*)para, &interval);
	if(ret != WID_DBUS_SUCCESS){			
            if(ret == WID_ILLEGAL_INPUT){
				retu = -5;
            }
			else{
				retu = -1;
			}
			return retu;
	}	
	if(interval > 5 || interval == 0){
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_INTERVAL);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_INTERVAL);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,	
							 DBUS_TYPE_BYTE,&interval,
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
	else if (ret == SWITCH_IS_DISABLE)
		retu=-3;
	else
		retu=-4;
		
	dbus_message_unref(reply);
	
	return retu;			
}

/*policy_type为"probe"或"other"*/
/*para的范围是1-100*/
int set_wtp_wids_threshold_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *policy_type,char *para)
																						 /*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'probe' or 'other'*/
																						 /*返回-2表示unknown id format，返回-3表示wtp wids threshold error,should be 1 to 100*/
																						 /*返回-4表示wids switch is enable，返回-5表示error，返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == policy_type)||(NULL == para))
		return 0;
	
	int ret = 0,retu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char threshold = 0;
	unsigned char policy = 2;

	if (!strcmp(policy_type,"probe"))
	{
		policy = 0;	
	}		
	else if (!strcmp(policy_type,"other"))
	{
		policy = 1;
	}
	else
	{
		return -1;
	}
	
	ret = parse_char_ID((char*)para, &threshold);
	if(ret != WID_DBUS_SUCCESS){			
            if(ret == WID_ILLEGAL_INPUT){
				retu = -6;
            }
			else{
				retu = -2;
			}
			return retu;
	}	
	if(threshold > 100 || threshold == 0){
		return -3;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_THRESHOLD);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_THRESHOLD);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_BYTE,&threshold,
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
	else if (ret == SWITCH_IS_DISABLE)
		retu=-4;
	else
		retu=-5;

		
	dbus_message_unref(reply);
	
	return retu;			
}


/*para的范围是1-36000*/
int set_wtp_wids_lasttime_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *para)
																	 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																	 /*返回-2表示wtp wids lasttime in black error,should be 1 to 36000*/
																	 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == para)
		return 0;
	
	int ret = 0,retu;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int time = 0;

	ret = parse_int_ID((char*)para, &time);
	if(ret != WID_DBUS_SUCCESS){
			return -1;
	}	
	if(time > 36000 || time == 0){
		return -2;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_LASTTIME_IN_BLACK);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_LASTTIME_IN_BLACK);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
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
		
	dbus_message_unref(reply);
	
	return retu;			
}

/*level的范围是0-25*/
int set_wid_trap_open_func(dbus_parameter parameter, DBusConnection *connection,char *level)/*返回1表示成功，返回0表示失败，返回-1表示error*/
																								 /*返回-2表示trap level should be 0-25*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == level)
		return 0;
	
	int ret,ret2,retu;
	DBusMessage *query, *reply, *query2,*reply2;	
	DBusMessageIter	 iter,iter2;
	DBusError err,err2;
    unsigned char trapflag = 0;

	trapflag = (unsigned char)atoi(level);
	if((trapflag < 0) || (trapflag > 25))
	{
		return -2;
	}
	/*if (!strcmp(state,"open"))
	{
		trapflag = 1;	
	}
	else if (!strcmp(state,"close"))
	{
		trapflag = 0;	
	}
	else
	{
		return -1;
	}*/
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRAP_DEBUG_OPEN);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRAP_DEBUG_OPEN);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&trapflag,
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
	{
		retu=1;
	}				
	else
	{
		retu=-1;
	}
		
	dbus_message_unref(reply);
	
	if(ret == 0)
	{	
	/*	int index;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		char METHOD[PATH_LEN];	
		if(vty->node == CONFIG_NODE){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = vty->index;
		}*/
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);
		ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
		ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_TRAP_OPEN);

		/*query2 = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_TRAP_OPEN);*/
		dbus_error_init(&err2);
		dbus_message_append_args(query2,
						 DBUS_TYPE_BYTE,&trapflag,
						 DBUS_TYPE_INVALID);
		reply2 = dbus_connection_send_with_reply_and_block (connection,query2,-1, &err2);	
		dbus_message_unref(query2);
		if (NULL == reply2) {
			if (dbus_error_is_set(&err2)) {
				dbus_error_free(&err2);
			}
			return SNMPD_CONNECTION_ERROR;
		}
		
		dbus_message_iter_init(reply2,&iter2);
		dbus_message_iter_get_basic(&iter2,&ret2);
		dbus_message_unref(reply2);
		if(ret2 != ASD_DBUS_SUCCESS){
			return 0;		
		}	
	}
	
	return retu;			
}

void Free_rogue_ap_trap_threshold(DCLI_AC_API_GROUP_FIVE *rogue_trap)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD,rogue_trap);
			rogue_trap = NULL;
		}
	}
}

/*返回1时，调用Free_rogue_ap_trap_threshold()释放空间*/
int show_rogue_ap_trap_threshold_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **rogue_trap)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;
	
	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*rogue_trap = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*rogue_trap = (*dcli_init_func)
				(
					parameter.instance_id,
					SECOND,/*"show rogue ap trap threshold"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD
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
	else if((ret == 0)&&(*rogue_trap))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
	}
	
	return retu;			
}

/*value的范围是1-200*/
int set_rogue_ap_trap_threshold_func(dbus_parameter parameter, DBusConnection *connection,char *value)
																	   /*返回0表示失败，返回1表示成功，返回-1表示input patameter should be 1 to 200*/
																	   /*返回-2表示radio resource managment is disable please enable first，返回-3表示error*/
																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == value)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int rogue_ap_threshold = 0;
	int ret;
	int retu;
	
	rogue_ap_threshold = atoi(value);
	
	if (rogue_ap_threshold < 1 || rogue_ap_threshold > 200)
	{	
		//vty_out(vty,"<error> input patameter should be 1 to 200\n");
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_TRAP_THRESHOLD);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_TRAP_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&rogue_ap_threshold,
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
	{
		retu = 1;
		//vty_out(vty,"set rogue ap trap threshold %s successfully\n",argv[0]);
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		retu = -2;
		//vty_out(vty,"radio resource managment is disable please enable first\n");
	}
	else
	{
		retu = -3;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return retu;	
	
}

/*未使用*/
/*type为"add"或"del"*/
int set_wirelesscontrol_auto_ap_binding_l3_interface_new_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *ifname)
																												/*返回0表示失败，返回1表示成功*/
																												/*返回-1表示interface name is too long,should be no more than 15*/
																												/*返回-2表示input patameter only with 'add'or 'del'*/
																												/*返回-3表示auto ap login switch is enable,you should disable it first*/
																												/*返回-4表示interface argv[1] error, no index or interface down*/
																												/*返回-5表示interface argv[1] is down，返回-6表示interface argv[1] is no flags*/
																												/*返回-7表示interface argv[1] is no index*/
																												/*返回-8表示is no local interface, permission denial*/
																												/*返回-9表示interface argv[1] error*/
																												/*返回-10表示interface has not been added or has already been deleted*/
																												/*返回-11表示interface has be binded in other hansi*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == type)||(NULL == ifname))
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int len = 0;
	int quitreason = 0;
	char *name = NULL;
	unsigned char policy = 2;
	int boot_flag = 0;
	int retu;
    
	
	len = strlen(ifname);	
	if(len > 15)
	{		
		return -1;
	}
	
	if (!strcmp(type,"add"))
	{
		policy = 1;	
	}
	else if (!strcmp(type,"del"))
	{
		policy = 0;	
	}
	else
	{
		return -2;
	}	
	
	name = (char*)malloc(strlen(ifname)+1);
	if(NULL == name)
		return 0;
	memset(name, 0, strlen(ifname)+1);
	memcpy(name, ifname, strlen(ifname)); 
	
	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_UINT32,&boot_flag,
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
	dbus_message_iter_get_basic(&iter,&quitreason);
	/*printf("ret %d quitreason %d\n",ret,quitreason);*/
	if(ret == 0)
	{
		retu = 1;
		//vty_out(vty,"set wireless-control dynamic_ap_login_interface %s %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == IF_BINDING_FLAG)		/* Huangleilei fixed for AXSSZFI-1615 */
	{
		retu = -11;
		//vty_out(vty, "<error> interface %s has be binded in other hansi.\n", argv[1]);
	}
	else if(ret == SWITCH_IS_DISABLE)
	{
		retu = -3;
		//vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	}
	else if(ret == APPLY_IF_FAIL)
	{
		retu = -4;
		//vty_out(vty,"<error> interface %s error, no index or interface down\n",argv[1]);
	}	
	else if (ret == INTERFACE_NOT_BE_BINDED)
	{
		retu = -10;
		//vty_out(vty, "<error>interface %s has not been added or has already been deleted. ", argv[1]);
	}
	else
	{
		if(quitreason == IF_DOWN)
		{
			retu = -5;
			//vty_out(vty,"<error> interface %s is down\n",argv[1]);
		}
		else if(quitreason == IF_NOFLAGS)
		{
			retu = -6;
			//vty_out(vty,"<error> interface %s is no flags\n",argv[1]);
		}
		else if(quitreason == IF_NOINDEX)
		{
			retu = -7;
			//vty_out(vty,"<error> interface %s is no index\n",argv[1]);
		}
		else if (quitreason == WID_INTERFACE_NOT_BE_LOCAL_BOARD)
		{
			retu = -8;
			//vty_out(vty,"<error> %s is no local interface, permission denial\n",argv[1]);
		}
		else
		{
			retu = -9;
			//vty_out(vty,"<error>  interface %s error\n",argv[1]);
		}
	}
		
	dbus_message_unref(reply);
	FREE_OBJECT(name);

	return retu;			
}

/*update_time的范围是5-3600*/
int set_ap_update_img_timer_cmd(dbus_parameter parameter, DBusConnection *connection,char *update_time)
																		 /*返回0表示失败，返回1表示成功*/
																		 /*返回-1表示unknown id format，返回-2表示error*/
																		 /*返回-3表示input time should be 5-3600*/
																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;
	
	if(NULL == update_time)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned int time = 0;
	int retu;

	ret = parse_int_ID((char*)update_time, &time);
	if(ret != WID_DBUS_SUCCESS){
		//vty_out(vty,"<error> unknown id format\n");
		return -1;
	}	
	if((time < 5)||(time > 3600))
	{
		return -3;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_TIMER);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_TIMER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
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
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		retu = 1;
		//vty_out(vty,"set sample time %s successfully\n",argv[0]);
	}
	else 
	{
		retu = -2;
		//vty_out(vty,"<error> %d\n",ret);
	}
	
	return retu;
}

void Free_ap_update_img_timer(DCLI_AC_API_GROUP_FIVE *up_timer)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER,up_timer);
			up_timer = NULL;
		}
	}
}

/*返回1时，调用Free_ap_update_img_timer()释放空间*/
int show_ap_update_img_timer_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **up_timer)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = 0 ;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*up_timer = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*up_timer = (*dcli_init_func)
				(
					parameter.instance_id,
					ELEVENTH,/*"show ap update img timer"*/
					0,
					0,
					1,/*didn't transfer ret,so set it to 1 for function dcli_ac_show_api_group_five() */
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER
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
	else if((ret == 0)&&(*up_timer))
	{	
		retu = 1;
	}
	else 
	{
		retu = -1;
	}

	return retu;	
}

/*未使用*/
int update_wtpcompatible_cmd(dbus_parameter parameter, DBusConnection *connection)/*返回0表示失败，返回1表示成功*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTPCOMPATIBLE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTPCOMPATIBLE);*/
	dbus_error_init(&err);

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
	dbus_message_unref(reply);	
	if(ret == 0){
		retu = 1;
		//vty_out(vty,"update wtpcompatible successfully\n");		
	}
	return retu;			
}

/*未使用*/
int set_ap_update_fail_count_cmd(dbus_parameter parameter, DBusConnection *connection,char *update_fail_count)/*返回0表示失败，返回1表示成功*/
																													/*返回-1表示unknown id format*/
																													/*返回-2表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;
	
	if(NULL == update_fail_count)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned char count = 0;
	int retu;

	ret = parse_char_ID((char*)update_fail_count, &count);
	if(ret != WID_DBUS_SUCCESS){
		//vty_out(vty,"<error> unknown id format\n");
		return -1;
	}
	#if 0
	if(count < 0||count > 255)
	{
		return -3;		
	}
	#endif

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_FAIL_COUNT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_FAIL_COUNT);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&count,
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
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		retu = 1;
		//vty_out(vty,"set update max fail count %s successfully\n",argv[0]);
	}
	else 
	{
		retu = -2;
		//vty_out(vty,"<error> %d\n",ret);
	}
	
	return retu;
}

void Free_ap_update_fail_count(DCLI_AC_API_GROUP_FIVE *update_fail)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT,update_fail);
			update_fail = NULL;
		}
	}
}

/*未使用*/
/*返回1时，调用Free_ap_update_fail_count()释放空间*/
int show_ap_update_fail_count_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **update_fail)/*返回0表示失败，返回1表示成功*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret = 0;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*update_fail = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*update_fail = (*dcli_init_func)
				(
					parameter.instance_id,
					TWELFTH,/*"show wireless-control config"*/
					0,
					0,
					1,/*didn't transfer ret,so set it to 1 for function dcli_ac_show_api_group_five() */
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT
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
	else if((ret == 0)&&(*update_fail))
	{
		retu = 1;
	}

	return retu;	
}

/*未使用*/
/*state为"open"或"close"*/
int set_wid_watch_dog_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char open;
	int ret;
	int retu;

	if (!strcmp(state,"open"))
	{
		open = 1;	
	}		
	else if (!strcmp(state,"close"))
	{
		open = 0;
	}

	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_WATCH_DOG);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_WATCH_DOG);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&open,
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
	{
		retu = 1;
		//vty_out(vty,"set wid watch dog %s successfully\n",argv[0]);
	}
	else
	{
		retu = -1;
		//vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	return retu;		
	
}

void Free_ap_network_bywtpid(DCLI_AC_API_GROUP_THREE *network)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_THREE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_three");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK,network);
			network = NULL;
		}
	}
}

/*返回1时，调用Free_ap_network_bywtpid()释放空间*/
int show_ap_network_bywtpid_cmd(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,DCLI_AC_API_GROUP_THREE **network)
																										/*返回0表示失败，返回1表示成功*/
																									    /*返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																									    /*返回-2表示ap have not ip information*/
																									    /*返回-3表示wtp id no exist*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	if(NULL == wtp_id)
	{
		*network = NULL;
		return 0;
	}
	
	int ret = -1;
	int retu;

	int wtpid = 0;
	wtpid = atoi(wtp_id);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
	}	

	void*(*dcli_init_func)(
				int ,
				unsigned int ,
				unsigned int ,
				unsigned int ,
				unsigned int ,
				unsigned int* ,
				unsigned int* ,
				unsigned int* ,
				DBusConnection *,
				char *
				);
    *network = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_three");
		if(NULL != dcli_init_func && connection)
		{			
			*network =(*dcli_init_func)
				  (
					parameter.instance_id,
					FIRST,/*"show ap network bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK
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
	else if((ret == 0)&&(*network))
	{
		retu = 1;
	}
	else if(ret == WID_AP_NO_STATICS)
	{
		retu = -2;
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		retu = -3;
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
		
	return retu;			
}

/*method_type为"number","flow"或"disable"*/
int ac_load_balance_cmd(dbus_parameter parameter, DBusConnection *connection,char *method_type)/*返回0表示失败，返回1表示成功，返回-1表示operation fail，返回-2表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == method_type)
		return 0;
	
	int ret;
	int retu;
	unsigned char method=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	

	if (!strcmp(method_type,"number")){
		method=1;
	}
	else if (!strcmp(method_type,"flow")){
		method=2;
	} 
	else if (!strcmp(method_type,"disable")){
		method=0;
	}



	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AC_LOAD_BALANCE);	
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AC_LOAD_BALANCE);*/

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&method,
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
		retu = 1;
	else if(ret == WID_DBUS_ERROR)
		retu = -1;
	else
		retu = -2;	
	
	dbus_message_unref(reply);
	return retu;
}

void Free_ac_balance(DCLI_AC_API_GROUP_FIVE *balance)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION,balance);
			balance = NULL;
		}
	}
}

/*返回1时，调用Free_ac_balance()释放空间*/
int show_ac_balance_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **balance)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{	
	if(NULL == connection)
		return 0;

	int ret = WID_DBUS_SUCCESS;
	int retu;


	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*balance = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*balance =(*dcli_init_func)
				  (
					parameter.instance_id,
					THIRTEENTH,/*"show ac balance configuration"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION
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
	else if((ret == 0)&&(*balance))
	{
		retu = 1;
	}
	else if(ret == WID_DBUS_ERROR) 
	{
		retu = -1;
	}
	
	return retu;
}

/*未使用*/
/*state为"enable"或"disable"*/
int set_ap_hotreboot_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;
	int retu;

	
	if (!strcmp(state,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(state,"disable"))
	{
		policy = 0;	
	}
	else
	{
		//vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_HOTREBOOT);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_HOTREBOOT);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
		//vty_out(vty," set ap hotreboot %s successfully\n",argv[0]);
	}				
	else
	{
		retu = -2;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*state为"enable"或"disable"*/
int set_ap_access_through_nat_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																										   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

    int policy = 0;

	
	if (!strcmp(state,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(state,"disable"))
	{
		policy = 0;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_ACCESS_THROUGH_NAT);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_ACCESS_THROUGH_NAT);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*policy_type为"no"或"forbid"*/
int set_wtp_wids_policy_cmd(dbus_parameter parameter, DBusConnection *connection,char *policy_type)/*返回0表示失败，返回1表示成功，返回-1表示wids switch is enable，返回-2表示error*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == policy_type)
		return 0;
	
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
    unsigned char policy = 0;
	
	if (!strcmp(policy_type,"forbid"))
	{
		policy = 1;	
	}	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_WID_MAC);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_POLICY);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,	
							 DBUS_TYPE_BYTE,&policy,
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
	{
		retu = 1;
		//vty_out(vty,"set wtp wids policy %s successfully\n",argv[0]);
	}
	else if (ret == SWITCH_IS_DISABLE)
	{
		retu = -1;
		//vty_out(vty,"<error> wids switch is enable\n");
	}
	else
	{
		retu = -2;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return retu;			
}

int add_wids_mac_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC)/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == MAC)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)MAC,&macaddr);
	if (CMD_FAILURE == ret) {
		//vty_out(vty,"<error> Unknown mac addr format.\n");
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_WID_MAC);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_WID_MAC);*/

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu = 1;
		//vty_out(vty," add wids ignore mac successfully\n");
	}				
	else
	{
		retu = -2;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return retu;			
}

/*未使用*/
int del_wids_mac_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC)/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == MAC)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)MAC,&macaddr);
	if (CMD_FAILURE == ret) {
		//vty_out(vty,"<error> Unknown mac addr format.\n");
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_MAC);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_MAC);*/

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
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
	{
		retu = 1;
		//vty_out(vty," del wids ignore mac successfully\n");
	}				
	else
	{
		retu = -2;
		//vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return retu;			
}

/*未使用*/
/*返回1且mac_num>0时，调用Free_maclist_head()释放空间*/
int show_wids_mac_list_cmd(dbus_parameter parameter, DBusConnection *connection,WIDMACADDR *mac_head,int *mac_num)/*返回0表示失败，返回1表示成功，返回-1表示there is no wids ignore mac list，返回-2表示error*/
																														 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	if(NULL == mac_head)
	{
		*mac_num = 0;
		return 0;
	}
	
	int ret;
	int num = 0;
	int i = 0;
	//int length = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;
	
    int state = 1;
	unsigned char mac[DCLIAC_MAC_LEN];	
	WIDMACADDR *q,*tail;
	int retu = 1;
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_WIDS_MAC_SHOW );

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE, WID_DBUS_CONF_METHOD_WIDS_MAC_SHOW );*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
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
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		mac_head->next = NULL;
    	tail=mac_head;
		*mac_num = num;
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(mac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[5]));

			dbus_message_iter_next(&iter_array);


			//vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",i+1,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			q=(WIDMACADDR *)malloc(sizeof(WIDMACADDR));
			if(q == NULL)
			{
				retu = 0;
				break;
			}

			memset(q->macaddr,0,DCLIAC_MAC_LEN);
            q->macaddr[0]=mac[0];
			q->macaddr[1]=mac[1];
			q->macaddr[2]=mac[2];
			q->macaddr[3]=mac[3];
			q->macaddr[4]=mac[4];
			q->macaddr[5]=mac[5];
            
			q->next=NULL;
			if(tail)
			{
				tail->next=q;
				tail=q;
			}

		}
	}
	else if(ret == WID_NO_WHITELIST)
	{
		retu = -1;
		//vty_out(vty,"there is no wids ignore mac list \n");
	}
	else
	{
		retu = -2;
		//vty_out(vty,"error %d \n",ret);
	}	

		
	dbus_message_unref(reply);	
	
	return retu;				
}

/*state为"enable"或"disable"*/
int set_ap_countermeasures_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

    int policy = 0;

	
	if (!strcmp(state,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(state,"disable"))
	{
		policy = 0;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);
	
	return retu;			
}

/*mode为"ap"或"adhoc"或"all"*/
int set_ap_countermeasures_mode_cmd(dbus_parameter parameter, DBusConnection *connection,char *mode)/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == mode)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

    int policy = 0;

	
	if (!strcmp(mode,"ap"))
	{
		policy = 0;	
	}
	else if (!strcmp(mode,"adhoc"))
	{
		policy = 1;	
	}
	else if (!strcmp(mode,"all"))
	{
		policy = 2;	
	}
	else
	{
		return -1;
	}
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES_MODE);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES_MODE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);
	
	return retu;			
}

/*未使用*/
/*argc的范围是2-4*/
int dynamic_channel_selection_range_cmd(dbus_parameter parameter, DBusConnection *connection,int argc,char **argv)
																					  /*返回0表示失败，返回1表示成功*/
																					  /*返回-1表示range of the channel num is 2-4.，返回-2表示patameter format error.*/
																					  /*返回-3表示you should enable radio resource management first*/
																					  /*返回-4表示illegal input:Input exceeds the maximum value of the parameter type*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == (*argv))
		return 0;
	
	int ret;
	int retu;

	unsigned char channel[4] = {0,0,0,0};
	int i;
	if(argc <= 1 || argc > 4){
		return -1;
	}
	for(i = 0; i < argc; i++){		
		ret = parse_char_ID((char*)((*argv)+i), &channel[i]);
		if(ret != WID_DBUS_SUCCESS){			
			if(ret == WID_ILLEGAL_INPUT){
				retu = -4;
			}
			else{
				retu = -2;
			 }
			return retu;
		}
	}
	


	int(*dcli_init_func)(
		            int ,
					int ,
					unsigned int ,
					unsigned char *,
					DBusConnection *
					);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_set_dynamic_channel_selection_range");
		if(NULL != dcli_init_func)
		{
			ret = (*dcli_init_func)
				(
                    parameter.local_id,
			        parameter.instance_id,
					argc,
					channel,
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

	if(ret == 0)
	{
		retu = 1;
	}				
	else if(ret == -1)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else
	{
		retu = -3;
	}
	
	return retu; 		
}

void Free_vrrp_state(DCLI_AC_API_GROUP_FIVE *vrrp_state)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FIVE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_five");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_AC_METHOD_VRRP_INFO,vrrp_state);
			vrrp_state = NULL;
		}
	}
}

/*未使用*/
/*返回1时，调用Free_vrrp_state()释放空间*/
int show_vrrp_state_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **vrrp_state)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;
	
	void*(*dcli_init_func)(
				int ,
				unsigned int ,
				unsigned int ,
				unsigned int ,
				unsigned int ,
				unsigned int *,
				char *,
				char *,
				int ,
				DBusConnection *,
				char *
				);

	*vrrp_state = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_five");
		if(NULL != dcli_init_func)
		{
			*vrrp_state = (*dcli_init_func)
				(
					parameter.instance_id,
					TENTH,/*"show wid vrrp state"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_AC_METHOD_VRRP_INFO
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
	else if((ret == 0)&&(*vrrp_state))
	{	
		retu = 1;
	}
	else
	{
		retu = -1;
	}

	return retu;
}

void Free_vrrp_sock(DCLI_AC_API_GROUP_FOUR *baksock)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_FOUR *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_four");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_AC_METHOD_VRRP_SOCK_INFO,baksock);
			baksock = NULL;
		}
	}
}

/*未使用*/
/*返回1时，调用Free_vrrp_sock()释放空间*/
int show_vrrp_sock_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FOUR **baksock)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int retu;

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int *,
					char *,
					char *,
					int ,
					DBusConnection *,
					char *
					);

	*baksock = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_four");
		if(NULL != dcli_init_func)
		{
			*baksock = (*dcli_init_func)
				(
					parameter.instance_id,
					FIFTH,/*"show wid vrrp sock list"*/
					0,
					0,
					&ret,
					0,
					0,
					parameter.local_id,
					connection,
					WID_DBUS_AC_METHOD_VRRP_SOCK_INFO
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
	else if((ret == 0)&&(*baksock))
	{
		retu = 1;
	}
	else
	{
		retu = -1;
	}
	
	return retu;				
}

/*type为"number"或"flow"*/
/*number参数的范围是1-10*/
/*flow参数的范围是1-30*/
int ac_balance_parameter_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *para)
																		  /*返回0表示失败，返回1表示成功，返回-1表示unknown input*/
																		  /*返回-2表示balance parameter should be 1 to 10，返回-3表示balance parameter should be 1 to 30*/
																		  /*返回-4表示operation fail，返回-5表示error*/
																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == type)||(NULL == para))
		return 0;
	
	int ret;
	unsigned char method = 0;
	unsigned int bal_para=0;
	int res = WID_DBUS_SUCCESS;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int retu;

	dbus_error_init(&err);
	
	if (!strcmp(type,"number")){
		method=1;
	}
	else if (!strcmp(type,"flow")){
		method=2;
	}else {
		return -1;
	}
	
	res = parse_int((char*)para, &bal_para);
	if(res != WID_DBUS_SUCCESS){
		return -1;
	}	
	if(method == 1){
		if(bal_para > 10 || bal_para < 1){
			return -2;
		}
	}else if(method == 2){
		if(bal_para > 30 || bal_para < 1){
			return -3;
		}
	}
			

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_AC_LOAD_BALANCE_PARA);	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&method,
							 DBUS_TYPE_UINT32,&bal_para,
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
		retu = 1;
	else if(ret == WID_DBUS_ERROR)
		retu = -4;
	else
		retu = -5;
	
	dbus_message_unref(reply);
	return retu;
}


/*未使用*/
/*trap_type为"ap_run_quit","ap_cpu_threshold","ap_mem_threshold","ap_update_fail","rrm_change","rogue_ap_threshold",
	                "rogue_terminal_threshold","rogue_ap","rogue_device","wireless_interface_down","channel_count_minor"或"channel_change"*/
/*state为"enable"或"disable"*/
int set_wid_trap_switch_able_cmd(dbus_parameter parameter, DBusConnection *connection,char *trap_type,char *state)
																				   /*返回0表示失败，返回1表示成功，返回-1表示the first input patameter error*/
																				   /*返回-2表示input patameter should only be 'enable' or 'disable'，返回-3表示error*/
																				   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == trap_type)||(NULL == state))
		return 0;
		
	int ret;
	unsigned int trap_switch = 0;
	unsigned int trap_policy = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
		

	/*parse first papatameter*/
	if (!strcmp(trap_type,"ap_run_quit"))
	{
		trap_switch = 1;	
	}
	else if (!strcmp(trap_type,"ap_cpu_threshold"))
	{
		trap_switch = 2;	
	}
	else if (!strcmp(trap_type,"ap_mem_threshold"))
	{
		trap_switch = 3;	
	}
	else if (!strcmp(trap_type,"ap_update_fail"))
	{
		trap_switch = 4;	
	}
	else if (!strcmp(trap_type,"rrm_change"))
	{
		trap_switch = 5;	
	}
	else if (!strcmp(trap_type,"rogue_ap_threshold"))
	{
		trap_switch = 6;	
	}
	else if (!strcmp(trap_type,"rogue_terminal_threshold"))
	{
		trap_switch = 7;	
	}
	else if (!strcmp(trap_type,"rogue_device"))
	{
		trap_switch = 8;	
	}
	else if (!strcmp(trap_type,"wireless_interface_down"))
	{
		trap_switch = 9;	
	}
	else if (!strcmp(trap_type,"channel_count_minor"))
	{
		trap_switch = 10;	
	}
	else if (!strcmp(trap_type,"channel_change"))
	{
		trap_switch = 11;	
	}
	else if (!strcmp(trap_type,"rogue_ap"))
	{
		trap_switch = 12;	
	}
	else
	{
		return -1;
	}
	/*parse second papatameter*/
	if (!strcmp(state,"enable"))
	{
		trap_policy = 1;	
	}
	else if (!strcmp(state,"disable"))
	{
		trap_policy = 0;	
	}
	else
	{
		return -2;
	}

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRAP_SWITCH_ABLE);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRAP_SWITCH_ABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
 							DBUS_TYPE_UINT32,&trap_switch,
 						    DBUS_TYPE_UINT32,&trap_policy,
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
		retu = 1;
	else
		retu = -3;
		
	dbus_message_unref(reply);	
	
	return retu;			
}

/*未使用*/
int show_wid_trap_switch_info_cmd(dbus_parameter parameter, DBusConnection *connection,WID_TRAP_SWITCH **INFO)/*返回0表示失败，返回1表示成功*/
{
	if(NULL == connection)
		return 0;

	int ret = 0;

	void*(*dcli_init_func)(
					int ,
					int ,
					int *,
					DBusConnection *
					);

	*INFO = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_wtp_trap_switch");
		if(NULL != dcli_init_func)
		{
			*INFO = (typeof(*INFO))(*dcli_init_func)
				(
					parameter.local_id,
					parameter.instance_id,
					&ret,
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

	if(NULL == (*INFO))
		return 0;
	
	return 1; 
}


int modify_legal_essid_cmd(dbus_parameter parameter, DBusConnection *connection,char *old_essid,char *new_essid)
																		/*返回0表示失败，返回1表示成功*/
																		/*返回-1表示The essid list is null,there is no essid*/
																		/*返回-2表示The essid input is not exit*/
																		/*返回-3表示error*/
																		/*返回-4表示first essid is too long,out of the limit of 32*/
																		/*返回-5表示second essid is too long,out of the limit of 32*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == old_essid)||(NULL == new_essid))
		return 0;

	if (strlen(old_essid) > 32)
	{
		return -4;
	}
	if (strlen(new_essid) > 32)
	{
		return -5;
	}
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

	char *essid=(char *)malloc(strlen(old_essid)+1);
	if(NULL == essid)
		return 0;
	memset(essid,0,strlen(old_essid)+1);
	memcpy(essid,old_essid,strlen(old_essid));

	char *essid_new=(char *)malloc(strlen(new_essid)+1);
	if(NULL == essid_new){
		if(NULL!=essid){free(essid);essid=NULL;}
		return 0;
	}
	memset(essid_new,0,strlen(new_essid)+1);
	memcpy(essid_new,new_essid,strlen(new_essid));
	
	/*printf("essid:\t%s\n",essid);*/

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MODIFY_ESSID);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_MODIFY_ESSID );*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_STRING,&essid_new,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

	dbus_message_unref(query);

	FREE_OBJECT(essid);
	FREE_OBJECT(essid_new);

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
		retu = 1;	
	else if(ret == ESSID_LIST_IS_NULL)
		retu = -1;
	else if(ret == ESSID_NOT_EXIT)
		retu = -2;
	else
		retu = -3;
		
	dbus_message_unref(reply);

	return retu;	
}

/*state为"enable"或"disable"*/
int set_ac_all_ap_extension_information_enable_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)
																						/*返回0表示失败，返回1表示成功*/
																						/*返回-1表示input patameter only with 'enable' or 'disable'*/
																						/*返回-2表示error*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	dbus_error_init(&err);

    int policy = 0;

	if (!strcmp(state,"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(state,"disable"))
	{
		policy = 0;	
	}
	else
	{
		return -1;
	}


	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_AC_EXTENTION_INFOR_ENABLE);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
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
		retu = 1;
	else if(ret == WTP_ID_NOT_EXIST)
		retu = 1;
	else
		retu = -2;
		
	dbus_message_unref(reply);

	
	return retu;			
}

/*state为"enable"或"disable"*/
int set_wid_mac_whitelist_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

	unsigned char macfilterflag = 0;
	int ret;

	if (!strcmp(state,"enable"))
	{
		macfilterflag = 1;	
	}	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_MAC_WHITELIST_SWITCH);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_MAC_WHITELIST_SWITCH);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&macfilterflag,
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
		retu = 1;
	else
		retu = -1;

	dbus_message_unref(reply);
	return retu;		
}

/*state为"enable"或"disable"*/
int set_wid_essid_whitelist_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;

	unsigned char essidfilterflag = 0;
	int ret;

	if (!strcmp(state,"enable"))
	{
		essidfilterflag = 1;	
	}	


	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_ESSID_WHITELIST_SWITCH);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_ESSID_WHITELIST_SWITCH);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&essidfilterflag,
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
		retu = 1;
	else
		retu = -1;

	dbus_message_unref(reply);
	return retu;			
}


int change_wirelesscontrol_whitelist_cmd(dbus_parameter parameter, DBusConnection *connection,char *old_mac,char *new_mac)
																							/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format*/
																							/*返回-2表示src mac isn't exist，返回-3表示dst mac already in white list，返回-4表示error*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == old_mac)||(NULL == new_mac))
		return 0;
	
	int ret;
	WIDMACADDR macaddr; 
	WIDMACADDR macaddrdest; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = wid_parse_mac_addr((char *)old_mac,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	
	ret = wid_parse_mac_addr((char *)new_mac,&macaddrdest);
	if (CMD_FAILURE == ret) {
		return -1;
	}	


	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_WHITELIST);	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_WHITELIST);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],

							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[5],							 
							 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	/*printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
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
		retu = 1;
	else if(MAC_DOESNOT_EXIT == ret)
		retu = -2;
	else if(MAC_ALREADY_EXIT == ret)
		retu = -3;
	else
		retu = -4;
		
	dbus_message_unref(reply);
	return retu;			
}

/*未使用*/
int change_wirelesscontrol_blacklist_cmd(dbus_parameter parameter, DBusConnection *connection,char *old_mac,char *new_mac)
																							/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format*/
																							/*返回-2表示src mac isn't exist，返回-3表示dst mac already in white list，返回-4表示error*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == old_mac)||(NULL == new_mac))
		return 0;
	
	int ret;
	WIDMACADDR macaddr; 
	WIDMACADDR macaddrdest; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int retu;
	
	ret = wid_parse_mac_addr((char *)old_mac,&macaddr);
	if (CMD_FAILURE == ret) {
		return -1;
	}
	
	ret = wid_parse_mac_addr((char *)new_mac,&macaddrdest);
	if (CMD_FAILURE == ret) {
		return -1;
	}	
	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_BLACKLIST);		
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_BLACKLIST);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],

							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[5],							 
							 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	/*printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
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
		retu = 1;
	else if(ret == MAC_DOESNOT_EXIT)
		retu = -2;
	else if(ret == MAC_ALREADY_EXIT)
		retu = -3;
	else
		retu = -4;
		
	dbus_message_unref(reply);

	return retu;			
}




/*config节点下wtp_id为0*/
/*value的范围是1-32767*/
int set_ap_statistics_inter_cmd(dbus_parameter parameter, DBusConnection *connection,int wtp_id,char *value)
																		  /*返回0表示失败，返回1表示成功*/
																		  /*返回-1表示input interval should be 1 to 332767*/
																		  /*返回-2表示invalid wtpid，返回-3表示error*/
																		  /*返回-4表示illegal input:Input exceeds the maximum value of the parameter type*/
																		  /*返回-5表示unknown id format*/
																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == value)
		return 0;
		
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int interval = 0;
	unsigned int wtpid = 0;

	int retu;
	ret = parse_int_ID((char*)value, &interval);
	
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
				retu = -4;
            }
			else{
				retu = -5;
			}
			return retu;
	}	
	if ((interval <= 0) || (interval > 32767)) {
		return -1;
	}
	

	wtpid = wtp_id;
	if(/*(wtpid<0)||*/(wtpid>(WTP_NUM-1))){
		return -2;
	}
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//printf("wtpid:%d,index:%d\n",wtpid,index);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS_INTERVAL);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&interval,
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


void Free_show_neighbor_ap_list_cmd(struct allwtp_neighborap *neighborap)
{
	void (*dcli_init_free_func)(struct allwtp_neighborap *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_free_allwtp_neighbor_ap");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(neighborap);
		}
	}
}

/*只要调用函数，就调用Free_show_neighbor_ap_list_cmd()释放空间*/
int show_neighbor_ap_list_cmd(dbus_parameter parameter, DBusConnection *connection,struct allwtp_neighborap **neighborap)/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;

	int ret;
	int ret1;
	int wtp_num = 0;
	int retu = 0;	

	void*(*dcli_init_func)(
					int ,
					int ,
					DBusConnection *,
					int* ,
					int* ,
					int* 
					);

	*neighborap = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_neighbor_ap_list_cmd_allap");
		if(NULL != dcli_init_func)
		{
			*neighborap =(*dcli_init_func)
				  (
					parameter.local_id,
					parameter.instance_id,
					connection,
					&wtp_num,
					&ret,
					&ret1
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

	

	if((*neighborap != NULL)&&(0 == ret))
		retu = 1;
	else if(ret == -1)
		retu = SNMPD_CONNECTION_ERROR;
	else
		retu = -1;

	return retu;	
}

void Free_show_wids_device_of_all_cmd(DCLI_AC_API_GROUP_TWO *dcli_list)
{
	struct tag_wids_device_ele *f1 = NULL;
	struct tag_wids_device_ele *f2 = NULL;
	if(dcli_list)
	{
		if(dcli_list->wids_device_list)
		{
			f1 = dcli_list->wids_device_list->wids_device_info;
			while(f1)
			{
				f2 = f1->next;
				free(f1);
				f1 = f2;
			}
		}
		CW_FREE_OBJECT(dcli_list->wids_device_list);
	}
	CW_FREE_OBJECT(dcli_list);	
}

/*返回1时，调用Free_show_wids_device_of_all_cmd()释放空间*/
int show_wids_device_of_all_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **dcli_list,unsigned int *last_time)
																												 /*返回0表示失败，返回1表示成功*/
																												 /*返回-1表示good luck there is no wids device*/
																												 /*返回-2表示error*/
																												 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	int ret = 0;
	unsigned int lasttime = 0;
	int retu = 0;
	
	void*(*dcli_init_func)(
						int ,
						DBusConnection *,
						int ,
						int* ,
						unsigned int* 
					);

    *dcli_list = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"show_wids_device_of_all_device");
		if(NULL != dcli_init_func)
		{
			*dcli_list =(*dcli_init_func)
				  (
				  	parameter.local_id,
					connection,
					parameter.instance_id,
					&ret,
					&lasttime
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
	else if((ret == 0)&&(*dcli_list))
	{
		retu = 1;
		*last_time = lasttime;
	}
	else if(ret == NO_WIDS_DEVICE)
	{
		retu = -1;
	}
	else
	{
		retu = -2;
	}
	
	return retu; 		
}

void Free_show_ac_access_wtp_vendor_count(struct acAccessWtpCount *count_head, struct acAccessBindLicCount *bind_lic_head) {

    while(count_head) {
       struct acAccessWtpCount *temp = count_head->next;
       free(count_head);
       count_head = temp;
    }

	while(bind_lic_head) {
       struct acAccessBindLicCount *temp = bind_lic_head->next;
       free(bind_lic_head);
       bind_lic_head = temp;
    }

    return ;
}

/*返回1时，调用Free_show_ac_access_wtp_vendor_count()释放空间*/
int show_ac_access_wtp_vendor_count(dbus_parameter parameter, DBusConnection *connection, struct acAccessWtpCount **count_head, struct acAccessBindLicCount **bind_lic_head) {
    
    if(NULL == connection || NULL == count_head || NULL == bind_lic_head)
        return 0;

    *count_head = NULL;
    struct acAccessWtpCount *temp_head = NULL;

	*bind_lic_head = NULL;
    struct acAccessBindLicCount *head = NULL;
    
    int ret = 0;
	int state = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;
	
	unsigned int licensetypecount;
	int bind_lic_count = 0;
	int cur_count = 0;
	int max_count = 0;
	int bind_flag = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id, WID_DBUS_BUSNAME, BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id, WID_DBUS_OBJPATH, OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id, WID_DBUS_INTERFACE, INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_ACCESS_WTP_VENDOR_COUNT_SHOW );

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
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

	if(ret == 0) {
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &licensetypecount);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		int i = 0;
		for (i = 0; i < licensetypecount; i++) {

		    struct acAccessWtpCount *temp_node = (struct acAccessWtpCount *)malloc(sizeof(struct acAccessWtpCount));
		    if(NULL == temp_node) {
                dbus_message_unref(reply);  
                Free_show_ac_access_wtp_vendor_count(temp_head,head);
                return 0;
		    }
            memset(temp_node, 0, sizeof(*temp_node));
		    
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&temp_node->license_count[0]);      /*CurrentWtpCount*/ 
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&temp_node->license_count[1]);     /*license num*/
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&temp_node->license_count[2]);    /*BindingFlag*/     

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&temp_node->license_count[3]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&temp_node->license_count[4]);
			
			dbus_message_iter_next(&iter_array);			

			temp_node->license_type = i + 1;

			temp_node->next = temp_head;
			temp_head = temp_node;
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bind_lic_count);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		for (i = 0; i < bind_lic_count; i++)
		{
			struct acAccessBindLicCount *temp_node = (struct acAccessBindLicCount *)malloc(sizeof(struct acAccessBindLicCount));
		    if(NULL == temp_node) {
                dbus_message_unref(reply);  
                Free_show_ac_access_wtp_vendor_count(temp_head,head);
                return 0;
		    }
            memset(temp_node, 0, sizeof(*temp_node));
			
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&cur_count);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&max_count);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&bind_flag);

			dbus_message_iter_next(&iter_array);

			temp_node->bind_flag = bind_flag;
			temp_node->cur_count = cur_count;
			temp_node->max_count = max_count;

			temp_node->next = head;
			head = temp_node;
		}		
	}
	else {	    
        dbus_message_unref(reply);
		return -1;
	}
	
	dbus_message_unref(reply);

    *count_head = temp_head;
	*bind_lic_head = head;
	
	return 1;			
}


/*config节点下wtp_id为0*/
/*scan_mode为"2--monitor"、"3--halfmonitor"或"1--disable"*/
int set_wids_monitor_mode_cmd(dbus_parameter parameter, DBusConnection *connection, unsigned int wtp_id, unsigned int scan_mode)
																									 /*返回0表示失败，返回1表示成功，返回-1表示invalid input*/
																								     /*返回-2表示invalid wtp id，返回-3表示error*/
{
    
    if(NULL == connection)
        return 0;

	int ret = 0;
	int mode = 0;

	switch(scan_mode) {
        case 1:
            mode = 0;
            break;

        case 2:
            mode = 2;
            break;
            
        case 3:
            mode = 1;
            break;

        default:
            return -1;
	}

	if((wtp_id < 0)||(wtp_id > (WTP_NUM-1))){
		return -2;
	}

	int(*dcli_init_func)(
					int ,
					int,
					unsigned int ,
					int ,
					DBusConnection *
					);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"set_wids_monitor_mode");
		if(NULL != dcli_init_func) {
		
            ret = (*dcli_init_func) (parameter.instance_id,
                                     parameter.local_id,
                                     wtp_id,
                                     mode,
                                     connection);  
		}
		else {
			return 0;
		}
	}
	else {
        return 0;
	}

    	
	if(ret == -1) {
		return SNMPD_CONNECTION_ERROR;
	}
	else if(ret == 0) {
		return 1;
	}	
	else {
		return -3;
	}
}

/*level为"dump","debug","info","notice","warning","error","crit","alert","emerg","default"*/
int set_asd_daemonlog_level_cmd(dbus_parameter parameter, DBusConnection *connection,char *level)
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示input patameter should only be dump|debug|info|notice|warning|error|crit|alert|emerg*/
																			/*返回-2表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == level)
		return 0;
	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
    unsigned int daemonloglevel = 5;
	int retu = 0;
	
	if (!strcmp(level,"dump"))
	{
		daemonloglevel = 1;	
	}
	else if (!strcmp(level,"debug"))
	{
		daemonloglevel = 2;	
	}
	else if (!strcmp(level,"info"))
	{
		daemonloglevel = 3;	
	}
	else if ((!strcmp(level,"notice"))||(!strcmp(level,"default")))
	{
		daemonloglevel = 4;	
	}
	else if (!strcmp(level,"warning"))
	{
		daemonloglevel = 5;	
	}
	else if (!strcmp(level,"error"))
	{
		daemonloglevel = 6;	
	}
	else if (!strcmp(level,"crit"))
	{
		daemonloglevel = 7;	
	}
	else if (!strcmp(level,"alert"))
	{
		daemonloglevel = 8;	
	}
	else if (!strcmp(level,"emerg"))
	{
		daemonloglevel = 9;	
	}
	else
	{
		return -1;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_LEVEL);
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&daemonloglevel,
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
	{
		retu = 1;
	}				
	else
	{
		retu = -2;
	}
		
	dbus_message_unref(reply);
	return retu;			
}

/*Type为"master","bakup","disable"*/
int set_ac_master_ipaddr_cmd(dbus_parameter parameter, DBusConnection *connection,char *Type,char *ipaddr)
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示invalid input,input should be master or bakup*/
																			/*返回-2表示unknown ip format，返回-3表示more if have this ip*/
																			/*返回-4表示no if has this ip，返回-5表示please disable it first*/
																			/*返回-6表示no interface binding this ip*/
																			/*返回-7表示this ip has not been added or has already been deleted*/
																			/*返回-8表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == Type) || (NULL == ipaddr))
		return 0;

	int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0; 	
	int interval = 0;
	char type = 0;
	unsigned int ip = 0;
	int retu = 0;
	
	if(!strcmp("master",(char*)Type)){
		type = 1;
	}else if(!strcmp("bakup",(char*)Type)){
		type = 2;
	}else if(!strcmp("disable",(char*)Type)){
		type = 3;
	}else{
		return -1;
	}

	ret = WID_Check_IP_Format((char*)ipaddr);
	if(ret != WID_DBUS_SUCCESS){
		return -2;
	}
	ip = dcli_ip2ulong((char*)ipaddr);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_AC_ACTIVE_BAK_STATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_UINT32,&ip,
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
		retu = 1;
	else if(ret == MORE_THAN_ONE_IF)
		retu = -3;
	else if(ret == NO_IF_HAS_THIS_IP)
		retu = -4;
	else if(ret == INVALID_TYPE)
		retu = -5;
	else if (ret == INTERFACE_NOT_EXIST)
		retu = -6;
	else if (ret == AC_STATE_IP_NOT_EXIST)
		retu = -7;
	else if (ret == AC_STATE_FIRST_DISABLE)
		retu = -5;
	else
		retu = -8;
	
#ifdef __nouseif__
	if(ifname){
		free(ifname);
		ifname = NULL;
	}
#endif

	return retu;		
}

void Free_show_wireless_listen_if_cmd(Listen_IF *ListenIF)
{
	int (*dcli_init_free_func)(Listen_IF *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_listen_if_node");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(ListenIF);
		}
	}
}


/*只要调用函数，就调用Free_show_wireless_listen_if_cmd()释放空间*/
int show_wireless_listen_if_cmd(dbus_parameter parameter, DBusConnection *connection, Listen_IF **Listen_IF)/*返回0表示失败，返回1表示成功，返回-1表示wid listenning interface:NULL*/
{
	if(NULL == connection || NULL == Listen_IF)
        return 0;
	
	int ret = 0;
	int retu = 0;
	
	void*(*dcli_init_func)(
						int ,
						int ,
						int* ,
						DBusConnection *
						);

    *Listen_IF = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_wid_listen_if");
		if(NULL != dcli_init_func)
		{
			*Listen_IF =(*dcli_init_func)
				  (
				  	parameter.local_id,
					parameter.instance_id,
					&ret,
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
	
	if(ret == -1)
	{
		return SNMPD_CONNECTION_ERROR;
	}
	if((*Listen_IF)->count == 0)
	{
		retu = -1;
	}

	if((*Listen_IF)->interface != NULL)
	{
		retu = 1;
	}

	return retu;
}


/*Oper为"add"或"del"*/
int set_wirelesscontrol_listen_l3_interface_cmd(dbus_parameter parameter, DBusConnection *connection,char *Oper,char *ifname)
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示interface name is too long,should be no more than 15*/
																			/*返回-2表示input patameter only with 'add'or 'del'*/
																			/*返回-3表示 auto ap login switch is enable,you should disable it first*/
																			/*返回-4表示interface error, no index or interface down*/
																			/*返回-5表示this interface has not been added or has already been deleted*/
																			/*返回-6表示interface is down，返回-7表示interface is no flags*/
																			/*返回-8表示tinterface is no index，返回-9表示interface is no local interface, permission denial*/
																			/*返回-10表示interface is other hansi listen*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if((NULL == Oper) || (NULL == ifname))
		return 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int len = 0;
	int quitreason = 0;
	char *name;
	unsigned char policy = 2;
	int boot_flag = 0;
    int retu = 0;
	
	len = strlen(ifname);
	
	if(len > 15)
	{		
		return -1;
	}
	
	if (!strcmp(Oper,"add"))
	{
		policy = 1;	
	}
	else if (!strcmp(Oper,"del"))
	{
		policy = 0;	
	}
	else
	{
		return -2;
	}	
	
	name = (char*)malloc(strlen(ifname)+1);
	memset(name, 0, strlen(ifname)+1);
	memcpy(name, ifname, strlen(ifname)); 
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_LISTEN_L3_INTERFACE);
	
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_UINT32,&boot_flag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		free(name);
		name = NULL;
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_get_basic(&iter,&quitreason);

	if((ret == 0)||(ret == IF_HAS_BEEN_LISTENNING))
		retu = 1;
	else if(ret == SWITCH_IS_DISABLE)
		retu = -3;
	else if(ret == APPLY_IF_FAIL)
		retu = -4;
	else if (ret == INTERFACE_NOT_EXIST)
		retu = -5;
	else
	{
		if(quitreason == IF_DOWN)
			retu = -6;
		else if(quitreason == IF_NOFLAGS)
			retu = -7;
		else if(quitreason == IF_NOINDEX)
			retu = -8;
		else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD)
			retu = -9;
		else if(ret == IF_BINDING_FLAG)
			retu = -10;
		else
			retu = -5;
	}
		
	dbus_message_unref(reply);
	free(name);
	name = NULL;
	return retu;			
}

/*state为"enable"或"disable"*/
int set_vlan_switch_cmd(dbus_parameter parameter, DBusConnection *connection,char *state)
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == connection)
		return 0;
	
	if(NULL == state)
		return 0;

	int ret;
	unsigned char type=0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;
	int retu = 0;

	dbus_error_init(&err);
	

	if (!strncmp(state,"enable",strlen(state))){
		type = 1;
	}
	else if (!strncmp(state,"disable",strlen(state))){
		type = 0;
	} 
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_VLAN_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
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
		retu = 1;
	else
		retu = -1;
	
	dbus_message_unref(reply);
	return retu;
}


#ifdef __cplusplus
}
#endif

