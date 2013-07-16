#ifdef _D_WCPSS_

#include <string.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDBusPath.h"
#include "dbus/asd/ASDDbusPath.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wid_ac.h"
#include "../lib/dcli_main.h"
void ReInitDbusPath(int index, char * path, char * newpath)
{
	int len;
	sprintf(newpath,"%s%d",path,index);	
	if(path == ASD_DBUS_SECURITY_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","security");
	}
	else if(path == ASD_DBUS_SECURITY_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","security");			
	}
	else if(path == ASD_DBUS_STA_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","sta");
	}
	else if(path == ASD_DBUS_STA_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","sta");			
	}
	else if(path == WID_DBUS_WLAN_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wlan");
	}
	else if(path == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wlan");			
	}
	else if(path == WID_DBUS_WTP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wtp");
	}
	else if(path == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wtp");			
	}
	else if(path == WID_DBUS_RADIO_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","radio");
	}
	else if(path == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","radio");			
	}
	else if(path == WID_DBUS_QOS_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","qos");
	}
	else if(path == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","qos");			
	}
	else if(path == WID_DBUS_EBR_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","ebr");
	}
	else if(path == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","ebr");			
	}	
	else if(path == WID_BAK_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","bak");
	}
	else if(path == WID_BAK_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","bak");			
	}
	else if(path == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","aciplist");
	}
	else if(path == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","aciplist");			
	}
	else if(path == ASD_DBUS_AC_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","acgroup");
	}
	else if(path == ASD_DBUS_AC_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","acgroup");			
	}
	else if(path == WID_DBUS_AP_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","apgroup");
	}
	else if(path == WID_DBUS_AP_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","apgroup");			
	}
}
void ReInitDbusPath_V2(int local,int index, char * path, char * newpath)
{
	int len;
	sprintf(newpath,"%s%d_%d",path,local,index); 
	if(path == ASD_DBUS_SECURITY_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","security");
	}
	else if(path == ASD_DBUS_SECURITY_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","security");			
	}
	else if(path == ASD_DBUS_STA_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","sta");
	}
	else if(path == ASD_DBUS_STA_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","sta");			
	}
	else if(path == WID_DBUS_WLAN_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wlan");
	}
	else if(path == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wlan");			
	}
	else if(path == WID_DBUS_WTP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wtp");
	}
	else if(path == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wtp");			
	}
	else if(path == WID_DBUS_RADIO_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","radio");
	}
	else if(path == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","radio");			
	}
	else if(path == WID_DBUS_QOS_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","qos");
	}
	else if(path == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","qos");			
	}
	else if(path == WID_DBUS_EBR_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","ebr");
	}
	else if(path == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","ebr");			
	}	
	else if(path == WID_BAK_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","bak");
	}
	else if(path == WID_BAK_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","bak");			
	}
	else if(path == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","aciplist");
	}
	else if(path == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","aciplist");			
	}
	else if(path == ASD_DBUS_AC_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","acgroup");
	}
	else if(path == ASD_DBUS_AC_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","acgroup");			
	}
	else if(path == WID_DBUS_AP_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","apgroup");
	}
	else if(path == WID_DBUS_AP_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","apgroup");			
	}
}
void ReInitDbusPath_Hmd(int slotid,char * path, char * newpath)
{
	sprintf(newpath,"%s%d",path,slotid); 
}
int wid_mac_format_check(char* str,int len) 
{
	int i = 0;
	unsigned int result = 0;
	char c = 0;
	
	if( 17 != len){
	   return -1;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
			if((':'!=c)&&('-'!=c)&&('.'!=c))
				return -1;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = -1;
			return result;
		}
    }
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
		
        result = -1;
		return result;
	}
	return result;
}

 int wid_parse_mac_addr(char* input,WIDMACADDR* macAddr) 
 {
 	
	int i = 0;
	char cur = 0,value = 0;
	
	if((NULL == input)||(NULL == macAddr)) return -1;
	if(-1 == wid_mac_format_check(input,strlen(input))) return -1;
	
	
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
	
	return 0;
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

int Check_Batch_Command_Format(char* str, batch_arg *arg){
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int i;
	c = str[0];
	if (c=='<'){
		arg->min = strtoul(str+1,&endptr,10);
		if(arg->min < 0){
			return WID_UNKNOWN_ID;
		}
		else if(((arg->min < 10)&&((endptr - str-1) > 1))||((arg->min < 100)&&((endptr - str-1) > 2))||((arg->min < 1000)&&((endptr - str-1) > 3))){
			return WID_UNKNOWN_ID;
		}
		for(i = 0; i < 1; i++){			
			if(endptr[0] == '\0'||endptr[0] != '-'){
				return WID_UNKNOWN_ID;
			}
			else{
				endptr1 = &endptr[1];
				arg->max= strtoul(&endptr[1],&endptr,10);					
				if(arg->max < 0){					
					return WID_UNKNOWN_ID;	
				}			
				else if(((arg->max < 10)&&((endptr - endptr1) > 1))||((arg->max < 100)&&((endptr - endptr1) > 2))||((arg->max < 1000)&&((endptr - endptr1) > 3))){
					return WID_UNKNOWN_ID;
				}
			}
		}
		if(endptr[0] == '>' && arg->max > 0){			
			if(arg->max > arg->min){
				arg->num = arg->max - arg->min + 1;
				return WID_DBUS_SUCCESS;
			}
			else{
				return WID_UNKNOWN_ID;	
			}
		}
		else{
			return WID_UNKNOWN_ID;
		}
	}
	else{
		return WID_UNKNOWN_ID;
	}

}

int parse_str_by_character(char* str, char *arg, char ch){
	int len = strlen(str);
	int i = 0;
	int c = 0;
	char *t_str = str;
	char *t_arg = arg;
	int ret = 0;
	while(len > 0){
		for(i=0;i<len;i++){
			////printf("%c",t_str[i]);
			if(t_str[i] == ch){
				memcpy(t_arg, t_str,i);
				len = len-i-1;
				t_str = t_str + i+1;
				t_arg = t_arg + i;
				memcpy(t_arg,"%d",2);
				t_arg = t_arg +2;
				c++;
				i = 0;
				break;
			}else if(t_str[i] == '$'){		
				memcpy(t_arg, t_str,i);
				len = len-i-1;
				t_str = t_str + i+1;
				t_arg = t_arg + i;
				sprintf(t_arg," \n");
				t_arg = t_arg+ 2;
				i = 0 ;
				break;
			}
		}
		if(i == len){
			memset(t_arg, 0, i+1);
			memcpy(t_arg, t_str,i);
			break;
		}
	}
	return c;
}

int parse_daemonlog_level(int Loglevel, char *arg){


			if(Loglevel == 0)
			{
				strcpy(arg, "CLOSE");
			}
			else if(Loglevel == WID_ALL)
			{
				strcpy(arg, "ALL");
			}
			else{ 
				if(Loglevel & WID_DEFAULT)
				{
					strncat(arg, "DEFAULT,",8);
				}
				if(Loglevel & WID_DBUS)
				{
					strncat(arg, "DBUS,",5);
				}
				if(Loglevel & WID_WTPINFO)
				{
					strncat(arg, "WTPINFO,",8);
				}
				if(Loglevel & WID_MB)
				{
					strncat(arg, "MB",2);
				}
			}
/*
			if(Loglevel == WID_DEFAULT)
			{
				strcpy(arg, "DEFAULT");
			}
			else if(Loglevel == WID_DBUS)
			{
				strcpy(arg, "DBUS");
			}
			else if(Loglevel == WID_WTPINFO)
			{
				strcpy(arg, "WTPINFO");
			}
			else if(Loglevel == WID_MB)
			{
				strcpy(arg, "MB");
			}
			else if(Loglevel == WID_ALL)
			{
				strcpy(arg, "ALL");
			}
			else{
				strcpy(arg, "CLOSE");
			}*/
	return 0;		

}
typedef enum{
	check_id=0,
	check_comma,
	check_fail,
	check_end,
	check_success,
	check_bar
}check_state;

int dcli_ac_method_parse(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,"show_attack_mac_list"))
	{
		//printf("before dcli_sn is %d.\n",sn);
		sn = 1;
		//printf("after dcli_sn is %d.\n",sn);
	}		
	else if (!strcmp(DBUS_METHOD,"show_legal_essid_list"))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,"show_manufacturer_oui_list"))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,"show_mac_whitelist"))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,"show_mac_blacklist"))
	{
		sn = 5;
	}	
	else //if(!strcmp(DBUS_METHOD,"show_rogue_ap_list"))
	{
		//sn = 6;
	}
	return sn;
}
int dcli_ac_method_parse_two(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST))
	{
		//printf("before dcli_sn is %d.\n",sn);
		sn = 1;
		//printf("after dcli_sn is %d.\n",sn);
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID))
	{
		sn = 3;
	}
	else if ((!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID))||(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2)))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID))
	{
		sn = 6;
	}
	return sn;
}

int dcli_ac_method_parse_three(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST))
	{
		sn = 6;
	}
	return sn;
}


int dcli_ac_method_parse_four(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_MODEL))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_AC_METHOD_VRRP_SOCK_INFO))
	{
		sn = 5;
	}	
	else
	{
		//sn = 6;
	}
	return sn;
}


int dcli_ac_method_parse_five(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WIDCONFIG))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO))
	{
		sn = 6;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD))
	{
		sn = 7;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG))
	{
		sn = 8;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL))
	{
		sn = 9;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_AC_METHOD_VRRP_INFO))
	{
		sn = 10;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER))
	{
		sn = 11;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT))
	{
		sn = 12;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION))
	{
		sn = 13;
	}
	return sn;
}
int dcli_add_attack_ap_mac(unsigned char mac[],ATTACK_MAC_LIST_S	**attacklist){
//printf("111111111111111111\n");
	int ret;
	struct attack_mac_node *node=NULL;
//	ATTACK_MAC_LIST_S	attacklist;/*最终得在函数外定义*/
	if(mac==NULL)
		return -1;
//printf("222222222222222222222\n");
/*	if(*attacklist == NULL) {
		//printf("attacklist is NULL\n");
		return -1;
	}*/
	if(0==(*attacklist)->list_len){
		CW_CREATE_OBJECT_ERR(node, struct attack_mac_node, return -1;);

		memset(node->mac,0,6);
		memcpy(node->mac,mac,6);
		
		node->next=NULL;

		(*attacklist)->list_len++;
		(*attacklist)->attack_mac_list=node;

	}else{
	/*	ret=get_mac_node(mac,NULL,NULL);
		if(0==ret)
			return 0;*/
	//	else{
			CW_CREATE_OBJECT_ERR(node, struct attack_mac_node, return -1;);
			
			memset(node->mac,0,6);
			memcpy(node->mac,mac,6);

			node->next=(*attacklist)->attack_mac_list;
			(*attacklist)->attack_mac_list=node;
			
			(*attacklist)->list_len++;
		//}

	}

	return 0;
}
int dcli_add_legal_essid(char *essid,ESSID_LIST_S **dcli_essid_list){

	int ret;
	struct essid_node *node=NULL;

	if(essid==NULL)
		return -1;

	if(0==(*dcli_essid_list)->list_len){
		CW_CREATE_OBJECT_ERR(node, struct essid_node, return -1;);
		memset(node,0,sizeof(struct essid_node));

		node->essid=(char *)malloc(strlen(essid)+1);
		if (node->essid == NULL)
		{
			CW_FREE_OBJECT(node);
			return -1;
		}
		memset(node->essid,0,strlen(essid)+1);
		memcpy(node->essid,essid,strlen(essid));

		node->len=strlen(essid);
		node->next=NULL;

		(*dcli_essid_list)->list_len++;
		(*dcli_essid_list)->essid_list = node;

	}else{
	/*	ret=get_essid_node(essid,NULL,NULL);
		if(0==ret)
			return 0;*/
	//	else{
			CW_CREATE_OBJECT_ERR(node, struct essid_node, return -1;);
			memset(node,0,sizeof(struct essid_node));
			
			node->essid=(char *)malloc(strlen(essid)+1);
			if (node->essid == NULL)
			{
				CW_FREE_OBJECT(node);
				return -1;
			}
			memset(node->essid,0,strlen(essid)+1);
			memcpy(node->essid,essid,strlen(essid));
			
			node->len=strlen(essid);

			node->next=(*dcli_essid_list)->essid_list;
			(*dcli_essid_list)->essid_list=node;
			
			(*dcli_essid_list)->list_len++;
			
	//	}

	}

	return 0;
}


int dcli_add_manufacturer_oui(unsigned char oui[],OUI_LIST_S **oui_list){

	int ret;
	struct oui_node *node=NULL;

	if(oui==NULL)
		return -1;

	if(0==(*oui_list)->list_len){
		CW_CREATE_OBJECT_ERR(node, struct oui_node, return -1;);
		node->oui[0]=oui[0];
		node->oui[1]=oui[1];
		node->oui[2]=oui[2];
		node->next=NULL;

		(*oui_list)->list_len++;
		(*oui_list)->oui_list=node;

	}else{
			CW_CREATE_OBJECT_ERR(node, struct oui_node, return -1;);
			node->oui[0]=oui[0];
			node->oui[1]=oui[1];
			node->oui[2]=oui[2];

			node->next=(*oui_list)->oui_list;
			(*oui_list)->oui_list=node;
			
			(*oui_list)->list_len++;			
		
	}

	return 0;
}

int dcli_add_ap_statics_node(Ap_statics_INFOS **list,wlan_stats_info *ele){

	int ret;
	wlan_stats_info *tmp = NULL;
	//wlan_stats_info *node=NULL;

	if(ele==NULL)
		return -1;

	tmp = (*list)->ap_statics_ele;
	if(tmp == NULL){
		//printf("if 1111111111\n");
		(*list)->count++;
		(*list)->ap_statics_ele = ele;
		ele->next = NULL;
	}else{
		//printf("if 2222222222222\n");
		while(tmp->next)
			tmp = tmp->next;

		tmp->next = ele;
		ele->next = NULL;
		(*list)->count++;
	}
	//printf("count = %d\n",(*list)->count);
	#if 0
	if(0==(*list)->count){
	//	CW_CREATE_OBJECT_ERR(node, wlan_stats_info, return -1;);
	//	node->oui[0]=oui[0];
	//	node->oui[1]=oui[1];
	//	node->oui[2]=oui[2];
		ele->next=NULL;

		(*list)->count++;
		(*list)->ap_statics_ele = ele;

	}else{
		//	CW_CREATE_OBJECT_ERR(node, struct oui_node, return -1;);
		//	node->oui[0]=oui[0];
		//	node->oui[1]=oui[1];
		//	node->oui[2]=oui[2];
		while((*list)->ap_statics_ele->next){
			(*list)->ap_statics_ele = (*list)->ap_statics_ele->next;
		}
		(*list)->ap_statics_ele->next = ele;
		ele->next = NULL;
		/*	ele->next=(*list)->ap_statics_ele;
			(*list)->ap_statics_ele=ele;
			
			(*list)->count++;		*/	
		
	}
	#endif
	return 0;
}

int dcli_add_bak_sock_node(Bak_Sock_INFO **b_list,struct bak_sock *b_sock){

	int ret;
	struct bak_sock *node=NULL;

	if(b_sock == NULL)
		return -1;

	if(0==(*b_list)->list_len){
		CW_CREATE_OBJECT_ERR(node, struct bak_sock, return -1;);
		node->sock = b_sock->sock;
		node->ip = b_sock->ip;
		node->next=NULL;

		(*b_list)->list_len++;
		(*b_list)->b_sock_node = node;

	}else{
			CW_CREATE_OBJECT_ERR(node, struct bak_sock, return -1;);
			node->sock = b_sock->sock;
			node->ip = b_sock->ip;

			node->next=(*b_list)->b_sock_node;
			(*b_list)->b_sock_node = node;
			
			(*b_list)->list_len++;			
		
	}

	return 0;
}

int dcli_add_ap_conf_info_node(Config_Ver_Info **list,CWConfigVersionInfo_dcli *ele){

	int ret;
	CWConfigVersionInfo_dcli *node=NULL;
	if(ele==NULL)
		return -1;
	if(0==(*list)->list_len){
		CW_CREATE_OBJECT_ERR(node, CWConfigVersionInfo_dcli, return -1;);
		memset(node, 0, sizeof(CWConfigVersionInfo_dcli));
		/*node->str_ap_model = ele->str_ap_model;
		node->str_ap_code = ele->str_ap_code;
		node->str_ap_version_name = ele->str_ap_version_name;
		node->str_ap_version_path = ele->str_ap_version_path;*/
		node->radio_num = ele->radio_num;
		node->bss_num = ele->bss_num;
		//node->apcodeflag = ele->apcodeflag;		//book del
		node->str_ap_model = (char*)malloc(strlen(ele->str_ap_model));
		if (node->str_ap_model == NULL)
		{
			CW_FREE_OBJECT(node);
			return -1;
		}
		memset(node->str_ap_model,0,strlen(ele->str_ap_model)+1);
		memcpy(node->str_ap_model,ele->str_ap_model,strlen(ele->str_ap_model));
		
		node->str_ap_version_name = (char*)malloc(strlen(ele->str_ap_version_name));
		if (node->str_ap_version_name == NULL)
		{			
			CW_FREE_OBJECT(node->str_ap_model);
			free(node);
			return -1;
		}
		memset(node->str_ap_version_name,0,strlen(ele->str_ap_version_name)+1);
		memcpy(node->str_ap_version_name,ele->str_ap_version_name,strlen(ele->str_ap_version_name));
		
		node->str_ap_version_path = (char*)malloc(strlen(ele->str_ap_version_path));
		if (node->str_ap_version_path == NULL)
		{		
			CW_FREE_OBJECT(node->str_ap_version_name);
			free(node->str_ap_model);
			free(node);
			return -1;
		}
		memset(node->str_ap_version_path,0,strlen(ele->str_ap_version_path)+1);
		memcpy(node->str_ap_version_path,ele->str_ap_version_path,strlen(ele->str_ap_version_path));

		/*
		node->str_ap_code = (char*)malloc(strlen(ele->str_ap_code));
		memset(node->str_ap_code,0,strlen(ele->str_ap_code)+1);
		memcpy(node->str_ap_code,ele->str_ap_code,strlen(ele->str_ap_code));
		*/
		node->next = NULL;
		(*list)->list_len++;
		(*list)->config_ver_node = node;

	}else{
			CW_CREATE_OBJECT_ERR(node, CWConfigVersionInfo_dcli, return -1;);
			memset(node, 0, sizeof(CWConfigVersionInfo_dcli));
		/*	node->str_ap_model = ele->str_ap_model;
			node->str_ap_code = ele->str_ap_code;
			node->str_ap_version_name = ele->str_ap_version_name;
			node->str_ap_version_path = ele->str_ap_version_path;*/
			node->radio_num = ele->radio_num;
			node->bss_num = ele->bss_num;
			//node->apcodeflag = ele->apcodeflag; 	
			node->str_ap_model = (char*)malloc(strlen(ele->str_ap_model));
			if (node->str_ap_model == NULL)
			{
				CW_FREE_OBJECT(node);
				return -1;
			}
			memset(node->str_ap_model,0,strlen(ele->str_ap_model)+1);
			memcpy(node->str_ap_model,ele->str_ap_model,strlen(ele->str_ap_model));
			
			node->str_ap_version_name = (char*)malloc(strlen(ele->str_ap_version_name));
			if (node->str_ap_version_name == NULL)
			{			
				free(node->str_ap_model);
				CW_FREE_OBJECT(node);
				return -1;
			}
			memset(node->str_ap_version_name,0,strlen(ele->str_ap_version_name)+1);
			memcpy(node->str_ap_version_name,ele->str_ap_version_name,strlen(ele->str_ap_version_name));
			
			node->str_ap_version_path = (char*)malloc(strlen(ele->str_ap_version_path));
			if (node->str_ap_version_path == NULL)
			{		
				free(node->str_ap_model);
				free(node->str_ap_version_name);
				CW_FREE_OBJECT(node);
				return -1;
			}
			memset(node->str_ap_version_path,0,strlen(ele->str_ap_version_path)+1);
			memcpy(node->str_ap_version_path,ele->str_ap_version_path,strlen(ele->str_ap_version_path));
			/*
			node->str_ap_code = (char*)malloc(strlen(ele->str_ap_code));
			memset(node->str_ap_code,0,strlen(ele->str_ap_code)+1);
			memcpy(node->str_ap_code,ele->str_ap_code,strlen(ele->str_ap_code));
			*/
			node->next=(*list)->config_ver_node;
			(*list)->config_ver_node = node;
			
			(*list)->list_len++;	
	}
	return 0;
}

int dcli_add_rogue_ap_ele_fun(Neighbor_AP_INFOS **paplist,struct Neighbor_AP_ELE *elem)
{
	if((elem == NULL)||((*paplist) == NULL))
	{
		//printf("insert_elem_into_ap_list_heads parameter error\n");
		return -1;
	}	

	if((*paplist)->neighborapInfosCount == 0)
	{
		(*paplist)->neighborapInfos = elem;
		(*paplist)->neighborapInfosCount++;	
		return 0;
	}
	struct Neighbor_AP_ELE *pnode = (*paplist)->neighborapInfos;

	(*paplist)->neighborapInfos = elem;
	elem->next = pnode;
	(*paplist)->neighborapInfosCount++;	
	return 0;


}

int dcli_add_wids_ap_ele_fun(wid_wids_device **paplist,struct tag_wids_device_ele *elem)
{
	if((elem == NULL)||((*paplist) == NULL))
	{
		return -1;
	}	
	if((*paplist)->count== 0)
	{
		(*paplist)->wids_device_info= elem;
		(*paplist)->count++;	
		return 0;
	}
	struct tag_wids_device_ele *pnode = (*paplist)->wids_device_info;

	(*paplist)->wids_device_info = elem;
	elem->next = pnode;
	(*paplist)->count++;	
	return 0;


}
int dcli_ac_add_auto_ap_if_node(wid_auto_ap_info *auto_login,wid_auto_ap_if	*auto_ap_if)
	{
		if((auto_ap_if == NULL)||(auto_login == NULL))
		{
			return -1;
		}	
		//printf("add ifname is %s\n",auto_ap_if->ifname);
		if(auto_login->list_len == 0)
		{
			auto_login->auto_ap_if = auto_ap_if;
			auto_ap_if->ifnext = NULL;
			auto_login->list_len++;
			//printf("firt node,add ifname is %s\n",auto_login->auto_ap_if->ifname);
			return 0;
		}
		//wid_auto_ap_info *tmp = NULL;
		/*tmp = auto_login;
		while(tmp->auto_ap_if->ifnext){
			tmp->auto_ap_if = tmp->auto_ap_if->ifnext;
		}
		tmp->auto_ap_if->ifnext = auto_ap_if;
		auto_ap_if->ifnext = NULL;
		tmp->list_len++;*/
		wid_auto_ap_if *tmp=NULL;
		tmp = auto_login->auto_ap_if;
		while(tmp->ifnext){
			tmp = tmp->ifnext;
		}
		tmp->ifnext = auto_ap_if;
		auto_ap_if->ifnext = NULL;
		auto_login->list_len++;
		//printf("tmp,other node,add ifname is %s\n",tmp->ifname);
		//printf("firt node of list,ifname is %s\n",auto_login->auto_ap_if->ifname);
		return 0;
	
	
	}
void* dcli_ac_show_api_group_one(
	int index,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* ret,
	unsigned int* num2,
	unsigned int* num3,
	//DCLI_AC_API_GROUP_ONE **LIST,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int num;
	int i = 0;
	char en[] = "enable";
	char dis[] = "disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
//	int state = 1;
	DCLI_AC_API_GROUP_ONE  *LIST = NULL;
	unsigned int dcli_sn = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int localid = *num3;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");
	dcli_sn = dcli_ac_method_parse(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 2)||(dcli_sn == 3)||(dcli_sn == 4)||(dcli_sn == 5)){	
		//printf("aaaaaaaaaaaaaaaaaaa\n");
		int state = 1;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&state,
								 DBUS_TYPE_INVALID);
		
	}
/*	else if(dcli_sn == 2){
		//printf("sn=2 22222222\n");
		int state = 1;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ESSID_SHOW);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 3){
		//printf("sn=3  333333333\n");
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_OUI_SHOW);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 4){
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_SHOW);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 5){
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_SHOW);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}
*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	
	if((*ret) == 0){
		
		CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_API_GROUP_ONE, return NULL;);	
		LIST->dcli_attack_mac_list = NULL;
		LIST->dcli_essid_list = NULL;
		LIST->dcli_oui_list = NULL;
	switch(dcli_sn)
	{
		case 1 :
			{	
			//printf("ret = 0  sn=1   111111111111\n");
					CW_CREATE_OBJECT_ERR(LIST->dcli_attack_mac_list, ATTACK_MAC_LIST_S, return NULL;);	
					LIST->dcli_attack_mac_list->list_len = 0;
					LIST->dcli_attack_mac_list->attack_mac_list = NULL;
				unsigned char mac[DCLIAC_MAC_LEN];	
				dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
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
						dcli_add_attack_ap_mac(mac,&(LIST->dcli_attack_mac_list));
				}
				
		}
			break;
		case 2 :
			{
			//printf("ret = 0  sn=2 22222222\n");
				char *essid=NULL;
					CW_CREATE_OBJECT_ERR(LIST->dcli_essid_list, ESSID_LIST_S, return NULL;);	
					LIST->dcli_essid_list->list_len = 0;
					LIST->dcli_essid_list->essid_list = NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);

					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					for (i = 0; i < num; i++)
					{
						DBusMessageIter iter_struct;
						dbus_message_iter_recurse(&iter_array,&iter_struct);

						dbus_message_iter_get_basic(&iter_struct,&(essid));
						dbus_message_iter_next(&iter_array);

						dcli_add_legal_essid(essid,&(LIST->dcli_essid_list));

				}
	}
			break;
		case 3 :
			{
		//printf("ret = 0  sn=3  33333333333\n");
				unsigned char oui[DCLIAC_MAC_LEN];	
					CW_CREATE_OBJECT_ERR(LIST->dcli_oui_list, OUI_LIST_S, return NULL;);	
					LIST->dcli_oui_list->list_len=0;
					LIST->dcli_oui_list->oui_list=NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);

					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					for (i = 0; i < num; i++)
					{
						DBusMessageIter iter_struct;
						dbus_message_iter_recurse(&iter_array,&iter_struct);
					
						dbus_message_iter_get_basic(&iter_struct,&(oui[0]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(oui[1]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(oui[2]));
						dbus_message_iter_next(&iter_array);

						dcli_add_manufacturer_oui(oui,&(LIST->dcli_oui_list));
					}
		}
			break;
		case 4 :
			{
				unsigned char whitemac[DCLIAC_MAC_LEN]; 
					CW_CREATE_OBJECT_ERR(LIST->dcli_attack_mac_list, ATTACK_MAC_LIST_S, return NULL;);	
					LIST->dcli_attack_mac_list->list_len = 0;
					LIST->dcli_attack_mac_list->attack_mac_list = NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					
					for (i = 0; i < num; i++)
				{
						DBusMessageIter iter_struct;
						dbus_message_iter_recurse(&iter_array,&iter_struct);
					
						dbus_message_iter_get_basic(&iter_struct,&(whitemac[0]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(whitemac[1]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(whitemac[2]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(whitemac[3]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(whitemac[4]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(whitemac[5]));
		
						dbus_message_iter_next(&iter_array);
							dcli_add_attack_ap_mac(whitemac,&(LIST->dcli_attack_mac_list));
		
				}
				}
			break;
		case 5 :
			{
				unsigned char blackmac[DCLIAC_MAC_LEN]; 
					CW_CREATE_OBJECT_ERR(LIST->dcli_attack_mac_list, ATTACK_MAC_LIST_S, return NULL;);	
					LIST->dcli_attack_mac_list->list_len = 0;
					LIST->dcli_attack_mac_list->attack_mac_list = NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					for (i = 0; i < num; i++)
				{
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(blackmac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(blackmac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(blackmac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(blackmac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(blackmac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(blackmac[5]));
		
					dbus_message_iter_next(&iter_array);
						dcli_add_attack_ap_mac(blackmac,&(LIST->dcli_attack_mac_list));
		
				}
			}
			break;
		case 6 :
			{

			}
			break;
		case 7 :
			{

			}
			break;
		default :	break;

	}
/*
	if(ret == 0 )
	{	
		if(dcli_sn == 1){	
			//printf("ret = 0  sn=1   111111111111\n");
				CW_CREATE_OBJECT_ERR((*LIST)->dcli_attack_mac_list, ATTACK_MAC_LIST_S, return NULL;);	
				(*LIST)->dcli_attack_mac_list->list_len = 0;
				(*LIST)->dcli_attack_mac_list->attack_mac_list = NULL;
				unsigned char mac[DCLIAC_MAC_LEN];	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num1);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
			//	vty_out(vty,"Attack mac list \n");
				for (i = 0; i < num1; i++)
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
		
		
			//		vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",i+1,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		
					dcli_add_attack_ap_mac(mac,&((*LIST)->dcli_attack_mac_list));
				}
				
		}
		else if(dcli_sn == 2){
			//printf("ret = 0  sn=2 22222222\n");
					char *essid=NULL;
					CW_CREATE_OBJECT_ERR((*LIST)->dcli_essid_list, ESSID_LIST_S, return NULL;);	
					(*LIST)->dcli_essid_list->list_len = 0;
					(*LIST)->dcli_essid_list->essid_list = NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num1);

					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);

				//	vty_out(vty,"Legal essid list\n");
					for (i = 0; i < num1; i++)
					{
						DBusMessageIter iter_struct;
						
						
						dbus_message_iter_recurse(&iter_array,&iter_struct);

						dbus_message_iter_get_basic(&iter_struct,&(essid));
						dbus_message_iter_next(&iter_array);

						dcli_add_legal_essid(essid,&((*LIST)->dcli_essid_list));

					}
	}
	else if(dcli_sn == 3){
		//printf("ret = 0  sn=3  33333333333\n");
				unsigned char oui[DCLIAC_MAC_LEN];	
				CW_CREATE_OBJECT_ERR((*LIST)->dcli_oui_list, OUI_LIST_S, return NULL;);	
				(*LIST)->dcli_oui_list->list_len=0;
				(*LIST)->dcli_oui_list->oui_list=NULL;
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num1);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
		//		vty_out(vty,"Legal manufacturer (OUI) list\n");
				for (i = 0; i < num1; i++)
				{
					DBusMessageIter iter_struct;
					
					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(oui[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(oui[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(oui[2]));
					dbus_message_iter_next(&iter_array);

					dcli_add_manufacturer_oui(oui,&((*LIST)->dcli_oui_list));
					}
		}
	else if(dcli_sn == 4){
		unsigned char whitemac[DCLIAC_MAC_LEN]; 
		CW_CREATE_OBJECT_ERR((*LIST)->dcli_attack_mac_list, ATTACK_MAC_LIST_S, return NULL;);	
		(*LIST)->dcli_attack_mac_list->list_len = 0;
		(*LIST)->dcli_attack_mac_list->attack_mac_list = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num1);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num1; i++)
		{
				DBusMessageIter iter_struct;
				
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(whitemac[0]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(whitemac[1]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(whitemac[2]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(whitemac[3]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(whitemac[4]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(whitemac[5]));

				dbus_message_iter_next(&iter_array);

				dcli_add_attack_ap_mac(whitemac,&((*LIST)->dcli_attack_mac_list));

		}
	}
	else if (dcli_sn == 5){

		unsigned char blackmac[DCLIAC_MAC_LEN]; 
		CW_CREATE_OBJECT_ERR((*LIST)->dcli_attack_mac_list, ATTACK_MAC_LIST_S, return NULL;);	
		(*LIST)->dcli_attack_mac_list->list_len = 0;
		(*LIST)->dcli_attack_mac_list->attack_mac_list = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num1);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
	//	vty_out(vty,"black list is \n");
	//	vty_out(vty,"%-17s \n","mac:");
		for (i = 0; i < num1; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[5]));

			dbus_message_iter_next(&iter_array);

			dcli_add_attack_ap_mac(blackmac,&((*LIST)->dcli_attack_mac_list));

		}
	}
	}*/
	}
	dbus_message_unref(reply);
	return LIST;

}

//fengwenchao add 20101220
void* show_neighbor_ap_list_cmd_allap(int localid,int index,DBusConnection *dbus_connection,int* wtp_num,int* ret,int* ret1)
{
	DBusMessage *query, *reply; 
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter  iter_sub_struct;
	DBusMessageIter  iter_sub_sub_array;
	DBusMessageIter	 iter_sub_sub_struct;
	DBusMessageIter  iter_sub_sub_sub_array;
	DBusMessageIter  iter_sub_sub_sub_struct;

	DBusError err;

	int i,j,k,q;
	unsigned char radio_num = 0;
	//int rouge_ap_count = 0;
	unsigned char* essid =NULL;
	unsigned char nullessid =0;
	char* IEs_INFO = NULL;
	//char* zeroessid = " 0 ";
	char* zeroessid = NULL;
	
	//DCLI_AC_API_GROUP_TWO *LIST = NULL;	

	struct allwtp_neighborap *all_wtp_neighbor_ap = NULL;
	struct allwtp_neighborap *all_wtp_neighbor_apNODE = NULL;
	struct allwtp_neighborap_radioinfo	*radioinfo_node = NULL;
	struct Neighbor_AP_ELE *neighborapInfos_node = NULL;
	int state = 1;

	//int ret1 = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);


	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST);
	dbus_error_init(&err);									
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}	

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);

	//printf("ret =  %d\n",*ret);
	//printf("wtp_num =  %d\n",*wtp_num);

	if(*ret == 0)
	{
		//printf("accessinto ret = 0\n");
		if((all_wtp_neighbor_ap = (struct allwtp_neighborap *)malloc(sizeof(struct allwtp_neighborap))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		
		memset(all_wtp_neighbor_ap,0,sizeof(struct allwtp_neighborap));
		all_wtp_neighbor_ap->allwtp_neighborap_list= NULL;
		all_wtp_neighbor_ap->allwtp_neighborap_last= NULL;
		all_wtp_neighbor_ap->radioinfo_head = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0;i<(*wtp_num);i++)
		{
			if((all_wtp_neighbor_apNODE = (struct allwtp_neighborap*)malloc(sizeof(struct allwtp_neighborap))) == NULL)
				{
					dcli_free_allwtp_neighbor_ap(all_wtp_neighbor_ap);   
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
			memset(all_wtp_neighbor_apNODE,0,sizeof(struct allwtp_neighborap));
			all_wtp_neighbor_apNODE->next = NULL;
			all_wtp_neighbor_apNODE->radioinfo_head = NULL;

			if(all_wtp_neighbor_ap->allwtp_neighborap_list == NULL){
				all_wtp_neighbor_ap->allwtp_neighborap_list = all_wtp_neighbor_apNODE;
				all_wtp_neighbor_ap->next = all_wtp_neighbor_apNODE;
			}
			else{
				all_wtp_neighbor_ap->allwtp_neighborap_last->next = all_wtp_neighbor_apNODE;
			}
			all_wtp_neighbor_ap->allwtp_neighborap_last = all_wtp_neighbor_apNODE;
			
			
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->wtpid));

			//printf("all_wtp_neighbor_apNODE->wtpid =  %d \n",all_wtp_neighbor_apNODE->wtpid);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->WTPmac[0]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->WTPmac[1]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->WTPmac[2]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->WTPmac[3]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->WTPmac[4]));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(all_wtp_neighbor_apNODE->WTPmac[5]));

			//printf("all_wtp_neighbor_apNODE->WTPmac =  %s \n",all_wtp_neighbor_apNODE->WTPmac);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&all_wtp_neighbor_apNODE->radio_num);	

			//printf("all_wtp_neighbor_apNODE->radio_num =  %d \n",all_wtp_neighbor_apNODE->radio_num);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0;j<all_wtp_neighbor_apNODE->radio_num;j++)
			{
				if((radioinfo_node = (struct allwtp_neighborap_radioinfo*)malloc(sizeof(struct allwtp_neighborap_radioinfo))) == NULL)
				{
					dcli_free_allwtp_neighbor_ap(all_wtp_neighbor_ap); 
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
				   memset(radioinfo_node,0,sizeof(struct allwtp_neighborap_radioinfo));
				   radioinfo_node->next = NULL;
				   radioinfo_node->radioinfo_list = NULL;
				   radioinfo_node->radioinfo_last = NULL;
				   radioinfo_node->neighborapInfos_head=NULL;

				   
                  if(all_wtp_neighbor_apNODE->radioinfo_head== NULL){
					   all_wtp_neighbor_apNODE->radioinfo_head = radioinfo_node;
					   //all_wtp_neighbor_apNODE->next = radioinfo_node;
				    }
				    else{
					all_wtp_neighbor_apNODE->radioinfo_head->radioinfo_last->next = radioinfo_node;
				   }
				
				 all_wtp_neighbor_apNODE->radioinfo_head->radioinfo_last = radioinfo_node;

			
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&radioinfo_node->wtpWirelessIfIndex);

				//printf("radioinfo_node->wtpWirelessIfIndex  =   %d\n",radioinfo_node->wtpWirelessIfIndex);
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&radioinfo_node->failreason);

				//printf("radioinfo_node->failreason =  %d \n",radioinfo_node->failreason);

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&radioinfo_node->rouge_ap_count);

				//printf("radioinfo_node->rouge_ap_count  =   %d \n",radioinfo_node->rouge_ap_count);

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);

				if((radioinfo_node->failreason == WID_DBUS_SUCCESS)&&(radioinfo_node->rouge_ap_count != 0))
				{
					// printf("accessinto ret1 = WID_DBUS_SUCCESS   rouge_ap_count != 0  \n");
					for(k=0;k<radioinfo_node->rouge_ap_count;k++)
					{
						  if((neighborapInfos_node = (struct Neighbor_AP_ELE*)malloc(sizeof(struct Neighbor_AP_ELE))) == NULL){
								dcli_free_allwtp_neighbor_ap(all_wtp_neighbor_ap);  
					           *ret = MALLOC_ERROR;
					            dbus_message_unref(reply);
					            return NULL;
				               }
						  
							memset(neighborapInfos_node,0,sizeof(struct Neighbor_AP_ELE));
                             
							neighborapInfos_node->next = NULL;
					    	neighborapInfos_node->neighborapInfos_list = NULL;
							neighborapInfos_node->neighborapInfos_last = NULL;


							if(radioinfo_node->neighborapInfos_head == NULL){
								radioinfo_node->neighborapInfos_head = neighborapInfos_node;
								// radioinfo_node->next = neighborapInfos_node;
							}else{
								radioinfo_node->neighborapInfos_head->neighborapInfos_last->next = neighborapInfos_node;
							}
								
							 radioinfo_node->neighborapInfos_head->neighborapInfos_last = neighborapInfos_node;


							essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
							memset(essid,0,ESSID_DEFAULT_LEN+1);
						
							dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[0]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[1]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[2]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[3]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[4]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[5]);

							//printf("neighborapInfos_node->BSSID  =   %s\n",neighborapInfos_node->BSSID);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->wtpid);

							//printf("neighborapInfos_node->wtpid  =   %d\n",neighborapInfos_node->wtpid);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->Rate);

							//printf("neighborapInfos_node->Rate  =   %d\n",neighborapInfos_node->Rate);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->Channel);

							//printf("neighborapInfos_node->Channel =   %d\n",neighborapInfos_node->Channel);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->RSSI);

							//printf("neighborapInfos_node->RSSI =   %d\n",neighborapInfos_node->RSSI);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->NOISE);

							//printf("neighborapInfos_node->NOISE =   %d\n",neighborapInfos_node->NOISE);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BEACON_INT);

							//printf("neighborapInfos_node->BEACON_INT =   %d\n",neighborapInfos_node->BEACON_INT);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->status);

							//printf("neighborapInfos_node->status =   %d\n",neighborapInfos_node->status);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->opstatus);

							//printf("neighborapInfos_node->opstatus =   %d\n",neighborapInfos_node->opstatus);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->capabilityinfo);

							//printf("neighborapInfos_node->capabilityinfo =   %d\n",neighborapInfos_node->capabilityinfo);

							dbus_message_iter_next(&iter_sub_sub_struct);

								
							dbus_message_iter_recurse(&iter_sub_sub_struct,&iter_sub_sub_sub_array);

							for(q=0;q<ESSID_DEFAULT_LEN+1;q++)
								{
									dbus_message_iter_recurse(&iter_sub_sub_sub_array,&iter_sub_sub_sub_struct);
									dbus_message_iter_get_basic(&iter_sub_sub_sub_struct,&neighborapInfos_node->ESSID[q]);
									dbus_message_iter_next(&iter_sub_sub_sub_struct);
									dbus_message_iter_next(&iter_sub_sub_sub_array);
								}
							dbus_message_iter_next(&iter_sub_sub_struct);
							/*neighborapInfos_node->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
							memset(neighborapInfos_node->ESSID,0,ESSID_DEFAULT_LEN+1);
							memcpy(neighborapInfos_node->ESSID,essid,ESSID_DEFAULT_LEN);*/

							dbus_message_iter_get_basic(&iter_sub_sub_struct,&IEs_INFO);/*(rouge_ap->IEs_INFO)*/
							dbus_message_iter_next(&iter_sub_sub_array);

							neighborapInfos_node->IEs_INFO = (char*)malloc(strlen(IEs_INFO)+1);
							memset(neighborapInfos_node->IEs_INFO,0,strlen(IEs_INFO)+1);
							memcpy(neighborapInfos_node->IEs_INFO,IEs_INFO,strlen(IEs_INFO));

							//printf(" neighborapInfos_node->ESSID =  %s \n",neighborapInfos_node->ESSID);
							//printf("  neighborapInfos_node->IEs_INFO  =  %s  \n",neighborapInfos_node->IEs_INFO);

							if(essid)
								{
									free(essid);
									essid =NULL;
								}						
					}					
				}
				else
					{
							if((neighborapInfos_node = (struct Neighbor_AP_ELE*)malloc(sizeof(struct Neighbor_AP_ELE))) == NULL){
								dcli_free_allwtp_neighbor_ap(all_wtp_neighbor_ap);  
					           *ret = MALLOC_ERROR;
					            dbus_message_unref(reply);
					            return NULL;
				               }
						  
							memset(neighborapInfos_node,0,sizeof(struct Neighbor_AP_ELE));
                             
							neighborapInfos_node->next = NULL;
					    	neighborapInfos_node->neighborapInfos_list = NULL;
							neighborapInfos_node->neighborapInfos_last = NULL;


							if(radioinfo_node->neighborapInfos_head == NULL){
								radioinfo_node->neighborapInfos_head = neighborapInfos_node;
							}else{
								radioinfo_node->neighborapInfos_head->neighborapInfos_last->next = neighborapInfos_node;
							}
								
							 radioinfo_node->neighborapInfos_head->neighborapInfos_last = neighborapInfos_node;


							//essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
							//memset(essid,0,ESSID_DEFAULT_LEN+1);
						
							dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[0]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[1]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[2]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[3]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[4]);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BSSID[5]);

							//printf("neighborapInfos_node->BSSID  =   %s\n",neighborapInfos_node->BSSID);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->wtpid);

							//printf("neighborapInfos_node->wtpid  =   %d\n",neighborapInfos_node->wtpid);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->Rate);

							//printf("neighborapInfos_node->Rate  =   %d\n",neighborapInfos_node->Rate);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->Channel);

							//printf("neighborapInfos_node->Channel =   %d\n",neighborapInfos_node->Channel);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->RSSI);

						//	printf("neighborapInfos_node->RSSI =   %d\n",neighborapInfos_node->RSSI);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->NOISE);

						//	printf("neighborapInfos_node->NOISE =   %d\n",neighborapInfos_node->NOISE);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->BEACON_INT);

						//	printf("neighborapInfos_node->BEACON_INT =   %d\n",neighborapInfos_node->BEACON_INT);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->status);

						//	printf("neighborapInfos_node->status =   %d\n",neighborapInfos_node->status);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->opstatus);

						//	printf("neighborapInfos_node->opstatus =   %d\n",neighborapInfos_node->opstatus);

							dbus_message_iter_next(&iter_sub_sub_struct);
							dbus_message_iter_get_basic(&iter_sub_sub_struct,&neighborapInfos_node->capabilityinfo);

						//	printf("neighborapInfos_node->capabilityinfo =   %d\n",neighborapInfos_node->capabilityinfo);

							dbus_message_iter_next(&iter_sub_sub_struct);

								
							dbus_message_iter_recurse(&iter_sub_sub_struct,&iter_sub_sub_sub_array);

							//for(q=0;q<ESSID_DEFAULT_LEN;q++)
							//	{
									dbus_message_iter_recurse(&iter_sub_sub_sub_array,&iter_sub_sub_sub_struct);
									dbus_message_iter_get_basic(&iter_sub_sub_sub_struct,&nullessid/*[q]*/);
									dbus_message_iter_next(&iter_sub_sub_sub_struct);
									dbus_message_iter_next(&iter_sub_sub_sub_array);
							//	}
						//	printf("nullessid = %d\n",nullessid);
						//	printf("11111111111111111\n");
							dbus_message_iter_next(&iter_sub_sub_struct);
							//neighborapInfos_node->ESSID = (char*)malloc(strlen(zeroessid)+1);
						//	printf("222222222222222222222222\n");
						   // memset(neighborapInfos_node->ESSID,0,strlen(zeroessid)+1);

						//	printf("3333333333333333333333\n");
							//memcpy(neighborapInfos_node->ESSID,zeroessid,strlen(zeroessid));
						//	printf("4444444444444444444444\n");

							dbus_message_iter_get_basic(&iter_sub_sub_struct,&IEs_INFO);/*(rouge_ap->IEs_INFO)*/
							dbus_message_iter_next(&iter_sub_sub_array);

						//	printf("5555555555555555555\n");

							neighborapInfos_node->IEs_INFO = (char*)malloc(strlen(IEs_INFO)+1);
							memset(neighborapInfos_node->IEs_INFO,0,strlen(IEs_INFO)+1);
							memcpy(neighborapInfos_node->IEs_INFO,IEs_INFO,strlen(IEs_INFO));

							//printf(" neighborapInfos_node->ESSID =  %s \n",neighborapInfos_node->ESSID);
							//printf(" neighborapInfos_node->IEs_INFO  =  %s  \n",neighborapInfos_node->IEs_INFO);

							/*if(essid)
								{
									free(essid);
									essid =NULL;
								}	*/		
					}
				dbus_message_iter_next(&iter_sub_array);				
			}
			dbus_message_iter_next(&iter_array);		
		}
	}
	//if(all_wtp_neighbor_ap == NULL)
	//	printf("all_wtp_neighbor_ap =  NULL ");
	//printf("qqqqqqqqqqqqqqqqq");
	dbus_message_unref(reply);
	return all_wtp_neighbor_ap;
	
}
//fengwenchao add end

//fengwenchao add 20101227
DCLI_AC_API_GROUP_TWO * show_wids_device_of_all_device(int localid,DBusConnection *dcli_dbus_connection,int index,int* ret,unsigned int* lasttime)
{
	int num;
	int i = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	int state = 1;
	DCLI_AC_API_GROUP_TWO *LIST = NULL;
//	printf("111111111111111111111\n");

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_OF_ALL);
//	printf("eeeeeeeeeeeeeeeeeeeeeeeeeeeee\n");

	dbus_error_init(&err);	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);
	//printf("wwwwwwwwwwwwwwwwwwwwwwwwww\n");

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	//printf("qqqqqqqqqqqqqqqqqqqqqqq\n");

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,lasttime);	

	//printf("ret = %d \n",*ret);
	//printf("lasttime = %d \n",*lasttime);

	if(*ret == 0)
	{
		CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_API_GROUP_TWO, return NULL;);	
		LIST->rouge_ap_list = NULL;
		LIST->wids_device_list = NULL;
		//printf("zzzzzzzzzzzzzzzzzzzzzzzz\n");
		CW_CREATE_OBJECT_ERR(LIST->wids_device_list, wid_wids_device, return NULL;);	
		//printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
		LIST->wids_device_list->count = 0;
		LIST->wids_device_list->wids_device_info = NULL;
		//printf("cccccccccccccccccccccccccccccccccc\n");
		struct tag_wids_device_ele *wids_device = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
		//printf("22222222222222222\n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		//printf("33333333333333333333333\n");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			wids_device = (struct tag_wids_device_ele *)malloc(sizeof(struct tag_wids_device_ele));
			if((wids_device == NULL))
			{
				//printf("##malloc memory fail #\n");
				return NULL;
			}
		//	printf("444444444444444444444444\n");		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[5]));
			/*fengwenchao add 20110513*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->fst_attack));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->lst_attack));
			/*fengwenchao add end*/
		//	printf("55555555555555555555555\n");
			dbus_message_iter_next(&iter_array);
		
			if(dcli_add_wids_ap_ele_fun(&(LIST->wids_device_list),wids_device) == -1)
			{
				//printf("err here free.\n");
			}
		}
	}

	dbus_message_unref(reply);
	//printf("!!!!!!!!!!!!!!!!!!!\n");
	return LIST;
	
}

//fengwenchao add end
void* dcli_ac_show_api_group_two(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* ret,
	unsigned int* num2,
	unsigned int* num3,
	//DCLI_AC_API_GROUP_TWO *LIST,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int num;
	int i = 0;
	char en[] = "enable";
	char dis[] = "disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
//	int state = 1;
	DCLI_AC_API_GROUP_TWO *LIST = NULL;
	int wtpid = id1;
	unsigned int dcli_sn = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int localid = *num3;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");
	
	dcli_sn = dcli_ac_method_parse_two(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 2)){	
		//printf("aaaaaaaaaaaaaaaaaaa\n");
		int state = 1;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&state,
								 DBUS_TYPE_INVALID);
	}
	else if((dcli_sn == 3)||(dcli_sn == 4)||(dcli_sn == 5)||(dcli_sn == 6)){
		//printf("sn=3  333333333\n");
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_error_init(&err);			
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&wtpid,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);			
	}
/*	else if(dcli_sn == 4){
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&wtpid,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 5){
		//printf("11111111dcli_sn is 5\n");
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&wtpid,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 6){
		int state =1;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID);
		dbus_error_init(&err);	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_UINT32,&state,
								 DBUS_TYPE_INVALID);

	}
	else if(dcli_sn == 7){
		
	}*/

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	//printf("ret is %d,\n",ret);
	if(*ret == 0 )
	{	
		unsigned char* essid = NULL;
		char* IEs_INFO = NULL;
		CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_API_GROUP_TWO, return NULL;);	
		LIST->rouge_ap_list = NULL;
		LIST->wids_device_list = NULL;
		switch(dcli_sn){
				case 1 :
					{
					essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
					if (essid == NULL)
					{
						CW_FREE_OBJECT(LIST);
						return NULL;
					}
					memset(essid,0,ESSID_DEFAULT_LEN+1);
					CW_CREATE_OBJECT_ERR(LIST->rouge_ap_list, Neighbor_AP_INFOS, return NULL;); 	
					LIST->rouge_ap_list->neighborapInfosCount = 0;
					LIST->rouge_ap_list->neighborapInfos = NULL;
					struct Neighbor_AP_ELE *rouge_ap = NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					
		//printf("num is not %d.\n",num);
					for (i = 0; i < num; i++)
					{
						DBusMessageIter iter_struct;
						DBusMessageIter iter_sub_array;
						rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
						if((rouge_ap == NULL))
						{
							//printf("##malloc memory fail #\n");
							dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST,LIST);
							return NULL;
						}
						rouge_ap->next = NULL;
						memset(rouge_ap->ESSID,0,ESSID_DEFAULT_LEN+1);
						dbus_message_iter_recurse(&iter_array,&iter_struct);
					
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
						dbus_message_iter_next(&iter_struct);

						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
						dbus_message_iter_next(&iter_struct);	

						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
						dbus_message_iter_next(&iter_struct);				
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
						dbus_message_iter_next(&iter_struct);	
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
						dbus_message_iter_next(&iter_struct);	
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
						dbus_message_iter_next(&iter_struct);	
						
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
						dbus_message_iter_next(&iter_struct);	
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
						dbus_message_iter_next(&iter_struct);	
						dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
						dbus_message_iter_next(&iter_struct);

						dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
						int j = 0;
						for(j=0;j<ESSID_DEFAULT_LEN+1;j++){
							DBusMessageIter iter_sub_struct;
							dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							//dbus_message_iter_get_basic(&iter_sub_struct,&essid[j]);
							dbus_message_iter_get_basic(&iter_sub_struct,&rouge_ap->ESSID[j]);
							dbus_message_iter_next(&iter_sub_struct);
							dbus_message_iter_next(&iter_sub_array);
						}
							/*rouge_ap->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
							memset(rouge_ap->ESSID,0,ESSID_DEFAULT_LEN+1);
							memcpy(rouge_ap->ESSID,essid,ESSID_DEFAULT_LEN);*/
						dbus_message_iter_next(&iter_struct);


						dbus_message_iter_get_basic(&iter_struct,&IEs_INFO);/*(rouge_ap->IEs_INFO)*/
						dbus_message_iter_next(&iter_array);

							rouge_ap->IEs_INFO = (char*)malloc(strlen(IEs_INFO)+1);
							memset(rouge_ap->IEs_INFO,0,strlen(IEs_INFO)+1);
							memcpy(rouge_ap->IEs_INFO,IEs_INFO,strlen(IEs_INFO));

						if(dcli_add_rogue_ap_ele_fun(&(LIST->rouge_ap_list),rouge_ap) == -1)
						{
							//printf("err here free.\n");
							//CW_FREE_OBJECT(rouge_ap->ESSID);
							dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST,LIST);
							CW_FREE_OBJECT(rouge_ap->IEs_INFO);
						
							CW_FREE_OBJECT(rouge_ap);
							return NULL;
						}

					}
					if(essid){
						free(essid);
						essid = NULL;
					} 
				}
				break;
			case 2 :
				{
					essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
					if (essid == NULL)
					{
						CW_FREE_OBJECT(LIST);
						return NULL;
					}
					memset(essid,0,ESSID_DEFAULT_LEN+1);
					CW_CREATE_OBJECT_ERR(LIST->rouge_ap_list, Neighbor_AP_INFOS, return NULL;);	
					LIST->rouge_ap_list->neighborapInfosCount = 0;
					LIST->rouge_ap_list->neighborapInfos = NULL;
					
					struct Neighbor_AP_ELE *rouge_ap = NULL;	
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);
					//fengwenchao add 20110402
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&LIST->rouge_ap_list->wtp_online_num);  
					//fengwenchao add end
					
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					
					for (i = 0; i < num; i++)
					{
						DBusMessageIter iter_struct;
						DBusMessageIter iter_sub_array;
						rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
						if((rouge_ap == NULL))
						{
							//printf("##malloc memory fail #\n");
							dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1,LIST);
							return NULL;
						}
						rouge_ap->next = NULL;
			
						
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
					dbus_message_iter_next(&iter_struct);
		
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
					dbus_message_iter_next(&iter_struct);	
		
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
					dbus_message_iter_next(&iter_struct);				
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
					dbus_message_iter_next(&iter_struct);	
					
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
					/*fengwenchao add 20110402*/
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RogueAPAttackedStatus));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RogueAPToIgnore));
					/*fengwenchao add end*/
					dbus_message_iter_next(&iter_struct);
		
							
					dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
					int j = 0;
					for(j=0;j<ESSID_DEFAULT_LEN+1;j++){
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
						//dbus_message_iter_get_basic(&iter_sub_struct,&essid[j]);
						dbus_message_iter_get_basic(&iter_sub_struct,&rouge_ap->ESSID[j]);
						dbus_message_iter_next(&iter_sub_struct);
						dbus_message_iter_next(&iter_sub_array);
					}
					dbus_message_iter_next(&iter_struct);

						/*rouge_ap->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
						memset(rouge_ap->ESSID,0,ESSID_DEFAULT_LEN+1);
						memcpy(rouge_ap->ESSID,essid,ESSID_DEFAULT_LEN);*/
			
					dbus_message_iter_get_basic(&iter_struct,&IEs_INFO);/*(rouge_ap->IEs_INFO)*/
					dbus_message_iter_next(&iter_struct);

						rouge_ap->IEs_INFO = (char*)malloc(strlen(IEs_INFO)+1);
						memset(rouge_ap->IEs_INFO,0,strlen(IEs_INFO)+1);
						memcpy(rouge_ap->IEs_INFO,IEs_INFO,strlen(IEs_INFO));
		
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->fst_dtc_tm));
					dbus_message_iter_next(&iter_struct);
		
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->lst_dtc_tm));
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->encrp_type));
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->polcy));
					dbus_message_iter_next(&iter_array);
		
					if(dcli_add_rogue_ap_ele_fun(&(LIST->rouge_ap_list),rouge_ap) == -1)
					{
						//printf("err here free.\n");
						//CW_FREE_OBJECT(rouge_ap->ESSID);
						dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1,LIST);
						CW_FREE_OBJECT(rouge_ap->IEs_INFO);
					
						CW_FREE_OBJECT(rouge_ap);
						return  NULL;
					}
				}
				if(essid){
					free(essid);
					essid = NULL;
				} 
			}
			break;
		case 3 :
			{
				essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
				if (essid == NULL)
				{
					CW_FREE_OBJECT(LIST);
					return NULL;	
				}
				memset(essid,0,ESSID_DEFAULT_LEN+1);
				CW_CREATE_OBJECT_ERR(LIST->rouge_ap_list, Neighbor_AP_INFOS, return NULL;);	
				LIST->rouge_ap_list->neighborapInfosCount = 0;
				LIST->rouge_ap_list->neighborapInfos = NULL;
				struct Neighbor_AP_ELE *rouge_ap = NULL;	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
				for (i = 0; i < num; i++)
				{
					DBusMessageIter iter_struct;
					DBusMessageIter iter_sub_array;
					rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
					if((rouge_ap == NULL))
					{
						//printf("##malloc memory fail #\n");
						dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID,LIST);
						return NULL;
					}
					rouge_ap->next = NULL;
					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
					dbus_message_iter_next(&iter_struct);
		
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
					dbus_message_iter_next(&iter_struct);	
		
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
					dbus_message_iter_next(&iter_struct);				
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
					dbus_message_iter_next(&iter_struct);	
					
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
					dbus_message_iter_next(&iter_struct);
		
							
					dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
					int j = 0;
					for(j=0;j<ESSID_DEFAULT_LEN+1;j++){
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
						//dbus_message_iter_get_basic(&iter_sub_struct,&essid[j]);
						dbus_message_iter_get_basic(&iter_sub_struct,&rouge_ap->ESSID[j]);
						dbus_message_iter_next(&iter_sub_struct);
						dbus_message_iter_next(&iter_sub_array);
					}
					dbus_message_iter_next(&iter_struct);

						/*rouge_ap->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
						memset(rouge_ap->ESSID,0,ESSID_DEFAULT_LEN+1);
						memcpy(rouge_ap->ESSID,essid,ESSID_DEFAULT_LEN);*/

					dbus_message_iter_get_basic(&iter_struct,&IEs_INFO);/*(rouge_ap->IEs_INFO)*/
					dbus_message_iter_next(&iter_array);

						rouge_ap->IEs_INFO = (char*)malloc(strlen(IEs_INFO)+1);
						memset(rouge_ap->IEs_INFO,0,strlen(IEs_INFO)+1);
						memcpy(rouge_ap->IEs_INFO,IEs_INFO,strlen(IEs_INFO));
		
				if(dcli_add_rogue_ap_ele_fun(&(LIST->rouge_ap_list),rouge_ap) == -1)
				{
					//printf("err here free.\n");
					//CW_FREE_OBJECT(rouge_ap->ESSID);
					dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID,LIST);
					CW_FREE_OBJECT(rouge_ap->IEs_INFO);
				
					CW_FREE_OBJECT(rouge_ap);
					return NULL;
				}
		
				}
				if(essid){
					free(essid);
					essid = NULL;
				} 
			}
			break;
		case 4 :
			{
				essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
				if (essid == NULL)
				{
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(essid,0,ESSID_DEFAULT_LEN+1);
				CW_CREATE_OBJECT_ERR(LIST->rouge_ap_list, Neighbor_AP_INFOS, return NULL;);	
				LIST->rouge_ap_list->neighborapInfosCount = 0;
				LIST->rouge_ap_list->neighborapInfos = NULL;
				struct Neighbor_AP_ELE *rouge_ap = NULL;	
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
				for (i = 0; i < num; i++)
				{
					DBusMessageIter iter_struct;
					DBusMessageIter iter_sub_array;
					rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
					if((rouge_ap == NULL))
					{
						//printf("##malloc memory fail #\n");
						dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,LIST);
						return NULL;
					}
					rouge_ap->next = NULL;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->wtpid));
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
					dbus_message_iter_next(&iter_struct);	

					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
					dbus_message_iter_next(&iter_struct);				
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
					dbus_message_iter_next(&iter_struct);	
					
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
					dbus_message_iter_next(&iter_struct);

							
					dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
					int j = 0;
					for(j=0;j<ESSID_DEFAULT_LEN+1;j++){
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
						//dbus_message_iter_get_basic(&iter_sub_struct,&essid[j]);
						dbus_message_iter_get_basic(&iter_sub_struct,&rouge_ap->ESSID[j]);
						dbus_message_iter_next(&iter_sub_struct);
						dbus_message_iter_next(&iter_sub_array);
					}
					dbus_message_iter_next(&iter_struct);

						/*rouge_ap->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
						memset(rouge_ap->ESSID,0,ESSID_DEFAULT_LEN+1);
						memcpy(rouge_ap->ESSID,essid,ESSID_DEFAULT_LEN);*/

					dbus_message_iter_get_basic(&iter_struct,&IEs_INFO);/*(rouge_ap->IEs_INFO)*/
					dbus_message_iter_next(&iter_array);


						rouge_ap->IEs_INFO = (char*)malloc(strlen(IEs_INFO)+1);
						if (rouge_ap->IEs_INFO == NULL)
						{
							dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,LIST);
							free(rouge_ap);
							return NULL;
						}
						memset(rouge_ap->IEs_INFO,0,strlen(IEs_INFO)+1);
						memcpy(rouge_ap->IEs_INFO,IEs_INFO,strlen(IEs_INFO));

					if(dcli_add_rogue_ap_ele_fun(&(LIST->rouge_ap_list),rouge_ap) == -1)
					{
						//printf("err here free.\n");
						//CW_FREE_OBJECT(rouge_ap->ESSID);
						dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,LIST);
						CW_FREE_OBJECT(rouge_ap->IEs_INFO);
					
						CW_FREE_OBJECT(rouge_ap);
						return NULL;
					}
				}
				if(essid){
					free(essid);
					essid = NULL;
				}
			}
			break;
		case 5 :
			{	
				CW_CREATE_OBJECT_ERR(LIST->wids_device_list, wid_wids_device, return NULL;);	
				LIST->wids_device_list->count = 0;
				LIST->wids_device_list->wids_device_info = NULL;
				//CW_CREATE_OBJECT_ERR(LIST->wids_device_list->wids_device_info, struct tag_wids_device_ele, return NULL;);	
				struct tag_wids_device_ele *wids_device = NULL;
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
				for (i = 0; i < num; i++)
				{

					DBusMessageIter iter_struct;
					
					wids_device = (struct tag_wids_device_ele *)malloc(sizeof(struct tag_wids_device_ele));
					if((wids_device == NULL))
					{
						//printf("##malloc memory fail #\n");
						dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST, LIST);
						return NULL;
					}
					wids_device->next = NULL;
					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[5]));
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(wids_device->attacktype));
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(wids_device->frametype));
					dbus_message_iter_next(&iter_struct);	

					dbus_message_iter_get_basic(&iter_struct,&(wids_device->attackcount));
					dbus_message_iter_next(&iter_struct);	
					
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->fst_attack));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->lst_attack));
					dbus_message_iter_next(&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->channel));
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->rssi));
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(wids_device->RogStaAttackStatus));  //fengwenchao add 20110415					
					dbus_message_iter_next(&iter_array);

					if(dcli_add_wids_ap_ele_fun(&(LIST->wids_device_list),wids_device) == -1)
					{
						//printf("err here free.\n");
						dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST, LIST);
						CW_FREE_OBJECT(wids_device);
						return NULL;
					}

				}
			
			}
			break;
		case 6 :
			{

					CW_CREATE_OBJECT_ERR(LIST->wids_device_list, wid_wids_device, return NULL;);	
					LIST->wids_device_list->count = 0;
					LIST->wids_device_list->wids_device_info = NULL;
					struct tag_wids_device_ele *wids_device = NULL;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&num);
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
			
					for (i = 0; i < num; i++)
					{
						DBusMessageIter iter_struct;
						
						wids_device = (struct tag_wids_device_ele *)malloc(sizeof(struct tag_wids_device_ele));
						if((wids_device == NULL))
						{
							//printf("##malloc memory fail #\n");
							dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID, LIST);
							return NULL;
						}
						wids_device->next = NULL;
						
						dbus_message_iter_recurse(&iter_array,&iter_struct);
					
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[0]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[1]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[2]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[3]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[4]));
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[5]));
						dbus_message_iter_next(&iter_struct);
			
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->attacktype));
						dbus_message_iter_next(&iter_struct);
			
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->frametype));
						dbus_message_iter_next(&iter_struct);	
			
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->attackcount));
						dbus_message_iter_next(&iter_struct);	
						
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->fst_attack));
						dbus_message_iter_next(&iter_struct);	
						dbus_message_iter_get_basic(&iter_struct,&(wids_device->lst_attack));
						dbus_message_iter_next(&iter_array);
			
						if(dcli_add_wids_ap_ele_fun(&(LIST->wids_device_list),wids_device) == -1)
						{
							//printf("err here free.\n");
							dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID, LIST);
							CW_FREE_OBJECT(wids_device);
							return NULL;
						}
					}
			}
			break;
		default :	break;
		}
	}
	dbus_message_unref(reply);
	return LIST;

}

void* dcli_ac_show_api_group_three(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* ret,
	unsigned int* num2,
	unsigned int* num3,
	//DCLI_AC_API_GROUP_THREE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int num = 0;
	int i = 0;
	char en[] = "enable";
	char dis[] = "disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
//	int state = 1;
	DCLI_AC_API_GROUP_THREE *LIST = NULL;
	int wtpid = id1;
	unsigned int dcli_sn = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int localid = *num3;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");
	
	dcli_sn = dcli_ac_method_parse_three(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 2)||(dcli_sn == 5)||(dcli_sn == 6)){	
		//printf("show ap network bywtp ID\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);	
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
/*	else if(dcli_sn == 2){
		//printf("sn=2 22222222show ap ip bywtp ID\n");
		int state = 1;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR);
		dbus_error_init(&err);		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}*/
	else if((dcli_sn == 3)||(dcli_sn == 4)){
		//printf("sn=3  333333333\n");
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&wtpid,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
		
	}
/*	else if(dcli_sn == 4){
			int state = 1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST);
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&wtpid,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
	}*/
/*	else if(dcli_sn == 5){
		//printf("11111111dcli_sn is 5\n");
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO);
			dbus_error_init(&err);		
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&wtpid,
									 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 6){
		//printf("dcli_sn is 6\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
		
	}*/

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
//printf("ret is %d,\n",ret);
	if(*ret == 0 )
	{	
		char* wtpip = NULL;
		CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_API_GROUP_THREE, return NULL;);	
		LIST->WTPIP = NULL;
		LIST->ap_statics_list = NULL;
	switch(dcli_sn){
		case 1 :
			{
			//printf("aa dcli_sn is %d,\n",dcli_sn);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpip);/*LIST->WTPIP*/

				LIST->WTPIP = (char*)malloc(strlen(wtpip)+1);
				if (LIST->WTPIP == NULL)
				{
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(LIST->WTPIP,0,strlen(wtpip)+1);
				memcpy(LIST->WTPIP,wtpip,strlen(wtpip));
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->ap_mask_new);
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->ap_gateway);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->ap_dnsfirst);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->ap_dnssecend);		

		}
		break;
	case 2 :
		{
		//printf("ret = 0  sn=2 22222222\n");
		
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wtpip);/*LIST->WTPIP*/
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->ap_mask_new);
		
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->ap_gateway);
					
		}
		break;
	case 3 :
		{
		//printf("ret = 0  sn=3  33333333333\n");
				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->floodingcount));
				dbus_message_iter_next(&iter);
		
				dbus_message_iter_get_basic(&iter,&(LIST->sproofcount));
				dbus_message_iter_next(&iter);	
		
				dbus_message_iter_get_basic(&iter,&(LIST->weakivcount));
				dbus_message_iter_next(&iter);
					
		}
		break;
	case 4 :
		{
		//printf("ret = 0  sn=4  44444444444444\n");
		
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->floodingcount));
				dbus_message_iter_next(&iter);
		
				dbus_message_iter_get_basic(&iter,&(LIST->sproofcount));
				dbus_message_iter_next(&iter);	
		
				dbus_message_iter_get_basic(&iter,&(LIST->weakivcount));
				dbus_message_iter_next(&iter);
	
		}
		break;
	case 5 :{
	//printf("ret = 0 dcli_sn is 5\n");
	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->txpowr);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->rssi[0]);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->rssi[1]);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->rssi[2]);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->rssi[3]);

		}
		break;
	case 6 :
		{
//printf("ret = 0 dcli_sn is 6\n");
		wlan_stats_info *ap_statics = NULL;
	//	CW_CREATE_OBJECT_ERR(ap_statics, wlan_stats_info, return -1;);
	//	wlan_stats_info apstatsinfo[TOTAL_AP_IF_NUM];
		CW_CREATE_OBJECT_ERR(LIST->ap_statics_list, Ap_statics_INFOS, return NULL;); 	
		LIST->ap_statics_list->count = 0;
		LIST->ap_statics_list->ap_statics_ele= NULL;
	//	CW_CREATE_OBJECT_ERR(LIST->ap_statics_list->ap_statics_ele, wlan_stats_info, return NULL;);	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
			for (i = 0; i < num; i++)
			{
				CW_CREATE_OBJECT_ERR(ap_statics, wlan_stats_info, return NULL;);
				memset(ap_statics, 0, sizeof(wlan_stats_info));
				DBusMessageIter iter_struct;
				
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->type));
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->radioId));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->wlanId));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->mac[0]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->mac[1]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->mac[2]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->mac[3]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->mac[4]));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->mac[5]));
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_packets));
				dbus_message_iter_next(&iter_struct);	
						
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_packets));
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_errors));
				dbus_message_iter_next(&iter_struct);	
						
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_errors));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_bytes));
				dbus_message_iter_next(&iter_struct);	
						
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_bytes));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_rate));
				dbus_message_iter_next(&iter_struct);	
						
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_rate));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->ast_rx_crcerr));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->ast_rx_badcrypt));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->ast_rx_badmic));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->ast_rx_phyerr));
				dbus_message_iter_next(&iter_struct);

                /*zhangshu add from 1.2 2010-09-13 */
				/*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，start*/
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_frame));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_frame));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_error_frame));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_error_frame));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_drop_frame));

                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_drop_frame));
                dbus_message_iter_next(&iter_struct);

                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_unicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_unicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_multicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_multicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_broadcast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_broadcast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_unicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_unicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_multicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_multicast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_broadcast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_broadcast));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_retry));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_retry));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_data));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_data));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_retry));
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_retry));
                dbus_message_iter_next(&iter_struct);
				/*-----------------------------------------------------------------*/
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_drop));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_drop));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_mgmt));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_mgmt));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_mgmt));
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_mgmt));
				dbus_message_iter_next(&iter_struct);

				
				/* zhangshu append 2010-09-13 */
    			dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_sum_bytes));
    			dbus_message_iter_next(&iter_struct);
    			
    			dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_sum_bytes));
    			dbus_message_iter_next(&iter_struct);
    			
    			/*--------------------------*/
                /*zhaoruijia,20100907,添加rx_pkt_control，tx_pkt_control,start*/
    			dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_pkt_control));
    			dbus_message_iter_next(&iter_struct);
    			
    			dbus_message_iter_get_basic(&iter_struct,&(ap_statics->tx_pkt_control));
    		    /*zhaoruijia,20100907,添加rx_pkt_control，tx_pkt_control,start*/
    		    
    		    /* zhangshu add for error frames, 2010-09-26 */ 
    			dbus_message_iter_next(&iter_struct);
    			dbus_message_iter_get_basic(&iter_struct,&(ap_statics->rx_errors_frames));

				dbus_message_iter_next(&iter_array);

				if(ap_statics->type == 0)
				{
					sprintf(ap_statics->ifname,"ath%d",ap_statics->wlanId);
					if(ap_statics->rx_rate > 1)
					{						
						ap_statics->rx_band = ((int)((float)ap_statics->rx_rate/20))%100;
					}
					else
					{
						ap_statics->rx_band = 0;
					}
					if(ap_statics->tx_rate > 1)
					{						
						ap_statics->tx_band = ((int)((float)ap_statics->tx_rate/15))%100;		
					}
					else
					{
						ap_statics->tx_band = 0;	
					}				
				}
				else if(ap_statics->type == 1)
				{
					sprintf(ap_statics->ifname,"eth%d",ap_statics->wlanId);
					if(ap_statics->rx_rate > 1)
					{						
						ap_statics->rx_band = ((int)((float)ap_statics->rx_rate/100))%100;
					}
					else
					{
						ap_statics->rx_band = 0;
					}
					if(ap_statics->tx_rate > 1)
					{						
						ap_statics->tx_band = ((int)((float)ap_statics->tx_rate/100))%100;		
					}
					else
					{
						ap_statics->tx_band = 0;	
					}					
				}
				else
				{
					sprintf(ap_statics->ifname,"wifi%d",ap_statics->wlanId);
					if(ap_statics->rx_rate > 1)
					{						
						ap_statics->rx_band = ((int)((float)ap_statics->rx_rate/20))%100;
					}
					else
					{
						ap_statics->rx_band = 0;
					}	
					if(ap_statics->tx_rate > 1)
					{						
						ap_statics->tx_band = ((int)((float)ap_statics->tx_rate/15))%100;		
					}
					else
					{
						ap_statics->tx_band = 0;	
					}
				}
				
				dcli_add_ap_statics_node(&(LIST->ap_statics_list),ap_statics);
			}
		}
		break;
	default	:	break;
	}}
	dbus_message_unref(reply);
	return LIST;

}

void* dcli_ac_show_api_group_four(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int *ret,
	char *char1,
	char *char2,
	int localid,
	//DCLI_AC_API_GROUP_FOUR*LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	//int ret;
	int i = 0;
	int num = 0;
	unsigned int dcli_sn = 0;
	char en[] = "enable";
	char dis[] = "disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
//	int state = 1;
	int wtpid = id1;
	DCLI_AC_API_GROUP_FOUR *LIST = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//int localid = (int)*char2;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");	
	dcli_sn = dcli_ac_method_parse_four(DBUS_METHOD);
	if(dcli_sn == 1){	
		//printf("(dcli_sn == 1)\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&id2,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 2){
		//printf("sn=2 22222222\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);	
	}
	else if((dcli_sn == 3)||(dcli_sn == 4)){
		//printf("sn=3 ||4  333333333444444444444\n");
		char *model = NULL;
		model = char1;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query, 	
							DBUS_TYPE_STRING,&model,
							DBUS_TYPE_INVALID);
		
	}
/*	else if(dcli_sn == 4){
			char *model = NULL;
			model = char1;
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION);
			dbus_error_init(&err);
			dbus_message_append_args(query, 	
								DBUS_TYPE_STRING,&model,
								DBUS_TYPE_INVALID);
	}*/
	else if(dcli_sn == 5){
//printf("11111111dcli_sn is 5\n");
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
	
	}

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	//printf("ret is %d  0000000000000000,\n",*ret);
	if(*ret == 0 )
	{	
		//char *code = NULL;
		char *str_ap_model = NULL; //for oem change
		char *str_ap_version_name = NULL;
		char *str_ap_version_path = NULL;
		//char *str_ap_code = NULL;// for model match
		char *str_oem_version = NULL;
	
		char   *hw_version = NULL;
		char   *sw_name = NULL;
		char   *sw_version = NULL;
		char   *sw_supplier = NULL;
		char   *supplier = NULL;
		char   *model = NULL;
		CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_API_GROUP_FOUR, return NULL;);	
		LIST->model_info = NULL;
		LIST->bak_sock = NULL;
		LIST->code_info = NULL;
		LIST->config_ver_info = NULL;
		switch(dcli_sn){
			case 1 :{
				CW_CREATE_OBJECT_ERR(LIST->model_info, model_infomation, return NULL;);	
				memset(LIST->model_info, 0, sizeof(model_infomation));

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter, &model);
				LIST->model_info->model = (char *)malloc(strlen(model) + 1);
				if (LIST->model_info->model == NULL)
				{
					CW_FREE_OBJECT(LIST->model_info);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(LIST->model_info->model, 0, strlen(model) + 1);
				memcpy(LIST->model_info->model, model, strlen(model));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->model_info->ap_11a_antenna_gain);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->model_info->ap_11bg_antenna_gain);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->model_info->ap_eth_num);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->model_info->ap_wifi_num);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter, &hw_version);
				LIST->model_info->hw_version = (char *)malloc(strlen(hw_version) + 1);
				if (LIST->model_info->hw_version == NULL)
				{
					CW_FREE_OBJECT(LIST->model_info->model);
					CW_FREE_OBJECT(LIST->model_info);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(LIST->model_info->hw_version, 0, strlen(hw_version) + 1);
				memcpy(LIST->model_info->hw_version, hw_version, strlen(hw_version));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter, &sw_name);
				LIST->model_info->sw_name = (char *)malloc(strlen(sw_name) + 1);
				if (LIST->model_info->sw_name == NULL)
				{
					CW_FREE_OBJECT(LIST->model_info->model);
					CW_FREE_OBJECT(LIST->model_info->hw_version);
					CW_FREE_OBJECT(LIST->model_info);
					CW_FREE_OBJECT(LIST);
					break;
				}
				memset(LIST->model_info->sw_name, 0, strlen(sw_name) + 1);
				memcpy(LIST->model_info->sw_name, sw_name, strlen(sw_name));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter, &sw_version);
				LIST->model_info->sw_version = (char *)malloc(strlen(sw_version) + 1);
				if (LIST->model_info->sw_version == NULL)
				{
					CW_FREE_OBJECT(LIST->model_info->sw_name);
					CW_FREE_OBJECT(LIST->model_info->model);
					CW_FREE_OBJECT(LIST->model_info->hw_version);
					CW_FREE_OBJECT(LIST->model_info);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(LIST->model_info->sw_version, 0, strlen(sw_version) + 1);
				memcpy(LIST->model_info->sw_version, sw_version, strlen(sw_version));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter, &sw_supplier);
				LIST->model_info->sw_supplier = (char *)malloc(strlen(sw_supplier) + 1);
				if (LIST->model_info->sw_supplier == NULL)
				{
					CW_FREE_OBJECT(LIST->model_info->sw_version);
					CW_FREE_OBJECT(LIST->model_info->sw_name);
					CW_FREE_OBJECT(LIST->model_info->hw_version);
					CW_FREE_OBJECT(LIST->model_info->model);
					CW_FREE_OBJECT(LIST->model_info);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(LIST->model_info->sw_supplier, 0, strlen(sw_supplier) + 1);
				memcpy(LIST->model_info->sw_supplier, sw_supplier, strlen(sw_supplier));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter, &supplier);
				LIST->model_info->supplier = (char *)malloc(strlen(supplier) + 1);
				if (LIST->model_info->supplier == NULL)
				{
					CW_FREE_OBJECT(LIST->model_info->sw_supplier);
					CW_FREE_OBJECT(LIST->model_info->sw_version);
					CW_FREE_OBJECT(LIST->model_info->sw_name);
					CW_FREE_OBJECT(LIST->model_info->hw_version);
					CW_FREE_OBJECT(LIST->model_info->model);
					CW_FREE_OBJECT(LIST->model_info);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				memset(LIST->model_info->supplier, 0, strlen(supplier) + 1);
				memcpy(LIST->model_info->supplier, supplier, strlen(supplier));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->model_info->ap_if_mtu);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->model_info->ap_if_rate);
				
			}
			break;
		case 2 :{
	//printf("dcli_sn is 22222222\n");
				CW_CREATE_OBJECT_ERR(LIST->config_ver_info, Config_Ver_Info, return NULL;);	
				LIST->config_ver_info->config_ver_node = NULL;
				LIST->config_ver_info->list_len = 0;
				CWConfigVersionInfo_dcli *ap_conf_node = NULL;
				CW_CREATE_OBJECT_ERR(ap_conf_node, CWConfigVersionInfo_dcli, return NULL;); 
				memset(ap_conf_node, 0, sizeof(CWConfigVersionInfo_dcli));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
	//printf("num is %d.\n",num);
				for (i = 0; i < num; i++) {
					DBusMessageIter iter_struct;
					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &str_ap_model);
				
					memset(ap_conf_node, 0, sizeof(CWConfigVersionInfo_dcli));
					ap_conf_node->str_ap_model = (char *)malloc(strlen(str_ap_model) + 1);
					if (ap_conf_node->str_ap_model == NULL)
					{
						dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST, LIST);
						CW_FREE_OBJECT(ap_conf_node);
						return NULL;
					}
					memset(ap_conf_node->str_ap_model, 0, strlen(str_ap_model) + 1);
					memcpy(ap_conf_node->str_ap_model, str_ap_model, strlen(str_ap_model));	
					str_ap_model = NULL;

					//ap_conf_node->str_ap_model = (char*)malloc(strlen(str_ap_model));
					//memset(ap_conf_node->str_ap_model,0,strlen(str_ap_model)+1);
					//memcpy(ap_conf_node->str_ap_model,str_ap_model,strlen(str_ap_model));
					
					dbus_message_iter_next(&iter_struct);
					
		
					//dbus_message_iter_get_basic(&iter_struct,&(ap_conf_node->str_ap_code));	
					
					//dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &str_ap_version_name);
					ap_conf_node->str_ap_version_name = (char *)malloc(strlen(str_ap_version_name) + 1);
					if (ap_conf_node->str_ap_version_name == NULL)
					{
						dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST, LIST);
						CW_FREE_OBJECT(ap_conf_node->str_ap_model);
						CW_FREE_OBJECT(ap_conf_node);
						return NULL;
					}
					memset(ap_conf_node->str_ap_version_name, 0, strlen(str_ap_version_name) + 1);
					memcpy(ap_conf_node->str_ap_version_name, str_ap_version_name, strlen(str_ap_version_name));
					str_ap_version_name = NULL;
				
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &str_ap_version_path);
					if ((ap_conf_node->str_ap_version_path = (char *)malloc(strlen(str_ap_version_path) + 1)) == NULL)
					{
						dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST, LIST);
						CW_FREE_OBJECT(ap_conf_node->str_ap_version_name);
						CW_FREE_OBJECT(ap_conf_node->str_ap_model);
						CW_FREE_OBJECT(ap_conf_node);
						return NULL;
					}
					memset(ap_conf_node->str_ap_version_path, 0, strlen(str_ap_version_path) + 1);
					memcpy(ap_conf_node->str_ap_version_path, str_ap_version_path, strlen(str_ap_version_path));
					str_ap_version_path = NULL;
					
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(ap_conf_node->radio_num));	
		
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(ap_conf_node->bss_num));	

					dbus_message_iter_next(&iter_struct);
					
					//dbus_message_iter_get_basic(&iter_struct,&(ap_conf_node->apcodeflag));	
					
					dbus_message_iter_next(&iter_array);
					dcli_add_ap_conf_info_node(&(LIST->config_ver_info),ap_conf_node);
					CW_FREE_OBJECT(ap_conf_node->str_ap_version_path);
					CW_FREE_OBJECT(ap_conf_node->str_ap_version_name);
					CW_FREE_OBJECT(ap_conf_node->str_ap_model);
				}
				
				CW_FREE_OBJECT(ap_conf_node);	
			}
			break;
		case 3 :{
				CW_CREATE_OBJECT_ERR(LIST->config_ver_info, Config_Ver_Info, return NULL;);	
				LIST->config_ver_info->list_len = 0;
				LIST->config_ver_info->config_ver_node = NULL;
				CWConfigVersionInfo_dcli	*modelinfo = NULL;
				CW_CREATE_OBJECT_ERR(modelinfo, CWConfigVersionInfo_dcli, return NULL;);	
				memset(modelinfo, 0, sizeof(CWConfigVersionInfo_dcli));
				LIST->config_ver_info->config_ver_node = modelinfo;
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&str_ap_model);/*modelinfo->str_ap_model*/
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&str_ap_version_name);/*(modelinfo->str_ap_version_name)*/	
					
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&str_ap_version_path);/*(modelinfo->str_ap_version_path)*/
				
					//dbus_message_iter_next(&iter);
					//dbus_message_iter_get_basic(&iter,&str_ap_code);/*(modelinfo->str_ap_code)*/

						modelinfo->str_ap_model = (char*)malloc(strlen(str_ap_model) + 1);
						memset(modelinfo->str_ap_model,0,strlen(str_ap_model)+1);
						memcpy(modelinfo->str_ap_model,str_ap_model,strlen(str_ap_model));

						modelinfo->str_ap_version_name = (char*)malloc(strlen(str_ap_version_name) + 1);
						memset(modelinfo->str_ap_version_name,0,strlen(str_ap_version_name)+1);
						memcpy(modelinfo->str_ap_version_name,str_ap_version_name,strlen(str_ap_version_name));

						modelinfo->str_ap_version_path = (char*)malloc(strlen(str_ap_version_path) + 1);
						memset(modelinfo->str_ap_version_path,0,strlen(str_ap_version_path)+1);
						memcpy(modelinfo->str_ap_version_path,str_ap_version_path,strlen(str_ap_version_path));
                    /*
						modelinfo->str_ap_code = (char*)malloc(strlen(str_ap_code));
						memset(modelinfo->str_ap_code,0,strlen(str_ap_code)+1);
						memcpy(modelinfo->str_ap_code,str_ap_code,strlen(str_ap_code));
					*/
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(modelinfo->radio_num));	
					/*fengwenchao modify 20120502 for autelan-2917*/
					dbus_message_iter_next(&iter);
					dbus_message_iter_recurse(&iter,&iter_array);
					for(i=0;i<modelinfo->radio_num;i++)
					{
						DBusMessageIter iter_struct;					
						dbus_message_iter_recurse(&iter_array,&iter_struct);
						
						//dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(modelinfo->radio_info[i].radio_type));
			
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(modelinfo->radio_info[i].radio_id)); 
						
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(modelinfo->radio_info[i].reserved1)); 
			
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&(modelinfo->radio_info[i].bss_count));	

						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_next(&iter_array);
					}	
					
			}
			break;
		case 4 :{
					CW_CREATE_OBJECT_ERR(LIST->code_info, wid_code_infomation, return NULL;);	
					LIST->code_info->code = NULL;
					//dbus_message_iter_next(&iter);	
					//dbus_message_iter_get_basic(&iter,&code);/*LIST->code_info->code*/
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&hw_version); /*(LIST->code_info->hw_version)*/
					
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&supplier);/*(LIST->code_info->supplier)*/
				
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&sw_name);/*(LIST->code_info->sw_name)*/
					
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&sw_version);/*(LIST->code_info->sw_version)*/ 
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&sw_supplier);/*(LIST->code_info->sw_supplier)*/

                        /*

						LIST->code_info->code = (char*)malloc(strlen(code));
						memset(LIST->code_info->code,0,strlen(code)+1);
						memcpy(LIST->code_info->code,code,strlen(code));
						*/
						LIST->code_info->hw_version = (char*)malloc(strlen(hw_version)+1);
						memset(LIST->code_info->hw_version,0,strlen(hw_version)+1);
						memcpy(LIST->code_info->hw_version,hw_version,strlen(hw_version));
						
						LIST->code_info->supplier = (char*)malloc(strlen(supplier)+1);
						memset(LIST->code_info->supplier,0,strlen(supplier)+1);
						memcpy(LIST->code_info->supplier,supplier,strlen(supplier));

						LIST->code_info->sw_name = (char*)malloc(strlen(sw_name)+1);
						memset(LIST->code_info->sw_name,0,strlen(sw_name)+1);
						memcpy(LIST->code_info->sw_name,sw_name,strlen(sw_name));
						
						LIST->code_info->sw_version = (char*)malloc(strlen(sw_version)+1);
						memset(LIST->code_info->sw_version,0,strlen(sw_version)+1);
						memcpy(LIST->code_info->sw_version,sw_version,strlen(sw_version));
						
						LIST->code_info->sw_supplier = (char*)malloc(strlen(sw_supplier)+1);
						memset(LIST->code_info->sw_supplier,0,strlen(sw_supplier)+1);
						memcpy(LIST->code_info->sw_supplier,sw_supplier,strlen(sw_supplier));
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->ap_if_mtu));	
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->ap_if_rate)); 
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->card_capacity));
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->flash_capacity)); 
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->cpu_type));	
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->mem_type));	
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->ap_eth_num)); 
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->ap_wifi_num));
			
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&(LIST->code_info->ap_antenna_gain));	
			
					for(i=0;i<LIST->code_info->ap_wifi_num;i++)
					{
						dbus_message_iter_next(&iter);
						dbus_message_iter_get_basic(&iter,&(LIST->code_info->support_mode[i]));	
					}	
			}
			break;
		case 5 :{
		//printf("ret = 0 dcli_sn is 5\n");
				
				struct bak_sock *baksock = NULL;
				baksock = (struct bak_sock *)malloc(sizeof(struct bak_sock));
				if (baksock == NULL)
				{
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
				for (i = 0; i < num; i++)
				{
					DBusMessageIter iter_struct;
					
					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(baksock->sock));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(baksock->ip));
					dbus_message_iter_next(&iter_array);

					dcli_add_bak_sock_node(&(LIST->bak_sock),baksock);
		
				}
			}
			break;
		default :	break;
		}
	}
	dbus_message_unref(reply);
	return LIST;

}

void* dcli_ac_show_api_group_five(
	int index,
	unsigned int dcli_sn,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int *ret,
	char *char1,
	char *char2,
	int localid,
	//DCLI_AC_API_GROUP_FIVE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	//int ret = 0;
	int i = 0;
	int j = 0;
	DCLI_AC_API_GROUP_FIVE *LIST = NULL;	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int state = 1;
	int wtpid = id1;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//int localid = (int)*char2;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

//printf("dcli_sn is %d bbbbbbbbbb\n",dcli_sn);
	dcli_sn = dcli_ac_method_parse_five(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 3)||(dcli_sn == 4)||(dcli_sn == 5)||(dcli_sn == 6)||(dcli_sn == 7)||(dcli_sn == 8)||(dcli_sn == 9)||(dcli_sn == 11)||(dcli_sn == 12)||(dcli_sn == 13)){	
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 2){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&state,
								 DBUS_TYPE_INVALID);
	}
/*	else if(dcli_sn == 3){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 4){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 5){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 6){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 7){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD);
		dbus_error_init(&err);
	}	
	else if(dcli_sn == 8){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 9){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL);
		dbus_error_init(&err);
	}*/
	else if (dcli_sn == 10){
		ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
	}
/*	else if(dcli_sn == 11){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER);
		dbus_error_init(&err);
	}	
	else if(dcli_sn == 12){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT);
		dbus_error_init(&err);
	}	
	else if(dcli_sn == 13){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION); 
		dbus_error_init(&err);
	}
	else if(dcli_sn == 14){

	}
	else if(dcli_sn == 15){

	}*/
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}
	int haveret = id3;


	if(haveret == 0){
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&(*ret));
	}
	if(*ret == 0 )
	{	
		
		char *hw_version_char =	NULL;
		char *sw_version_char =	NULL;
		char *ac_name =	NULL;
		CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_API_GROUP_FIVE, return NULL;); 
		LIST->wireless_control = NULL;
		LIST->auto_login = NULL;
		LIST->tx_control = NULL;
		LIST->sample_info = NULL;
		LIST->wid_vrrp = NULL;
		LIST->num = 0;
		switch(dcli_sn){
		case 1 :{
			CW_CREATE_OBJECT_ERR(LIST->wireless_control, wireless_config, return NULL;); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->hw_version));/*001*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->sw_version));/*002*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->auth_security));/*003*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->uclev3_protocol));/*004*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&ac_name);/*(LIST->wireless_control->ac_name)*/

				LIST->wireless_control->ac_name = (char*)malloc(strlen(ac_name));
				if (LIST->wireless_control->ac_name == NULL)
				{
					CW_FREE_OBJECT(LIST->wireless_control);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				else
				{
				memset(LIST->wireless_control->ac_name,0,strlen(ac_name)+1);
				memcpy(LIST->wireless_control->ac_name,ac_name,strlen(ac_name));
				}
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->sta_count));/*006*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->max_wtp));/*007*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->gmaxwtps));/* gMaxWTPs */
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->static_wtp));/*007	////////////*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->force_mtu));/*008*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->log_switch));/*009*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->log_size));/*010*/
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->log__level));/*011*/

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->trapflag));

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->apstaticstate));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->macfiltrflag));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->essidfiltrflag));			

			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->g_ac_all_extention_information_switch));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->result));
				if(LIST->wireless_control->result == 2)
				{
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&hw_version_char);/*(LIST->wireless_control->hw_version_char)*/
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&sw_version_char);/*(LIST->wireless_control->sw_version_char)*/

						
					LIST->wireless_control->hw_version_char = (char*)malloc(strlen(hw_version_char) + 1);				
					if (LIST->wireless_control->hw_version_char == NULL)
					{
						CW_FREE_OBJECT(LIST->wireless_control->ac_name);
						CW_FREE_OBJECT(LIST->wireless_control);
						CW_FREE_OBJECT(LIST);
						return NULL;
					}
					else
					{
					memset(LIST->wireless_control->hw_version_char,0,strlen(hw_version_char)+1);
					memcpy(LIST->wireless_control->hw_version_char,hw_version_char,strlen(hw_version_char));
					}

					LIST->wireless_control->sw_version_char = (char*)malloc(strlen(sw_version_char) + 1);
					if (LIST->wireless_control->sw_version_char  == NULL)
					{
						CW_FREE_OBJECT(LIST->wireless_control->ac_name);
						CW_FREE_OBJECT(LIST->wireless_control->hw_version_char);
						CW_FREE_OBJECT(LIST->wireless_control);
						CW_FREE_OBJECT(LIST);
						return NULL;
					}
					else
					{
					memset(LIST->wireless_control->sw_version_char,0,strlen(sw_version_char)+1);
					memcpy(LIST->wireless_control->sw_version_char,sw_version_char,strlen(sw_version_char));
					}
						
				}
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->apstatisticsinterval));
			/*xiaodawei add, 20110115*/
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->radioresmgmt));/*set radio resource management*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->tranpowerctrl));/*transmit power control*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->tranpwrctrlscope));/*transmit power control scope*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->autoaploginswitch));/*set auto_ap_login switch*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->autoaplogin_saveconfigswitch));/*set auto_ap_login save_config_switch*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wirelessctrlmonitor));/*set wireless-control monitor*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wirelessctrlsample));/*set wireless-control sample*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->widwatchdog));/*set wid watch dog*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ac_balance_method));/*set ac balance method*/
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ap_hotreboot));/*set ap hotreboot*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ap_acc_through_nat));/*set ap access through nat*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wtp_wids_policy));/*set wtp wids policy*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->radio_src_mgmt_countermeasures));/*set radio resource management countermeasures*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->radio_src_mgmt_countermeasures_mode));/*set radio resource management countermeasures mode*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wireless_interface_vmac));/*set wireless interface vmac*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wtp_link_detect));/*set wtp link detect*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wsm_switch));/*set wsm switch*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->service_tftp));/*service tftp*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->service_ftp));/*service ftp*/
			/*xiaodawei add for trap switch, 20110115*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ap_run_quit));/*set wireless-control trap ap_run_quit*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ap_cpu_threshold));/*set wireless-control trap ap_cpu_threshold*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ap_mem_threshold));/*set wireless-control trap ap_mem_threshold*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->ap_update_fail));/*set wireless-control trap ap_update_fail*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->rrm_change));/*set wireless-control trap rrm_change*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->rogue_ap_threshold));/*set wireless-control trap rogue_ap_threshold*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->rogue_terminal_threshold));/*set wireless-control trap rogue_terminal_threshold*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->rogue_device));/*set wireless-control trap rogue_device*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->wireless_interface_down));/*set wireless-control trap wireless_interface_down*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->channel_count_minor));/*set wireless-control trap channel_count_minor*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->channel_change));/*set wireless-control trap channel_change*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->rogue_ap));/*set wireless-control trap rogue_ap*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(LIST->wireless_control->country_code)); /*wcl add*/
			/*end of trap switch*/
			}
			break;
		case 2 :{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->num);
			}
			break;
		case 3 :{
			CW_CREATE_OBJECT_ERR(LIST->auto_login, wid_auto_ap_info, return NULL;); 
			memset(LIST->auto_login, 0, sizeof(wid_auto_ap_info));
			//CW_CREATE_OBJECT_ERR(LIST->auto_login->auto_ap_if, wid_auto_ap_if, return NULL;); 
			//memset(LIST->auto_login->auto_ap_if->ifname,0,ETH_IF_NAME_LEN);
			wid_auto_ap_if	*auto_ap_if = NULL;
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&LIST->auto_login->auto_ap_switch);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&LIST->auto_login->save_switch);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->auto_login->ifnum);
			char *ifname = NULL;
			//printf("1111 ifnum is %d",ifnum);
			#if 0
			ifname = (char*)malloc(ETH_IF_NAME_LEN+1);
			if (ifname == NULL)
			{
				CW_FREE_OBJECT(LIST->auto_login);
				return NULL;
			}
			#endif
			unsigned char ifnum = 0;
			ifnum = LIST->auto_login->ifnum;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			i =0;
			for(i=0;i<ifnum;i++)
			{
				DBusMessageIter iter_struct;
				DBusMessageIter iter_sub_array;
				auto_ap_if = (typeof(auto_ap_if))malloc(sizeof(wid_auto_ap_if));
				if (auto_ap_if == NULL)
				{
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG, LIST);
					return NULL;
				}
				memset(auto_ap_if, 0, sizeof(wid_auto_ap_if));
				//CW_CREATE_OBJECT_ERR(auto_ap_if, wid_auto_ap_if, return NULL;); 

				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&ifname);
				memcpy(auto_ap_if->ifname,ifname,ETH_IF_NAME_LEN);
				auto_ap_if->ifindex = 0;
				auto_ap_if->wlannum = 0;
				memset(auto_ap_if->wlanid,0,L_BSS_NUM);
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&auto_ap_if->ifindex);
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&auto_ap_if->wlannum);
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
				j = 0;
				for(j=0;j<L_BSS_NUM;j++){
					DBusMessageIter iter_sub_struct;
					dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_struct,&auto_ap_if->wlanid[j]);
					dbus_message_iter_next(&iter_sub_struct);
					dbus_message_iter_next(&iter_sub_array);
				}
				dcli_ac_add_auto_ap_if_node(LIST->auto_login,auto_ap_if);
				ifname = NULL;
				dbus_message_iter_next(&iter_array);
			}

#if 0
			for(i=0;i<ifnum;i++)
			{
				CW_CREATE_OBJECT_ERR(auto_ap_if, wid_auto_ap_if, return NULL;); 
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ifname);
				memcpy(auto_ap_if->ifname,ifname,ETH_IF_NAME_LEN);
				auto_ap_if->ifindex = 0;
				auto_ap_if->wlannum = 0;
				memset(auto_ap_if->wlanid,0,L_BSS_NUM);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&auto_ap_if->ifindex);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&auto_ap_if->wlannum);
				int j =0;
				for(j=0;j<L_BSS_NUM;j++)
				{
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&auto_ap_if->wlanid[j]);
				}
				dcli_ac_add_auto_ap_if_node(LIST->auto_login,auto_ap_if);
			}
#endif			
			}
			break;
		case 4 :{
				CW_CREATE_OBJECT_ERR(LIST->tx_control, txpower_control, return NULL;); 
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->tx_control->state);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->tx_control->scope);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->tx_control->th1);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->tx_control->th2);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->tx_control->constant);
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->tx_control->max);
			}
			break;
		case 5 :{
			LIST->num = 0;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->num);
			}
			break;
		case 6 :{
			CW_CREATE_OBJECT_ERR(LIST->sample_info, wid_sample_info, return NULL;); 
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->sample_info->monitor_switch);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->sample_info->monitor_time);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->sample_info->sample_switch);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->sample_info->sample_time);
			}
			break;
		case 7 :{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->cpu);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->memoryuse);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->temperature);
			}
			break;
		case 8 :{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->rrm_state);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->flag);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->report_interval);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->d_channel_state);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->countermeasures_mode);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->countermeasures_switch); 

			}
			break;
		case 9 :{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->neighbordead_interval);
			}
			break;
		case 10 :{
			char *vir_uplinkname = NULL;
			char *vir_downlinkname = NULL;
			char *global_ht_ifname = NULL;

			CW_CREATE_OBJECT_ERR(LIST->wid_vrrp, wid_vrrp_state, return NULL;); 
			LIST->wid_vrrp->vir_uplinkname = NULL;
			LIST->wid_vrrp->vir_downlinkname = NULL;
			LIST->wid_vrrp->global_ht_ifname = NULL;
			if (!(dbus_message_get_args ( reply, &err,	
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->vrid,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->flag,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->master_uplinkip,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->master_downlinkip,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->bak_uplinkip,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->bak_downlinkip,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->vir_uplinkip,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->vir_downlinkip,
										DBUS_TYPE_STRING,&vir_uplinkname,
										DBUS_TYPE_STRING,&vir_downlinkname, 							
										DBUS_TYPE_STRING,&global_ht_ifname,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->global_ht_ip,
										DBUS_TYPE_UINT32,&LIST->wid_vrrp->global_ht_opposite_ip,
											DBUS_TYPE_INVALID))){
				
							
					if (dbus_error_is_set(&err)) {
						dbus_error_free(&err);
					}
					return NULL;
				}
				LIST->wid_vrrp->vir_uplinkname = (char*)malloc(strlen(vir_uplinkname));
				if (LIST->wid_vrrp->vir_uplinkname == NULL)
				{
					CW_FREE_OBJECT(LIST->wid_vrrp);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				else
				{
				memset(LIST->wid_vrrp->vir_uplinkname,0,strlen(vir_uplinkname)+1);
				memcpy(LIST->wid_vrrp->vir_uplinkname,vir_uplinkname,strlen(vir_uplinkname));
				}

				LIST->wid_vrrp->vir_downlinkname = (char*)malloc(strlen(vir_downlinkname));
				if (LIST->wid_vrrp->vir_downlinkname == NULL)
				{
					CW_FREE_OBJECT(LIST->wid_vrrp->vir_uplinkname);
					CW_FREE_OBJECT(LIST->wid_vrrp);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				else
				{
				memset(LIST->wid_vrrp->vir_downlinkname,0,strlen(vir_downlinkname)+1);
				memcpy(LIST->wid_vrrp->vir_downlinkname,vir_downlinkname,strlen(vir_downlinkname));
				}

				LIST->wid_vrrp->global_ht_ifname = (char*)malloc(strlen(global_ht_ifname));
				if (LIST->wid_vrrp->global_ht_ifname == NULL)
				{
					CW_FREE_OBJECT(LIST->wid_vrrp->vir_downlinkname);
					CW_FREE_OBJECT(LIST->wid_vrrp->vir_uplinkname);
					CW_FREE_OBJECT(LIST->wid_vrrp);
					CW_FREE_OBJECT(LIST);
					return NULL;
				}
				else
				{
				memset(LIST->wid_vrrp->global_ht_ifname,0,strlen(global_ht_ifname)+1);
				memcpy(LIST->wid_vrrp->global_ht_ifname,global_ht_ifname,strlen(global_ht_ifname));
				}
			}
			break;
		case 11 :{
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&LIST->timer);
			}
			break;
		case 12 :{
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&LIST->num);
			}	
			break;
		case 13 :{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->state);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->number);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->flow);
			}
			break;
		default	:	break;
		}
	}
	dbus_message_unref(reply);
	return LIST;

}
void* dcli_ac_show_wtp_trap_switch(
	int localid,
	int index,
	int *ret,
	DBusConnection *dcli_dbus_connection
	)
{
	WID_TRAP_SWITCH *LIST = NULL;	
	CW_CREATE_OBJECT_ERR(LIST, WID_TRAP_SWITCH, return NULL;);	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRAP_SWITCH_SHOW);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRAP_SWITCH_SHOW);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));/*000*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_ap_run_quit_trap_switch));/*001*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_ap_cpu_trap_switch));/*002*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_ap_mem_trap_switch));/*003*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_flash_write_fail_trap_switch));/*004*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_rrm_change_trap_switch));/*005*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_rogue_ap_threshold_switch));/*006*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_channel_terminal_interference_switch));/*007*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_channel_device_interference_switch));/*008*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_wireless_interface_down_switch));/*009*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_channel_count_minor_switch));/*0010*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_channel_change_switch));/*011*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->dcli_rogue_ap_switch));/*012*/


	dbus_message_unref(reply);
	return LIST;

}
int dcli_ac_set_dynamic_channel_selection_range(
	int localid,
	int index,
	unsigned int rangeNum,
	unsigned char *channelRange,
	DBusConnection *dcli_dbus_connection
	)
{		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusMessageIter  iter2;
	DBusError err;
	int i = 0;
	int ret;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION_RANGE);
	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
	
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rangeNum);

	for(i = 0; i < rangeNum; i++){
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &channelRange[i]);
	}
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return 0;
	}
	
	dbus_message_iter_init(reply,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret);
		
	dbus_message_unref(reply);
	return ret;
}

void dcli_ac_free_fun(char *DBUS_METHOD,DCLI_AC_API_GROUP_ONE *LIST){
	if (LIST == NULL)
		return ;
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_ac_method_parse(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{/*"show_legal_essid_list"*/
				if (LIST->dcli_attack_mac_list != NULL && LIST->dcli_attack_mac_list->attack_mac_list != NULL)
				{
					struct attack_mac_node*pre_attack_mac_node = NULL;
					struct attack_mac_node*next_attack_mac_node = NULL;
					pre_attack_mac_node = next_attack_mac_node = LIST->dcli_attack_mac_list->attack_mac_list;
					while (pre_attack_mac_node != NULL)
					{
						next_attack_mac_node = pre_attack_mac_node->next;
						CW_FREE_OBJECT(pre_attack_mac_node);
						pre_attack_mac_node = next_attack_mac_node;
					}					
				CW_FREE_OBJECT(LIST->dcli_attack_mac_list);
				CW_FREE_OBJECT(LIST);
				}
				else if (LIST->dcli_attack_mac_list != NULL)
				{
					free(LIST->dcli_attack_mac_list);
					free(LIST);
				}
				else
				{}
			}
			break;
			case 2 :{
				if (LIST->dcli_essid_list!= NULL && LIST->dcli_essid_list->essid_list != NULL)
				{
					struct essid_node *pre_essid = NULL;
					struct essid_node *next_essid = NULL;
					pre_essid = next_essid = LIST->dcli_essid_list->essid_list;
					while (pre_essid)
					{
						next_essid = pre_essid->next;
						if (pre_essid->essid != NULL)
							CW_FREE_OBJECT(pre_essid->essid);
						CW_FREE_OBJECT(pre_essid);
						pre_essid = next_essid;
					}
					CW_FREE_OBJECT(LIST->dcli_essid_list);
					CW_FREE_OBJECT(LIST);
				}
				else if (LIST->dcli_essid_list != NULL)
				{
				CW_FREE_OBJECT(LIST->dcli_essid_list);
				CW_FREE_OBJECT(LIST);
				}
				else
				{}
			}
			break;
			case 3 :{
				if (LIST->dcli_oui_list != NULL && LIST->dcli_oui_list->oui_list != NULL)
				{
					struct oui_node *pre_oui_node = NULL;
					struct oui_node *next_oui_node = NULL;
					next_oui_node = pre_oui_node = LIST->dcli_oui_list->oui_list;
					while (pre_oui_node)
					{
						next_oui_node = pre_oui_node->next;
						CW_FREE_OBJECT(pre_oui_node);
						pre_oui_node = next_oui_node;
					}
					CW_FREE_OBJECT(LIST->dcli_oui_list);
					CW_FREE_OBJECT(LIST);
				}
				else if (LIST->dcli_oui_list != NULL)
				{
				CW_FREE_OBJECT(LIST->dcli_oui_list);
				CW_FREE_OBJECT(LIST);
				}
				else
				{}
			}
			break;
			case 4 :{
				if (LIST->dcli_attack_mac_list != NULL && LIST->dcli_attack_mac_list->attack_mac_list != NULL)
				{
					struct attack_mac_node*pre_attack_mac_node = NULL;
					struct attack_mac_node*next_attack_mac_node = NULL;
					pre_attack_mac_node = next_attack_mac_node = LIST->dcli_attack_mac_list->attack_mac_list;
					while (pre_attack_mac_node != NULL)
					{
						next_attack_mac_node = pre_attack_mac_node->next;
						CW_FREE_OBJECT(pre_attack_mac_node);
						pre_attack_mac_node = next_attack_mac_node;
					}
					CW_FREE_OBJECT(LIST->dcli_attack_mac_list);
					CW_FREE_OBJECT(LIST);
				}
				else if (LIST->dcli_attack_mac_list != NULL)
				{
				CW_FREE_OBJECT(LIST->dcli_attack_mac_list);
				CW_FREE_OBJECT(LIST);
				}
				else 
				{}
			}
			break;
			case 5 :{
				if (LIST->dcli_attack_mac_list != NULL && LIST->dcli_attack_mac_list->attack_mac_list != NULL)
				{
					struct attack_mac_node*pre_attack_mac_node = NULL;
					struct attack_mac_node*next_attack_mac_node = NULL;
					pre_attack_mac_node = next_attack_mac_node = LIST->dcli_attack_mac_list->attack_mac_list;
					while (pre_attack_mac_node != NULL)
					{
						next_attack_mac_node = pre_attack_mac_node->next;
						CW_FREE_OBJECT(pre_attack_mac_node);
						pre_attack_mac_node = next_attack_mac_node;
					}
					CW_FREE_OBJECT(LIST->dcli_attack_mac_list);
					CW_FREE_OBJECT(LIST);
				}
				else if (LIST->dcli_attack_mac_list != NULL)
				{
				CW_FREE_OBJECT(LIST->dcli_attack_mac_list);
				CW_FREE_OBJECT(LIST);
				}
				else
				{}
			}
			break;
			case 6 :{

			}
			break;
			default : break;
			
		}

}
void dcli_ac_free_fun_two(char *DBUS_METHOD,DCLI_AC_API_GROUP_TWO *LIST){
	if (LIST == NULL)
		return ;
		int i = 0;
		int dcli_sn = 0;
		struct Neighbor_AP_ELE *temp1 = NULL;
		struct Neighbor_AP_ELE *temp2 = NULL;
		struct tag_wids_device_ele *t1 = NULL;
		struct tag_wids_device_ele *t2 = NULL;
		dcli_sn = dcli_ac_method_parse_two(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				if((LIST)&&(LIST->rouge_ap_list))
				{
					temp1 = LIST->rouge_ap_list->neighborapInfos;
					while(temp1)
					{
						temp2 = temp1->next;
						CW_FREE_OBJECT(temp1->IEs_INFO);
						//CW_FREE_OBJECT(temp1->ESSID);
						CW_FREE_OBJECT(temp1);
						temp1 = temp2;
					}
					CW_FREE_OBJECT(LIST->rouge_ap_list);
				}
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 2 :{
				if((LIST)&&(LIST->rouge_ap_list))
				{
					temp1 = LIST->rouge_ap_list->neighborapInfos;
					while(temp1)
					{
						temp2 = temp1->next;
						CW_FREE_OBJECT(temp1->IEs_INFO);
						//CW_FREE_OBJECT(temp1->ESSID);
						CW_FREE_OBJECT(temp1);
						temp1 = temp2;
					}
					CW_FREE_OBJECT(LIST->rouge_ap_list);
				}
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 3 :{
				if((LIST)&&(LIST->rouge_ap_list))
				{
					temp1 = LIST->rouge_ap_list->neighborapInfos;
					while(temp1)
					{
						temp2 = temp1->next;
						CW_FREE_OBJECT(temp1->IEs_INFO);
						//CW_FREE_OBJECT(temp1->ESSID);
						CW_FREE_OBJECT(temp1);
						temp1 = temp2;
					}
					CW_FREE_OBJECT(LIST->rouge_ap_list);
				}
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 4 :{
				if((LIST)&&(LIST->rouge_ap_list))
				{
					temp1 = LIST->rouge_ap_list->neighborapInfos;
					while(temp1)
					{
						temp2 = temp1->next;
						CW_FREE_OBJECT(temp1->IEs_INFO);
						//CW_FREE_OBJECT(temp1->ESSID);
						CW_FREE_OBJECT(temp1);
						temp1 = temp2;
					}
					CW_FREE_OBJECT(LIST->rouge_ap_list);
				}
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 5 :{
				if((LIST)&&(LIST->wids_device_list))
				{
					t1 = LIST->wids_device_list->wids_device_info;
					while(t1)
					{
						t2 = t1->next;
						CW_FREE_OBJECT(t1);
						t1 = t2;
					}
					CW_FREE_OBJECT(LIST->wids_device_list);
				}
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 6 :{
				if((LIST)&&(LIST->wids_device_list))
				{
					t1 = LIST->wids_device_list->wids_device_info;
					while(t1)
					{
						t2 = t1->next;
						CW_FREE_OBJECT(t1);
						t1 = t2;
					}
					CW_FREE_OBJECT(LIST->wids_device_list);
				}
				CW_FREE_OBJECT(LIST);
			}
			break;
			default : break;
			
		}

}
void dcli_ac_free_fun_three(char *DBUS_METHOD,DCLI_AC_API_GROUP_THREE*LIST){
	if (LIST == NULL)
		return ;
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_ac_method_parse_three(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				CW_FREE_OBJECT(LIST->WTPIP);
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 2 :{
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 3 :{
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 4 :{
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 5 :{
				CW_FREE_OBJECT(LIST);
			}
			break;
			case 6 :{
				if (LIST->ap_statics_list != NULL && LIST->ap_statics_list->ap_statics_ele != NULL)
				{
					wlan_stats_info *pre_ap_statics_ele = NULL;
					wlan_stats_info *next_ap_statics_ele = NULL;
					next_ap_statics_ele = pre_ap_statics_ele = LIST->ap_statics_list->ap_statics_ele;
					while (pre_ap_statics_ele != NULL)
					{
						next_ap_statics_ele = pre_ap_statics_ele->next;
						CW_FREE_OBJECT(pre_ap_statics_ele);
						pre_ap_statics_ele = next_ap_statics_ele;
					}
					CW_FREE_OBJECT(LIST->ap_statics_list);
					CW_FREE_OBJECT(LIST);
				}
				else if (LIST->ap_statics_list != NULL)
				{
					CW_FREE_OBJECT(LIST->ap_statics_list);
					CW_FREE_OBJECT(LIST);
				}
				else
				{}
			}
			break;
			default : break;
			
		}

}
void dcli_ac_free_fun_four(char *DBUS_METHOD,DCLI_AC_API_GROUP_FOUR*LIST){
	if (LIST == NULL)
		return ;
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_ac_method_parse_four(DBUS_METHOD);
		
		switch(dcli_sn){
			case 1 :{
				if (LIST->model_info == NULL)
				{
					CW_FREE_OBJECT(LIST);
					break;
				}
				if (LIST->model_info->model != NULL)
					CW_FREE_OBJECT(LIST->model_info->model);
				if (LIST->model_info->hw_version != NULL)
					CW_FREE_OBJECT(LIST->model_info->hw_version);
				if (LIST->model_info->sw_name != NULL)
					CW_FREE_OBJECT(LIST->model_info->sw_name);
				if (LIST->model_info->sw_version != NULL)
					CW_FREE_OBJECT(LIST->model_info->sw_version);
				if (LIST->model_info->sw_supplier != NULL)
					CW_FREE_OBJECT(LIST->model_info->sw_supplier);
				if (LIST->model_info->supplier != NULL)
					CW_FREE_OBJECT(LIST->model_info->supplier);
				if (LIST->model_info->model != NULL)
					CW_FREE_OBJECT(LIST->model_info->model);
				CW_FREE_OBJECT(LIST->model_info);		
				CW_FREE_OBJECT(LIST);		
			}
			break;
			case 2 :{
				if (LIST->config_ver_info != NULL && LIST->config_ver_info->config_ver_node != NULL)
				{
					CWConfigVersionInfo_dcli *pre_config_ver_node = NULL;
					CWConfigVersionInfo_dcli *next_config_ver_node = NULL;
					next_config_ver_node = pre_config_ver_node = LIST->config_ver_info->config_ver_node;
					while (pre_config_ver_node != NULL)
					{
						next_config_ver_node = pre_config_ver_node->next;
						if (pre_config_ver_node->str_ap_model != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_model);
						if (pre_config_ver_node->str_ap_version_name != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_version_name);
						if (pre_config_ver_node->str_ap_version_path != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_version_path);
						if (pre_config_ver_node->str_ap_code != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_code);
						if (pre_config_ver_node->str_oem_version != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_oem_version);
						memset(pre_config_ver_node, 0, sizeof(CWConfigVersionInfo_dcli));
						CW_FREE_OBJECT(pre_config_ver_node);
						pre_config_ver_node = next_config_ver_node;
					}
					CW_FREE_OBJECT(LIST->config_ver_info); 
					CW_FREE_OBJECT(LIST);	
				}
				else if (LIST->config_ver_info != NULL)
				{
				CW_FREE_OBJECT(LIST->config_ver_info); 
				CW_FREE_OBJECT(LIST);		
				}
				else
				{}
			}
			break;
			case 3 :{
				if (LIST->config_ver_info != NULL && LIST->config_ver_info->config_ver_node != NULL)
				{
					CWConfigVersionInfo_dcli *pre_config_ver_node = NULL;
					CWConfigVersionInfo_dcli *next_config_ver_node = NULL;
					pre_config_ver_node = LIST->config_ver_info->config_ver_node;
					while (pre_config_ver_node != NULL)
					{
						next_config_ver_node = pre_config_ver_node->next;
						if (pre_config_ver_node->str_ap_model != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_model);
						if (pre_config_ver_node->str_ap_version_name != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_version_name);
						if (pre_config_ver_node->str_ap_version_path != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_version_path);
						if (pre_config_ver_node->str_ap_code != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_ap_code);
						if (pre_config_ver_node->str_oem_version != NULL)
							CW_FREE_OBJECT(pre_config_ver_node->str_oem_version);
						memset(pre_config_ver_node, 0, sizeof(CWConfigVersionInfo_dcli));
						CW_FREE_OBJECT(pre_config_ver_node);
						pre_config_ver_node = next_config_ver_node;
					}
					CW_FREE_OBJECT(LIST->config_ver_info); 	
					CW_FREE_OBJECT(LIST);		
				}
				else if (LIST->config_ver_info != NULL)
				{
				CW_FREE_OBJECT(LIST->config_ver_info); 	
				CW_FREE_OBJECT(LIST);		
				}
				else
				{
					CW_FREE_OBJECT(LIST);
				}
			}
			break;
			case 4 :{
				CW_FREE_OBJECT(LIST->code_info);		
				CW_FREE_OBJECT(LIST);		
			}
			break;
			case 5 :{
				if (LIST->bak_sock != NULL && LIST->bak_sock->b_sock_node != NULL)
				{
					struct bak_sock *pre_b_sock_elem = NULL;
					struct bak_sock *next_b_sock_elem = NULL;
					pre_b_sock_elem= LIST->bak_sock->b_sock_node;
					while (pre_b_sock_elem != NULL)
					{
						next_b_sock_elem = pre_b_sock_elem->next;
						CW_FREE_OBJECT(pre_b_sock_elem);
						pre_b_sock_elem = next_b_sock_elem;
					}
					CW_FREE_OBJECT(LIST->bak_sock);
					CW_FREE_OBJECT(LIST);
				}
				else if (LIST->bak_sock != NULL)
				{
				CW_FREE_OBJECT(LIST->bak_sock);
					CW_FREE_OBJECT(LIST);
				}
				else
				{}
			}
			break;
			case 6 :{

			}
			break;
			default : break;
			
		}

}
void dcli_ac_free_fun_five(char *DBUS_METHOD,DCLI_AC_API_GROUP_FIVE*LIST){
	if (LIST == NULL)
		return ;
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_ac_method_parse_five(DBUS_METHOD);
		
		switch(dcli_sn){
			case 1 :{
					if (LIST->wireless_control->ac_name != NULL)
					{
						CW_FREE_OBJECT(LIST->wireless_control->ac_name);
					}
					if (LIST->wireless_control->hw_version_char != NULL)
					{
						CW_FREE_OBJECT(LIST->wireless_control->hw_version_char);
					}
					if (LIST->wireless_control->sw_version_char != NULL)
					{
						CW_FREE_OBJECT(LIST->wireless_control->sw_version_char);
					}
					CW_FREE_OBJECT(LIST->wireless_control);		
					CW_FREE_OBJECT(LIST); 	
			}
			break;
			case 2 :{
				CW_FREE_OBJECT(LIST); 	
			}
			break;
			case 3 :{
				if (LIST->auto_login != NULL && LIST->auto_login->auto_ap_if != NULL)
				{
					wid_auto_ap_if	*pre_auto_ap_if = NULL;
					wid_auto_ap_if	*next_auto_ap_if = NULL;
					pre_auto_ap_if = next_auto_ap_if = LIST->auto_login->auto_ap_if;
					while (pre_auto_ap_if != NULL)
					{
						next_auto_ap_if = pre_auto_ap_if->ifnext;
						memset(pre_auto_ap_if, 0, sizeof(wid_auto_ap_if));
						CW_FREE_OBJECT(pre_auto_ap_if);
						pre_auto_ap_if = next_auto_ap_if;
					}
					CW_FREE_OBJECT(LIST->auto_login);		
					CW_FREE_OBJECT(LIST);	
				}
				else if (LIST->auto_login != NULL)
				{
				CW_FREE_OBJECT(LIST->auto_login);		
				CW_FREE_OBJECT(LIST);		
				}
				else
				{}
			}
			break;
			case 4 :{
				CW_FREE_OBJECT(LIST->tx_control); 
				CW_FREE_OBJECT(LIST); 		
			}
			break;
			case 5 :{
				CW_FREE_OBJECT(LIST);		
			}
			break;
			case 6 :{
				CW_FREE_OBJECT(LIST->sample_info);		
				CW_FREE_OBJECT(LIST);		
			}
			break;			
			case 7 :{
				CW_FREE_OBJECT(LIST);		
			}	
			break;
			case 8 :{
				CW_FREE_OBJECT(LIST);		
			}
			break;
			case 9 :{
				CW_FREE_OBJECT(LIST);		
			}
			break;
			case 10 :{
				if (LIST->wid_vrrp->vir_uplinkname != NULL)
				{
					CW_FREE_OBJECT(LIST->wid_vrrp->vir_uplinkname);
				}
				if (LIST->wid_vrrp->vir_downlinkname != NULL)
				{
					CW_FREE_OBJECT(LIST->wid_vrrp->vir_downlinkname);
				}
				if (LIST->wid_vrrp->global_ht_ifname != NULL)
					CW_FREE_OBJECT(LIST->wid_vrrp->global_ht_ifname);
				CW_FREE_OBJECT(LIST->wid_vrrp);	
				CW_FREE_OBJECT(LIST); 
			}
			break;
			case 11 :{
				CW_FREE_OBJECT(LIST);		
			}
			break;
			case 12 :{
				CW_FREE_OBJECT(LIST);	
			}
			break;
			case 13 :{
				CW_FREE_OBJECT(LIST);		
			}
			break;			
			case 14 :{

			}
			break;
			default : break;
			
		}

}

void dcli_wtp_free_fun_trap_switch(WID_TRAP_SWITCH *LIST)
{
	CW_FREE_OBJECT(LIST);
}
int dcli_ac_add_listen_if_node(Listen_IF *listen_if,char *ifname,unsigned int addr,LISTEN_FLAG flag)
{
	if((listen_if == NULL)||(ifname == NULL))
	{
		return -1;
	}
	struct Listenning_IF *node = NULL;
	node =(struct Listenning_IF *) malloc(sizeof(struct Listenning_IF)+1);
	if (node == NULL)
	{
		return -1;
	}
	memset(node,0,(sizeof(struct Listenning_IF)+1));
	memcpy(node->ifname,ifname,strlen(ifname));
	node->ipaddr = addr;
	node->lic_flag = flag;
	if(listen_if->count == 0)
	{
		listen_if->interface = node;
		node->if_next = NULL;
		listen_if->count++;
		return 0;
	}
	struct Listenning_IF *tmp=NULL;
	tmp = listen_if->interface;
	while(tmp->if_next){
		tmp = tmp->if_next;
	}
	tmp->if_next = node;
	node->if_next = NULL;
	listen_if->count++;
	return 0;
}
int dcli_ac_free_listen_if_node(Listen_IF *listen_if)
{
	if(listen_if == NULL)
	{
		return -1;
	}
	struct Listenning_IF *tmp=NULL;
	tmp = listen_if->interface;
	while(tmp){
		listen_if->interface = tmp->if_next;
		tmp->if_next = NULL;
		free(tmp);
		tmp = NULL;
		tmp = listen_if->interface;
		listen_if->count--;
		//printf("listen_if->count=%d\n",listen_if->count);
	}
	if(listen_if != NULL){
		free(listen_if);
		listen_if = NULL;
	}
	return 0;
}

/* 
  *****************************************************************************
  *
  * NOTES:	get bakup router's synchronization interval time(all of the information in Master),  unit: second
  * INPUT:	localid:	Whether to use local hansi config, 1 - yes, 0 - no
  *			index:	the number of AC count control WTP-groups
  *			ret:		LicBakReqInterval, if success get; otherwise -1
  *			dcli_dbus_connection:	DBus connection info
  * OUTPUT:	 
  * return:	 -1 - invalid value
  *  
  * author: 		Huang Leilei
  * begin time:	2012-10-24 9:00 
  * finish time:		2012-10-24 11:00 
  * history:		
  *
  *****************************************************************************
  */
int
dcli_ac_show_bak_check_interval(int localid, int index, int *ret, DBusConnection *dcli_dbus_connection)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int bak_check_interval = 0;

	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_CONF_METHOD_BAK_CHECK_INTERVAL_SHOW);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&bak_check_interval);
	*ret = bak_check_interval;

	dbus_message_unref(reply);
	
	return bak_check_interval;
}

/* 
  *****************************************************************************
  *
  * NOTES:	get bakup router's lincense info synchronization interval time,  unit: second
  * INPUT:	localid:	Whether to use local hansi config, 1 - yes, 0 - no
  *			index:	the number of AC count control WTP-groups
  *			ret:		LicBakReqInterval, if success get; otherwise -1
  *			dcli_dbus_connection:	DBus connection info
  * OUTPUT:	 
  * return:	 -1 - invalid value
  *  
  * author: 		Huang Leilei
  * begin time:	2012-10-24 14:36 
  * finish time:		2012-10-24  
  * history:		
  *
  *****************************************************************************
  */
int
dcli_ac_show_lic_bak_req_interval(int localid, int index, int *ret, DBusConnection *dcli_dbus_connection)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int lic_bak_req_interval = 0;

	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_CONF_METHOD_LIC_BAK_REQ_INTERVAL_SHOW);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&lic_bak_req_interval);
	*ret = lic_bak_req_interval;

	dbus_message_unref(reply);
	
	return lic_bak_req_interval;
}

void* dcli_ac_show_wid_listen_if(
	int localid,
	int index,
	int *ret,
	DBusConnection *dcli_dbus_connection
	)
{

	DBusMessage *query = NULL;
	DBusMessage  *reply = NULL;	
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	Listen_IF *IF_LIST = NULL;
	CW_CREATE_OBJECT_ERR(IF_LIST, Listen_IF, return NULL;);  
	memset(IF_LIST,0,sizeof(Listen_IF));
	unsigned int listen_if_count = 0;
	char * ifname = NULL;
	unsigned int ipaddr = 0;
	LISTEN_FLAG lic_flag = LIC_TYPE;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_LISTEN_L3_INTERFACE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	  
	if (NULL == reply)
	{
		  printf("<error> failed get reply.\n");
		  if (dbus_error_is_set(&err))
		  {
			  printf("%s raised: %s",err.name,err.message);
			 dbus_error_free_for_dcli(&err);
		  }
		  *ret = -1;
		  return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&listen_if_count);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);
	int  i =0;
	for(i=0;i<listen_if_count;i++)
	{
	  DBusMessageIter iter_struct;

	  dbus_message_iter_recurse(&iter_array,&iter_struct);

	  dbus_message_iter_get_basic(&iter_struct,&ifname);
	  //printf("1,%s.\n",ifname);
	  dbus_message_iter_next(&iter_struct);
	  dbus_message_iter_get_basic(&iter_struct,&ipaddr);
	  dbus_message_iter_next(&iter_struct);
	  dbus_message_iter_get_basic(&iter_struct,&lic_flag);

	  dbus_message_iter_next(&iter_array);
	 // printf("2,%d.%d.%d.%d.\n",(ipaddr>>24)&0xFF,(ipaddr>>16)&0xFF,(ipaddr>>8)&0xFF,(ipaddr)&0xFF);
	  dcli_ac_add_listen_if_node(IF_LIST,ifname,ipaddr,lic_flag);
	}

	dbus_message_unref(reply);
	return IF_LIST;
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
			//printf("secondid=%d,\n",*secondid);
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
int set_wids_judge_policy(int index,int localid,int mode,int policy,DBusConnection *dcli_dbus_connection){		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusError err;
	int i = 0;
	int ret;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDS_JUDGE_POLICY);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mode,	
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free_for_dcli(&err);
		}
		
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	return ret;
}

int show_wids_judge_policy(int index,int localid,int *mode,DBusConnection *dcli_dbus_connection)	
{		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusError err;
	int i = 0;
	int state = 0;
	int ret;
	int policy;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDS_JUDGE_POLICY_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&policy);
	
	 *mode = policy;

	dbus_message_unref(reply);
	
	return ret;

}
int set_wids_scanning_mode(int index,int localid,unsigned int wtpid,int mode,DBusConnection *dcli_dbus_connection){		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusError err;
	int i = 0;
	int ret;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDS_SCANNING_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mode,							 
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	return ret;
}

int set_wids_monitor_mode(int index,int localid,unsigned int wtpid,int mode,DBusConnection *dcli_dbus_connection){		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusError err;
	int i = 0;
	int ret;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDS_MONITOR_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mode,							 
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	return ret;
}
void *show_wids_scanning_mode_channel(int index,int localid,unsigned int wtpid,int *ret,DBusConnection *dcli_dbus_connection){		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusError err;
	int i = 0;
	int state = 0;
	unsigned int channel_t = 0;
	DCLI_WTP_API_GROUP_ONE *LIST = NULL;
	CW_CREATE_OBJECT_ERR(LIST, DCLI_WTP_API_GROUP_ONE, return NULL;);	
	LIST->WTP =  (WID_WTP**)malloc(sizeof(WID_WTP *));/*分配一个空间即可*/
	LIST->WTP[0] =	(WID_WTP*)malloc(sizeof(WID_WTP));
	memset(&(LIST->WTP[0]->WIDS),0,sizeof(SCNANNING_MODE));
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDS_SCANNING_MODE_CHANNEL_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,	
							 DBUS_TYPE_UINT32,&wtpid,   
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WIDS.num));
	//printf("num is %d\n",(LIST->WIDS.num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WIDS.monitorMode));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WIDS.scanningMode));

	if(LIST->WTP[0]->WIDS.scanningMode == 3){

		for(i = 0;((i<LIST->WTP[0]->WIDS.num)&&(i<SCANNING_CHANNEL_NUM));i++){

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&channel_t);

			LIST->WTP[0]->WIDS.channel[i] = channel_t;
		}
	}
	dbus_message_unref(reply);
	return LIST;
}
int parse_input_list(char* ptr,int* count,int output[],int output_array_len,int min,int max)
{
	char* endPtr = NULL;
	int   i=0;
	int  j=0;
	endPtr = ptr;
	check_state state = check_id;

	while(1){
		switch(state)
		{
			case check_id: 
				output[i] = strtoul(endPtr,&endPtr,10);
			
				if(output[i]>=min&&output[i]<=max)
				{
	        		state=check_comma;	
				}
				else
					state=check_fail;
				break;
			case check_comma: 
				if (CHECK_COMMA == endPtr[0]){
					endPtr = (char*)endPtr + 1;
					state = check_id;
				
					i++;
					if(i>=output_array_len) return -1;
				}
				else
					state = check_end;
				break;
			case check_fail:
				return -1;
				break;
			case check_end: 
				if ('\0' == endPtr[0]) {
						state = check_success;
						i++;
						if(i>=output_array_len) return -1;
				}
				else
					state = check_fail;
				break;
			case check_success: 
				*count = i;
				return 0;
				break;
			default: break;
		}
		
	}
		
}

int process_input_list(int iArray[],int num) 
{ 

	int iTemp,i,j,iCount=1; 
	/*排序 */
	for(i=1;i<num;i++)
	{
		for(j=0;j<i;j++) 
		{ 
			if(iArray[i]<iArray[j]) 
			{ 
				iTemp=iArray[i]; 
				iArray[i]=iArray[j]; 
				iArray[j]=iTemp; 

			} 
		} 
	}
	i=0; 
	j=1; 
	//printf("i is %d,j is %d\n",i,j);
	/*去除重复的*/
	while(j<num) 
	{ 
		if(iArray[i]==iArray[j]) 
		{ 
			j++; 
		} 
		else 
		{ 
			iArray[++i]=iArray[j]; 
			j++; 
			iCount++; 
		} 
	} 
	
	return iCount;
}

int set_wids_scanning_channel(int index,int localid,unsigned int wtpid,unsigned int num,int list[],DBusConnection *dcli_dbus_connection){		
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;	
	DBusError err;
	int i = 0;
	int ret;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDS_SCANNING_CHANNEL);
	dbus_error_init(&err);
		
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtpid);
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&num);
	for(i = 0; i < num; i++)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&list[i]);
	}
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	return ret;
}
void dcli_ac_free_fun_scanning_show(DCLI_WTP_API_GROUP_ONE *LIST)
{
	if((LIST != NULL)&&(LIST->WTP)){
		CW_FREE_OBJECT(LIST->WTP[0]);
		CW_FREE_OBJECT(LIST->WTP);
	}
	CW_FREE_OBJECT(LIST);
}
#endif

