#ifdef _D_WCPSS_
#include <string.h>
//#include <zebra.h>
//#include "vtysh/vtysh.h"
#include <dbus/dbus.h>

//#include "command.h"

//#include "dcli_main.h"
//#include "dcli_wlan.h"
//#include "wid_ac.h"
//#include "wid_wtp.h"
//#include "wid_wlan.h"
#include "wid_wlan.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/dcli_wid_wlan.h"

void str2lower(char **str) {  
	int i,len;
	char *ptr;

	len = strlen(*str);
	ptr = *str;

	for(i=0; i<len ; i++) {
		if((ptr[i]<='Z')&&(ptr[i]>='A'))  
			ptr[i] = ptr[i]-'A'+'a';  
	}
	
	return;
}

int strcheck(char **str) {  
	int i,len;
	char *ptr;

	len = strlen(*str);
	ptr = *str;

	for(i=0; i<len ; i++) {
		if((ptr[i]<32)||(ptr[i]>126))  
			return 0;  
	}
	
	return 1;
}

int parse_int(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
			return WID_DBUS_SUCCESS;
		else
			return WID_UNKNOWN_ID;
	}
	else
		return WID_UNKNOWN_ID;
}


int parse_char_ID(char* str,unsigned char* ID){
	 /* before modify*/
	char *endptr = NULL;
	char c;
	unsigned long int t_ID = 0;
	c = str[0];
	if (c>='0'&&c<='9'){
		t_ID=  strtoul(str,&endptr,10);
		if(t_ID < 256){
          *ID = (unsigned char)t_ID;
          if((c=='0')&&(str[1]!='\0'))
		  	return WID_UNKNOWN_ID;
		  else if(endptr[0] == '\0')
			return WID_DBUS_SUCCESS;
		else
			return WID_UNKNOWN_ID;
		}
		else{
            return WID_ILLEGAL_INPUT;
		}
	}
	else
		return WID_UNKNOWN_ID;
	
}
/*wuwl add*/
unsigned long int HexToDec(unsigned char *hex)
{
	char *tmp=hex;
	unsigned int dec=0;
	//int m = 0;
	//printf("tmp:%p\n");
	while (*tmp)
	{
		//m++;
		//printf("$m:%d$",m);
		dec<<=4;
		//printf("$dec:%d$",dec);
		if (*tmp&16) dec+=*tmp++&15;
		else dec+=(*tmp++&15)+9;
	}
	//printf("dec:%d\n",dec);
	return dec;
}
/* wuwl add */
CWBool_DCLI check_ascii_32_to126(const char * str)
{
	if(str == NULL)
	{
		return CW_TRUE_DCLI;
	}

	CWBool_DCLI ret = CW_TRUE_DCLI;
	const char *p = str;

	while(*p != '\0')
	{
		//printf("*p = %d\n",*p);
		if((*p < 32 )||(*p > 126))
		{
			ret = CW_FALSE_DCLI;
			break;
		}
		p++;
	}
	//printf("ret = %d\n",ret);
	return ret;
}

/*sz20080820*/
void CheckWIDIfPolicy(char *whichinterface, unsigned char wlan_if_policy){
	
	switch(wlan_if_policy){

		case 0 :
			strcpy(whichinterface, "NO_IF");
			break;
			
		case 1 :
			strcpy(whichinterface, "WLAN_IF");
			break;
		
		case 2 :
			strcpy(whichinterface, "BSS_IF");
			break;
		}
}

int dcli_wlan_method_parse(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WLANLIST))
	{
		sn = 1;/*"show wlan (list|all)"*/
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOWWLAN))
	{
		sn = 2;/*"show wlan WLANID"*/
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO))
	{
		sn = 3;/*"show wlan_vlan info"*/
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION))
	{
		sn = 4;/*"show bridge_isolation infomation"*/
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN))
	{
		sn = 5;/*"show tunnel wlan-vlan infomation"*/
	}	
	else// if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METsHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID))
	{
	//	sn = 6;/**/
	}
	return sn;
}
void dcli_free_SSIDConfigInfo_head(struct SSIDConfigInfo *head)
{
	struct SSIDConfigInfo * temp=NULL;
	while(head){
		temp=head;
		head=temp->next;
		if(temp->NewSSIDName){
			free(temp->NewSSIDName);
			temp->NewSSIDName=NULL;
			}
		if(temp->NewSecurityKEY){
			free(temp->NewSecurityKEY);
			temp->NewSecurityKEY=NULL;
			}
		if(temp->NewAuthIP){
			free(temp->NewAuthIP);
			temp->NewAuthIP=NULL;
			}
		if(temp->NewAcctIP){
			free(temp->NewAcctIP);
			temp->NewAcctIP=NULL;
			}
		if(temp->NewAuthSharedSecret){
			free(temp->NewAuthSharedSecret);
			temp->NewAuthSharedSecret=NULL;
			}
		if(temp->NewAcctSharedSecret){
			free(temp->NewAcctSharedSecret);
			temp->NewAcctSharedSecret=NULL;
			}
		if(temp->asip){
			free(temp->asip);
			temp->asip=NULL;
		}
		if(temp->as_path){
			free(temp->as_path);
			temp->as_path=NULL;
		}
		if(temp->ae_path){
			free(temp->ae_path);
			temp->ae_path=NULL;
		}
		free(temp);
		temp=NULL;			
		}
}

void dcli_free_wtp_UnicastInfo(struct UnicastInfo *WlanNode)
{
	struct UnicastInfo *tmp = NULL;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->UnicastInfo_last != NULL) {
		WlanNode->UnicastInfo_last = NULL;
	}
	while(WlanNode->UnicastInfo_list != NULL) {
		tmp = WlanNode->UnicastInfo_list;
		WlanNode->UnicastInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->NewUnicastCipher);		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
		
	}
	free(WlanNode);
	WlanNode = NULL;
	return ;
}

void dcli_free_wtp_ConfigWapiInfoInfo(struct ConfigWapiInfo *WlanNode)
{
	struct ConfigWapiInfo *tmp = NULL;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->ConfigWapiInfo_last != NULL) {
		WlanNode->ConfigWapiInfo_last = NULL;
	}
	while(WlanNode->ConfigWapiInfo_list != NULL) {
		tmp = WlanNode->ConfigWapiInfo_list;
		WlanNode->ConfigWapiInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->WAPIASIPAddress);		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
		
	}
	free(WlanNode);
	WlanNode = NULL;
	return ;
}
/*table 21-2 free */
void dcli_free_Sub_Sta_WtpWAPIPerformance(struct Sub_Sta_WtpWAPIPerformance *StaNode)
{
	struct Sub_Sta_WtpWAPIPerformance *tmp = NULL;
	struct Sub_Sta_WtpWAPIPerformance *t = StaNode;

	if(StaNode == NULL)
		return ;
	
	if(StaNode->Sub_Sta_WtpWAPIPerformance_last != NULL) {
		StaNode->Sub_Sta_WtpWAPIPerformance_last = NULL;
	}

	while(t != NULL) {
		tmp = t;
		t = t->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->staMacAddr);	
		
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	return ;
}
/*table 21 free */
void dcli_free_WtpWAPIPerformanceStatsInfo(struct WtpWAPIPerformanceStatsInfo *WlanNode)
{
	struct WtpWAPIPerformanceStatsInfo *tmp = NULL;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->WtpWAPIPerformanceStatsInfo_last != NULL) {
		WlanNode->WtpWAPIPerformanceStatsInfo_last = NULL;
	}
	while(WlanNode->WtpWAPIPerformanceStatsInfo_list != NULL) {
		tmp = WlanNode->WtpWAPIPerformanceStatsInfo_list;
		WlanNode->WtpWAPIPerformanceStatsInfo_list = tmp->next;

		dcli_free_Sub_Sta_WtpWAPIPerformance(tmp->Sub_Sta_WtpWAPIPerformance_head);
		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
		
	}
	free(WlanNode);
	WlanNode = NULL;
	return ;
}

void dcli_free_WAPI_WLAN_ExtendConfigInfo(struct WtpWAPIExtendConfigInfo *WlanNode)
{
	struct WtpWAPIExtendConfigInfo *tmp = NULL;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->WtpWAPIExtendConfigInfo_last != NULL) {
		WlanNode->WtpWAPIExtendConfigInfo_last = NULL;
	}
	while(WlanNode->WtpWAPIExtendConfigInfo_list != NULL) {
		tmp = WlanNode->WtpWAPIExtendConfigInfo_list;
		WlanNode->WtpWAPIExtendConfigInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiPSKValue);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiPSKPassPhrase);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiControlledPortControl);		

		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiUnicastCipherSelected);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiMulticastCipherSelected);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiUnicastCipherRequested);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWapiMulticastCipherRequested);		
		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
		
	}
	free(WlanNode);
	WlanNode = NULL;
	return ;
}

/*table 28-2 free */
void dcli_free_Sub_BssWAPIPerformanceStatsInfo(struct Sub_BssWAPIPerformanceStatsInfo *BssNode)
	
{
	struct Sub_BssWAPIPerformanceStatsInfo *tmp = NULL;
	struct Sub_BssWAPIPerformanceStatsInfo *t = BssNode;

	if(BssNode == NULL)
		return ;
	
	if(BssNode->Sub_BssWAPIPerformanceStatsInfo_last != NULL) {
		BssNode->Sub_BssWAPIPerformanceStatsInfo_last = NULL;
	}

	while(t != NULL) {
		tmp = t;
		t = t->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpBssCurrID);	

		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	return ;
}

/*table 28 free */
void dcli_free_BssWAPIPerformanceStatsInfo(struct BssWAPIPerformanceStatsInfo *WlanNode)
{
	struct BssWAPIPerformanceStatsInfo *tmp = NULL;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->BssWAPIPerformanceStatsInfo_last != NULL) {
		WlanNode->BssWAPIPerformanceStatsInfo_last = NULL;
	}
	while(WlanNode->BssWAPIPerformanceStatsInfo_list != NULL) {
		tmp = WlanNode->BssWAPIPerformanceStatsInfo_list;
		WlanNode->BssWAPIPerformanceStatsInfo_list = tmp->next;

		dcli_free_Sub_BssWAPIPerformanceStatsInfo(tmp->Sub_BssWAPIPerformanceStatsInfo_head);
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
		
	}
	free(WlanNode);
	WlanNode = NULL;
	return ;
}

/*for showting ConfigWapiInfo by nl 20100521 bb1*/
struct ConfigWapiInfo* show_ConfigWapiInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	

	int i;
	struct ConfigWapiInfo  *WlanNode = NULL;
	struct ConfigWapiInfo  *WlanHead = NULL;
	struct ConfigWapiInfo  *WlanSearchNode = NULL;
	unsigned char wlanid[WLAN_NUM] = {0}; //should make 

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
								INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_BASIC_INFO_BY_WLAN_OF_ALL_WTP);
		
	dbus_error_init(&err);
			
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,(num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if((WlanHead = (struct ConfigWapiInfo*)malloc(sizeof(struct ConfigWapiInfo))) == NULL){
		*ret = MALLOC_ERROR;
		dbus_message_unref(reply);
		return NULL;
		}
		memset(WlanHead,0,sizeof(struct ConfigWapiInfo));
		WlanHead->ConfigWapiInfo_list = NULL;
		WlanHead->ConfigWapiInfo_last = NULL;
	
	if((*ret == 0)&&(*num !=0)){	
		for(i=0;i<*num;i++){
			if((WlanNode = (struct ConfigWapiInfo*)malloc(sizeof(struct ConfigWapiInfo))) == NULL){
							dcli_free_wtp_RadioStatsInfo(WlanHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
			}
					
			memset(WlanNode,0,sizeof(struct ConfigWapiInfo));
			WlanNode->next = NULL;

			if(WlanHead->ConfigWapiInfo_list == NULL){
				WlanHead->ConfigWapiInfo_list = WlanNode;
				WlanHead->next = WlanNode;
			}
			else{
				WlanHead->ConfigWapiInfo_last->next = WlanNode;
			}
			WlanHead->ConfigWapiInfo_last = WlanNode;
	
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->WlanId));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->WapiCipherKeyCharType));

			char *ip = NULL;
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ip));

			WlanNode->WAPIASIPAddress = (char*)malloc(strlen(ip)+1);
			memset(WlanNode->WAPIASIPAddress, 0, strlen(ip)+1);
			memcpy(WlanNode->WAPIASIPAddress, ip, strlen(ip));	

			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WlanHead;
}

/*for showting WtpWAPIPerformanceStatsInfo by nl 20100529 bb2*/
struct WtpWAPIPerformanceStatsInfo* show_WtpWAPIPerformanceStatsInfo_of_all_wlan(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_sub_struct;
	DBusMessageIter	 iter_sub_sub_array;
	DBusMessageIter	 iter_sub_sub_struct;
	
	struct WtpWAPIPerformanceStatsInfo  *WlanNode = NULL;
	struct WtpWAPIPerformanceStatsInfo  *WlanHead = NULL;
	struct WtpWAPIPerformanceStatsInfo  *WlanSearchNode = NULL;
	struct Sub_Sta_WtpWAPIPerformance * StaNode = NULL;
	
	int i;
	int j;
	int k;
	unsigned char bss_id;
	unsigned char wlan_id;
	int bss_sta_num;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
								INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_STATS_INFO_OF_ALL_WLAN);
		
	dbus_error_init(&err);
			
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,(num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if((WlanHead = (struct WtpWAPIPerformanceStatsInfo*)malloc(sizeof(struct WtpWAPIPerformanceStatsInfo))) == NULL){
		*ret = MALLOC_ERROR;
		dbus_message_unref(reply);
		return NULL;
		}
		memset(WlanHead,0,sizeof(struct WtpWAPIPerformanceStatsInfo));
		WlanHead->WtpWAPIPerformanceStatsInfo_list = NULL;
		WlanHead->WtpWAPIPerformanceStatsInfo_last = NULL;
		
	if((*ret == 0)&&(*num !=0)){	
		for(i=0;i<*num;i++){
			if((WlanNode = (struct WtpWAPIPerformanceStatsInfo*)malloc(sizeof(struct WtpWAPIPerformanceStatsInfo))) == NULL){
							dcli_free_WtpWAPIPerformanceStatsInfo(WlanHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
			}
					
			memset(WlanNode,0,sizeof(struct WtpWAPIPerformanceStatsInfo));
			WlanNode->next = NULL;

			if(WlanHead->WtpWAPIPerformanceStatsInfo_list == NULL){
				WlanHead->WtpWAPIPerformanceStatsInfo_list = WlanNode;
				WlanHead->next = WlanNode;
			}
			else{
				WlanHead->WtpWAPIPerformanceStatsInfo_last->next = WlanNode;
			}
			WlanHead->WtpWAPIPerformanceStatsInfo_last = WlanNode;
	
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->WlanId));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityType));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wlan_bss_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WlanNode->wlan_bss_num; j++){  
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_id));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wlan_id));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_sta_num));

				WlanNode->wlan_total_sta_num += bss_sta_num;
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);
				
				for(k=0;k<bss_sta_num;k++){
					if((StaNode = (struct Sub_Sta_WtpWAPIPerformance*)malloc(sizeof(struct Sub_Sta_WtpWAPIPerformance))) == NULL){
										dcli_free_WtpWAPIPerformanceStatsInfo(WlanHead);
										*ret = MALLOC_ERROR;
										dbus_message_unref(reply);
										return NULL;
					}
								
					memset(StaNode,0,sizeof(struct Sub_Sta_WtpWAPIPerformance));
					StaNode->next = NULL;
					StaNode->Sub_Sta_WtpWAPIPerformance_list = NULL;
					StaNode->Sub_Sta_WtpWAPIPerformance_last = NULL;
	
					if(WlanNode->Sub_Sta_WtpWAPIPerformance_head == NULL){
						WlanNode->Sub_Sta_WtpWAPIPerformance_head = StaNode;
					}else{
						WlanNode->Sub_Sta_WtpWAPIPerformance_head->Sub_Sta_WtpWAPIPerformance_last->next = StaNode;
						
					}
					WlanNode->Sub_Sta_WtpWAPIPerformance_head->Sub_Sta_WtpWAPIPerformance_last = StaNode;

					
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(StaNode->sta_seq));

					StaNode->staMacAddr = (unsigned char *)malloc(MAC_LEN +1);
					memset(StaNode->staMacAddr,0,(MAC_LEN +1));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->staMacAddr[0]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->staMacAddr[1]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->staMacAddr[2]));
					
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->staMacAddr[3]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->staMacAddr[4]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->staMacAddr[5]));

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiVersion));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiControlledPortStatus));

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiSelectedUnicastCipher[0]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiSelectedUnicastCipher[1]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiSelectedUnicastCipher[2]));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiSelectedUnicastCipher[3]));

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWPIReplayCounters));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWPIDecryptableErrors));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWPIMICErrors));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAISignatureErrors));

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAIHMACErrors));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAIAuthResultFailures));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAIDiscardCounters));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAITimeoutCounters));

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAIFormatErrors));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAICertificateHandshakeFailures));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAIUnicastHandshakeFailures));
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,
												&(StaNode->wtpWapiWAIMulticastHandshakeFailures));
					
					dbus_message_iter_next(&iter_sub_sub_array);
				}
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WlanHead;
}

/*for showting WtpWAPIExtendConfigInfo by nl 20100601 bb3*/
struct WtpWAPIExtendConfigInfo* show_All_WAPIWlan_ExtendConfigInfo(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	
	struct WtpWAPIExtendConfigInfo  *WlanNode = NULL;
	struct WtpWAPIExtendConfigInfo  *WlanHead = NULL;
	struct WtpWAPIExtendConfigInfo  *WlanSearchNode = NULL;
	
	unsigned char WapiPreauth ;
	unsigned char MulticaseRekeyStrict;
			
	unsigned char wapi_ucast_rekey_method;
	unsigned int wapi_ucast_rekey_para_t;
	unsigned int wapi_ucast_rekey_para_p;
	unsigned char wapi_mcast_rekey_method;
	unsigned int wapi_mcast_rekey_para_t;
	unsigned int wapi_mcast_rekey_para_p;
	char *key;
	int i;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
								INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_EXTEND_CONFIG_INFO_BY_WLAN_OF_ALL_WTP);
		
	dbus_error_init(&err);
			
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,(num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if((WlanHead = (struct WtpWAPIExtendConfigInfo*)malloc(sizeof(struct WtpWAPIExtendConfigInfo))) == NULL){
		*ret = MALLOC_ERROR;
		dbus_message_unref(reply);
		return NULL;
		}
		memset(WlanHead,0,sizeof(struct WtpWAPIExtendConfigInfo));
		WlanHead->WtpWAPIExtendConfigInfo_list = NULL;
		WlanHead->WtpWAPIExtendConfigInfo_last = NULL;
	
	if((*ret == 0)&&(*num !=0)){	

		for(i=0;i<*num;i++){
			int j = 0;	
			unsigned char akm = 0;
			unsigned char bkid[16] = {0};
			if((WlanNode = (struct WtpWAPIExtendConfigInfo*)malloc(sizeof(struct WtpWAPIExtendConfigInfo))) == NULL){
							dcli_free_WAPI_WLAN_ExtendConfigInfo(WlanHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
			}
					
			memset(WlanNode,0,sizeof(struct WtpWAPIExtendConfigInfo));
			WlanNode->next = NULL;

			if(WlanHead->WtpWAPIExtendConfigInfo_list == NULL){
				WlanHead->WtpWAPIExtendConfigInfo_list = WlanNode;
				WlanHead->next = WlanNode;
			}
			else{
				WlanHead->WtpWAPIExtendConfigInfo_last->next = WlanNode;
			}
			WlanHead->WtpWAPIExtendConfigInfo_last = WlanNode;
	
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->WapiWlanID));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityType));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiCertificateUpdateCount));//a1
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiMulticastUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiUnicastUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiBKLifetime));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiBKReauthThreshold));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiSATimeout));			//a6
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WapiPreauth));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(MulticaseRekeyStrict));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiControlledAuthenControlenabled));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiMulticastCipher[0]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiMulticastCipher[1]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiMulticastCipher[2]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wtpWapiMulticastCipher[3]));	

			if(WapiPreauth == 1){
	         	WlanNode->wtpWapiWPIPreauthImplemented = 1;
				WlanNode->wtpWapiEnabled = 1;
				WlanNode->wtpWapiPreauthEnabled = 1;
			}
			else if(WapiPreauth == 0){
				WlanNode->wtpWapiWPIPreauthImplemented = 2;
				WlanNode->wtpWapiEnabled = 2;
				WlanNode->wtpWapiPreauthEnabled = 2;
			}

			if(MulticaseRekeyStrict == 1){
				WlanNode->wtpWapiMulticastRekeyStrict = 1;
			}
	   		else if(MulticaseRekeyStrict == 0){
				WlanNode->wtpWapiMulticastRekeyStrict = 2;
	    	}

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapi_ucast_rekey_method));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapi_ucast_rekey_para_t));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapi_ucast_rekey_para_p));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapi_mcast_rekey_method));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapi_mcast_rekey_para_t));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapi_mcast_rekey_para_p));	

			WlanNode->wtpWapiUnicastRekeyMethod = wapi_ucast_rekey_method + 1;
			WlanNode->wtpWapiUnicastRekeyTime = wapi_ucast_rekey_para_t;
			WlanNode->wtpWapiUnicastRekeyPackets = wapi_ucast_rekey_para_p;
			
			WlanNode->wtpWapiMulticastRekeyMethod = wapi_mcast_rekey_method + 1;
			WlanNode->wtpWapiMulticastRekeyTime = wapi_mcast_rekey_para_t; 
		    WlanNode->wtpWapiMulticastRekeyPackets = wapi_mcast_rekey_para_p; 

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(key));	

			WlanNode->wtpWapiPSKValue = (char *)malloc(strlen(key)+1);
			memset(WlanNode->wtpWapiPSKValue,0,strlen(key)+1);
			strcpy(WlanNode->wtpWapiPSKValue,key);

			WlanNode->wtpWapiPSKPassPhrase = (char *)malloc(strlen(key)+1);
			memset(WlanNode->wtpWapiPSKPassPhrase,0,strlen(key)+1);
			strcpy(WlanNode->wtpWapiPSKPassPhrase,key);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(akm));

			for(j=0; j<16; j++){
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(bkid[j]));
			}
			
			WlanNode->wtpWapiAuthSuiteSelected[0] = 0x0;
			WlanNode->wtpWapiAuthSuiteSelected[1] = 0x14;
			WlanNode->wtpWapiAuthSuiteSelected[2] = 0x72;
			WlanNode->wtpWapiAuthSuiteSelected[3] = akm;

			memcpy(WlanNode->wtpWapiBKIDUsed,bkid,16);

			WlanNode->wtpWapiAuthSuiteRequested[0] = 0x0;
			WlanNode->wtpWapiAuthSuiteRequested[1] = 0x14;
			WlanNode->wtpWapiAuthSuiteRequested[2] = 0x72;
			WlanNode->wtpWapiAuthSuiteRequested[3] = akm;

			WlanNode->wtpWapiConfigVersion = 1;
			WlanNode->wtpWapiWPIOptionImplement = 1;
			WlanNode->wtpWapiUnicastKeysSupported = 2;
			WlanNode->wtpWapiMulticastCipherSize = 256;
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WlanHead;
}
/*for showting ConfigWapiInfo by nl 20100521 bb4*/
struct UnicastInfo* show_UnicastInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	
	int i;
	struct UnicastInfo  *WlanNode = NULL;
	struct UnicastInfo  *WlanHead = NULL;
	struct UnicastInfo  *WlanSearchNode = NULL;
	unsigned char unicast_cipher_enabled;
	unsigned char wlanid[WLAN_NUM] = {0}; //should make 

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
								INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_UNICAST_INFO_BY_WLAN_OF_ALL_WTP);
		
	dbus_error_init(&err);
			
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,(num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if((WlanHead = (struct UnicastInfo*)malloc(sizeof(struct UnicastInfo))) == NULL){
		*ret = MALLOC_ERROR;
		dbus_message_unref(reply);
		return NULL;
		}
		memset(WlanHead,0,sizeof(struct UnicastInfo));
		WlanHead->UnicastInfo_list = NULL;
		WlanHead->UnicastInfo_last = NULL;
	
	if((*ret == 0)&&(*num !=0)){	
		for(i=0;i<*num;i++){
			if((WlanNode = (struct UnicastInfo*)malloc(sizeof(struct UnicastInfo))) == NULL){
							dcli_free_wtp_UnicastInfo(WlanHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
			}
					
			memset(WlanNode,0,sizeof(struct UnicastInfo));
			WlanNode->next = NULL;

			if(WlanHead->UnicastInfo_list == NULL){
				WlanHead->UnicastInfo_list = WlanNode;
				WlanHead->next = WlanNode;
			}
			else{
				WlanHead->UnicastInfo_last->next = WlanNode;
			}
			WlanHead->UnicastInfo_last = WlanNode;
	
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->UnicastWlanID));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityType));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(unicast_cipher_enabled));

			if(unicast_cipher_enabled == 1){
				WlanNode->NewUnicastCipherEnabled = 1;
			}
			else{
				WlanNode->NewUnicastCipherEnabled = 2;
			}

			WlanNode->NewUnicastCipherSize = 512;
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WlanHead;
}
/*for showting WtpWAPIPerformanceStatsInfo by nl 20100529 bb5*/
struct BssWAPIPerformanceStatsInfo* show_BssWAPIPerformanceStatsInfo_of_all_wlan(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_sub_struct;
	
	struct BssWAPIPerformanceStatsInfo  *WlanNode = NULL;
	struct BssWAPIPerformanceStatsInfo  *WlanHead = NULL;
	struct BssWAPIPerformanceStatsInfo  *WlanSearchNode = NULL;
	struct Sub_BssWAPIPerformanceStatsInfo * BssNode = NULL;
	
	int i, j, k;
	unsigned int wai_sign_errors = 0;
	unsigned int wai_hmac_errors = 0;
	unsigned int wai_auth_res_fail = 0;
	unsigned int wai_discard = 0;
	unsigned int wai_timeout = 0;
	unsigned int wai_format_errors = 0;
	unsigned int wai_cert_handshake_fail = 0;
	unsigned int wai_unicast_handshake_fail = 0;
	unsigned int wai_multi_handshake_fail = 0;
	unsigned int wpi_mic_errors = 0;
	unsigned int wpi_replay_counters = 0;
	unsigned int wpi_decryptable_errors = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
								INTERFACE,\
								ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_BSS_INFO_OF_ALL_WLAN);
		
	dbus_error_init(&err);
			
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,(num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if((WlanHead = (struct BssWAPIPerformanceStatsInfo*)malloc(sizeof(struct BssWAPIPerformanceStatsInfo))) == NULL){
		*ret = MALLOC_ERROR;
		dbus_message_unref(reply);
		return NULL;
		}
		memset(WlanHead,0,sizeof(struct BssWAPIPerformanceStatsInfo));
		WlanHead->BssWAPIPerformanceStatsInfo_list = NULL;
		WlanHead->BssWAPIPerformanceStatsInfo_last = NULL;
		
	if((*ret == 0)&&(*num !=0)){	
		for(i=0;i<*num;i++){
			if((WlanNode = (struct BssWAPIPerformanceStatsInfo*)malloc(sizeof(struct BssWAPIPerformanceStatsInfo))) == NULL){
							dcli_free_BssWAPIPerformanceStatsInfo(WlanHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
			}
					
			memset(WlanNode,0,sizeof(struct ConfigWapiInfo));
			WlanNode->next = NULL;

			if(WlanHead->BssWAPIPerformanceStatsInfo_list == NULL){
				WlanHead->BssWAPIPerformanceStatsInfo_list = WlanNode;
				//WlanHead->next = WlanNode;
			}
			else{
				WlanHead->BssWAPIPerformanceStatsInfo_last->next = WlanNode;
			}
			WlanHead->BssWAPIPerformanceStatsInfo_last = WlanNode;
	
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->WlanID));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->SecurityType));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WlanNode->wlan_bss_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WlanNode->wlan_bss_num; j++){  
				if((BssNode = (struct Sub_BssWAPIPerformanceStatsInfo*)malloc(sizeof(struct Sub_BssWAPIPerformanceStatsInfo))) == NULL){
										dcli_free_BssWAPIPerformanceStatsInfo(WlanHead);
										*ret = MALLOC_ERROR;
										dbus_message_unref(reply);
										return NULL;
				}
							
				memset(BssNode,0,sizeof(struct Sub_BssWAPIPerformanceStatsInfo));
				BssNode->next = NULL;
				BssNode->Sub_BssWAPIPerformanceStatsInfo_list = NULL;
				BssNode->Sub_BssWAPIPerformanceStatsInfo_last = NULL;

				if(WlanNode->Sub_BssWAPIPerformanceStatsInfo_head == NULL){
					WlanNode->Sub_BssWAPIPerformanceStatsInfo_head = BssNode;
				}
				else{
					WlanNode->Sub_BssWAPIPerformanceStatsInfo_head->Sub_BssWAPIPerformanceStatsInfo_last->next = BssNode;
					
				}
				WlanNode->Sub_BssWAPIPerformanceStatsInfo_head->Sub_BssWAPIPerformanceStatsInfo_last = BssNode;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(BssNode->bss_id));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(BssNode->wlan_id));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(BssNode->wtp_id));

				BssNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
				memset(BssNode->wtpMacAddr,0,(MAC_LEN +1));

				BssNode->wtpBssCurrID = (unsigned char *)malloc(MAC_LEN +1);
				memset(BssNode->wtpBssCurrID,0,(MAC_LEN +1));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpMacAddr[0]));							//mac 0
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpMacAddr[1]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpMacAddr[2]));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpMacAddr[3]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpMacAddr[4]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpMacAddr[5]));							//mac 5

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpBssCurrID[0]));						//bssid 0
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpBssCurrID[1]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpBssCurrID[2]));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpBssCurrID[3]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpBssCurrID[4]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(BssNode->wtpBssCurrID[5]));						//bssid 5

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_sign_errors));				//a1
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_hmac_errors));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_auth_res_fail));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_discard));					//a4
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_timeout));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_format_errors));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_cert_handshake_fail));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_unicast_handshake_fail));		//a8
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wai_multi_handshake_fail));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wpi_mic_errors));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wpi_replay_counters));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(wpi_decryptable_errors));			//a12

				BssNode->bssWapiWPIReplayCounters = wpi_replay_counters;
				BssNode->bssWapiWPIDecryptableErrors = wpi_decryptable_errors;
				BssNode->bssWapiWPIMICErrors = wpi_mic_errors;	
				BssNode->bssWapiWAISignatureErrors	= wai_sign_errors;
				
				BssNode->bssWapiWAIHMACErrors = wai_hmac_errors;
				BssNode->bssWapiWAIAuthResultFailures = wai_auth_res_fail;	
				BssNode->bssWapiWAIDiscardCounters = wai_discard;		
				BssNode->bssWapiWAITimeoutCounters = wai_timeout;	
				
				BssNode->bssWapiWAIFormatErrors = wai_format_errors;			
				BssNode->bssWapiWAICertificateHandshakeFailures = wai_cert_handshake_fail;	
				BssNode->bssWapiWAIUnicastHandshakeFailures = wai_unicast_handshake_fail;	
				BssNode->bssWapiWAIMulticastHandshakeFailures = wai_multi_handshake_fail;
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WlanHead;
}

//fengwenchao add 20101224
void  dcli_wlan_free_info_all(struct WLAN_INFO *wlannode)
{
	struct WLAN_INFO *wlannode_tmp = NULL;
	struct ifi *ifi_node = NULL;
	struct ifi *ifi_tmp = NULL;

	if(wlannode == NULL)
		return;
	if(wlannode->wlan_info_last != NULL){
		wlannode->wlan_info_last = NULL;
		}
	while(wlannode->wlan_info_list != NULL)
	{
		wlannode_tmp = wlannode->wlan_info_list;
		wlannode->wlan_info_list = wlannode_tmp->next;
		DCLI_FORMIB_FREE_OBJECT(wlannode_tmp->wlanname);
		if(wlannode_tmp->ifi_head != NULL)
		{
			ifi_node = wlannode_tmp->ifi_head;
			while(ifi_node != NULL)
			{
				ifi_tmp = ifi_node;
				ifi_node = ifi_node->ifi_next;
				//DCLI_FORMIB_FREE_OBJECT(ifi_tmp->ifi_name);
				//DCLI_FORMIB_FREE_OBJECT(ifi_tmp->nas_id);

				ifi_tmp->ifi_next = NULL;
				free(ifi_tmp);
				ifi_tmp = NULL;										
			}
		}
		wlannode_tmp->next= NULL;
		free(wlannode_tmp);
		wlannode_tmp = NULL;
	}
	free(wlannode);
	wlannode = NULL;
	return ;
}
//fengwenchao add end
void dcli_wlan_free_fun(char *DBUS_METHOD,DCLI_WLAN_API_GROUP *WLANINFO){
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_wlan_method_parse(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				for (i = 0; i < WLANINFO->wlan_num; i++) {
					CW_FREE_OBJECT(WLANINFO->WLAN[i]->WlanName);
					CW_FREE_OBJECT(WLANINFO->WLAN[i]->ESSID);
					CW_FREE_OBJECT(WLANINFO->WLAN[i]);
				}
				if(WLANINFO)
					CW_FREE_OBJECT(WLANINFO->WLAN);
				CW_FREE_OBJECT(WLANINFO);
			}
			break;
			case 2 :{
				struct ifi * phead = NULL;
				struct ifi * pnext = NULL;
				phead = WLANINFO->WLAN[0]->Wlan_Ifi;
				WLANINFO->WLAN[0]->Wlan_Ifi = NULL;
				while(phead != NULL){
					pnext = phead->ifi_next;
					CW_FREE_OBJECT(phead);
					phead = pnext;
				}
				if((WLANINFO)&&(WLANINFO->WLAN)&&(WLANINFO->WLAN[0])){
					CW_FREE_OBJECT(WLANINFO->WLAN[0]->WlanName);
					CW_FREE_OBJECT(WLANINFO->WLAN[0]->ESSID);
					CW_FREE_OBJECT(WLANINFO->WLAN[0]);
				}
				if(WLANINFO)
					CW_FREE_OBJECT(WLANINFO->WLAN);
				CW_FREE_OBJECT(WLANINFO);
			}
			break;
			case 3 :{
				if((WLANINFO)&&(WLANINFO->WLAN)){
					CW_FREE_OBJECT(WLANINFO->WLAN[0]);
					CW_FREE_OBJECT(WLANINFO->WLAN);
				}
				CW_FREE_OBJECT(WLANINFO);
			}
			break;
			case 4 :{
				if((WLANINFO)&&(WLANINFO->WLAN)){
					CW_FREE_OBJECT(WLANINFO->WLAN[0]);
					CW_FREE_OBJECT(WLANINFO->WLAN);
				}
				CW_FREE_OBJECT(WLANINFO);
			}
			break;
			case 5 :{
				struct WID_TUNNEL_WLAN_VLAN *phead = NULL;
				struct WID_TUNNEL_WLAN_VLAN *pnext = NULL;
				if((WLANINFO)&&(WLANINFO->WLAN)&&(WLANINFO->WLAN[0])){
					phead = WLANINFO->WLAN[0]->tunnel_wlan_vlan;
					WLANINFO->WLAN[0]->tunnel_wlan_vlan = NULL;
					while(phead != NULL){
						pnext = phead->ifnext;
						CW_FREE_OBJECT(phead);
						phead = pnext;
					}
					CW_FREE_OBJECT(WLANINFO->WLAN[0]);
				}
				if(WLANINFO)
					CW_FREE_OBJECT(WLANINFO->WLAN);
				CW_FREE_OBJECT(WLANINFO);
			}
			break;
			case 6 :{
			}
			break;
			case 7 :{}
			break;
			default : break;
			
		}

}
dcli_wlan_add_Wlan_Ifi_node(WID_WLAN **WLAN,struct ifi *Wlan_Ifi)
{
	if(Wlan_Ifi ==NULL)
		return -1;	
	if((*WLAN)->Wlan_Ifi == NULL){
		(*WLAN)->Wlan_Ifi = Wlan_Ifi;
		Wlan_Ifi->ifi_next = NULL;
		//printf("tmp == null.\n");

	}else{
		struct ifi *tmp = (*WLAN)->Wlan_Ifi;
		while(tmp->ifi_next != NULL){
			tmp = tmp->ifi_next;
			//printf("tmp =111= null.\n");
		}
		tmp->ifi_next = Wlan_Ifi;
		Wlan_Ifi->ifi_next = NULL;
		//printf("tmp != null.\n");
	}
	
		return 0;
}
dcli_wlan_add_tunnel_wlan_vlan_node(WID_WLAN *WLAN,struct WID_TUNNEL_WLAN_VLAN *node)
{
	if(node == NULL)
		return -1;

	if(WLAN->tunnel_wlan_vlan == NULL){
		WLAN->tunnel_wlan_vlan = node;
		node->ifnext = NULL;
	}else{
		struct WID_TUNNEL_WLAN_VLAN *tmp = WLAN->tunnel_wlan_vlan;
		while(tmp->ifnext != NULL){
			tmp = tmp->ifnext;
		}
		tmp->ifnext = node;
		node->ifnext = NULL;
		}	
		//printf("111111111111111111\n");
		return 0;
}

struct SSIDConfigInfo * show_SSIDConfig_info_of_all_wlan(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_wlan;
	DBusMessageIter  iter_wlan_essid_array;
	DBusMessageIter  iter_wlan_essid_struct;
	struct SSIDConfigInfo  *WlanNode = NULL;
	struct SSIDConfigInfo  *WlanHead = NULL;
	struct SSIDConfigInfo  *WlanTail = NULL;
	int i;
	int j = 0;
	int wlan_num=0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		*ret=0 ;
		printf("%s dbus_error  \n",__func__);
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&wlan_num);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	char *NewSSIDName = NULL;
	NewSSIDName =(char*) malloc(ESSID_DEFAULT_LEN+1);
	memset(NewSSIDName,0,ESSID_DEFAULT_LEN+1);

	*num=wlan_num;	
	for(i=0;i<wlan_num;i++){
		if((WlanNode = (struct SSIDConfigInfo*)malloc(sizeof(struct SSIDConfigInfo))) == NULL){
				dcli_free_SSIDConfigInfo_head(WlanHead);
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WlanNode,0,sizeof(struct SSIDConfigInfo));
		if(WlanHead == NULL){
			WlanHead = WlanNode;
			WlanTail = WlanNode;
		}else{
			WlanTail->next=WlanNode;
			WlanTail=WlanNode;
		}
		dbus_message_iter_recurse(&iter_array,&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->wlanCurrID));/*1*/
		dbus_message_iter_next(&iter_wlan);

		dbus_message_iter_recurse(&iter_wlan,&iter_wlan_essid_array);
		
		//NewSSIDName =(char*)malloc(ESSID_DEFAULT_LEN+1);
		memset(NewSSIDName,0,ESSID_DEFAULT_LEN+1);	
		
		for(j=0;j<ESSID_DEFAULT_LEN;j++){
			dbus_message_iter_recurse(&iter_wlan_essid_array,&iter_wlan_essid_struct);
			dbus_message_iter_get_basic(&iter_wlan_essid_struct,&NewSSIDName[j]);
			dbus_message_iter_next(&iter_wlan_essid_struct);
			dbus_message_iter_next(&iter_wlan_essid_array);
		}		
		//dbus_message_iter_get_basic(&iter_wlan,&NewSSIDName);
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewSSIDEnabled));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewSSIDHidden));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewStaIsolate));/*5*/
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewDot11Auth));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->Newsecurity));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewAuthenMode));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewSecurityCiphers));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewVlanId));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewMaxSimultUsers));/*10*/
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewStaUplinkMaxRate));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->NewStaDwlinkMaxRate));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->SecurityID));
		dbus_message_iter_next(&iter_wlan);
		dbus_message_iter_get_basic(&iter_wlan,&(WlanNode->authentication_aged));

		//printf("NewStaUplinkMaxRate: %d\n",WlanNode->NewStaUplinkMaxRate);
		//printf("NewStaDwlinkMaxRate: %d\n",WlanNode->NewStaDwlinkMaxRate);
		
		if(!(WlanNode->NewSSIDName=(char *)malloc(ESSID_DEFAULT_LEN+1))){
			printf("In function %s : malloc(strlen(NewSSIDName)+1) failed!\n",__func__);
			dcli_free_SSIDConfigInfo_head(WlanHead);
			dbus_message_unref(reply);
			CW_FREE_OBJECT(NewSSIDName);
			return NULL;
		}
		memset(WlanNode->NewSSIDName,0,ESSID_DEFAULT_LEN+1);
		memcpy(WlanNode->NewSSIDName,NewSSIDName,ESSID_DEFAULT_LEN);
		//CW_FREE_OBJECT(NewSSIDName);
		WlanNode->SSIDRowStatus=1;
		dbus_message_iter_next(&iter_array);
		}
	dbus_message_unref(reply);
	CW_FREE_OBJECT(NewSSIDName);

	//ASD operation 
	DBusMessageIter  iter_security;
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);         //fengwenchao modify 20110513
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);    //fengwenchao modify 20110513
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);    //fengwenchao modify 20110513
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
								ASD_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		*ret=0 ;
		printf("%s dbus_error  \n",__func__);
		dcli_free_SSIDConfigInfo_head(WlanHead);
		return NULL;
	}


	unsigned int security_num=0;
	unsigned int security_id=0;
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&security_num);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	char *NewSecurityKEY;
	char *NewAuthSharedSecret;
	char *NewAcctSharedSecret;
	long NewEncrInputType;
	long NewExtensibleAuth;
	long NewAuthPort;
	long NewAcctPort;
	char *NewAuthIP;
	char *NewAcctIP;
	char* STRING_NONE = "none";
	char *String_zero = "0";
	char *asip;
	long cert_type;
	char *as_path;
	char *ae_path;
	long sta_aged_time;
	//printf("security_num : %d\n",security_num);//weichao test
	for(i=0;i<security_num;i++){

		dbus_message_iter_recurse(&iter_array,&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewEncrInputType);/*1*/
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewSecurityKEY);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewExtensibleAuth);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewAuthIP);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewAuthPort);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewAuthSharedSecret);/*5*/
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewAcctIP);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewAcctPort);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&NewAcctSharedSecret);

		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&security_id);

		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&asip);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&as_path);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&ae_path);
		
		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&cert_type);

		dbus_message_iter_next(&iter_security);
		dbus_message_iter_get_basic(&iter_security,&sta_aged_time);
		//printf("NewSecurityKEY = %s\n",NewSecurityKEY);
		//printf("NewAuthSharedSecret = %s\n",NewAuthSharedSecret);
		//printf("NewAcctSharedSecret = %s\n",NewAcctSharedSecret);
		WlanNode = WlanHead;
		
		while(WlanNode){
			
			if(WlanNode->SecurityID == security_id){
				//printf("111xxxxxxxx----\n");
				WlanNode->NewEncrInputType = NewEncrInputType;
				WlanNode->NewExtensibleAuth = NewExtensibleAuth;
				WlanNode->NewAuthPort= NewAuthPort;					/*xiaodawei modify, 20101116*/
				//WlanNode->NewAuthIP = NewAuthIP;					/*xiaodawei modify, 20101116*/
				//WlanNode->NewAcctIP = NewAcctIP;
				WlanNode->NewAcctPort = NewAcctPort;
				WlanNode->cert_type = cert_type;
				WlanNode->sta_aged = sta_aged_time;

				if(!(WlanNode->NewSecurityKEY=(char *)malloc(strlen(NewSecurityKEY)+1))){
					printf("In function %s : malloc(strlen(NewSecurityKEY)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
					}
				memset(WlanNode->NewSecurityKEY,0,strlen(NewSecurityKEY)+1);
				memcpy(WlanNode->NewSecurityKEY,NewSecurityKEY,strlen(NewSecurityKEY)); 	
				/*weichao add 20110805*/
				if(!(WlanNode->NewAuthIP=(char *)malloc(strlen(NewAuthIP)+1))){
					printf("In function %s : malloc(strlen(NewAuthIP)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
					}
				memset(WlanNode->NewAuthIP,0,strlen(NewAuthIP)+1);
				memcpy(WlanNode->NewAuthIP,NewAuthIP,strlen(NewAuthIP));
				
				if(!(WlanNode->NewAcctIP=(char *)malloc(strlen(NewAcctIP)+1))){
					printf("In function %s : malloc(strlen(NewAcctIP)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
					}
				memset(WlanNode->NewAcctIP,0,strlen(NewAcctIP)+1);
				memcpy(WlanNode->NewAcctIP,NewAcctIP,strlen(NewAcctIP));
					
				if(!(WlanNode->NewAuthSharedSecret=(char *)malloc(strlen(NewAuthSharedSecret)+1))){
					printf("In function %s : malloc(strlen(NewAuthSharedSecret)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
					}
				memset(WlanNode->NewAuthSharedSecret,0,strlen(NewAuthSharedSecret)+1);
				memcpy(WlanNode->NewAuthSharedSecret,NewAuthSharedSecret,strlen(NewAuthSharedSecret));
				
				
				if(!(WlanNode->NewAcctSharedSecret=(char *)malloc(strlen(NewAcctSharedSecret)+1))){
					printf("In function %s : malloc(strlen(NewAcctSharedSecret)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
					}
				memset(WlanNode->NewAcctSharedSecret,0,strlen(NewAcctSharedSecret)+1);
				memcpy(WlanNode->NewAcctSharedSecret,NewAcctSharedSecret,strlen(NewAcctSharedSecret));

				if(!(WlanNode->asip =(char *)malloc(strlen(asip)+1))){
					printf("In function %s : malloc(strlen(asip)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
				}
				memset(WlanNode->asip,0,strlen(asip)+1);
				memcpy(WlanNode->asip,asip,strlen(asip));

				if(!(WlanNode->as_path = (char *)malloc(strlen(as_path)+1))){
					printf("In function %s : malloc(strlen(as_path)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
				}
				memset(WlanNode->as_path,0,strlen(as_path)+1);
				memcpy(WlanNode->as_path,as_path,strlen(as_path));

				if(!(WlanNode->ae_path = (char *)malloc(strlen(ae_path)+1))){
					printf("In function %s : malloc(strlen(ae_path)+1) failed!\n",__func__);
					dcli_free_SSIDConfigInfo_head(WlanHead);
					dbus_message_unref(reply);
					return NULL;
				}
				memset(WlanNode->ae_path,0,strlen(ae_path)+1);
				memcpy(WlanNode->ae_path,ae_path,strlen(ae_path));
			}
	
		WlanNode = WlanNode->next;
		}
		dbus_message_iter_next(&iter_array);
	}
	dbus_message_unref(reply);
	
	return WlanHead;
}

//fengwenchao add 20101224
struct WLAN_INFO *show_wlan_of_all(DBusConnection *dcli_dbus_connection,int index,int localid,int *ret,int *wlan_num)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;	
	dbus_error_init(&err);

	int i,j =0;
	struct WLAN_INFO *wlanhead = NULL;
	struct WLAN_INFO *wlannode = NULL;
	struct WLAN_INFO *asdnode = NULL;	
	struct ifi *ifinode = NULL;

	char *name = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);


	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOWWLAN_OF_ALL);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		dbus_error_free_for_dcli(&err);

		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wlan_num);
	//printf("ret = %d \n",*ret);
	//printf("wlan_num = %d \n",*wlan_num);
	if(*ret == 0)
	{
		if((wlanhead = (struct WLAN_INFO*)malloc(sizeof(struct WLAN_INFO))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(wlanhead,0,sizeof(struct WLAN_INFO));
		wlanhead->wlan_info_list= NULL;
		wlanhead->wlan_info_last = NULL;
		wlanhead->ifi_head= NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);	

		for(i=0;i< *wlan_num;i++)
		{
			DBusMessageIter iter_struct;
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;
		    DBusMessageIter iter_sub_sub_array;
			DBusMessageIter iter_sub_sub_struct;

			if((wlannode = (struct WLAN_INFO*)malloc(sizeof(struct WLAN_INFO))) == NULL){
			dcli_wlan_free_info_all(wlanhead);
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
			}
			
			memset(wlannode,0,sizeof(struct WLAN_INFO));
			wlannode->next = NULL;
			wlannode->ifi_head= NULL;

			if(wlanhead->wlan_info_list== NULL){
				wlanhead->wlan_info_list = wlannode;
				wlanhead->next = wlannode;
			}
			else{
				wlanhead->wlan_info_last->next = wlannode;
			}
			wlanhead->wlan_info_last = wlannode;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->Wlanid));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanLoadBalanceUsersDiffThreshhd));

			//printf("wlannode->wlanLoadBalanceUsersDiffThreshhd = %d \n",wlannode->wlanLoadBalanceUsersDiffThreshhd);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanLoadBalanceTrafficDiffThreshhd));

			//printf("wlannode->wlanLoadBalanceTrafficDiffThreshhd = %d \n",wlannode->wlanLoadBalanceTrafficDiffThreshhd);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanLoadBalanceFunction));		

			//printf("wlannode->wlanLoadBalanceFunction = %d \n",wlannode->wlanLoadBalanceFunction);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanLoadBalanceStatus));

			//printf("wlannode->wlanLoadBalanceStatus = %d \n",wlannode->wlanLoadBalanceStatus);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanMaxConnectUsr));

			//printf("wlannode->wlanMaxConnectUsr = %d \n",wlannode->wlanMaxConnectUsr);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanServiceEnable));

			//printf("wlannode->wlanServiceEnable= %d \n",wlannode->wlanServiceEnable);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanBindSecurity));

			//printf("wlannode->wlanBindSecurity = %d \n",wlannode->wlanBindSecurity);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanBindSecType));

			//printf("wlannode->wlanBindSecType = %d \n",wlannode->wlanBindSecType);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanBindEncryType));

			//printf("wlannode->wlanBindEncryType = %d \n",wlannode->wlanBindEncryType);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->wlanHideEssid));

			//printf("wlannode->wlanHideEssid = %d \n",wlannode->wlanHideEssid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->bifnum));	

			//printf("wlannode->bifnum = %d \n",wlannode->bifnum);
			/*fengwenchao add 20110401*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->ethernetRecvCorrectFrames));
		
			//printf("wlannode->ethernetSendCorrectBytes = %d \n",wlannode->ethernetRecvCorrectFrames);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->ethernetRecvCorrectBytes));
			
			//printf("wlannode->ethernetSendCorrectBytes = %llu \n",wlannode->ethernetRecvCorrectBytes);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->ethernetSendCorrectBytes));
			
			//printf("wlannode->ethernetSendCorrectBytes = %llu \n",wlannode->ethernetSendCorrectBytes);
			/*fengwenchao add end*/
 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);			

			for(j=0;j<wlannode->bifnum;j++)
			{                
				   if((ifinode = (struct ifi*)malloc(sizeof(struct ifi))) == NULL){
					dcli_wlan_free_info_all(wlanhead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				 }
				   memset(ifinode,0,sizeof(struct ifi));
				   ifinode->ifi_next= NULL;
				   ifinode->ifi_list= NULL;
				   ifinode->ifi_last= NULL;

				  //ifinode->ifi_name= (unsigned char *)malloc(ETH_IF_NAME_LEN +1);
				 // memset(ifinode->ifi_name,0,(ETH_IF_NAME_LEN +1));

				   
                  if(wlannode->ifi_head== NULL){
					   wlannode->ifi_head = ifinode;
				    }
				    else{
					wlannode->ifi_head->ifi_last->ifi_next= ifinode;
				   }
				
				 wlannode->ifi_head->ifi_last = ifinode;
				 dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(name));

				memcpy(ifinode->ifi_name,name,ETH_IF_NAME_LEN);

				//printf("ifinode->ifi_name = %s \n",ifinode->ifi_name);
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		
		}
	}
	else {
		dbus_message_unref(reply);
		return NULL;
	}
	dbus_message_unref(reply);
	//printf("ASD!!!!!!!\n");
	/*Receive ASD information*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	unsigned int ret2;
	int num = 0;
	//num = *wlan_num;
//	printf("num = %d \n",num);

	unsigned char asd_wlanid = 0;
	int bss_num = 0;
	int bss_sta_num = 0;
	int wlan_sta_num = 0;
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);             //fengwenchao modify 20110513
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);      //fengwenchao modify 20110513
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE); //fengwenchao modify 20110513
  
	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALLWLAN_STA_NUM);
	
	dbus_error_init(&err2);
	/*dbus_message_append_args(query2,
							 DBUS_TYPE_UINT32,&num,
							 DBUS_TYPE_INVALID);*/
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_wlan_free_info_all(wlanhead);
		return NULL;
	}

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	dbus_message_iter_next(&iter2);
	dbus_message_iter_get_basic(&iter2,&num);

	if((ret2 == 0)&&(num != 0))
	{
		dbus_message_iter_next(&iter2);	
	    dbus_message_iter_recurse(&iter2,&iter_array2);

		for(i=0;i<num;i++)
		{
			DBusMessageIter iter_struct;
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;

			dbus_message_iter_recurse(&iter_array2,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(asd_wlanid));

			//printf("asd_wlanid  =  %d\n",asd_wlanid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss_num));

			//printf("bss_num  =  %d\n",bss_num);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			wlan_sta_num = 0;
			for(j=0;j<bss_num;j++)
			{
				 dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(bss_sta_num));

				 wlan_sta_num +=bss_sta_num;
				 dbus_message_iter_next(&iter_sub_array);
			}
			//printf("wlan_sta_num = %d\n",wlan_sta_num);

			asdnode = wlanhead->wlan_info_list;
			while(asdnode != NULL)
			{
				if(asdnode->Wlanid == asd_wlanid)
				{
					asdnode->wlanStaOnlineNum = wlan_sta_num;
					asdnode->wlanUsrWirelessResoUseRate = ( float)((float)wlan_sta_num*100/(float)(asdnode->wlanMaxConnectUsr));
				//	printf("asdnode->wlanUsrWirelessResoUseRate = %f \n",asdnode->wlanUsrWirelessResoUseRate);
					
				}
				asdnode = asdnode->next;
			}
			 dbus_message_iter_next(&iter_array2);
		}
		dbus_message_unref(reply2);
	}
	return wlanhead;
}

//fengwenchao add 20101224
void *dcli_wlan_show_api_group(
	int index,
	unsigned char wlanid,
	unsigned int id,
	unsigned int* ret,
	unsigned char *num4,
	//DCLI_WTP_API_GROUP_ONE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int num;
	unsigned char num_char;
	int i = 0;	
	unsigned int dcli_sn = 0;
	int localid  = id;  //fengwenchao add 20110507
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	//int wlanid = id1;
	
	DCLI_WLAN_API_GROUP * LIST = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
//	ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
//	ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	dcli_sn = dcli_wlan_method_parse(DBUS_METHOD);
	switch(dcli_sn){
		case 1 :{
			ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		}
		break;
		case 2 :{
			ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_message_append_args(query,
									 DBUS_TYPE_BYTE,&wlanid,
									 DBUS_TYPE_INVALID);
		}
		break;
		case 3 :{
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_message_append_args(query,
									 DBUS_TYPE_BYTE,&wlanid,
									 DBUS_TYPE_INVALID);
		}
		break;
		case 4 :{
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_message_append_args(query,
									 DBUS_TYPE_BYTE,&wlanid,
									 DBUS_TYPE_INVALID);
		}
		break;
		case 5 :{
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_message_append_args(query,
									 DBUS_TYPE_BYTE,&wlanid,
									 DBUS_TYPE_INVALID);
			
		}
		break;
		default :{		
			if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
			}
			*ret = -1;
			return NULL;
		}
			break;
	}
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	//printf("ccccccccc  ret is %d \n",*ret);
	//printf("ccccccccc  dcli_sn is %d \n",dcli_sn);
	if(*ret == 0 )
	{	
		
		CW_CREATE_OBJECT_ERR(LIST, DCLI_WLAN_API_GROUP, return NULL;); 
		LIST->WLAN = NULL;
		LIST->enable_num = 0;
		LIST->wlan_num = 0;
		switch(dcli_sn){
		case 1 :{
				int j =0;
				unsigned char* essid = NULL;
				essid =(unsigned char*) malloc(ESSID_DEFAULT_LEN+1);
				memset(essid,0,ESSID_DEFAULT_LEN+1);
				//	DCLI_WLAN_API_GROUP *WLANIFO = NULL;
				//	WLANIFO->WLAN = NULL;
				//LIST->WLAN = (WID_WLAN*)malloc(sizeof(WID_WLAN*));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wlan_num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
				LIST->WLAN = (WID_WLAN*)malloc((LIST->wlan_num)*sizeof(WID_WLAN*));
				for (i = 0; i < LIST->wlan_num; i++) {
					DBusMessageIter iter_struct;
					DBusMessageIter iter_sub_array;
					char* wlanname = NULL;
					LIST->WLAN[i] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(wlanname));

						LIST->WLAN[i]->WlanName = (char*)malloc(WID_DEFAULT_NUM+1);
						memset(LIST->WLAN[i]->WlanName,0,WID_DEFAULT_NUM+1);
						memcpy(LIST->WLAN[i]->WlanName,wlanname,WID_DEFAULT_NUM);
				
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->WlanID));
				
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
					j = 0;
					for(j=0;j<ESSID_DEFAULT_LEN;j++){
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
						dbus_message_iter_get_basic(&iter_sub_struct,&essid[j]);
						dbus_message_iter_next(&iter_sub_struct);
						dbus_message_iter_next(&iter_sub_array);
					}
						LIST->WLAN[i]->ESSID= (char*)malloc(ESSID_DEFAULT_LEN+1);
						memset(LIST->WLAN[i]->ESSID,0,ESSID_DEFAULT_LEN+1);
						memcpy(LIST->WLAN[i]->ESSID,essid,strlen((char *)essid));
				
					dbus_message_iter_next(&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->Status));
							
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->SecurityID));
					
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->HideESSid));

					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->wlan_if_policy));

					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->WDSStat));
					
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->WLAN[i]->wds_mesh));
					if(LIST->WLAN[i]->Status == 0)
					{
						LIST->enable_num++;
					}
					dbus_message_iter_next(&iter_array);
				}
				if(essid){
					free(essid);
				}
		}
		break;
		case 2 :{
				DBusMessageIter iter_struct;
				LIST->WLAN = (WID_WLAN*)malloc(sizeof(WID_WLAN*));
				LIST->WLAN[0] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
				LIST->WLAN[0]->Wlan_Ifi = NULL;
				char* wlanname = NULL;
				char* essid = NULL;
				essid =(char*) malloc(ESSID_DEFAULT_LEN+1);
				memset(essid,0,ESSID_DEFAULT_LEN+1);
				char *eap_mac = NULL;// = (char*)malloc(18);
				char* key2 = NULL;
				int j;
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->balance_para));/*xm*/

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->flow_balance_para));/*xm*/

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->balance_switch));/*xm*/

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->balance_method));/*xm*/
				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wlan_max_allowed_sta_num));
				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&wlanname);/*(LIST->WLAN[0]->WlanName)*/
				LIST->WLAN[0]->WlanName = (char*)malloc(WID_DEFAULT_NUM+1);
				memset(LIST->WLAN[0]->WlanName,0,WID_DEFAULT_NUM+1);
				memcpy(LIST->WLAN[0]->WlanName,wlanname,WID_DEFAULT_NUM);
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->WlanID));
				
				dbus_message_iter_next(&iter);		
				dbus_message_iter_recurse(&iter,&iter_array);
				for(j=0;j<ESSID_DEFAULT_LEN;j++){
					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&essid[j]);
					dbus_message_iter_next(&iter_array);
				}

					LIST->WLAN[0]->ESSID= (char*)malloc(ESSID_DEFAULT_LEN+1);
					memset(LIST->WLAN[0]->ESSID,0,ESSID_DEFAULT_LEN+1);
					memcpy(LIST->WLAN[0]->ESSID,essid,strlen((char *)essid));
					if(essid != NULL){
						free(essid);
						essid = NULL;
					}
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->Status));

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter, &(LIST->WLAN[0]->isolation_policy));
					
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter, &(LIST->WLAN[0]->multicast_isolation_policy));
					
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter, &(LIST->WLAN[0]->sameportswitch));
					
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter, &(LIST->WLAN[0]->bridge_ucast_solicit_stat));
					
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter, &(LIST->WLAN[0]->bridge_mcast_solicit_stat));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->SecurityID));
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->SecurityType));
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->EncryptionType));
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(key2));
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->asic_hex));
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->HideESSid));
				
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->SecurityIndex));
				
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wlan_if_policy));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->WLAN_TUNNEL_POLICY));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->Roaming_Policy));
				
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wlan_traffic_limit));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->flow_check));
				
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->no_flow_time));
				
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->limit_flow));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wlan_send_traffic_limit));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wlan_station_average_traffic_limit));
								
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wlan_station_average_send_traffic_limit));

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->WDSStat));
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->wds_mesh));
                /* book add for eap mac show,2011-11-15 */
				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->eap_mac_switch));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(eap_mac));
				LIST->WLAN[0]->eap_mac = (char*)malloc(strlen(eap_mac)+1);
				memset(LIST->WLAN[0]->eap_mac,0,strlen(eap_mac)+1);
				memcpy(LIST->WLAN[0]->eap_mac,eap_mac,strlen(eap_mac));
								
				/* book add end */
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num_char);

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&(LIST->WLAN[0]->hotspot_id));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				memcpy(LIST->WLAN[0]->WlanKey,key2,DEFAULT_LEN);
				int i = 0;
				struct ifi *Wlan_Ifi = NULL;
				char *name = NULL;
				char *nasid = NULL;
				//name = (char*)malloc(ETH_IF_NAME_LEN+1);
				for (i = 0; i < num_char; i++){
					Wlan_Ifi = (struct ifi*)malloc(sizeof(struct ifi));
					//DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&name);
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&nasid);
					
					dbus_message_iter_next(&iter_array);
					
					memcpy(Wlan_Ifi->ifi_name,name,ETH_IF_NAME_LEN);
					memcpy(Wlan_Ifi->nas_id,nasid,NAS_IDENTIFIER_NAME);
					dcli_wlan_add_Wlan_Ifi_node(&(LIST->WLAN[0]),Wlan_Ifi);
				}
		}
		break;
		case 3 :{
				LIST->WLAN = (WID_WLAN*)malloc(sizeof(WID_WLAN*));
				LIST->WLAN[0] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->WLAN[0]->vlanid);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->WLAN[0]->wlan_1p_priority);
		}
		break;
		case 4 :{
				LIST->WLAN = (WID_WLAN*)malloc(sizeof(WID_WLAN*));
				LIST->WLAN[0] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&LIST->WLAN[0]->isolation_policy);

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&LIST->WLAN[0]->multicast_isolation_policy);

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&LIST->WLAN[0]->sameportswitch);
		}
		break;
		case 5 :{
				LIST->WLAN = (WID_WLAN*)malloc(sizeof(WID_WLAN*));
				LIST->WLAN[0] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
				LIST->WLAN[0]->tunnel_wlan_vlan = NULL;
				//LIST->WLAN[0]->tunnel_wlan_vlan = (struct WID_TUNNEL_WLAN_VLAN *)malloc(sizeof(struct WID_TUNNEL_WLAN_VLAN));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&num);
				//printf("case 5:num is %d \n",num);
				if(num != 0)
				{	
					char* name = NULL;
				    //printf("name address %p\n",name);
					struct WID_TUNNEL_WLAN_VLAN *node = NULL;
					for(i=0;i<num;i++)
					{	
						node = (struct WID_TUNNEL_WLAN_VLAN*)malloc(sizeof(struct WID_TUNNEL_WLAN_VLAN));
						//name = (char*)malloc(ETH_IF_NAME_LEN+1);
						dbus_message_iter_next(&iter);	
						dbus_message_iter_get_basic(&iter,&name);
						memcpy(node->ifname,name,ETH_IF_NAME_LEN);
					    //printf("case 5:ifname is %s \n",node->ifname);
						dcli_wlan_add_tunnel_wlan_vlan_node(LIST->WLAN[0],node);
					}
				}
		}
		break;
		case 6 :{}
		break;
		case 7 :{}
		break;
		case 8 :{}
		break;
		default : break;
		}
	}
	dbus_message_unref(reply);
	return LIST;

}

int dcli_wlan_sta_ip_mac_binding(int index,int localid,int policy,unsigned char wlanid,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_STA_IP_MAC_BINDING);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wlanid);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

int dcli_wlan_service_control_timer(int index,int localid,int policy,unsigned char wlanid,int is_once,int wday,int time,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SERVICE_CONTROL_TIMER);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wlanid);

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &is_once);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &time);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wday);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
int dcli_wlan_timer_able(int index,int localid,int policy,int timer,unsigned char wlanid,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_TIMER_ABLE);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &timer);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wlanid);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
void *create_wlan_CN(int index,int localid,int policy,unsigned char wlan_id,unsigned char *name,unsigned char *ESSID,DBusConnection *dcli_dbus_connection)
{
	int ret,len;
	unsigned char isAdd;	
	int lenth = 32;
	int i;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	isAdd = 1;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_DEL_WLAN_CN);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isAdd,								
						DBUS_TYPE_BYTE,&wlan_id,
						DBUS_TYPE_UINT32,&lenth,
						
						DBUS_TYPE_STRING,&name,
						//DBUS_TYPE_STRING,&essid_cn_str,

						DBUS_TYPE_BYTE,&ESSID[0],
						DBUS_TYPE_BYTE,&ESSID[1],
						DBUS_TYPE_BYTE,&ESSID[2],
						DBUS_TYPE_BYTE,&ESSID[3],
						DBUS_TYPE_BYTE,&ESSID[4],
						
						DBUS_TYPE_BYTE,&ESSID[5],
						DBUS_TYPE_BYTE,&ESSID[6],
						DBUS_TYPE_BYTE,&ESSID[7],
						DBUS_TYPE_BYTE,&ESSID[8],
						DBUS_TYPE_BYTE,&ESSID[9],
						
						DBUS_TYPE_BYTE,&ESSID[10],
						DBUS_TYPE_BYTE,&ESSID[11],
						DBUS_TYPE_BYTE,&ESSID[12],
						DBUS_TYPE_BYTE,&ESSID[13],
						DBUS_TYPE_BYTE,&ESSID[14],
						
						DBUS_TYPE_BYTE,&ESSID[15],
						DBUS_TYPE_BYTE,&ESSID[16],
						DBUS_TYPE_BYTE,&ESSID[17],
						DBUS_TYPE_BYTE,&ESSID[18],
						DBUS_TYPE_BYTE,&ESSID[19],
						
						DBUS_TYPE_BYTE,&ESSID[20],
						DBUS_TYPE_BYTE,&ESSID[21],
						DBUS_TYPE_BYTE,&ESSID[22],
						DBUS_TYPE_BYTE,&ESSID[23],
						DBUS_TYPE_BYTE,&ESSID[24],
						
						DBUS_TYPE_BYTE,&ESSID[25],
						DBUS_TYPE_BYTE,&ESSID[26],
						DBUS_TYPE_BYTE,&ESSID[27],
						DBUS_TYPE_BYTE,&ESSID[28],
						DBUS_TYPE_BYTE,&ESSID[29],
						
						DBUS_TYPE_BYTE,&ESSID[30],
						DBUS_TYPE_BYTE,&ESSID[31],

						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);

	return ret;	
}
#endif
