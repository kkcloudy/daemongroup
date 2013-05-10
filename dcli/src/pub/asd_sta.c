#ifdef _D_WCPSS_

#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "wid_ac.h"
#include "asd_sta.h"
#include "wcpss/asd/asd.h"
#include "time.h"//qiuchen add it
typedef enum {
	EAP_TYPE_NONE = 0,
	EAP_TYPE_IDENTITY = 1 /* RFC 3748 */,
	EAP_TYPE_NOTIFICATION = 2 /* RFC 3748 */,
	EAP_TYPE_NAK = 3 /* Response only, RFC 3748 */,
	EAP_TYPE_MD5 = 4, /* RFC 3748 */
	EAP_TYPE_OTP = 5 /* RFC 3748 */,
	EAP_TYPE_GTC = 6, /* RFC 3748 */
	EAP_TYPE_TLS = 13 /* RFC 2716 */,
	EAP_TYPE_LEAP = 17 /* Cisco proprietary */,
	EAP_TYPE_SIM = 18 /* RFC 4186 */,
	EAP_TYPE_TTLS = 21 /* draft-ietf-pppext-eap-ttls-02.txt */,
	EAP_TYPE_AKA = 23 /* RFC 4187 */,
	EAP_TYPE_PEAP = 25 /* draft-josefsson-pppext-eap-tls-eap-06.txt */,
	EAP_TYPE_MSCHAPV2 = 26 /* draft-kamath-pppext-eap-mschapv2-00.txt */,
	EAP_TYPE_TLV = 33 /* draft-josefsson-pppext-eap-tls-eap-07.txt */,
	EAP_TYPE_TNC = 38 /* TNC IF-T v1.0-r3; note: tentative assignment;
			   * type 38 has previously been allocated for
			   * EAP-HTTP Digest, (funk.com) */,
	EAP_TYPE_FAST = 43 /* RFC 4851 */,
	EAP_TYPE_PAX = 46 /* RFC 4746 */,
	EAP_TYPE_PSK = 47 /* RFC 4764 */,
	EAP_TYPE_SAKE = 48 /* RFC 4763 */,
	EAP_TYPE_IKEV2 = 49 /* RFC 5106 */,
	EAP_TYPE_EXPANDED = 254 /* RFC 3748 */,
	EAP_TYPE_GPSK = 255 /* EXPERIMENTAL - type not yet allocated
			     * draft-ietf-emu-eap-gpsk-01.txt */
} EapType;

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

int parse2_int_ID(char* str,unsigned int* ID)
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
char *eap_type_text(unsigned char type)
{
	switch (type) {
		case EAP_TYPE_IDENTITY: return "Identity";
		case EAP_TYPE_NOTIFICATION: return "Notification";
		case EAP_TYPE_NAK: return "Nak";
		case EAP_TYPE_MD5: return "MD5-Challenge";
		case EAP_TYPE_OTP: return "One-Time Password";
		case EAP_TYPE_GTC: return "Generic Token Card";
		case EAP_TYPE_TLS: return "TLS";
		case EAP_TYPE_TTLS: return "TTLS";
		case EAP_TYPE_PEAP: return "PEAP";
		case EAP_TYPE_SIM: return "SIM";
		case EAP_TYPE_FAST: return "FAST";
		case EAP_TYPE_SAKE: return "SAKE";
		case EAP_TYPE_PSK: return "PSK";
		case EAP_TYPE_PAX: return "PAX";
		default: return "Unknown";
	}
}

void dcli_free_maclist(struct maclist *acl)
{
	struct maclist *tmp = NULL;
	if(acl == NULL)
		return;
	while(acl != NULL) {
		tmp = acl;
		acl = tmp->next;
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	return;
	
}


void dcli_free_sta(struct dcli_sta_info *sta)
{
	if(sta == NULL)
		return;
	if(sta->ip != NULL) {
		free(sta->ip);
		sta->ip = NULL;
	}
	free(sta);
	sta = NULL;
	return;
	
}

void dcli_free_sta_v2(struct dcli_sta_info_v2 *sta)
{
	if(sta == NULL)
		return;
	if(sta->essid!= NULL) {
		free(sta->essid);
		sta->essid = NULL;
	}
	free(sta);
	sta = NULL;
	return;
	
}
void dcli_free_base_sta_info(struct dcli_sta_base_info *sta)
{
	if(sta == NULL)
		return;
	if(sta->essid!= NULL) {
		free(sta->essid);
		sta->essid = NULL;
	}
	if(sta->ip != NULL) {
		free(sta->ip);
		sta->ip = NULL;
	}
	free(sta);
	sta = NULL;
	return;
}


void dcli_free_bss(struct dcli_bss_info *bss)
{
	struct dcli_sta_info *tmp = NULL;

	if(bss == NULL)
		return ;
	
	if(bss->sta_last != NULL) {
		bss->sta_last = NULL;
	}
	while(bss->sta_list != NULL) {
		tmp = bss->sta_list;
		bss->sta_list = tmp->next;
		tmp->next = NULL;
		dcli_free_sta(tmp);
	}
	free(bss);
	bss = NULL;
	return ;
	
}

void dcli_free_base_bss(struct dcli_base_bss_info *bss)
{
	struct dcli_sta_base_info *tmp = NULL;
	struct dcli_base_bss_info  *bssnode = NULL;
	struct dcli_base_bss_info  *bsstmp = NULL;

	if(bss == NULL)
		return ;
	bssnode = bss;

	while(bssnode != NULL){
		bsstmp = bssnode;
		if(bsstmp->sta_last != NULL) {
			bsstmp->sta_last = NULL;
		}
		while(bsstmp->sta_list != NULL) {
			tmp = bsstmp->sta_list;
			bsstmp->sta_list = tmp->next;
			tmp->next = NULL;
			dcli_free_base_sta_info(tmp);
		}
		
		bssnode = bssnode->next;
		free(bsstmp);
		bsstmp= NULL;
	}
	return ;
}

void dcli_free_radio(struct dcli_radio_info *radio)
{

	if(radio == NULL)
		return ;
	
	free(radio);
	radio = NULL;
	return ;
	
}

void dcli_free_wlanlist(struct dcli_wlan_info *wlan)  //fengwenchao add 20110113
{
	struct dcli_wlan_info *wlan_tmp = NULL;
	if(wlan == NULL)
		return;
	while(wlan != NULL)
	{
		wlan_tmp = wlan;
		wlan = wlan->next;
		wlan_tmp->next = NULL;
		dcli_free_wlan(wlan_tmp);
	}
	return;
}
void dcli_free_wlan(struct dcli_wlan_info *wlan)
{
	struct dcli_bss_info *tmp = NULL;

	if(wlan == NULL)
		return ;
	
	if(wlan->bss_last != NULL) {
		wlan->bss_last = NULL;
	}
	while(wlan->bss_list != NULL) {
		tmp = wlan->bss_list;
		wlan->bss_list = tmp->next;
		tmp->next = NULL;
		dcli_free_bss(tmp);
	}
	free(wlan);
	wlan = NULL;
	return ;
	
}

void dcli_free_allwlan(struct dcli_wlan_info *wlan)  //fengwenchao add 20101221
{
	struct dcli_wlan_info *wlan_tmp = NULL;
	struct dcli_bss_info *tmp = NULL;

	if(wlan == NULL)
		return;
	if(wlan->wlan_last != NULL)
	{
		wlan->wlan_last = NULL;
	}
	while(wlan->wlan_list != NULL)
	{
		wlan_tmp = wlan->wlan_list;
		wlan->wlan_list = wlan_tmp->next;
		wlan_tmp->next = NULL;
		free(wlan_tmp);
		wlan_tmp = NULL;
	}
	if(wlan->bss_last != NULL) {
		wlan->bss_last = NULL;
	}
	while(wlan->bss_list != NULL) {
		tmp = wlan->bss_list;
		wlan->bss_list = tmp->next;
		tmp->next = NULL;
		dcli_free_bss(tmp);
	}
	free(wlan);
	wlan = NULL;
	return ;
}

void dcli_free_wtp(struct dcli_wtp_info *wtp)
{
	struct dcli_bss_info *tmp = NULL;
	struct dcli_radio_info *tmp2 = NULL;

	if(wtp == NULL)
		return ;
	
	if(wtp->bss_last != NULL) {
		wtp->bss_last = NULL;
	}
	while(wtp->bss_list != NULL) {
		tmp = wtp->bss_list;
		wtp->bss_list = tmp->next;
		tmp->next = NULL;
		dcli_free_bss(tmp);
	}
	if(wtp->radio_last != NULL) {
		wtp->radio_last = NULL;
	}
	while(wtp->radio_list != NULL) {
		tmp2 = wtp->radio_list;
		wtp->radio_list = tmp2->next;
		tmp2->next = NULL;
		dcli_free_radio(tmp2);
	}
	if(wtp->acl_conf.deny_mac != NULL)
		dcli_free_maclist(wtp->acl_conf.deny_mac);
	if(wtp->acl_conf.accept_mac != NULL)
		dcli_free_maclist(wtp->acl_conf.accept_mac);
	
	free(wtp);
	wtp = NULL;
	return ;
	
}
//
//mahz add 2011.1.18
void dcli_free_wtp_list(struct dcli_wtp_info *wtp)
{
	struct dcli_wtp_info *tmp = NULL;
	int i = 0;
	while(wtp != NULL) {
		tmp = wtp;
		wtp = tmp->next;
		tmp->next = NULL;
		dcli_free_wtp(tmp);
	}
	return ;
}
//


//mahz add 2011.1.25 , for mib request , dot11StaWAPIProtocolConfigTable
void dcli_sta_wapi_mib_info_free_wtp(struct wapi_mib_wtp_info *wtp)
{
	struct dcli_sta_info *tmp = NULL;
	
	if(wtp == NULL)
		return ;
	
	if(wtp->sta_last != NULL) {
		wtp->sta_last = NULL;
	}
	while(wtp->sta_list != NULL) {
		tmp = wtp->sta_list;
		wtp->sta_list = tmp->next;
		tmp->next = NULL;
		dcli_free_sta(tmp);
	}
	free(wtp);
	wtp = NULL;
	return ;	
}
void dcli_sta_wapi_mib_info_free_wtp_list(struct wapi_mib_wtp_info *wtp)
{
	struct wapi_mib_wtp_info *tmp = NULL;

	while(wtp != NULL) {
		tmp = wtp;
		wtp = tmp->next;
		tmp->next = NULL;
		dcli_sta_wapi_mib_info_free_wtp(tmp);
	}
	return ;
}
//


void dcli_free_channel(struct dcli_channel_info *channel)
{
	struct dcli_channel_info *tmp = NULL;

	if(channel == NULL)
		return ;
	
	if(channel->channel_last != NULL) {
		channel->channel_last = NULL;
	}
	while(channel->channel_list != NULL) {
		tmp = channel->channel_list;
		channel->channel_list = channel->next;
		tmp->next = NULL;
		dcli_free_channel(tmp);
	}
	free(channel);
	channel = NULL;
	return ;
	
}


void dcli_free_ac(struct dcli_ac_info *ac)
{
	struct dcli_bss_info 	*tmp1 = NULL;
	struct dcli_wtp_info 	*tmp2 = NULL;
	struct dcli_wlan_info 	*tmp3 = NULL;

	if(ac == NULL)
		return ;
	
	if(ac->bss_last != NULL) {
		ac->bss_last = NULL;
	}
	while(ac->bss_list != NULL) {
		tmp1 = ac->bss_list;
		ac->bss_list = tmp1->next;
		tmp1->next = NULL;
		dcli_free_bss(tmp1);
	}
	
	if(ac->wtp_last != NULL) {
		ac->wtp_last = NULL;
	}
	while(ac->wtp_list != NULL) {
		tmp2 = ac->wtp_list;
		ac->wtp_list = tmp2->next;
		tmp2->next = NULL;
		dcli_free_wtp(tmp2);
	}

	if(ac->wlan_last != NULL) {
		ac->wlan_last = NULL;
	}
	while(ac->wlan_list != NULL) {
		tmp3 = ac->wlan_list;
		ac->wlan_list = tmp3->next;
		tmp3->next = NULL;
		dcli_free_wlan(tmp3);
	}

	free(ac);
	ac = NULL;
	return ;
	
}

void dcli_free_static_sta_tab(struct sta_static_info *tab)
{
	struct sta_static_info *tmp = NULL;

	if(tab == NULL)
		return ;
	
	while(tab != NULL) {
		tmp = tab;
		tab = tab->next;
		tmp->next = NULL;
		free(tmp);
	}

	return ;
}
//qiuchen add it 2012.10.23
void dcli_free_hotspot_list(struct sta_nas_info *hs[])
{
	struct sta_nas_info *temp;
	int i = 1;
	for(i=1;i<4097;i++){
		if(hs[i]){
			free(hs[i]);
			hs[i] = NULL;
		}
	}
	return;
}
//qiuchen add it 2012.10.24
void dcli_free_r_sta_group(struct dcli_r_sta_info *r_sta_group,int NUM){
	if(r_sta_group == NULL)
		return;
	int i=0;
 	for(i=0;i<NUM;i++){
		if(r_sta_group[i].r_sta_list)
			free(r_sta_group[i].r_sta_list);
	}
	free(r_sta_group);
}
void dcli_free_bss_summary_list(struct dcli_bss_summary_info *bss_summary_list,int NUM)
{
	if(bss_summary_list == NULL)
		return;
	int i=0;
	for(i=0;i<NUM;i++){
		if(bss_summary_list[i].bss_list)
			free(bss_summary_list[i].bss_list);
	}
	free(bss_summary_list);
}
struct sta_nas_info* show_hotspot_list(DBusConnection *dcli_dbus_connection,struct sta_nas_info *hs[],int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	//struct sta_nas_info *hs[HOTSPOT_ID+1];
	unsigned int num = 0;
	int i = 0,j = 0;
	char *nas_identifier = NULL;
	char *nas_port_id = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_HOTSPOT_LIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&num);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for(i = 0;i<num;i++){
		DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(j));
		
		hs[j] = (struct sta_nas_info*)malloc(sizeof(struct sta_nas_info));
		if(hs[j] == NULL){
			dcli_free_hotspot_list(hs);
			hs = NULL;
			return NULL;
		}	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(nas_identifier));
	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(nas_port_id));
		
		//printf("*************nas_identifier is %s\n",nas_identifier);
		//printf("*************nas_port_id is %s\n",nas_port_id);
		
		memset(hs[j]->nas_identifier,0,sizeof(hs[j]->nas_identifier));
		memcpy(hs[j]->nas_identifier,nas_identifier,strlen(nas_identifier));

		memset(hs[j]->nas_port_id,0,sizeof(hs[j]->nas_port_id));
		memcpy(hs[j]->nas_port_id,nas_port_id,strlen(nas_port_id));
		//printf("*************hs-nas_identifier is %s\n",hs[j]->nas_identifier);
		//printf("*************hs-nas_port_id is %s\n",hs[j]->nas_port_id);

		dbus_message_iter_next(&iter_array);
		
	}
	dbus_message_unref(reply);
	
	//return hs;

	
}

struct dcli_ac_info* show_sta_by_interface(DBusConnection *dcli_dbus_connection,int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j;
	char *Binding_IF_NAME = NULL;
	struct dcli_ac_info  *ac = NULL;
	struct dcli_iface_info *iface = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STALIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(ac,0,sizeof(*ac));
		ac->bss_list = NULL;
		ac->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_bss_wireless));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_auth));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_auth_fail));		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_assoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_reassoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_assoc_failure));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_reassoc_failure));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_normal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_abnormal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<ac->num_bss_wireless; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		
			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));

			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->traffic_limit));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->send_traffic_limit));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->Radio_G_ID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->BSSIndex));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->SecurityID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_reassoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc_failure));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));	
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&Binding_IF_NAME);
			memset(bss->Binding_IF_NAME,0,ETH_IF_NAME_LEN);
			memcpy(bss->Binding_IF_NAME,Binding_IF_NAME,strlen(Binding_IF_NAME));
			

			for(iface = ac->iface_list; iface; iface = iface->next){
				if((strlen(iface->IF_NAME) == strlen(Binding_IF_NAME)) &&
					(!memcmp(iface->IF_NAME,Binding_IF_NAME,strlen(Binding_IF_NAME))))
					break;
			}
			if(iface) {
				iface->num_sta += bss->num_sta;
				iface->num_bss ++;
				iface->bss_last->next = bss;
			}else {
				if((iface = (struct dcli_iface_info*)malloc(sizeof(*iface))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_ac(ac);
					return NULL;
				}
				memset(iface,0,sizeof(*iface));
				memcpy(iface->IF_NAME,Binding_IF_NAME,ETH_IF_NAME_LEN);
				iface->num_sta += bss->num_sta;
				iface->num_bss ++;
				iface->bss_list = bss;
				iface->next = ac->iface_list;
				ac->iface_list = iface;
				ac->num_iface++;
			}
			iface->bss_last = bss;
				
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			ac->num_sta += bss->num_sta;
			bss->WtpID = bss->Radio_G_ID/L_RADIO_NUM;
			bss->Radio_L_ID = bss->Radio_G_ID%L_RADIO_NUM;
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_ac(ac);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->vlan_id));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_traffic_limit));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_send_traffic_limit));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_flags));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->pae_state));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->backend_state));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->StaTime));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));
				if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
					memset(sta->ip,0,strlen(in_addr)+1);
					memcpy(sta->ip,in_addr,strlen(in_addr));
				}

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rxbytes));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->txbytes));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->eap_type));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->identify));
				//qiuchen
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_access_time));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time_new));	
				//end
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}


		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_bss_wired));


		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<ac->num_bss_wired; i++){		
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		

			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(ac->bss_list == NULL){
				ac->bss_list = bss;
			}else{
				ac->bss_last->next = bss;
			}
			ac->bss_last = bss;

			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->PortID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->VlanID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->SecurityID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			ac->num_sta_wired += bss->num_sta;
			
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_ac(ac);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_flags));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->pae_state));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->backend_state));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->StaTime));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));

				if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
					memset(sta->ip,0,strlen(in_addr)+1);
					memcpy(sta->ip,in_addr,strlen(in_addr));
				}

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rxpackets));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->txpackets));
							
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}
		ac->num_sta_all = ac->num_sta + ac->num_sta_wired;
	}
	dbus_message_unref(reply);
	
	return ac;
}


struct dcli_sta_info* get_sta_info_by_mac(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError	err;
	struct dcli_sta_info* sta = NULL;

	
	unsigned char *in_addr = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(*ret == 0){
		if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) != NULL){
			memset(sta,0,sizeof(*sta));
			memcpy(sta->addr,mac,MAC_LEN);	
			sta->next = NULL;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wlan_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->radio_g_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->bssindex)); 
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->security_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->vlan_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_flags));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->pae_state));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->backend_state));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->StaTime));
			//qiuchen add it
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_online_time));
			//end

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_traffic_limit));	

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_send_traffic_limit)); 
#if 1
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(in_addr));	

			
			if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
										memset(sta->ip,0,strlen(in_addr)+1);
										memcpy(sta->ip,in_addr,strlen(in_addr));		
				}
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->snr)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->rr)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->tr)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->tp)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->rxbytes)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->txbytes)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->rxpackets)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->txpackets)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->retrybytes)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->retrypackets)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->errpackets)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->mode)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->channel)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->rssi)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->nRate)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->isPowerSave)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->isQos)); 

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->info_channel)); 

			//mahz add 2011.3.1
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->security_type)); 
#endif
			/*wapi mib*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wapi_version));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->controlled_port_status));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->selected_unicast_cipher[0]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->selected_unicast_cipher[1]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->selected_unicast_cipher[2]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->selected_unicast_cipher[3]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wpi_replay_counters));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wpi_decryptable_errors));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wpi_mic_errors));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_sign_errors));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_hmac_errors));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_auth_res_fail));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_discard));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_timeout));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_format_errors));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_cert_handshake_fail));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_unicast_handshake_fail));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wai_multi_handshake_fail));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_access_time));	

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_online_time_new));	
		}
			
	}	
	
	dbus_message_unref(reply);
	
	return sta;
}

struct dcli_sta_info_v2* get_sta_info_by_mac_v2(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError	err;
	struct dcli_sta_info_v2* sta = NULL;
	int i = 0;
	char *essid = NULL;	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_V2);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(*ret == 0){
		if((sta = (struct dcli_sta_info_v2*)malloc(sizeof(*sta))) != NULL){
			memset(sta,0,sizeof(*sta));
			sta->next = NULL;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wlan_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->radio_g_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->bssindex)); 
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->security_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->vlan_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[0]));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[1]));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[2]));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[3]));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[4]));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[5]));	
				
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->essidlen));	

			essid = (char*)malloc(sta->essidlen+1);
			memset(essid,0,sta->essidlen+1);
			for(i=0;i<sta->essidlen;i++){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(essid[i]));	
			}
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->flow_check));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->no_flow_time));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->limit_flow));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->auth_type)); 

			sta->essid = (char *)malloc(sta->essidlen+1);
			memset(sta->essid, 0, sta->essidlen+1);
			memcpy(sta->essid, essid,sta->essidlen);
			free(essid);
		}
			
	}	
	
	dbus_message_unref(reply);
	
	return sta;
}

struct dcli_wtp_info * sort_sta_ascending(struct dcli_wtp_info *head, unsigned int 	num_wtp){
	char flag = 0;
	unsigned int i=0;
	struct dcli_wtp_info *p=NULL;
	struct dcli_wtp_info *q=NULL;
	struct dcli_wtp_info *tmp=NULL;
	struct dcli_wtp_info *tmp1=NULL;

	if((head==NULL)||(num_wtp==0)||(num_wtp==1))
		return head;
	
	for(i=0;i<num_wtp;i++){
		flag = 0;
		tmp = p = head;
		q = p->next;

		if(p&&q&&(p->num_sta > q->num_sta)){
			p->next = q->next;
			q->next = p;
			head = q;
			flag = 1;

			tmp = head;
			p = head->next;
			q = p->next;
		}
		if(flag != 1){
			tmp = head;
			p = head->next;
			if(p != NULL){
				q = p->next;
			}
			else{
				q = NULL;
			}
		}
		while(p&&q){
			if(p->num_sta > q->num_sta){
				p->next = q->next;
				q->next = p;
				tmp->next = q;
				flag = 1;
			}
			tmp = tmp->next;
			p = tmp->next;
			q = p->next;
		}

		if(flag == 0)
			break;
	}
	return head;
}	
struct dcli_wtp_info * sort_sta_descending(struct dcli_wtp_info *head, unsigned int 	num_wtp){
	char flag = 0;
	unsigned int i=0;
	struct dcli_wtp_info *p=NULL;
	struct dcli_wtp_info *q=NULL;
	struct dcli_wtp_info *tmp=NULL;
	struct dcli_wtp_info *tmp1=NULL;

	//printf("num_wtp = %d\n",num_wtp);//for test
	if((head==NULL)||(num_wtp==0)||(num_wtp==1))
		return head;
	
	for(i=0;i<num_wtp;i++){
		//printf("i=%d\n",i);//for test
		flag = 0;
		tmp = p = head;
		q = p->next;

		if(p&&q&&(p->num_sta < q->num_sta)){
			//printf("if branch\n");//for test
			p->next = q->next;
			q->next = p;
			head = q;
			flag = 1;
			//printf("head->wtpid = %d\n",head->WtpID);//for test

			tmp = head;
			p = head->next;
			q = p->next;
		}
		if(flag != 1){
			//printf("2222222222\n");//for test
			tmp = head;
			p = head->next;
			if(p != NULL){
				q = p->next;
				//printf("p is not NULL\n");//for test
				//printf("p->wtpid = %d\n",p->WtpID);//for test
			}
			else{
				q = NULL;
				//printf("q is set to NULL\n");//for test
			}
			//printf("head->wtpid = %d\n",head->WtpID);//for test
			//printf("p->wtpid = %d\n",p->WtpID);//for test
		}
		while(p&&q){
			//printf("33333333333\n");//for test
			if(p->num_sta < q->num_sta){
				p->next = q->next;
				q->next = p;
				tmp->next = q;
				flag = 1;
			}
			//printf("head->wtpid = %d\n",head->WtpID);//for test
			tmp = tmp->next;
			p = tmp->next;
			q = p->next;
		}

	/*	tmp1 = head;	//for test
		while(tmp1 != NULL){
			printf("tmp1->wtpid = %d\n",tmp1->WtpID);//for test
			tmp1 = tmp1->next;
		}
		printf("head->wtpid = %d\n",head->WtpID);//for test
	*/	
		if(flag == 0)
			break;
	}
	return head;
}	

struct dcli_ac_info* show_sta_summary(DBusConnection *dcli_dbus_connection,int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j;
	struct dcli_ac_info  *ac = NULL;
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_wlan_info *wlan = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_SUMMARY);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		dbus_message_unref(reply);
		return NULL;
	}
	memset(ac,0,sizeof(*ac));
	ac->bss_list = NULL;
	ac->bss_last = NULL;
	ac->wtp_list = NULL;
	ac->wtp_last = NULL;
	ac->wlan_list = NULL;
	ac->wlan_last = NULL;

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac->num_accessed_sta));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac->num_sta));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac->num_local_roam));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac->num_unconnect_sta));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac->num_wlan));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<ac->num_wlan; i++){
		DBusMessageIter iter_struct;
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			dcli_free_ac(ac);
			return NULL;
		}
		memset(wlan,0,sizeof(*wlan));
		wlan->next = NULL;
		if(ac->wlan_list == NULL){
			ac->wlan_list = wlan;
		}else{
			ac->wlan_last->next = wlan;
		}
		ac->wlan_last = wlan;

		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(wlan->WlanID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wlan->num_sta));

		dbus_message_iter_next(&iter_array);

	}


	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac->num_wtp));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<ac->num_wtp; i++){
		DBusMessageIter iter_struct;
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			dcli_free_ac(ac);
			return NULL;
		}
		memset(wtp,0,sizeof(*wtp));
		wtp->next = NULL;
		if(ac->wtp_list == NULL){
			ac->wtp_list = wtp;
		}else{
			ac->wtp_last->next = wtp;
		}
		ac->wtp_last = wtp;

		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(wtp->WtpID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wtp->num_sta));

		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);

	return ac;
}

struct dcli_ac_info* show_sta_list(DBusConnection *dcli_dbus_connection,int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j;
	char *Binding_IF_NAME = NULL;
	struct dcli_ac_info  *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	unsigned char *identify = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STALIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(ac,0,sizeof(*ac));
		ac->bss_list = NULL;
		ac->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_bss_wireless));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_auth));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_auth_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_assoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_reassoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_assoc_failure));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_reassoc_failure));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_normal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_abnormal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<ac->num_bss_wireless; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		
			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(ac->bss_list == NULL){
				ac->bss_list = bss;
			}else{
				ac->bss_last->next = bss;
			}
			ac->bss_last = bss;

			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->traffic_limit));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->send_traffic_limit));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->Radio_G_ID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->BSSIndex));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->SecurityID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_reassoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc_failure));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));	
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&Binding_IF_NAME);
			memset(bss->Binding_IF_NAME,0,ETH_IF_NAME_LEN);
			memcpy(bss->Binding_IF_NAME,Binding_IF_NAME,ETH_IF_NAME_LEN);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			ac->num_sta += bss->num_sta;
			bss->WtpID = bss->Radio_G_ID/L_RADIO_NUM;
			bss->Radio_L_ID = bss->Radio_G_ID%L_RADIO_NUM;
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;
				identify = NULL;
				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_ac(ac);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->vlan_id));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_traffic_limit));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_send_traffic_limit));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_flags));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->pae_state));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->backend_state));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->StaTime));
				
				//qiuchen add it
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time));	
				//end

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));
				if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
					memset(sta->ip,0,strlen(in_addr)+1);
					memcpy(sta->ip,in_addr,strlen(in_addr));
				}

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rxbytes));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->txbytes));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->security_type)); 

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->eap_type));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(identify));
				if((sta->identify = (unsigned char *)malloc(strlen(identify)+1)) !=NULL){
					memset(sta->identify,0,strlen(identify)+1);
					memcpy(sta->identify,identify,strlen(identify));
				}
				//qiuchen
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_access_time));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time_new));	
				//end
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}


		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_bss_wired));


		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<ac->num_bss_wired; i++){		
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		

			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(ac->bss_list == NULL){
				ac->bss_list = bss;
			}else{
				ac->bss_last->next = bss;
			}
			ac->bss_last = bss;

			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->PortID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->VlanID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->SecurityID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			ac->num_sta_wired += bss->num_sta;
			
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dcli_free_ac(ac);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_flags));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->pae_state));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->backend_state));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->StaTime));
				//qiuchen add it
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time));	
				//end
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));

				if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
					memset(sta->ip,0,strlen(in_addr)+1);
					memcpy(sta->ip,in_addr,strlen(in_addr));
				}

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rxpackets));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->txpackets));
							
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}
		ac->num_sta_all = ac->num_sta + ac->num_sta_wired;
	}
	dbus_message_unref(reply);
	
	return ac;
}

struct dcli_ac_info* show_sta_base_info(DBusConnection *dcli_dbus_connection,int index, int localid, unsigned int *ret,unsigned int *bss_num)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j,k;
	struct dcli_base_bss_info *bss = NULL;
	struct dcli_base_bss_info *bsshead = NULL;
	struct dcli_base_bss_info *bsstail = NULL;
	struct dcli_sta_base_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_BASE_INFO);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,bss_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0;i<(*bss_num);i++){
			char *essid = malloc(ESSID_DEFAULT_LEN+1); 
			memset(essid,0,ESSID_DEFAULT_LEN+1);
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		
			if((bss = (struct dcli_base_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_base_bss(bsshead);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			bss->sta_list = NULL;
			bss->sta_last = NULL;

			if(bsshead == NULL){
				bsshead = bss;
				bsstail = bss;
			}else{
				bsstail->next = bss;
				bsstail = bss;
			}
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->Radio_G_ID));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));	

			//use bss->mac to accept wtp->mac
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[0]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[1]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[2]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[3]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[4]));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[5]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	

			for(k=0;k<ESSID_DEFAULT_LEN;k++){
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(essid[k]));	
			}
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			bss->WtpID = bss->Radio_G_ID/L_RADIO_NUM;
			bss->Radio_L_ID = bss->Radio_G_ID%L_RADIO_NUM;

			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_base_bss(bsshead);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));
				if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
					memset(sta->ip,0,strlen(in_addr)+1);
					memcpy(sta->ip,in_addr,strlen(in_addr));
				}
				if((sta->essid = (unsigned char *)malloc(ESSID_DEFAULT_LEN+1)) != NULL) {
					memset(sta->essid,0,ESSID_DEFAULT_LEN+1);
					memcpy(sta->essid,essid,ESSID_DEFAULT_LEN);
				}
				memcpy(sta->wtp_addr,bss->mac,6);
				sta->wtp_id = bss->WtpID;
				sta->wlan_id = bss->WlanID;
				sta->radio_g_id = bss->Radio_G_ID;
				sta->radio_l_id = bss->Radio_L_ID;
				
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	
	return bsshead;
}

struct dcli_wtp_info* show_sta_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wtp_info  *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_STALIST);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_bss));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_assoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_reassoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_assoc_failure));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_normal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_abnormal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<wtp->num_bss; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		
			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wtp(wtp);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(wtp->bss_list == NULL){
				wtp->bss_list = bss;
			}else{
				wtp->bss_last->next = bss;
			}
			wtp->bss_last = bss;

			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->Radio_G_ID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->BSSIndex));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->SecurityID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_reassoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc_failure));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));	
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			wtp->num_sta += bss->num_sta;
			bss->WtpID = bss->Radio_G_ID/L_RADIO_NUM;
			bss->Radio_L_ID = bss->Radio_G_ID%L_RADIO_NUM;
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wtp(wtp);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_flags));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->pae_state));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->backend_state));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->StaTime));	
				//qiuchen add it
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time));
				//end
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_access_time));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time_new));
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}
	}
	dbus_message_unref(reply);
	
	return wtp;
}

struct dcli_wtp_info* extend_show_sta_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtp_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wtp_info  *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){	
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
			memset(wtp,0,sizeof(*wtp));
			wtp->bss_list = NULL;
			wtp->bss_last = NULL;
				
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->acc_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->repauth_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_bss));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->deny_num));

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<wtp->num_bss;i++){		
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
				
			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wtp(wtp);
				return NULL;
				}
				memset(bss,0,sizeof(*bss));
				bss->next = NULL;
					
			if(wtp->bss_list == NULL){
				wtp->bss_list = bss;
			}else{
				wtp->bss_last->next = bss;
			}
				wtp->bss_last = bss;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				
				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
						*ret = ASD_DBUS_MALLOC_FAIL;
						dbus_message_unref(reply);
						dcli_free_wtp(wtp);
						return NULL;
					}
					memset(sta,0,sizeof(*sta));
					sta->next = NULL;
					if(bss->sta_list == NULL){
						bss->sta_list = sta;
					}else{
						bss->sta_last->next = sta;
					}
					bss->sta_last = sta;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->mode)); 
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->channel));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rssi)); 
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->nRate));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->isPowerSave));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->isQos));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->snr));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rr));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->tr));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->tp));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rx_pkts));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->tx_pkts));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rtx));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rtx_pkts)); 
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->err_pkts)); 
			
				dbus_message_iter_next(&iter_sub_array);
			}
			
			dbus_message_iter_next(&iter_array);
		}
		
	}		
	
	dbus_message_unref(reply);	
	return wtp;
}
//fengwenchao add 20110113  for dot11WlanStationTable
struct dcli_wlan_info* show_sta_of_allwlan(DBusConnection *dcli_dbus_connection, int index, int *wlan_num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;	
	DBusMessageIter iter_sub_struct;
	DBusMessageIter iter_sub_sub_array;	
	DBusMessageIter iter_sub_sub_struct;			
	DBusError	err;
	int i = 0;
	int j = 0;
	int k = 0;
	struct dcli_wlan_info  *wlan = NULL;
	struct dcli_wlan_info  *wlan2 = NULL;
	struct dcli_wlan_info  *wlannode = NULL;	
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_ALL_WLAN_STALIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,wlan_num);

	if(*ret == 0)
	{

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0;i<(*wlan_num);i++)
		{
			if((wlannode = (struct dcli_wlan_info*)malloc(sizeof(struct dcli_wlan_info))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wlanlist(wlan);
				return NULL;
			}
		
			memset(wlannode,0,sizeof(*wlan));
			wlannode->next = NULL;
			wlannode->bss_list = NULL;
			wlannode->bss_last = NULL;

			if(wlan == NULL){
				wlan = wlannode;
				wlan2= wlannode;
			}
			else{
				wlan2->next = wlannode;
				wlan2 = wlannode;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->WlanID));

		//	printf("wlannode->WlanID = %d \n",wlannode->WlanID);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wlannode->num_bss));

		//	printf("wlannode->num_bss = %d \n",wlannode->num_bss);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<wlannode->num_bss; j++)
			{	
	
				if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wlanlist(wlan);
					return NULL;
				}
				memset(bss,0,sizeof(*bss));
				bss->next = NULL;
				if(wlannode->bss_list == NULL){
					wlannode->bss_list = bss;
				}else{
					wlannode->bss_last->next = bss;
				}
				wlannode->bss_last = bss;	

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss->num_sta));

				dbus_message_iter_next(&iter_sub_struct);			
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);

				for(k=0; k<bss->num_sta;k++)
				{	
					//DBusMessageIter iter_sub_struct;
					unsigned char *in_addr = NULL;

					if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
						*ret = ASD_DBUS_MALLOC_FAIL;
						dbus_message_unref(reply);
						dcli_free_wlanlist(wlan);
						return NULL;
					}
					memset(sta,0,sizeof(*sta));
					sta->next = NULL;
					if(bss->sta_list == NULL){
						bss->sta_list = sta;
					}else{
						bss->sta_last->next = sta;
					}
					bss->sta_last = sta;
					
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);

					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->addr[0]));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->addr[1]));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->addr[2]));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->addr[3]));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->addr[4]));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->addr[5]));	
						
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->StaTime));
					//qiuchen add it
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->sta_online_time));	
					//end

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->sta_traffic_limit));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->sta_send_traffic_limit));

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->rxbytes));	

					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->txbytes));						

					
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->sta_access_time));	
					dbus_message_iter_next(&iter_sub_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sta->sta_online_time_new));	

					dbus_message_iter_next(&iter_sub_sub_array);
				}		
				dbus_message_iter_next(&iter_sub_array);	
			}	
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);	
	return wlan;
}
//fengwenchao add end
struct dcli_wlan_info* show_sta_bywlan(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wlan_info  *wlan = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_STALIST);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wlan,0,sizeof(*wlan));
		wlan->bss_list = NULL;
		wlan->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->num_bss));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->num_assoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->num_reassoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->num_assoc_failure));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->num_normal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->num_abnormal_sta_down));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<wlan->num_bss; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		
			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wlan(wlan);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(wlan->bss_list == NULL){
				wlan->bss_list = bss;
			}else{
				wlan->bss_last->next = bss;
			}
			wlan->bss_last = bss;

			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->Radio_G_ID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->BSSIndex));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->SecurityID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_reassoc));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_assoc_failure));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));	
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			wlan->num_sta += bss->num_sta;
			bss->WtpID = bss->Radio_G_ID/L_RADIO_NUM;
			bss->Radio_L_ID = bss->Radio_G_ID%L_RADIO_NUM;
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char *in_addr = NULL;

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wlan(wlan);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_flags));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->pae_state));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->backend_state));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->StaTime));
				//qiuchen add it
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time));
				//end
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_access_time));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_online_time_new));
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}
	}
	dbus_message_unref(reply);
	
	return wlan;
}

//fengwenchao add 20101224
struct dcli_wlan_info* show_info_allwlan(DBusConnection *dcli_dbus_connection, int index, int localid,unsigned int *ret,int *wlan_num)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusError	err;
	dbus_error_init(&err);

	int i,j;
	struct dcli_wlan_info  *wlan_head = NULL;
	struct dcli_wlan_info  *wlan_node = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_ALLWLAN);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wlan_num);


	if((*ret == 0)&&(*wlan_num != 0))
	{
		if((wlan_head = (struct dcli_wlan_info*)malloc(sizeof(struct dcli_wlan_info))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		dbus_message_unref(reply);
		return NULL;
			}
		memset(wlan_head,0,sizeof(struct dcli_wlan_info));
		wlan_head->wlan_list = NULL;
		wlan_head->wlan_last = NULL;
		
		dbus_message_iter_next(&iter);	
	    dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0;i<(*wlan_num);i++)
		{

			if((wlan_node = (struct dcli_wlan_info*)malloc(sizeof(struct dcli_wlan_info))) == NULL)
			{
				dcli_free_allwlan(wlan_head);
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wlan_node,0,sizeof(struct dcli_wlan_info));
			wlan_node->next = NULL;

			if(wlan_head->wlan_list == NULL){
				wlan_head->wlan_list = wlan_node;
				wlan_head->next = wlan_node;
			}
			else{
				wlan_head->wlan_last->next = wlan_node;
			}
			wlan_head->wlan_last = wlan_node;


			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->WlanID));

			//printf("wlan_node->WlanID  =  %u  \n",wlan_node->WlanID);

			/*dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->rx_pkts));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->tx_pkts));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->rx_bytes));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->tx_bytes));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->rx_pkts));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->tx_pkts));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->rx_bytes));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->tx_bytes));*/

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->assoc_req_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->assoc_resp_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->assoc_fail_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->num_normal_sta_down));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->num_abnormal_sta_down));			

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->num_sta));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->sta_assoced_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_node->sta_accessed_num));

			dbus_message_iter_next(&iter_array);
		}
	
		
	}
	dbus_message_unref(reply);
	//printf("wlan_head->WlanID  =  %u  \n",wlan_head->wlan_list->WlanID);

	/*fengwenchao add 20110622*/
		/*fengwenchao add 20110617*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	
	unsigned int rx_data_pkts = 0;			 
	unsigned int tx_data_pkts = 0;
	unsigned long long rx_data_bts = 0;
	unsigned long long tx_data_bts = 0;
	unsigned char wlan_id = 0;
	unsigned int ret2= 0;
	unsigned int wid_wlan_num = 0;
	struct dcli_wlan_info  *wlan_search = NULL;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,WD_DBUS_CONF_METHOD_SHOW_INFO_ALLWLAN);
						    
	dbus_error_init(&err2);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_free_allwlan(wlan_head);
		return NULL;
	}	

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	dbus_message_iter_next(&iter2);	
	dbus_message_iter_get_basic(&iter2,&wid_wlan_num);

	if((ret2 ==0)&&(wid_wlan_num == (*wlan_num))&&(*ret == 0))
	{
	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_recurse(&iter2,&iter_array2);
		
	    for(i = 0; i < wid_wlan_num; i++)
	    {
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array2,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wlan_id));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_data_pkts));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_data_pkts));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_data_bts));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_data_bts));			

			wlan_search = wlan_head->wlan_list ;

			while(wlan_search != NULL)
			{
				if(wlan_search->WlanID == wlan_id)
				{
					wlan_search->rx_pkts = rx_data_pkts;
					wlan_search->tx_pkts = tx_data_pkts;
					wlan_search->rx_bytes = rx_data_bts;
					wlan_search->tx_bytes = tx_data_bts;
				}
				wlan_search = wlan_search->next;
			}
			 dbus_message_iter_next(&iter_array2);
	    }
		dbus_message_unref(reply2);
	}	
    /*fengwenchao add end*/

	return wlan_head;
}
//fengwenchao add end
struct dcli_wlan_info* show_info_bywlan(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wlan_info  *wlan = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wlan,0,sizeof(*wlan));
		wlan->bss_list = NULL;
		wlan->bss_last = NULL;

		/*dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->rx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->tx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->rx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->tx_bytes));*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_resp_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->sta_accessed_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->sta_assoced_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->reassoc_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->reassoc_success_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_req_interim));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_resp_interim));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_success_interim));

	}
	dbus_message_unref(reply);

	/*fengwenchao add 20110617*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	
	unsigned int wtpid = 0;
	unsigned int ret2= 0;
	unsigned int wtp_num = 0;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,WID_DBUS_CONF_METHOD_SHOW_INFO_BYWLANID);
				
	dbus_error_init(&err2);
	dbus_message_append_args(query2,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_INVALID);
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		return NULL;
	}	

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	if((ret2 ==0)&&(wlan != NULL))
	{
		dbus_message_iter_next(&iter2);	
		dbus_message_iter_get_basic(&iter2,&(wlan->rx_pkts));
		dbus_message_iter_next(&iter2);	
		dbus_message_iter_get_basic(&iter2,&(wlan->tx_pkts));
		dbus_message_iter_next(&iter2);	
		dbus_message_iter_get_basic(&iter2,&(wlan->rx_bytes));
		dbus_message_iter_next(&iter2);	
		dbus_message_iter_get_basic(&iter2,&(wlan->tx_bytes));
		//printf("  rx_pkts  =  %u \n ",wlan->rx_pkts);
		//printf("  tx_pkts  =  %u \n ",wlan->tx_pkts);
		//printf("  rx_bytes  =  %llu \n ",wlan->rx_bytes);
		//printf("  tx_bytes  =  %llu \n ",wlan->tx_bytes);

		dbus_message_unref(reply2);
	}	
    /*fengwenchao add end*/	
	return wlan;
}

/*nl add 0091229*/
struct dcli_wtp_info* show_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wtp_info  *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	

	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wtp_total_past_online_time));	/*	xm0703*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_assoc_failure));	/*	xm0703*/

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_accessed_sta));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->acc_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->repauth_tms));//5

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_refused_num));//10
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_resp_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_timeout_num));//15
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_others_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_invalid_num));//20
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->identify_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->identify_success_num));//25
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->abort_key_error_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->abort_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->abort_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->abort_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->abort_others_num));//30

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->deauth_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->deauth_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->deauth_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->deauth_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->deauth_others_num));//35
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->disassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->disassoc_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->disassoc_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->disassoc_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->disassoc_others_num));//40
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->rx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->tx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->rx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->tx_ctrl_pkts));
	/*	dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->rx_data_pkts));//45
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->tx_data_pkts));*/
		//printf("   wtp->rx_data_pkts   =  %d  \n ",wtp->rx_data_pkts);
		//printf("   wtp->tx_data_pkts   =  %d  \n ",wtp->tx_data_pkts);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->rx_auth_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->tx_auth_pkts));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_bss));//49

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);


		for(i=0; i<wtp->num_bss; i++){
			if((bss = (struct dcli_bss_info*)malloc(sizeof(struct dcli_bss_info))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wtp(wtp);
					return NULL;
				}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(wtp->bss_list == NULL){
				wtp->bss_list = bss;
			}else{
				wtp->bss_last->next = bss;
			}
			wtp->bss_last = bss;
			
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->BSSIndex));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->acc_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->repauth_tms));//5
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_fail_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_refused_num));//10
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->auth_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->assoc_req_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->assoc_resp_num));
	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->assoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->assoc_timeout_num));//15
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->assoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->assoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->reassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->reassoc_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->reassoc_invalid_num));//20
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->reassoc_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->reassoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->reassoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->identify_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->identify_success_num));//25
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->abort_key_error_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->abort_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->abort_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->abort_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->abort_others_num));//30
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->deauth_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->deauth_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->deauth_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->deauth_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->deauth_others_num));//35
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->disassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->disassoc_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->disassoc_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->disassoc_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->disassoc_others_num));//40

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->rx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->tx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->rx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->tx_ctrl_pkts));
		/*	dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->rx_data_pkts));//45
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->tx_data_pkts));*/
			//printf("   bss->rx_data_pkts   =  %d  \n ",bss->rx_data_pkts);
			//printf("   bss->tx_data_pkts   =  %d  \n ",bss->tx_data_pkts);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->tx_auth_pkts));//48
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->total_past_online_time));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->total_present_online_time));//48
			
			dbus_message_iter_next(&iter_array);
			}
		//mahz add 2011.5.3
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->usr_auth_tms_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->ac_rspauth_tms_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_fail_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->auth_success_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_assoc_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_reassoc_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_assoc_failure_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_reassoc_failure_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_success_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->reassoc_success_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_req_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->assoc_resp_record));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->total_ap_flow_record));
		//
	}
	dbus_message_unref(reply);	

	/*fengwenchao add 20110617*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	struct dcli_bss_info *bss_wid = NULL;
	unsigned int rx_data_pkts = 0;			 
	unsigned int tx_data_pkts = 0;
	//unsigned int wtpid = 0;
	unsigned int ret2= 0;
	unsigned int bss_num = 0;
	unsigned int bssindex = 0;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,WID_DBUS_CONF_METHOD_SHOW_INFO_BYWTPID);
						   						
	dbus_error_init(&err2);
	dbus_message_append_args(query2,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_free_wtp(wtp);
		return NULL;
	}	

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	if((ret2 == 0)&&(*ret == 0))
	{
	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_get_basic(&iter2,&wtp->rx_data_pkts);

	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_get_basic(&iter2,&wtp->tx_data_pkts);
		//printf("   wtp->rx_data_pkts   =  %d  \n ",wtp->rx_data_pkts);
		//printf("   wtp->tx_data_pkts   =  %d  \n ",wtp->tx_data_pkts);

	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_get_basic(&iter2,&bss_num);
		
	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_recurse(&iter2,&iter_array2);
		
	    for(i = 0; i < bss_num; i++)
	    {
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array2,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bssindex));
			//printf("  bssindex   =  %d  \n ",bssindex);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_data_pkts));
			//printf("  rx_data_pkts   =  %d  \n ",rx_data_pkts);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_data_pkts));
			//printf("  tx_data_pkts   =  %d  \n ",tx_data_pkts);
			bss_wid = wtp->bss_list;

			while(bss_wid != NULL)
			{
				if(bss_wid->BSSIndex == bssindex)
				{
					bss_wid->rx_data_pkts = rx_data_pkts;
					//printf("   bss_wid->rx_data_pkts   =  %d  \n ",bss_wid->rx_data_pkts);
					bss_wid->tx_data_pkts = tx_data_pkts;
					//printf("   bss_wid->tx_data_pkts   =  %d  \n ",bss_wid->tx_data_pkts);
				}
				bss_wid = bss_wid->next;
			}
			 dbus_message_iter_next(&iter_array2);
	    }
		dbus_message_unref(reply2);
	}	    
	return wtp;
}


//mahz add for mib request , 2011.1.17 , dot11DistinguishTable
struct dcli_wtp_info* show_distinguish_info_of_all_wtp(int index,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;

	int i;
	struct dcli_wtp_info *head = NULL;
	struct dcli_wtp_info *tail = NULL;
	struct dcli_wtp_info *tmp = NULL;
		
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_DISTINGUISH_INFO_OF_ALL_WTP);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);			
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret=ASD_DBUS_ERROR;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);	

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);

	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			if((tmp = (struct dcli_wtp_info*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(head == NULL)
				head = tmp;
			else 
				tail->next = tmp;
			tail = tmp;
			
			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->WtpID);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[0]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[1]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[2]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[3]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[4]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[5]));		
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->identify_request_num);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->identify_success_num);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->abort_key_error_num);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->abort_invalid_num);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->abort_refused_num);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->abort_timeout_num);
			
			dbus_message_iter_next(&iter_wtp);		//add 2011.4.26
			dbus_message_iter_get_basic(&iter_wtp,&tmp->abort_others_num);	
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->auth_tms);	

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->acc_tms);							

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->num_accessed_sta);							
			dbus_message_iter_next(&iter_wtp_array);
		}
	}

	dbus_message_unref(reply);
	return head;	
}
/*nl add 100107*/
struct dcli_radio_info* show_mib_info_byradio(DBusConnection *dcli_dbus_connection, int index, unsigned int radioid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_bss_info *bss = NULL;
	struct dcli_radio_info *radio = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_MIB_INFO_BYRDID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_MIB_INFO_BYRDID);*/
						
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){	
		if((radio = (struct dcli_radio_info*)malloc(sizeof(*radio))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		
		memset(radio,0,sizeof(*radio));
		radio->bss_list = NULL;
		radio->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&radio->num_bss);
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<radio->num_bss; i++) {		
			if((bss = (struct dcli_bss_info*)malloc(sizeof(struct dcli_bss_info))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_radio(radio);
					return NULL;
				}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(radio->bss_list == NULL){
				radio->bss_list = bss;
			}else{
				radio->bss_last->next = bss;
			}
			radio->bss_last = bss;

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->wl_up_flow));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wl_dw_flow));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_pck));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_los_pck));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_mac_err_pck));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_resend_pck));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_up_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_err_frm));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_los_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_dw_resend_frm));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_up_los_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ch_up_resend_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->send_bytes));
	
			dbus_message_iter_next(&iter_array);
		
		}
	}
	
	dbus_message_unref(reply);
	
	return radio;
	
}

/*nl add 100108*/
struct dcli_wtp_info* show_wapi_mib_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	int i,j,k;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_BYRDID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_BYRDID);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){	

		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(struct dcli_wtp_info))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_bss));
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for(i=0; i<wtp->num_bss; i++) {		

			if((bss = (struct dcli_bss_info*)malloc(sizeof(struct dcli_bss_info))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wtp(wtp);
					return NULL;
				}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(wtp->bss_list == NULL){
				wtp->bss_list = bss;
			}else{
				wtp->bss_last->next = bss;
			}
			wtp->bss_last = bss;

			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[0]));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[1]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[2]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[3]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[4]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->ControlledAuthControl));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->ControlledPortControl));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wapiEnabled));
						
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->CertificateUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->MulticastUpdateCount));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->UnicastUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->AuthenticationSuite));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->AuthSuiteSelected));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<bss->num_sta; j++){	

				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wtp(wtp);
					return NULL;
				}				
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
			
				DBusMessageIter iter_sub_struct;
												
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	
			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));

				for(k=0;k<16;k++){
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&(sta->BKIDUsed[k]));
				}			
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->UnicastRekeyTime));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->UnicastRekeyPackets));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->MulticastRekeyTime));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->MulticastRekeyPackets));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->ControlledPortStatus));
				
				dbus_message_iter_next(&iter_sub_array);
			}					
			dbus_message_iter_next(&iter_array);
			
		}
	}
							
	dbus_message_unref(reply);		
	return wtp;
	
}


//mahz add for mib request , 2011.1.19, dot11BSSIDWAPIProtocolConfigTable
struct dcli_wtp_info*  show_wapi_mib_info_of_all_wtp(DBusConnection *dcli_dbus_connection, int index, unsigned int *wtp_num, int localid, unsigned int *ret)

{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_bss_array;
	DBusMessageIter iter_bss;
	DBusError	err;
		
	struct dcli_wtp_info *head = NULL;
	struct dcli_wtp_info *tail = NULL;
	struct dcli_wtp_info *tmp = NULL;
	struct dcli_bss_info *bss = NULL;
			
	int i,j,k;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_OF_ALL_WTP);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);	

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);

	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			if((tmp = (struct dcli_wtp_info*)malloc(sizeof(struct dcli_wtp_info))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);				//之前内存泄露的问题可能与这有关
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			tmp->bss_list = NULL;
			tmp->bss_last = NULL;

			if(head == NULL)
				head = tmp;
			else 
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->WtpID);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[0]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[1]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[2]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[3]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[4]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[5]));		
			
			dbus_message_iter_next(&iter_wtp);	
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->num_bss));

			dbus_message_iter_next(&iter_wtp);		
			dbus_message_iter_recurse(&iter_wtp,&iter_bss_array);

			for(j=0; j<tmp->num_bss; j++) {
				if((bss = (struct dcli_bss_info*)malloc(sizeof(struct dcli_bss_info))) == NULL){
						*ret = ASD_DBUS_MALLOC_FAIL;
						dbus_message_unref(reply);
					//	dcli_free_wtp(tmp);				//free  or free list ??
						dcli_free_wtp_list(head);
						return NULL;
				}
				memset(bss,0,sizeof(*bss));
				bss->next = NULL;
				if(tmp->bss_list == NULL){
					tmp->bss_list = bss;
				}else{
					tmp->bss_last->next = bss;
				}
				tmp->bss_last = bss;
										
				dbus_message_iter_recurse(&iter_bss_array,&iter_bss);

				dbus_message_iter_get_basic(&iter_bss,&(bss->bssid[0]));
				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->bssid[1]));
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&(bss->bssid[2]));
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&(bss->bssid[3]));
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&(bss->bssid[4]));
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&(bss->bssid[5]));
				dbus_message_iter_next(&iter_bss);

				dbus_message_iter_get_basic(&iter_bss,&(bss->ControlledAuthControl));
				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->ControlledPortControl));
				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->wapiEnabled));

				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->CertificateUpdateCount));
				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->MulticastUpdateCount));
				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->UnicastUpdateCount));

				dbus_message_iter_next(&iter_bss);					
				dbus_message_iter_get_basic(&iter_bss,&(bss->AuthenticationSuite));
				dbus_message_iter_next(&iter_bss);	
				dbus_message_iter_get_basic(&iter_bss,&(bss->AuthSuiteSelected));
								
				dbus_message_iter_next(&iter_bss_array);
			}
		dbus_message_iter_next(&iter_wtp_array);
		}
	}
							
	dbus_message_unref(reply);		
	return head;	
}


//mahz add for mib request , 2011.1.24, dot11StaWAPIProtocolConfigTable
struct wapi_mib_wtp_info*  show_sta_wapi_mib_info_of_all_wtp(DBusConnection *dcli_dbus_connection, int index, unsigned int *wtp_num, int localid, unsigned int *ret)

{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_sta_array;
	DBusMessageIter iter_sta;
	DBusError	err;
		
	struct wapi_mib_wtp_info *head = NULL;
	struct wapi_mib_wtp_info *tail = NULL;
	struct wapi_mib_wtp_info *tmp = NULL;
	struct dcli_sta_info     *sta = NULL;
				
	int i,j,k,m;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STA_WAPI_MIB_INFO_OF_ALL_WTP);
		
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);	

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);

	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			if((tmp = (struct wapi_mib_wtp_info*)malloc(sizeof(struct wapi_mib_wtp_info))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);				
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			tmp->sta_list = NULL;
			tmp->sta_last = NULL;

			if(head == NULL)
				head = tmp;
			else 
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&tmp->WtpID);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[0]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[1]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[2]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[3]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[4]));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->mac[5]));		
			
			dbus_message_iter_next(&iter_wtp);	
			dbus_message_iter_get_basic(&iter_wtp,&(tmp->num_sta));

			dbus_message_iter_next(&iter_wtp);		
			dbus_message_iter_recurse(&iter_wtp,&iter_sta_array);

			//printf("tmp->num_sta = %d\n",tmp->num_sta);
			
			for(j=0; j<tmp->num_sta; j++) {
				if((sta = (struct dcli_sta_info*)malloc(sizeof(struct dcli_sta_info))) == NULL){
						*ret = ASD_DBUS_MALLOC_FAIL;
						dbus_message_unref(reply);
						dcli_sta_wapi_mib_info_free_wtp_list(head);
						return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(tmp->sta_list == NULL){
					tmp->sta_list = sta;
				}else{
					tmp->sta_last->next = sta;
				}
				tmp->sta_last = sta;
										
				dbus_message_iter_recurse(&iter_sta_array,&iter_sta);

				dbus_message_iter_get_basic(&iter_sta,&(sta->addr[0]));
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&(sta->addr[1]));
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&(sta->addr[2]));
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&(sta->addr[3]));
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&(sta->addr[4]));
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&(sta->addr[5]));

				for(k=0;k<16;k++){
					dbus_message_iter_next(&iter_sta);	
					dbus_message_iter_get_basic(&iter_sta,&(sta->BKIDUsed[k]));
				}					
				dbus_message_iter_next(&iter_sta);	
				dbus_message_iter_get_basic(&iter_sta,&(sta->ControlledPortStatus));			
								
				dbus_message_iter_next(&iter_sta_array);
			}
		dbus_message_iter_next(&iter_wtp_array);
		}
	}
	dbus_message_unref(reply);		
	return head;	
}



/*nl add 20100104*/
struct dcli_wtp_info* show_radio_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;	
	struct dcli_radio_info *radio = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
	
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;

		wtp->radio_last = NULL;
		wtp->radio_last = NULL;
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_radio));
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
				
		for(i=0; i<wtp->num_radio; i++) { 	
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			if((radio = (struct dcli_radio_info*)malloc(sizeof(*radio))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_radio(radio);
				return NULL;
			}
			memset(radio,0,sizeof(*radio));
			radio->next = NULL;
			if(wtp->radio_list == NULL){
				wtp->radio_list = radio;
			}else{
				wtp->radio_last->next = radio;
			}
			wtp->radio_last = radio;

			dbus_message_iter_get_basic(&iter_struct,&(radio->radioid));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->acc_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->repauth_tms));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_success_num));//5
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_fail_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->auth_others_num));//10
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->assoc_req_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->assoc_resp_num));
	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->assoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->assoc_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->assoc_refused_num));//15
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->assoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->reassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->reassoc_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->reassoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->reassoc_timeout_num));//20
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->reassoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->reassoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->identify_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->identify_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->abort_key_error_num));//25
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->abort_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->abort_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->abort_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->abort_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->deauth_request_num));//30
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->deauth_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->deauth_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->deauth_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->deauth_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->disassoc_request_num));//35
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->disassoc_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->disassoc_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->disassoc_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->disassoc_others_num));
    /*
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->rx_mgmt_pkts));//40
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->tx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->rx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->tx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->rx_data_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->tx_data_pkts));//45
	*/	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->tx_auth_pkts));//47

			dbus_message_iter_next(&iter_array);
		}
		
	}

	dbus_message_unref(reply);	
	//receive WID message in this function , mahz add 2011.1.21

	DBusMessage *query2, *reply2; 
	DBusError err2;
	DBusMessageIter	 iter_wid;
	DBusMessageIter	 iter_wid_array;	
	DBusMessageIter	 iter_wid_struct;	

	unsigned char radio_num = 0;
	unsigned int radio_id = 0;
	
	unsigned int  rx_data_pkts = 0;
	unsigned int  tx_data_pkts = 0;
	unsigned int  rx_mgmt_pkts = 0;
	unsigned int  tx_mgmt_pkts = 0;
	unsigned int  rx_ctrl_pkts = 0;
	unsigned int  tx_ctrl_pkts = 0;
	
	unsigned int ret2;
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RADIO_INFO_BYWTPID_WID);

	dbus_error_init(&err2);

	dbus_message_append_args(query2, DBUS_TYPE_UINT32,&wtpid, DBUS_TYPE_INVALID);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
	dbus_message_unref(query2);
	
	if (NULL == reply2) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}		
		printf("error in receive wid message\n");
		dcli_free_wtp(wtp);			
		return NULL;
	}

	dbus_message_iter_init(reply2,&iter_wid);
	dbus_message_iter_get_basic(&iter_wid,&ret2);
	
	
	if(ret2 == 0){
		dbus_message_iter_next(&iter_wid);
		dbus_message_iter_get_basic(&iter_wid,&(radio_num));
		
		dbus_message_iter_next(&iter_wid);
		dbus_message_iter_recurse(&iter_wid,&iter_wid_array);

		for(j=0;j<radio_num;j++){			
			//printf("j = %d\n",j);

			dbus_message_iter_recurse(&iter_wid_array,&iter_wid_struct);
			dbus_message_iter_get_basic(&iter_wid_struct,&(rx_data_pkts));
			
			dbus_message_iter_next(&iter_wid_struct);	
			dbus_message_iter_get_basic(&iter_wid_struct,&(tx_data_pkts));
					
			dbus_message_iter_next(&iter_wid_struct);	
			dbus_message_iter_get_basic(&iter_wid_struct,&(rx_mgmt_pkts));

			dbus_message_iter_next(&iter_wid_struct);	
			dbus_message_iter_get_basic(&iter_wid_struct,&(tx_mgmt_pkts));

			dbus_message_iter_next(&iter_wid_struct);	
			dbus_message_iter_get_basic(&iter_wid_struct,&(rx_ctrl_pkts));

			dbus_message_iter_next(&iter_wid_struct);	
			dbus_message_iter_get_basic(&iter_wid_struct,&(tx_ctrl_pkts));

			dbus_message_iter_next(&iter_wid_struct);
			dbus_message_iter_get_basic(&iter_wid_struct,&(radio_id));
			
			dbus_message_iter_next(&iter_wid_array);

			//printf("rx_data_pkts = %d\n",rx_data_pkts);
			//printf("radio_id = %d\n",radio_id);
					
			struct dcli_radio_info *radio_wid = NULL;
			if(wtp != NULL)
				radio_wid = wtp->radio_list;
					
			while(radio_wid != NULL){	
				//printf("radio_wid->radioid = %d\n",radio_wid->radioid);
				if(radio_id == radio_wid->radioid){					
					radio_wid->rx_data_pkts = rx_data_pkts;
					radio_wid->tx_data_pkts = tx_data_pkts;
					radio_wid->rx_mgmt_pkts = rx_mgmt_pkts;
					radio_wid->tx_mgmt_pkts = tx_mgmt_pkts;
					radio_wid->rx_ctrl_pkts = rx_ctrl_pkts;
					radio_wid->tx_ctrl_pkts = tx_ctrl_pkts;					
					break;
				}
				else{
					radio_wid = radio_wid->next;
				}
			}
		}
	}
	return wtp;
}



/*nl add 100107*/
struct dcli_wtp_info* show_wapi_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;
		wtp->radio_list = NULL;
		wtp->radio_last = NULL;
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->mac[0]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->mac[1]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->mac[2]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->mac[3]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->mac[4]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->mac[5]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->ap_wapi_version));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_sign_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_hmac_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_auth_res_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_discard));//5
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_timeout));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_format_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_cert_handshake_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_unicast_handshake_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wai_multi_handshake_fail));//10
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wpi_mic_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wpi_replay_counters));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->wpi_decryptable_errors));//13

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->num_bss));
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for(i=0; i<wtp->num_bss; i++){		
			if((bss = (struct dcli_bss_info*)malloc(sizeof(struct dcli_bss_info))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_wtp(wtp);
					return NULL;
				}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(wtp->bss_list == NULL){
				wtp->bss_list = bss;
			}else{
				wtp->bss_last->next = bss;
			}
			wtp->bss_last = bss;
			
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->WlanID));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->BSSIndex));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->mac[5]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->WapiEnabled));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_sign_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_hmac_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_auth_res_fail));
						
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_discard));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_timeout));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_format_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_cert_handshake_fail));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_unicast_handshake_fail));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wai_multi_handshake_fail));
						
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wpi_mic_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wpi_replay_counters));
	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->wpi_decryptable_errors));

			
			dbus_message_iter_next(&iter_array);
			}
		
		}
		
	dbus_message_unref(reply);		
	
	return wtp;
}

struct dcli_ac_info* show_traffic_limit_byradio(DBusConnection *dcli_dbus_connection, int index, unsigned int radioid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_ac_info  *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(ac,0,sizeof(*ac));
		ac->bss_list = NULL;
		ac->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_bss_wireless));
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<ac->num_bss_wireless; i++) {		
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 
			
			if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(bss,0,sizeof(*bss));
			bss->next = NULL;
			if(ac->bss_list == NULL){
				ac->bss_list = bss;
			}else{
				ac->bss_last->next = bss;
			}
			ac->bss_last = bss;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->bssid[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bss->traffic_limit));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(bss->send_traffic_limit));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->num_sta));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(j=0; j<bss->num_sta; j++){	
				DBusMessageIter iter_sub_struct;
				if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
					*ret = ASD_DBUS_MALLOC_FAIL;
					dbus_message_unref(reply);
					dcli_free_ac(ac);
					return NULL;
				}
				memset(sta,0,sizeof(*sta));
				sta->next = NULL;
				if(bss->sta_list == NULL){
					bss->sta_list = sta;
				}else{
					bss->sta_last->next = sta;
				}
				bss->sta_last = sta;
			
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[0]));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[1]));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[2]));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[3]));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[4]));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->addr[5]));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_traffic_limit));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->sta_send_traffic_limit));

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);

		}
	}
	dbus_message_unref(reply);
	
	return ac;
}

struct dcli_bss_info* show_traffic_limit_bybss(DBusConnection *dcli_dbus_connection, int index, unsigned int bssindex, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i,j;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&bssindex,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(bss,0,sizeof(*bss));
		bss->next = NULL;

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&bss->traffic_limit);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&bss->send_traffic_limit);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bss->num_sta);

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for(j=0; j<bss->num_sta; j++){	
			DBusMessageIter iter_struct;
			if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_bss(bss);
				return NULL;
			}
			memset(sta,0,sizeof(*sta));
			sta->next = NULL;
			if(bss->sta_list == NULL){
				bss->sta_list = sta;
			}else{
				bss->sta_last->next = sta;
			}
			bss->sta_last = sta;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[0]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[1]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[2]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[3]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[4]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->sta_traffic_limit));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->sta_send_traffic_limit));

			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	
	return bss;
}

struct dcli_wtp_info* show_wtp_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array; 	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	struct dcli_wtp_info *wtp = NULL;
	struct maclist  	 *tmp = NULL;
	struct maclist 		 *tail = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(wtp->acl_conf.macaddr_acl));	 //32bit or 8bit ?
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wtp->acl_conf.num_deny_mac));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wtp->acl_conf.num_accept_mac));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wtp(wtp);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wtp->acl_conf.deny_mac == NULL)
				wtp->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}

		for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wtp(wtp);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wtp->acl_conf.accept_mac== NULL)
				wtp->acl_conf.accept_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);
	
	return wtp;
}

//nl add 20100111
struct dcli_bss_info* show_radio_bss_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned int radio_id, unsigned char wlan_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array; 	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	struct dcli_bss_info *bss = NULL;
	struct maclist  	 *tmp = NULL;
	struct maclist 		 *tail = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_BSS_MAC_LIST);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(bss,0,sizeof(*bss));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(bss->acl_conf.macaddr_acl));	 //32bit or 8bit ?
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(bss->acl_conf.num_deny_mac));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(bss->acl_conf.num_accept_mac));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		for(j=0; j<bss->acl_conf.num_deny_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_bss(bss);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(bss->acl_conf.deny_mac == NULL)
				bss->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}

		for(j=0; j<bss->acl_conf.num_accept_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_bss(bss);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(bss->acl_conf.accept_mac== NULL)
				bss->acl_conf.accept_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);
	
	return bss;
}
//nl add 20100111
struct dcli_wlan_info* show_wlan_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array; 	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	struct dcli_wlan_info *wlan = NULL;
	struct maclist  	 *tmp = NULL;
	struct maclist 		 *tail = NULL;


	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlanid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){

		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(wlan,0,sizeof(*wlan));
		wlan->bss_list = NULL;
		wlan->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(wlan->acl_conf.macaddr_acl));	 //32bit or 16bit?
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wlan->acl_conf.num_deny_mac));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wlan->acl_conf.num_accept_mac));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);


		for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wlan(wlan);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wlan->acl_conf.deny_mac == NULL)
				wlan->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}

		for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wlan(wlan);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wlan->acl_conf.accept_mac== NULL)
				wlan->acl_conf.accept_mac = tmp;
			else
				tail->next = tmp;
			
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);
	
	return wlan;
}
struct dcli_ac_info* show_all_wtp_maclist(DBusConnection *dcli_dbus_connection, int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array;	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	
	struct dcli_ac_info  *ac = NULL;
	struct dcli_wtp_info *wtp = NULL;
	struct maclist		 *tmp = NULL;
	struct maclist		 *tail = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}else
		*ret = ASD_DBUS_SUCCESS;

	if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		dbus_message_unref(reply);
		return NULL;
	}
	memset(ac,0,sizeof(*ac));

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(ac->num_wtp));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<ac->num_wtp; i++){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			dcli_free_ac(ac);
			return NULL;
		}
		memset(wtp,0,sizeof(*wtp));
		if(ac->wtp_list == NULL){
			ac->wtp_list = wtp;
		}else{
			ac->wtp_last->next = wtp;
		}
		ac->wtp_last = wtp;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(wtp->WtpID));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(wtp->acl_conf.macaddr_acl));  //32bit or 8bit ?
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wtp->acl_conf.num_deny_mac));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wtp->acl_conf.num_accept_mac));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wtp->acl_conf.deny_mac == NULL)
				wtp->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}

		for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wtp->acl_conf.accept_mac== NULL)
				wtp->acl_conf.accept_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);
	
	return ac;
}

//nl add 20100111
struct dcli_ac_info* show_all_wlan_maclist(DBusConnection *dcli_dbus_connection, int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array;	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	unsigned char num_wlan;
	
	struct dcli_ac_info  *ac = NULL;
	struct dcli_wlan_info *wlan = NULL;
	struct maclist		 *tmp = NULL;
	struct maclist		 *tail = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}else
		*ret = ASD_DBUS_SUCCESS;

	if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		dbus_message_unref(reply);
		return NULL;
	}
	memset(ac,0,sizeof(*ac));

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&num_wlan);
	ac->num_wlan = (int)num_wlan ;
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<ac->num_wlan; i++){
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			dcli_free_ac(ac);
			return NULL;
		}
		memset(wlan,0,sizeof(*wlan));
		if(ac->wlan_list == NULL){
			ac->wlan_list = wlan;
		}else{
			ac->wlan_last->next = wlan;
		}
		ac->wlan_last = wlan;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(wlan->WlanID));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(wlan->acl_conf.macaddr_acl));  //32bit or 8bit ?
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wlan->acl_conf.num_deny_mac));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(wlan->acl_conf.num_accept_mac));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wlan->acl_conf.deny_mac == NULL)
				wlan->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}

		for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wlan->acl_conf.accept_mac== NULL)
				wlan->acl_conf.accept_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);
	
	return ac;
}

//nl 20100111
struct dcli_ac_info* show_all_bss_maclist(DBusConnection *dcli_dbus_connection, int index, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array;	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	unsigned char bss_index;
	unsigned char wlan_id;			//mahz add 2011.5.11
	
	struct dcli_ac_info  *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	struct maclist		 *tmp = NULL;
	struct maclist		 *tail = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}else
		*ret = ASD_DBUS_SUCCESS;

	if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		dbus_message_unref(reply);
		return NULL;
	}
	memset(ac,0,sizeof(*ac));

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(ac->num_bss));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<ac->num_bss; i++){
		if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			dcli_free_ac(ac);
			return NULL;
		}
		memset(bss,0,sizeof(*bss));
		if(ac->bss_list == NULL){
			ac->bss_list = bss;
		}else{
			ac->bss_last->next = bss;
		}
		ac->bss_last = bss;


		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(bss->Radio_G_ID));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&wlan_id);	

		bss->WlanID = wlan_id; 
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(bss->acl_conf.macaddr_acl));  //32bit or 8bit ?
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(bss->acl_conf.num_deny_mac));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(bss->acl_conf.num_accept_mac));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		
		for(j=0; j<bss->acl_conf.num_deny_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(bss->acl_conf.deny_mac == NULL)
				bss->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}

		for(j=0; j<bss->acl_conf.num_accept_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_ac(ac);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(bss->acl_conf.accept_mac== NULL)
				bss->acl_conf.accept_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);

	}
	dbus_message_unref(reply);
	
	return ac;
}


struct dcli_wlan_info* show_wlan_wids_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid, int localid, unsigned int *ret)
{
	DBusMessage 	*query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array;	
	DBusMessageIter  iter_sub_struct;
	DBusError	err;
	int i,j;
	struct dcli_wlan_info 	*wlan = NULL;
	struct maclist		 	*tmp = NULL;
	struct maclist		 	*tail = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(wlan,0,sizeof(*wlan));
		wlan->bss_list = NULL;
		wlan->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wlan->acl_conf.wids_set);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wlan->acl_conf.num_wids_mac);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wlan->acl_conf.wids_last_time);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(j=0; j<wlan->acl_conf.num_wids_mac; j++){
			if((tmp = (struct maclist*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_wlan(wlan);
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(wlan->acl_conf.deny_mac == NULL)
				wlan->acl_conf.deny_mac = tmp;
			else
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->addr[0]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->addr[1]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->addr[2]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->addr[3]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->addr[4]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->addr[5]));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->add_reason);	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->vapbssid[0]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->vapbssid[1]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->vapbssid[2]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->vapbssid[3]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->vapbssid[4]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->vapbssid[5]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->attacktype);	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->frametype);	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->channel); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->rssi);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->add_time);	
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_next(&iter_array);
		}

	}
	dbus_message_unref(reply);
	
	return wlan;
}

/*nl add 100107*/
struct dcli_channel_info* show_channel_access_time(DBusConnection *dcli_dbus_connection, int index, unsigned char *num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	int i ;

	struct dcli_channel_info *channel = NULL;
	
	struct dcli_channel_info *channel_node = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME);*/
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0 ){

		if((channel = (struct dcli_channel_info*)malloc(sizeof(struct dcli_channel_info))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		
		memset(channel,0,sizeof(*channel));
		channel->channel_list = NULL;
		channel->channel_last = NULL;
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *num; i++){

			if((channel_node = (struct dcli_channel_info*)malloc(sizeof(struct dcli_channel_info))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_channel(channel);
				return NULL;
					
			}
			
			memset(channel_node,0,sizeof(*channel_node));
			channel_node->next = NULL;
				
			if(channel->channel_list == NULL){
				channel->channel_list = channel_node;
			}else{
				channel->channel_last->next = channel_node;
				
			}
			channel->channel_last = channel_node;
						
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(channel_node->channel_id));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(channel_node->sta_num));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(channel_node->StaTime));
			//qiuchen add it
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(channel_node->statime_sysrun));
			//end
			
			dbus_message_iter_next(&iter_array);
				
		}

	}

			
	dbus_message_unref(reply);		
	return channel;
}

struct dcli_sta_info* check_sta_by_mac(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char type, unsigned int value, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError	err;
	struct dcli_sta_info* sta = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_CHECK_STA_BYMAC);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(*ret == 0){
		if((sta = (struct dcli_sta_info*)malloc(sizeof(*sta))) != NULL){
			memset(sta,0,sizeof(*sta));
			memcpy(sta->addr,mac,MAC_LEN);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wlan_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->radio_g_id));	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->bssindex)); 
		}
	}	
	
	dbus_message_unref(reply);
	
	return sta;
}


void set_sta_static_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char wlan_id, unsigned int radio_id, unsigned char type, unsigned int value, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError	err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_STA_STATIC_INFO);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_unref(reply);
	
	return ;
}

void del_sta_static_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN],unsigned char wlan_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError	err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_DEL_STA_STATIC_INFO);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_unref(reply);
	
	return ;
}

struct sta_static_info *show_sta_static_info_bymac(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN],unsigned char wlan_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError	err;
	struct sta_static_info *sta = NULL;
	struct sta_static_info *sta_table = NULL;
	struct sta_static_info *tail = NULL;
	unsigned int 	num = 0;
	unsigned int 	i = 0;
		
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SHOW_STA_STATIC_INFO_BYMAC);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
							 DBUS_TYPE_BYTE,&mac[5],
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&num);

	if(*ret == 0){
		for(i=0; i< num; i++){
			if((sta = (struct sta_static_info *)malloc(sizeof(*sta))) == NULL){
				*ret = ASD_DBUS_ERROR;
				dcli_free_static_sta_tab(sta_table);
				dbus_message_unref(reply);
				return NULL;		
			}
			memset(sta, 0, sizeof(*sta));
			if(sta_table == NULL)
				sta_table = sta;
			else
				tail->next = sta;
			tail = sta;
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[0]));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[1]));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[2]));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[3]));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[4]));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->addr[5]));

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->vlan_id));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_traffic_limit));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->sta_send_traffic_limit));

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(sta->wlan_id));
		}
	}

	dbus_message_unref(reply);
	
	return sta_table;
}


struct sta_static_info *show_sta_static_info(DBusConnection *dcli_dbus_connection,int index, unsigned int *num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter iter_array; 	
	DBusMessageIter iter_struct;
	DBusError	err;
	struct sta_static_info *sta_table = NULL;
	struct sta_static_info *sta = NULL;
	struct sta_static_info *tail = NULL;
	unsigned int i = 0;
		
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SHOW_STA_STATIC_INFO);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	if(*ret == 0){
		for(i=0; i< *num; i++){
			if((sta = (struct sta_static_info *)malloc(sizeof(*sta))) == NULL){
				*ret = ASD_DBUS_ERROR;
				dcli_free_static_sta_tab(sta_table);
				dbus_message_unref(reply);
				return NULL;		
			}
			memset(sta, 0, sizeof(*sta));
			if(sta_table == NULL)
				sta_table = sta;
			else
				tail->next = sta;
			tail = sta;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->addr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->vlan_id));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->sta_traffic_limit));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->sta_send_traffic_limit));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta->wlan_id));

			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);
	
	return sta_table;
}

struct dcli_bss_summary_info *show_bss_summary(DBusConnection *dcli_dbus_connection,int index,int localid, char type,unsigned int *ret,unsigned int *Total_bss_num,int id,int num)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j,circlenum;
	struct dcli_bss_summary_info *bss_summary_list = NULL;
	struct bss_summary_info *bss_list = NULL;
	int ID = 0;
	int NUM = num;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_BSS_METHOD_SHOW_BSS_SUMMARY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_UINT32,&id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(*ret == 0){
		bss_summary_list = (struct dcli_bss_summary_info*)malloc(NUM*sizeof(struct dcli_bss_summary_info));
		if(bss_summary_list == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(bss_summary_list,0,NUM*sizeof(struct dcli_bss_summary_info));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,Total_bss_num);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&circlenum);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0; i<circlenum; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
	
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ID));	
			if(type == 3){
				bss_summary_list[0].ID = ID;
				ID = 0;
			}
			else
				bss_summary_list[ID].ID = ID;
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss_summary_list[ID].local_bss_num));
	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			bss_summary_list[ID].bss_list = (struct bss_summary_info*)malloc(bss_summary_list[ID].local_bss_num*sizeof(struct bss_summary_info));
			if(bss_summary_list[ID].bss_list == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_bss_summary_list(bss_summary_list,NUM);
				return NULL;
			}
			memset(bss_summary_list[ID].bss_list,0,bss_summary_list[ID].local_bss_num*sizeof(struct bss_summary_info));
			for(j=0; j<bss_summary_list[ID].local_bss_num; j++){	
				DBusMessageIter iter_sub_struct;
				bss_list = &(bss_summary_list[ID].bss_list[j]);
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSID[0])); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSID[1])); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSID[2])); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSID[3])); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSID[4])); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSID[5])); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->WLANID)); 
	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->RGID));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->sta_num));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(bss_list->BSSINDEX));	
				
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);
	
		}
	
	}
	dbus_message_unref(reply);
	
	return bss_summary_list;
}


struct dcli_ac_info* show_roaming_sta_list(DBusConnection *dcli_dbus_connection,int index, int localid, char type ,unsigned int *ret,unsigned int *Total_roam_sta_num,int wtpnum)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError	err;
	int i,j,circlenum;
	struct dcli_r_sta_info *r_sta_group = NULL;
	struct r_sta_info *r_sta = NULL;
	int NUM = 0;
	int ID = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_ROAMING_STALIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if(type ==1)
			NUM = WLAN_NUM;
		else if(type == 2)
			NUM = 3;
		else if(type == 3)
			NUM = wtpnum;
		if((r_sta_group = (struct dcli_r_sta_info*)malloc(NUM*sizeof(struct dcli_r_sta_info))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(r_sta_group,0,NUM*sizeof(struct dcli_r_sta_info));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,Total_roam_sta_num);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&circlenum);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0; i<circlenum; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ID));	
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(r_sta_group[ID].roaming_sta_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			if((r_sta_group[ID].r_sta_list = (struct r_sta_info*)malloc(r_sta_group[ID].roaming_sta_num*sizeof(struct r_sta_info))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);
				dcli_free_r_sta_group(r_sta_group,NUM);
				return NULL;
			}
			memset(r_sta_group[ID].r_sta_list,0,r_sta_group[ID].roaming_sta_num*sizeof(struct r_sta_info));
			for(j=0; j<r_sta_group[ID].roaming_sta_num; j++){	
				DBusMessageIter iter_sub_struct;
				r_sta = &(r_sta_group[ID].r_sta_list[j]);
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->STA_MAC[0]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->STA_MAC[1]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->STA_MAC[2]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->STA_MAC[3]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->STA_MAC[4]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->STA_MAC[5]));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->r_type));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->preAPID));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(r_sta->curAPID));	
				
				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}

	}
	dbus_message_unref(reply);
	
	return r_sta_group;
}
int dcli_asd_set_sta_arp(int index,int localid, int is_add, char *ip, char*macAddr, char*ifname,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_STA_ARP);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_KICKSTA);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&is_add,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&macAddr,
						   	 DBUS_TYPE_STRING,&ifname,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);
	return ret;
}
//qiuchen add it 2012.10.31
void get_sysruntime(time_t *sysruntime){
	struct timespec *sysrunt = NULL;;
	sysrunt = (struct timespec *)malloc(sizeof(*sysrunt));
	if(sysrunt == NULL){
		printf("%s,%d malloc error.\n",__func__,__LINE__);
		return;
	}
	clock_gettime(CLOCK_MONOTONIC,sysrunt);
	*sysruntime = sysrunt->tv_sec;
	free(sysrunt);
	sysrunt = NULL;
}
#endif

