#ifdef _D_WCPSS_

#ifndef HAVE_SOCKLEN_T
#define HAVE_SOCKLEN_T
#endif

#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <time.h>     /*xm add*/
#include <sys/time.h> /*xm add*/

#include "config/wireless_config.h"
#include "command.h"
#include "dcli_ac.h"
#include "dcli_acl.h"
//#include "../dcli_main.h" wangchao changed
#include "dcli_main.h"
#include "dcli_wlan.h"
#include "dcli_sta.h"

#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "wid_ac.h"
#include "asd_sta.h"
#include "sysdef/npd_sysdef.h"
#include "bsd/bsdpub.h"

#define PRINTKMG(stream,variable)\
{ \
	double flux; \
	if( stream < OKB ){ \
		vty_out(vty,"%s:	%llu(B)\n",variable,stream); \
	} else if((stream >= OKB) && (stream < OMB)){ \
		flux = (double)stream/OKB; \
		vty_out(vty,"%s:	%.1f(KB)\n",variable,flux);  \
	} else if((stream >= OMB) && (stream < OGB)){ \
		flux = (double)stream/OMB; \
		vty_out(vty,"%s:	%.1f(MB)\n",variable,flux); \
	} else{ \
		flux = (double)stream/OGB; \
		vty_out(vty,"%s:	%.1f(GB)\n",variable,flux);	\
	} \
} 

struct cmd_node sta_node =
{
	STA_NODE,
	"%s(config-sta)# "
};

struct cmd_node hansi_sta_node =
{
	HANSI_STA_NODE,
	"%s(hansi-sta)# "
};

struct cmd_node local_hansi_sta_node =
{
	LOCAL_HANSI_STA_NODE,
	"%s(local-hansi-sta)# "
};

/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
char *dcli_u32ip2str(unsigned int u32_ipaddr)
{	
#if 1
	struct in_addr inaddr;

	inaddr.s_addr = u32_ipaddr;

	return inet_ntoa(inaddr);
#else
	int len = sizeof("255.255.255.255\0");
	
	memset(static_buffer, 0, len);
	snprintf(static_buffer, sizeof(static_buffer), "%u.%u:%u.%u", 
		((u32_ipaddr >> 24) & 0xff), ((u32_ipaddr >> 16) & 0xff),
		((u32_ipaddr >> 8) & 0xff), ((u32_ipaddr >> 0) & 0xff));
	
	return static_buffer;

#endif
}


#if 0
struct dcli_ac_info* show_sta_list(DBusConnection *dcli_dbus_connection,int index, unsigned int *ret)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
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
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STALIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			return NULL;
		}
		memset(ac,0,sizeof(*ac));
		ac->bss_list = NULL;
		ac->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_bss_wireless));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_assoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_reassoc));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ac->num_assoc_failure));
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
			
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			ac->num_sta += bss->num_sta;
			bss->WtpID = bss->Radio_G_ID/L_RADIO_NUM;
			bss->Radio_L_ID = bss->Radio_G_ID%L_RADIO_NUM;
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
				dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));
				if((sta->ip = (unsigned char *)malloc(strlen(in_addr)+1)) != NULL) {
					memset(sta->ip,0,strlen(in_addr)+1);
					memcpy(sta->ip,in_addr,strlen(in_addr));
				}

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->rxbytes));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta->txbytes));
				
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

struct dcli_wtp_info* show_sta_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
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

				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}
	}
	dbus_message_unref(reply);
	
	return wtp;
}

struct dcli_wlan_info* show_sta_bywlan(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
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

				dbus_message_iter_next(&iter_sub_array);
			}		
			dbus_message_iter_next(&iter_array);

		}
	}
	dbus_message_unref(reply);
	
	return wlan;
}

struct dcli_wlan_info* show_info_bywlan(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((wlan = (struct dcli_wlan_info*)malloc(sizeof(*wlan))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			return NULL;
		}
		memset(wlan,0,sizeof(*wlan));
		wlan->bss_list = NULL;
		wlan->bss_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->rx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->tx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->rx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->tx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wlan->assoc_resp_num));
		
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
	
	return wlan;
}

/*nl add 091229*/
struct dcli_wtp_info* show_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	

	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
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
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->rx_data_pkts));//45
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp->tx_data_pkts));
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
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->rx_data_pkts));//45
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->tx_data_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(bss->tx_auth_pkts));//48
			
			dbus_message_iter_next(&iter_array);
			}

	}
	dbus_message_unref(reply);	
	return wtp;
}

/*nl add 20100104*/
struct dcli_wtp_info* show_radio_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
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
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(radio->tx_auth_pkts));//47

			dbus_message_iter_next(&iter_array);
		}
		

	}

	
	dbus_message_unref(reply);	
	return wtp;
}



/*nl add 100107*/
struct dcli_wtp_info* show_wapi_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	

	if(*ret == 0){
		if((wtp = (struct dcli_wtp_info*)malloc(sizeof(*wtp))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
			return NULL;
		}
		
		memset(wtp,0,sizeof(*wtp));
		wtp->bss_list = NULL;
		wtp->bss_last = NULL;
	
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
		dbus_message_iter_get_basic(&iter,&(wtp->wai_hamc_errors));
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
			}
		
		}
		
	dbus_message_unref(reply);		
	return wtp;
}


struct dcli_ac_info* show_traffic_limit_byradio(DBusConnection *dcli_dbus_connection, int index, unsigned int radioid, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((ac = (struct dcli_ac_info*)malloc(sizeof(*ac))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
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

struct dcli_bss_info* show_traffic_limit_bybss(DBusConnection *dcli_dbus_connection, int index, unsigned int bssindex, unsigned int *ret)
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
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((bss = (struct dcli_bss_info*)malloc(sizeof(*bss))) == NULL){
			*ret = ASD_DBUS_MALLOC_FAIL;
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
#endif

/*xm0814*/
DEFUN(set_wlan_extern_balance_cmd_func,
		set_wlan_extern_balance_cmd,
		"set wlan WLANID extern balance (enable|disable)",
		"wlan configure\n"
		"extern balance \n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/


	unsigned char wlan_id = 0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	
	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e')){
		type=1;		
	}else if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd')){
		type=0;		
	}else{
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE);

	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else 
		vty_out(vty,"successful!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}



#if DCLI_NEW
DEFUN(show_traffic_limit_info_rd_cmd_func,
	  show_traffic_limit_info_rd_cmd,
	  "show traffic limit info by radioid RDID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "traffic limit information\n"
	  "traffic limit information\n"
	  "traffic limit information\n"
	  "by radioid\n"
	  "Search by radio id \n"
	  "radion id\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_ac_info 	*ac = NULL;
	struct dcli_bss_info	*bss = NULL;
	unsigned int radioid = 0;
	unsigned int ret = 0;
	int i,j;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	ret = parse2_int_ID((char*)argv[0], &radioid);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radioid >= G_RADIO_NUM || radioid < 1*L_RADIO_NUM){
		vty_out(vty,"<error> radio id invalid\n");
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_traffic_limit_byradio(dcli_dbus_connection, index, radioid, localid, &ret);
		if((ret == 0) && (ac != NULL)) {
			vty_out(vty,"BSS Infomation Under Radio %u\n",radioid);
			vty_out(vty,"============================================================\n\n");

			for(i=0; i<ac->num_bss_wireless; i++) {		
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;

				vty_out(vty,"------------------------------------------------------------\n");
				vty_out(vty,"BSSID: "MACSTR"\n",MAC2STR(bss->bssid));
				vty_out(vty,"bss uplink trffic limit:       %4u(Kbps)\n",bss->traffic_limit);
				vty_out(vty,"bss downlink trffic limit:     %4u(Kbps)\n",bss->send_traffic_limit);
				vty_out(vty,"sta num:                       %4u\n",bss->num_sta);
				vty_out(vty,"------------------------------------------------------------\n\n");

				for(j=0; j<bss->num_sta; j++){	
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;

					if(sta == NULL)
						break;

					vty_out(vty,"----------------------------------\n");
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"sta uplink traffic limit:      %4u\n",sta->sta_traffic_limit);
					vty_out(vty,"sta downlink traffic limit:    %4u\n",sta->sta_send_traffic_limit);
					vty_out(vty,"----------------------------------\n");
				}
			}
			vty_out(vty,"============================================================\n");
			dcli_free_ac(ac);
		}	
		else if (ret ==ASD_WTP_NOT_EXIST) 
			vty_out(vty,"<error> radio %d does not provide service or it maybe does not exist\n",radioid);
		else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
			vty_out(vty,"<error> input radio id should not be great than %d\n",G_RADIO_NUM-1);
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				ac = NULL;
				ac = show_traffic_limit_byradio(dcli_dbus_connection, profile, radioid, localid, &ret);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)) {
					vty_out(vty,"BSS Infomation Under Radio %u\n",radioid);
					vty_out(vty,"============================================================\n\n");

					bss = NULL;
					for(i=0; i<ac->num_bss_wireless; i++) {		
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;

						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"BSSID: "MACSTR"\n",MAC2STR(bss->bssid));
						vty_out(vty,"bss uplink trffic limit:       %4u(Kbps)\n",bss->traffic_limit);
						vty_out(vty,"bss downlink trffic limit:     %4u(Kbps)\n",bss->send_traffic_limit);
						vty_out(vty,"sta num:                       %4u\n",bss->num_sta);
						vty_out(vty,"------------------------------------------------------------\n\n");

						for(j=0; j<bss->num_sta; j++){	
							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;

							vty_out(vty,"----------------------------------\n");
							vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
							vty_out(vty,"sta uplink traffic limit:      %4u\n",sta->sta_traffic_limit);
							vty_out(vty,"sta downlink traffic limit:    %4u\n",sta->sta_send_traffic_limit);
							vty_out(vty,"----------------------------------\n");
						}
					}
					dcli_free_ac(ac);
				}	
				else if (ret ==ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> radio %d does not provide service or it maybe does not exist\n",radioid);
				else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input radio id should not be great than %d\n",G_RADIO_NUM-1);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				ac = NULL;
				ac = show_traffic_limit_byradio(dcli_dbus_connection, profile, radioid, localid, &ret);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)) {
					vty_out(vty,"BSS Infomation Under Radio %u\n",radioid);
					vty_out(vty,"============================================================\n\n");

					bss = NULL;
					for(i=0; i<ac->num_bss_wireless; i++) {		
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;

						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"BSSID: "MACSTR"\n",MAC2STR(bss->bssid));
						vty_out(vty,"bss uplink trffic limit:       %4u(Kbps)\n",bss->traffic_limit);
						vty_out(vty,"bss downlink trffic limit:     %4u(Kbps)\n",bss->send_traffic_limit);
						vty_out(vty,"sta num:                       %4u\n",bss->num_sta);
						vty_out(vty,"------------------------------------------------------------\n\n");

						for(j=0; j<bss->num_sta; j++){	
							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;

							vty_out(vty,"----------------------------------\n");
							vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
							vty_out(vty,"sta uplink traffic limit:      %4u\n",sta->sta_traffic_limit);
							vty_out(vty,"sta downlink traffic limit:    %4u\n",sta->sta_send_traffic_limit);
							vty_out(vty,"----------------------------------\n");
						}
					}
					dcli_free_ac(ac);
				}	
				else if (ret ==ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> radio %d does not provide service or it maybe does not exist\n",radioid);
				else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input radio id should not be great than %d\n",G_RADIO_NUM-1);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;
}
#else
DEFUN(show_traffic_limit_info_rd_cmd_func,
	  show_traffic_limit_info_rd_cmd,
	  "show traffic limit info by radioid RDID",
	  CONFIG_STR
	  "traffic limit information\n"
	  "Search by radio id \n"
	  "radion id\n"
	 )
{	
/*	xm0723*/

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	unsigned int radioid=0 , bssnum=0;
	int i=0 ,j=0,k=0;
	unsigned int  stanum=0, traffic_limit=0, sta_traffic_limit=0;
	unsigned int  send_traffic_limit=0, sta_send_traffic_limit=0;
	unsigned char mac[MAC_LEN];
	unsigned char bssid[MAC_LEN];

	
	ret = parse2_int_ID((char*)argv[0], &radioid);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radioid >= G_RADIO_NUM || radioid < 1*L_RADIO_NUM){
		vty_out(vty,"<error> radio id invalid\n");
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssnum);
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		vty_out(vty,"BSS Infomation Under Radio %u\n",radioid);
		vty_out(vty,"============================================================\n\n");

		for(i=0; i<bssnum; i++) {		

			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[0]));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(bssid[1]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[2]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[3]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[4]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(traffic_limit));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(send_traffic_limit));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(stanum));
			
			
			
			vty_out(vty,"------------------------------------------------------------\n");
			
			vty_out(vty,"BSSID:%02X:%02X:%02X:%02X:%02X:%02X\n",bssid[0],bssid[1],bssid[2]
															,bssid[3],bssid[4],bssid[5]);
			
			vty_out(vty,"bss uplink trffic limit:       %4u(Kbps)\n",traffic_limit);
			vty_out(vty,"bss downlink trffic limit:     %4u(Kbps)\n",send_traffic_limit);
			vty_out(vty,"sta num:                       %4u\n",stanum);
			
			vty_out(vty,"------------------------------------------------------------\n\n");

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(j=0; j<stanum; j++){	
				DBusMessageIter iter_sub_struct;
						
			
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

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta_traffic_limit));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta_send_traffic_limit));

				vty_out(vty,"----------------------------------\n");
				vty_out(vty,"STA MAC: %02X:",mac[0]);
				vty_out(vty,"%02X:",mac[1]);		
				vty_out(vty,"%02X:",mac[2]);
				vty_out(vty,"%02X:",mac[3]);
				vty_out(vty,"%02X:",mac[4]);
				vty_out(vty,"%02X\n",mac[5]);
				
				vty_out(vty,"sta uplink traffic limit:      %4u\n\n",sta_traffic_limit);
				vty_out(vty,"sta downlink traffic limit:    %4u\n\n",sta_send_traffic_limit);
				dbus_message_iter_next(&iter_sub_array);
			}
			
			dbus_message_iter_next(&iter_array);

		}
		vty_out(vty,"============================================================\n");

	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> radio %d does not provide service or it maybe does not exist\n",radioid);
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input radio id should not be great than %d\n",G_RADIO_NUM-1);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
DEFUN(show_traffic_limit_info_cmd_func,
	  show_traffic_limit_info_cmd,
	  "show traffic limit info by bssindex INDEX [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "traffic limit information\n"
	  "traffic limit information\n"
	  "traffic limit information\n"
	  "by bssindex\n"
	  "Search by bssindex\n"
	  "bssindex value\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_bss_info	*bss = NULL;
	struct dcli_sta_info 	*sta = NULL;
	unsigned int bssindex = 0;
	unsigned int ret = 0;
	int i;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	ret = parse2_int_ID((char*)argv[0], &bssindex);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown bssindex format\n");
		return CMD_SUCCESS;
	}	
	if(bssindex >= (G_RADIO_NUM+1)*L_BSS_NUM|| bssindex <1* L_RADIO_NUM*L_BSS_NUM){
		vty_out(vty,"<error> bssindex invalid\n");
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		bss = show_traffic_limit_bybss(dcli_dbus_connection, index, bssindex, localid, &ret);
		if((ret == 0) && (bss != NULL)){	
			vty_out(vty,"============================================================\n");
			vty_out(vty,"BSS  %u Infomation \n",bssindex);
			vty_out(vty,"BSS uplink traffic limit  %4u(Kbps)\n",bss->traffic_limit);
			vty_out(vty,"BSS downlink traffic limit  %4u(Kbps)\n",bss->send_traffic_limit);
			vty_out(vty,"============================================================\n\n");

			for(i=0; i<bss->num_sta; i++){	
				if(sta == NULL)
					sta = bss->sta_list;
				else 
					sta = sta->next;
			
				if(sta == NULL)
					break;

				vty_out(vty,"------------------------------------------------------------\n");
				vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
				vty_out(vty,"sta uplink traffic limit:          %4u(Kbps)\n",sta->sta_traffic_limit);
				vty_out(vty,"sta downlink traffic limit:        %4u(Kbps)\n",sta->sta_send_traffic_limit);
				vty_out(vty,"------------------------------------------------------------\n");
			}
			vty_out(vty,"============================================================\n");
			dcli_free_bss(bss);
		}
		else if (ret ==ASD_WTP_NOT_EXIST) 
			vty_out(vty,"<error> bssindex %d does not provide service or it maybe does not exist\n",bssindex);
		else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
			vty_out(vty,"<error> input bssindex  should not be great than %d\n",G_RADIO_NUM*L_BSS_NUM);
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				bss = show_traffic_limit_bybss(dcli_dbus_connection, profile, bssindex, localid, &ret);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (bss != NULL)){	
					vty_out(vty,"============================================================\n");
					vty_out(vty,"BSS  %u Infomation \n",bssindex);
					vty_out(vty,"BSS uplink traffic limit  %4u(Kbps)\n",bss->traffic_limit);
					vty_out(vty,"BSS downlink traffic limit  %4u(Kbps)\n",bss->send_traffic_limit);
					vty_out(vty,"============================================================\n\n");

					sta = NULL;
					for(i=0; i<bss->num_sta; i++){	
						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;
					
						if(sta == NULL)
							break;

						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
						vty_out(vty,"sta uplink traffic limit:          %4u(Kbps)\n",sta->sta_traffic_limit);
						vty_out(vty,"sta downlink traffic limit:        %4u(Kbps)\n",sta->sta_send_traffic_limit);
						vty_out(vty,"------------------------------------------------------------\n");
					}
					dcli_free_bss(bss);
				}
				else if (ret ==ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> bssindex %d does not provide service or it maybe does not exist\n",bssindex);
				else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input bssindex  should not be great than %d\n",G_RADIO_NUM*L_BSS_NUM);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				bss = show_traffic_limit_bybss(dcli_dbus_connection, profile, bssindex, localid, &ret);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (bss != NULL)){	
					vty_out(vty,"============================================================\n");
					vty_out(vty,"BSS  %u Infomation \n",bssindex);
					vty_out(vty,"BSS uplink traffic limit  %4u(Kbps)\n",bss->traffic_limit);
					vty_out(vty,"BSS downlink traffic limit  %4u(Kbps)\n",bss->send_traffic_limit);
					vty_out(vty,"============================================================\n\n");

					sta = NULL;
					for(i=0; i<bss->num_sta; i++){	
						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;
					
						if(sta == NULL)
							break;

						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
						vty_out(vty,"sta uplink traffic limit:          %4u(Kbps)\n",sta->sta_traffic_limit);
						vty_out(vty,"sta downlink traffic limit:        %4u(Kbps)\n",sta->sta_send_traffic_limit);
						vty_out(vty,"------------------------------------------------------------\n");
					}
					dcli_free_bss(bss);
				}
				else if (ret ==ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> bssindex %d does not provide service or it maybe does not exist\n",bssindex);
				else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input bssindex  should not be great than %d\n",G_RADIO_NUM*L_BSS_NUM);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;
}
#else
DEFUN(show_traffic_limit_info_cmd_func,
	  show_traffic_limit_info_cmd,
	  "show traffic limit info by bssindex INDEX",
	  CONFIG_STR
	  "traffic limit information\n"
	  "Search by bssindex \n"
	 )
{	
/*	xm0723*/

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i=0;
	unsigned int bssindex=0 ,traffic_limit=0, sta_traffic_limit=0, stanum=0;
	unsigned int send_traffic_limit=0, sta_send_traffic_limit=0;
	unsigned char mac[MAC_LEN];

	ret = parse2_int_ID((char*)argv[0], &bssindex);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown bssindex format\n");
		return CMD_SUCCESS;
	}	
	if(bssindex >= (G_RADIO_NUM+1)*L_BSS_NUM|| bssindex <1* L_RADIO_NUM*L_BSS_NUM){
		vty_out(vty,"<error> bssindex invalid\n");
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&bssindex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	


		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&traffic_limit);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&send_traffic_limit);


		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&stanum);

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);

		vty_out(vty,"============================================================\n");
		vty_out(vty,"BSS  %u Infomation \n",bssindex);
		vty_out(vty,"BSS uplink traffic limit  %4u(Kbps)\n",traffic_limit);
		vty_out(vty,"BSS downlink traffic limit  %4u(Kbps)\n",send_traffic_limit);
		vty_out(vty,"============================================================\n\n");

		for(i=0; i<stanum; i++) {		

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
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta_traffic_limit));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta_send_traffic_limit));
	
			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"sta MAC:%02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2]
															,mac[3],mac[4],mac[5]);
			vty_out(vty,"sta uplink traffic limit:          %4u(Kbps)\n",sta_traffic_limit);
			vty_out(vty,"sta downlink traffic limit:        %4u(Kbps)\n",sta_send_traffic_limit);
			
			vty_out(vty,"------------------------------------------------------------\n\n");

			dbus_message_iter_next(&iter_array);

		}

	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> bssindex %d does not provide service or it maybe does not exist\n",bssindex);
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input bssindex  should not be great than %d\n",G_RADIO_NUM*L_BSS_NUM);
	}

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif


/*xm0714*/
DEFUN(set_ac_flow_cmd_func,
		set_ac_flow_cmd,
		"set wlan WLANID flow (enable|disable)",
		"wlan configure\n"
		"flow compute\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/


	unsigned char wlan_id = 0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	
	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e')){
		type=1;		
	}else if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd')){
		type=2;		
	}else{
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_AC_FLOW);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_AC_FLOW);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else 
		vty_out(vty,"successful!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


#if DCLI_NEW	
DEFUN(show_wapi_mib_info_cmd_func,
	  show_wapi_mib_info_cmd,
	  "show wapi mib info bywtpid WTPID",
	  CONFIG_STR
	  "wapi mib information\n"
	  "Search by wtp id \n"
	  "wtp id\n"
	 )
{	
/*	xm0630*/
	 
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret = 0;
	int i=0 ,j=0 ;
	unsigned int wtpid=0 ;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	
	struct dcli_wtp_info *wtp = NULL;	
	struct dcli_bss_info *bss = NULL;

	
	ret = parse2_int_ID((char*)argv[0], &wtpid);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtpid >= WTP_NUM ){
		vty_out(vty,"<error> wtpid id invalid\n");
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	wtp = show_wapi_mib_info_bywtp(dcli_dbus_connection, index, wtpid, localid, &ret);	

	if((ret == 0)&&(wtp != NULL)){	

		vty_out(vty,"============================================================\n\n");

		for(i=0; i<wtp->num_bss; i++) {					
			if(bss == NULL)
				bss = wtp->bss_list;
			else 
				bss = bss->next;

			if(bss == NULL)
				break;
		
			vty_out(vty,"bss[%d] "MACSTR"\n",i+1,MAC2STR(bss->bssid));
			vty_out(vty,"wapiEnabled = %4u\n\n",bss->wapiEnabled);
			vty_out(vty,"------------------------------------------------------------\n");			
			vty_out(vty,"BSSID:%02X:%02X:%02X:%02X:%02X:%02X\n",bss->bssid[0],bss->bssid[1],bss->bssid[2]
															,bss->bssid[3],bss->bssid[4],bss->bssid[5]);
			if(bss->wapiEnabled== 1){
				vty_out(vty,"wapiEnabled:				%4u\n",bss->wapiEnabled);
				vty_out(vty,"ControlledAuthControl:		%4u\n",bss->ControlledAuthControl);
				vty_out(vty,"ControlledPortControl:		%4u\n",bss->ControlledPortControl);
				
				vty_out(vty,"CertificateUpdateCount:	%4llu\n",bss->CertificateUpdateCount);
				vty_out(vty,"MulticastUpdateCount:		%4llu\n",bss->MulticastUpdateCount);
				vty_out(vty,"UnicastUpdateCount:		%4llu\n",bss->UnicastUpdateCount);
			
				vty_out(vty,"AuthenticationSuite:		%4u\n",bss->AuthenticationSuite);
				vty_out(vty,"AuthSuiteSelected:			%4u\n",bss->AuthSuiteSelected);
				vty_out(vty,"WAPI sta num:				%4u\n",bss->num_sta);
				
			}else{
				vty_out(vty,"wapiEnabled:			%4u\n",bss->wapiEnabled);
				vty_out(vty,"WAPI sta num:			%4u\n",bss->num_sta);
				
			}
			vty_out(vty,"------------------------------------------------------------\n\n");
			
			
			for(j=0; j<bss->num_sta; j++){
				struct dcli_sta_info *sta = NULL;				
				if(sta == NULL)
					sta = bss->sta_list;				
				else 
					sta = sta->next;				
				if(sta == NULL)
					break;
			
				vty_out(vty,"----------------------------------\n");
				vty_out(vty,"STA MAC: %02X:",sta->addr[0]);
				vty_out(vty,"%02X:",sta->addr[1]);		
				vty_out(vty,"%02X:",sta->addr[2]);
				vty_out(vty,"%02X:",sta->addr[3]);
				vty_out(vty,"%02X:",sta->addr[4]);
				vty_out(vty,"%02X\n",sta->addr[5]);
				
				vty_out(vty,"UnicastRekeyTime:	%4u\n",sta->UnicastRekeyTime);
				vty_out(vty,"UnicastRekeyPackets:	%4u\n",sta->UnicastRekeyPackets);
				vty_out(vty,"MulticastRekeyTime:	%4u\n",sta->MulticastRekeyTime);
				vty_out(vty,"MulticastRekeyPackets:	%4u\n",sta->MulticastRekeyPackets);
				vty_out(vty,"ControlledPortStatus:	%4u\n",sta->ControlledPortStatus);
				vty_out(vty,"BKIDUsed:	%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n\n",
					sta->BKIDUsed[0],sta->BKIDUsed[1],sta->BKIDUsed[2],sta->BKIDUsed[3],
					sta->BKIDUsed[4],sta->BKIDUsed[5],sta->BKIDUsed[6],sta->BKIDUsed[7],
					sta->BKIDUsed[8],sta->BKIDUsed[9],sta->BKIDUsed[10],sta->BKIDUsed[11],
					sta->BKIDUsed[12],sta->BKIDUsed[13],sta->BKIDUsed[14],sta->BKIDUsed[15]);	/*	xm0626*/
				
			}
			
		}		
		vty_out(vty,"============================================================\n");		
	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtpid);
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input wtp id should not be great than %d\n",WTP_NUM-1-1);
	}
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
		}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	dcli_free_wtp(wtp);

	return CMD_SUCCESS;
		
}

#else
DEFUN(show_wapi_mib_info_cmd_func,
	  show_wapi_mib_info_cmd,
	  "show wapi mib info bywtpid WTPID",
	  CONFIG_STR
	  "wapi mib information\n"
	  "Search by wtp id \n"
	  "wtp id\n"
	 )
{	
/*	xm0630*/

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i=0 ,j=0,k=0;
	unsigned int wtpid=0 , bssnum=0 , stanum=0;
	unsigned char mac[MAC_LEN];
	unsigned char bssid[MAC_LEN];
	/*BSS info */
	unsigned char	ControlledAuthControl;		/*	是否启用鉴权(待定)*/
	unsigned char	ControlledPortControl;		/*	端口的控制类型*/
	unsigned char	wapiEnabled;				/*	是否启用WAPI*/
	
	unsigned long long	CertificateUpdateCount; 	/*	鉴权握手的重试次数*/
	unsigned long long	MulticastUpdateCount;		/*	MSK握手的重试次数*/
	unsigned long long	UnicastUpdateCount; 		/*	单播密钥握手的重试次数*/
	
	unsigned char	AuthenticationSuite;			/*	选择的AKM套件*/
	unsigned char	AuthSuiteSelected;				/*	选择的AKM*/

	/*STA info*/
	unsigned long	UnicastRekeyTime;		/*	单播密钥有效时间*/
	unsigned int	UnicastRekeyPackets;	/*	单播密钥有效的数据包数量*/
	
	unsigned long	MulticastRekeyTime; 	/*	组播密钥有效时间*/
	unsigned int	MulticastRekeyPackets;	/*	组播密钥有效的数据包数量*/
	
	unsigned char	ControlledPortStatus;	/*	鉴权控制端口的状态*/

	unsigned char	BKIDUsed[16];			/*	上一个使用的BK?ID*/
	
	ret = parse2_int_ID((char*)argv[0], &wtpid);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtpid >= WTP_NUM ){
		vty_out(vty,"<error> wtpid id invalid\n");
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
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
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssnum);
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		vty_out(vty,"WPAI Infomation Under WTP %u\n",wtpid);
		vty_out(vty,"============================================================\n\n");

		for(i=0; i<bssnum; i++) {		

			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[0]));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(bssid[1]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[2]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[3]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[4]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(bssid[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(ControlledAuthControl));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ControlledPortControl));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wapiEnabled));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(CertificateUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(MulticastUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(UnicastUpdateCount));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(AuthenticationSuite));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(AuthSuiteSelected));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(stanum));
			
			
			
			vty_out(vty,"------------------------------------------------------------\n");
			
			vty_out(vty,"BSSID:%02X:%02X:%02X:%02X:%02X:%02X\n",bssid[0],bssid[1],bssid[2]
															,bssid[3],bssid[4],bssid[5]);
			if(wapiEnabled==1){
				vty_out(vty,"wapiEnabled:				%4u\n",wapiEnabled);
				vty_out(vty,"ControlledAuthControl:		%4u\n",ControlledAuthControl);
				vty_out(vty,"ControlledPortControl:		%4u\n",ControlledPortControl);
				
				vty_out(vty,"CertificateUpdateCount:	%4llu\n",CertificateUpdateCount);
				vty_out(vty,"MulticastUpdateCount:		%4llu\n",MulticastUpdateCount);
				vty_out(vty,"UnicastUpdateCount:		%4llu\n",UnicastUpdateCount);
			
				vty_out(vty,"AuthenticationSuite:		%4u\n",AuthenticationSuite);
				vty_out(vty,"AuthSuiteSelected:			%4u\n",AuthSuiteSelected);
				vty_out(vty,"WAPI sta num:				%4u\n",stanum);
			}else{
				vty_out(vty,"wapiEnabled:			%4u\n",wapiEnabled);
				vty_out(vty,"WAPI sta num:			%4u\n",stanum);
			}
			vty_out(vty,"------------------------------------------------------------\n\n");

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(j=0; j<stanum; j++){	
				DBusMessageIter iter_sub_struct;
						
			
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

				for(k=0;k<16;k++){
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&(BKIDUsed[k]));
				}
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(UnicastRekeyTime));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(UnicastRekeyPackets));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(MulticastRekeyTime));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(MulticastRekeyPackets));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(ControlledPortStatus));

				vty_out(vty,"----------------------------------\n");
				vty_out(vty,"STA MAC: %02X:",mac[0]);
				vty_out(vty,"%02X:",mac[1]);		
				vty_out(vty,"%02X:",mac[2]);
				vty_out(vty,"%02X:",mac[3]);
				vty_out(vty,"%02X:",mac[4]);
				vty_out(vty,"%02X\n",mac[5]);
				
				vty_out(vty,"UnicastRekeyTime:	%4u\n",UnicastRekeyTime);
				vty_out(vty,"UnicastRekeyPackets:	%4u\n",UnicastRekeyPackets);
				vty_out(vty,"MulticastRekeyTime:	%4u\n",MulticastRekeyTime);
				vty_out(vty,"MulticastRekeyPackets:	%4u\n",MulticastRekeyPackets);
				vty_out(vty,"ControlledPortStatus:	%4u\n",ControlledPortStatus);
				vty_out(vty,"BKIDUsed:	%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n\n",
					BKIDUsed[0],BKIDUsed[1],BKIDUsed[2],BKIDUsed[3],
					BKIDUsed[4],BKIDUsed[5],BKIDUsed[6],BKIDUsed[7],
					BKIDUsed[8],BKIDUsed[9],BKIDUsed[10],BKIDUsed[11],
					BKIDUsed[12],BKIDUsed[13],BKIDUsed[14],BKIDUsed[15]);	/*	xm0626*/
				dbus_message_iter_next(&iter_sub_array);
			}
			
			dbus_message_iter_next(&iter_array);

		}
		vty_out(vty,"============================================================\n");

	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtpid);
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input wtp id should not be great than %d\n",WTP_NUM-1);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#endif



DEFUN(show_wapi_protocol_info_cmd_func,
	  show_wapi_protocol_info_cmd,
	  "show wapi protocol info",
	  CONFIG_STR
	  "mib information\n"
	 )
{
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

	vty_out(vty,"==============================================================================\n");
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
	vty_out(vty,"==============================================================================\n");
	
	return CMD_SUCCESS;
}

#if DCLI_NEW
DEFUN(show_wapi_info_cmd_func,
	  show_wapi_info_cmd,
	  "show wapi info bywtpid WTPID",
	  CONFIG_STR
	  "mib information\n"
	  "Search by wtp id \n"
	  "wtp id\n"
	 )
{	

	 
	unsigned int wtp_id = 0;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	wtp = show_wapi_info_bywtp(dcli_dbus_connection, index, wtp_id, localid, &ret);

	if((ret==0)&&(wtp!=NULL)){
		
		unsigned char mac[MAC_LEN];
		unsigned char wlan_id = 0, WapiEnabled = 0;
		//unsigned int bssnum = 0, wtp_id = 0;
		unsigned int BSSIndex = 0;	
		
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID: %u\n",wtp_id);
		vty_out(vty,"WTP MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",wtp->mac[0],wtp->mac[1],wtp->mac[2],wtp->mac[3],wtp->mac[4],wtp->mac[5]);
		vty_out(vty,"WTP WAPI VERSION: %u\n",wtp->ap_wapi_version);
		vty_out(vty,"---------------------------------\n");
		vty_out(vty,"Wai Sign Errors:               %u\t\t",wtp->wai_sign_errors);
		vty_out(vty,"Wai Hmac Errors:               %u\n",wtp->wai_hmac_errors);
		vty_out(vty,"Wai Auth Res Fail:             %u\t\t",wtp->wai_auth_res_fail);
		vty_out(vty,"Wai Discard Num:               %u\n",wtp->wai_discard);
		vty_out(vty,"Wai Timeout Num:               %u\t\t",wtp->wai_timeout);
		vty_out(vty,"Wai Format Errors:             %u\n",wtp->wai_format_errors);
		vty_out(vty,"Wai Cert Handshake Fail:       %u\t\t",wtp->wai_cert_handshake_fail);
		vty_out(vty,"Wai Unicast Handshake Fail:    %u\n",wtp->wai_unicast_handshake_fail);
		vty_out(vty,"Wai Multi Handshake Fail:      %u\t\t",wtp->wai_multi_handshake_fail);
		vty_out(vty,"Wpi Mic Errors:                %u\n",wtp->wpi_mic_errors);
		vty_out(vty,"Wpi Replay Counters:           %u\t\t",wtp->wpi_replay_counters);
		vty_out(vty,"Wpi Decryptable Errors:        %u\n",wtp->wpi_decryptable_errors);
		vty_out(vty,"==============================================================================\n");
		
		for(i=0;i<wtp->num_bss;i++){
			
			struct dcli_bss_info *bss = NULL;
			
			if(bss == NULL)
				bss = wtp->bss_list;
			else 
				bss = bss->next;

			if(bss == NULL)
				break;
			
			vty_out(vty,"------------------------------------------------------------------------------\n");
			vty_out(vty,"WLANID:%u\n",bss->WlanID);
			vty_out(vty,"BSSIndex:%u\n",bss->BSSIndex);
			vty_out(vty,"WapiEnabled:%s\n",((bss->WapiEnabled==1)?"Yes":"No"));
			vty_out(vty,"BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			vty_out(vty,"---------------------------------\n");
			vty_out(vty,"Wai Sign Errors:               %u\t\t",bss->wai_sign_errors);
			vty_out(vty,"Wai Hmac Errors:               %u\n",bss->wai_hmac_errors);
			vty_out(vty,"Wai Auth Res Fail:             %u\t\t",bss->wai_auth_res_fail);
			vty_out(vty,"Wai Discard Num:               %u\n",bss->wai_discard);
			vty_out(vty,"Wai Timeout Num:               %u\t\t",bss->wai_timeout);
			vty_out(vty,"Wai Format Errors:             %u\n",bss->wai_format_errors);
			vty_out(vty,"Wai Cert Handshake Fail:       %u\t\t",bss->wai_cert_handshake_fail);
			vty_out(vty,"Wai Unicast Handshake Fail:    %u\n",bss->wai_unicast_handshake_fail);
			vty_out(vty,"Wai Multi Handshake Fail:      %u\t\t",bss->wai_multi_handshake_fail);
			vty_out(vty,"Wpi Mic Errors:                %u\n",bss->wpi_mic_errors);
			vty_out(vty,"Wpi Replay Counters:           %u\t\t",bss->wpi_replay_counters);
			vty_out(vty,"Wpi Decryptable Errors:        %u\n",bss->wpi_decryptable_errors);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			}
		
		if(wtp->num_bss == 0){
			vty_out(vty,"WTP %d does not provide service\n",wtp_id);
		}
		
		vty_out(vty,"==============================================================================\n");
	}
		
	/*else if(wtp->num_bss == 0){
		vty_out(vty,"WTP %d does not provide service\n",wtp_id);
	}*/
	else if (ret == ASD_WTP_NOT_EXIST){
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}	
	
	dcli_free_wtp(wtp);

	return CMD_SUCCESS;
}

#else
DEFUN(show_wapi_info_cmd_func,
	  show_wapi_info_cmd,
	  "show wapi info bywtpid WTPID",
	  CONFIG_STR
	  "mib information\n"
	  "Search by wtp id \n"
	  "wtp id\n"
	 )
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i;

	unsigned char mac[MAC_LEN];
	unsigned char wlan_id = 0, WapiEnabled = 0;
	unsigned int bssnum = 0, wtp_id = 0, BSSIndex = 0;	
	
	unsigned int ap_wapi_version = 0;
	unsigned int wai_sign_errors = 0;
	unsigned int wai_hmac_errors = 0;
	unsigned int wai_auth_res_fail = 0;
	unsigned int wai_discard = 0;//5
	unsigned int wai_timeout = 0;
	unsigned int wai_format_errors = 0;
	unsigned int wai_cert_handshake_fail = 0;
	unsigned int wai_unicast_handshake_fail = 0;
	unsigned int wai_multi_handshake_fail = 0;//10
	unsigned int wpi_mic_errors = 0;
	unsigned int wpi_replay_counters = 0;
	unsigned int wpi_decryptable_errors = 0;//13

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac[0]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac[1]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac[2]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac[3]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac[4]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac[5]));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(ap_wapi_version));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_sign_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_hmac_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_auth_res_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_discard));//5
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_timeout));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_format_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_cert_handshake_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_unicast_handshake_fail));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wai_multi_handshake_fail));//10
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wpi_mic_errors));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wpi_replay_counters));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wpi_decryptable_errors));//13

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(bssnum));
		
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID: %u\n",wtp_id);
		vty_out(vty,"WTP MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		vty_out(vty,"WTP WAPI VERSION: %u\n",ap_wapi_version);
		vty_out(vty,"---------------------------------\n");
		vty_out(vty,"Wai Sign Errors:               %u\t\t",wai_sign_errors);
		vty_out(vty,"Wai Hmac Errors:               %u\n",wai_hmac_errors);
		vty_out(vty,"Wai Auth Res Fail:             %u\t\t",wai_auth_res_fail);
		vty_out(vty,"Wai Discard Num:               %u\n",wai_discard);
		vty_out(vty,"Wai Timeout Num:               %u\t\t",wai_timeout);
		vty_out(vty,"Wai Format Errors:             %u\n",wai_format_errors);
		vty_out(vty,"Wai Cert Handshake Fail:       %u\t\t",wai_cert_handshake_fail);
		vty_out(vty,"Wai Unicast Handshake Fail:    %u\n",wai_unicast_handshake_fail);
		vty_out(vty,"Wai Multi Handshake Fail:      %u\t\t",wai_multi_handshake_fail);
		vty_out(vty,"Wpi Mic Errors:                %u\n",wpi_mic_errors);
		vty_out(vty,"Wpi Replay Counters:           %u\t\t",wpi_replay_counters);
		vty_out(vty,"Wpi Decryptable Errors:        %u\n",wpi_decryptable_errors);
		vty_out(vty,"==============================================================================\n");

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0; i<bssnum; i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wlan_id));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(BSSIndex));

			dbus_message_iter_next(&iter_struct);	
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
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WapiEnabled));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_sign_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_hmac_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_auth_res_fail));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_discard));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_timeout));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_format_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_cert_handshake_fail));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_unicast_handshake_fail));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wai_multi_handshake_fail));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wpi_mic_errors));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wpi_replay_counters));
	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wpi_decryptable_errors));

			vty_out(vty,"------------------------------------------------------------------------------\n");
			vty_out(vty,"WLANID:%u\n",wlan_id);
			vty_out(vty,"BSSIndex:%u\n",BSSIndex);
			vty_out(vty,"WapiEnabled:%s\n",((WapiEnabled==1)?"Yes":"No"));
			vty_out(vty,"BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			vty_out(vty,"---------------------------------\n");
			vty_out(vty,"Wai Sign Errors:               %u\t\t",wai_sign_errors);
			vty_out(vty,"Wai Hmac Errors:               %u\n",wai_hmac_errors);
			vty_out(vty,"Wai Auth Res Fail:             %u\t\t",wai_auth_res_fail);
			vty_out(vty,"Wai Discard Num:               %u\n",wai_discard);
			vty_out(vty,"Wai Timeout Num:               %u\t\t",wai_timeout);
			vty_out(vty,"Wai Format Errors:             %u\n",wai_format_errors);
			vty_out(vty,"Wai Cert Handshake Fail:       %u\t\t",wai_cert_handshake_fail);
			vty_out(vty,"Wai Unicast Handshake Fail:    %u\n",wai_unicast_handshake_fail);
			vty_out(vty,"Wai Multi Handshake Fail:      %u\t\t",wai_multi_handshake_fail);
			vty_out(vty,"Wpi Mic Errors:                %u\n",wpi_mic_errors);
			vty_out(vty,"Wpi Replay Counters:           %u\t\t",wpi_replay_counters);
			vty_out(vty,"Wpi Decryptable Errors:        %u\n",wpi_decryptable_errors);
			vty_out(vty,"------------------------------------------------------------------------------\n");

			dbus_message_iter_next(&iter_array);
		}		
	vty_out(vty,"==============================================================================\n");

	if(bssnum == 0)
		vty_out(vty,"WTP %d does not provide service\n",wtp_id);
	}else if (ret == ASD_WTP_NOT_EXIST) 	
	{
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif


#if DCLI_NEW
DEFUN(show_mib_info_cmd_func,
	  show_mib_info_cmd,
	  "show mib info byradioid RDID",
	  CONFIG_STR
	  "mib information\n"
	  "Search by radio id \n"
	  "radion id\n"
	 )
{	
/*	xm0616*/
	
	 
	unsigned int radio_id = 0;
	unsigned int num_bss = 0;
	unsigned int ret = 0;
	int i;int localid = 1;int slot_id = HostSlotId;int index = 0;
	
	struct dcli_bss_info *bss = NULL;	
	struct dcli_radio_info *radio = NULL;

	ret = parse2_int_ID((char*)argv[0], &radio_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(radio_id >= G_RADIO_NUM || radio_id < 1*L_RADIO_NUM){
		vty_out(vty,"<error> radio id invalid\n");
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	radio = show_mib_info_byradio(dcli_dbus_connection, index, radio_id, localid, &ret);
	
	if((ret==0)&&(radio!=NULL)){
		
		radio->radioid = radio_id;

		vty_out(vty,"BSS Infomation Under Radio %u\n",radio_id);
		vty_out(vty,"============================================================\n\n");
			
		for(i=0;i<radio->num_bss;i++){
						
			if(bss == NULL)
				bss = radio->bss_list;
			else 
				bss = bss->next;
			
			if(bss == NULL)
				break;
			vty_out(vty,"BssNo:%u\n",i+1);
			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"Wireless Up Flow:                 %4llu(byte)\n",bss->wl_up_flow);
			vty_out(vty,"Wireless Down Flow:               %4llu(byte)\n",bss->wl_dw_flow);
			vty_out(vty,"Channel Down Packets:             %4llu\n",bss->ch_dw_pck);
			vty_out(vty,"Channel Down Loss Packets:        %4llu\n",bss->ch_dw_los_pck);
			vty_out(vty,"Channel Down MAC Error Packets:   %4llu\n",bss->ch_dw_mac_err_pck);
			
			vty_out(vty,"Channel Down Resend Packets:      %4llu\n",bss->ch_dw_resend_pck);
			
			vty_out(vty,"Channel Up Frames:                %4llu\n",bss->ch_up_frm);
			vty_out(vty,"Channel Down Frames:              %4llu\n",bss->ch_dw_frm);
			vty_out(vty,"Channel Down Error Frames:        %4llu\n",bss->ch_dw_err_frm);
			vty_out(vty,"Channel Down Loss Frames:         %4llu\n",bss->ch_dw_los_frm);
			vty_out(vty,"Channel Down Resend Frames:       %4llu\n",bss->ch_dw_resend_frm);

			vty_out(vty,"Channel UP Loss Frames:        	%4llu\n",bss->ch_up_los_frm);
			vty_out(vty,"Channel Up Resend Frames:         	%4llu\n",bss->ch_up_resend_frm);
			vty_out(vty,"Send Bytes:						%4llu\n",bss->send_bytes);
			
			vty_out(vty,"------------------------------------------------------------\n\n");


			}
		vty_out(vty,"============================================================\n");

		}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> radio %d does not provide service or it maybe does not exist\n",radio_id);
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input radio id should not be great than %d\n",G_RADIO_NUM-1);
	}	
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}	
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_radio(radio);
	return CMD_SUCCESS;
}


#else
DEFUN(show_mib_info_cmd_func,
	  show_mib_info_cmd,
	  "show mib info byradioid RDID",
	  CONFIG_STR
	  "mib information\n"
	  "Search by radio id \n"
	  "radion id\n"
	 )
{	
/*	xm0616*/

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i=0;
	unsigned int radioid=0 , bssnum=0;

	unsigned long wl_up_flow;			/*	无线上行端口的总流量*/
	unsigned long wl_dw_flow;			/*	无线下行端口的总流量*/
	   
	unsigned long ch_dw_pck;			/*	信道下行总的包数*/
	unsigned long ch_dw_los_pck;		/*	信道下行总的丢包数*/
	unsigned long ch_dw_mac_err_pck;	/*	信道下行总的MAC错包数*/
	unsigned long ch_dw_resend_pck;		/*	信道下行总的重传包数*/
	   
	unsigned long ch_up_frm;			/*	信道上行总的帧数*/
	unsigned long ch_dw_frm;			/*	信道下行总的帧数*/
	unsigned long ch_dw_err_frm;		/*	信道下行总的错帧数*/
	unsigned long ch_dw_los_frm;		/*	信道下行总的丢帧数*/
	unsigned long ch_dw_resend_frm;		/*	信道下行总的重传帧*/

	unsigned long ch_up_los_frm;		/*信道上行总的丢帧数*/
	unsigned long ch_up_resend_frm;

/*/信道上行总的重传帧数*/
	unsigned long long send_bytes;		/*发送的数据帧字节数*/
	
	ret = parse2_int_ID((char*)argv[0], &radioid);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radioid >= G_RADIO_NUM || radioid < 1*L_RADIO_NUM){
		vty_out(vty,"<error> radio id invalid\n");
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
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
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssnum);
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		vty_out(vty,"BSS Infomation Under Radio %u\n",radioid);
		vty_out(vty,"============================================================\n\n");

		for(i=0; i<bssnum; i++) {		

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wl_up_flow));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wl_dw_flow));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_pck));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_los_pck));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_mac_err_pck));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_resend_pck));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_up_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_err_frm));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_los_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_dw_resend_frm));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_up_los_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(ch_up_resend_frm));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(send_bytes));
	
			vty_out(vty,"BssNo:%u\n",i+1);
			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"Wireless Up Flow:                 %4u(byte)\n",wl_up_flow);
			vty_out(vty,"Wireless Down Flow:               %4u(byte)\n",wl_dw_flow);
			vty_out(vty,"Channel Down Packets:             %4u\n",ch_dw_pck);
			vty_out(vty,"Channel Down Loss Packets:        %4u\n",ch_dw_los_pck);
			vty_out(vty,"Channel Down MAC Error Packets:   %4u\n",ch_dw_mac_err_pck);
			
			vty_out(vty,"Channel Down Resend Packets:      %4u\n",ch_dw_resend_pck);
			
			vty_out(vty,"Channel Up Frames:                %4u\n",ch_up_frm);
			vty_out(vty,"Channel Down Frames:              %4u\n",ch_dw_frm);
			vty_out(vty,"Channel Down Error Frames:        %4u\n",ch_dw_err_frm);
			vty_out(vty,"Channel Down Loss Frames:         %4u\n",ch_dw_los_frm);
			vty_out(vty,"Channel Down Resend Frames:       %4u\n",ch_dw_resend_frm);

			vty_out(vty,"Channel UP Loss Frames:        	%4u\n",ch_up_los_frm);
			vty_out(vty,"Channel Up Resend Frames:         	%4u\n",ch_up_resend_frm);
			vty_out(vty,"Send Bytes:						%4llu\n",send_bytes);
			
			vty_out(vty,"------------------------------------------------------------\n\n");

			dbus_message_iter_next(&iter_array);

		}
		vty_out(vty,"============================================================\n");

	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> radio %d does not provide service or it maybe does not exist\n",radioid);
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input radio id should not be great than %d\n",G_RADIO_NUM-1);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#endif

#if DCLI_NEW
DEFUN(show_channel_access_time_cmd_func,
	  show_channel_access_time_cmd,
	  "show channel access time",
	  CONFIG_STR
	  "Channel access time information\n"
	 )
{
	 
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;	
	int ret;
	unsigned char i=0;	
	struct dcli_channel_info *channel = NULL;	
	struct dcli_channel_info *channel_node = NULL;
	
	unsigned char num ;
	int localid = 1;int slot_id = HostSlotId;int index = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	channel = show_channel_access_time(dcli_dbus_connection, index, &num, localid, &ret);
	
	if(ret == 0 ){
				
		vty_out(vty,"Channel access time list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-10s	%-15s	%-15s\n","Channel","Sta num","Access Time");
		
		for (i = 0; i < num; i++) {
			
			if(channel_node == NULL)
				channel_node = channel->channel_list;
			else 
				channel_node = channel_node->next;
			
			if(channel_node == NULL)
				break;
					
			int hour,min,sec;
			
			hour=(channel_node->StaTime)/3600;
			min=(channel_node->StaTime-hour*3600)/60;
			sec=(channel_node->StaTime-hour*3600)%60;

			vty_out(vty,"%-10u	%-15u\t %2d:%2d:%2d\n",channel_node->channel_id, channel_node->sta_num, hour,min,sec);
		}
		
		vty_out(vty,"==============================================================================\n");
	}
	
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
		}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
		
	dcli_free_channel(channel);
	
	return CMD_SUCCESS;	
}


#else
DEFUN(show_channel_access_time_cmd_func,
	  show_channel_access_time_cmd,
	  "show channel access time",
	  CONFIG_STR
	  "Channel access time information\n"
	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;	
	int ret;
	unsigned char i=0;
	unsigned char num;
	unsigned char channel;
	unsigned int sta_num;
	time_t StaTime;
	time_t statimesys;//qiuchen add it
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
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
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0 ){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"Channel access time list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-10s	%-15s	%-15s\n","Channel","Sta num","Access Time");
		
		for (i = 0; i < num; i++) {
			
			int hour,min,sec;
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(channel));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(sta_num));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(StaTime));
			//qiuchen add it
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(statimesys));
			//end
			
			dbus_message_iter_next(&iter_array);
			


			hour=StaTime/3600;
			min=(StaTime-hour*3600)/60;
			sec=(StaTime-hour*3600)%60;

			vty_out(vty,"%-10u	%-15u\t %2d:%2d:%2d\n",channel, sta_num, hour,min,sec);
		}
		
		vty_out(vty,"==============================================================================\n");
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	
}
#endif

#if DCLI_NEW
DEFUN(show_radio_info_cmd_func,
	  show_radio_info_cmd,
	  "show radio info bywtpid WTPID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD radio statistics information\n"
	  "radio stats info\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	

	 
	unsigned int wtp_id = 0;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;	
	struct dcli_radio_info *radio = NULL;

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}
	else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		wtp = show_radio_info_bywtp(dcli_dbus_connection, index, wtp_id, localid, &ret);
		
		if((ret==0)&&(wtp!=NULL)){
			vty_out(vty,"==============================================================================\n");		
			for(i=0;i<wtp->num_radio;i++){
				
				if(radio == NULL)
					radio = wtp->radio_list;
				else 
					radio = radio->next;

				if(radio == NULL)
					break;
							
			vty_out(vty,"Radio: 					%u\n",radio->radioid);
			vty_out(vty,"-----------------------------\n");
			vty_out(vty,"Access Times:			%u\t\t",radio->acc_tms);
			vty_out(vty,"User Request Times:		%u\n",radio->auth_tms);
			vty_out(vty,"Wtp Response Times:		%u\t\t",radio->repauth_tms);
			vty_out(vty,"Auth Success Times:		%u\n",radio->auth_success_num);
			vty_out(vty,"Auth Fail Times:		%u\t\t",radio->auth_fail_num);
			vty_out(vty,"Auth Invalid Times:		%u\n",radio->auth_invalid_num);
			vty_out(vty,"Auth Timeout Times:		%u\t\t",radio->auth_timeout_num);
			vty_out(vty,"Auth Refused Times:		%u\n",radio->auth_refused_num);
			vty_out(vty,"Auth Others Times: 		%u\t\t",radio->auth_others_num);
			vty_out(vty,"Assoc Request Times:		%u\n",radio->assoc_req_num);
			vty_out(vty,"Assoc Response Times:		%u\t\t",radio->assoc_resp_num);
			vty_out(vty,"Assoc Invalid Times:		%u\n",radio->assoc_invalid_num);
			vty_out(vty,"Assoc Timeout Times:		%u\t\t",radio->assoc_timeout_num);
			vty_out(vty,"Assoc Refused Times:		%u\n",radio->assoc_refused_num);
			vty_out(vty,"Assoc Others Times:		%u\t\t",radio->assoc_others_num);
			vty_out(vty,"Reassoc Request Times: 	\t%u\n",radio->reassoc_request_num);
			vty_out(vty,"Reassoc Success Times: 	\t%u\t\t",radio->reassoc_success_num);
			vty_out(vty,"Reassoc Invalid Times: 	\t%u\n",radio->reassoc_invalid_num);
			vty_out(vty,"Reassoc Timeout Times: 	\t%u\t\t",radio->reassoc_timeout_num);
			vty_out(vty,"Reassoc Refused Times: 	\t%u\n",radio->reassoc_refused_num);
			vty_out(vty,"Reassoc Others Times:		%u\t\t",radio->reassoc_others_num);
			vty_out(vty,"Identify Request Times:	\t%u\n",radio->identify_request_num);
			vty_out(vty,"Identify Success Times:	\t%u\t\t",radio->identify_success_num);
			vty_out(vty,"Abort Key Error Times: 	\t%u\n",radio->abort_key_error_num);
			vty_out(vty,"Abort Invalid Times:		%u\t\t",radio->abort_invalid_num);
			vty_out(vty,"Abort Timeout Times:		%u\n",radio->abort_timeout_num);
			vty_out(vty,"Abort Refused Times:		%u\t\t",radio->abort_refused_num);
			vty_out(vty,"Abort Others Times:		%u\n",radio->abort_others_num);
			vty_out(vty,"Deauth Request Times:		%u\t\t",radio->deauth_request_num);
			vty_out(vty,"Deauth User Leave Times:	%u\n",radio->deauth_user_leave_num);
			vty_out(vty,"Deauth Ap Unable Times:	\t%u\t\t",radio->deauth_ap_unable_num);
			vty_out(vty,"Deauth Abnormal Times: 	\t%u\n",radio->deauth_abnormal_num);
			vty_out(vty,"Deauth Others Times:		%u\t\t",radio->deauth_others_num);
			vty_out(vty,"Disassoc Request Times:	\t%u\n",radio->disassoc_request_num);
			vty_out(vty,"Disassoc User Leave Times: \t%u\t\t",radio->disassoc_user_leave_num);
			vty_out(vty,"Disassoc Ap Unable Times:	%u\n",radio->disassoc_ap_unable_num);
			vty_out(vty,"Disassoc Abnormal Times:	%u\t\t",radio->disassoc_abnormal_num);
			vty_out(vty,"Disassoc Others Times: 	\t%u\n",radio->disassoc_others_num);
			vty_out(vty,"Recv Mgmt Packets: 		%u\t\t",radio->rx_mgmt_pkts);
			vty_out(vty,"Send Mgmt Packets: 		%u\n",radio->tx_mgmt_pkts);
			vty_out(vty,"Recv Ctrl Packets: 		%u\t\t",radio->rx_ctrl_pkts);
			vty_out(vty,"Send Ctrl Packets: 		%u\n",radio->tx_ctrl_pkts);
			vty_out(vty,"Recv Data Packets: 		%u\t\t",radio->rx_data_pkts);
			vty_out(vty,"Send Data Packets: 		%u\n",radio->tx_data_pkts);
			vty_out(vty,"Recv Auth Packets: 		%u\t\t",radio->rx_auth_pkts);
			vty_out(vty,"Send Auth Packets: 		%u\n",radio->tx_auth_pkts);

			vty_out(vty,"------------------------------------------------------------\n");			
			}	
			
			vty_out(vty,"==============================================================================\n");
		}

		else if (ret == ASD_WTP_NOT_EXIST) {
			vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
		}
		else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		}
		else if (ret == ASD_DBUS_ERROR){
			cli_syslog_info("<error> failed get reply.\n");
			}
		else{
			vty_out(vty,"<error> ret = %d\n",ret);
		}
		dcli_free_wtp(wtp);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				wtp = show_radio_info_bywtp(dcli_dbus_connection, profile, wtp_id, localid, &ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret==0)&&(wtp!=NULL)){
					radio = NULL;
					for(i=0;i<wtp->num_radio;i++){
						
						if(radio == NULL)
							radio = wtp->radio_list;
						else 
							radio = radio->next;

						if(radio == NULL)
							break;
									
					vty_out(vty,"Radio: 					%u\n",radio->radioid);
					vty_out(vty,"-----------------------------\n");
					vty_out(vty,"Access Times:			%u\t\t",radio->acc_tms);
					vty_out(vty,"User Request Times:		%u\n",radio->auth_tms);
					vty_out(vty,"Wtp Response Times:		%u\t\t",radio->repauth_tms);
					vty_out(vty,"Auth Success Times:		%u\n",radio->auth_success_num);
					vty_out(vty,"Auth Fail Times:		%u\t\t",radio->auth_fail_num);
					vty_out(vty,"Auth Invalid Times:		%u\n",radio->auth_invalid_num);
					vty_out(vty,"Auth Timeout Times:		%u\t\t",radio->auth_timeout_num);
					vty_out(vty,"Auth Refused Times:		%u\n",radio->auth_refused_num);
					vty_out(vty,"Auth Others Times: 		%u\t\t",radio->auth_others_num);
					vty_out(vty,"Assoc Request Times:		%u\n",radio->assoc_req_num);
					vty_out(vty,"Assoc Response Times:		%u\t\t",radio->assoc_resp_num);
					vty_out(vty,"Assoc Invalid Times:		%u\n",radio->assoc_invalid_num);
					vty_out(vty,"Assoc Timeout Times:		%u\t\t",radio->assoc_timeout_num);
					vty_out(vty,"Assoc Refused Times:		%u\n",radio->assoc_refused_num);
					vty_out(vty,"Assoc Others Times:		%u\t\t",radio->assoc_others_num);
					vty_out(vty,"Reassoc Request Times: 	\t%u\n",radio->reassoc_request_num);
					vty_out(vty,"Reassoc Success Times: 	\t%u\t\t",radio->reassoc_success_num);
					vty_out(vty,"Reassoc Invalid Times: 	\t%u\n",radio->reassoc_invalid_num);
					vty_out(vty,"Reassoc Timeout Times: 	\t%u\t\t",radio->reassoc_timeout_num);
					vty_out(vty,"Reassoc Refused Times: 	\t%u\n",radio->reassoc_refused_num);
					vty_out(vty,"Reassoc Others Times:		%u\t\t",radio->reassoc_others_num);
					vty_out(vty,"Identify Request Times:	\t%u\n",radio->identify_request_num);
					vty_out(vty,"Identify Success Times:	\t%u\t\t",radio->identify_success_num);
					vty_out(vty,"Abort Key Error Times: 	\t%u\n",radio->abort_key_error_num);
					vty_out(vty,"Abort Invalid Times:		%u\t\t",radio->abort_invalid_num);
					vty_out(vty,"Abort Timeout Times:		%u\n",radio->abort_timeout_num);
					vty_out(vty,"Abort Refused Times:		%u\t\t",radio->abort_refused_num);
					vty_out(vty,"Abort Others Times:		%u\n",radio->abort_others_num);
					vty_out(vty,"Deauth Request Times:		%u\t\t",radio->deauth_request_num);
					vty_out(vty,"Deauth User Leave Times:	%u\n",radio->deauth_user_leave_num);
					vty_out(vty,"Deauth Ap Unable Times:	\t%u\t\t",radio->deauth_ap_unable_num);
					vty_out(vty,"Deauth Abnormal Times: 	\t%u\n",radio->deauth_abnormal_num);
					vty_out(vty,"Deauth Others Times:		%u\t\t",radio->deauth_others_num);
					vty_out(vty,"Disassoc Request Times:	\t%u\n",radio->disassoc_request_num);
					vty_out(vty,"Disassoc User Leave Times: \t%u\t\t",radio->disassoc_user_leave_num);
					vty_out(vty,"Disassoc Ap Unable Times:	%u\n",radio->disassoc_ap_unable_num);
					vty_out(vty,"Disassoc Abnormal Times:	%u\t\t",radio->disassoc_abnormal_num);
					vty_out(vty,"Disassoc Others Times: 	\t%u\n",radio->disassoc_others_num);
					vty_out(vty,"Recv Mgmt Packets: 		%u\t\t",radio->rx_mgmt_pkts);
					vty_out(vty,"Send Mgmt Packets: 		%u\n",radio->tx_mgmt_pkts);
					vty_out(vty,"Recv Ctrl Packets: 		%u\t\t",radio->rx_ctrl_pkts);
					vty_out(vty,"Send Ctrl Packets: 		%u\n",radio->tx_ctrl_pkts);
					vty_out(vty,"Recv Data Packets: 		%u\t\t",radio->rx_data_pkts);
					vty_out(vty,"Send Data Packets: 		%u\n",radio->tx_data_pkts);
					vty_out(vty,"Recv Auth Packets: 		%u\t\t",radio->rx_auth_pkts);
					vty_out(vty,"Send Auth Packets: 		%u\n",radio->tx_auth_pkts);

					vty_out(vty,"------------------------------------------------------------\n");			
					}	
					
				}

				else if (ret == ASD_WTP_NOT_EXIST) {
					vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
				}
				else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				else if (ret == ASD_DBUS_ERROR){
					cli_syslog_info("<error> failed get reply.\n");
					}
				else{
					vty_out(vty,"<error> ret = %d\n",ret);
				}
				dcli_free_wtp(wtp);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
		
	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			wtp = show_radio_info_bywtp(dcli_dbus_connection, profile, wtp_id, localid, &ret);
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret==0)&&(wtp!=NULL)){
				radio = NULL;
				for(i=0;i<wtp->num_radio;i++){
					
					if(radio == NULL)
						radio = wtp->radio_list;
					else 
						radio = radio->next;

					if(radio == NULL)
						break;
								
				vty_out(vty,"Radio: 					%u\n",radio->radioid);
				vty_out(vty,"-----------------------------\n");
				vty_out(vty,"Access Times:			%u\t\t",radio->acc_tms);
				vty_out(vty,"User Request Times:		%u\n",radio->auth_tms);
				vty_out(vty,"Wtp Response Times:		%u\t\t",radio->repauth_tms);
				vty_out(vty,"Auth Success Times:		%u\n",radio->auth_success_num);
				vty_out(vty,"Auth Fail Times:		%u\t\t",radio->auth_fail_num);
				vty_out(vty,"Auth Invalid Times:		%u\n",radio->auth_invalid_num);
				vty_out(vty,"Auth Timeout Times:		%u\t\t",radio->auth_timeout_num);
				vty_out(vty,"Auth Refused Times:		%u\n",radio->auth_refused_num);
				vty_out(vty,"Auth Others Times: 		%u\t\t",radio->auth_others_num);
				vty_out(vty,"Assoc Request Times:		%u\n",radio->assoc_req_num);
				vty_out(vty,"Assoc Response Times:		%u\t\t",radio->assoc_resp_num);
				vty_out(vty,"Assoc Invalid Times:		%u\n",radio->assoc_invalid_num);
				vty_out(vty,"Assoc Timeout Times:		%u\t\t",radio->assoc_timeout_num);
				vty_out(vty,"Assoc Refused Times:		%u\n",radio->assoc_refused_num);
				vty_out(vty,"Assoc Others Times:		%u\t\t",radio->assoc_others_num);
				vty_out(vty,"Reassoc Request Times: 	\t%u\n",radio->reassoc_request_num);
				vty_out(vty,"Reassoc Success Times: 	\t%u\t\t",radio->reassoc_success_num);
				vty_out(vty,"Reassoc Invalid Times: 	\t%u\n",radio->reassoc_invalid_num);
				vty_out(vty,"Reassoc Timeout Times: 	\t%u\t\t",radio->reassoc_timeout_num);
				vty_out(vty,"Reassoc Refused Times: 	\t%u\n",radio->reassoc_refused_num);
				vty_out(vty,"Reassoc Others Times:		%u\t\t",radio->reassoc_others_num);
				vty_out(vty,"Identify Request Times:	\t%u\n",radio->identify_request_num);
				vty_out(vty,"Identify Success Times:	\t%u\t\t",radio->identify_success_num);
				vty_out(vty,"Abort Key Error Times: 	\t%u\n",radio->abort_key_error_num);
				vty_out(vty,"Abort Invalid Times:		%u\t\t",radio->abort_invalid_num);
				vty_out(vty,"Abort Timeout Times:		%u\n",radio->abort_timeout_num);
				vty_out(vty,"Abort Refused Times:		%u\t\t",radio->abort_refused_num);
				vty_out(vty,"Abort Others Times:		%u\n",radio->abort_others_num);
				vty_out(vty,"Deauth Request Times:		%u\t\t",radio->deauth_request_num);
				vty_out(vty,"Deauth User Leave Times:	%u\n",radio->deauth_user_leave_num);
				vty_out(vty,"Deauth Ap Unable Times:	\t%u\t\t",radio->deauth_ap_unable_num);
				vty_out(vty,"Deauth Abnormal Times: 	\t%u\n",radio->deauth_abnormal_num);
				vty_out(vty,"Deauth Others Times:		%u\t\t",radio->deauth_others_num);
				vty_out(vty,"Disassoc Request Times:	\t%u\n",radio->disassoc_request_num);
				vty_out(vty,"Disassoc User Leave Times: \t%u\t\t",radio->disassoc_user_leave_num);
				vty_out(vty,"Disassoc Ap Unable Times:	%u\n",radio->disassoc_ap_unable_num);
				vty_out(vty,"Disassoc Abnormal Times:	%u\t\t",radio->disassoc_abnormal_num);
				vty_out(vty,"Disassoc Others Times: 	\t%u\n",radio->disassoc_others_num);
				vty_out(vty,"Recv Mgmt Packets: 		%u\t\t",radio->rx_mgmt_pkts);
				vty_out(vty,"Send Mgmt Packets: 		%u\n",radio->tx_mgmt_pkts);
				vty_out(vty,"Recv Ctrl Packets: 		%u\t\t",radio->rx_ctrl_pkts);
				vty_out(vty,"Send Ctrl Packets: 		%u\n",radio->tx_ctrl_pkts);
				vty_out(vty,"Recv Data Packets: 		%u\t\t",radio->rx_data_pkts);
				vty_out(vty,"Send Data Packets: 		%u\n",radio->tx_data_pkts);
				vty_out(vty,"Recv Auth Packets: 		%u\t\t",radio->rx_auth_pkts);
				vty_out(vty,"Send Auth Packets: 		%u\n",radio->tx_auth_pkts);

				vty_out(vty,"------------------------------------------------------------\n");			
				}	
				
			}

			else if (ret == ASD_WTP_NOT_EXIST) {
				vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
			}
			else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
				vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
			}
			else if (ret == ASD_DBUS_ERROR){
				cli_syslog_info("<error> failed get reply.\n");
				}
			else{
				vty_out(vty,"<error> ret = %d\n",ret);
			}
			dcli_free_wtp(wtp);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;
}




#else
DEFUN(show_radio_info_cmd_func,
	  show_radio_info_cmd,
	  "show radio info bywtpid WTPID",
	  CONFIG_STR
	  "ASD radio statistics information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	 )
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned int radionum = 0;
	unsigned int radioid = 0;
	unsigned int wtp_id = 0;	
	unsigned int acc_tms=0,auth_tms=0,repauth_tms=0;
	unsigned int auth_success_num = 0; 
	unsigned int auth_fail_num = 0; 
	unsigned int auth_invalid_num = 0; 
	unsigned int auth_timeout_num = 0; 
	unsigned int auth_refused_num = 0; 
	unsigned int auth_others_num = 0; 
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 
	unsigned int assoc_invalid_num = 0; 
	unsigned int assoc_timeout_num = 0; 
	unsigned int assoc_refused_num = 0; 
	unsigned int assoc_others_num = 0; 
	unsigned int reassoc_request_num = 0; 
	unsigned int reassoc_success_num = 0; 
	unsigned int reassoc_invalid_num = 0; 
	unsigned int reassoc_timeout_num = 0; 
	unsigned int reassoc_refused_num = 0; 
	unsigned int reassoc_others_num = 0; 
	unsigned int identify_request_num = 0; 
	unsigned int identify_success_num = 0; 
	unsigned int abort_key_error_num = 0; 
	unsigned int abort_invalid_num = 0; 
	unsigned int abort_timeout_num = 0; 
	unsigned int abort_refused_num = 0; 
	unsigned int abort_others_num = 0; 
	unsigned int deauth_request_num = 0; 
	unsigned int deauth_user_leave_num = 0; 
	unsigned int deauth_ap_unable_num = 0; 
	unsigned int deauth_abnormal_num = 0; 
	unsigned int deauth_others_num = 0; 
	unsigned int disassoc_request_num = 0; 
	unsigned int disassoc_user_leave_num = 0; 
	unsigned int disassoc_ap_unable_num = 0; 
	unsigned int disassoc_abnormal_num = 0; 
	unsigned int disassoc_others_num = 0; 

	unsigned int rx_mgmt_pkts = 0;
	unsigned int tx_mgmt_pkts = 0;
	unsigned int rx_ctrl_pkts = 0;
	unsigned int tx_ctrl_pkts = 0;
	unsigned int rx_data_pkts = 0;
	unsigned int tx_data_pkts = 0;
	unsigned int rx_auth_pkts = 0;
	unsigned int tx_auth_pkts = 0;

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radionum));
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"==============================================================================\n");
		for(i=0; i<radionum; i++) {		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(radioid));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(acc_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(repauth_tms));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_fail_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_req_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_resp_num));
	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(identify_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(identify_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_key_error_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_others_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_data_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_data_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_auth_pkts));


			vty_out(vty,"Radio:                     %u\n",radioid);
			vty_out(vty,"-----------------------------\n");
			vty_out(vty,"Access Times:              %u\t\t",acc_tms);
			vty_out(vty,"User Request Times:        %u\n",auth_tms);
			vty_out(vty,"Wtp Response Times:        %u\t\t",repauth_tms);
			vty_out(vty,"Auth Success Times:        %u\n",auth_success_num);
			vty_out(vty,"Auth Fail Times:           %u\t\t",auth_fail_num);
			vty_out(vty,"Auth Invalid Times:        %u\n",auth_invalid_num);
			vty_out(vty,"Auth Timeout Times:        %u\t\t",auth_timeout_num);
			vty_out(vty,"Auth Refused Times:        %u\n",auth_refused_num);
			vty_out(vty,"Auth Others Times:         %u\t\t",auth_others_num);
			vty_out(vty,"Assoc Request Times:       %u\n",assoc_req_num);
			vty_out(vty,"Assoc Response Times:      %u\t\t",assoc_resp_num);
			vty_out(vty,"Assoc Invalid Times:       %u\n",assoc_invalid_num);
			vty_out(vty,"Assoc Timeout Times:       %u\t\t",assoc_timeout_num);
			vty_out(vty,"Assoc Refused Times:       %u\n",assoc_refused_num);
			vty_out(vty,"Assoc Others Times:        %u\t\t",assoc_others_num);
			vty_out(vty,"Reassoc Request Times:     %u\n",reassoc_request_num);
			vty_out(vty,"Reassoc Success Times:     %u\t\t",reassoc_success_num);
			vty_out(vty,"Reassoc Invalid Times:     %u\n",reassoc_invalid_num);
			vty_out(vty,"Reassoc Timeout Times:     %u\t\t",reassoc_timeout_num);
			vty_out(vty,"Reassoc Refused Times:     %u\n",reassoc_refused_num);
			vty_out(vty,"Reassoc Others Times:      %u\t\t",reassoc_others_num);
			vty_out(vty,"Identify Request Times:    %u\n",identify_request_num);
			vty_out(vty,"Identify Success Times:    %u\t\t",identify_success_num);
			vty_out(vty,"Abort Key Error Times:     %u\n",abort_key_error_num);
			vty_out(vty,"Abort Invalid Times:       %u\t\t",abort_invalid_num);
			vty_out(vty,"Abort Timeout Times:       %u\n",abort_timeout_num);
			vty_out(vty,"Abort Refused Times:       %u\t\t",abort_refused_num);
			vty_out(vty,"Abort Others Times:        %u\n",abort_others_num);
			vty_out(vty,"Deauth Request Times:      %u\t\t",deauth_request_num);
			vty_out(vty,"Deauth User Leave Times:   %u\n",deauth_user_leave_num);
			vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",deauth_ap_unable_num);
			vty_out(vty,"Deauth Abnormal Times:     %u\n",deauth_abnormal_num);
			vty_out(vty,"Deauth Others Times:       %u\t\t",deauth_others_num);
			vty_out(vty,"Disassoc Request Times:    %u\n",disassoc_request_num);
			vty_out(vty,"Disassoc User Leave Times: %u\t\t",disassoc_user_leave_num);
			vty_out(vty,"Disassoc Ap Unable Times:  %u\n",disassoc_ap_unable_num);
			vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",disassoc_abnormal_num);
			vty_out(vty,"Disassoc Others Times:     %u\n",disassoc_others_num);
			vty_out(vty,"Recv Mgmt Packets:         %u\t\t",rx_mgmt_pkts);
			vty_out(vty,"Send Mgmt Packets:         %u\n",tx_mgmt_pkts);
			vty_out(vty,"Recv Ctrl Packets:         %u\t\t",rx_ctrl_pkts);
			vty_out(vty,"Send Ctrl Packets:         %u\n",tx_ctrl_pkts);
			vty_out(vty,"Recv Data Packets:         %u\t\t",rx_data_pkts);
			vty_out(vty,"Send Data Packets:         %u\n",tx_data_pkts);
			vty_out(vty,"Recv Auth Packets:         %u\t\t",rx_auth_pkts);
			vty_out(vty,"Send Auth Packets:         %u\n",tx_auth_pkts);
			vty_out(vty,"------------------------------------------------------------\n");

			dbus_message_iter_next(&iter_array);
		}

		vty_out(vty,"==============================================================================\n");

	}else if (ret == ASD_WTP_NOT_EXIST) 
	{
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#endif
//fengwenchao add 20101221
DEFUN(show_wlan_info_allwlan_cmd_func,
	  show_wlan_info_allwlan_cmd,
	  "show info of all wlan",
	  CONFIG_STR
	  "ASD wlan statistics information\n"
	  "Search by WLANID \n"
	  "WLAN id\n"
	 )
{	
	 
	struct dcli_wlan_info *wlan = NULL;
	struct dcli_wlan_info *wlan_show = NULL;
	unsigned char wlan_id = 0;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int wlan_num = 0;

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	wlan = show_info_allwlan(dcli_dbus_connection,index, localid,&ret,&wlan_num);
	
	if((ret == 0) && (wlan != NULL)){
		for(i=0;i<wlan_num;i++)
		{	
			if(wlan_show == NULL)
				wlan_show = wlan->wlan_list;
		
			else 
				wlan_show = wlan_show->next;

			if(wlan_show == NULL)
				break;	

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WLANID:                 %d\n",wlan_show->WlanID);
			vty_out(vty,"--------------------------\n");
			vty_out(vty,"Recv Data Pkts:         %u\n",wlan_show->rx_pkts);
			vty_out(vty,"Send Data Pkts:         %u\n",wlan_show->tx_pkts);
			vty_out(vty,"Recv Assoc Num:         %u\n",wlan_show->assoc_req_num);
			vty_out(vty,"Send Assoc Num:         %u\n",wlan_show->assoc_resp_num);
			vty_out(vty,"TxAssocFrameNum:         %u\n",wlan_show->assoc_resp_num);
			vty_out(vty,"RxAssocFrameNum:         %u\n",wlan_show->assoc_req_num);
			vty_out(vty,"Assoc Fail Num:         %u\n",wlan_show->assoc_fail_num);
			vty_out(vty,"Assoced Sta Num:        %u\n",wlan_show->sta_assoced_num);	
			vty_out(vty,"Accessed Sta Num:       %u\n",wlan_show->num_sta);
			vty_out(vty,"Assoc Success Times:		 %u\n",wlan_show->sta_assoced_num);
			vty_out(vty,"Associated Total UserNum:		 %u\n",wlan_show->sta_assoced_num);

			vty_out(vty,"normal down Num:        %u\n",wlan_show->num_normal_sta_down); 
			vty_out(vty,"abnormal down Num:      %u\n",wlan_show->num_abnormal_sta_down);
			
			vty_out(vty,"Recv Data Bytes:        %llu\n",wlan_show->rx_bytes);
			vty_out(vty,"Send Data Bytes:        %llu\n",wlan_show->tx_bytes);
			vty_out(vty,"TxDataFrameBytes :        %llu\n",wlan_show->tx_bytes);
			PRINTKMG(wlan_show->rx_bytes,"Recv Data Bytes")
			PRINTKMG(wlan_show->tx_bytes,"Send Data Bytes")
			vty_out(vty,"==============================================================================\n");
		}

	}
	else if (ret == ASD_WLAN_NOT_EXIST) {
		vty_out(vty,"<error> wlan does not provide service or it maybe does not exist\n");
	}	
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);

	dcli_free_allwlan(wlan);
	return CMD_SUCCESS;
}

//fengwenchao add end

#if DCLI_NEW
DEFUN(show_wlan_info_cmd_func,
	  show_wlan_info_cmd,
	  "show info bywlanid WLANID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD wlan statistics information\n"
	  "Search by WLANID \n"
	  "WLAN id\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_wlan_info *wlan = NULL;
	unsigned char wlan_id = 0;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		wlan = show_info_bywlan(dcli_dbus_connection, index, wlan_id, localid, &ret);
		
		if((ret == 0) && (wlan != NULL)){
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WLANID:                 %u\n",wlan_id);
			vty_out(vty,"--------------------------\n");
			vty_out(vty,"Recv Data Pkts:         %u\n",wlan->rx_pkts);
			vty_out(vty,"Send Data Pkts:         %u\n",wlan->tx_pkts);
			vty_out(vty,"Recv Assoc Num:         %u\n",wlan->assoc_req_num);
			vty_out(vty,"Send Assoc Num:         %u\n",wlan->assoc_resp_num);
			
			vty_out(vty,"Assoc Fail Num:         %u\n",wlan->assoc_fail_num);
			vty_out(vty,"Assoced Sta Num:        %u\n",wlan->sta_assoced_num);
			vty_out(vty,"Reassoc Req Num:        %u\n",wlan->reassoc_num);
			vty_out(vty,"Reassoc Success:        %u\n",wlan->reassoc_success_num);
			vty_out(vty,"Accessed Sta Num:       %u\n",wlan->sta_accessed_num);

			vty_out(vty,"Assoc Req Interim:      %u\n",wlan->assoc_req_interim);
			vty_out(vty,"Assoc Resp Interim:     %u\n",wlan->assoc_resp_interim);
			vty_out(vty,"Assoc Success Interim:  %u\n",wlan->assoc_success_interim);
			
			vty_out(vty,"Recv Data Bytes:        %llu\n",wlan->rx_bytes);
			vty_out(vty,"Send Data Bytes:        %llu\n",wlan->tx_bytes);
			PRINTKMG(wlan->rx_bytes,"Recv Data Bytes")
			PRINTKMG(wlan->tx_bytes,"Send Data Bytes")
			vty_out(vty,"==============================================================================\n");
			dcli_free_wlan(wlan);
		}else if (ret == ASD_WLAN_NOT_EXIST) 	{
			vty_out(vty,"<error> wlan %d does not provide service or it maybe does not exist\n",wlan_id);
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				wlan = show_info_bywlan(dcli_dbus_connection, profile, wlan_id, localid, &ret);
				
				vty_out(vty,"slotid %d,instid %d,wlanid %d,ret %d\n",slot_id,profile,wlan_id,ret);//for test
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wlan != NULL)){
					vty_out(vty,"WLANID:                 %u\n",wlan_id);
					vty_out(vty,"--------------------------\n");
					vty_out(vty,"Recv Data Pkts:         %u\n",wlan->rx_pkts);
					vty_out(vty,"Send Data Pkts:         %u\n",wlan->tx_pkts);
					vty_out(vty,"Recv Assoc Num:         %u\n",wlan->assoc_req_num);
					vty_out(vty,"Send Assoc Num:         %u\n",wlan->assoc_resp_num);
					
					vty_out(vty,"Assoc Fail Num:         %u\n",wlan->assoc_fail_num);
					vty_out(vty,"Assoced Sta Num:        %u\n",wlan->sta_assoced_num);
					vty_out(vty,"Reassoc Req Num:        %u\n",wlan->reassoc_num);
					vty_out(vty,"Reassoc Success:        %u\n",wlan->reassoc_success_num);
					vty_out(vty,"Accessed Sta Num:       %u\n",wlan->sta_accessed_num);

					vty_out(vty,"Assoc Req Interim:      %u\n",wlan->assoc_req_interim);
					vty_out(vty,"Assoc Resp Interim:     %u\n",wlan->assoc_resp_interim);
					vty_out(vty,"Assoc Success Interim:  %u\n",wlan->assoc_success_interim);
					
					vty_out(vty,"Recv Data Bytes:        %llu\n",wlan->rx_bytes);
					vty_out(vty,"Send Data Bytes:        %llu\n",wlan->tx_bytes);
					PRINTKMG(wlan->rx_bytes,"Recv Data Bytes")
					PRINTKMG(wlan->tx_bytes,"Send Data Bytes")
					dcli_free_wlan(wlan);
				}else if (ret == ASD_WLAN_NOT_EXIST) 	{
					vty_out(vty,"<error> wlan %d does not provide service or it maybe does not exist\n",wlan_id);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				wlan = show_info_bywlan(dcli_dbus_connection, profile, wlan_id, localid, &ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wlan != NULL)){
					vty_out(vty,"WLANID:                 %u\n",wlan_id);
					vty_out(vty,"--------------------------\n");
					vty_out(vty,"Recv Data Pkts:         %u\n",wlan->rx_pkts);
					vty_out(vty,"Send Data Pkts:         %u\n",wlan->tx_pkts);
					vty_out(vty,"Recv Assoc Num:         %u\n",wlan->assoc_req_num);
					vty_out(vty,"Send Assoc Num:         %u\n",wlan->assoc_resp_num);
					
					vty_out(vty,"Assoc Fail Num:         %u\n",wlan->assoc_fail_num);
					vty_out(vty,"Assoced Sta Num:        %u\n",wlan->sta_assoced_num);
					vty_out(vty,"Reassoc Req Num:        %u\n",wlan->reassoc_num);
					vty_out(vty,"Reassoc Success:        %u\n",wlan->reassoc_success_num);
					vty_out(vty,"Accessed Sta Num:       %u\n",wlan->sta_accessed_num);

					vty_out(vty,"Assoc Req Interim:      %u\n",wlan->assoc_req_interim);
					vty_out(vty,"Assoc Resp Interim:     %u\n",wlan->assoc_resp_interim);
					vty_out(vty,"Assoc Success Interim:  %u\n",wlan->assoc_success_interim);
					
					vty_out(vty,"Recv Data Bytes:        %llu\n",wlan->rx_bytes);
					vty_out(vty,"Send Data Bytes:        %llu\n",wlan->tx_bytes);
					PRINTKMG(wlan->rx_bytes,"Recv Data Bytes")
					PRINTKMG(wlan->tx_bytes,"Send Data Bytes")
					dcli_free_wlan(wlan);
				}else if (ret == ASD_WLAN_NOT_EXIST) 	{
					vty_out(vty,"<error> wlan %d does not provide service or it maybe does not exist\n",wlan_id);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;
}
#else
DEFUN(show_wlan_info_cmd_func,
	  show_wlan_info_cmd,
	  "show info bywlanid WLANID",
	  CONFIG_STR
	  "ASD wlan statistics information\n"
	  "Search by WLANID \n"
	  "WLAN id\n"
	 )
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned char wlan_id = 0;
	unsigned int rx_pkts = 0;	
	unsigned int tx_pkts = 0;	
	unsigned long long rx_bytes = 0;
	unsigned long long tx_bytes = 0;
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 

	unsigned int assoc_fail_num = 0;
	unsigned int sta_assoced_num = 0;
	unsigned int reassoc_num = 0; 
	unsigned int reassoc_success_num = 0; 

	unsigned int assoc_req_interim = 0;
	unsigned int assoc_resp_interim = 0;
	unsigned int assoc_success_interim = 0;


	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_resp_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_assoced_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_success_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_req_interim));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_resp_interim));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_success_interim));
		
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WLANID:                 %u\n",wlan_id);
		vty_out(vty,"--------------------------\n");
		vty_out(vty,"Recv Data Pkts:         %u\n",rx_pkts);
		vty_out(vty,"Send Data Pkts:         %u\n",tx_pkts);
		vty_out(vty,"Recv Assoc Num:         %u\n",assoc_req_num);
		vty_out(vty,"Send Assoc Num:         %u\n",assoc_resp_num);
		
		vty_out(vty,"Assoc Fail Num:         %u\n",assoc_fail_num);
		vty_out(vty,"Assoced Sta Num:        %u\n",sta_assoced_num);
		vty_out(vty,"Reassoc Req Num:        %u\n",reassoc_num);
		vty_out(vty,"Reassoc Success:        %u\n",reassoc_success_num);

		vty_out(vty,"Assoc Req Interim:      %u\n",assoc_req_interim);
		vty_out(vty,"Assoc Resp Interim:     %u\n",assoc_resp_interim);
		vty_out(vty,"Assoc Success Interim:  %u\n",assoc_success_interim);
		
		vty_out(vty,"Recv Data Bytes:        %llu\n",rx_bytes);
		vty_out(vty,"Send Data Bytes:        %llu\n",tx_bytes);
		PRINTKMG(rx_bytes,"Recv Data Bytes")
		PRINTKMG(tx_bytes,"Send Data Bytes")
		vty_out(vty,"==============================================================================\n");

	}else if (ret == ASD_WLAN_NOT_EXIST) 	{
		vty_out(vty,"<error> wlan %d does not provide service or it maybe does not exist\n",wlan_id);
	}	

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
DEFUN(show_bss_info_cmd_func,
	  show_bss_info_cmd,
	  "show info bywtpid WTPID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD bss statistics information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	
	 
	struct dcli_wtp_info *wtp = NULL;
	unsigned int wtp_id = 0;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	struct dcli_bss_info *bss = NULL;
	struct dcli_sta_info *sta = NULL;

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
	
	if(vty->node != VIEW_NODE){
		wtp = show_info_bywtp(dcli_dbus_connection, index, wtp_id, localid, &ret);
		
		if((ret == 0) && (wtp != NULL)){		
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WTPID:                     %u\n",wtp_id);
			vty_out(vty,"-----------------------------\n");
			vty_out(vty,"Access Times:              %u\t\t",wtp->acc_tms);
			vty_out(vty,"User Request Times:        %u\n",wtp->auth_tms);
			vty_out(vty,"Wtp Response Times:        %u\t\t",wtp->repauth_tms);
			vty_out(vty,"Auth Success Times:        %u\n",wtp->auth_success_num);//
			vty_out(vty,"Auth Fail Times:           %u\t\t",wtp->auth_fail_num);//5
			vty_out(vty,"Auth Invalid Times:        %u\n",wtp->auth_invalid_num);
			vty_out(vty,"Auth Timeout Times:        %u\t\t",wtp->auth_timeout_num);
			vty_out(vty,"Auth Refused Times:        %u\n",wtp->auth_refused_num);
			vty_out(vty,"Auth Others Times:         %u\t\t",wtp->auth_others_num);
			vty_out(vty,"Assoc Request Times:       %u\n",wtp->assoc_req_num);//10
			vty_out(vty,"Assoc Response Times:      %u\t\t",wtp->assoc_resp_num);
			vty_out(vty,"Assoc Invalid Times:       %u\n",wtp->assoc_invalid_num);
			vty_out(vty,"Assoc Timeout Times:       %u\t\t",wtp->assoc_timeout_num);
			vty_out(vty,"Assoc Refused Times:       %u\n",wtp->assoc_refused_num);
			vty_out(vty,"Assoc Others Times:        %u\t\t",wtp->assoc_others_num);//15
			vty_out(vty,"Reassoc Request Times:     %u\n",wtp->reassoc_request_num);
			vty_out(vty,"Reassoc Success Times:     %u\t\t",wtp->reassoc_success_num);
			vty_out(vty,"Reassoc Invalid Times:     %u\n",wtp->reassoc_invalid_num);
			vty_out(vty,"Reassoc Timeout Times:     %u\t\t",wtp->reassoc_timeout_num);
			vty_out(vty,"Reassoc Refused Times:     %u\n",wtp->reassoc_refused_num);//20
			vty_out(vty,"Reassoc Others Times:      %u\t\t",wtp->reassoc_others_num);
			vty_out(vty,"Identify Request Times:    %u\n",wtp->identify_request_num);
			vty_out(vty,"Identify Success Times:    %u\t\t",wtp->identify_success_num);
			vty_out(vty,"Abort Key Error Times:     %u\n",wtp->abort_key_error_num);
			vty_out(vty,"Abort Invalid Times:       %u\t\t",wtp->abort_invalid_num);//25
			vty_out(vty,"Abort Timeout Times:       %u\n",wtp->abort_timeout_num);
			vty_out(vty,"Abort Refused Times:       %u\t\t",wtp->abort_refused_num);
			vty_out(vty,"Abort Others Times:        %u\n",wtp->abort_others_num);
			vty_out(vty,"Deauth Request Times:      %u\t\t",wtp->deauth_request_num);
			vty_out(vty,"Deauth User Leave Times:   %u\n",wtp->deauth_user_leave_num);//30
			vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",wtp->deauth_ap_unable_num);
			vty_out(vty,"Deauth Abnormal Times:     %u\n",wtp->deauth_abnormal_num);
			vty_out(vty,"Deauth Others Times:       %u\t\t",wtp->deauth_others_num);
			vty_out(vty,"Disassoc Request Times:    %u\n",wtp->disassoc_request_num);
			vty_out(vty,"Disassoc User Leave Times: %u\t\t",wtp->disassoc_user_leave_num);//35
			vty_out(vty,"Disassoc Ap Unable Times:  %u\n",wtp->disassoc_ap_unable_num);
			vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",wtp->disassoc_abnormal_num);
			vty_out(vty,"Disassoc Others Times:     %u\n",wtp->disassoc_others_num);
			vty_out(vty,"Recv Mgmt Packets:         %u\t\t",wtp->rx_mgmt_pkts);
			vty_out(vty,"Send Mgmt Packets:         %u\n",wtp->tx_mgmt_pkts);//40
			vty_out(vty,"Recv Ctrl Packets:         %u\t\t",wtp->rx_ctrl_pkts);
			vty_out(vty,"Send Ctrl Packets:         %u\n",wtp->tx_ctrl_pkts);
			vty_out(vty,"Recv Data Packets:         %u\t\t",wtp->rx_data_pkts);
			vty_out(vty,"Send Data Packets:         %u\n",wtp->tx_data_pkts);
			vty_out(vty,"Recv Auth Packets:         %u\t\t",wtp->rx_auth_pkts);//45
			vty_out(vty,"Send Auth Packets:         %u\n",wtp->tx_auth_pkts);

			vty_out(vty,"Total Past Online Time:    %llu\t\t",wtp->wtp_total_past_online_time/60);
			vty_out(vty,"Total Assoc Failure Count: %u\n",wtp->num_assoc_failure);	//48
			vty_out(vty,"Accessed Sta Num:          %u\n",wtp->num_accessed_sta);//weichao add 20110802

			//mahz add 2011.5.3
			vty_out(vty,"-----------------------------\n");
			vty_out(vty,"WTP History Record:\n");
			vty_out(vty,"-------------------\n");
			vty_out(vty,"History User Request Times:         %u\n",wtp->usr_auth_tms_record);
			vty_out(vty,"History Wtp Response Times:         %u\n",wtp->ac_rspauth_tms_record);
			vty_out(vty,"History Auth Fail Times:            %u\n",wtp->auth_fail_record);
			vty_out(vty,"History Auth Success Times:         %u\n",wtp->auth_success_record);
			vty_out(vty,"History Assoc Request Times:        %u\n",wtp->num_assoc_record);
			vty_out(vty,"History Reassoc Request Times:      %u\n",wtp->num_reassoc_record);
			vty_out(vty,"History Assoc Failed Times:         %u\n",wtp->num_assoc_failure_record);
			vty_out(vty,"History Reassoc Failed Times:       %u\n",wtp->num_reassoc_failure_record);
			vty_out(vty,"History Assoc Success Times:        %u\n",wtp->assoc_success_record);
			vty_out(vty,"History Reassoc Success Times:      %u\n",wtp->reassoc_success_record);
			vty_out(vty,"History Total Assoc Request Times:  %u\n",wtp->assoc_req_record);
			vty_out(vty,"History Total Assoc Response Times: %u\n",wtp->assoc_resp_record);
			vty_out(vty,"History WTP Total Flow:             %llu (KB)\n",wtp->total_ap_flow_record);
			//
			vty_out(vty,"==============================================================================\n");



			/*for(i=0; i<wtp->num_bss; i++){
				if((bss = (struct dcli_bss_info*)malloc(sizeof(struct dcli_bss_info))) == NULL){
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
					wtp->bss_last = bss;*/				
			
			struct dcli_bss_info *bss = NULL;
			for(i=0;i<wtp->num_bss;i++){
				
				if(bss == NULL)
					bss = wtp->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
					
				vty_out(vty,"------------------------------------------------------------\n");
				vty_out(vty,"WLANID:                    %u\t\t",bss->WlanID);
				vty_out(vty,"BSSIndex:                  %u\n",bss->BSSIndex);
				vty_out(vty,"-----------------------------\n");
				vty_out(vty,"Access Times:              %u\t\t",bss->acc_tms);
				vty_out(vty,"User Request Times:        %u\n",bss->auth_tms);
				vty_out(vty,"Wtp Response Times:        %u\t\t",bss->repauth_tms);
				vty_out(vty,"Auth Success Times:        %u\n",bss->auth_success_num);
				vty_out(vty,"Auth Fail Times:           %u\t\t",bss->auth_fail_num);
				vty_out(vty,"Auth Invalid Times:        %u\n",bss->auth_invalid_num);
				vty_out(vty,"Auth Timeout Times:        %u\t\t",bss->auth_timeout_num);
				vty_out(vty,"Auth Refused Times:        %u\n",bss->auth_refused_num);
				vty_out(vty,"Auth Others Times:         %u\t\t",bss->auth_others_num);
				vty_out(vty,"Assoc Request Times:       %u\n",bss->assoc_req_num);
				vty_out(vty,"Assoc Response Times:      %u\t\t",bss->assoc_resp_num);
				vty_out(vty,"Assoc Invalid Times:       %u\n",bss->assoc_invalid_num);
				vty_out(vty,"Assoc Timeout Times:       %u\t\t",bss->assoc_timeout_num);
				vty_out(vty,"Assoc Refused Times:       %u\n",bss->assoc_refused_num);
				vty_out(vty,"Assoc Others Times:        %u\t\t",bss->assoc_others_num);
				vty_out(vty,"Reassoc Request Times:     %u\n",bss->reassoc_request_num);
				vty_out(vty,"Reassoc Success Times:     %u\t\t",bss->reassoc_success_num);
				vty_out(vty,"Reassoc Invalid Times:     %u\n",bss->reassoc_invalid_num);
				vty_out(vty,"Reassoc Timeout Times:     %u\t\t",bss->reassoc_timeout_num);
				vty_out(vty,"Reassoc Refused Times:     %u\n",bss->reassoc_refused_num);
				vty_out(vty,"Reassoc Others Times:      %u\t\t",bss->reassoc_others_num);
				vty_out(vty,"Identify Request Times:    %u\n",bss->identify_request_num);
				vty_out(vty,"Identify Success Times:    %u\t\t",bss->identify_success_num);
				vty_out(vty,"Abort Key Error Times:     %u\n",bss->abort_key_error_num);
				vty_out(vty,"Abort Invalid Times:       %u\t\t",bss->abort_invalid_num);
				vty_out(vty,"Abort Timeout Times:       %u\n",bss->abort_timeout_num);
				vty_out(vty,"Abort Refused Times:       %u\t\t",bss->abort_refused_num);
				vty_out(vty,"Abort Others Times:        %u\n",bss->abort_others_num);
				vty_out(vty,"Deauth Request Times:      %u\t\t",bss->deauth_request_num);
				vty_out(vty,"Deauth User Leave Times:   %u\n",bss->deauth_user_leave_num);
				vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",bss->deauth_ap_unable_num);
				vty_out(vty,"Deauth Abnormal Times:     %u\n",bss->deauth_abnormal_num);
				vty_out(vty,"Deauth Others Times:       %u\t\t",bss->deauth_others_num);
				vty_out(vty,"Disassoc Request Times:    %u\n",bss->disassoc_request_num);
				vty_out(vty,"Disassoc User Leave Times: %u\t\t",bss->disassoc_user_leave_num);
				vty_out(vty,"Disassoc Ap Unable Times:  %u\n",bss->disassoc_ap_unable_num);
				vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",bss->disassoc_abnormal_num);
				vty_out(vty,"Disassoc Others Times:     %u\n",bss->disassoc_others_num);
				vty_out(vty,"Recv Mgmt Packets:         %u\t\t",bss->rx_mgmt_pkts);
				vty_out(vty,"Send Mgmt Packets:         %u\n",bss->tx_mgmt_pkts);
				vty_out(vty,"Recv Ctrl Packets:         %u\t\t",bss->rx_ctrl_pkts);
				vty_out(vty,"Send Ctrl Packets:         %u\n",bss->tx_ctrl_pkts);
				vty_out(vty,"Recv Data Packets:         %u\t\t",bss->rx_data_pkts);
				vty_out(vty,"Send Data Packets:         %u\n",bss->tx_data_pkts);
				vty_out(vty,"Recv Auth Packets:         %u\t\t",bss->rx_auth_pkts);
				vty_out(vty,"Send Auth Packets:         %u\n",bss->tx_auth_pkts);
				vty_out(vty,"Total Past Online Time:    %llu\t\t",bss->total_past_online_time/60);
				vty_out(vty,"Total present Online Time: %llu\n",bss->total_present_online_time/60);
				vty_out(vty,"------------------------------------------------------------\n");
			}				
			vty_out(vty,"==============================================================================\n");
		}
		else if (ret == ASD_WTP_NOT_EXIST){
			vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
		}	
		else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		}
		else if (ret == ASD_DBUS_ERROR){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else{
			vty_out(vty,"<error> ret = %d\n",ret);
		}
		dcli_free_wtp(wtp);
	}


	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				wtp = show_info_bywtp(dcli_dbus_connection, profile, wtp_id, localid, &ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wtp != NULL)){		
					vty_out(vty,"WTPID:                     %u\n",wtp_id);
					vty_out(vty,"-----------------------------\n");
					vty_out(vty,"Access Times:              %u\t\t",wtp->acc_tms);
					vty_out(vty,"User Request Times:        %u\n",wtp->auth_tms);
					vty_out(vty,"Wtp Response Times:        %u\t\t",wtp->repauth_tms);
					vty_out(vty,"Auth Success Times:        %u\n",wtp->auth_success_num);//
					vty_out(vty,"Auth Fail Times:           %u\t\t",wtp->auth_fail_num);//5
					vty_out(vty,"Auth Invalid Times:        %u\n",wtp->auth_invalid_num);
					vty_out(vty,"Auth Timeout Times:        %u\t\t",wtp->auth_timeout_num);
					vty_out(vty,"Auth Refused Times:        %u\n",wtp->auth_refused_num);
					vty_out(vty,"Auth Others Times:         %u\t\t",wtp->auth_others_num);
					vty_out(vty,"Assoc Request Times:       %u\n",wtp->assoc_req_num);//10
					vty_out(vty,"Assoc Response Times:      %u\t\t",wtp->assoc_resp_num);
					vty_out(vty,"Assoc Invalid Times:       %u\n",wtp->assoc_invalid_num);
					vty_out(vty,"Assoc Timeout Times:       %u\t\t",wtp->assoc_timeout_num);
					vty_out(vty,"Assoc Refused Times:       %u\n",wtp->assoc_refused_num);
					vty_out(vty,"Assoc Others Times:        %u\t\t",wtp->assoc_others_num);//15
					vty_out(vty,"Reassoc Request Times:     %u\n",wtp->reassoc_request_num);
					vty_out(vty,"Reassoc Success Times:     %u\t\t",wtp->reassoc_success_num);
					vty_out(vty,"Reassoc Invalid Times:     %u\n",wtp->reassoc_invalid_num);
					vty_out(vty,"Reassoc Timeout Times:     %u\t\t",wtp->reassoc_timeout_num);
					vty_out(vty,"Reassoc Refused Times:     %u\n",wtp->reassoc_refused_num);//20
					vty_out(vty,"Reassoc Others Times:      %u\t\t",wtp->reassoc_others_num);
					vty_out(vty,"Identify Request Times:    %u\n",wtp->identify_request_num);
					vty_out(vty,"Identify Success Times:    %u\t\t",wtp->identify_success_num);
					vty_out(vty,"Abort Key Error Times:     %u\n",wtp->abort_key_error_num);
					vty_out(vty,"Abort Invalid Times:       %u\t\t",wtp->abort_invalid_num);//25
					vty_out(vty,"Abort Timeout Times:       %u\n",wtp->abort_timeout_num);
					vty_out(vty,"Abort Refused Times:       %u\t\t",wtp->abort_refused_num);
					vty_out(vty,"Abort Others Times:        %u\n",wtp->abort_others_num);
					vty_out(vty,"Deauth Request Times:      %u\t\t",wtp->deauth_request_num);
					vty_out(vty,"Deauth User Leave Times:   %u\n",wtp->deauth_user_leave_num);//30
					vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",wtp->deauth_ap_unable_num);
					vty_out(vty,"Deauth Abnormal Times:     %u\n",wtp->deauth_abnormal_num);
					vty_out(vty,"Deauth Others Times:       %u\t\t",wtp->deauth_others_num);
					vty_out(vty,"Disassoc Request Times:    %u\n",wtp->disassoc_request_num);
					vty_out(vty,"Disassoc User Leave Times: %u\t\t",wtp->disassoc_user_leave_num);//35
					vty_out(vty,"Disassoc Ap Unable Times:  %u\n",wtp->disassoc_ap_unable_num);
					vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",wtp->disassoc_abnormal_num);
					vty_out(vty,"Disassoc Others Times:     %u\n",wtp->disassoc_others_num);
					vty_out(vty,"Recv Mgmt Packets:         %u\t\t",wtp->rx_mgmt_pkts);
					vty_out(vty,"Send Mgmt Packets:         %u\n",wtp->tx_mgmt_pkts);//40
					vty_out(vty,"Recv Ctrl Packets:         %u\t\t",wtp->rx_ctrl_pkts);
					vty_out(vty,"Send Ctrl Packets:         %u\n",wtp->tx_ctrl_pkts);
					vty_out(vty,"Recv Data Packets:         %u\t\t",wtp->rx_data_pkts);
					vty_out(vty,"Send Data Packets:         %u\n",wtp->tx_data_pkts);
					vty_out(vty,"Recv Auth Packets:         %u\t\t",wtp->rx_auth_pkts);//45
					vty_out(vty,"Send Auth Packets:         %u\n",wtp->tx_auth_pkts);

					vty_out(vty,"Total Past Online Time:    %llu\t\t",wtp->wtp_total_past_online_time/60);
					vty_out(vty,"Total Assoc Failure Count: %u\n",wtp->num_assoc_failure);	//48
					vty_out(vty,"Accessed Sta Num:          %u\n",wtp->num_accessed_sta);//weichao add 20110802

					//mahz add 2011.5.3
					vty_out(vty,"-----------------------------\n");
					vty_out(vty,"WTP History Record:\n");
					vty_out(vty,"-------------------\n");
					vty_out(vty,"History User Request Times:         %u\n",wtp->usr_auth_tms_record);
					vty_out(vty,"History Wtp Response Times:         %u\n",wtp->ac_rspauth_tms_record);
					vty_out(vty,"History Auth Fail Times:            %u\n",wtp->auth_fail_record);
					vty_out(vty,"History Auth Success Times:         %u\n",wtp->auth_success_record);
					vty_out(vty,"History Assoc Request Times:        %u\n",wtp->num_assoc_record);
					vty_out(vty,"History Reassoc Request Times:      %u\n",wtp->num_reassoc_record);
					vty_out(vty,"History Assoc Failed Times:         %u\n",wtp->num_assoc_failure_record);
					vty_out(vty,"History Reassoc Failed Times:       %u\n",wtp->num_reassoc_failure_record);
					vty_out(vty,"History Assoc Success Times:        %u\n",wtp->assoc_success_record);
					vty_out(vty,"History Reassoc Success Times:      %u\n",wtp->reassoc_success_record);
					vty_out(vty,"History Total Assoc Request Times:  %u\n",wtp->assoc_req_record);
					vty_out(vty,"History Total Assoc Response Times: %u\n",wtp->assoc_resp_record);
					vty_out(vty,"History WTP Total Flow:             %llu (KB)\n",wtp->total_ap_flow_record);
					//
					vty_out(vty,"==============================================================================\n");

					struct dcli_bss_info *bss = NULL;
					for(i=0;i<wtp->num_bss;i++){
						
						if(bss == NULL)
							bss = wtp->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
							
						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"WLANID:                    %u\t\t",bss->WlanID);
						vty_out(vty,"BSSIndex:                  %u\n",bss->BSSIndex);
						vty_out(vty,"-----------------------------\n");
						vty_out(vty,"Access Times:              %u\t\t",bss->acc_tms);
						vty_out(vty,"User Request Times:        %u\n",bss->auth_tms);
						vty_out(vty,"Wtp Response Times:        %u\t\t",bss->repauth_tms);
						vty_out(vty,"Auth Success Times:        %u\n",bss->auth_success_num);
						vty_out(vty,"Auth Fail Times:           %u\t\t",bss->auth_fail_num);
						vty_out(vty,"Auth Invalid Times:        %u\n",bss->auth_invalid_num);
						vty_out(vty,"Auth Timeout Times:        %u\t\t",bss->auth_timeout_num);
						vty_out(vty,"Auth Refused Times:        %u\n",bss->auth_refused_num);
						vty_out(vty,"Auth Others Times:         %u\t\t",bss->auth_others_num);
						vty_out(vty,"Assoc Request Times:       %u\n",bss->assoc_req_num);
						vty_out(vty,"Assoc Response Times:      %u\t\t",bss->assoc_resp_num);
						vty_out(vty,"Assoc Invalid Times:       %u\n",bss->assoc_invalid_num);
						vty_out(vty,"Assoc Timeout Times:       %u\t\t",bss->assoc_timeout_num);
						vty_out(vty,"Assoc Refused Times:       %u\n",bss->assoc_refused_num);
						vty_out(vty,"Assoc Others Times:        %u\t\t",bss->assoc_others_num);
						vty_out(vty,"Reassoc Request Times:     %u\n",bss->reassoc_request_num);
						vty_out(vty,"Reassoc Success Times:     %u\t\t",bss->reassoc_success_num);
						vty_out(vty,"Reassoc Invalid Times:     %u\n",bss->reassoc_invalid_num);
						vty_out(vty,"Reassoc Timeout Times:     %u\t\t",bss->reassoc_timeout_num);
						vty_out(vty,"Reassoc Refused Times:     %u\n",bss->reassoc_refused_num);
						vty_out(vty,"Reassoc Others Times:      %u\t\t",bss->reassoc_others_num);
						vty_out(vty,"Identify Request Times:    %u\n",bss->identify_request_num);
						vty_out(vty,"Identify Success Times:    %u\t\t",bss->identify_success_num);
						vty_out(vty,"Abort Key Error Times:     %u\n",bss->abort_key_error_num);
						vty_out(vty,"Abort Invalid Times:       %u\t\t",bss->abort_invalid_num);
						vty_out(vty,"Abort Timeout Times:       %u\n",bss->abort_timeout_num);
						vty_out(vty,"Abort Refused Times:       %u\t\t",bss->abort_refused_num);
						vty_out(vty,"Abort Others Times:        %u\n",bss->abort_others_num);
						vty_out(vty,"Deauth Request Times:      %u\t\t",bss->deauth_request_num);
						vty_out(vty,"Deauth User Leave Times:   %u\n",bss->deauth_user_leave_num);
						vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",bss->deauth_ap_unable_num);
						vty_out(vty,"Deauth Abnormal Times:     %u\n",bss->deauth_abnormal_num);
						vty_out(vty,"Deauth Others Times:       %u\t\t",bss->deauth_others_num);
						vty_out(vty,"Disassoc Request Times:    %u\n",bss->disassoc_request_num);
						vty_out(vty,"Disassoc User Leave Times: %u\t\t",bss->disassoc_user_leave_num);
						vty_out(vty,"Disassoc Ap Unable Times:  %u\n",bss->disassoc_ap_unable_num);
						vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",bss->disassoc_abnormal_num);
						vty_out(vty,"Disassoc Others Times:     %u\n",bss->disassoc_others_num);
						vty_out(vty,"Recv Mgmt Packets:         %u\t\t",bss->rx_mgmt_pkts);
						vty_out(vty,"Send Mgmt Packets:         %u\n",bss->tx_mgmt_pkts);
						vty_out(vty,"Recv Ctrl Packets:         %u\t\t",bss->rx_ctrl_pkts);
						vty_out(vty,"Send Ctrl Packets:         %u\n",bss->tx_ctrl_pkts);
						vty_out(vty,"Recv Data Packets:         %u\t\t",bss->rx_data_pkts);
						vty_out(vty,"Send Data Packets:         %u\n",bss->tx_data_pkts);
						vty_out(vty,"Recv Auth Packets:         %u\t\t",bss->rx_auth_pkts);
						vty_out(vty,"Send Auth Packets:         %u\n",bss->tx_auth_pkts);
						vty_out(vty,"Total Past Online Time:    %llu\t\t",bss->total_past_online_time/60);
						vty_out(vty,"Total present Online Time: %llu\n",bss->total_present_online_time/60);
						vty_out(vty,"------------------------------------------------------------\n");
					}				
				}
				else if (ret == ASD_WTP_NOT_EXIST){
					vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
				}	
				else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				else if (ret == ASD_DBUS_ERROR){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else{
					vty_out(vty,"<error> ret = %d\n",ret);
				}
				dcli_free_wtp(wtp);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}


	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
	
			wtp = show_info_bywtp(dcli_dbus_connection, profile, wtp_id, localid, &ret);
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) && (wtp != NULL)){		
				vty_out(vty,"WTPID:                     %u\n",wtp_id);
				vty_out(vty,"-----------------------------\n");
				vty_out(vty,"Access Times:              %u\t\t",wtp->acc_tms);
				vty_out(vty,"User Request Times:        %u\n",wtp->auth_tms);
				vty_out(vty,"Wtp Response Times:        %u\t\t",wtp->repauth_tms);
				vty_out(vty,"Auth Success Times:        %u\n",wtp->auth_success_num);//
				vty_out(vty,"Auth Fail Times:           %u\t\t",wtp->auth_fail_num);//5
				vty_out(vty,"Auth Invalid Times:        %u\n",wtp->auth_invalid_num);
				vty_out(vty,"Auth Timeout Times:        %u\t\t",wtp->auth_timeout_num);
				vty_out(vty,"Auth Refused Times:        %u\n",wtp->auth_refused_num);
				vty_out(vty,"Auth Others Times:         %u\t\t",wtp->auth_others_num);
				vty_out(vty,"Assoc Request Times:       %u\n",wtp->assoc_req_num);//10
				vty_out(vty,"Assoc Response Times:      %u\t\t",wtp->assoc_resp_num);
				vty_out(vty,"Assoc Invalid Times:       %u\n",wtp->assoc_invalid_num);
				vty_out(vty,"Assoc Timeout Times:       %u\t\t",wtp->assoc_timeout_num);
				vty_out(vty,"Assoc Refused Times:       %u\n",wtp->assoc_refused_num);
				vty_out(vty,"Assoc Others Times:        %u\t\t",wtp->assoc_others_num);//15
				vty_out(vty,"Reassoc Request Times:     %u\n",wtp->reassoc_request_num);
				vty_out(vty,"Reassoc Success Times:     %u\t\t",wtp->reassoc_success_num);
				vty_out(vty,"Reassoc Invalid Times:     %u\n",wtp->reassoc_invalid_num);
				vty_out(vty,"Reassoc Timeout Times:     %u\t\t",wtp->reassoc_timeout_num);
				vty_out(vty,"Reassoc Refused Times:     %u\n",wtp->reassoc_refused_num);//20
				vty_out(vty,"Reassoc Others Times:      %u\t\t",wtp->reassoc_others_num);
				vty_out(vty,"Identify Request Times:    %u\n",wtp->identify_request_num);
				vty_out(vty,"Identify Success Times:    %u\t\t",wtp->identify_success_num);
				vty_out(vty,"Abort Key Error Times:     %u\n",wtp->abort_key_error_num);
				vty_out(vty,"Abort Invalid Times:       %u\t\t",wtp->abort_invalid_num);//25
				vty_out(vty,"Abort Timeout Times:       %u\n",wtp->abort_timeout_num);
				vty_out(vty,"Abort Refused Times:       %u\t\t",wtp->abort_refused_num);
				vty_out(vty,"Abort Others Times:        %u\n",wtp->abort_others_num);
				vty_out(vty,"Deauth Request Times:      %u\t\t",wtp->deauth_request_num);
				vty_out(vty,"Deauth User Leave Times:   %u\n",wtp->deauth_user_leave_num);//30
				vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",wtp->deauth_ap_unable_num);
				vty_out(vty,"Deauth Abnormal Times:     %u\n",wtp->deauth_abnormal_num);
				vty_out(vty,"Deauth Others Times:       %u\t\t",wtp->deauth_others_num);
				vty_out(vty,"Disassoc Request Times:    %u\n",wtp->disassoc_request_num);
				vty_out(vty,"Disassoc User Leave Times: %u\t\t",wtp->disassoc_user_leave_num);//35
				vty_out(vty,"Disassoc Ap Unable Times:  %u\n",wtp->disassoc_ap_unable_num);
				vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",wtp->disassoc_abnormal_num);
				vty_out(vty,"Disassoc Others Times:     %u\n",wtp->disassoc_others_num);
				vty_out(vty,"Recv Mgmt Packets:         %u\t\t",wtp->rx_mgmt_pkts);
				vty_out(vty,"Send Mgmt Packets:         %u\n",wtp->tx_mgmt_pkts);//40
				vty_out(vty,"Recv Ctrl Packets:         %u\t\t",wtp->rx_ctrl_pkts);
				vty_out(vty,"Send Ctrl Packets:         %u\n",wtp->tx_ctrl_pkts);
				vty_out(vty,"Recv Data Packets:         %u\t\t",wtp->rx_data_pkts);
				vty_out(vty,"Send Data Packets:         %u\n",wtp->tx_data_pkts);
				vty_out(vty,"Recv Auth Packets:         %u\t\t",wtp->rx_auth_pkts);//45
				vty_out(vty,"Send Auth Packets:         %u\n",wtp->tx_auth_pkts);

				vty_out(vty,"Total Past Online Time:    %llu\t\t",wtp->wtp_total_past_online_time/60);
				vty_out(vty,"Total Assoc Failure Count: %u\n",wtp->num_assoc_failure);	//48
				vty_out(vty,"Accessed Sta Num:          %u\n",wtp->num_accessed_sta);//weichao add 20110802

				//mahz add 2011.5.3
				vty_out(vty,"-----------------------------\n");
				vty_out(vty,"WTP History Record:\n");
				vty_out(vty,"-------------------\n");
				vty_out(vty,"History User Request Times:         %u\n",wtp->usr_auth_tms_record);
				vty_out(vty,"History Wtp Response Times:         %u\n",wtp->ac_rspauth_tms_record);
				vty_out(vty,"History Auth Fail Times:            %u\n",wtp->auth_fail_record);
				vty_out(vty,"History Auth Success Times:         %u\n",wtp->auth_success_record);
				vty_out(vty,"History Assoc Request Times:        %u\n",wtp->num_assoc_record);
				vty_out(vty,"History Reassoc Request Times:      %u\n",wtp->num_reassoc_record);
				vty_out(vty,"History Assoc Failed Times:         %u\n",wtp->num_assoc_failure_record);
				vty_out(vty,"History Reassoc Failed Times:       %u\n",wtp->num_reassoc_failure_record);
				vty_out(vty,"History Assoc Success Times:        %u\n",wtp->assoc_success_record);
				vty_out(vty,"History Reassoc Success Times:      %u\n",wtp->reassoc_success_record);
				vty_out(vty,"History Total Assoc Request Times:  %u\n",wtp->assoc_req_record);
				vty_out(vty,"History Total Assoc Response Times: %u\n",wtp->assoc_resp_record);
				vty_out(vty,"History WTP Total Flow:             %llu (KB)\n",wtp->total_ap_flow_record);
				//
				vty_out(vty,"==============================================================================\n");

				struct dcli_bss_info *bss = NULL;
				for(i=0;i<wtp->num_bss;i++){
					
					if(bss == NULL)
						bss = wtp->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
						
					vty_out(vty,"------------------------------------------------------------\n");
					vty_out(vty,"WLANID:                    %u\t\t",bss->WlanID);
					vty_out(vty,"BSSIndex:                  %u\n",bss->BSSIndex);
					vty_out(vty,"-----------------------------\n");
					vty_out(vty,"Access Times:              %u\t\t",bss->acc_tms);
					vty_out(vty,"User Request Times:        %u\n",bss->auth_tms);
					vty_out(vty,"Wtp Response Times:        %u\t\t",bss->repauth_tms);
					vty_out(vty,"Auth Success Times:        %u\n",bss->auth_success_num);
					vty_out(vty,"Auth Fail Times:           %u\t\t",bss->auth_fail_num);
					vty_out(vty,"Auth Invalid Times:        %u\n",bss->auth_invalid_num);
					vty_out(vty,"Auth Timeout Times:        %u\t\t",bss->auth_timeout_num);
					vty_out(vty,"Auth Refused Times:        %u\n",bss->auth_refused_num);
					vty_out(vty,"Auth Others Times:         %u\t\t",bss->auth_others_num);
					vty_out(vty,"Assoc Request Times:       %u\n",bss->assoc_req_num);
					vty_out(vty,"Assoc Response Times:      %u\t\t",bss->assoc_resp_num);
					vty_out(vty,"Assoc Invalid Times:       %u\n",bss->assoc_invalid_num);
					vty_out(vty,"Assoc Timeout Times:       %u\t\t",bss->assoc_timeout_num);
					vty_out(vty,"Assoc Refused Times:       %u\n",bss->assoc_refused_num);
					vty_out(vty,"Assoc Others Times:        %u\t\t",bss->assoc_others_num);
					vty_out(vty,"Reassoc Request Times:     %u\n",bss->reassoc_request_num);
					vty_out(vty,"Reassoc Success Times:     %u\t\t",bss->reassoc_success_num);
					vty_out(vty,"Reassoc Invalid Times:     %u\n",bss->reassoc_invalid_num);
					vty_out(vty,"Reassoc Timeout Times:     %u\t\t",bss->reassoc_timeout_num);
					vty_out(vty,"Reassoc Refused Times:     %u\n",bss->reassoc_refused_num);
					vty_out(vty,"Reassoc Others Times:      %u\t\t",bss->reassoc_others_num);
					vty_out(vty,"Identify Request Times:    %u\n",bss->identify_request_num);
					vty_out(vty,"Identify Success Times:    %u\t\t",bss->identify_success_num);
					vty_out(vty,"Abort Key Error Times:     %u\n",bss->abort_key_error_num);
					vty_out(vty,"Abort Invalid Times:       %u\t\t",bss->abort_invalid_num);
					vty_out(vty,"Abort Timeout Times:       %u\n",bss->abort_timeout_num);
					vty_out(vty,"Abort Refused Times:       %u\t\t",bss->abort_refused_num);
					vty_out(vty,"Abort Others Times:        %u\n",bss->abort_others_num);
					vty_out(vty,"Deauth Request Times:      %u\t\t",bss->deauth_request_num);
					vty_out(vty,"Deauth User Leave Times:   %u\n",bss->deauth_user_leave_num);
					vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",bss->deauth_ap_unable_num);
					vty_out(vty,"Deauth Abnormal Times:     %u\n",bss->deauth_abnormal_num);
					vty_out(vty,"Deauth Others Times:       %u\t\t",bss->deauth_others_num);
					vty_out(vty,"Disassoc Request Times:    %u\n",bss->disassoc_request_num);
					vty_out(vty,"Disassoc User Leave Times: %u\t\t",bss->disassoc_user_leave_num);
					vty_out(vty,"Disassoc Ap Unable Times:  %u\n",bss->disassoc_ap_unable_num);
					vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",bss->disassoc_abnormal_num);
					vty_out(vty,"Disassoc Others Times:     %u\n",bss->disassoc_others_num);
					vty_out(vty,"Recv Mgmt Packets:         %u\t\t",bss->rx_mgmt_pkts);
					vty_out(vty,"Send Mgmt Packets:         %u\n",bss->tx_mgmt_pkts);
					vty_out(vty,"Recv Ctrl Packets:         %u\t\t",bss->rx_ctrl_pkts);
					vty_out(vty,"Send Ctrl Packets:         %u\n",bss->tx_ctrl_pkts);
					vty_out(vty,"Recv Data Packets:         %u\t\t",bss->rx_data_pkts);
					vty_out(vty,"Send Data Packets:         %u\n",bss->tx_data_pkts);
					vty_out(vty,"Recv Auth Packets:         %u\t\t",bss->rx_auth_pkts);
					vty_out(vty,"Send Auth Packets:         %u\n",bss->tx_auth_pkts);
					vty_out(vty,"Total Past Online Time:    %llu\t\t",bss->total_past_online_time/60);
					vty_out(vty,"Total present Online Time: %llu\n",bss->total_present_online_time/60);
					vty_out(vty,"------------------------------------------------------------\n");
				}				
			}
			else if (ret == ASD_WTP_NOT_EXIST){
				vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
			}	
			else if(ret == ASD_WTP_ID_LARGE_THAN_MAX){
				vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
			}
			else if (ret == ASD_DBUS_ERROR){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else{
				vty_out(vty,"<error> ret = %d\n",ret);
			}
			dcli_free_wtp(wtp);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}

#else
DEFUN(show_bss_info_cmd_func,
	  show_bss_info_cmd,
	  "show info bywtpid WTPID",
	  CONFIG_STR
	  "ASD bss statistics information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned char wlan_id = 0;
	unsigned int bssnum = 0, wtp_id = 0;	
	unsigned int BSSIndex = 0,acc_tms=0,auth_tms=0,repauth_tms=0;
	unsigned int auth_success_num = 0; 
	unsigned int auth_fail_num = 0; 
	unsigned int auth_invalid_num = 0; 
	unsigned int auth_timeout_num = 0; 
	unsigned int auth_refused_num = 0; 
	unsigned int auth_others_num = 0; 
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 
	unsigned int assoc_invalid_num = 0; 
	unsigned int assoc_timeout_num = 0; 
	unsigned int assoc_refused_num = 0; 
	unsigned int assoc_others_num = 0; 
	unsigned int reassoc_request_num = 0; 
	unsigned int reassoc_success_num = 0; 
	unsigned int reassoc_invalid_num = 0; 
	unsigned int reassoc_timeout_num = 0; 
	unsigned int reassoc_refused_num = 0; 
	unsigned int reassoc_others_num = 0; 
	unsigned int identify_request_num = 0; 
	unsigned int identify_success_num = 0; 
	unsigned int abort_key_error_num = 0; 
	unsigned int abort_invalid_num = 0; 
	unsigned int abort_timeout_num = 0; 
	unsigned int abort_refused_num = 0; 
	unsigned int abort_others_num = 0; 
	unsigned int deauth_request_num = 0; 
	unsigned int deauth_user_leave_num = 0; 
	unsigned int deauth_ap_unable_num = 0; 
	unsigned int deauth_abnormal_num = 0; 
	unsigned int deauth_others_num = 0; 
	unsigned int disassoc_request_num = 0; 
	unsigned int disassoc_user_leave_num = 0; 
	unsigned int disassoc_ap_unable_num = 0; 
	unsigned int disassoc_abnormal_num = 0; 
	unsigned int disassoc_others_num = 0; 

	unsigned int rx_mgmt_pkts = 0;
	unsigned int tx_mgmt_pkts = 0;
	unsigned int rx_ctrl_pkts = 0;
	unsigned int tx_ctrl_pkts = 0;
	unsigned int rx_data_pkts = 0;
	unsigned int tx_data_pkts = 0;
	unsigned int rx_auth_pkts = 0;
	unsigned int tx_auth_pkts = 0;

	unsigned long long wtp_total_past_online_time = 0;	/*	xm0703*/
	unsigned int num_assoc_failure = 0;	/*	xm0703*/

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_total_past_online_time));	/*	xm0703*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(num_assoc_failure));	/*	xm0703*/
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(acc_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(repauth_tms));//5

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_refused_num));//10
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_resp_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_timeout_num));//15
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(assoc_others_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_invalid_num));//20
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(reassoc_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(identify_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(identify_success_num));//25
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(abort_key_error_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(abort_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(abort_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(abort_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(abort_others_num));//30

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(deauth_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(deauth_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(deauth_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(deauth_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(deauth_others_num));//35
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(disassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(disassoc_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(disassoc_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(disassoc_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(disassoc_others_num));//4  //40
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_data_pkts));//45
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_data_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(rx_auth_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(tx_auth_pkts));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(bssnum));//49
		
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID:                     %u\n",wtp_id);
		vty_out(vty,"-----------------------------\n");
		vty_out(vty,"Access Times:              %u\t\t",acc_tms);
		vty_out(vty,"User Request Times:        %u\n",auth_tms);
		vty_out(vty,"Wtp Response Times:        %u\t\t",repauth_tms);
		vty_out(vty,"Auth Success Times:        %u\n",auth_success_num);
		vty_out(vty,"Auth Fail Times:           %u\t\t",auth_fail_num);
		vty_out(vty,"Auth Invalid Times:        %u\n",auth_invalid_num);
		vty_out(vty,"Auth Timeout Times:        %u\t\t",auth_timeout_num);
		vty_out(vty,"Auth Refused Times:        %u\n",auth_refused_num);
		vty_out(vty,"Auth Others Times:         %u\t\t",auth_others_num);
		vty_out(vty,"Assoc Request Times:       %u\n",assoc_req_num);
		vty_out(vty,"Assoc Response Times:      %u\t\t",assoc_resp_num);
		vty_out(vty,"Assoc Invalid Times:       %u\n",assoc_invalid_num);
		vty_out(vty,"Assoc Timeout Times:       %u\t\t",assoc_timeout_num);
		vty_out(vty,"Assoc Refused Times:       %u\n",assoc_refused_num);
		vty_out(vty,"Assoc Others Times:        %u\t\t",assoc_others_num);
		vty_out(vty,"Reassoc Request Times:     %u\n",reassoc_request_num);
		vty_out(vty,"Reassoc Success Times:     %u\t\t",reassoc_success_num);
		vty_out(vty,"Reassoc Invalid Times:     %u\n",reassoc_invalid_num);
		vty_out(vty,"Reassoc Timeout Times:     %u\t\t",reassoc_timeout_num);
		vty_out(vty,"Reassoc Refused Times:     %u\n",reassoc_refused_num);
		vty_out(vty,"Reassoc Others Times:      %u\t\t",reassoc_others_num);
		vty_out(vty,"Identify Request Times:    %u\n",identify_request_num);
		vty_out(vty,"Identify Success Times:    %u\t\t",identify_success_num);
		vty_out(vty,"Abort Key Error Times:     %u\n",abort_key_error_num);
		vty_out(vty,"Abort Invalid Times:       %u\t\t",abort_invalid_num);
		vty_out(vty,"Abort Timeout Times:       %u\n",abort_timeout_num);
		vty_out(vty,"Abort Refused Times:       %u\t\t",abort_refused_num);
		vty_out(vty,"Abort Others Times:        %u\n",abort_others_num);
		vty_out(vty,"Deauth Request Times:      %u\t\t",deauth_request_num);
		vty_out(vty,"Deauth User Leave Times:   %u\n",deauth_user_leave_num);
		vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",deauth_ap_unable_num);
		vty_out(vty,"Deauth Abnormal Times:     %u\n",deauth_abnormal_num);
		vty_out(vty,"Deauth Others Times:       %u\t\t",deauth_others_num);
		vty_out(vty,"Disassoc Request Times:    %u\n",disassoc_request_num);
		vty_out(vty,"Disassoc User Leave Times: %u\t\t",disassoc_user_leave_num);
		vty_out(vty,"Disassoc Ap Unable Times:  %u\n",disassoc_ap_unable_num);
		vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",disassoc_abnormal_num);
		vty_out(vty,"Disassoc Others Times:     %u\n",disassoc_others_num);
		vty_out(vty,"Recv Mgmt Packets:         %u\t\t",rx_mgmt_pkts);
		vty_out(vty,"Send Mgmt Packets:         %u\n",tx_mgmt_pkts);
		vty_out(vty,"Recv Ctrl Packets:         %u\t\t",rx_ctrl_pkts);
		vty_out(vty,"Send Ctrl Packets:         %u\n",tx_ctrl_pkts);
		vty_out(vty,"Recv Data Packets:         %u\t\t",rx_data_pkts);
		vty_out(vty,"Send Data Packets:         %u\n",tx_data_pkts);
		vty_out(vty,"Recv Auth Packets:         %u\t\t",rx_auth_pkts);
		vty_out(vty,"Send Auth Packets:         %u\n",tx_auth_pkts);

		vty_out(vty,"Total Past Online Time:    %llu\t\t",wtp_total_past_online_time/60);
		vty_out(vty,"Total Assoc Failure Count: %u\n",num_assoc_failure);	/*	xm0703*/
		vty_out(vty,"==============================================================================\n");

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0; i<bssnum; i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wlan_id));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(BSSIndex));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(acc_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_tms));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(repauth_tms));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_fail_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_req_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_resp_num));
	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(identify_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(identify_success_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_key_error_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_invalid_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_timeout_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_refused_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abort_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deauth_others_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_request_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_user_leave_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_ap_unable_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_abnormal_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(disassoc_others_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_data_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_data_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_auth_pkts));


			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"WLANID:                    %u\t\t",wlan_id);
			vty_out(vty,"BSSIndex:                  %u\n",BSSIndex);
			vty_out(vty,"-----------------------------\n");
			vty_out(vty,"Access Times:              %u\t\t",acc_tms);
			vty_out(vty,"User Request Times:        %u\n",auth_tms);
			vty_out(vty,"Wtp Response Times:        %u\t\t",repauth_tms);
			vty_out(vty,"Auth Success Times:        %u\n",auth_success_num);
			vty_out(vty,"Auth Fail Times:           %u\t\t",auth_fail_num);
			vty_out(vty,"Auth Invalid Times:        %u\n",auth_invalid_num);
			vty_out(vty,"Auth Timeout Times:        %u\t\t",auth_timeout_num);
			vty_out(vty,"Auth Refused Times:        %u\n",auth_refused_num);
			vty_out(vty,"Auth Others Times:         %u\t\t",auth_others_num);
			vty_out(vty,"Assoc Request Times:       %u\n",assoc_req_num);
			vty_out(vty,"Assoc Response Times:      %u\t\t",assoc_resp_num);
			vty_out(vty,"Assoc Invalid Times:       %u\n",assoc_invalid_num);
			vty_out(vty,"Assoc Timeout Times:       %u\t\t",assoc_timeout_num);
			vty_out(vty,"Assoc Refused Times:       %u\n",assoc_refused_num);
			vty_out(vty,"Assoc Others Times:        %u\t\t",assoc_others_num);
			vty_out(vty,"Reassoc Request Times:     %u\n",reassoc_request_num);
			vty_out(vty,"Reassoc Success Times:     %u\t\t",reassoc_success_num);
			vty_out(vty,"Reassoc Invalid Times:     %u\n",reassoc_invalid_num);
			vty_out(vty,"Reassoc Timeout Times:     %u\t\t",reassoc_timeout_num);
			vty_out(vty,"Reassoc Refused Times:     %u\n",reassoc_refused_num);
			vty_out(vty,"Reassoc Others Times:      %u\t\t",reassoc_others_num);
			vty_out(vty,"Identify Request Times:    %u\n",identify_request_num);
			vty_out(vty,"Identify Success Times:    %u\t\t",identify_success_num);
			vty_out(vty,"Abort Key Error Times:     %u\n",abort_key_error_num);
			vty_out(vty,"Abort Invalid Times:       %u\t\t",abort_invalid_num);
			vty_out(vty,"Abort Timeout Times:       %u\n",abort_timeout_num);
			vty_out(vty,"Abort Refused Times:       %u\t\t",abort_refused_num);
			vty_out(vty,"Abort Others Times:        %u\n",abort_others_num);
			vty_out(vty,"Deauth Request Times:      %u\t\t",deauth_request_num);
			vty_out(vty,"Deauth User Leave Times:   %u\n",deauth_user_leave_num);
			vty_out(vty,"Deauth Ap Unable Times:    %u\t\t",deauth_ap_unable_num);
			vty_out(vty,"Deauth Abnormal Times:     %u\n",deauth_abnormal_num);
			vty_out(vty,"Deauth Others Times:       %u\t\t",deauth_others_num);
			vty_out(vty,"Disassoc Request Times:    %u\n",disassoc_request_num);
			vty_out(vty,"Disassoc User Leave Times: %u\t\t",disassoc_user_leave_num);
			vty_out(vty,"Disassoc Ap Unable Times:  %u\n",disassoc_ap_unable_num);
			vty_out(vty,"Disassoc Abnormal Times:   %u\t\t",disassoc_abnormal_num);
			vty_out(vty,"Disassoc Others Times:     %u\n",disassoc_others_num);
			vty_out(vty,"Recv Mgmt Packets:         %u\t\t",rx_mgmt_pkts);
			vty_out(vty,"Send Mgmt Packets:         %u\n",tx_mgmt_pkts);
			vty_out(vty,"Recv Ctrl Packets:         %u\t\t",rx_ctrl_pkts);
			vty_out(vty,"Send Ctrl Packets:         %u\n",tx_ctrl_pkts);
			vty_out(vty,"Recv Data Packets:         %u\t\t",rx_data_pkts);
			vty_out(vty,"Send Data Packets:         %u\n",tx_data_pkts);
			vty_out(vty,"Recv Auth Packets:         %u\t\t",rx_auth_pkts);
			vty_out(vty,"Send Auth Packets:         %u\n",tx_auth_pkts);
			vty_out(vty,"------------------------------------------------------------\n");

			dbus_message_iter_next(&iter_array);
		}		
	vty_out(vty,"==============================================================================\n");

	if(bssnum == 0)
		vty_out(vty,"WTP %d does not provide service\n",wtp_id);
	}else if (ret == ASD_WTP_NOT_EXIST)		
	{
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif
DEFUN(show_sta_list_by_interface_func,
	  show_sta_list_by_interface_cmd,
	  "show sta list by interface",
	  CONFIG_STR
	  "ASD station list information\n"
	  "all sta\n"
	 )
{	

	struct dcli_ac_info *ac = NULL;
	struct dcli_iface_info *iface = NULL;
	unsigned int ret = 0;
	int i,j,k,m;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ac = show_sta_by_interface(dcli_dbus_connection,index,localid,&ret);

	if((ret == 0) && (ac != NULL)){
		for(k=0; k<ac->num_iface; k++){			
			struct dcli_bss_info *bss = NULL;
			if(iface == NULL)
				iface = ac->iface_list;
			else 
				iface = iface->next;

			if(iface == NULL)
				break;
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"IF_NAME:     %s\t\t",iface->IF_NAME);
			vty_out(vty,"BSS num:     %d\t\t",iface->num_bss);
			vty_out(vty,"Sta num:     %d\n",iface->num_sta);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Sta_MAC            IP               WTPID  Radio_ID  BSSIndex	WlanID	Sec_ID\n");
			for(i=0; i<iface->num_bss; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = iface->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				
				for(j=0; j<bss->num_sta; j++){	
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					
					vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s	",sta->ip);
					vty_out(vty,"%-5d  ",bss->WtpID);
					vty_out(vty,"%-5d	  ",bss->Radio_G_ID);		
					vty_out(vty,"%-5d	  ",bss->BSSIndex);
					vty_out(vty,"%-5d	",bss->WlanID);
					vty_out(vty,"%-5d\n",bss->SecurityID);
					vty_out(vty,"ipv6 address:      ");     /* add for ipv6 sta */
					for (m = 0; m < 8; m++)
                	{   
						if(m==7)
						{
                            vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[m]);
						}
					    else
					    {
                            vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[m]);
					    }
                	}					
				}
			}		
			vty_out(vty,"==============================================================================\n");
		}
		vty_out(vty,"==============================================================================\n");
		if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
			vty_out(vty,"<error> there is no station\n");
		dcli_free_ac(ac);
	}
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);
	
	return CMD_SUCCESS;
}


//mahz add for mib request , 2011.1.17 , dot11DistinguishTable	
DEFUN(show_distinguish_information_of_all_wtp_cmd_func,
	  show_distinguish_information_of_all_wtp_cmd,
	  "show distinguish information of all wtp",
	  SHOW_STR
	  "Display wtps information\n"
	  "List wtp summary\n"	
	 )
{
	 
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_wtp_info *tmp = NULL;
	unsigned int wtp_num;
	unsigned int ret = 0;
	int i;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char wtp_mac[18] = {0};

	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	wtp = show_distinguish_info_of_all_wtp(index,dcli_dbus_connection,&wtp_num, localid,&ret);
	
	if((ret == 0) && (wtp != NULL)){
		tmp = wtp;
		for(i=0;i<wtp_num;i++){
			
			sprintf(wtp_mac,"%02X:%02X:%02X:%02X:%02X:%02X",tmp->mac[0],tmp->mac[1],
					tmp->mac[2],tmp->mac[3],tmp->mac[4],tmp->mac[5]);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WTPID:                     %u\n",tmp->WtpID);
			vty_out(vty,"-----------------------------\n");
			vty_out(vty,"WTP MAC:              	    %s\n",wtp_mac);
			vty_out(vty,"Access Times:              %u\n",tmp->acc_tms);
			vty_out(vty,"User Request Times:        %u\n",tmp->auth_tms);
			vty_out(vty,"Identify Request Times:    %u\n",tmp->identify_request_num);
			vty_out(vty,"Identify Success Times:    %u\n",tmp->identify_success_num);
			vty_out(vty,"Abort Key Error Times:     %u\n",tmp->abort_key_error_num);
			vty_out(vty,"Abort Invalid Times:       %u\n",tmp->abort_invalid_num);
			vty_out(vty,"Abort Refused Times:       %u\n",tmp->abort_refused_num);
			vty_out(vty,"Abort Timeout Times:       %u\n",tmp->abort_timeout_num);
			vty_out(vty,"Abort Others Times:        %u\n",tmp->abort_others_num);
			vty_out(vty,"wtpAssociatedTotalUserNum:        %u\n",tmp->num_accessed_sta);
			vty_out(vty,"==============================================================================\n");
			tmp = tmp->next;
		}
	}
	else if (ret == ASD_WTP_NOT_EXIST){
		vty_out(vty,"<error> wtp does not provide service or it maybe does not exist\n");
	}	
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_wtp_list(wtp);
	return CMD_SUCCESS;	
}



//mahz add for mib request , 2011.1.19 , dot11BSSIDWAPIProtocolConfigTable
DEFUN(show_wapi_mib_info_of_all_wtp_cmd_func,
	  show_wapi_mib_info_of_all_wtp_cmd,
	  "show wapi mib info of all wtp",
	  CONFIG_STR
	  "wapi mib information\n"
	  "Search all wtp\n"
	  )
{	
	unsigned int ret = 0;
	int i=0 ,j=0, k=0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	unsigned int wtp_num;
	char wtp_mac[18] = {0};
	
	struct dcli_wtp_info *wtp = NULL;	
	struct dcli_wtp_info *tmp = NULL;
	
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	wtp = show_wapi_mib_info_of_all_wtp(dcli_dbus_connection,index,&wtp_num, localid,&ret);

	if((ret == 0)&&(wtp != NULL)){
		tmp = wtp;
		for(i=0;i<wtp_num;i++){
			sprintf(wtp_mac,"%02X:%02X:%02X:%02X:%02X:%02X",tmp->mac[0],tmp->mac[1],
					tmp->mac[2],tmp->mac[3],tmp->mac[4],tmp->mac[5]);
			
			vty_out(vty,"\n============================================================\n");
			vty_out(vty,"WTPID:                     %u\n",tmp->WtpID);
			vty_out(vty,"WTP MAC:              	   %s\n",wtp_mac);
			vty_out(vty,"---------------------------------------------------\n");	

			struct dcli_bss_info *bss = NULL;

			for(j=0; j<tmp->num_bss; j++) {
				
				if(bss == NULL)
					bss = tmp->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
			
		//		vty_out(vty,"bss[%d] "MACSTR"\n",i+1,MAC2STR(bss->bssid));
				vty_out(vty,"wapiEnabled = %4u\n",bss->wapiEnabled);
				vty_out(vty,"---------------------------------------\n");		
				vty_out(vty,"BSSID:				%02X:%02X:%02X:%02X:%02X:%02X\n",bss->bssid[0],bss->bssid[1],bss->bssid[2],
																	bss->bssid[3],bss->bssid[4],bss->bssid[5]);
				if(bss->wapiEnabled == 1){
					vty_out(vty,"wapiEnabled:			%4u\n",bss->wapiEnabled);
					vty_out(vty,"ControlledAuthControl:		%4u\n",bss->ControlledAuthControl);
					vty_out(vty,"ControlledPortControl:		%4u\n",bss->ControlledPortControl);
					
					vty_out(vty,"CertificateUpdateCount:		%4llu\n",bss->CertificateUpdateCount);
					vty_out(vty,"MulticastUpdateCount:		%4llu\n",bss->MulticastUpdateCount);
					vty_out(vty,"UnicastUpdateCount:		%4llu\n",bss->UnicastUpdateCount);
				
					vty_out(vty,"AuthenticationSuite:		%4u\n",bss->AuthenticationSuite);
					vty_out(vty,"AuthSuiteSelected:		%4u\n",bss->AuthSuiteSelected);
				}
				vty_out(vty,"---------------------------------------\n");
			}		
		tmp = tmp->next;
		vty_out(vty,"============================================================\n");		
		}
	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> wtp does not provide service or it maybe does not exist\n");
	}
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
		}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	dcli_free_wtp_list(wtp);
	return CMD_SUCCESS;
		
}
//


//mahz add for mib request , 2011.1.24 , dot11StaWAPIProtocolConfigTable
DEFUN(show_sta_wapi_mib_info_of_all_wtp_cmd_func,
	  show_sta_wapi_mib_info_of_all_wtp_cmd,
	  "show sta wapi mib info of all wtp",
	  CONFIG_STR
	  "wapi mib information\n"
	  "Search all wtp\n"
	  )
{	
	unsigned int ret = 0;
	int i=0 ,j=0, k=0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	unsigned int wtp_num;
	char wtp_mac[18] = {0};
	
	struct wapi_mib_wtp_info *wtp = NULL;	
	struct wapi_mib_wtp_info *tmp = NULL;
	
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	wtp = show_sta_wapi_mib_info_of_all_wtp(dcli_dbus_connection,index,&wtp_num, localid,&ret);

	if((ret == 0)&&(wtp != NULL)){
		tmp = wtp;
		for(i=0;i<wtp_num;i++){
			sprintf(wtp_mac,"%02X:%02X:%02X:%02X:%02X:%02X",tmp->mac[0],tmp->mac[1],
					tmp->mac[2],tmp->mac[3],tmp->mac[4],tmp->mac[5]);
			
			vty_out(vty,"\n============================================================\n");
			vty_out(vty,"WTPID:                     %u\n",tmp->WtpID);
			vty_out(vty,"WTP MAC:              	   %s\n",wtp_mac);
			vty_out(vty,"---------------------------------------------------\n");	

			struct dcli_sta_info *sta = NULL;

			for(j=0; j<tmp->num_sta; j++) {
				
				if(sta == NULL)
					sta = tmp->sta_list;
				else 
					sta = sta->next;

				if(sta == NULL)
					break;
			
				vty_out(vty,"STA_MAC:		%02X:%02X:%02X:%02X:%02X:%02X\n",sta->addr[0],sta->addr[1],sta->addr[2],
																					sta->addr[3],sta->addr[4],sta->addr[5]);
				vty_out(vty,"ControlledPortStatus:	%4u\n",sta->ControlledPortStatus);
				vty_out(vty,"BKIDUsed:		%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
					sta->BKIDUsed[0],sta->BKIDUsed[1],sta->BKIDUsed[2],sta->BKIDUsed[3],
					sta->BKIDUsed[4],sta->BKIDUsed[5],sta->BKIDUsed[6],sta->BKIDUsed[7],
					sta->BKIDUsed[8],sta->BKIDUsed[9],sta->BKIDUsed[10],sta->BKIDUsed[11],
					sta->BKIDUsed[12],sta->BKIDUsed[13],sta->BKIDUsed[14],sta->BKIDUsed[15]);
				vty_out(vty,"---------------------------------------\n");
			}		
		tmp = tmp->next;
		vty_out(vty,"============================================================\n");		
		}
	}
	else if (ret ==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> wtp does not provide service or it maybe does not exist\n");
	}
	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	dcli_sta_wapi_mib_info_free_wtp_list(wtp);		
	return CMD_SUCCESS;
}

DEFUN(show_all_sta_base_info_cmd_func,
	  show_all_sta_base_info_cmd,
	  "show all sta base info",
	  CONFIG_STR
	  "ASD station list information\n"
	  "all sta\n"
	 )
{	
	 
	struct dcli_base_bss_info *bss = NULL;
	struct dcli_base_bss_info *bsshead = NULL;
	unsigned int ret = 0;
	unsigned int bss_num = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	bsshead = show_sta_base_info(dcli_dbus_connection,index,localid,&ret,&bss_num);

	if((ret == 0) && (bsshead != NULL)){
		bss = bsshead;
		while(bss != NULL){
			vty_out(vty,"==============================================================================\n");
			struct dcli_sta_base_info *sta = NULL;
			for(i=0; i<bss->num_sta; i++){	
				if(sta == NULL)
					sta = bss->sta_list;
				else 
					sta = sta->next;
				
				if(sta == NULL)
					break;
				
				vty_out(vty,"------------------------------------------------------------\n");
				vty_out(vty,"WTPID:                %d\n",sta->wtp_id);
				vty_out(vty,"WTP MAC:              "MACSTR"\n",MAC2STR(sta->wtp_addr));
				vty_out(vty,"Radio_G_ID:           %d\n",sta->radio_g_id);
				vty_out(vty,"Radio_L_ID:           %d\n",sta->radio_l_id);
				vty_out(vty,"WLANID:               %d\n",sta->wlan_id);
				vty_out(vty,"ESSID:                %s\n",sta->essid);		
				vty_out(vty,"STA MAC:              "MACSTR"\n",MAC2STR(sta->addr));
				vty_out(vty,"STA IP:               %s\n",sta->ip);
				vty_out(vty,"------------------------------------------------------------\n");
			}		
			bss = bss->next;
		}
		dcli_free_base_bss(bsshead);
	}
	else if(bss_num == 0)
		vty_out(vty,"<error> there is no station\n");
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);
	
	return CMD_SUCCESS;
}


DEFUN(extend_show_sta_cmd_func,
	  extend_show_sta_cmd,
	  "extend show sta MAC",
	  "extend command\n"
	  CONFIG_STR
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to check\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	unsigned char mac1[MAC_LEN];
	unsigned int mac[MAC_LEN],ret,snr=0;
	unsigned long long rr=0,tr=0,tp=0,rx_bytes=0,tx_bytes=0;
	unsigned long long rx_pkts=0, tx_pkts=0, rtx=0, rtx_pkts, err_pkts=0;
	unsigned char ip[16];
	unsigned char *in_addr = ip;
	double flux;
	time_t StaTime,sta_online_time,sta_access_time,online_time;//qiuchen change it 2012.10.31

	memset(mac,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
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
		//qiuchen add it
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_online_time));	
		//end
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_access_time));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(online_time));	
	}	
	dbus_message_unref(reply);
	if(ret == 0){		

		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"STA MAC: %02X:",mac[0]);
		vty_out(vty,"%02X:",mac[1]);		
		vty_out(vty,"%02X:",mac[2]);
		vty_out(vty,"%02X:",mac[3]);
		vty_out(vty,"%02X:",mac[4]);
		vty_out(vty,"%02X\n",mac[5]);
		vty_out(vty,"STA IP:  %s\n",in_addr);
		vty_out(vty,"SNR:     %d(dB)\n",snr);
		PRINTKMG(rx_bytes,"Rx Bytes")
		PRINTKMG(tx_bytes,"Tx Bytes")
		vty_out(vty,"Rx Packets:     %llu\n",rx_pkts);
		vty_out(vty,"Tx Packets:     %llu\n",tx_pkts);		/*ht add 090220*/
		vty_out(vty,"Retry Packets:     %llu\n",rtx_pkts);
		vty_out(vty,"Error Packets:     %llu\n",err_pkts);		/*ht add 090220*/
		PRINTKMG(rtx,"Retry Bytes")
		if( rr < OKB )
			vty_out(vty,"Receive Rate:    %llu(B/s)\n",rr); 
		else if((rr >= OKB) && (rr < OMB)){
			flux = (double)rr/OKB;
			vty_out(vty,"Receive Rate:    %.1f(KB/s)\n",flux);  
		}
		else if((rr >= OMB) && (rr < OGB)){
			flux = (double)rr/OMB;
			vty_out(vty,"Receive Rate:	  %.1f(MB/s)\n",flux);	
		}
		else{
			flux = (double)rr/OGB;
			vty_out(vty,"Receive Rate:	  %.1f(GB/s)\n",flux);	
		}
			
		if( tr < OKB )
			vty_out(vty,"Transmit Rate:	 %llu(B/s)\n",tr); 
		else if((tr >= OKB) && (tr < OMB)){
			flux = (double)tr/OKB;
			vty_out(vty,"Transmit Rate:	 %.1f(KB/s)\n",flux);  
		}
		else if((tr >= OMB) && (tr < OGB)){
			flux = (double)tr/OMB;
			vty_out(vty,"Transmit Rate:	 %.1f(MB/s)\n",flux);  
		}
		else{
			flux = (double)tr/OGB;
			vty_out(vty,"Transmit Rate:	 %.1f(GB/s)\n",flux);  
		}
			
			
		if( tp < OKB )
			vty_out(vty,"Throughput:  %llu(B/s)\n",tp); 
		else if((tp >= OKB) && (tp < OMB)){
			flux = (double)tp/OKB;
			vty_out(vty,"Throughput:  %.1f(KB/s)\n",flux);  
		}
		else if((tp >= OMB) && (tp < OGB)){
			flux = (double)tp/OMB;
			vty_out(vty,"Throughput:	  %.1f(MB/s)\n",flux);	
		}
		else{
			flux = (double)tp/OGB;
			vty_out(vty,"Throughput:	  %.1f(GB/s)\n",flux);	
		}
		/*
		time_t now,online_time,now_sysrun,statime;
		get_sysruntime(&now_sysrun);
		time(&now);
		
		online_time=now_sysrun-StaTime+sta_online_time;
		statime = now - online_time;*/
		vty_out(vty,"Access Time:	  %s",ctime(&sta_access_time));/*xm add*/
		
		int hour,min,sec;
		
		
		hour=online_time/3600;
		min=(online_time-hour*3600)/60;
		sec=(online_time-hour*3600)%60;
		
		vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
		vty_out(vty,"==============================================================================\n");
		
	}else if (ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist\n");

	return CMD_SUCCESS;
}

#if DCLI_NEW
DEFUN(extend_show_wtp_sta_cmd_func,
	  extend_show_wtp_sta_cmd,
	  "extend show sta bywtpid WTPID",
	  CONFIG_STR
	  "extend command\n"
	  "ASD wlan station information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	 )
{	
	
	 
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;	
	struct dcli_sta_info *sta = NULL;
	
	unsigned int ret = 0;
	unsigned int wtp_id = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	wtp = extend_show_sta_bywtp(dcli_dbus_connection, index, wtp_id, localid, &ret);
	
	if((ret == 0)&&(wtp != NULL)){					
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID:           %u\n",wtp_id);
		vty_out(vty,"TOTAL DENY num:  %u\n",wtp->deny_num);
		vty_out(vty,"ACCESS TIMES:    %u\n",wtp->acc_tms);
		vty_out(vty,"USER REQUEST TIMES:    %u\n",wtp->auth_tms);
		vty_out(vty,"WTP RESPONSE TIMES:    %u\n",wtp->repauth_tms);	
		vty_out(vty,"BSS NUM:    %u\n",wtp->num_bss);
		vty_out(vty,"==============================================================================\n");

		for(i=0; i<wtp->num_bss;i++){				
			if(bss == NULL)
				bss = wtp->bss_list;					
			else 
				bss = bss->next;		
			if(bss == NULL)		
				break;
			
			for(j=0; j<bss->num_sta; j++){				
				if(sta == NULL)
					sta = bss->sta_list;					
				else 
					sta = sta->next;
				if(sta == NULL)
					break;

				vty_out(vty,"------------------------------------------------------------\n");
				vty_out(vty,"STA MAC: %02X:",sta->addr[0]);
				vty_out(vty,"%02X:",sta->addr[1]);		
				vty_out(vty,"%02X:",sta->addr[2]);
				vty_out(vty,"%02X:",sta->addr[3]);
				vty_out(vty,"%02X:",sta->addr[4]);
				vty_out(vty,"%02X\n",sta->addr[5]);
				
				vty_out(vty,"MODE:  %02X(11b-0x01,11a-0x02,11g-0x04,11n-0x08)\n",sta->mode);
				vty_out(vty,"CHANNEL:  %u\n",sta->channel);
				vty_out(vty,"RSSI:   %u\n",sta->rssi);
				vty_out(vty,"nRATE:   %u\n",sta->nRate);
				vty_out(vty,"IS POWER SAVE:   %u(0-off,1-on)\n",sta->isPowerSave);
				vty_out(vty,"IS QOS:   %u\n",sta->isQos);
				
				vty_out(vty,"SNR:     %d(dB)\n",sta->snr);
				vty_out(vty,"Rx Packets:	 %llu\n",sta->rx_pkts);
				vty_out(vty,"Tx Packets:	 %llu\n",sta->tx_pkts);		/*ht add 090220*/
				vty_out(vty,"Retry Packets: 	%llu\n",sta->rtx_pkts);
				vty_out(vty,"Error Packets: 	%llu\n",sta->err_pkts);		
				
				PRINTKMG(sta->rtx,"Retry Bytes")

				if( sta->rr < OKB )
					vty_out(vty,"Receive Rate:    %llu(B/s)\n",sta->rr); 
				else if((sta->rr >= OKB) && (sta->rr < OMB))
				{
					sta->flux = (double)sta->rr/OKB;
					vty_out(vty,"Receive Rate:    %.1f(KB/s)\n",sta->flux);  
				}
				else if((sta->rr >= OMB) && (sta->rr < OGB))
				{
					sta->flux = (double)sta->rr/OMB;
					vty_out(vty,"Receive Rate:	  %.1f(MB/s)\n",sta->flux);	
				}
				else
				{
					sta->flux = (double)sta->rr/OGB;
					vty_out(vty,"Receive Rate:	  %.1f(GB/s)\n",sta->flux);	
				}
				
				if( sta->tr < OKB )
					vty_out(vty,"Transmit Rate:	 %llu(B/s)\n",sta->tr); 
				else if((sta->tr >= OKB) && (sta->tr < OMB))
				{
					sta->flux = (double)sta->tr/OKB;
					vty_out(vty,"Transmit Rate:	 %.1f(KB/s)\n",sta->flux);  
				}
				else if((sta->tr >= OMB) && (sta->tr < OGB))
				{
					sta->flux = (double)sta->tr/OMB;
					vty_out(vty,"Transmit Rate:	 %.1f(MB/s)\n",sta->flux);  
				}
			 	else
				{
					sta->flux = (double)sta->tr/OGB;
					vty_out(vty,"Transmit Rate:	 %.1f(GB/s)\n",sta->flux);  
				}
								
				if( sta->tp < OKB )
					vty_out(vty,"Throughput:  %llu(B/s)\n",sta->tp); 
				else if((sta->tp >= OKB) && (sta->tp < OMB))
				{
					sta->flux = (double)sta->tp/OKB;
					vty_out(vty,"Throughput:  %.1f(KB/s)\n",sta->flux);  
				}
				else if((sta->tp >= OMB) && (sta->tp < OGB))
				{
					sta->flux = (double)sta->tp/OMB;
					vty_out(vty,"Throughput:	  %.1f(MB/s)\n",sta->flux);	
				}
				else
				{
					sta->flux = (double)sta->tp/OGB;
					vty_out(vty,"Throughput:	  %.1f(GB/s)\n",sta->flux);	
				}

				vty_out(vty,"------------------------------------------------------------\n");
				
			}
		
			vty_out(vty,"==============================================================================\n");

		}		
		
		if(wtp->num_bss == 0)
			vty_out(vty,"WTP %d does not provide service\n",wtp_id);
	}
	else if (ret == ASD_WTP_NOT_EXIST)	
	{
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}

	dcli_free_wtp(wtp);
	return CMD_SUCCESS;
}

#else
DEFUN(extend_show_wtp_sta_cmd_func,
	  extend_show_wtp_sta_cmd,
	  "extend show sta bywtpid WTPID",
	  CONFIG_STR
	  "extend command\n"
	  "ASD wlan station information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned int bssnum = 0, stanum = 0, wtp_id = 0;	
	unsigned int deny_num = 0;
	unsigned int bss_deny_num = 0,acc_tms=0,auth_tms=0,repauth_tms=0;
	unsigned int snr=0;

	unsigned char mode=0;  /*11b-0x01,11a-0x02,11g-0x04,11n-0x08,*/
	unsigned char channel=0;
	unsigned char rssi=0;
	unsigned short nRate=0;
	unsigned char isPowerSave=0;
	unsigned char isQos=0;
	
	unsigned long long rr=0,tr=0,tp=0;
	unsigned long long rx_pkts=0, tx_pkts=0, rtx=0, rtx_pkts, err_pkts=0;

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}


	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(acc_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(auth_tms));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(repauth_tms));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(bssnum));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(deny_num));

		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID:           %u\n",wtp_id);
		vty_out(vty,"TOTAL DENY num:  %u\n",deny_num);
		vty_out(vty,"ACCESS TIMES:    %u\n",acc_tms);
		vty_out(vty,"USER REQUEST TIMES:    %u\n",auth_tms);
		vty_out(vty,"WTP RESPONSE TIMES:    %u\n",repauth_tms);
		vty_out(vty,"==============================================================================\n");

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i=0; i<bssnum; i++){		
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;		
			int j;			
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(stanum));	
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

		for(j=0; j<stanum; j++){	
			DBusMessageIter iter_sub_struct;
			unsigned char mac[MAC_LEN];
			double flux=0;

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(mode));	
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(channel));	
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rssi));	
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(nRate));	
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(isPowerSave));	
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(isQos));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(snr));

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rr));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tr));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tp));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkts));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkts));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rtx));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rtx_pkts)); 
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(err_pkts)); 

			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"STA MAC: %02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X\n",mac[5]);
			
			vty_out(vty,"MODE:  %02X(11b-0x01,11a-0x02,11g-0x04,11n-0x08)\n",mode);
			vty_out(vty,"CHANNEL:  %u\n",channel);
			vty_out(vty,"RSSI:   %u\n",rssi);
			vty_out(vty,"nRATE:   %u\n",nRate);
			vty_out(vty,"IS POWER SAVE:   %u(0-off,1-on)\n",isPowerSave);
			vty_out(vty,"IS QOS:   %u\n",isQos);
			
			vty_out(vty,"SNR:     %d(dB)\n",snr);
			vty_out(vty,"Rx Packets:	 %llu\n",rx_pkts);
			vty_out(vty,"Tx Packets:	 %llu\n",tx_pkts);		/*ht add 090220*/
			vty_out(vty,"Retry Packets: 	%llu\n",rtx_pkts);
			vty_out(vty,"Error Packets: 	%llu\n",err_pkts);		
			PRINTKMG(rtx,"Retry Bytes")

			 if( rr < OKB )
				 vty_out(vty,"Receive Rate:    %llu(B/s)\n",rr); 
			 else if((rr >= OKB) && (rr < OMB))
			 {
				 flux = (double)rr/OKB;
				 vty_out(vty,"Receive Rate:    %.1f(KB/s)\n",flux);  
			}
			else if((rr >= OMB) && (rr < OGB))
			{
				flux = (double)rr/OMB;
				vty_out(vty,"Receive Rate:	  %.1f(MB/s)\n",flux);	
			}
			else
			{
				flux = (double)rr/OGB;
				vty_out(vty,"Receive Rate:	  %.1f(GB/s)\n",flux);	
			}
			
			   if( tr < OKB )
				   vty_out(vty,"Transmit Rate:	 %llu(B/s)\n",tr); 
			   else if((tr >= OKB) && (tr < OMB))
			   {
				   flux = (double)tr/OKB;
				   vty_out(vty,"Transmit Rate:	 %.1f(KB/s)\n",flux);  
			  }
			  else if((tr >= OMB) && (tr < OGB))
			  {
				  flux = (double)tr/OMB;
				  vty_out(vty,"Transmit Rate:	 %.1f(MB/s)\n",flux);  
			  }
			  else
			  {
				  flux = (double)tr/OGB;
				  vty_out(vty,"Transmit Rate:	 %.1f(GB/s)\n",flux);  
			  }
			
			
			   if( tp < OKB )
				   vty_out(vty,"Throughput:  %llu(B/s)\n",tp); 
			   else if((tp >= OKB) && (tp < OMB))
			   {
				   flux = (double)tp/OKB;
				   vty_out(vty,"Throughput:  %.1f(KB/s)\n",flux);  
			  }
			  else if((tp >= OMB) && (tp < OGB))
			  {
				  flux = (double)tp/OMB;
				  vty_out(vty,"Throughput:	  %.1f(MB/s)\n",flux);	
			  }
			  else
			  {
				  flux = (double)tp/OGB;
				  vty_out(vty,"Throughput:	  %.1f(GB/s)\n",flux);	
			  }


			vty_out(vty,"------------------------------------------------------------\n");
			
			dbus_message_iter_next(&iter_sub_array);
		}
		
		vty_out(vty,"==============================================================================\n");
		dbus_message_iter_next(&iter_array);

	}		
	
		if(bssnum == 0)
			vty_out(vty,"WTP %d does not provide service\n",wtp_id);
	}else if (ret == ASD_WTP_NOT_EXIST)	
	{
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif
/*xm add 08/10/31*/
DEFUN(kick_sta_cmd_func,
	  kick_sta_cmd,
	  "kick sta MAC",
	  "kick station\n"
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to kick\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	/*DBusMessageIter	 iter_array;*/
	DBusError err;
	unsigned char mac1[MAC_LEN];
	//unsigned int mac[MAC_LEN];
	unsigned int ret;
//qiuchen change it
	ret = wid_parse_mac_addr((char *)argv[0],&mac1);
	if (CMD_FAILURE == ret) {
			vty_out(vty,"<error> Unknown mac addr format.\n");
			return CMD_FAILURE;
	}

	/*memset(mac,0,6);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];*/	
	//end
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist, check it in the station list\n"); 
	else 
		vty_out(vty,"remove station successfully!(It won't be shown if it's not in the list)\n"); //qiuchen change it 

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2011.6.3 for force sta downline after receive radius msg from radius server
DEFUN(set_acct_id_sta_down_cmd_func,
	  set_acct_id_sta_down_cmd,
	  "set acct-id ID sta down",
	  "kick station\n"
	  "ASD station information\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int len = 0;
	//int acct_id = 0;
	unsigned int ret;

	char *acct_id = NULL;

	len = strlen(argv[0]);
	if(len > 18){		
		vty_out(vty,"<error> acct_id is too long,out of the limit of 18\n");
		return CMD_SUCCESS;
	}
	acct_id = (char*)malloc(strlen(argv[0])+1);
	memset(acct_id, 0, strlen(argv[0])+1);
	memcpy(acct_id, argv[0], strlen(argv[0]));		
	
/*
	ret = parse2_int_ID((char*)argv[0], &acct_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown acct_id format\n");
		return CMD_SUCCESS;
	}	
*/
	int index = 0;
	int localid = 1;  
	int slot_id = HostSlotId;   
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;		
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_RADIUS_FORCE_STA_DOWNLINE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						  	 DBUS_TYPE_STRING,&acct_id,
							 // DBUS_TYPE_UINT32,&acct_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	//qiuchen add it
	free(acct_id);
	acct_id = NULL;
	//end
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist, check it in the station list\n"); 
	else if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"remove station successfully!\n"); 
	else
		vty_out(vty,"<error> ret = %d\n",ret);

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
//

#if DCLI_NEW
DEFUN(show_sta_cmd_func,
	  show_sta_cmd,
	  "show sta MAC [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to check\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_sta_info *sta = NULL;
	unsigned char mac1[MAC_LEN],ieee80211_state[20], PAE[20], BACKEND[20];
	unsigned int mac[6];
	unsigned int ret;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char SecurityType[20];		//mahz add 2011.3.1
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int i = 0;
	
	memset(ieee80211_state, 0, 20);
	memset(PAE, 0, 20);
	memset(BACKEND, 0, 20);
	memset(mac,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		sta = get_sta_info_by_mac(dcli_dbus_connection,index,mac1, localid,&ret);
		if((ret == 0) && (sta != NULL)){		
			sta->radio_l_id = sta->radio_g_id%L_RADIO_NUM;
			sta->wtp_id = sta->radio_g_id/L_RADIO_NUM;
			asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);

			memset(SecurityType, 0, 20);	//mahz add 2011.3.1
			CheckSecurityType(SecurityType, sta->security_type);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
			vty_out(vty,"WLANID: %d\n",sta->wlan_id);
			vty_out(vty,"WTPID: %d\n",sta->wtp_id);
			vty_out(vty,"Radio_L_ID: %d\n",sta->radio_l_id);		
			vty_out(vty,"Radio_G_ID: %d\n",sta->radio_g_id);		
			vty_out(vty,"BSSIndex: %d\n",sta->bssindex);
			vty_out(vty,"SecurityID: %d\n",sta->security_id);
			vty_out(vty,"Vlan_ID: %d\n",sta->vlan_id);
			vty_out(vty,"Sta Access Security Type: %s\n",SecurityType);		//mahz add 2011.3.1
			vty_out(vty,"STA IEEE80211 State: %s\n",ieee80211_state);
			vty_out(vty,"PAE_STATE: %s\n",PAE);
			vty_out(vty,"BACKEND_STATE: %s\n",BACKEND);	
			/*
			time_t now,online_time,now_sysrun,statime;
			time(&now);
			get_sysruntime(&now_sysrun);
			online_time=now_sysrun-sta->StaTime+sta->sta_online_time;
			statime = now - online_time;*/
			
			vty_out(vty,"Access Time:	%s",ctime(&sta->sta_access_time));/*xm add*/
			vty_out(vty,"sta_traffic_limit: %d\n",sta->sta_traffic_limit);/*nl add*/
			vty_out(vty,"sta_send_traffic_limit: %d\n",sta->sta_send_traffic_limit);

#if 1		
			vty_out(vty,"ip:  %s\n",sta->ip);
			vty_out(vty,"ipv6: ");
			/* add ipv6 address */
			for (i = 0; i < 8; i++)
        	{   
				if(i==7)
				{
                    vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[i]);
				}
			    else
			    {
                    vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[i]);
			    }
        	}

			vty_out(vty,"snr:  %llu\n",sta->snr);
			vty_out(vty,"rr:  %llu\n",sta->rr);
			vty_out(vty,"tr:  %llu\n",sta->tr);
			vty_out(vty,"tp:  %llu\n",sta->tp);
			vty_out(vty,"rxbytes:  %llu\n",sta->rxbytes);
			vty_out(vty,"txbytes:  %llu\n",sta->txbytes);
			vty_out(vty,"rxpackets:  %llu\n",sta->rxpackets);
			vty_out(vty,"txpackets:  %llu\n",sta->txpackets);
			
			vty_out(vty,"retrybytes:  %llu\n",sta->retrybytes);
			vty_out(vty,"retrypackets:  %llu\n",sta->retrypackets);
			vty_out(vty,"errpackets:  %llu\n",sta->errpackets);

			vty_out(vty,"mode:	%d\n",sta->mode);  		
			vty_out(vty,"channel:	%d\n",sta->channel);  
			vty_out(vty,"rssi:	%d\n",sta->rssi);  
			vty_out(vty,"nRate:	%d\n",sta->nRate);  
			vty_out(vty,"isPowerSave:	%d\n",sta->isPowerSave);  
			vty_out(vty,"isQos:	%d\n",sta->isQos);  		
			vty_out(vty,"info_channel:	%d\n",sta->info_channel);  

#endif
			vty_out(vty,"Wapi Version:              %d\n",sta->wapi_version);
			vty_out(vty,"Controlled Port State:	    %d\n",sta->controlled_port_status);
			vty_out(vty,"Selected Unicast Cipher:   %02X:%02X:%02X:%02X\n",sta->selected_unicast_cipher[0],sta->selected_unicast_cipher[1],sta->selected_unicast_cipher[2],sta->selected_unicast_cipher[3]);
			vty_out(vty,"Wapi Replay Counters:      %d\n",sta->wpi_replay_counters);
			vty_out(vty,"Wapi Decryptable Errors:   %d\n",sta->wpi_decryptable_errors);
			vty_out(vty,"Wai Mic Errors:            %d\n",sta->wpi_mic_errors);
			vty_out(vty,"Wai Sign Errors:           %d\n",sta->wai_sign_errors);
			vty_out(vty,"Wai Hmac Errors:           %d\n",sta->wai_hmac_errors);
			vty_out(vty,"Wai Auth Fail:	            %d\n",sta->wai_auth_res_fail);
			vty_out(vty,"Wai Discard Counters:      %d\n",sta->wai_discard);
			vty_out(vty,"Wai Timeout Counters:      %d\n",sta->wai_timeout);
			vty_out(vty,"Wai Format Errors:         %d\n",sta->wai_format_errors);
			vty_out(vty,"Wai Cert Handshake Fail:   %d\n",sta->wai_cert_handshake_fail);
			vty_out(vty,"Wai Unicast Handshake Fail:%d\n",sta->wai_unicast_handshake_fail);
			vty_out(vty,"Wai Multi Handshake Fail:  %d\n",sta->wai_multi_handshake_fail);
						
			time_t online_time = sta->sta_online_time_new;
			int hour,min,sec;

			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
			
			vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
			vty_out(vty,"==============================================================================\n");	
		}
		
		else if (ret == ASD_STA_NOT_EXIST)
			vty_out(vty,"<error> station does not exist\n");
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
		
		dcli_free_sta(sta);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		 hansi_parameter:
				sta = get_sta_info_by_mac(dcli_dbus_connection,profile,mac1, localid,&ret);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (sta != NULL)){		
					sta->radio_l_id = sta->radio_g_id%L_RADIO_NUM;
					sta->wtp_id = sta->radio_g_id/L_RADIO_NUM;
					asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);
			
					memset(SecurityType, 0, 20);	//mahz add 2011.3.1
					CheckSecurityType(SecurityType, sta->security_type);
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"WLANID: %d\n",sta->wlan_id);
					vty_out(vty,"WTPID: %d\n",sta->wtp_id);
					vty_out(vty,"Radio_L_ID: %d\n",sta->radio_l_id);		
					vty_out(vty,"Radio_G_ID: %d\n",sta->radio_g_id);		
					vty_out(vty,"BSSIndex: %d\n",sta->bssindex);
					vty_out(vty,"SecurityID: %d\n",sta->security_id);
					vty_out(vty,"Vlan_ID: %d\n",sta->vlan_id);
					vty_out(vty,"Sta Access Security Type: %s\n",SecurityType); 	//mahz add 2011.3.1
					vty_out(vty,"STA IEEE80211 State: %s\n",ieee80211_state);
					vty_out(vty,"PAE_STATE: %s\n",PAE);
					vty_out(vty,"BACKEND_STATE: %s\n",BACKEND); 	
					//qiuchen change it
					time_t now,online_time,now_sysrun,statime;
					time(&now);
					get_sysruntime(&now_sysrun);
					online_time=now_sysrun-sta->StaTime+sta->sta_online_time;
					statime = now - online_time;
					//end
					vty_out(vty,"Access Time:	%s",ctime(&statime));/*xm add*/
					vty_out(vty,"sta_traffic_limit: %d\n",sta->sta_traffic_limit);/*nl add*/
					vty_out(vty,"sta_send_traffic_limit: %d\n",sta->sta_send_traffic_limit);
			
#if 1		
					vty_out(vty,"ip:  %s\n",sta->ip);
					vty_out(vty,"snr:  %llu\n",sta->snr);
					vty_out(vty,"rr:  %llu\n",sta->rr);
					vty_out(vty,"tr:  %llu\n",sta->tr);
					vty_out(vty,"tp:  %llu\n",sta->tp);
					vty_out(vty,"rxbytes:  %llu\n",sta->rxbytes);
					vty_out(vty,"txbytes:  %llu\n",sta->txbytes);
					vty_out(vty,"rxpackets:  %llu\n",sta->rxpackets);
					vty_out(vty,"txpackets:  %llu\n",sta->txpackets);
					
					vty_out(vty,"retrybytes:  %llu\n",sta->retrybytes);
					vty_out(vty,"retrypackets:	%llu\n",sta->retrypackets);
					vty_out(vty,"errpackets:  %llu\n",sta->errpackets);
			
					vty_out(vty,"mode:	%d\n",sta->mode);		
					vty_out(vty,"channel:	%d\n",sta->channel);  
					vty_out(vty,"rssi:	%d\n",sta->rssi);  
					vty_out(vty,"nRate: %d\n",sta->nRate);	
					vty_out(vty,"isPowerSave:	%d\n",sta->isPowerSave);  
					vty_out(vty,"isQos: %d\n",sta->isQos);			
					vty_out(vty,"info_channel:	%d\n",sta->info_channel);  
			
#endif
					vty_out(vty,"Wapi Version:				%d\n",sta->wapi_version);
					vty_out(vty,"Controlled Port State: 	%d\n",sta->controlled_port_status);
					vty_out(vty,"Selected Unicast Cipher:	%02X:%02X:%02X:%02X\n",sta->selected_unicast_cipher[0],sta->selected_unicast_cipher[1],sta->selected_unicast_cipher[2],sta->selected_unicast_cipher[3]);
					vty_out(vty,"Wapi Replay Counters:		%d\n",sta->wpi_replay_counters);
					vty_out(vty,"Wapi Decryptable Errors:	%d\n",sta->wpi_decryptable_errors);
					vty_out(vty,"Wai Mic Errors:			%d\n",sta->wpi_mic_errors);
					vty_out(vty,"Wai Sign Errors:			%d\n",sta->wai_sign_errors);
					vty_out(vty,"Wai Hmac Errors:			%d\n",sta->wai_hmac_errors);
					vty_out(vty,"Wai Auth Fail: 			%d\n",sta->wai_auth_res_fail);
					vty_out(vty,"Wai Discard Counters:		%d\n",sta->wai_discard);
					vty_out(vty,"Wai Timeout Counters:		%d\n",sta->wai_timeout);
					vty_out(vty,"Wai Format Errors: 		%d\n",sta->wai_format_errors);
					vty_out(vty,"Wai Cert Handshake Fail:	%d\n",sta->wai_cert_handshake_fail);
					vty_out(vty,"Wai Unicast Handshake Fail:%d\n",sta->wai_unicast_handshake_fail);
					vty_out(vty,"Wai Multi Handshake Fail:	%d\n",sta->wai_multi_handshake_fail);
								
					int hour,min,sec;
			
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					
					vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
				}
				
				else if (ret == ASD_STA_NOT_EXIST)
					vty_out(vty,"<error> station does not exist\n");
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				
				dcli_free_sta(sta);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}

	 	local_hansi_parameter:
	
				sta = get_sta_info_by_mac(dcli_dbus_connection,profile,mac1, localid,&ret);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (sta != NULL)){		
					sta->radio_l_id = sta->radio_g_id%L_RADIO_NUM;
					sta->wtp_id = sta->radio_g_id/L_RADIO_NUM;
					asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);
			
					memset(SecurityType, 0, 20);	//mahz add 2011.3.1
					CheckSecurityType(SecurityType, sta->security_type);
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"WLANID: %d\n",sta->wlan_id);
					vty_out(vty,"WTPID: %d\n",sta->wtp_id);
					vty_out(vty,"Radio_L_ID: %d\n",sta->radio_l_id);		
					vty_out(vty,"Radio_G_ID: %d\n",sta->radio_g_id);		
					vty_out(vty,"BSSIndex: %d\n",sta->bssindex);
					vty_out(vty,"SecurityID: %d\n",sta->security_id);
					vty_out(vty,"Vlan_ID: %d\n",sta->vlan_id);
					vty_out(vty,"Sta Access Security Type: %s\n",SecurityType); 	//mahz add 2011.3.1
					vty_out(vty,"STA IEEE80211 State: %s\n",ieee80211_state);
					vty_out(vty,"PAE_STATE: %s\n",PAE);
					vty_out(vty,"BACKEND_STATE: %s\n",BACKEND); 
					//qiuchen add it
					time_t now,online_time,now_sysrun,statime;
					time(&now);
					get_sysruntime(&now_sysrun);
					online_time=now_sysrun-sta->StaTime+sta->sta_online_time;
					statime = now - online_time;
					//end
					vty_out(vty,"Access Time:	%s",ctime(&statime));/*xm add*/
					vty_out(vty,"sta_traffic_limit: %d\n",sta->sta_traffic_limit);/*nl add*/
					vty_out(vty,"sta_send_traffic_limit: %d\n",sta->sta_send_traffic_limit);
			
#if 1		
					vty_out(vty,"ip:  %s\n",sta->ip);
					vty_out(vty,"snr:  %llu\n",sta->snr);
					vty_out(vty,"rr:  %llu\n",sta->rr);
					vty_out(vty,"tr:  %llu\n",sta->tr);
					vty_out(vty,"tp:  %llu\n",sta->tp);
					vty_out(vty,"rxbytes:  %llu\n",sta->rxbytes);
					vty_out(vty,"txbytes:  %llu\n",sta->txbytes);
					vty_out(vty,"rxpackets:  %llu\n",sta->rxpackets);
					vty_out(vty,"txpackets:  %llu\n",sta->txpackets);
					
					vty_out(vty,"retrybytes:  %llu\n",sta->retrybytes);
					vty_out(vty,"retrypackets:	%llu\n",sta->retrypackets);
					vty_out(vty,"errpackets:  %llu\n",sta->errpackets);
			
					vty_out(vty,"mode:	%d\n",sta->mode);		
					vty_out(vty,"channel:	%d\n",sta->channel);  
					vty_out(vty,"rssi:	%d\n",sta->rssi);  
					vty_out(vty,"nRate: %d\n",sta->nRate);	
					vty_out(vty,"isPowerSave:	%d\n",sta->isPowerSave);  
					vty_out(vty,"isQos: %d\n",sta->isQos);			
					vty_out(vty,"info_channel:	%d\n",sta->info_channel);  
			
#endif
					vty_out(vty,"Wapi Version:				%d\n",sta->wapi_version);
					vty_out(vty,"Controlled Port State: 	%d\n",sta->controlled_port_status);
					vty_out(vty,"Selected Unicast Cipher:	%02X:%02X:%02X:%02X\n",sta->selected_unicast_cipher[0],sta->selected_unicast_cipher[1],sta->selected_unicast_cipher[2],sta->selected_unicast_cipher[3]);
					vty_out(vty,"Wapi Replay Counters:		%d\n",sta->wpi_replay_counters);
					vty_out(vty,"Wapi Decryptable Errors:	%d\n",sta->wpi_decryptable_errors);
					vty_out(vty,"Wai Mic Errors:			%d\n",sta->wpi_mic_errors);
					vty_out(vty,"Wai Sign Errors:			%d\n",sta->wai_sign_errors);
					vty_out(vty,"Wai Hmac Errors:			%d\n",sta->wai_hmac_errors);
					vty_out(vty,"Wai Auth Fail: 			%d\n",sta->wai_auth_res_fail);
					vty_out(vty,"Wai Discard Counters:		%d\n",sta->wai_discard);
					vty_out(vty,"Wai Timeout Counters:		%d\n",sta->wai_timeout);
					vty_out(vty,"Wai Format Errors: 		%d\n",sta->wai_format_errors);
					vty_out(vty,"Wai Cert Handshake Fail:	%d\n",sta->wai_cert_handshake_fail);
					vty_out(vty,"Wai Unicast Handshake Fail:%d\n",sta->wai_unicast_handshake_fail);
					vty_out(vty,"Wai Multi Handshake Fail:	%d\n",sta->wai_multi_handshake_fail);
								
					int hour,min,sec;
			
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					
					vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
				}
				
				else if (ret == ASD_STA_NOT_EXIST)
					vty_out(vty,"<error> station does not exist\n");
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				
				dcli_free_sta(sta);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;
}
#else
DEFUN(show_sta_cmd_func,
	  show_sta_cmd,
	  "show sta MAC",
	  CONFIG_STR
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to check\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned char mac1[MAC_LEN],ieee80211_state[20], PAE[20], BACKEND[20];
	unsigned int mac[6];
	unsigned char WlanID,Security_ID,Radio_L_ID;
	unsigned int Radio_G_ID,BSSIndex,WTPID,vlan_id;
	unsigned int ret,sta_flags, pae_state, backend_state;
	unsigned int sta_traffic_limit=0;
	unsigned int sta_send_traffic_limit=0;
	/*char * StaTime;*/
	time_t StaTime,sta_online_time;//qiuchen add it
	/*unsigned int time_hi,time_lo;*/
	
	memset(ieee80211_state, 0, 20);
	memset(PAE, 0, 20);
	memset(BACKEND, 0, 20);
	memset(mac,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac1[0],
							 DBUS_TYPE_BYTE,&mac1[1],
							 DBUS_TYPE_BYTE,&mac1[2],
							 DBUS_TYPE_BYTE,&mac1[3],
							 DBUS_TYPE_BYTE,&mac1[4],
						   	 DBUS_TYPE_BYTE,&mac1[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(WlanID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(Radio_G_ID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(BSSIndex));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(Security_ID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(vlan_id));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_flags));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(pae_state));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(backend_state));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(StaTime));	
		
		//qiuchen add it
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_online_time));	
		//end
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_traffic_limit)); 
					
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sta_send_traffic_limit));	


		
	}	
	dbus_message_unref(reply);
	if(ret == 0){		
		Radio_L_ID = Radio_G_ID%L_RADIO_NUM;
		WTPID = Radio_G_ID/L_RADIO_NUM;
		asd_state_check(ieee80211_state,sta_flags,PAE,pae_state,BACKEND,backend_state);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"STA MAC: %02X:",mac[0]);
		vty_out(vty,"%02X:",mac[1]);		
		vty_out(vty,"%02X:",mac[2]);
		vty_out(vty,"%02X:",mac[3]);
		vty_out(vty,"%02X:",mac[4]);
		vty_out(vty,"%02X\n",mac[5]);
		vty_out(vty,"WLANID: %d\n",WlanID);
		vty_out(vty,"WTPID: %d\n",WTPID);
		vty_out(vty,"Radio_L_ID: %d\n",Radio_L_ID);		
		vty_out(vty,"Radio_G_ID: %d\n",Radio_G_ID);		
		vty_out(vty,"BSSIndex: %d\n",BSSIndex);
		vty_out(vty,"SecurityID: %d\n",Security_ID);
		vty_out(vty,"Vlan_ID: %d\n",vlan_id);
		vty_out(vty,"STA IEEE80211 State: %s\n",ieee80211_state);
		vty_out(vty,"PAE_STATE: %s\n",PAE);
		vty_out(vty,"BACKEND_STATE: %s\n",BACKEND);	
		//qiuchen change it 2012.10.31
		time_t now,online_time,statime,now_sysrun;

		time(&now);
		get_sysruntime(&now_sysrun);
		online_time=now_sysrun-StaTime+sta_online_time;
		statime = now-online_time;
		//end
		vty_out(vty,"Access Time:	%s",ctime(&statime));/*xm add*/
		vty_out(vty,"sta_traffic_limit: %d\n",sta_traffic_limit);/*nl add*/
		vty_out(vty,"sta_send_traffic_limit: %d\n",sta_send_traffic_limit);
					
		
		int hour,min,sec;


		hour=online_time/3600;
		min=(online_time-hour*3600)/60;
		sec=(online_time-hour*3600)%60;
		
		vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
		vty_out(vty,"==============================================================================\n");
		
	}else if (ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist\n");
/*	free(mac);*/
	return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
DEFUN(show_sta_v2_cmd_func,
	  show_sta_v2_cmd,
	  "show sta_v2 MAC",
	  CONFIG_STR
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to check\n"
	 )
{	
	
	int ret = 0;	
	 
	struct dcli_sta_info_v2 *sta = NULL;
	unsigned char mac1[MAC_LEN];
	unsigned int mac[6];
	unsigned char Radio_L_ID;
	unsigned int WTPID;
	char auth_type[20] = {0}; //weichao add
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int m = 0;
	memset(mac,0,MAC_LEN);	
	memset(mac1,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	sta = get_sta_info_by_mac_v2(dcli_dbus_connection,index,mac1, localid,&ret);

	if((ret == 0)&&(sta!=NULL)){//qiuchen change it 2012.10.16		
		memset(auth_type, 0, 20);	
		CheckSecurityType(auth_type, sta->auth_type);
		Radio_L_ID = sta->radio_g_id%L_RADIO_NUM;
		WTPID = sta->radio_g_id/L_RADIO_NUM;
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"STA MAC: %02X:",mac[0]);
		vty_out(vty,"%02X:",mac[1]);		
		vty_out(vty,"%02X:",mac[2]);
		vty_out(vty,"%02X:",mac[3]);
		vty_out(vty,"%02X:",mac[4]);
		vty_out(vty,"%02X\n",mac[5]);
		vty_out(vty,"WLANID: %d\n",sta->wlan_id);
		vty_out(vty,"ESSID: %s\n",sta->essid);		
		vty_out(vty,"WTPID: %d\n",WTPID);
		vty_out(vty,"WTP MAC: %02X:",sta->addr[0]);
		vty_out(vty,"%02X:",sta->addr[1]);		
		vty_out(vty,"%02X:",sta->addr[2]);
		vty_out(vty,"%02X:",sta->addr[3]);
		vty_out(vty,"%02X:",sta->addr[4]);
		vty_out(vty,"%02X\n",sta->addr[5]);
		vty_out(vty,"WTP NAME: %s\n",sta->wtp_name);		
		vty_out(vty,"Radio_L_ID: %d\n",Radio_L_ID);		
		vty_out(vty,"Radio_G_ID: %d\n",sta->radio_g_id);		
		vty_out(vty,"BSSIndex: %d\n",sta->bssindex);
		vty_out(vty,"SecurityID: %d\n",sta->security_id);
		vty_out(vty,"STA vlanid: %d\n",sta->vlan_id);
		vty_out(vty,"auth_type: %s\n",auth_type);
		if(sta->flow_check){
			vty_out(vty,"STA flow check : enable\n");
			vty_out(vty,"STA no flow time: %d\n",sta->no_flow_time);
			vty_out(vty,"STA limit flow: %d\n",sta->limit_flow);
		}
		else
			vty_out(vty,"STA flow check : disable\n");

		vty_out(vty,"Real IP: %s\n", dcli_u32ip2str(sta->realip));/* yjl 2014-2-28 */

		/* add sta ip info for ipv6 protal */
		vty_out(vty,"ipv4 address: ");
        vty_out(vty,"%d.%d.%d.%d\n",(sta->ip_addr.s_addr & 0xFF000000)>>24,(sta->ip_addr.s_addr&0xFF0000)>>16,(sta->ip_addr.s_addr&0xFF00)>>8,(sta->ip_addr.s_addr&0xFF));
		vty_out(vty,"ipv6 address: ");
		for (m = 0; m < 8; m++)
    	{   
			if(m==7)
			{
                vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[m]);
			}
		    else
		    {
                vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[m]);
		    }
    	}

		vty_out(vty,"Framed_IPv6_Prefix: ");
		for (m = 0; m < 8; m++)
    	{   
			if(m==7)
			{
                vty_out(vty,"%x/%d\n",sta->Framed_IPv6_Prefix.s6_addr16[m],sta->IPv6_Prefix_length);
			}
		    else
		    {
                vty_out(vty,"%x:",sta->Framed_IPv6_Prefix.s6_addr16[m]);
		    }
    	}
		
		vty_out(vty,"Login_IPv6_Host: ");
		for (m = 0; m < 8; m++)
    	{   
			if(m==7)
			{
                vty_out(vty,"%x\n",sta->Login_IPv6_Host.s6_addr16[m]);
			}
		    else
		    {
                vty_out(vty,"%x:",sta->Login_IPv6_Host.s6_addr16[m]);
		    }
    	}
		
		vty_out(vty,"Framed_Interface_Id: ");
        vty_out(vty,"0x%llx\n",sta->Framed_Interface_Id);

		vty_out(vty,"==============================================================================\n");
		dcli_free_sta_v2(sta);
	}else if (ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist\n");
/*	free(mac);*/
	return CMD_SUCCESS;
}

#else
DEFUN(show_sta_v2_cmd_func,
	  show_sta_v2_cmd,
	  "show sta_v2 MAC",
	  CONFIG_STR
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to check\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = 0;
	unsigned char mac1[MAC_LEN], mac2[MAC_LEN];
	unsigned int mac[6];
	unsigned char WlanID,Security_ID,Radio_L_ID;
	unsigned int Radio_G_ID,BSSIndex,WTPID;
	char *essid = NULL;
	int vlanid = 0; 
	memset(mac,0,MAC_LEN);	
	memset(mac1,0,MAC_LEN);
	memset(mac2,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_V2);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_V2);
*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac1[0],
							 DBUS_TYPE_BYTE,&mac1[1],
							 DBUS_TYPE_BYTE,&mac1[2],
							 DBUS_TYPE_BYTE,&mac1[3],
							 DBUS_TYPE_BYTE,&mac1[4],
						   	 DBUS_TYPE_BYTE,&mac1[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(WlanID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(Radio_G_ID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(BSSIndex));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(Security_ID));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(vlanid));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac2[0]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac2[1]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac2[2]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac2[3]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac2[4]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mac2[5]));	
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(essid));	
		
	}	
	dbus_message_unref(reply);
	if(ret == 0){		
		Radio_L_ID = Radio_G_ID%L_RADIO_NUM;
		WTPID = Radio_G_ID/L_RADIO_NUM;
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"STA MAC: %02X:",mac[0]);
		vty_out(vty,"%02X:",mac[1]);		
		vty_out(vty,"%02X:",mac[2]);
		vty_out(vty,"%02X:",mac[3]);
		vty_out(vty,"%02X:",mac[4]);
		vty_out(vty,"%02X\n",mac[5]);
		vty_out(vty,"WLANID: %d\n",WlanID);
		vty_out(vty,"ESSID: %s\n",essid);		
		vty_out(vty,"WTPID: %d\n",WTPID);
		vty_out(vty,"WTP MAC: %02X:",mac2[0]);
		vty_out(vty,"%02X:",mac2[1]);		
		vty_out(vty,"%02X:",mac2[2]);
		vty_out(vty,"%02X:",mac2[3]);
		vty_out(vty,"%02X:",mac2[4]);
		vty_out(vty,"%02X\n",mac2[5]);
		vty_out(vty,"Radio_L_ID: %d\n",Radio_L_ID);		
		vty_out(vty,"Radio_G_ID: %d\n",Radio_G_ID);		
		vty_out(vty,"BSSIndex: %d\n",BSSIndex);
		vty_out(vty,"SecurityID: %d\n",Security_ID);
		vty_out(vty,"STA vlanid: %d\n",vlanid);
		vty_out(vty,"==============================================================================\n");
		
	}else if (ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist\n");
/*	free(mac);*/
	return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
#ifdef __ASD_STA_ACL
/* caojia add for sta acl */
DEFUN(set_sta_acl_cmd_func,
		set_sta_acl_cmd,
		"set sta MAC acl <0-2048>",
		"Config STA\n"
		"STA information\n"
		"STA MAC\n"
		"STA ACL policy\n"
		"STA ACL policy ID <0-2048>\n"		
	 )
{	
	DBusConnection *dcli_dbus_connection = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	unsigned int index = 0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned int acl_id = 0;
	MACADDR haddr;

	memset(&haddr, 0, sizeof(haddr));
	if (parse_mac_addr(argv[0], &haddr))
	{
		vty_out(vty,"%% Invalid MAC %s\n", argv[0]);
		return CMD_SUCCESS;		
	}
    acl_id = strtoul(argv[1], NULL, 10);
	if (acl_id < 0 || acl_id > 2048) {
		vty_out(vty, "ACL ID [%d] out of range <0-2048>\n", acl_id);
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_asd_set_sta_acl(dcli_dbus_connection, index, localid, &haddr.arEther[0], acl_id);
	if (ASD_STA_NOT_EXIST == ret) {
		vty_out(vty, "<error> STA %02x:%02x:%02x:%02x:%02x:%02x not exist.\n", 
			haddr.arEther[0], haddr.arEther[1], haddr.arEther[2], 
			haddr.arEther[3], haddr.arEther[4], haddr.arEther[5]);
	}
	else if (ASD_DBUS_SUCCESS != ret)
	{
		vty_out(vty,"<error> set sta acl failed %d.\n", ret);
	}

	return CMD_SUCCESS;
}

/* caojia add for sta acl */
DEFUN(show_sta_acl_cmd_func,
	show_sta_acl_cmd,
	"show sta MAC acl",
	CONFIG_STR
	"Station\n"
	"Station MAC\n"
	"Station ACL policy \n")
{
	DBusConnection *dcli_dbus_connection = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	unsigned int index = 0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	MACADDR haddr;
	struct dcli_asd_acl acl;

	memset(&haddr, 0, sizeof(haddr));
	if (parse_mac_addr(argv[0], &haddr))
	{
		vty_out(vty,"%% Invalid MAC %s\n", argv[0]);
		return CMD_SUCCESS; 	
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	memset(&acl, 0, sizeof(acl));
	
	ret = dcli_asd_show_sta_acl(dcli_dbus_connection, index, localid, &haddr.arEther[0], &acl);
	if (ASD_DBUS_SUCCESS != ret)
	{
		vty_out(vty,"<error> show sta acl failed retval %d.\n", ret);
	}
	else
	{
		vty_out(vty," ASD STA ACL policy ID : %-4d\n", acl.id);
		vty_out(vty,"WIFI STA ACL policy ID : %-4d\n", acl.id_wifi);
	}
	return CMD_SUCCESS;
	
}
#endif
#endif

#if DCLI_NEW
DEFUN(show_sta_summary_cmd_func,
	  show_sta_summary_cmd,
	  "show sta summary [ascend] [descend] [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station summary\n"
	  "all sta information\n"
	  "show sta ascending by wtp\n"
	  "show sta descending by wtp\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
	 
	struct dcli_ac_info 	*ac = NULL;
	struct dcli_wtp_info 	*wtp = NULL;
	struct dcli_wlan_info 	*wlan = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	unsigned char type = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if(argc == 1){
		if (!strcmp(argv[0],"ascend")){
			type = 1;
		}else if(!strcmp(argv[0],"descend")){
			type = 0;
		}else{
			vty_out(vty,"<error>if there's only one parameter,it should be 'ascend' or 'descend'\n");
			return CMD_SUCCESS;
		}
	}
	if((argc == 4)||(argc == 5)){
		vty_out(vty,"<error>the number of input parameters is error,\n");
		vty_out(vty,"it should be '(ascend|descend) (remote|local) SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if(((!strcmp(argv[0],"ascend"))&&(!strcmp(argv[1],"descend")))||
			((!strcmp(argv[0],"ascend"))&&(!strcmp(argv[1],"remote")))||
			((!strcmp(argv[0],"ascend"))&&(!strcmp(argv[1],"local")))||
			((!strcmp(argv[0],"descend"))&&(!strcmp(argv[1],"remote")))||
			((!strcmp(argv[0],"descend"))&&(!strcmp(argv[1],"local")))||
			((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local")))){
				vty_out(vty,"<error>if there's only one parameter,it should be 'ascend' or 'descend'\n");
				vty_out(vty,"if there's two parameters,they should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
				return CMD_SUCCESS;
			}

		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
	if(argc == 3){
		if(((!strcmp(argv[0],"ascend"))&&(!strcmp(argv[1],"descend")))||
			((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local")))){
				vty_out(vty,"<error>the correct parameters should be '(ascend|descend) (remote|local) SLOTID-INSTID'\n");
				return CMD_SUCCESS;
			}

		if (!strcmp(argv[0],"ascend")){
			type = 1;
		}else if(!strcmp(argv[0],"descend")){
			type = 0;
		}else{
			vty_out(vty,"<error>the first parameter should be 'ascend' or 'descend'\n");
			return CMD_SUCCESS;
		}
		
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"<error>the second parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}		
	if(vty->node != VIEW_NODE){
		ac = show_sta_summary(dcli_dbus_connection,index, localid,&ret);
		if((ret == 0) && (ac != NULL)) {
			vty_out(vty,"                           Accessed Sta Summary\n");
			vty_out(vty,"===============================================================================\n");
			//vty_out(vty,"Total accessed sta:                   %d\n",ac->num_accessed_sta);
			vty_out(vty,"Total accessed sta now:               %d\n",ac->num_sta);
			vty_out(vty,"Local roam success times:             %d\n",ac->num_local_roam);
			vty_out(vty,"Total sta unconnect times:            %d\n",ac->num_unconnect_sta);
			vty_out(vty,"-------------------------------------------------------------------------------\n");
			vty_out(vty,"Wlan number:                          %d\n",ac->num_wlan);
			for(i=0; i<ac->num_wlan; i++){
				if(wlan == NULL)
					wlan = ac->wlan_list;
				else 
					wlan = wlan->next;
		
				if(wlan == NULL)
					break;
				vty_out(vty,"Sta under wlan %d:                     %d\n",wlan->WlanID,wlan->num_sta);
			}
			
			if(type == 1)
				ac->wtp_list = sort_sta_ascending(ac->wtp_list,ac->num_wtp);
			else
				ac->wtp_list = sort_sta_descending(ac->wtp_list,ac->num_wtp);
			vty_out(vty,"-------------------------------------------------------------------------------\n");
			vty_out(vty,"Wtp number:                           %d\n",ac->num_wtp);
			for(i=0; i<ac->num_wtp; i++){
				if(wtp == NULL)
					wtp = ac->wtp_list;
				else 
					wtp = wtp->next;
		
				if(wtp == NULL)
					break;
				vty_out(vty,"Sta under wtp %d:                      %d\n",wtp->WtpID,wtp->num_sta);
			}
			vty_out(vty,"-------------------------------------------------------------------------------\n");
			vty_out(vty,"Wireless accessing sta:               %d\n",ac->num_sta); /*this to be Modified, don't consider wired access.*/
			vty_out(vty,"===============================================================================\n");
			
			dcli_free_ac(ac);
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				ac = NULL;
				ac = show_sta_summary(dcli_dbus_connection,profile, localid,&ret);
			    vty_out(vty,"==============================================================================\n");
				if((ret == 0) && (ac != NULL)) {
					vty_out(vty,"hansi %d-%d                  Accessed Sta Summary\n",slot_id,profile);
					vty_out(vty,"==========================================================================\n");
					//vty_out(vty,"Total accessed sta:                   %d\n",ac->num_accessed_sta);
					vty_out(vty,"Total accessed sta now:               %d\n",ac->num_sta);
					vty_out(vty,"Local roam success times:             %d\n",ac->num_local_roam);
					vty_out(vty,"Total sta unconnect times:            %d\n",ac->num_unconnect_sta);
					vty_out(vty,"--------------------------------------------------------------------------\n");
					vty_out(vty,"Wlan number:                          %d\n",ac->num_wlan);
					wlan = NULL;
					for(i=0; i<ac->num_wlan; i++){
						if(wlan == NULL)
							wlan = ac->wlan_list;
						else 
							wlan = wlan->next;
				
						if(wlan == NULL)
							break;
						vty_out(vty,"Sta under wlan %d:                     %d\n",wlan->WlanID,wlan->num_sta);
					}
					if(type == 1)
						ac->wtp_list = sort_sta_ascending(ac->wtp_list,ac->num_wtp);
					else
						ac->wtp_list = sort_sta_descending(ac->wtp_list,ac->num_wtp);
					vty_out(vty,"--------------------------------------------------------------------------\n");
					vty_out(vty,"Wtp number:                           %d\n",ac->num_wtp);
					wtp = NULL;
					for(i=0; i<ac->num_wtp; i++){
						if(wtp == NULL)
							wtp = ac->wtp_list;
						else 
							wtp = wtp->next;
				
						if(wtp == NULL)
							break;
						vty_out(vty,"Sta under wtp %d:                      %d\n",wtp->WtpID,wtp->num_sta);
					}
					vty_out(vty,"--------------------------------------------------------------------------\n");
					vty_out(vty,"Wireless accessing sta:               %d\n",ac->num_sta); /*this to be Modified, don't consider wired access.*/
					
					dcli_free_ac(ac);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2 || argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	local_hansi_parameter:
			ac = NULL;
			ac = show_sta_summary(dcli_dbus_connection,profile, localid,&ret);
			vty_out(vty,"==============================================================================\n");
			if((ret == 0) && (ac != NULL)) {
				vty_out(vty,"local hansi %d-%d                Accessed Sta Summary\n",slot_id,profile);
				vty_out(vty,"==========================================================================\n");
				//vty_out(vty,"Total accessed sta:                   %d\n",ac->num_accessed_sta);
				vty_out(vty,"Total accessed sta now:               %d\n",ac->num_sta);
				vty_out(vty,"Local roam success times:             %d\n",ac->num_local_roam);
				vty_out(vty,"Total sta unconnect times:            %d\n",ac->num_unconnect_sta);
				vty_out(vty,"--------------------------------------------------------------------------\n");
				vty_out(vty,"Wlan number:                          %d\n",ac->num_wlan);
				wlan = NULL;
				for(i=0; i<ac->num_wlan; i++){
					if(wlan == NULL)
						wlan = ac->wlan_list;
					else 
						wlan = wlan->next;
			
					if(wlan == NULL)
						break;
					vty_out(vty,"Sta under wlan %d:                     %d\n",wlan->WlanID,wlan->num_sta);
				}
				if(type == 1)
					ac->wtp_list = sort_sta_ascending(ac->wtp_list,ac->num_wtp);
				else
					ac->wtp_list = sort_sta_descending(ac->wtp_list,ac->num_wtp);
				vty_out(vty,"--------------------------------------------------------------------------\n");
				vty_out(vty,"Wtp number:                           %d\n",ac->num_wtp);
				wtp = NULL;
				for(i=0; i<ac->num_wtp; i++){
					if(wtp == NULL)
						wtp = ac->wtp_list;
					else 
						wtp = wtp->next;
			
					if(wtp == NULL)
						break;
					vty_out(vty,"Sta under wtp %d:                      %d\n",wtp->WtpID,wtp->num_sta);
				}
				vty_out(vty,"--------------------------------------------------------------------------\n");
				vty_out(vty,"Wireless accessing sta:               %d\n",ac->num_sta); /*this to be Modified, don't consider wired access.*/
				
				dcli_free_ac(ac);
			}	
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2 || argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;
}
#else
/*xm 08/10/24*/
DEFUN(show_sta_summary_cmd_func,
	  show_sta_summary_cmd,
	  "show sta summary",
	  CONFIG_STR
	  "ASD station summary\n"
	  "all sta information\n"
	 )
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array,iter_array1;
	DBusError err;
	unsigned int ret;

	unsigned int total_wireless=0,wlanid_len=0,wtpid_len=0;
	unsigned char tem_wlanid=0;
	unsigned int tem_wlanid_num=0,tem_wtpid=0,tem_wtpid_num=0;
	unsigned int local_roam_count=0,total_unconnect_count=0;
	int i=0;

	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_SUMMARY);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
							ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_SUMMARY);*/
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
		
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&total_wireless);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&local_roam_count);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&total_unconnect_count);

		/*show under wlanid*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wlanid_len);
		/*create table*/
		vty_out(vty,"                           Accessed Sta Summary\n");
		vty_out(vty,"===============================================================================\n");
		vty_out(vty,"Total accessed sta:                   %d\n",total_wireless);
		vty_out(vty,"Local roam success times:             %d\n",local_roam_count);
		vty_out(vty,"Total sta unconnect times:            %d\n",total_unconnect_count);
		vty_out(vty,"-------------------------------------------------------------------------------\n");
		vty_out(vty,"Wlan number:                          %d\n",wlanid_len);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0;i<wlanid_len;i++){
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,& tem_wlanid);
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,& tem_wlanid_num);

			dbus_message_iter_next(&iter_array);
			
		vty_out(vty,"Sta under wlan %d:                    %d\n",tem_wlanid,tem_wlanid_num);
		}

		/*dbus_message_iter_recurse(&iter,&iter_array);*/
		
		vty_out(vty,"-------------------------------------------------------------------------------\n");

		/*show under wtpid*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpid_len);

		vty_out(vty,"Wtp number:                           %d\n",wtpid_len);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array1);
		
		for(i=0;i<wtpid_len;i++){

			DBusMessageIter iter_struct1;
			
			dbus_message_iter_recurse(&iter_array1,&iter_struct1);
			dbus_message_iter_get_basic(&iter_struct1,& tem_wtpid);
			
			dbus_message_iter_next(&iter_struct1);
			dbus_message_iter_get_basic(&iter_struct1,& tem_wtpid_num);

			dbus_message_iter_next(&iter_array1);
			
		vty_out(vty,"Sta under wtp %d:                    %d\n",tem_wtpid,tem_wtpid_num);
		}		
		vty_out(vty,"-------------------------------------------------------------------------------\n");
		vty_out(vty,"Wireless accessing sta:              %d\n",total_wireless); /*this to be Modified, don't consider wired access.*/
		vty_out(vty,"===============================================================================\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
DEFUN(show_sta_list_cmd_func,
	  show_sta_list_cmd,
	  "show sta list [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station list information\n"
	  "all sta\n"
	  "all sta\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_ac_info *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
    int m = 0;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_sta_list(dcli_dbus_connection,index, localid,&ret);

		if((ret == 0) && (ac != NULL)) {
			vty_out(vty,"Wireless sta list \n");
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Sta_MAC            IP               WTPID  Radio_ID  BSSIndex  WlanID  Sec_ID\n");
			for(i=0; i<ac->num_bss_wireless; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				
				for(j=0; j<bss->num_sta; j++){	
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					
					vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s  ",sta->ip);
					vty_out(vty,"%-5d  ",bss->WtpID);
					vty_out(vty,"%-5d     ",bss->Radio_G_ID);		
					vty_out(vty,"%-5d     ",bss->BSSIndex);
					vty_out(vty,"%-5d   ",bss->WlanID);
					vty_out(vty,"%-5d\n",bss->SecurityID);
					
					vty_out(vty,"ipv6 address:      ");
					for (m = 0; m < 8; m++)
                	{   
						if(m==7)
						{
                            vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[m]);
						}
					    else
					    {
                            vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[m]);
					    }
                	}
					
					/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
					if (ASD_MAC_TYPE_TL == bss->if_policy)
					{
						vty_out(vty,"                  %-15s(REAL IP)\n",dcli_u32ip2str(sta->realip)); 			
					}
				}
			}
			
			vty_out(vty,"==============================================================================\n");

			vty_out(vty,"\nWired sta list\n");
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Sta_MAC            IP               PortID  VlanID  SecurityID\n");
			for(i=0; i<ac->num_bss_wired; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				
				for(j=0; j<bss->num_sta; j++){	
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;

					if(sta == NULL)
						break;
					
					vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s  ",sta->ip);
					vty_out(vty,"%-5d   ",bss->PortID);
					vty_out(vty,"%-5d   ",bss->VlanID);		
					vty_out(vty,"%-5d\n",bss->SecurityID);

					/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
					if (ASD_MAC_TYPE_TL == bss->if_policy)
					{
						vty_out(vty,"                  %-15s(REAL IP)\n",dcli_u32ip2str(sta->realip)); 			
					}
				}
			}
			vty_out(vty,"==============================================================================\n");
			if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
				vty_out(vty,"<error> there is no bss\n");
			dcli_free_ac(ac);
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				ac = NULL;
				ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"------------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)) {
					vty_out(vty,"Wireless sta list \n");
					vty_out(vty,"==========================================================================\n");
					vty_out(vty,"Sta_MAC            IP               WTPID  Radio_ID  BSSIndex  WlanID  Sec_ID\n");
					bss = NULL;
					for(i=0; i<ac->num_bss_wireless; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
						
						for(j=0; j<bss->num_sta; j++){	
							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;
							
							vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
							vty_out(vty,"%-15s  ",sta->ip);
							vty_out(vty,"%-5d  ",bss->WtpID);
							vty_out(vty,"%-5d     ",bss->Radio_G_ID);		
							vty_out(vty,"%-5d     ",bss->BSSIndex);
							vty_out(vty,"%-5d   ",bss->WlanID);
							vty_out(vty,"%-5d\n",bss->SecurityID);

							/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
					        if (ASD_MAC_TYPE_TL == bss->if_policy)
					        {
						        vty_out(vty,"                  %-15s(REAL IP)\n",dcli_u32ip2str(sta->realip)); 			
					        }
						}
					}
					
					vty_out(vty,"==========================================================================\n");

					vty_out(vty,"\nWired sta list\n");
					vty_out(vty,"==========================================================================\n");
					vty_out(vty,"Sta_MAC            IP               PortID  VlanID  SecurityID\n");
					bss = NULL;
					for(i=0; i<ac->num_bss_wired; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
						
						for(j=0; j<bss->num_sta; j++){	
							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;
							
							vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
							vty_out(vty,"%-15s  ",sta->ip);
							vty_out(vty,"%-5d   ",bss->PortID);
							vty_out(vty,"%-5d   ",bss->VlanID);		
							vty_out(vty,"%-5d\n",bss->SecurityID);	
							
						}
					}
					vty_out(vty,"==========================================================================\n");
					if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
						vty_out(vty,"<error> there is no bss\n");
					dcli_free_ac(ac);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			ac = NULL;
			ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)) {
				vty_out(vty,"Wireless sta list \n");
				vty_out(vty,"==========================================================================\n");
				vty_out(vty,"Sta_MAC            IP               WTPID  Radio_ID  BSSIndex  WlanID  Sec_ID\n");
				bss = NULL;
				for(i=0; i<ac->num_bss_wireless; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					
					for(j=0; j<bss->num_sta; j++){	
						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;
						
						vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
						vty_out(vty,"%-15s  ",sta->ip);
						vty_out(vty,"%-5d  ",bss->WtpID);
						vty_out(vty,"%-5d     ",bss->Radio_G_ID);		
						vty_out(vty,"%-5d     ",bss->BSSIndex);
						vty_out(vty,"%-5d   ",bss->WlanID);
						vty_out(vty,"%-5d\n",bss->SecurityID);

						/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
					    if (ASD_MAC_TYPE_TL == bss->if_policy)
					    {
						    vty_out(vty,"                  %-15s(REAL IP)\n",dcli_u32ip2str(sta->realip)); 			
					    }
					}
				}
				
				vty_out(vty,"==========================================================================\n");

				vty_out(vty,"\nWired sta list\n");
				vty_out(vty,"==========================================================================\n");
				vty_out(vty,"Sta_MAC            IP               PortID  VlanID  SecurityID\n");
				bss = NULL;
				for(i=0; i<ac->num_bss_wired; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					
					for(j=0; j<bss->num_sta; j++){	
						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;

						if(sta == NULL)
							break;
						
						vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
						vty_out(vty,"%-15s  ",sta->ip);
						vty_out(vty,"%-5d   ",bss->PortID);
						vty_out(vty,"%-5d   ",bss->VlanID);		
						vty_out(vty,"%-5d\n",bss->SecurityID);	
						
					}
				}
				vty_out(vty,"==========================================================================\n");
				if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
					vty_out(vty,"<error> there is no bss\n");
				dcli_free_ac(ac);
			}	
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}
#else
DEFUN(show_sta_list_cmd_func,
	  show_sta_list_cmd,
	  "show sta list",
	  CONFIG_STR
	  "ASD station list information\n"
	  "all sta\n"
	 )
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned int bssnum = 0, stanum = 0, allstanum = 0, bssnum_wired=0, stanum_wired=0; /*xm add*/
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STALIST_NEW);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STALIST_NEW);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(bssnum));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	vty_out(vty,"Wireless sta list \n");
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"Sta_MAC            IP               WTPID  Radio_ID  BSSIndex  WlanID  Sec_ID\n");
	for(i=0; i<bssnum; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned char WlanID,Security_ID,Radio_L_ID;
		unsigned int Radio_G_ID,BSSIndex,WTPID;
		int j;
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(WlanID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Radio_G_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(BSSIndex));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Security_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(stanum));	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		allstanum += stanum;
		WTPID = Radio_G_ID/L_RADIO_NUM;
		Radio_L_ID = Radio_G_ID%L_RADIO_NUM;

		for(j=0; j<stanum; j++){	
			DBusMessageIter iter_sub_struct;
			unsigned char mac[MAC_LEN];
			unsigned char ip[16];
			unsigned char *in_addr = ip;

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));

			vty_out(vty,"%02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X  ",mac[5]);
			vty_out(vty,"%-15s  ",in_addr);
			vty_out(vty,"%-5d  ",WTPID);
			vty_out(vty,"%-5d     ",Radio_G_ID);		
			vty_out(vty,"%-5d     ",BSSIndex);
			vty_out(vty,"%-5d   ",WlanID);
			vty_out(vty,"%-5d\n",Security_ID);
			
			
			dbus_message_iter_next(&iter_sub_array);
		}		
		dbus_message_iter_next(&iter_array);

	}		
	vty_out(vty,"==============================================================================\n");

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(bssnum_wired));

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	vty_out(vty,"\nWired sta list\n");
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"Sta_MAC            IP               PortID  VlanID  SecurityID\n");
	for(i=0; i<bssnum_wired; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned char securityid;
		unsigned int portid,vlanid,numsta;
		int j;
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(portid));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(vlanid));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(securityid));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(numsta));
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		allstanum += numsta;
		
		for(j=0; j<numsta; j++){	
			DBusMessageIter iter_sub_struct;
			unsigned char mac[6];
			unsigned char ip[16];
			unsigned char *in_addr = ip;

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));

			vty_out(vty,"%02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X  ",mac[5]);
			vty_out(vty,"%-15s  ",in_addr);
			vty_out(vty,"%-5d   ",portid);
			vty_out(vty,"%-5d   ",vlanid);		
			vty_out(vty,"%-5d\n",securityid);
			
			dbus_message_iter_next(&iter_sub_array);
		}		
		vty_out(vty,"------------------------------------------------------------\n");
		dbus_message_iter_next(&iter_array);

	}	
	vty_out(vty,"==============================================================================\n");
	dbus_message_unref(reply);
	if(bssnum == 0&&bssnum_wired==0)
		vty_out(vty,"<error> there is no bss\n");
	return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
DEFUN(show_sta_list_by_group_cmd_func,
	  show_sta_list_by_group_cmd,
	  "show sta list by group [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station list information\n"
	  "all sta\n"
	  "all sta\n"
	  "by method\n"
	  "by group\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_ac_info *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_sta_list(dcli_dbus_connection,index, localid,&ret);

		if((ret == 0) && (ac != NULL)){
			for(i=0; i<ac->num_bss_wireless; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"WTPID:       %d\t\t",bss->WtpID);
				vty_out(vty,"BSS STA num: %d\n",bss->num_sta);
				vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
				vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
				vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
				vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
				vty_out(vty,"Assoc:       %d\t\t",bss->num_assoc);
				vty_out(vty,"Reassoc:     %d\n",bss->num_reassoc);
				vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
				vty_out(vty,"BSS uplink traffic limit:    %u kbps\n",bss->traffic_limit);
				vty_out(vty,"BSS downlink traffic limit:  %u kbps\n",bss->send_traffic_limit);
				vty_out(vty,"==============================================================================\n");
				for(j=0; j<bss->num_sta; j++){	
					unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
					
					memset(ieee80211_state, 0, 20);
					memset(PAE, 0, 20);
					memset(BACKEND, 0, 20);

					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;

					if(sta == NULL)
						break;
					//time_t 	now,online_time,now_sysrun,statime;
					int 	hour,min,sec;
					/*//qiuchen change it
					time(&now);
					get_sysruntime(&now_sysrun);
					online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
					statime = now - online_time;
					//end*/
					asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
					vty_out(vty,"------------------------------------------------------------\n");
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"STA UPLINK TRAFFIC LIMIT:        %u kbps\n",sta->sta_traffic_limit);	
					vty_out(vty,"STA DOWNLINK TRAFFIC LIMIT:      %u kbps\n",sta->sta_send_traffic_limit);	
					vty_out(vty,"STA Vlan_ID:  %d\n",sta->vlan_id);
					vty_out(vty,"STA IP:  %s\n",sta->ip);
					vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
					vty_out(vty,"PAE_STATE:		%s\n",PAE);
					vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);

					PRINTKMG(sta->rxbytes,"Receive Bytes");
					PRINTKMG(sta->txbytes,"Transmit Bytes");
				   
					vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen
					
					time_t online_time=sta->sta_online_time_new;
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
				
					vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
					vty_out(vty,"Access Type:   Wireless\n");
					vty_out(vty,"------------------------------------------------------------\n");
				}		
				vty_out(vty,"==============================================================================\n");
			}		

			vty_out(vty,"==============================================================================\n");
			for(i=0; i<ac->num_bss_wired; i++){		
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;

				vty_out(vty,"PortID:       %d\n",bss->PortID);
				vty_out(vty,"VlanID:       %d\n",bss->VlanID);
				vty_out(vty,"SecurityID:   %d\n",bss->SecurityID);
				vty_out(vty,"Vlan STA num: %d\n",bss->num_sta);
				vty_out(vty,"==============================================================================\n");
				for(j=0; j<bss->num_sta; j++){	
					unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
					
					memset(ieee80211_state, 0, 20);
					memset(PAE, 0, 20);
					memset(BACKEND, 0, 20);

					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;

					if(sta == NULL)
						break;
					
					//time_t	now,online_time,now_sysrun,statime;
					int 	hour,min,sec;
					/*//qiuchen change it
					time(&now);
					get_sysruntime(&now_sysrun);
					online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
					statime = now - online_time;
					//end*/
					asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
					vty_out(vty,"------------------------------------------------------------\n");
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"STA IP: %s\n",sta->ip);
					vty_out(vty,"PAE_STATE:		  %s\n",PAE);
					vty_out(vty,"BACKEND_STATE:	  %s\n",BACKEND);			
					

					PRINTKMG(sta->rxbytes,"Receive Bytes");
					PRINTKMG(sta->txbytes,"Transmit Bytes");
				   
					vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));/*xm add*/
					
					time_t online_time=sta->sta_online_time_new;//qiuchen change it
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
				
					vty_out(vty,"Online Time:	  %2d:%2d:%2d\n",hour,min,sec);
					vty_out(vty,"Access Type:     Wired\n");
					vty_out(vty,"------------------------------------------------------------\n");
					
				}		
				vty_out(vty,"------------------------------------------------------------\n");
			}	

			vty_out(vty,"ALL STA summary\n"); 	
			vty_out(vty,"BSS num        %d\n",ac->num_bss_wireless);
			vty_out(vty,"VLAN num       %d\n",ac->num_bss_wired);
			vty_out(vty,"STA num        %d\n",ac->num_sta_all);
			vty_out(vty,"All auth       %d\n",ac->num_auth);		
			vty_out(vty,"All auth fail  %d\n",ac->num_auth_fail);		
			vty_out(vty,"All assoc      %d\n",ac->num_assoc);		
			vty_out(vty,"All reassoc    %d\n",ac->num_reassoc);		
			vty_out(vty,"All assoc fail %d\n",ac->num_assoc_failure); 	
			vty_out(vty,"All reassoc fail %d\n",ac->num_reassoc_failure); 	
			vty_out(vty,"All normal down num %d\n",ac->num_normal_sta_down); 
			vty_out(vty,"All abnormal down num %d\n",ac->num_abnormal_sta_down);
			vty_out(vty,"==============================================================================\n");
			if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
				vty_out(vty,"<error> there is no bss\n");
			dcli_free_ac(ac);
		}
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}	

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 	hansi_parameter:
				ac = NULL;
				ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)){
					bss = NULL;
					for(i=0; i<ac->num_bss_wireless; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
						
						vty_out(vty,"=========================================================================\n");
						vty_out(vty,"WTPID:       %d\t\t",bss->WtpID);
						vty_out(vty,"BSS STA num: %d\n",bss->num_sta);
						vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
						vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
						vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
						vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
						vty_out(vty,"Assoc:       %d\t\t",bss->num_assoc);
						vty_out(vty,"Reassoc:     %d\n",bss->num_reassoc);
						vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
						vty_out(vty,"BSS uplink traffic limit:    %u kbps\n",bss->traffic_limit);
						vty_out(vty,"BSS downlink traffic limit:  %u kbps\n",bss->send_traffic_limit);
						vty_out(vty,"=========================================================================\n");
						for(j=0; j<bss->num_sta; j++){	
							unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
							
							memset(ieee80211_state, 0, 20);
							memset(PAE, 0, 20);
							memset(BACKEND, 0, 20);

							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;
							//time_t 	now,online_time,now_sysrun,statime;
							int 	hour,min,sec;
							/*//qiuchen change it
							time(&now);
							get_sysruntime(&now_sysrun);
							online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
							statime = now - online_time;
							//end*/
							
							asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
							vty_out(vty,"------------------------------------------------------------\n");
							vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
							vty_out(vty,"STA UPLINK TRAFFIC LIMIT:        %u kbps\n",sta->sta_traffic_limit);	
							vty_out(vty,"STA DOWNLINK TRAFFIC LIMIT:      %u kbps\n",sta->sta_send_traffic_limit);	
							vty_out(vty,"STA Vlan_ID:  %d\n",sta->vlan_id);
							vty_out(vty,"STA IP:  %s\n",sta->ip);
							vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
							vty_out(vty,"PAE_STATE:		%s\n",PAE);
							vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);

							PRINTKMG(sta->rxbytes,"Receive Bytes");
							PRINTKMG(sta->txbytes,"Transmit Bytes");
						   
							vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));
							
							time_t online_time=sta->sta_online_time_new;//qiuchen change it
							hour=online_time/3600;
							min=(online_time-hour*3600)/60;
							sec=(online_time-hour*3600)%60;
						
							vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
							vty_out(vty,"Access Type:   Wireless\n");
							vty_out(vty,"------------------------------------------------------------\n");
						}		
						vty_out(vty,"=========================================================================\n");
					}		

					vty_out(vty,"=========================================================================\n");
					bss = NULL;
					for(i=0; i<ac->num_bss_wired; i++){		
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;

						vty_out(vty,"PortID:       %d\n",bss->PortID);
						vty_out(vty,"VlanID:       %d\n",bss->VlanID);
						vty_out(vty,"SecurityID:   %d\n",bss->SecurityID);
						vty_out(vty,"Vlan STA num: %d\n",bss->num_sta);
						vty_out(vty,"=========================================================================\n");
						for(j=0; j<bss->num_sta; j++){	
							unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
							

							memset(ieee80211_state, 0, 20);
							memset(PAE, 0, 20);
							memset(BACKEND, 0, 20);

							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;
							//time_t 	now,online_time,now_sysrun,statime;
							int 	hour,min,sec;
							/*//qiuchen change it
							time(&now);
							get_sysruntime(&now_sysrun);
							online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
							statime = now - online_time;
							//end*/
							
							asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
							vty_out(vty,"------------------------------------------------------------\n");
							vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
							vty_out(vty,"STA IP: %s\n",sta->ip);
							vty_out(vty,"PAE_STATE:		  %s\n",PAE);
							vty_out(vty,"BACKEND_STATE:	  %s\n",BACKEND);			
							

							PRINTKMG(sta->rxbytes,"Receive Bytes");
							PRINTKMG(sta->txbytes,"Transmit Bytes");
						   
							vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));/*xm add*/
							
							time_t online_time=sta->sta_online_time_new;//qiuchen change it
							hour=online_time/3600;
							min=(online_time-hour*3600)/60;
							sec=(online_time-hour*3600)%60;
						
							vty_out(vty,"Online Time:	  %2d:%2d:%2d\n",hour,min,sec);
							vty_out(vty,"Access Type:     Wired\n");
							vty_out(vty,"------------------------------------------------------------\n");
							
						}		
						vty_out(vty,"------------------------------------------------------------\n");
					}	

					vty_out(vty,"ALL STA summary\n"); 	
					vty_out(vty,"BSS num        %d\n",ac->num_bss_wireless);
					vty_out(vty,"VLAN num       %d\n",ac->num_bss_wired);
					vty_out(vty,"STA num        %d\n",ac->num_sta_all);
					vty_out(vty,"All auth       %d\n",ac->num_auth);		
					vty_out(vty,"All auth fail  %d\n",ac->num_auth_fail);		
					vty_out(vty,"All assoc      %d\n",ac->num_assoc);		
					vty_out(vty,"All reassoc    %d\n",ac->num_reassoc);		
					vty_out(vty,"All assoc fail %d\n",ac->num_assoc_failure); 	
					vty_out(vty,"All reassoc fail %d\n",ac->num_reassoc_failure); 	
					vty_out(vty,"All normal down num %d\n",ac->num_normal_sta_down); 
					vty_out(vty,"All abnormal down num %d\n",ac->num_abnormal_sta_down);
					vty_out(vty,"=========================================================================\n");
					if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
						vty_out(vty,"<error> there is no bss\n");
					dcli_free_ac(ac);
				}
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			ac = NULL;
			ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)){
				bss = NULL;
				for(i=0; i<ac->num_bss_wireless; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					
					vty_out(vty,"=========================================================================\n");
					vty_out(vty,"WTPID:       %d\t\t",bss->WtpID);
					vty_out(vty,"BSS STA num: %d\n",bss->num_sta);
					vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
					vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
					vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
					vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
					vty_out(vty,"Assoc:       %d\t\t",bss->num_assoc);
					vty_out(vty,"Reassoc:     %d\n",bss->num_reassoc);
					vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
					vty_out(vty,"BSS uplink traffic limit:    %u kbps\n",bss->traffic_limit);
					vty_out(vty,"BSS downlink traffic limit:  %u kbps\n",bss->send_traffic_limit);
					vty_out(vty,"========================================================================\n");
					for(j=0; j<bss->num_sta; j++){	
						unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
						
						memset(ieee80211_state, 0, 20);
						memset(PAE, 0, 20);
						memset(BACKEND, 0, 20);

						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;

						if(sta == NULL)
							break;
						//time_t 	now,online_time,now_sysrun,statime;
						int 	hour,min,sec;
						/*//qiuchen change it
						time(&now);
						get_sysruntime(&now_sysrun);
						online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
						statime = now - online_time;
						//end*/
						
						asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
						vty_out(vty,"STA UPLINK TRAFFIC LIMIT:        %u kbps\n",sta->sta_traffic_limit);	
						vty_out(vty,"STA DOWNLINK TRAFFIC LIMIT:      %u kbps\n",sta->sta_send_traffic_limit);	
						vty_out(vty,"STA Vlan_ID:  %d\n",sta->vlan_id);
						vty_out(vty,"STA IP:  %s\n",sta->ip);
						vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
						vty_out(vty,"PAE_STATE:		%s\n",PAE);
						vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);

						PRINTKMG(sta->rxbytes,"Receive Bytes");
						PRINTKMG(sta->txbytes,"Transmit Bytes");
					   
						vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
						
						time_t online_time=sta->sta_online_time_new;
						hour=online_time/3600;
						min=(online_time-hour*3600)/60;
						sec=(online_time-hour*3600)%60;
					
						vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
						vty_out(vty,"Access Type:   Wireless\n");
						vty_out(vty,"------------------------------------------------------------\n");
					}		
					vty_out(vty,"=========================================================================\n");
				}		

				vty_out(vty,"=========================================================================\n");
				bss = NULL;
				for(i=0; i<ac->num_bss_wired; i++){		
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;

					vty_out(vty,"PortID:       %d\n",bss->PortID);
					vty_out(vty,"VlanID:       %d\n",bss->VlanID);
					vty_out(vty,"SecurityID:   %d\n",bss->SecurityID);
					vty_out(vty,"Vlan STA num: %d\n",bss->num_sta);
					vty_out(vty,"=========================================================================\n");
					for(j=0; j<bss->num_sta; j++){	
						unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
						

						memset(ieee80211_state, 0, 20);
						memset(PAE, 0, 20);
						memset(BACKEND, 0, 20);

						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;

						if(sta == NULL)
							break;
						//time_t 	now,online_time,now_sysrun,statime;
						int 	hour,min,sec;
						/*//qiuchen change it
						time(&now);
						get_sysruntime(&now_sysrun);
						online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
						statime = now - online_time;
						//end;*/
						
						asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
						vty_out(vty,"STA IP: %s\n",sta->ip);
						vty_out(vty,"PAE_STATE:		  %s\n",PAE);
						vty_out(vty,"BACKEND_STATE:	  %s\n",BACKEND);			
						

						PRINTKMG(sta->rxbytes,"Receive Bytes");
						PRINTKMG(sta->txbytes,"Transmit Bytes");
					   
						vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));/*xm add*/
						
						time_t online_time=sta->sta_online_time_new;//qiuchen change it
						hour=online_time/3600;
						min=(online_time-hour*3600)/60;
						sec=(online_time-hour*3600)%60;
					
						vty_out(vty,"Online Time:	  %2d:%2d:%2d\n",hour,min,sec);
						vty_out(vty,"Access Type:     Wired\n");
						vty_out(vty,"------------------------------------------------------------\n");
						
					}		
					vty_out(vty,"------------------------------------------------------------\n");
				}	

				vty_out(vty,"ALL STA summary\n"); 	
				vty_out(vty,"BSS num        %d\n",ac->num_bss_wireless);
				vty_out(vty,"VLAN num       %d\n",ac->num_bss_wired);
				vty_out(vty,"STA num        %d\n",ac->num_sta_all);
				vty_out(vty,"All auth       %d\n",ac->num_auth);		
				vty_out(vty,"All auth fail  %d\n",ac->num_auth_fail);		
				vty_out(vty,"All assoc      %d\n",ac->num_assoc);		
				vty_out(vty,"All reassoc    %d\n",ac->num_reassoc);		
				vty_out(vty,"All assoc fail %d\n",ac->num_assoc_failure); 	
				vty_out(vty,"All reassoc fail %d\n",ac->num_reassoc_failure); 	
				vty_out(vty,"All normal down num %d\n",ac->num_normal_sta_down); 
				vty_out(vty,"All abnormal down num %d\n",ac->num_abnormal_sta_down);
				vty_out(vty,"=========================================================================\n");
				if((ac->num_bss_wireless == 0) && (ac->num_bss_wired==0))
					vty_out(vty,"<error> there is no bss\n");
				dcli_free_ac(ac);
			}
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;
}
#else
DEFUN(show_sta_list_by_group_cmd_func,
	  show_sta_list_by_group_cmd,
	  "show sta list by group",
	  CONFIG_STR
	  "ASD station list information\n"
	  "all sta\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned int bssnum = 0, stanum = 0, allstanum = 0, bssnum_wired=0, stanum_wired=0; /*xm add*/
	unsigned int assoc_num = 0, reassoc_num = 0, assoc_failure_num = 0;
	unsigned int all_assoc_num = 0, all_reassoc_num = 0, all_assoc_failure_num = 0;
	unsigned int bss_traffic_limit=0,sta_traffic_limit=0;
	unsigned int bss_send_traffic_limit=0,sta_send_traffic_limit=0;	
	unsigned int normal_st_down_num=0,abnormal_st_down_num=0;//nl091125

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STALIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STALIST);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(bssnum));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(all_assoc_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(all_reassoc_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(all_assoc_failure_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(normal_st_down_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(abnormal_st_down_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for(i=0; i<bssnum; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;		
		unsigned char WlanID,Security_ID,Radio_L_ID;
		unsigned int Radio_G_ID,BSSIndex,WTPID;
		int j;
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(WlanID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(bss_traffic_limit));

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(bss_send_traffic_limit));

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Radio_G_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(BSSIndex));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Security_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(assoc_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(reassoc_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(assoc_failure_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(stanum));	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		allstanum += stanum;
		WTPID = Radio_G_ID/L_RADIO_NUM;
		Radio_L_ID = Radio_G_ID%L_RADIO_NUM;
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID:       %d\t\t",WTPID);
		vty_out(vty,"BSS STA num: %d\n",stanum);
		vty_out(vty,"Radio_G_ID:  %d\t\t",Radio_G_ID);
		vty_out(vty,"Radio_L_ID:  %d\n",Radio_L_ID);
		vty_out(vty,"BSSIndex:    %d\t\t",BSSIndex);
		vty_out(vty,"SecurityID:  %d\n",Security_ID);
		vty_out(vty,"Assoc:       %d\t\t",assoc_num);
		vty_out(vty,"Reassoc:     %d\n",reassoc_num);
		vty_out(vty,"Assoc_fail:  %d\n",assoc_failure_num);
		vty_out(vty,"BSS uplink traffic limit:    %u kbps\n",bss_traffic_limit);
		vty_out(vty,"BSS downlink traffic limit:  %u kbps\n",bss_send_traffic_limit);

		/*vty_out(vty,"WlanID:%d WTPID:%d STAnum:%d Radio_G_ID:%d Radio_L_ID:%d BSSIndex %d SecurityID %d\n",WlanID,WTPID,stanum,Radio_G_ID,Radio_L_ID,BSSIndex,Security_ID);*/
		vty_out(vty,"==============================================================================\n");
		for(j=0; j<stanum; j++){	
			DBusMessageIter iter_sub_struct;
			unsigned char mac[MAC_LEN],ieee80211_state[20], PAE[20], BACKEND[20];
			unsigned int sta_flags, pae_state, backend_state, vlan_id;
			unsigned long long rxPackets,txPackets;
			unsigned char *in_addr = NULL;
			time_t StaTime,sta_online_time;//qiuchen add it
			double	flux;
			
/*			in_addr = (unsigned char *)malloc(sizeof(unsigned char)*16);*/
			memset(ieee80211_state, 0, 20);
			memset(PAE, 0, 20);
			memset(BACKEND, 0, 20);
/*			memset(in_addr,0,16);*/

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(vlan_id));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_traffic_limit));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_send_traffic_limit));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_flags));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(pae_state));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(backend_state));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(StaTime));	

			//qiuchen add it
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_online_time));	
			//end
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rxPackets));

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(txPackets));
			
			asd_state_check(ieee80211_state,sta_flags,PAE,pae_state,BACKEND,backend_state);			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"STA MAC: %02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X\n",mac[5]);
			vty_out(vty,"STA UPLINK TRAFFIC LIMIT:        %u kbps\n",sta_traffic_limit);	
			vty_out(vty,"STA DOWNLINK TRAFFIC LIMIT:      %u kbps\n",sta_send_traffic_limit);	
			vty_out(vty,"STA Vlan_ID:  %d\n",vlan_id);
			vty_out(vty,"STA IP:  %s\n",in_addr);
			vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
			vty_out(vty,"PAE_STATE:		%s\n",PAE);
			vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);			
			if( rxPackets < OKB )
				vty_out(vty,"Receive Bytes:	  %llu\n",rxPackets);	
		   	else if((rxPackets >= OKB) && (rxPackets < OMB))
		   	{
				flux = (double)rxPackets/OKB;
				vty_out(vty,"Receive Bytes:     %llu\t(%.1fKB)\n",rxPackets,flux);	
		   }
		   else if((rxPackets >= OMB) && (rxPackets < OGB))
		   {
			   flux = (double)rxPackets/OMB;
			   vty_out(vty,"Receive Bytes:	 %llu\t(%.1fMB)\n",rxPackets,flux);  
		   }
		   else
		   {
			   flux = (double)rxPackets/OGB;
			   vty_out(vty,"Receive Bytes:	 %llu\t(%.1fGB)\n",rxPackets,flux);  
		   }

			if( txPackets < OKB )
				vty_out(vty,"Transmit Bytes:	  %llu\n",txPackets);	
		   	else if((txPackets >= OKB) && (txPackets < OMB))
		   	{
				flux = (double)txPackets/OKB;
				vty_out(vty,"Transmit Bytes:     %llu\t(%.1fKB)\n",txPackets,flux);	
		   }
		   else if((txPackets >= OMB) && (txPackets < OGB))
		   {
			   flux = (double)txPackets/OMB;
			   vty_out(vty,"Transmit Bytes:	  %llu\t(%.1fMB)\n",txPackets,flux); 
		   }
		   else
		   {
			   flux = (double)txPackets/OGB;
			   vty_out(vty,"Transmit Bytes:	  %llu\t(%.1fGB)\n",txPackets,flux); 
		   }
		   //qiuchen change it 2012.10.31
		    time_t now,online_time,now_sysrun,statime;

			time(&now);
			get_sysruntime(&now_sysrun);
			online_time=now_sysrun-StaTime+sta_online_time;
			statime = now - online_time;
			vty_out(vty,"Access Time:     %s",ctime(&statime));/*xm add*/
			
			
			int hour,min,sec;


			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
		
			vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
			vty_out(vty,"Access Type:   Wireless\n");
			vty_out(vty,"------------------------------------------------------------\n");
			
			dbus_message_iter_next(&iter_sub_array);
		}		
		vty_out(vty,"==============================================================================\n");
		dbus_message_iter_next(&iter_array);

	}		
/*===================================================================
// xm 08/10/08*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(bssnum_wired));


	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	vty_out(vty,"==============================================================================\n");
	for(i=0; i<bssnum_wired; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;		
		unsigned char securityid;/*WlanID,Security_ID,Radio_L_ID;ls*/
		unsigned int portid,vlanid,numsta;/*Radio_G_ID,BSSIndex,WTPID;*/
		int j;
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(portid));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(vlanid));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(securityid));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(numsta));
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		allstanum += numsta;
		
		
		vty_out(vty,"PortID:       %d\n",portid);
		vty_out(vty,"VlanID:       %d\n",vlanid);
		vty_out(vty,"SecurityID:   %d\n",securityid);
		vty_out(vty,"Vlan STA num: %d\n",numsta);
		/*vty_out(vty,"PORTID:%d  VLANID:%d vlan STAnum:%d SecurityID:%d\n",portid,vlanid,securityid,numsta);*/
		vty_out(vty,"==============================================================================\n");
		for(j=0; j<numsta; j++){	
			DBusMessageIter iter_sub_struct;
			unsigned char mac[6],ieee80211_state[20], PAE[20], BACKEND[20];
			unsigned int sta_flags_wired, pae_state_wired, backend_state_wired;
			unsigned long long rxPackets,txPackets;
			unsigned char ip[16];
			unsigned char *in_addr = ip;
			double	flux;
			time_t StaTime,sta_online_time;//qiuchen add it

/*			in_addr = (unsigned char *)malloc(sizeof(unsigned char)*16);*/
			memset(ieee80211_state, 0, 20);
			memset(PAE, 0, 20);
			memset(BACKEND, 0, 20);
/*			memset(in_addr,0,16);*/

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_flags_wired));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(pae_state_wired));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(backend_state_wired));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(StaTime));
			//qiuchen add it
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_online_time));	
			//end
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(in_addr));

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(rxPackets));

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(txPackets));
						
			asd_state_check(ieee80211_state,sta_flags_wired,PAE,pae_state_wired,BACKEND,backend_state_wired);			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"STA MAC: %02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X\n",mac[5]);
			vty_out(vty,"STA IP: %s\n",in_addr);
			vty_out(vty,"PAE_STATE:		  %s\n",PAE);
			vty_out(vty,"BACKEND_STATE:	  %s\n",BACKEND);			
			
			if( rxPackets < OKB )
				vty_out(vty,"Receive Packets:	  %llu\n",rxPackets);	
		   	else if((rxPackets >= OKB) && (rxPackets < OMB))
		   	{
				flux = (double)rxPackets/OKB;
				vty_out(vty,"Receive Packets:     %llu\t(%gKB)\n",rxPackets,flux);	
		   }
		   else if((rxPackets >= OMB) && (rxPackets < OGB))
		   {
			   flux = (double)rxPackets/OMB;
			   vty_out(vty,"Receive Packets:	 %llu\t(%gMB)\n",rxPackets,flux);  
		   }
		   else
		   {
			   flux = (double)rxPackets/OGB;
			   vty_out(vty,"Receive Packets:	 %llu\t(%gGB)\n",rxPackets,flux);  
		   }

			if( txPackets < OKB )
				vty_out(vty,"Transmit Packets:	  %llu\n",txPackets);	
		   	else if((txPackets >= OKB) && (txPackets < OMB))
		   	{
				flux = (double)txPackets/OKB;
				vty_out(vty,"Transmit Packets:     %llu\t(%gKB)\n",txPackets,flux);	
		   }
		   else if((txPackets >= OMB) && (txPackets < OGB))
		   {
			   flux = (double)txPackets/OMB;
			   vty_out(vty,"Transmit Packets:	  %llu\t(%gMB)\n",txPackets,flux); 
		   }
		   else
		   {
			   flux = (double)txPackets/OGB;
			   vty_out(vty,"Transmit Packets:	  %llu\t(%gGB)\n",txPackets,flux); 
		   }
		   //qiuchen change it 2012.10.31
		   	time_t now,online_time,now_sysrun,statime;

			time(&now);
			get_sysruntime(&now_sysrun);
			online_time=now_sysrun-StaTime+sta_online_time;
			statime = now - online_time;
		   
			vty_out(vty,"Access Time:     %s",ctime(&statime));/*xm add*/
			
			
			int hour,min,sec;

			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
		
			vty_out(vty,"Online Time:	  %2d:%2d:%2d\n",hour,min,sec);
			vty_out(vty,"Access Type:     Wired\n");
			vty_out(vty,"------------------------------------------------------------\n");
			
			dbus_message_iter_next(&iter_sub_array);
		}		
		vty_out(vty,"------------------------------------------------------------\n");
		dbus_message_iter_next(&iter_array);

	}	
	vty_out(vty,"ALL STA summary\n"); 	
	vty_out(vty,"BSS num        %d\n",bssnum);
	vty_out(vty,"VLAN num       %d\n",bssnum_wired);/*xm add*/
	vty_out(vty,"STA num        %d\n",allstanum);
	vty_out(vty,"All assoc      %d\n",all_assoc_num);		
	vty_out(vty,"All reassoc    %d\n",all_reassoc_num);		
	vty_out(vty,"All assoc fail %d\n",all_assoc_failure_num); 	
	vty_out(vty,"All normal down num %d\n",normal_st_down_num); 
	vty_out(vty,"All abnormal down num %d\n",abnormal_st_down_num);
	vty_out(vty,"==============================================================================\n");
	dbus_message_unref(reply);
	if(bssnum == 0&&bssnum_wired==0)
		vty_out(vty,"<error> there is no station\n");
	return CMD_SUCCESS;
}
#endif


//fengwenchao add 20110113  for dot11WlanStationTable
DEFUN(show_wlan_sta_of_all_cmd_func,
	  show_wlan_sta_of_all_cmd,
	  "show sta of all wlan",
	  CONFIG_STR
	  "ASD wlan station information\n"
	  "search by ALLWLAN\n"
	  "ALL WLAN\n"
	 )
{	
	
	 
	struct dcli_wlan_info *wlan = NULL;
	struct dcli_wlan_info *wlan_shownode = NULL;

	//unsigned char wlan_id = 0;
	unsigned int ret = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int wlan_num = 0;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	wlan = show_sta_of_allwlan(dcli_dbus_connection, index, &wlan_num, localid,&ret);
	
	if((ret == 0) && (wlan != NULL)){

		wlan_shownode = wlan;
		while(wlan_shownode != NULL)
		{
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WLAN ID : %d\n",wlan_shownode->WlanID);

			struct dcli_bss_info *bss = NULL;
			for(j=0;j<wlan_shownode->num_bss;j++)
			{
				if(bss == NULL)
					bss = wlan_shownode->bss_list;
			    else 
					bss = bss->next;
				if(bss == NULL)
					break;	

				struct dcli_sta_info *sta = NULL;
				for(k=0;k<bss->num_sta;k++)
				{
			
					int 	hour,min,sec;
					
					unsigned long long  StaUplinkBitRate = 0;      //station上行位速率
					unsigned long long  StaDownlinkBitRate = 0;    //station下行位速率
					
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					if(sta == NULL)
						break;
					//qiuchen change it 
					//time_t now,online_time,now_sysrun,statime;

					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					//statime = now - online_time;
					
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));
				
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;	

					if(online_time > 0)
					{
						StaUplinkBitRate = (sta->rxbytes)*8/online_time;
						StaDownlinkBitRate = (sta->txbytes)*8/online_time;
					}
				
					vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
					vty_out(vty,"StaUplinkTrafficLimitThreshold:   %d\n",sta->sta_traffic_limit);
					vty_out(vty,"StaDownlinkTrafficLimitThreshold:  %d\n",sta->sta_send_traffic_limit);
					vty_out(vty,"StaUplinkBitRate:  %llu\n",StaUplinkBitRate);
					vty_out(vty,"StaDownlinkBitRate:  %llu\n",StaDownlinkBitRate);
					
				}
			}
					
			wlan_shownode = wlan_shownode->next;
		}
		
		dcli_free_wlanlist(wlan);
	}
	else if (ret == ASD_WLAN_NOT_EXIST)	
		vty_out(vty,"<error> wlan does not exist\n");
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);

	return CMD_SUCCESS;
}
//fengwenchao add end
#if DCLI_NEW
DEFUN(show_wlan_sta_cmd_func,
	  show_wlan_sta_cmd,
	  "show sta bywlanid WLANID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD wlan station information\n"
	  "search by WLANID\n"
	  "WLAN ID\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	
	 
	struct dcli_wlan_info *wlan = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned char wlan_id = 0;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		wlan = show_sta_bywlan(dcli_dbus_connection, index, wlan_id, localid, &ret);
		
		if((ret == 0) && (wlan != NULL)){
			for(i=0; i<wlan->num_bss; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = wlan->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"WlanID:      %d\t\t",bss->WlanID);
				vty_out(vty,"WTPID:       %d\n",bss->WtpID);
				vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
				vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
				vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
				vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
				vty_out(vty,"BSS STA num: %d\t\t",bss->num_sta);
				vty_out(vty,"Assoc:       %d\n",bss->num_assoc);
				vty_out(vty,"Reassoc:     %d\t\t",bss->num_reassoc);
				vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
				vty_out(vty,"==============================================================================\n");
				for(j=0; j<bss->num_sta; j++){	
					unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
						
					
					memset(ieee80211_state, 0, 20);
					memset(PAE, 0, 20);
					memset(BACKEND, 0, 20);

					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;

					if(sta == NULL)
						break;
					//time_t 	now,online_time,now_sysrun,statime;
					int 	hour,min,sec;
					/*//qiuchen change it
					time(&now);
					get_sysruntime(&now_sysrun);
					online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
					statime = now - online_time;
					//end;	*/
					asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
					vty_out(vty,"------------------------------------------------------------\n");
					vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
					vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
					vty_out(vty,"PAE_STATE:		%s\n",PAE);
					vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
					vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
					
					time_t online_time=sta->sta_online_time_new;
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
				
					vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
					vty_out(vty,"------------------------------------------------------------\n");
				}	
				vty_out(vty,"==============================================================================\n");

			}		
			vty_out(vty,"WLAN %d STA summary\n",wlan_id);		
			vty_out(vty,"WLAN BSS num    %d\n",wlan->num_bss);
			vty_out(vty,"WLAN STA num    %d\n",wlan->num_sta);
			vty_out(vty,"WLAN assoc      %d\n",wlan->num_assoc);		
			vty_out(vty,"WLAN reassoc    %d\n",wlan->num_reassoc);		
			vty_out(vty,"WLAN assoc fail %d\n",wlan->num_assoc_failure);		
			vty_out(vty,"WLAN normal down num %d\n",wlan->num_normal_sta_down); 
			vty_out(vty,"WLAN abnormal down num %d\n",wlan->num_abnormal_sta_down);
			if(wlan->num_bss == 0)
				vty_out(vty,"WLAN %d does provide service\n",wlan_id);
			dcli_free_wlan(wlan);
		}else if (ret == ASD_WLAN_NOT_EXIST)	
			vty_out(vty,"<error> wlan %d does not exist\n",wlan_id);
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				wlan = NULL;
				wlan = show_sta_bywlan(dcli_dbus_connection, profile, wlan_id, localid, &ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wlan != NULL)){
					bss = NULL;
					for(i=0; i<wlan->num_bss; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = wlan->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;

						vty_out(vty,"===========================================================================\n");
						vty_out(vty,"WlanID:      %d\t\t",bss->WlanID);
						vty_out(vty,"WTPID:       %d\n",bss->WtpID);
						vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
						vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
						vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
						vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
						vty_out(vty,"BSS STA num: %d\t\t",bss->num_sta);
						vty_out(vty,"Assoc:       %d\n",bss->num_assoc);
						vty_out(vty,"Reassoc:     %d\t\t",bss->num_reassoc);
						vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
						vty_out(vty,"===========================================================================\n");
						for(j=0; j<bss->num_sta; j++){	
							unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
								
							
							memset(ieee80211_state, 0, 20);
							memset(PAE, 0, 20);
							memset(BACKEND, 0, 20);

							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;
							//time_t 	now,online_time,now_sysrun,statime;
							int 	hour,min,sec;
							/*//qiuchen change it
							time(&now);
							get_sysruntime(&now_sysrun);
							online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
							statime = now - online_time;
							//end;*/
							asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
							vty_out(vty,"------------------------------------------------------------\n");
							vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
							vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
							vty_out(vty,"PAE_STATE:		%s\n",PAE);
							vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
							vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
							
							time_t online_time=sta->sta_online_time_new;
							hour=online_time/3600;
							min=(online_time-hour*3600)/60;
							sec=(online_time-hour*3600)%60;
						
							vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
							vty_out(vty,"------------------------------------------------------------\n");
						}	
						vty_out(vty,"===========================================================================\n");

					}		
					vty_out(vty,"WLAN %d STA summary\n",wlan_id);		
					vty_out(vty,"WLAN BSS num    %d\n",wlan->num_bss);
					vty_out(vty,"WLAN STA num    %d\n",wlan->num_sta);
					vty_out(vty,"WLAN assoc      %d\n",wlan->num_assoc);		
					vty_out(vty,"WLAN reassoc    %d\n",wlan->num_reassoc);		
					vty_out(vty,"WLAN assoc fail %d\n",wlan->num_assoc_failure);		
					vty_out(vty,"WLAN normal down num %d\n",wlan->num_normal_sta_down); 
					vty_out(vty,"WLAN abnormal down num %d\n",wlan->num_abnormal_sta_down);
					if(wlan->num_bss == 0)
						vty_out(vty,"WLAN %d does provide service\n",wlan_id);
					dcli_free_wlan(wlan);
				}else if (ret == ASD_WLAN_NOT_EXIST)	
					vty_out(vty,"<error> wlan %d does not exist\n",wlan_id);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	local_hansi_parameter:
			wlan = NULL;
			wlan = show_sta_bywlan(dcli_dbus_connection, profile, wlan_id, localid, &ret);
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) && (wlan != NULL)){
				bss = NULL;
				for(i=0; i<wlan->num_bss; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = wlan->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;

					vty_out(vty,"===========================================================================\n");
					vty_out(vty,"WlanID:      %d\t\t",bss->WlanID);
					vty_out(vty,"WTPID:       %d\n",bss->WtpID);
					vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
					vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
					vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
					vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
					vty_out(vty,"BSS STA num: %d\t\t",bss->num_sta);
					vty_out(vty,"Assoc:       %d\n",bss->num_assoc);
					vty_out(vty,"Reassoc:     %d\t\t",bss->num_reassoc);
					vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
					vty_out(vty,"===========================================================================\n");
					for(j=0; j<bss->num_sta; j++){	
						unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
						
						memset(ieee80211_state, 0, 20);
						memset(PAE, 0, 20);
						memset(BACKEND, 0, 20);

						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;

						if(sta == NULL)
							break;
						//time_t 	now,online_time,now_sysrun,statime;
						int 	hour,min,sec;
						/*//qiuchen change it
						time(&now);
						get_sysruntime(&now_sysrun);
						online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
						statime = now - online_time;
						//end;	*/
						
						asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
						vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
						vty_out(vty,"PAE_STATE:		%s\n",PAE);
						vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
						vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
						
						time_t online_time=sta->sta_online_time_new;
						hour=online_time/3600;
						min=(online_time-hour*3600)/60;
						sec=(online_time-hour*3600)%60;
					
						vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
						vty_out(vty,"------------------------------------------------------------\n");
					}	
					vty_out(vty,"===========================================================================\n");

				}		
				vty_out(vty,"WLAN %d STA summary\n",wlan_id);		
				vty_out(vty,"WLAN BSS num    %d\n",wlan->num_bss);
				vty_out(vty,"WLAN STA num    %d\n",wlan->num_sta);
				vty_out(vty,"WLAN assoc      %d\n",wlan->num_assoc);		
				vty_out(vty,"WLAN reassoc    %d\n",wlan->num_reassoc);		
				vty_out(vty,"WLAN assoc fail %d\n",wlan->num_assoc_failure);		
				vty_out(vty,"WLAN normal down num %d\n",wlan->num_normal_sta_down); 
				vty_out(vty,"WLAN abnormal down num %d\n",wlan->num_abnormal_sta_down);
				if(wlan->num_bss == 0)
					vty_out(vty,"WLAN %d does provide service\n",wlan_id);
				dcli_free_wlan(wlan);
			}else if (ret == ASD_WLAN_NOT_EXIST)	
				vty_out(vty,"<error> wlan %d does not exist\n",wlan_id);
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;
}
#else
DEFUN(show_wlan_sta_cmd_func,
	  show_wlan_sta_cmd,
	  "show sta bywlanid WLANID",
	  CONFIG_STR
	  "ASD wlan station information\n"
	  "search by WLANID\n"
	  "WLAN ID\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned int bssnum = 0, stanum = 0, wlanstanum = 0;
	unsigned int assoc_num = 0, reassoc_num = 0, assoc_failure_num = 0;	
	unsigned int normal_st_down_num=0,abnormal_st_down_num=0;//nl091120
	unsigned int wlan_assoc_num = 0, wlan_reassoc_num = 0, wlan_assoc_failure_num = 0;
	unsigned char wlan_id = 0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_STALIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_STALIST);*/
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(bssnum));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wlan_assoc_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wlan_reassoc_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wlan_assoc_failure_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(normal_st_down_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(abnormal_st_down_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for(i=0; i<bssnum; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;		
		unsigned char WlanID,Security_ID,Radio_L_ID;
		unsigned int Radio_G_ID,BSSIndex,WTPID;
		int j;
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(WlanID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Radio_G_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(BSSIndex));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Security_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(assoc_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(reassoc_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(assoc_failure_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(stanum));	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		wlanstanum += stanum;
		WTPID = Radio_G_ID/L_RADIO_NUM;
		Radio_L_ID = Radio_G_ID%L_RADIO_NUM;
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WlanID:      %d\t\t",WlanID);
		vty_out(vty,"WTPID:       %d\n",WTPID);
		vty_out(vty,"BSS STA num: %d\t\t",stanum);
		vty_out(vty,"Radio_G_ID:  %d\n",Radio_G_ID);
		vty_out(vty,"Radio_L_ID:  %d\t\t",Radio_L_ID);
		vty_out(vty,"BSSIndex:    %d\n",BSSIndex);
		vty_out(vty,"SecurityID:  %d\t\t",Security_ID);
		vty_out(vty,"Assoc:       %d\n",assoc_num);
		vty_out(vty,"Reassoc:     %d\t\t",reassoc_num);
		vty_out(vty,"Assoc_fail:  %d\n",assoc_failure_num);
		vty_out(vty,"normol sta down num:  %d\n",normal_st_down_num);
		vty_out(vty,"abnormol sta down num:  %d\n",abnormal_st_down_num);
		/*vty_out(vty,"WlanID:%d WTPID:%d BSS STAnum:%d Radio_G_ID:%d Radio_L_ID:%d BSSIndex:%d SecurityID:%d\n",WlanID,WTPID,stanum,Radio_G_ID,Radio_L_ID,BSSIndex,Security_ID);*/
		vty_out(vty,"==============================================================================\n");
		for(j=0; j<stanum; j++){
			DBusMessageIter iter_sub_struct;
			unsigned char mac[6],ieee80211_state[20], PAE[20], BACKEND[20];
			unsigned int sta_flags, pae_state, backend_state;
			/*char *StaTime;*/
			time_t StaTime,sta_online_time;//qiuchen add it
			memset(ieee80211_state, 0, 20);
			memset(PAE, 0, 20);
			memset(BACKEND, 0, 20);

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_flags));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(pae_state));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(backend_state));	
			
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(StaTime));	

			//qiuchen add it
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_online_time));
			//end

			asd_state_check(ieee80211_state,sta_flags,PAE,pae_state,BACKEND,backend_state);			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"STA MAC: %02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X\n",mac[5]);	
			vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
			vty_out(vty,"PAE_STATE:		%s\n",PAE);
			vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);	
			//qiuchen change it 2012.10.31
			time_t now,online_time,now_sysrun,statime;

			time(&now);
			get_sysruntime(&now_sysrun);
			online_time=now_sysrun-sta->StaTime+sta_online_time;
			statime = now - online_time;
			vty_out(vty,"Access Time:	%s",ctime(&statime));/*xm add*/
			
			int hour,min,sec;


			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
		
			vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);

			vty_out(vty,"------------------------------------------------------------\n");
			
			dbus_message_iter_next(&iter_sub_array);
		}
		vty_out(vty,"==============================================================================\n");
		dbus_message_iter_next(&iter_array);

	}	
		vty_out(vty,"WLAN %d STA summary\n",wlan_id); 	
		vty_out(vty,"WLAN BSS num    %d\n",bssnum);
		vty_out(vty,"WLAN STA num    %d\n",wlanstanum);		
		vty_out(vty,"WLAN assoc      %d\n",wlan_assoc_num);		
		vty_out(vty,"WLAN reassoc    %d\n",wlan_reassoc_num);		
		vty_out(vty,"WLAN assoc fail %d\n",wlan_assoc_failure_num);		
		if(bssnum == 0)
			vty_out(vty,"WLAN %d does provide service\n",wlan_id);
	}else if(ret == ASD_WLAN_NOT_EXIST){	
		vty_out(vty,"<error> wlan %d does not exist\n",wlan_id); 
	}	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

#if DCLI_NEW
DEFUN(show_wtp_sta_cmd_func,
	  show_wtp_sta_cmd,
	  "show sta bywtpid WTPID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD wlan station information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	
	 
	struct dcli_wtp_info *wtp = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned int ret = 0;
	unsigned int wtp_id = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
	wtp = show_sta_bywtp(dcli_dbus_connection, index, wtp_id, localid, &ret);
	
	if((ret == 0) && (wtp != NULL)){
		for(i=0; i<wtp->num_bss; i++){
			struct dcli_sta_info *sta = NULL;
			if(bss == NULL)
				bss = wtp->bss_list;
			else 
				bss = bss->next;

			if(bss == NULL)
				break;

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WTPID:       %d\t\t",bss->WtpID);
			vty_out(vty,"BSS STA num: %d\n",bss->num_sta);
			vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
			vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
			vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
			vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
			vty_out(vty,"Assoc:       %d\t\t",bss->num_assoc);
			vty_out(vty,"Reassoc:     %d\n",bss->num_reassoc);
			vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
			vty_out(vty,"==============================================================================\n");
			for(j=0; j<bss->num_sta; j++){	
				unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
				memset(ieee80211_state, 0, 20);
				memset(PAE, 0, 20);
				memset(BACKEND, 0, 20);

				if(sta == NULL)
					sta = bss->sta_list;
				else 
					sta = sta->next;

				if(sta == NULL)
					break;

				//time_t 	now,online_time,now_sysrun,statime;
				int 	hour,min,sec;
				/*//qiuchen change it
				time(&now);
				get_sysruntime(&now_sysrun);
				online_time = now_sysrun - sta->StaTime + sta->sta_online_time;
				statime = now - online_time;
				//end;	*/				
				asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
				vty_out(vty,"------------------------------------------------------------\n");
				vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
				vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
				vty_out(vty,"PAE_STATE:		%s\n",PAE);
				vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
				vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
				
				time_t online_time=sta->sta_online_time_new;
				hour=online_time/3600;
				min=(online_time-hour*3600)/60;
				sec=(online_time-hour*3600)%60;
			
				vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
				vty_out(vty,"------------------------------------------------------------\n");
			}	
			vty_out(vty,"==============================================================================\n");

		}		
		vty_out(vty,"WTP %d STA summary\n",wtp_id);		
		vty_out(vty,"WTP BSS num    %d\n",wtp->num_bss);
		vty_out(vty,"WTP STA num    %d\n",wtp->num_sta);
		vty_out(vty,"WTP assoc      %d\n",wtp->num_assoc);		
		vty_out(vty,"WTP reassoc    %d\n",wtp->num_reassoc);		
		vty_out(vty,"WTP assoc fail %d\n",wtp->num_assoc_failure);		
		vty_out(vty,"WTP normal down num %d\n",wtp->num_normal_sta_down); 
		vty_out(vty,"WTP abnormal down num %d\n",wtp->num_abnormal_sta_down);
		if(wtp->num_bss == 0)
			vty_out(vty,"WTP %d does not provide service\n",wtp_id);
		dcli_free_wtp(wtp);
	}else if (ret == ASD_WTP_NOT_EXIST)	
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				wtp = NULL;
				wtp = show_sta_bywtp(dcli_dbus_connection, profile, wtp_id, localid, &ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wtp != NULL)){
					bss = NULL;
					for(i=0; i<wtp->num_bss; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = wtp->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;

						vty_out(vty,"==================================================================\n");
						vty_out(vty,"WTPID:       %d\t\t",bss->WtpID);
						vty_out(vty,"BSS STA num: %d\n",bss->num_sta);
						vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
						vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
						vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
						vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
						vty_out(vty,"Assoc:       %d\t\t",bss->num_assoc);
						vty_out(vty,"Reassoc:     %d\n",bss->num_reassoc);
						vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
						vty_out(vty,"==================================================================\n");
						for(j=0; j<bss->num_sta; j++){	
							unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
							
							
							memset(ieee80211_state, 0, 20);
							memset(PAE, 0, 20);
							memset(BACKEND, 0, 20);

							if(sta == NULL)
								sta = bss->sta_list;
							else 
								sta = sta->next;

							if(sta == NULL)
								break;
							//time_t 	now,online_time,now_sysrun,statime;
							int 	hour,min,sec;
							/*//qiuchen change it
							time(&now);
							get_sysruntime(&now_sysrun);
							online_time = now_sysrun - sta->StaTime + sta->sta_online_time;
							statime = now - online_time;
							//end;*/
							asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
							vty_out(vty,"------------------------------------------------------------\n");
							vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
							vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
							vty_out(vty,"PAE_STATE:		%s\n",PAE);
							vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
							vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
							
							time_t online_time=sta->sta_online_time_new;
							hour=online_time/3600;
							min=(online_time-hour*3600)/60;
							sec=(online_time-hour*3600)%60;
						
							vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
							vty_out(vty,"------------------------------------------------------------\n");
						}	
						vty_out(vty,"==================================================================\n");

					}		
					vty_out(vty,"WTP %d STA summary\n",wtp_id);		
					vty_out(vty,"WTP BSS num    %d\n",wtp->num_bss);
					vty_out(vty,"WTP STA num    %d\n",wtp->num_sta);
					vty_out(vty,"WTP assoc      %d\n",wtp->num_assoc);		
					vty_out(vty,"WTP reassoc    %d\n",wtp->num_reassoc);		
					vty_out(vty,"WTP assoc fail %d\n",wtp->num_assoc_failure);		
					vty_out(vty,"WTP normal down num %d\n",wtp->num_normal_sta_down); 
					vty_out(vty,"WTP abnormal down num %d\n",wtp->num_abnormal_sta_down);
					if(wtp->num_bss == 0)
						vty_out(vty,"WTP %d does not provide service\n",wtp_id);
					dcli_free_wtp(wtp);
				}else if (ret == ASD_WTP_NOT_EXIST)	
					vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
				else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}

	local_hansi_parameter:
			wtp = NULL;
			wtp = show_sta_bywtp(dcli_dbus_connection, profile, wtp_id, localid, &ret);
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) && (wtp != NULL)){
				bss = NULL;
				for(i=0; i<wtp->num_bss; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = wtp->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;

					vty_out(vty,"==================================================================\n");
					vty_out(vty,"WTPID:       %d\t\t",bss->WtpID);
					vty_out(vty,"BSS STA num: %d\n",bss->num_sta);
					vty_out(vty,"Radio_G_ID:  %d\t\t",bss->Radio_G_ID);
					vty_out(vty,"Radio_L_ID:  %d\n",bss->Radio_L_ID);
					vty_out(vty,"BSSIndex:    %d\t\t",bss->BSSIndex);
					vty_out(vty,"SecurityID:  %d\n",bss->SecurityID);
					vty_out(vty,"Assoc:       %d\t\t",bss->num_assoc);
					vty_out(vty,"Reassoc:     %d\n",bss->num_reassoc);
					vty_out(vty,"Assoc_fail:  %d\n",bss->num_assoc_failure);
					vty_out(vty,"==================================================================\n");
					for(j=0; j<bss->num_sta; j++){	
						unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
						
						memset(ieee80211_state, 0, 20);
						memset(PAE, 0, 20);
						memset(BACKEND, 0, 20);

						if(sta == NULL)
							sta = bss->sta_list;
						else 
							sta = sta->next;

						if(sta == NULL)
							break;
						//time_t 	now,online_time,now_sysrun,statime;
						int 	hour,min,sec;
						/*//qiuchen change it
						time(&now);
						get_sysruntime(&now_sysrun);
						online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
						statime = now - online_time;
						//end;*/
						asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
						vty_out(vty,"------------------------------------------------------------\n");
						vty_out(vty,"STA MAC: "MACSTR"\n",MAC2STR(sta->addr));
						vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
						vty_out(vty,"PAE_STATE:		%s\n",PAE);
						vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
						vty_out(vty,"Access Time:     %s",ctime(&sta->sta_access_time));//qiuchen change it
						
						time_t online_time=sta->sta_online_time_new;
						hour=online_time/3600;
						min=(online_time-hour*3600)/60;
						sec=(online_time-hour*3600)%60;
					
						vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);
						vty_out(vty,"------------------------------------------------------------\n");
					}	
					vty_out(vty,"==================================================================\n");

				}		
				vty_out(vty,"WTP %d STA summary\n",wtp_id);		
				vty_out(vty,"WTP BSS num    %d\n",wtp->num_bss);
				vty_out(vty,"WTP STA num    %d\n",wtp->num_sta);
				vty_out(vty,"WTP assoc      %d\n",wtp->num_assoc);		
				vty_out(vty,"WTP reassoc    %d\n",wtp->num_reassoc);		
				vty_out(vty,"WTP assoc fail %d\n",wtp->num_assoc_failure);		
				vty_out(vty,"WTP normal down num %d\n",wtp->num_normal_sta_down); 
				vty_out(vty,"WTP abnormal down num %d\n",wtp->num_abnormal_sta_down);
				if(wtp->num_bss == 0)
					vty_out(vty,"WTP %d does not provide service\n",wtp_id);
				dcli_free_wtp(wtp);
			}else if (ret == ASD_WTP_NOT_EXIST)	
				vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
			else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
				vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;
}
#else
DEFUN(show_wtp_sta_cmd_func,
	  show_wtp_sta_cmd,
	  "show sta bywtpid WTPID",
	  CONFIG_STR
	  "ASD wlan station information\n"
	  "Search by WTPID \n"
	  "WTP id\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned int bssnum = 0, stanum = 0, wtp_id = 0, wtpstanum = 0;	
	unsigned int assoc_num = 0, reassoc_num = 0, assoc_failure_num = 0;
	unsigned int wtp_assoc_num = 0, wtp_reassoc_num = 0, wtp_assoc_failure_num = 0;	
	unsigned int normal_st_down_num=0,abnormal_st_down_num=0;//nl091125

	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_STALIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_STALIST);*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0){
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(bssnum));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wtp_assoc_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wtp_reassoc_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wtp_assoc_failure_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(normal_st_down_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(abnormal_st_down_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for(i=0; i<bssnum; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;		
		unsigned char WlanID,Security_ID,Radio_L_ID;
		unsigned int Radio_G_ID,BSSIndex,WTPID;
		int j;
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(WlanID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Radio_G_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(BSSIndex));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(Security_ID));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(assoc_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(reassoc_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(assoc_failure_num));	

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(stanum));	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		wtpstanum += stanum;
		WTPID = Radio_G_ID/L_RADIO_NUM;
		Radio_L_ID = Radio_G_ID%L_RADIO_NUM;
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WTPID:       %d\t\t",WTPID);
		vty_out(vty,"BSS STA num: %d\n",stanum);
		vty_out(vty,"Radio_G_ID:  %d\t\t",Radio_G_ID);
		vty_out(vty,"Radio_L_ID:  %d\n",Radio_L_ID);
		vty_out(vty,"BSSIndex:    %d\t\t",BSSIndex);
		vty_out(vty,"SecurityID:  %d\n",Security_ID);
		vty_out(vty,"Assoc:       %d\t\t",assoc_num);
		vty_out(vty,"Reassoc:     %d\n",reassoc_num);
		vty_out(vty,"Assoc_fail:  %d\n",assoc_failure_num);
		/*vty_out(vty,"WlanID:%d WTPID:%d BSS STAnum:%d Radio_G_ID:%d Radio_L_ID:%d BSSIndex:%d SecurityID:%d\n",WlanID,WTPID,stanum,Radio_G_ID,Radio_L_ID,BSSIndex,Security_ID);*/
		vty_out(vty,"==============================================================================\n");
		for(j=0; j<stanum; j++){	
			DBusMessageIter iter_sub_struct;
			unsigned char mac[6],ieee80211_state[20], PAE[20], BACKEND[20];
			unsigned int sta_flags, pae_state, backend_state;
			/*char *StaTime;*/
			time_t StaTime,sta_online_time;//qiuchen add it
			
			memset(ieee80211_state, 0, 20);
			memset(PAE, 0, 20);
			memset(BACKEND, 0, 20);

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

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_flags));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(pae_state));	
		
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(backend_state));	

			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(StaTime));
			//qiuchen add it
			dbus_message_iter_next(&iter_sub_struct);	
			dbus_message_iter_get_basic(&iter_sub_struct,&(sta_online_time));	
			//end
			
			asd_state_check(ieee80211_state,sta_flags,PAE,pae_state,BACKEND,backend_state);			
			vty_out(vty,"------------------------------------------------------------\n");
			vty_out(vty,"STA MAC: %02X:",mac[0]);
			vty_out(vty,"%02X:",mac[1]);		
			vty_out(vty,"%02X:",mac[2]);
			vty_out(vty,"%02X:",mac[3]);
			vty_out(vty,"%02X:",mac[4]);
			vty_out(vty,"%02X\n",mac[5]);
			vty_out(vty,"STA IEEE80211 State:	 %s\n",ieee80211_state);
			vty_out(vty,"PAE_STATE:		%s\n",PAE);
			vty_out(vty,"BACKEND_STATE:		%s\n",BACKEND);
			//qiuchen change it 2012.10.31
			time_t now,online_time,now_sysrun,statime;

			time(&now);
			get_sysruntime(&now_sysrun);
			online_time=now_sysrun-sta->StaTime+sta_online_time;
			statime = now - online_time;
			vty_out(vty,"Access Time:	%s",ctime(&statime));
			
			int hour,min,sec;


			hour=online_time/3600;
			min=(online_time-hour*3600)/60;
			sec=(online_time-hour*3600)%60;
		
			vty_out(vty,"Online Time:	%2d:%2d:%2d\n",hour,min,sec);

			vty_out(vty,"------------------------------------------------------------\n");
			
			dbus_message_iter_next(&iter_sub_array);
		}
		
		vty_out(vty,"==============================================================================\n");
		dbus_message_iter_next(&iter_array);

	}		
		vty_out(vty,"WTP %d STA summary\n",wtp_id);		
		vty_out(vty,"WTP BSS num    %d\n",bssnum);
		vty_out(vty,"WTP STA num    %d\n",wtpstanum);
		vty_out(vty,"WTP assoc      %d\n",wtp_assoc_num);		
		vty_out(vty,"WTP reassoc    %d\n",wtp_reassoc_num);		
		vty_out(vty,"WTP assoc fail %d\n",wtp_assoc_failure_num);	
		vty_out(vty,"WTP normal sta down %d\n",normal_st_down_num);
		vty_out(vty,"WTP abnormal sta down %d\n",abnormal_st_down_num);
		if(bssnum == 0)
			vty_out(vty,"WTP %d does not provide service\n",wtp_id);
	}else if (ret == ASD_WTP_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not provide service or it maybe does not exist\n",wtp_id);
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

/*xm 08/11/07*/
DEFUN(wlan_add_MAC_list_cmd_func,
		wlan_add_MAC_list_cmd,
		"wlan WLANID add (black|white) list MAC",
		"wlan configure\n"
		"wlan id\n" 
		"wlan add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   /*1--black list*/
								/* 2--white list*/
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;		
	}else if (!strcmp(argv[1],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[2],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret == ASD_MAC_ADD_ALREADY)
		vty_out(vty,"mac add already!\n");
	else 
		vty_out(vty,"wlan add mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(wlan_del_MAC_list_cmd_func,
		wlan_del_MAC_list_cmd,
		"wlan WLANID del (black|white) list MAC",
		"wlan configure\n"
		"wlan id\n" 
		"wlan add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   /*1--black list*/
								/* 2--white list*/
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;		
	}else if (!strcmp(argv[1],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[2],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret==ASD_UNKNOWN_ID)
		vty_out(vty,"<error> mac is not in the list\n");
	else 
		vty_out(vty,"wlan del mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


DEFUN(wlan_use_MAC_list_cmd_func,
		wlan_use_MAC_list_cmd,
		"wlan WLANID use (none|black|white) list ",
		"wlan configure\n"
		"wlan id\n" 
		"type of list\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char wlan_id = 0;
	unsigned char list_type=0;   /*0--none,1--black list, 2--white list*/
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"none")||(tolower(argv[1][0]) == 'n')){
		list_type=0;			
	}else if (!strcmp(argv[1],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;
	}else if (!strcmp(argv[1],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white/none' or 'b/w/n' \n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error> wlan isn't existed\n");
	else if(ret==ASD_WIDS_OPEN)
		vty_out(vty,"<error> wids has open,can't be set other except blank list\n");
	else
		vty_out(vty,"wlan use mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


DEFUN(wtp_add_MAC_list_cmd_func,
		wtp_add_MAC_list_cmd,
		"wtp WTPID add (black|white) list MAC",
		"wtp configure\n"
		"wtp id\n" 
		"wtp add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret;
	unsigned int wtp_id = 0;
	unsigned char list_type=0;   /*1--black list*/
								/* 2--white list*/
	
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;		
	}else if (!strcmp(argv[1],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[2],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
								
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WTP_NOT_EXIST)
		vty_out(vty,"<error> wtp is not existed\n");
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else if(ret == ASD_MAC_ADD_ALREADY)
	{
		vty_out(vty,"mac add already!\n");
	}
	else 
	{
		vty_out(vty,"wtp add mac list successfully!\n"); 
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(wtp_del_MAC_list_cmd_func,
		wtp_del_MAC_list_cmd,
		"wtp WTPID del (black|white) list MAC",
		"bss configure\n"
		"wtp id\n" 
		"wtp add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret;
	unsigned int wtp_id = 0;
	unsigned char list_type=0;   /*1--black list*/
								/* 2--white list*/
	
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;		
	}else if (!strcmp(argv[1],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[2],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
								
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WTP_NOT_EXIST)
		vty_out(vty,"<error> wtp is not existed\n");
	else if(ret==ASD_UNKNOWN_ID)
		vty_out(vty,"<error> mac is not in the list\n");	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else 
		vty_out(vty,"wtp del mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


DEFUN(wtp_use_MAC_list_cmd_func,
		wtp_use_MAC_list_cmd,
		"wtp WTPID use (none|black|white) list ",
		"wtp configure\n"
		"wtp id\n" 
		"type of list\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int wtp_id = 0;
	unsigned char list_type=0;   /*0--none,1--black list, 2--white list*/
	
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"none")||(tolower(argv[1][0]) == 'n')){
		list_type=0;			
	}else if (!strcmp(argv[1],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;
	}else if (!strcmp(argv[1],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;	
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_WTP_NOT_EXIST)
		vty_out(vty,"<error> wtp isn't existed\n");	
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else 
		vty_out(vty,"wtp use mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


DEFUN(bss_add_MAC_list_cmd_func,
		bss_add_MAC_list_cmd,
		"radio RADIO wlan WLANID add (black|white) list MAC",
		"bss configure\n"
		"G_Radio id\n" 
		"L_Bss id\n" 
		"wlan add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret;
	unsigned int radio_id = 0;
	unsigned char bss_id = 0;
	unsigned char list_type=0;   /*1--black list*/ /* 2--white list*/
	unsigned char wlan_id = 0;
	
	ret = parse2_int_ID((char*)argv[0], &radio_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
		return CMD_SUCCESS;
	}
	
	ret = parse2_char_ID((char*)argv[1], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	str2lower(&argv[2]);
	
	if (!strcmp(argv[2],"black")||(tolower(argv[2][0]) == 'b')){
		list_type=1;		
	}else if (!strcmp(argv[2],"white")||(tolower(argv[2][0]) == 'w')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[3],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
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

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==BSS_NOT_EXIST){
		vty_out(vty,"radio %d has not apply wlan %d not exist .\n",radio_id,wlan_id);
		return CMD_SUCCESS;
	}
	else if(ret == BSS_NOT_ENABLE)
	{
		vty_out(vty,"bss add mac list successfully!\n");// wid will store this while will notice to asd when the bss is enable
		return CMD_SUCCESS;
	}	
	else if(ret == WID_MAC_ADD_ALREADY){
		vty_out(vty,"mac add already!\n");
		return CMD_SUCCESS;
	}	
	else if(ret == RADIO_ID_NOT_EXIST){
		vty_out(vty,"radio id is not exist!\n");
		return CMD_SUCCESS;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"wlan %d is not exist!\n",wlan_id);
		return CMD_SUCCESS;
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
		return CMD_SUCCESS;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_BSS_NOT_EXIST)
		vty_out(vty,"<error> bss is not exist\n");	
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
	else if(ret == ASD_MAC_ADD_ALREADY)
		vty_out(vty,"mac add already!\n");
	else 
		vty_out(vty,"bss add mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(bss_del_MAC_list_cmd_func,
		bss_del_MAC_list_cmd,
		"radio RADIO wlan WLANID del (black|white) list MAC",
		"bss configure\n"
		"G_Radio id\n" 
		"L_Bss id\n" 
		"wlan add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN];
	unsigned int ret;
	unsigned int radio_id = 0;
	unsigned char bss_id = 0;
	unsigned char list_type=0;   /*1--black list*/	/* 2--white list*/
	unsigned char wlan_id = 0;
	
	ret = parse2_int_ID((char*)argv[0], &radio_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
		return CMD_SUCCESS;
	}
	
	ret = parse2_char_ID((char*)argv[1], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	str2lower(&argv[2]);
	
	if (!strcmp(argv[2],"black")||(tolower(argv[2][0]) == 'b')){
		list_type=1;		
	}else if (!strcmp(argv[2],"white")||(tolower(argv[2][0]) == 'w')){
		list_type=2;		
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[3],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
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

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==BSS_NOT_EXIST){
		vty_out(vty,"radio %d has not apply wlan %d not exist .\n",radio_id,wlan_id);
		return CMD_SUCCESS;
	}
	else if(ret== WID_UNKNOWN_ID){
		vty_out(vty,"<error> mac is not in the list\n");	
		return CMD_SUCCESS;
	}
	
	else if(ret == BSS_NOT_ENABLE)
	{
		vty_out(vty,"bss del mac list successfully!\n");//qiuchen
		return CMD_SUCCESS;
	}	
	else if(ret == RADIO_ID_NOT_EXIST){
		vty_out(vty,"radio id is not exist!\n");
		return CMD_SUCCESS;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"wlan %d is not exist!\n",wlan_id);
		return CMD_SUCCESS;
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
		return CMD_SUCCESS;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_BSS_NOT_EXIST)
		vty_out(vty,"<error> bss is not exist\n");
	else if(ret==ASD_UNKNOWN_ID)
		vty_out(vty,"<error> mac is not in the list\n");	
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
	else 
		vty_out(vty,"bss del mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


DEFUN(bss_use_MAC_list_cmd_func,
		bss_use_MAC_list_cmd,
		"radio RADIO wlan WLANID use (none|black|white) list",
		"bss configure\n"
		"G_Radio id\n" 
		"L_Bss id\n" 
		"wlan use MAC list\n" 
		"type of list\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int radio_id = 0;
	unsigned char bss_id = 0;
	unsigned char list_type=0;   /*0--none,1--black list, 2--white list*/
	unsigned char wlan_id = 0;
	
	ret = parse2_int_ID((char*)argv[0], &radio_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
		return CMD_SUCCESS;
	}
	
	ret = parse2_char_ID((char*)argv[1], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	str2lower(&argv[2]);
	
	if (!strcmp(argv[2],"none")||(tolower(argv[1][0]) == 'n')){
		list_type=0;			
	}else if (!strcmp(argv[2],"black")||(tolower(argv[1][0]) == 'b')){
		list_type=1;
	}else if (!strcmp(argv[2],"white")||(tolower(argv[1][0]) == 'w')){
		list_type=2;			
	}else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BSS_USE_MAC_LIST);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==BSS_NOT_EXIST){
		vty_out(vty,"radio %d has not apply wlan %d not exist .\n",radio_id,wlan_id);
		return CMD_SUCCESS;
	}
	else if(ret == BSS_NOT_ENABLE)
	{
		vty_out(vty,"bss use mac list successfully!\n");
		return CMD_SUCCESS;
	}	
	else if(ret == WID_MAC_ADD_ALREADY){
		vty_out(vty,"mac add already!\n");
		return CMD_SUCCESS;
	}	
	else if(ret == RADIO_ID_NOT_EXIST){
		vty_out(vty,"radio id is not exist!\n");
		return CMD_SUCCESS;
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"wlan %d is not exist!\n",wlan_id);
		return CMD_SUCCESS;
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
		return CMD_SUCCESS;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_BSS_NOT_EXIST)
		vty_out(vty,"<error> bss is not exist\n");	
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
	else 
		vty_out(vty,"bss use mac list successfully!\n"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


#if DCLI_NEW
DEFUN(show_wlan_MAC_list_cmd_func,
		show_wlan_MAC_list_cmd,
		"show wlan WLANID mac list [remote] [local] [PARAMETER]",
		SHOW_STR
		"wlan configure\n"
		"wlanid value\n"
		"show wlan mac list\n" 
		"show wlan mac list\n" 
		"'remote' or 'local' hansi\n"
		"'remote' or 'local' hansi\n"
		"slotid-instid\n"
		)
{	
	 
	unsigned int ret;
	unsigned char wlan_id = 0;
	int i=0, j=0;
	struct dcli_wlan_info *wlan = NULL;
	struct maclist		   *tmp = NULL;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
		
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);	
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		wlan = show_wlan_maclist(dcli_dbus_connection, index, wlan_id, localid, &ret);
	 	
		if((ret == 0) && (wlan != NULL))
		{		
			char *maclist_name[3]={"none","black","white"};
			
			vty_out(vty,"wlan %d use %s list \n",wlan_id,maclist_name[wlan->acl_conf.macaddr_acl]);
			vty_out(vty,"Black mac list(%d):\n",wlan->acl_conf.num_deny_mac);
			tmp = wlan->acl_conf.deny_mac;
			for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
				if(tmp == NULL)
					break;
				vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
				tmp = tmp->next;
				if((j%3 == 2) || (j == wlan->acl_conf.num_deny_mac-1))
					vty_out(vty,"\n");
			}
			
			vty_out(vty,"White mac list(%d):\n",wlan->acl_conf.num_accept_mac);
			tmp = wlan->acl_conf.accept_mac;
			for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
				if(tmp == NULL)
					break;
				vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
				tmp = tmp->next;
				if((j%3 == 2) || (j == wlan->acl_conf.num_accept_mac-1))
					vty_out(vty,"\n");
			}
			dcli_free_wlan(wlan);
		}

		else if(ret==ASD_WLAN_NOT_EXIST) 
			vty_out(vty,"<error> wlan isn't existed\n");
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				wlan = show_wlan_maclist(dcli_dbus_connection, profile, wlan_id, localid, &ret);
			 	
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wlan != NULL))
				{		
					char *maclist_name[3]={"none","black","white"};
					
					vty_out(vty,"wlan %d use %s list \n",wlan_id,maclist_name[wlan->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",wlan->acl_conf.num_deny_mac);
					tmp = wlan->acl_conf.deny_mac;
					for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wlan->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",wlan->acl_conf.num_accept_mac);
					tmp = wlan->acl_conf.accept_mac;
					for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wlan->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
					dcli_free_wlan(wlan);
				}

				else if(ret==ASD_WLAN_NOT_EXIST) 
					vty_out(vty,"<error> wlan isn't existed\n");
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				wlan = show_wlan_maclist(dcli_dbus_connection, profile, wlan_id, localid, &ret);
			 	
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wlan != NULL))
				{		
					char *maclist_name[3]={"none","black","white"};
					
					vty_out(vty,"wlan %d use %s list \n",wlan_id,maclist_name[wlan->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",wlan->acl_conf.num_deny_mac);
					tmp = wlan->acl_conf.deny_mac;
					for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wlan->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",wlan->acl_conf.num_accept_mac);
					tmp = wlan->acl_conf.accept_mac;
					for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wlan->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
					dcli_free_wlan(wlan);
				}

				else if(ret==ASD_WLAN_NOT_EXIST) 
					vty_out(vty,"<error> wlan isn't existed\n");
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;	

}


#else
DEFUN(show_wlan_MAC_list_cmd_func,
		show_wlan_MAC_list_cmd,
		"show wlan WLANID mac list",
		"wlan configure\n"
		"show wlan mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	unsigned int ret;
	unsigned char wlan_id = 0;
	int i=0, j=0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST);*/

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show wlan %d mac list failed get reply.\n",wlan_id);
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==ASD_WLAN_NOT_EXIST) {
		vty_out(vty,"<error> wlan isn't existed\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	{		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned int maclist_acl;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"wlan %d use %s list \n",wlan_id,maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
			for(j=0; j<num[i]; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char mac[MAC_LEN];

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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		dbus_message_iter_next(&iter_array);
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif

#if DCLI_NEW
DEFUN(show_wtp_MAC_list_cmd_func,
		show_wtp_MAC_list_cmd,
		"show wtp WTPID mac list [remote] [local] [PARAMETER]",
		SHOW_STR
		"wtp configure\n"
		"wtpid value\n"
		"show wtp mac list\n" 
		"show wtp mac list\n" 
		"'remote' or 'local' hansi\n"
		"'remote' or 'local' hansi\n"
		"slotid-instid\n"
		)
{	
	 
	struct dcli_wtp_info *wtp = NULL;
	struct maclist		 *tmp = NULL;
	unsigned int ret = 0;
	unsigned int wtp_id = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		wtp = show_wtp_maclist(dcli_dbus_connection, index, wtp_id, localid, &ret);

		if((ret == 0) && (wtp != NULL)){		
			char *maclist_name[3]={"none","black","white"};
			
			vty_out(vty,"Wtp %d use %s list \n",wtp_id,maclist_name[wtp->acl_conf.macaddr_acl]);
			vty_out(vty,"Black mac list(%d):\n",wtp->acl_conf.num_deny_mac);
			tmp = wtp->acl_conf.deny_mac;
			for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
				if(tmp == NULL)
					break;
				vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
				tmp = tmp->next;
				if((j%3 == 2) || (j == wtp->acl_conf.num_deny_mac-1))
					vty_out(vty,"\n");
			}
			
			vty_out(vty,"White mac list(%d):\n",wtp->acl_conf.num_accept_mac);
			tmp = wtp->acl_conf.accept_mac;
			for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
				if(tmp == NULL)
					break;
				vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
				tmp = tmp->next;
				if((j%3 == 2) || (j == wtp->acl_conf.num_accept_mac-1))
					vty_out(vty,"\n");
			}
			dcli_free_wtp(wtp);
		}
		else if(ret==ASD_WTP_NOT_EXIST) 
			vty_out(vty,"<error> wtp isn't existed\n");
		else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}



	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				wtp = show_wtp_maclist(dcli_dbus_connection, profile, wtp_id, localid, &ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wtp != NULL)){		
					char *maclist_name[3]={"none","black","white"};
					
					vty_out(vty,"Wtp %d use %s list \n",wtp_id,maclist_name[wtp->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",wtp->acl_conf.num_deny_mac);
					tmp = wtp->acl_conf.deny_mac;
					for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wtp->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",wtp->acl_conf.num_accept_mac);
					tmp = wtp->acl_conf.accept_mac;
					for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wtp->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
					dcli_free_wtp(wtp);
				}
				else if(ret==ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> wtp isn't existed\n");
				else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				wtp = show_wtp_maclist(dcli_dbus_connection, profile, wtp_id, localid, &ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (wtp != NULL)){		
					char *maclist_name[3]={"none","black","white"};
					
					vty_out(vty,"Wtp %d use %s list \n",wtp_id,maclist_name[wtp->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",wtp->acl_conf.num_deny_mac);
					tmp = wtp->acl_conf.deny_mac;
					for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wtp->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",wtp->acl_conf.num_accept_mac);
					tmp = wtp->acl_conf.accept_mac;
					for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == wtp->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
					dcli_free_wtp(wtp);
				}
				else if(ret==ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> wtp isn't existed\n");
				else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;	
}
#else	
DEFUN(show_wtp_MAC_list_cmd_func,
		show_wtp_MAC_list_cmd,
		"show wtp WTPID mac list",
		"wtp configure\n"
		"show wtp mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	unsigned int ret;
	char *showStr = NULL ;
	unsigned int wtp_id = 0;
	
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST);*/

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show wtp %d mac list failed get reply.\n",wtp_id);
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==ASD_WTP_NOT_EXIST) {
		vty_out(vty,"<error> wtp isn't existed\n");
		dbus_message_unref(reply);		
		return CMD_SUCCESS;
	}
	else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		dbus_message_unref(reply);		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	

	{		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned int maclist_acl;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"wtp %d use %s list \n",wtp_id,maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
			for(j=0; j<num[i]; j++){	
				DBusMessageIter iter_sub_struct;
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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		dbus_message_iter_next(&iter_array);
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}
#endif


#if DCLI_NEW
DEFUN(show_bss_MAC_list_cmd_func,
		show_bss_MAC_list_cmd,
		"show radio RADIO wlan WLANID mac list [remote] [local] [PARAMETER]",
		SHOW_STR
		"radio configure\n"
		"G_Radio id\n" 
		"wlan configure\n"
		"wlan id\n" 
		"show bss mac list\n" 
		"show bss mac list\n" 
		"'remote' or 'local' hansi\n"
		"'remote' or 'local' hansi\n"
		"slotid-instid\n"
		)
{	
	 
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	unsigned int ret;
	unsigned int radio_id = 0;
	unsigned char bss_id = 0;
	unsigned char wlan_id = 0;
	struct dcli_bss_info *bss = NULL;	
	struct maclist		 *tmp = NULL;
	
	ret = parse2_int_ID((char*)argv[0], &radio_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
		return CMD_SUCCESS;
	}
	
	ret = parse2_char_ID((char*)argv[1], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 3)||(argc == 5)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 4){
		if (!strcmp(argv[2],"remote")){
			localid = 0;
		}else if(!strcmp(argv[2],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[2],"remote"))&&(!strcmp(argv[3],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[3],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[3], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		bss = show_radio_bss_maclist(dcli_dbus_connection, index, radio_id, wlan_id, localid,&ret);
			
		if((ret == 0) && (bss != NULL))
		{		
			char *maclist_name[3]={"none","black","white"};
			vty_out(vty,"WLAN %d Radio %d use %s list \n",wlan_id,radio_id,maclist_name[bss->acl_conf.macaddr_acl]);
			vty_out(vty,"Black mac list(%d):\n",bss->acl_conf.num_deny_mac);
			
			tmp = bss->acl_conf.deny_mac;
			
			for(j=0; j<bss->acl_conf.num_deny_mac; j++){
				if(tmp == NULL)
					break;
				vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
				tmp = tmp->next;
				if((j%3 == 2) || (j == bss->acl_conf.num_deny_mac-1))
					vty_out(vty,"\n");
			}
			
			vty_out(vty,"White mac list(%d):\n",bss->acl_conf.num_accept_mac);
			tmp = bss->acl_conf.accept_mac;
			for(j=0; j<bss->acl_conf.num_accept_mac; j++){
				if(tmp == NULL)
					break;
				vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
				tmp = tmp->next;
				if((j%3 == 2) || (j == bss->acl_conf.num_accept_mac-1))
					vty_out(vty,"\n");
			}
			dcli_free_bss(bss);										
		}

		else if((ret==ASD_BSS_NOT_EXIST)||(ret==BSS_NOT_EXIST))
				vty_out(vty,"<error> bss isn't existed\n");		
		else if((ret == ASD_RADIO_ID_LARGE_THAN_MAX)||(ret == RADIO_ID_LARGE_THAN_MAX)) 	
			vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);		
		else if ((ret == ASD_DBUS_ERROR)||(ret == WID_DBUS_ERROR))
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				bss = show_radio_bss_maclist(dcli_dbus_connection, profile, radio_id, wlan_id, localid,&ret);
					
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (bss != NULL))
				{		
					char *maclist_name[3]={"none","black","white"};
					vty_out(vty,"WLAN %d Radio %d use %s list \n",wlan_id,radio_id,maclist_name[bss->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",bss->acl_conf.num_deny_mac);
					
					tmp = bss->acl_conf.deny_mac;
					
					for(j=0; j<bss->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == bss->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",bss->acl_conf.num_accept_mac);
					tmp = bss->acl_conf.accept_mac;
					for(j=0; j<bss->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == bss->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
					dcli_free_bss(bss);										
				}

				else if((ret==ASD_BSS_NOT_EXIST)||(ret==BSS_NOT_EXIST))
						vty_out(vty,"<error> bss isn't existed\n");		
				else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
					vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);		
				else if ((ret == ASD_DBUS_ERROR)||(ret == WID_DBUS_ERROR))
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 4){
					return CMD_SUCCESS;
				}
			}
		}


		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
		 local_hansi_parameter:
				bss = show_radio_bss_maclist(dcli_dbus_connection, profile, radio_id, wlan_id, localid,&ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (bss != NULL))
				{		
					char *maclist_name[3]={"none","black","white"};
					vty_out(vty,"WLAN %d Radio %d use %s list \n",wlan_id,radio_id,maclist_name[bss->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",bss->acl_conf.num_deny_mac);
					
					tmp = bss->acl_conf.deny_mac;
					
					for(j=0; j<bss->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == bss->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",bss->acl_conf.num_accept_mac);
					tmp = bss->acl_conf.accept_mac;
					for(j=0; j<bss->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr));		
						tmp = tmp->next;
						if((j%3 == 2) || (j == bss->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
					dcli_free_bss(bss);										
				}

				else if((ret==ASD_BSS_NOT_EXIST)||(ret==BSS_NOT_EXIST))
						vty_out(vty,"<error> bss isn't existed\n");		
				else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX) 	
					vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);		
				else if ((ret == ASD_DBUS_ERROR)||(ret == WID_DBUS_ERROR))
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 4){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;	
}


#else
DEFUN(show_bss_MAC_list_cmd_func,
		show_bss_MAC_list_cmd,
		"show radio RADIO bss BSS mac list",
		"bss configure\n"
		"G_Radio id\n" 
		"L_Bss id\n" 
		"show bss mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	unsigned int ret;
	unsigned int radio_id = 0;
	unsigned char bss_id = 0;
	
	ret = parse2_int_ID((char*)argv[0], &radio_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(radio_id >= G_RADIO_NUM || radio_id == 0){
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
		return CMD_SUCCESS;
	}
	
	ret = parse2_char_ID((char*)argv[1], &bss_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(bss_id > L_BSS_NUM || bss_id == 0){
		vty_out(vty,"<error> bss id should be 1 to %d\n",L_BSS_NUM);
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST);*/

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&bss_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show radio %d bss %d wlan mac list failed get reply.\n",radio_id,bss_id);
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==ASD_BSS_NOT_EXIST) {
		vty_out(vty,"<error> bss isn't existed\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(ret == ASD_RADIO_ID_LARGE_THAN_MAX){ 	
		vty_out(vty,"<error> radio id should be 1 to %d\n",G_RADIO_NUM-1);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	{		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned int maclist_acl;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"radio %d bss %d use %s list \n",radio_id,bss_id,maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
			for(j=0; j<num[i]; j++){	
				DBusMessageIter iter_sub_struct;
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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		dbus_message_iter_next(&iter_array);
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}
#endif

#if DCLI_NEW
DEFUN(show_all_wlan_MAC_list_cmd_func,
		show_all_wlan_MAC_list_cmd,
		"show all wlan mac list [remote] [local] [PARAMETER]",
		SHOW_STR
		"all wlan configure\n"
		"wlan configure\n"
		"show wlan mac list\n" 
		"show wlan mac list\n" 
		"'remote' or 'local' hansi\n"
		"'remote' or 'local' hansi\n"
		"slotid-instid\n"
		)
{	
	 
	struct dcli_ac_info  *ac = NULL;
	struct dcli_wlan_info *wlan = NULL;
	struct maclist		 *tmp = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_all_wlan_maclist(dcli_dbus_connection, index, localid, &ret);
				
		if((ret == 0) && (ac != NULL)){	
			
			char *maclist_name[3]={"none","black","white"};
			for(i=0; i< ac->num_wlan; i++){
				if(wlan == NULL)
					wlan = ac->wlan_list;
				else 
					wlan = wlan->next;
		
				if(wlan == NULL)
					break;
				vty_out(vty,"wlan %d use %s list \n",wlan->WlanID,maclist_name[wlan->acl_conf.macaddr_acl]);
				vty_out(vty,"Black mac list(%d):\n",wlan->acl_conf.num_deny_mac);
				tmp = wlan->acl_conf.deny_mac;
				for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
					if(tmp == NULL)
						break;
					vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
					tmp = tmp->next;
					if((j%3 == 2) || (j == wlan->acl_conf.num_deny_mac-1))
						vty_out(vty,"\n");
				}
				
				vty_out(vty,"White mac list(%d):\n",wlan->acl_conf.num_accept_mac);
				tmp = wlan->acl_conf.accept_mac;
				for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
					if(tmp == NULL)
						break;
					vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
					tmp = tmp->next;
					if((j%3 == 2) || (j == wlan->acl_conf.num_accept_mac-1))
						vty_out(vty,"\n");
				}
			}
			dcli_free_ac(ac);
		}
		else if(ret == ASD_WLAN_NOT_EXIST) 
			vty_out(vty,"<error> wlan isn't existed\n");
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				ac = NULL;
				ac = show_all_wlan_maclist(dcli_dbus_connection, profile, localid, &ret);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)){	
					
					char *maclist_name[3]={"none","black","white"};
					wlan = NULL;
					for(i=0; i< ac->num_wlan; i++){
						if(wlan == NULL)
							wlan = ac->wlan_list;
						else 
							wlan = wlan->next;
				
						if(wlan == NULL)
							break;
						vty_out(vty,"wlan %d use %s list \n",wlan->WlanID,maclist_name[wlan->acl_conf.macaddr_acl]);
						vty_out(vty,"Black mac list(%d):\n",wlan->acl_conf.num_deny_mac);
						tmp = wlan->acl_conf.deny_mac;
						for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
							if(tmp == NULL)
								break;
							vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
							tmp = tmp->next;
							if((j%3 == 2) || (j == wlan->acl_conf.num_deny_mac-1))
								vty_out(vty,"\n");
						}
						
						vty_out(vty,"White mac list(%d):\n",wlan->acl_conf.num_accept_mac);
						tmp = wlan->acl_conf.accept_mac;
						for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
							if(tmp == NULL)
								break;
							vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
							tmp = tmp->next;
							if((j%3 == 2) || (j == wlan->acl_conf.num_accept_mac-1))
								vty_out(vty,"\n");
						}
					}
					dcli_free_ac(ac);
				}
				else if(ret == ASD_WLAN_NOT_EXIST) 
					vty_out(vty,"<error> wlan isn't existed\n");
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
		local_hansi_parameter:
			ac = NULL;
			ac = show_all_wlan_maclist(dcli_dbus_connection, profile, localid, &ret);
					
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)){	
				
				char *maclist_name[3]={"none","black","white"};
				wlan = NULL;
				for(i=0; i< ac->num_wlan; i++){
					if(wlan == NULL)
						wlan = ac->wlan_list;
					else 
						wlan = wlan->next;
			
					if(wlan == NULL)
						break;
					vty_out(vty,"wlan %d use %s list \n",wlan->WlanID,maclist_name[wlan->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",wlan->acl_conf.num_deny_mac);
					tmp = wlan->acl_conf.deny_mac;
					for(j=0; j<wlan->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
						tmp = tmp->next;
						if((j%3 == 2) || (j == wlan->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",wlan->acl_conf.num_accept_mac);
					tmp = wlan->acl_conf.accept_mac;
					for(j=0; j<wlan->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
						tmp = tmp->next;
						if((j%3 == 2) || (j == wlan->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
				}
				dcli_free_ac(ac);
			}
			else if(ret == ASD_WLAN_NOT_EXIST) 
				vty_out(vty,"<error> wlan isn't existed\n");
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS; 
}		
		
#else
DEFUN(show_all_wlan_MAC_list_cmd_func,
		show_all_wlan_MAC_list_cmd,
		"show all wlan mac list ",
		"wlan configure\n"
		"show wlan mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	unsigned char wlan_num;
	int i;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show all wlan mac list failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&wlan_num);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<wlan_num; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned int maclist_acl;
		unsigned char wlan_id = 0;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(wlan_id));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"wlan %d use %s list \n",wlan_id,maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
			for(j=0; j<num[i]; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char mac[MAC_LEN];

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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		
		dbus_message_iter_next(&iter_array);
	}

	if(wlan_num == 0)
		vty_out(vty,"there is no wlan\n");
	
	dbus_message_unref(reply);

	return ;	
}

#endif

#if DCLI_NEW
DEFUN(show_all_wtp_MAC_list_cmd_func,
		show_all_wtp_MAC_list_cmd,
		"show all wtp mac list [remote] [local] [PARAMETER]",
		SHOW_STR
		"all wtp\n"
		"wtp configure\n"
		"show wtp mac list\n" 
		"show wtp mac list\n" 
		"'remote' or 'local' hansi\n"
		"'remote' or 'local' hansi\n"
		"slotid-instid\n"
		)
{	
	 
	struct dcli_ac_info  *ac = NULL;
	struct dcli_wtp_info *wtp = NULL;
	struct maclist		 *tmp = NULL;
	unsigned int ret = 0;
	int i,j;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_all_wtp_maclist(dcli_dbus_connection, index, localid, &ret);

		if((ret == 0) && (ac != NULL)){		
			char *maclist_name[3]={"none","black","white"};
			for(i=0; i< ac->num_wtp; i++){
				if(wtp == NULL)
					wtp = ac->wtp_list;
				else 
					wtp = wtp->next;
		
				if(wtp == NULL)
					break;
				vty_out(vty,"Wtp %d use %s list \n",wtp->WtpID,maclist_name[wtp->acl_conf.macaddr_acl]);
				vty_out(vty,"Black mac list(%d):\n",wtp->acl_conf.num_deny_mac);
				tmp = wtp->acl_conf.deny_mac;
				for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
					if(tmp == NULL)
						break;
					vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
					tmp = tmp->next;
					if((j%3 == 2) || (j == wtp->acl_conf.num_deny_mac-1))
						vty_out(vty,"\n");
				}
				
				vty_out(vty,"White mac list(%d):\n",wtp->acl_conf.num_accept_mac);
				tmp = wtp->acl_conf.accept_mac;
				for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
					if(tmp == NULL)
						break;
					vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
					tmp = tmp->next;
					if((j%3 == 2) || (j == wtp->acl_conf.num_accept_mac-1))
						vty_out(vty,"\n");
				}
			}
			dcli_free_ac(ac);
		}
		else if(ret == ASD_WTP_NOT_EXIST) 
			vty_out(vty,"<error> wtp isn't existed\n");
		else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				ac = NULL;
				ac = show_all_wtp_maclist(dcli_dbus_connection, profile, localid, &ret);

			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)){		
					char *maclist_name[3]={"none","black","white"};
					wtp = NULL;
					for(i=0; i< ac->num_wtp; i++){
						if(wtp == NULL)
							wtp = ac->wtp_list;
						else 
							wtp = wtp->next;
				
						if(wtp == NULL)
							break;
						vty_out(vty,"Wtp %d use %s list \n",wtp->WtpID,maclist_name[wtp->acl_conf.macaddr_acl]);
						vty_out(vty,"Black mac list(%d):\n",wtp->acl_conf.num_deny_mac);
						tmp = wtp->acl_conf.deny_mac;
						for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
							if(tmp == NULL)
								break;
							vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
							tmp = tmp->next;
							if((j%3 == 2) || (j == wtp->acl_conf.num_deny_mac-1))
								vty_out(vty,"\n");
						}
						
						vty_out(vty,"White mac list(%d):\n",wtp->acl_conf.num_accept_mac);
						tmp = wtp->acl_conf.accept_mac;
						for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
							if(tmp == NULL)
								break;
							vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
							tmp = tmp->next;
							if((j%3 == 2) || (j == wtp->acl_conf.num_accept_mac-1))
								vty_out(vty,"\n");
						}
					}
					dcli_free_ac(ac);
				}
				else if(ret == ASD_WTP_NOT_EXIST) 
					vty_out(vty,"<error> wtp isn't existed\n");
				else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	local_hansi_parameter:
			ac = NULL;
			ac = show_all_wtp_maclist(dcli_dbus_connection, profile, localid, &ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)){		
				char *maclist_name[3]={"none","black","white"};
				wtp = NULL;
				for(i=0; i< ac->num_wtp; i++){
					if(wtp == NULL)
						wtp = ac->wtp_list;
					else 
						wtp = wtp->next;
			
					if(wtp == NULL)
						break;
					vty_out(vty,"Wtp %d use %s list \n",wtp->WtpID,maclist_name[wtp->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",wtp->acl_conf.num_deny_mac);
					tmp = wtp->acl_conf.deny_mac;
					for(j=0; j<wtp->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
						tmp = tmp->next;
						if((j%3 == 2) || (j == wtp->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",wtp->acl_conf.num_accept_mac);
					tmp = wtp->acl_conf.accept_mac;
					for(j=0; j<wtp->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
						tmp = tmp->next;
						if((j%3 == 2) || (j == wtp->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
				}
				dcli_free_ac(ac);
			}
			else if(ret == ASD_WTP_NOT_EXIST) 
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == ASD_WTP_ID_LARGE_THAN_MAX)
				vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}
	return CMD_SUCCESS; 
}

#else
DEFUN(show_all_wtp_MAC_list_cmd_func,
		show_all_wtp_MAC_list_cmd,
		"show all wtp mac list ",
		"wtp configure\n"
		"show wtp mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	unsigned int wtp_num = 0;
	int i;

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show all wtp mac list failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&wtp_num);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<wtp_num; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned int maclist_acl;
		unsigned int wtp_id = 0;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(wtp_id));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"wtp %d use %s list \n",wtp_id,maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
			for(j=0; j<num[i]; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char mac[MAC_LEN];

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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		dbus_message_iter_next(&iter_array);
	}

	if(wtp_num == 0)
		vty_out(vty,"there is no wtp\n");
	
	dbus_message_unref(reply);

	return ;	
}
#endif


#if DCLI_NEW
DEFUN(show_all_bss_MAC_list_cmd_func,
		show_all_bss_MAC_list_cmd,
		"show all bss mac list [remote] [local] [PARAMETER]",
		SHOW_STR
		"all bss\n"
		"bss configure\n"
		"show bss mac list\n" 
		"show bss mac list\n" 
		"'remote' or 'local' hansi\n"
		"'remote' or 'local' hansi\n"
		"slotid-instid\n"
		)
{	
	 
	unsigned int bss_num = 0;
	unsigned int ret = 0;
	int i,j;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	struct dcli_ac_info  *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	struct maclist		 *tmp = NULL;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_all_bss_maclist(dcli_dbus_connection, index, localid, &ret);
		
		if((ret == 0) && (ac != NULL)){		
			char *maclist_name[3]={"none","black","white"};
			for(i=0; i< ac->num_bss; i++){
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;
		
				if(bss == NULL)
					break;
				
				vty_out(vty,"\n");
				vty_out(vty,"radio %d wlan %d use %s list \n",bss->Radio_G_ID,bss->WlanID,maclist_name[bss->acl_conf.macaddr_acl]);
				vty_out(vty,"Black mac list(%d):\n",bss->acl_conf.num_deny_mac);
				tmp = bss->acl_conf.deny_mac;
				for(j=0; j<bss->acl_conf.num_deny_mac; j++){
					if(tmp == NULL)
						break;
					vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
					tmp = tmp->next;
					if((j%3 == 2) || (j == bss->acl_conf.num_deny_mac-1))
						vty_out(vty,"\n");
				}
				
				vty_out(vty,"White mac list(%d):\n",bss->acl_conf.num_accept_mac);
				tmp = bss->acl_conf.accept_mac;
				for(j=0; j<bss->acl_conf.num_accept_mac; j++){
					if(tmp == NULL)
						break;
					vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
					tmp = tmp->next;
					if((j%3 == 2) || (j == bss->acl_conf.num_accept_mac-1))
						vty_out(vty,"\n");
				}
			}
			dcli_free_ac(ac);
		}
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				ac = NULL;
				ac = show_all_bss_maclist(dcli_dbus_connection, profile, localid, &ret);
				
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)){		
					char *maclist_name[3]={"none","black","white"};
					bss = NULL;
					for(i=0; i< ac->num_bss; i++){
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;
				
						if(bss == NULL)
							break;
						
						vty_out(vty,"\n");
						vty_out(vty,"radio %d wlan %d use %s list \n",bss->Radio_G_ID,bss->WlanID,maclist_name[bss->acl_conf.macaddr_acl]);
						vty_out(vty,"Black mac list(%d):\n",bss->acl_conf.num_deny_mac);
						tmp = bss->acl_conf.deny_mac;
						for(j=0; j<bss->acl_conf.num_deny_mac; j++){
							if(tmp == NULL)
								break;
							vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
							tmp = tmp->next;
							if((j%3 == 2) || (j == bss->acl_conf.num_deny_mac-1))
								vty_out(vty,"\n");
						}
						
						vty_out(vty,"White mac list(%d):\n",bss->acl_conf.num_accept_mac);
						tmp = bss->acl_conf.accept_mac;
						for(j=0; j<bss->acl_conf.num_accept_mac; j++){
							if(tmp == NULL)
								break;
							vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
							tmp = tmp->next;
							if((j%3 == 2) || (j == bss->acl_conf.num_accept_mac-1))
								vty_out(vty,"\n");
						}
					}
					dcli_free_ac(ac);
				}
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}

			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}

		local_hansi_parameter:
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			ac = NULL;
			ac = show_all_bss_maclist(dcli_dbus_connection, profile, localid, &ret);

			if((ret == 0) && (ac != NULL)){ 	
				char *maclist_name[3]={"none","black","white"};
				bss = NULL;
				for(i=0; i< ac->num_bss; i++){
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					
					vty_out(vty,"\n");
					vty_out(vty,"radio %d wlan %d use %s list \n",bss->Radio_G_ID,bss->WlanID,maclist_name[bss->acl_conf.macaddr_acl]);
					vty_out(vty,"Black mac list(%d):\n",bss->acl_conf.num_deny_mac);
					tmp = bss->acl_conf.deny_mac;
					for(j=0; j<bss->acl_conf.num_deny_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
						tmp = tmp->next;
						if((j%3 == 2) || (j == bss->acl_conf.num_deny_mac-1))
							vty_out(vty,"\n");
					}
					
					vty_out(vty,"White mac list(%d):\n",bss->acl_conf.num_accept_mac);
					tmp = bss->acl_conf.accept_mac;
					for(j=0; j<bss->acl_conf.num_accept_mac; j++){
						if(tmp == NULL)
							break;
						vty_out(vty,MACSTR"\t",MAC2STR(tmp->addr)); 	
						tmp = tmp->next;
						if((j%3 == 2) || (j == bss->acl_conf.num_accept_mac-1))
							vty_out(vty,"\n");
					}
				}
				dcli_free_ac(ac);
			}
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS; 
}
	
#else
DEFUN(show_all_bss_MAC_list_cmd_func,
		show_all_bss_MAC_list_cmd,
		"show all bss mac list ",
		"bss configure\n"
		"show bss mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	unsigned int bss_num = 0;
	int i = 0;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show all bss mac list failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&bss_num);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);

	for(i=0; i<bss_num; i++){		
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array; 	
		unsigned int maclist_acl;
		unsigned int radio = 0;
		unsigned char bss = 0;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(radio));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(bss));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"radio %d bss %d use %s list \n",radio,bss,maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
			for(j=0; j<num[i]; j++){	
				DBusMessageIter iter_sub_struct;
				unsigned char mac[MAC_LEN];

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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		dbus_message_iter_next(&iter_array);
		
	}

	if(bss_num == 0)
		vty_out(vty,"there is no bss\n");
	
	dbus_message_unref(reply);

	return ;	
}

#endif

#if DCLI_NEW
DEFUN(show_wlan_wids_MAC_list_cmd_func,
		show_wlan_wids_MAC_list_cmd,
		"show wlan WLANID wids mac list",
		"wlan configure\n"
		"show wlan mac list\n" 
		)
{	
	 
	struct dcli_wlan_info 	*wlan = NULL;
	struct maclist		 	*tmp = NULL;
	unsigned int ret = 0;
	unsigned char wlan_id = 0;
	int i,j;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	unsigned char attack_type[WIDS_TYPE_LEN] = {0};
	unsigned char frame_type[WIDS_TYPE_LEN] = {0};
	unsigned int  added_time;
	time_t add_time,addtime;
	time_t now_time,now_sysrun;//qiuchen add it
	struct tm *tm_add_time;
	int hour,min,sec;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	wlan = show_wlan_wids_maclist(dcli_dbus_connection, index, wlan_id, localid, &ret);
	if((ret == 0) && (wlan != NULL)){		
		vty_out(vty,"Wlan %d wids %s\n",wlan_id,(wlan->acl_conf.wids_set == 1)?"enable":"disable");
		vty_out(vty,"Wids balck list num: %d\n",wlan->acl_conf.num_wids_mac);
		vty_out(vty,"Wids balck list last time %d(s)\n",wlan->acl_conf.wids_last_time);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"MAC               Attack_Bssid      A_Type F_Type  Added_Time Add_Time\n");
		tmp = wlan->acl_conf.deny_mac;
		for(i=0; i<wlan->acl_conf.num_wids_mac; i++){		
			if(tmp == NULL)
				break;
			CheckWIDSType(attack_type,frame_type,tmp->attacktype,tmp->frametype);
			time(&now_time);
			get_sysruntime(&now_sysrun);//qiuchen add it
			added_time = now_sysrun - tmp->add_time;
			addtime = now_time - added_time;
			hour=added_time/3600;
			min=(added_time-hour*3600)/60;
			sec=(added_time-hour*3600)%60;
			tm_add_time = localtime(&addtime);//qiuchen change it 2012.10.31
			
			vty_out(vty,MACSTR" ",MAC2STR(tmp->addr));
			vty_out(vty,MACSTR" ",MAC2STR(tmp->vapbssid));
			vty_out(vty,"%-6s ",attack_type);
			vty_out(vty,"%-7s ",frame_type);
			vty_out(vty,"%02d:%02d:%02d   ",hour,min,sec);
			vty_out(vty,"%02d:%02d:%02d\n",tm_add_time->tm_hour,tm_add_time->tm_min,tm_add_time->tm_sec);
			tmp = tmp->next;
		}
		vty_out(vty,"==============================================================================\n");
		dcli_free_wlan(wlan);
	}

	return CMD_SUCCESS; 
}
#else
DEFUN(show_wlan_wids_MAC_list_cmd_func,
		show_wlan_wids_MAC_list_cmd,
		"show wlan WLANID wids mac list",
		"wlan configure\n"
		"show wlan mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;

	unsigned int ret;
	unsigned int num_wids_mac = 0;
	unsigned char wlan_id = 0;

	unsigned char wids_set = 0;
	unsigned int  last_time = 0;
	unsigned char attack_type[WIDS_TYPE_LEN] = {0};
	unsigned char frame_type[WIDS_TYPE_LEN] = {0};
	unsigned char addr1[MAC_LEN] = {0}, addr2[MAC_LEN] = {0};
	unsigned char add_reason = 0;
	unsigned char attacktype = 0;
	unsigned char frametype = 0;
	unsigned char channel = 0;
	unsigned char rssi = 0;
	unsigned int  added_time;
	time_t add_time,addtime;
	time_t now_time,now;//qiuchen add it
	struct tm *tm_add_time;
	int hour,min,sec;
	int i=0, j=0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
	}
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST);*/

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show wlan %d mac list failed get reply.\n",wlan_id);
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==ASD_WLAN_NOT_EXIST) {
		vty_out(vty,"<error> wlan isn't existed\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&wids_set);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&num_wids_mac);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&last_time);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	vty_out(vty,"Wlan %d wids %s\n",wlan_id,(wids_set == 1)?"enable":"disable");
	vty_out(vty,"Wids balck list num: %d\n",num_wids_mac);
	vty_out(vty,"Wids balck list last time %d(s)\n",last_time);
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"MAC               Attack_Bssid      A_Type F_Type  Added_Time Add_Time\n");
	for(i=0; i<num_wids_mac; i++){		
		DBusMessageIter iter_struct;
		unsigned int maclist_acl;
		unsigned int num[2];
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(addr1[0]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr1[1]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr1[2]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr1[3]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr1[4]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr1[5]));

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&add_reason);	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr2[0]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr2[1]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr2[2]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr2[3]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr2[4]));	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(addr2[5]));

		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&attacktype);	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&frametype);	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&channel);	
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&rssi);
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&add_time);	
		
		dbus_message_iter_next(&iter_array);
		
		CheckWIDSType(attack_type,frame_type,attacktype,frametype);
		time(&now);
		get_sysruntime(&now_time);//qiuchen change it
		added_time = now_time - add_time;
		hour=added_time/3600;
		min=(added_time-hour*3600)/60;
		sec=(added_time-hour*3600)%60;
		addtime = now - added_time;//qiuchen add it
		tm_add_time = localtime(&addtime);
		
		vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X ",addr1[0],addr1[1],addr1[2],addr1[3],addr1[4],addr1[5]);
		/*vty_out(vty,"%s ",(add_reason == 1)?"dynamic":"static");*/
		vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X ",addr2[0],addr2[1],addr2[2],addr2[3],addr2[4],addr2[5]);
		vty_out(vty,"%-6s ",attack_type);
		vty_out(vty,"%-7s ",frame_type);
		/*
		vty_out(vty,"%4d ",channel);
		vty_out(vty,"%3d ",rssi);
		*/	
		vty_out(vty,"%02d:%02d:%02d   ",hour,min,sec);
		vty_out(vty,"%02d:%02d:%02d\n",tm_add_time->tm_hour,tm_add_time->tm_min,tm_add_time->tm_sec);
	}
	vty_out(vty,"==============================================================================\n");

	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
#endif
DEFUN(ac_add_MAC_list_cmd_func,
		ac_add_MAC_list_cmd,
		"ac add (black|white) list MAC",
		"ac configure\n"
		"ac add MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN] = {0};
	unsigned int ret;
	unsigned char list_type=0;   /*1--black list, 2--white list*/

	str2lower(&argv[0]);
	
	if (!strcmp(argv[0],"black")||(tolower(argv[0][0]) == 'b')){
		list_type=1;		
	}
	else if (!strcmp(argv[0],"white")||(tolower(argv[0][0]) == 'w')){
		list_type=2;		
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[1],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS){
		vty_out(vty,"wtp add mac list successfully!\n"); 
	}	
	else {
		vty_out(vty,"<error> ret = %d \n",ret); 
	}
	
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(ac_del_MAC_list_cmd_func,
		ac_del_MAC_list_cmd,
		"ac del (black|white) list MAC",
		"ac configure\n"
		"ac del MAC to list\n" 
		"type of list\n"
		"MAC list\n"
		"station MAC\n"
		)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char mac[MAC_LEN]={0};
	unsigned int ret;
	unsigned char list_type=0;   /*1--black list, 2--white list*/

	str2lower(&argv[0]);
	
	if (!strcmp(argv[0],"black")||(tolower(argv[0][0]) == 'b')){
		list_type=1;		
	}
	else if (!strcmp(argv[0],"white")||(tolower(argv[0][0]) == 'w')){
		list_type=2;		
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}
	
	ret = wid_parse_mac_addr((char *)argv[1],&mac);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
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
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS){
		vty_out(vty,"wtp del mac list successfully!\n"); 
	}	
	else if(ret == ASD_UNKNOWN_ID)
		vty_out(vty,"<error> mac is not in the list\n");
	else {
		vty_out(vty,"<error> ret = %d\n",ret); 
	}
	
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(ac_use_MAC_list_cmd_func,
		ac_use_MAC_list_cmd,
		"ac use (none|black|white) list ",
		"ac configure\n"
		"type of list\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char list_type=0;   /*0--none,1--black list, 2--white list*/
	
	str2lower(&argv[0]);
	
	if (!strcmp(argv[0],"none")||(tolower(argv[0][0]) == 'n')){
		list_type=0;			
	}
	else if (!strcmp(argv[0],"black")||(tolower(argv[0][0]) == 'b')){
		list_type=1;
	}
	else if (!strcmp(argv[0],"white")||(tolower(argv[0][0]) == 'w')){
		list_type=2;	
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'black/white' or 'b/w' \n");
		return CMD_SUCCESS;
	}

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_AC_USE_MAC_LIST);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"ac use mac list successfully!\n"); 
	else if(ret==ASD_WIDS_OPEN)
		vty_out(vty,"Wids is enable,ac can only use black list!\n"); 
	else 
		vty_out(vty,"<error> %d\n",ret); 
	
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(show_ac_MAC_list_cmd_func,
		show_ac_MAC_list_cmd,
		"show ac mac list",
		"ac configure\n"
		"show  mac list\n" 
		)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
	DBusMessageIter iter_sub_struct;
	DBusError err;

	unsigned int ret = 0;
	char *showStr = NULL ;

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SHOW_AC_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"show mac list failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	{		
		unsigned int maclist_acl;
		unsigned int num[2];
		char *maclist_name[3]={"none","black","white"};
		int i=0, j=0;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(maclist_acl));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[0]));	
		
		dbus_message_iter_next(&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(num[1]));	
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		
		vty_out(vty,"ac use %s list \n",maclist_name[maclist_acl]);

		for(i=0; i<2; i++) {
			vty_out(vty,"%s mac list(%d):\n",maclist_name[i+1],num[i]);
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
				
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\t",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);		
				if((j%3 == 2) || (j == num[i]-1))
					vty_out(vty,"\n");
			}
		}
		dbus_message_iter_next(&iter_array);
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}


DEFUN(set_sta_static_info_traffic_limit_func,
	  set_sta_static_info_traffic_limit_cmd,
	  "set static sta MAC (vlanid | limit | send_limit) VALUE by wlan [wlanid]",
	  "set station static info\n"
	  "ASD station information\n"
	 )
{	
	 
	struct dcli_sta_info *sta = NULL;
	unsigned int 	value = 0;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned char   type = 0;
	unsigned int	ret=0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	unsigned char 	wlan_id=0;
	
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	

	if (!strcmp(argv[1],"vlanid")) {
		type = 0;	
	}else if (!strcmp(argv[1],"limit")) {
		type = 1;	
	}else if(!strcmp(argv[1],"send_limit")) {
		type = 2;
	}else {
		vty_out(vty,"<error> input parameter should only be 'vlanid' or 'limit' or 'send_limit'\n");
		return CMD_SUCCESS;
	}

	value = atoi(argv[2]);
	ret = parse2_int_ID((char *)argv[2],&value);
	
	if((type == 0) && (value > 4095 )){
		vty_out(vty,"<error> vlan id should be 0 to %d.\n",4095);
		return CMD_SUCCESS;
	}

	if(4 == argc)
	{
		ret = parse2_char_ID((char *)argv[3],&wlan_id);
		if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error>wlanid is not legal\n");
			return CMD_SUCCESS;
		}
		if(wlan_id < 0 || wlan_id > 128){
			vty_out(vty,"<error>wlanid should be 1 to %d\n",WLAN_NUM-1);
			return CMD_SUCCESS;
		}
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	sta = check_sta_by_mac(dcli_dbus_connection,index,mac1,type,value, localid,&ret);
	if((ret == 0) && (sta != NULL)){
		if((4 == argc)&&(wlan_id == sta->wlan_id)){
			wid_set_sta_info(dcli_dbus_connection,index,mac1,sta->wlan_id,sta->radio_g_id,type,value,localid,&ret);
			if(ret == 0){
				set_sta_static_info(dcli_dbus_connection,index,mac1,sta->wlan_id,sta->radio_g_id,type,value,localid,&ret);
				if(ret == 0){
					vty_out(vty,"set static info successfully.\n");
				}else if (ret == ASD_DBUS_ERROR)
					vty_out(vty,"<error> set info failed get reply.\n");
				else
					vty_out(vty,"<error> set info ret = %d.\n",ret);
			}else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret == IF_POLICY_CONFLICT)
				vty_out(vty,"<error> station traffic limit send value is more than bss traffic limit send value\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bing wlan %s\n",sta->wlan_id);
			else if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else
				vty_out(vty,"<error> wid set error ret = %d\n",ret);
			dcli_free_sta(sta);
		}
		else if((3 == argc)||((4 == argc)&&(wlan_id != sta->wlan_id))){
			if(3 == argc)
				wid_set_sta_info(dcli_dbus_connection,index,mac1,sta->wlan_id,sta->radio_g_id,type,value,localid,&ret);
			else
				wid_set_sta_info(dcli_dbus_connection,index,mac1,wlan_id,sta->radio_g_id,type,value,localid,&ret);
			if(ret == 0){
				set_sta_static_info(dcli_dbus_connection,index,mac1,wlan_id,sta->radio_g_id,type,value,localid,&ret);
				if(ret == 0){
					vty_out(vty,"set static info successfully.\n");
				}else if (ret == ASD_DBUS_ERROR)
					vty_out(vty,"<error> set info failed get reply.\n");
				else
					vty_out(vty,"<error> set info ret = %d.\n",ret);
			}else if(ret == WTP_ID_NOT_EXIST)
				vty_out(vty,"<error> wtp isn't existed\n");
			else if(ret == RADIO_ID_NOT_EXIST)
				vty_out(vty,"<error> radio isn't existed\n");
			else if(ret == IF_POLICY_CONFLICT)
				vty_out(vty,"<error> station traffic limit send value is more than bss traffic limit send value\n");
			else if(ret==RADIO_NO_BINDING_WLAN)
				vty_out(vty,"<error> radio doesn't bing wlan %d\n",wlan_id);
			else if(ret==WLAN_ID_NOT_EXIST)
				vty_out(vty,"<error> wlan isn't existed\n");
			else if(ret==WTP_IS_NOT_BINDING_WLAN_ID)
				vty_out(vty,"<error> wtp doesn't bing wlan %d\n",wlan_id);
			else
				vty_out(vty,"<error> wid set error ret = %d\n",ret);
			dcli_free_sta(sta);
		}
		else
			vty_out(vty,"<error> input wlanid is not sta accessed wlan.\n");
	}else if (ret == ASD_STA_NOT_EXIST){
		set_sta_static_info(dcli_dbus_connection,index,mac1,wlan_id,0,type,value,localid,&ret);
		if(ret == 0){
			vty_out(vty,"set static info successfully.\n");
		}else if (ret == ASD_DBUS_ERROR)
			vty_out(vty,"<error> set info failed get reply.\n");
		else
			vty_out(vty,"<error> set info ret = %d.\n",ret);
	}else if (ret == ASD_DBUS_SET_ERROR){
			vty_out(vty,"<error> check sta set invalid value.\n");
	}else if (ret == ASD_DBUS_ERROR)
		vty_out(vty,"<error> check sta failed get reply.\n");
	else
		vty_out(vty,"<error> check sta error ret = %d.\n",ret);

	return CMD_SUCCESS;
}


DEFUN(del_sta_static_info_func,
	  del_sta_static_info_cmd,
	  "(del | delete) static sta MAC by wlan [wlanid]",
	  "del station\n"
	  "ASD station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to del\n"
	 )
{	
	 
	struct sta_static_info *sta = NULL;
	unsigned int	mac[MAC_LEN]={0};
	unsigned char	mac1[MAC_LEN]={0};
	unsigned int	ret=0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	unsigned char 	wlan_id=0;
	char c = 0;
	
	sscanf(argv[1],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	

	if(3 == argc)
	{
		ret = parse2_char_ID((char *)argv[2],&wlan_id);
		c = (char *)argv[2][0];
		if(c == '0'){
			ret = WID_DBUS_SUCCESS;
			wlan_id = 0;
		}
		if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error>wlanid is not legal\n");
			return CMD_SUCCESS;
		}
		if(wlan_id > 128){
			vty_out(vty,"<error>wlanid should be 0 to %d\n",WLAN_NUM-1);
			return CMD_SUCCESS;
		}
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	del_sta_static_info(dcli_dbus_connection,index,mac1,wlan_id,localid,&ret);
	if(ret == 0) {		
		vty_out(vty,"del static sta successfully.\n");
	}else if (ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist\n");
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);

	return CMD_SUCCESS;
}



DEFUN(show_sta_mac_static_info_func,
	  show_sta_mac_static_info_cmd,
	  "show static sta MAC info by wlan [wlanid]",
	  CONFIG_STR
	  "ASD static station information\n"
	  "sta MAC (like 00:19:E0:81:48:B5) that you want to check\n"
	 )
{	
	 
	struct sta_static_info *tab = NULL;
	struct sta_static_info *sta = NULL;
	unsigned int 	mac[MAC_LEN]={0};
	unsigned char 	mac1[MAC_LEN]={0};
	unsigned char 	wlan_id=0;
	unsigned int 	ret=0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	

	if(2 == argc)
	{
		ret = parse2_char_ID((char *)argv[1],&wlan_id);
		if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error>wlanid is not legal\n");
			return CMD_SUCCESS;
		}
		if(wlan_id > 128){
			vty_out(vty,"<error>wlanid should be 1 to %d\n",WLAN_NUM-1);
			return CMD_SUCCESS;
		}
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	tab = show_sta_static_info_bymac(dcli_dbus_connection,index,mac1,wlan_id, localid,&ret);
	if((ret == 0) && (tab != NULL)){		
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"STA_MAC            VlanID  Up_Link  Down_link  WlanID\n");
		for(sta=tab; sta != NULL; sta=sta->next) {
			vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
			vty_out(vty,"%-4d    ",sta->vlan_id);
			vty_out(vty,"%-6d   ",sta->sta_traffic_limit);
			vty_out(vty,"%-6d     ",sta->sta_send_traffic_limit);
			vty_out(vty,"%-4d \n",sta->wlan_id);
		}
		vty_out(vty,"==============================================================================\n");
		dcli_free_static_sta_tab(tab);
	}else if (ret == ASD_STA_NOT_EXIST)
		vty_out(vty,"<error> station does not exist\n");
	else if (ret == ASD_DBUS_ERROR)
		cli_syslog_info("<error> failed get reply.\n");
	else
		vty_out(vty,"<error> ret = %d\n",ret);

	return CMD_SUCCESS;
}

DEFUN(show_all_sta_static_info_func,
	  show_all_sta_static_info_cmd,
	  "show all static sta info [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "all static sta\n"
	  "static sta\n"
	  "static sta info\n"
	  "ASD static station information\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct sta_static_info *tab = NULL;
	struct sta_static_info *sta = NULL;
	unsigned int 	num=0;
	unsigned int 	ret=0;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		tab = show_sta_static_info(dcli_dbus_connection,index,&num, localid,&ret);
		if((ret == 0) && (tab != NULL)){		
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"STA_MAC            VlanID  Up_Link  Down_Link  WlanID\n");
			for(sta=tab; sta != NULL; sta=sta->next) {
				vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
				vty_out(vty,"%-4d    ",sta->vlan_id);
				vty_out(vty,"%-6d   ",sta->sta_traffic_limit);
				vty_out(vty,"%-6d     ",sta->sta_send_traffic_limit);
				vty_out(vty,"%-6d \n",sta->wlan_id);
			}
			vty_out(vty,"==============================================================================\n");
			dcli_free_static_sta_tab(tab);
		}else if((ret == 0) && (num == 0)){
			vty_out(vty,"<error> there is no static sta.\n");
		}else if (ret == ASD_STA_NOT_EXIST)
			vty_out(vty,"<error> station does not exist.\n");
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				tab = NULL;
				tab = show_sta_static_info(dcli_dbus_connection,profile,&num, localid,&ret);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if((ret == 0) && (tab != NULL)){		
					vty_out(vty,"STA_MAC            VlanID  Up_Link  Down_Link  WlanID\n");
					for(sta=tab; sta != NULL; sta=sta->next) {
						vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
						vty_out(vty,"%-4d    ",sta->vlan_id);
						vty_out(vty,"%-6d   ",sta->sta_traffic_limit);
						vty_out(vty,"%-6d     ",sta->sta_send_traffic_limit);
						vty_out(vty,"%-6d \n",sta->wlan_id);
					}
					dcli_free_static_sta_tab(tab);
				}else if((ret == 0) && (num == 0)){
					vty_out(vty,"<error> there is no static sta.\n");
				}else if (ret == ASD_STA_NOT_EXIST)
					vty_out(vty,"<error> station does not exist.\n");
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
		local_hansi_parameter:
			tab = NULL;
			tab = show_sta_static_info(dcli_dbus_connection,profile,&num, localid,&ret);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if((ret == 0) && (tab != NULL)){		
				vty_out(vty,"STA_MAC            VlanID  Up_Link  Down_Link  WlanID\n");
				for(sta=tab; sta != NULL; sta=sta->next) {
					vty_out(vty,MACSTR"  ",MAC2STR(sta->addr));
					vty_out(vty,"%-4d    ",sta->vlan_id);
					vty_out(vty,"%-6d   ",sta->sta_traffic_limit);
					vty_out(vty,"%-6d     ",sta->sta_send_traffic_limit);
					vty_out(vty,"%-6d \n",sta->wlan_id);
				}
				dcli_free_static_sta_tab(tab);
			}else if((ret == 0) && (num == 0)){
				vty_out(vty,"<error> there is no static sta.\n");
			}else if (ret == ASD_STA_NOT_EXIST)
				vty_out(vty,"<error> station does not exist.\n");
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;
}
DEFUN(sta_arp_set_cmd_func,
	  sta_arp_set_cmd,
	  "(add|del) arp IP MAC base IFNAME",
	  "sta station arp\n"
	  "ip address\n"	  
	  "sta MAC (like 00:19:E0:81:48:B5)\n"
	  "ifname like eth0-1"
	 )
{
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int ret;
	int op_ret;
	unsigned char macAddr[MAC_LEN];
	char *ip;
	char *ifname;
	char *mac;
	int is_add = 0;
	int len = strlen(argv[0]);
	if(len <= 3 && strncmp(argv[0],"add",len) == 0){
		is_add = 1;
	}else if(len <= 3 && strncmp(argv[0],"del",len) == 0){
		is_add = 2;
	}else if(len <= 6 && strncmp(argv[0],"change",len) == 0){
		is_add = 3;
	}else if(len <= 7 && strncmp(argv[0],"replace",len) == 0){
		is_add = 4;
	}else{
    	vty_out(vty,"Unkown Command!\n");
		return CMD_WARNING;		
	}
	ret = Check_IP_Format((char*)argv[1]);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	ip = malloc(strlen(argv[1])+1);
	memset(ip, 0, strlen(argv[1])+1);
	memcpy(ip, argv[1],strlen(argv[1]));
	
	op_ret = wid_parse_mac_addr(argv[2],&macAddr);

	if (NPD_FAIL == op_ret) {
		free(ip);
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
	
	mac = malloc(strlen(argv[2])+1);
	memset(mac, 0, strlen(argv[2])+1);
	memcpy(mac, argv[2], strlen(argv[2]));

	ifname = malloc(strlen(argv[3])+1);
	memset(ifname, 0, strlen(argv[3])+1);
	memcpy(ifname, argv[3], strlen(argv[3]));

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ret = dcli_asd_set_sta_arp(index, localid, is_add, ip, mac, ifname, dcli_dbus_connection);
	free(ip);
	free(ifname);
	free(mac);
	if(ret == ASD_DBUS_SUCCESS)
		vty_out(vty,"%s arp %s %s base %s successfully!\n",argv[0],argv[1],argv[2],argv[3]);
	else 
		vty_out(vty,"%s arp %s %s base %s failed!\n",argv[0],argv[1],argv[2],argv[3]);
	return CMD_SUCCESS;
}
DEFUN(set_asd_sta_arp_listen_cmd_func,
		set_asd_sta_arp_listen_cmd,
		"set asd station arp (listen|listen_and_set) (enable|disable)",
		"asd sta arp listen\n"
		"listen arp info \n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char op = 0;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/	
	
	str2lower(&argv[0]);
	
	if (!strcmp(argv[0],"listen")){
		op=1; 	
	}else if (!strcmp(argv[0],"listen_and_set")){
		op=2; 	
	}else{
		return CMD_SUCCESS;
	}
	
	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e')){
		type=1;		
	}else if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd')){
		type=0;		
	}else{
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&op,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==0)
		vty_out(vty,"set asd sta arp listen %s successful!\n",type?"enable":"disable"); 
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
//weichao add
DEFUN(set_asd_sta_get_ip_from_dhcpsnoop_cmd_func,
		set_asd_sta_get_ip_from_dhcpsnoop_cmd,
		"set asd sta get ip from dhcpsnooping  (enable|disable)",
		"asd sta get ip\n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/	
	
	str2lower(&argv[0]);
	
	if (!strcmp(argv[0],"enable")||(tolower(argv[0][0]) == 'e')){
		type=1;		
	}else if (!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd')){
		type=0;		
	}else{
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_IP_FROM_DHCPSNOOP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd sta get ip from dhcpsnoop %s successful!\n",type?"enable":"disable"); 
	else
		vty_out(vty,"<error> %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

DEFUN(set_asd_sta_static_arp_cmd_func,
		set_sta_static_arp_cmd,
		"set (asd|dhcpsnooping|both) static arp op (enable|disable)",
		"sta static arp setting\n"
		"asd or dhcpsnooping\n"
		" \n"
		)
{

	DBusMessage *query, *reply;		
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int both = 0;
	unsigned char type=0;   /*1--open*/
							/* 0--close*/	
	str2lower(&argv[0]);
	
	if (!strcmp(argv[0],"asd")||(tolower(argv[0][0]) == 'a')){
		both = 2;		
	}else if (!strcmp(argv[0],"dhcpsnooping")||(tolower(argv[0][0]) == 'd')){
		both = 3;		
	}else if (!strcmp(argv[0],"both")||(tolower(argv[0][0]) == 'b')){
		both = 1;
	}

	str2lower(&argv[1]);
	
	if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e')){
		type=1;		
	}else if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd')){
		type=0;		
	}else{
		return CMD_SUCCESS;
	}		
	
	if(both == 1 || both == 2){
		int localid = 1;int slot_id = HostSlotId;int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if (vty->node == LOCAL_HANSI_NODE){
			index = vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP);
//		query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
//							ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID);

		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		if(ret==0)
			vty_out(vty,"set asd sta static arp op %s successful!\n",type?"enable":"disable"); 
		dbus_message_unref(reply);
	}

	if(both == 1 || both == 3){
		#if 0
		query = dbus_message_new_method_call(DHCPSNP_DBUS_BUSNAME,DHCPSNP_DBUS_OBJPATH,\
							DHCPSNP_DBUS_INTERFACE,DHCPSNP_DBUS_METHOD_INTERFACE_ONOFF_STA_STATIIC_ARP);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID);

		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		if(ret==0)
			vty_out(vty,"set dhcpsnooping sta static arp op %s successful!\n",type?"enable":"disable"); 
		dbus_message_unref(reply);
		#endif
	}

	return CMD_SUCCESS; 
}

DEFUN(set_sta_arp_group_cmd_func,
		set_sta_arp_group_cmd,
		"(add|del|change) asd sta gateway IFNAME as arp group ID",
		"set asd sta gateway\n"
		"listen arp info \n"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned char ID;
	char *name = 0;	
	int is_add = 0;
	int len = strlen(argv[0]);
	if(len <= 3 && strncmp(argv[0],"add",len) == 0){
		is_add = 1;
	}else if(len <= 3 && strncmp(argv[0],"del",len) == 0){
		is_add = 2;
	}else if(len <= 6 && strncmp(argv[0],"change",len) == 0){
		is_add = 3;
	}else{
    	vty_out(vty,"Unkown Command!\n");
		return CMD_WARNING;		
	}

	
	name = (char *)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name,argv[1],strlen(argv[1]));
	
	ret = parse_char_ID((char*)argv[2], &ID);

	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
		free(name);
		return CMD_SUCCESS;
	}	
	if(ID >= WLAN_NUM || ID == 0){
		vty_out(vty,"<error> group id should be 1 to %d\n",WLAN_NUM-1);
		free(name);
		return CMD_SUCCESS;
	}
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP_IF_GROUP);
//	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
//						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP_IF_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&is_add,
							DBUS_TYPE_BYTE,&ID,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret==0)
		vty_out(vty,"%s asd sta gateway %s as arp group %s successful!\n",argv[0],argv[1],argv[2]); 
	else
		vty_out(vty,"%s asd sta gateway %s as arp group %s failed!\n",argv[0],argv[1],argv[2]); 		
	dbus_message_unref(reply);
	free(name);
	name = NULL;
	return CMD_SUCCESS; 
}

DEFUN(set_asd_switch_func,
	  set_asd_switch_cmd,
	  "set asd switch (enable|disable)",
	  "set asd switch"
	  "asd and wifi commuticate switch"
	  "enable or disable"
	 )
{
	int ret;
	unsigned int type=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int localid = 1;int slot_id = HostSlotId;int index = 0;

	dbus_error_init(&err);
	

	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set asd switch %s successfully.\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
//weichao add 20110804
DEFUN(clean_wlan_accessed_sta_func,
	  clean_wlan_accessed_sta_cmd,
	  "clean wlan WLANID accessed sta",
	  CONFIG_STR
	  "clean wlan station information\n"
	  "search by WLANID\n"
	  "WLAN ID\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	int i;
	unsigned char wlan_id = 0;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_CLEAN_WLAN_STA);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret==0)
		vty_out(vty,"clean wlan accessed sta successful!\n"); 
	else if(ret == ASD_WLAN_NOT_EXIST)
		vty_out(vty,"<error>wlan isn't exist!\n"); 
	else
		vty_out(vty,"<error>ret = %d\n",ret); 

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_sta_hotspot_map_nas_cmd_func,
	  set_sta_hotspot_map_nas_cmd,
	  "set sta hotspotid HOTSPOTID map nas_port_id NAS_PORT_ID nas_identifier NAS_IDENTIFIER",
	  "set station\n"
	  "ASD station information\n"
	  "sta hotspotid\n"
	  "\n"
	  "map\n"
	  "nasportid\n"
	  "\n"
	  "nas_identifier\n"
	  
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	unsigned int hotspotid = 0 ;
	unsigned int len = 0;
	char *nas_port_id = NULL;
	char *nas_id = NULL;
	unsigned insize = 0 ; 
	
	ret = parse_int_ID((char*)argv[0], &hotspotid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(hotspotid > HOTSPOT_ID|| hotspotid == 0){
		vty_out(vty,"<error> hotspot id should be 1 to %d\n",HOTSPOT_ID);
		return CMD_SUCCESS;
	}
	
	len = strlen(argv[1]);
	if(len > 32){		
		vty_out(vty,"<error> nas-port-id name is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	
	nas_port_id = (char*)malloc(strlen(argv[1])+1);
	memset(nas_port_id, 0, strlen(argv[1])+1);
	memcpy(nas_port_id, argv[1], strlen(argv[1]));		
	
	insize = strlen(argv[2]);
	if(insize > 128)
	{
		vty_out(vty,"<error> the length of input parameter %s is %d ,excel the limit of 128\n",argv[2],insize);
		free(nas_port_id);
		nas_port_id = NULL;
		return CMD_SUCCESS;	
	}
	insize = strcheck(&argv[2]);	
	if(insize == 0)
	{
		vty_out(vty,"<error> nas identifier %s include unknow character\n",argv[2]);
		free(nas_port_id);
		nas_port_id = NULL;
		return CMD_SUCCESS;	
	}
	nas_id = (char*)malloc(strlen(argv[2])+1);
	memset(nas_id, 0, strlen(argv[2])+1);
	memcpy(nas_id, argv[2], strlen(argv[2]));	
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_HOTSPOT_MAP_NAS);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&hotspotid,
						   	 DBUS_TYPE_STRING,&nas_port_id,
						   	 DBUS_TYPE_STRING,&nas_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//qiuchen add it 2012.10.16
		free(nas_port_id);
		nas_port_id = NULL;
		free(nas_id);
		nas_id = NULL;
		//end
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
		vty_out(vty,"set station hotspotid %d map nas port %s identifier %s successfully!\n",hotspotid,nas_port_id,nas_id); 

	
	dbus_message_unref(reply);
	//qiuchen add it 2012.10.16
	free(nas_port_id);
	nas_port_id = NULL;
	free(nas_id);
	nas_id = NULL;
	//end
	return CMD_SUCCESS;
}
//qiuchen add it 2012.10.23
DEFUN(show_sta_hotspot_information_cmd_func,
		show_sta_hotspot_information_cmd,
		"show hotspot list",
		"show hotspot list\n"
		"hotspot info\n"
		"hotspot info\n"
		"hotspot info\n"
	)
{
	DBusMessage *query,*reply;
	DBusMessageIter iter;
	DBusError err;
 	int ret = 0;
	int i = 1;
	struct sta_nas_info *hs[HOTSPOT_ID+1] = {NULL};

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
		
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	show_hotspot_list(dcli_dbus_connection,hs,index,localid,&ret);	

	if(ret == 0){
		vty_out(vty,"==============================================================================\n");
		for(i=1;i<4097;i++){
			if(hs[i]){
				vty_out(vty,"hotspotID is %d\n",i);
				vty_out(vty,"nas_identifier is %s\n",hs[i]->nas_identifier);
				vty_out(vty,"nas_port_id is %s\n",hs[i]->nas_port_id);
				vty_out(vty,"------------------------------------------------------------------------------\n");
			}

		}
		vty_out(vty,"==============================================================================\n");
		dcli_free_hotspot_list(hs);
	}
	else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
	else
			vty_out(vty,"<error> ret = %d\n",ret);
	
	return CMD_SUCCESS;

	
}
//qiuchen add it 2012.10.24
DEFUN(del_sta_hotspot_cmd_func,
		del_sta_hotspot_cmd,
		"delete hotspotid HOTSPOTID",
		"delete hotspot\n"
		"delete hotspot\n"
		"hotspotid\n"
		)
{
	DBusMessage *query,*reply;
	DBusMessageIter iter;
	DBusError err;
 	int ret = 0;
	unsigned int hotspotid = 0;

	ret = parse_int_ID((char*)argv[0],&hotspotid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(hotspotid > HOTSPOT_ID|| hotspotid == 0){
		vty_out(vty,"<error> hotspot id should be 1 to %d\n",HOTSPOT_ID);
		return CMD_SUCCESS;
	}

	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
		
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_DEL_HOTSPOT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&hotspotid,
						 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
		
	if (NULL == reply) {
	vty_out(vty,"<error> failed get reply.\n");
	if (dbus_error_is_set(&err)) {
		vty_out(vty,"%s raised: %s",err.name,err.message);
		dbus_error_free_for_dcli(&err);
	}
	return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
		vty_out(vty,"Delete station hotspotid %d successfully!\n",hotspotid); 
	else if(ret ==  ASD_DBUS_HOTSPOTID_NOT_EXIST)
		vty_out(vty,"Hotspot ID %d is not exist!\n",hotspotid);
	else
		vty_out(vty,"<error> ret = %d",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_sta_list_detail_cmd_func,
	  show_sta_list_detial_cmd,
	  "show sta list detail [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station list information\n"
	  "detail information"	
	  "all sta\n"
	  "all sta\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_ac_info *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int m = 0;
	char SecurityType[20] = {0};
	char *sta_method = NULL;
	char *username = NULL;
	char onlinetime[10] = {0};
	char sta_exist = 0;

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_sta_list(dcli_dbus_connection,index, localid,&ret);

		if((ret == 0) && (ac != NULL)) {
			sta_exist = 0;
			for(i=0; i<ac->num_bss_wireless; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				if(bss->num_sta >0 )
				{
					sta_exist = 1;
					vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
				for(j=0; j<bss->num_sta; j++){	//qiuchen change it 1012.10.22
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020))
					{
    					//time_t online_time,now_sysrun;
    					//time(&now);
    					//get_sysruntime(&now_sysrun);
    					time_t online_time=sta->sta_online_time_new;
    					int hour,min,sec;
    					
    					hour=online_time/3600;
    					min=(online_time-hour*3600)/60;
    					sec=(online_time-hour*3600)%60;
    					memset(onlinetime,0,10);
    					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
    					

    					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
    					vty_out(vty,"%-15s ",sta->ip);
    					vty_out(vty,"%-7llu ",sta->txbytes);
    					vty_out(vty,"%-6llu ",sta->rxbytes);		
    					
    					if(!strcmp(sta_method,"Unknown"))
    						vty_out(vty,"%-5s ",SecurityType);
    					else
    						vty_out(vty,"%-5s ",sta_method);
    					vty_out(vty,"%-11s ",onlinetime);
    					vty_out(vty,"%-s\n",sta->identify);
    					vty_out(vty,"ipv6 address:    ");     /* add for ipv6 sta */
    					for (m = 0; m < 8; m++)
                    	{   
    						if(m==7)
    						{
                                vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[m]);
    						}
    					    else
    					    {
                                vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[m]);
    					    }
                    	}
					}
				}
				
				vty_out(vty,"==============================================================================\n");
				}
			}
			

			 if(sta_exist == 0)
				vty_out(vty,"<error> there is no station\n");
			dcli_free_ac(ac);
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"------------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)) {
					sta_exist = 0;
					for(i=0; i<ac->num_bss_wireless; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
						if(bss->num_sta >0 )
						{
							sta_exist  = 1;
							vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
							vty_out(vty,"==============================================================================\n");
							vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
						for(j=0; j<bss->num_sta; j++){	//qiuchen change it 1012.10.22
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020)){
					//time_t online_time,now_sysrun;
					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					int hour,min,sec;
					
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					memset(onlinetime,0,10);
					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
					

					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s ",sta->ip);
					vty_out(vty,"%-7llu ",sta->txbytes);
					vty_out(vty,"%-6llu ",sta->rxbytes);		
					
					if(!strcmp(sta_method,"Unknown"))
						vty_out(vty,"%-5s ",SecurityType);
					else
						vty_out(vty,"%-5s ",sta_method);
					vty_out(vty,"%-11s ",onlinetime);
					vty_out(vty,"%-s\n",sta->identify);
					}
				}
						
						vty_out(vty,"==============================================================================\n");
						}
					}
					

					if(sta_exist == 0)
						vty_out(vty,"<error> there is no station\n");
					dcli_free_ac(ac);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)) {
				sta_exist =0 ;
				for(i=0; i<ac->num_bss_wireless; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					if(bss->num_sta >0 )
					{
						sta_exist = 1;
						vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
						vty_out(vty,"==============================================================================\n");
						vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
					for(j=0; j<bss->num_sta; j++){	//qiuchen change it 1012.10.22
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020)){
					//time_t online_time,now_sysrun;
					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					int hour,min,sec;
					
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					memset(onlinetime,0,10);
					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
					
					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s ",sta->ip);
					vty_out(vty,"%-7llu ",sta->txbytes);
					vty_out(vty,"%-6llu ",sta->rxbytes);		
					
					if(!strcmp(sta_method,"Unknown"))
						vty_out(vty,"%-5s ",SecurityType);
					else
						vty_out(vty,"%-5s ",sta_method);
					vty_out(vty,"%-11s ",onlinetime);
					vty_out(vty,"%-s\n",sta->identify);
					}
				}
					
					vty_out(vty,"==============================================================================\n");
					}
				}
				

				 if(sta_exist == 0)
					vty_out(vty,"<error> there is no station\n");
				dcli_free_ac(ac);
			}	
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}
DEFUN(show_sta_detail_bywlan_cmd_func,
	  show_sta_detial_bywlan_cmd,
	  "show sta detail bywlanid WLANID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station list information\n"
	  "detail information"	
	  "all sta\n"
	  "all sta\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_ac_info *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned int ret = 0;
	int i,j,m;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	char SecurityType[20] = {0};
	char *sta_method = NULL;
	char *username = NULL;
	char onlinetime[10] = {0};
	unsigned char wlan_id = 0;
	char sta_exist = 0;
	char bss_exist = 0;
	
	ret = parse2_char_ID((char*)argv[0], &wlan_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_sta_list(dcli_dbus_connection,index, localid,&ret);

		if((ret == 0) && (ac != NULL)) {
			bss_exist = 0;
			sta_exist = 0;
			for(i=0; i<ac->num_bss_wireless; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				if(bss->WlanID != wlan_id)
					continue;
				else
					bss_exist = 1;
				if(bss->num_sta >0 )
				{
					sta_exist = 1;
					vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
					for(j=0; j<bss->num_sta; j++){	//qiuchen
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020))
					{
    					//time_t online_time,now_sysrun;//qiuchen
    					//time(&now);
    					//get_sysruntime(&now_sysrun);
    					time_t online_time=sta->sta_online_time_new;
    					int hour,min,sec;
    					
    					hour=online_time/3600;
    					min=(online_time-hour*3600)/60;
    					sec=(online_time-hour*3600)%60;
    					memset(onlinetime,0,10);
    					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
    					
    					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
    					vty_out(vty,"%-15s ",sta->ip);
    					vty_out(vty,"%-7llu ",sta->txbytes);
    					vty_out(vty,"%-6llu ",sta->rxbytes);		
    					
    					if(!strcmp(sta_method,"Unknown"))
    						vty_out(vty,"%-5s ",SecurityType);
    					else
    						vty_out(vty,"%-5s ",sta_method);
    					
    					vty_out(vty,"%-11s ",onlinetime);
    					vty_out(vty,"%-s\n",sta->identify);
						
    					vty_out(vty,"ipv6 address:     ");     /* add ipv6 addres for sta */
    					for (m = 0; m < 8; m++)
                    	{   
    						if(m==7)
    						{
                                vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[m]);
    						}
    					    else
    					    {
                                vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[m]);
    					    }
                    	}					
					}
				}
				
				vty_out(vty,"==============================================================================\n");
				}
			}
			
			if(bss_exist == 0)
				vty_out(vty,"<error> wlan %d is not exist!or there  is no bss in wlan %d\n",wlan_id,wlan_id);
			else if(sta_exist == 0)
				vty_out(vty,"<error> there is no station\n");
			dcli_free_ac(ac);
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"------------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)) {
					bss_exist = 0;
					sta_exist = 0;
					for(i=0; i<ac->num_bss_wireless; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
						
						if(bss->WlanID != wlan_id)
							continue;
						else
							bss_exist = 1;
						if(bss->num_sta >0 )
						{
							sta_exist = 1;
							vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
							vty_out(vty,"==============================================================================\n");
							vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
						for(j=0; j<bss->num_sta; j++){	//qiuchen
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020)){
					//time_t online_time,now_sysrun;
					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					int hour,min,sec;
					
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					memset(onlinetime,0,10);
					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
					
					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s ",sta->ip);
					vty_out(vty,"%-7llu ",sta->txbytes);
					vty_out(vty,"%-6llu ",sta->rxbytes);		
					
					if(!strcmp(sta_method,"Unknown"))
						vty_out(vty,"%-5s ",SecurityType);
					else
						vty_out(vty,"%-5s ",sta_method);
					vty_out(vty,"%-11s ",onlinetime);
					vty_out(vty,"%-s\n",sta->identify);
					}
				}
						
						vty_out(vty,"==============================================================================\n");
						}
					}
					
					if(bss_exist == 0)
						vty_out(vty,"<error> wlan %d is not exist!or there is no bss in wlan %d\n",wlan_id,wlan_id);
					else if(sta_exist == 0)
						vty_out(vty,"<error> there is no station\n");
					dcli_free_ac(ac);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)) {
				bss_exist = 0;
				sta_exist = 0;
				for(i=0; i<ac->num_bss_wireless; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					if(bss->WlanID != wlan_id)
						continue;
					else
						bss_exist = 1;
					if(bss->num_sta >0 )
					{
						sta_exist = 1;
						vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
						vty_out(vty,"==============================================================================\n");
						vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
					for(j=0; j<bss->num_sta; j++){	//qiuchen
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020)){
					//time_t online_time,now_sysrun;//qiuchen
					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					int hour,min,sec;
					
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					memset(onlinetime,0,10);
					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
					
					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s ",sta->ip);
					vty_out(vty,"%-7llu ",sta->txbytes);
					vty_out(vty,"%-6llu ",sta->rxbytes);		
					
					if(!strcmp(sta_method,"Unknown"))
						vty_out(vty,"%-5s ",SecurityType);
					else
						vty_out(vty,"%-5s ",sta_method);
					vty_out(vty,"%-11s ",onlinetime);
					vty_out(vty,"%-s\n",sta->identify);
					}
				}
					
					vty_out(vty,"==============================================================================\n");
					}
				}
				

				if(bss_exist == 0)
					vty_out(vty,"<error> wlan %d is not exist!or there is no bss in wlan %d\n",wlan_id,wlan_id);
				else if(sta_exist == 0)
					vty_out(vty,"<error> there is no station\n");
				dcli_free_ac(ac);
			}	
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}
DEFUN(show_sta_detail_bywtp_cmd_func,
	  show_sta_detial_bywtp_cmd,
	  "show sta detail bywtpid WTPID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "ASD station list information\n"
	  "detail information"	
	  "all sta\n"
	  "all sta\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	 
	struct dcli_ac_info *ac = NULL;
	struct dcli_bss_info *bss = NULL;
	unsigned int ret = 0;
	int i,j,m;
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	char SecurityType[20] = {0};
	char *sta_method = NULL;
	char *username = NULL;
	char onlinetime[10] = {0};
	unsigned int wtp_id = 0;
	char sta_exist = 0;
	char bss_exist = 0;
	ret = parse2_int_ID((char*)argv[0], &wtp_id);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	
	if(wtp_id >= WTP_NUM || wtp_id == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		ac = show_sta_list(dcli_dbus_connection,index, localid,&ret);

		if((ret == 0) && (ac != NULL)) {
			bss_exist = 0;
			sta_exist = 0;
			for(i=0; i<ac->num_bss_wireless; i++){
				struct dcli_sta_info *sta = NULL;
				if(bss == NULL)
					bss = ac->bss_list;
				else 
					bss = bss->next;

				if(bss == NULL)
					break;
				if(bss->WtpID!= wtp_id)
					continue;
				else
					bss_exist = 1;
				if(bss->num_sta >0 )
				{
					sta_exist = 1;
					vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
					for(j=0; j<bss->num_sta; j++){	//qiuchen
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020))
					{
    					//time_t online_time,now_sysrun;//qiuchen
    					//time(&now);
    					//get_sysruntime(&now_sysrun);
    					time_t online_time=sta->sta_online_time_new;
    					int hour,min,sec;
    					
    					hour=online_time/3600;
    					min=(online_time-hour*3600)/60;
    					sec=(online_time-hour*3600)%60;
    					memset(onlinetime,0,10);
    					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
    					
    					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
    					vty_out(vty,"%-15s ",sta->ip);
    					vty_out(vty,"%-7llu ",sta->txbytes);
    					vty_out(vty,"%-6llu ",sta->rxbytes);
    					if(!strcmp(sta_method,"Unknown"))
    						vty_out(vty,"%-5s ",SecurityType);
    					else
    						vty_out(vty,"%-5s ",sta_method);
    					
    					vty_out(vty,"%-11s ",onlinetime);
    					vty_out(vty,"%-s\n",sta->identify);

						vty_out(vty,"ipv6 address:     ");     /* add ipv6 addres for sta */
    					for (m = 0; m < 8; m++)
                    	{   
    						if(m==7)
    						{
                                vty_out(vty,"%x\n",sta->ip6_addr.s6_addr16[m]);
    						}
    					    else
    					    {
                                vty_out(vty,"%x:",sta->ip6_addr.s6_addr16[m]);
    					    }
                    	}
					}
				}
				
				vty_out(vty,"==============================================================================\n");
				}
			}
			
			if(bss_exist == 0)
				vty_out(vty,"<error> wtp %d is not exist!or there  is no bss in wtp %d\n",wtp_id,wtp_id);
			else if(sta_exist == 0)
				vty_out(vty,"<error> there is no station\n");
			dcli_free_ac(ac);
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
				ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"------------------------------------------------------------------------------\n");
				if((ret == 0) && (ac != NULL)) {
					bss_exist = 0;
					sta_exist = 0;
					for(i=0; i<ac->num_bss_wireless; i++){
						struct dcli_sta_info *sta = NULL;
						if(bss == NULL)
							bss = ac->bss_list;
						else 
							bss = bss->next;

						if(bss == NULL)
							break;
						
						if(bss->WtpID!= wtp_id)
							continue;
						else
							bss_exist = 1;
						if(bss->num_sta >0 )
						{
							sta_exist = 1;
							vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
							vty_out(vty,"==============================================================================\n");
							vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
						for(j=0; j<bss->num_sta; j++){	//qiuchen
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020)){
					//time_t online_time,now_sysrun;
					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					int hour,min,sec;
					
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					memset(onlinetime,0,10);
					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
					
					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s ",sta->ip);
					vty_out(vty,"%-7llu ",sta->txbytes);
					vty_out(vty,"%-6llu ",sta->rxbytes);		
					if(!strcmp(sta_method,"Unknown"))
						vty_out(vty,"%-5s ",SecurityType);
					else
						vty_out(vty,"%-5s ",sta_method);
					
					vty_out(vty,"%-11s ",onlinetime);
					vty_out(vty,"%-s\n",sta->identify);
					}
				}
						
						vty_out(vty,"==============================================================================\n");
						}
					}
					
					if(bss_exist == 0)
						vty_out(vty,"<error> wtp %d is not exist!or there is no bss in wtp %d\n",wtp_id,wtp_id);
					else if(sta_exist == 0)
						vty_out(vty,"<error> there is no station\n");
					dcli_free_ac(ac);
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			ac = show_sta_list(dcli_dbus_connection,profile, localid,&ret);

			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			if((ret == 0) && (ac != NULL)) {
				bss_exist = 0;
				sta_exist = 0;
				for(i=0; i<ac->num_bss_wireless; i++){
					struct dcli_sta_info *sta = NULL;
					if(bss == NULL)
						bss = ac->bss_list;
					else 
						bss = bss->next;

					if(bss == NULL)
						break;
					if(bss->WtpID!= wtp_id)
						continue;
					else
						bss_exist = 1;
					if(bss->num_sta >0 )
					{
						sta_exist = 1;
						vty_out(vty,"\nWlanID : %d	WTPID : %d	WIFI : %d	Sta_Num : %d\n",bss->WlanID,bss->WtpID,bss->Radio_L_ID,bss->num_sta);
						vty_out(vty,"==============================================================================\n");
						vty_out(vty,"Sta_MAC		  IP		  OUTFLOW INFLOW Method Onlinetime Username\n");
					for(j=0; j<bss->num_sta; j++){	//qiuchen 
					if(sta == NULL)
						sta = bss->sta_list;
					else 
						sta = sta->next;
					sta_method = eap_type_text(sta->eap_type);
					if(!strcmp(sta_method,"Unknown"))
					{
						memset(SecurityType,0,20);
						CheckSecurityType(SecurityType, sta->security_type);		
						if(!strcmp(SecurityType,"no_need_auth")){
							memset(SecurityType,0,20);
							memcpy(SecurityType,"open",5);
						}	
					}
					if((((!strcmp(SecurityType,"open")) || (!strcmp(SecurityType,"shared"))) && (sta->sta_flags & 0x00000002)) || (sta->sta_flags & 0x00000020)){
					//time_t online_time,now_sysrun;
					//time(&now);
					//get_sysruntime(&now_sysrun);
					time_t online_time=sta->sta_online_time_new;
					int hour,min,sec;
					
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;
					memset(onlinetime,0,10);
					sprintf(onlinetime,"%2d:%2d:%2d",hour,min,sec);
					
					vty_out(vty,MACSTR" ",MAC2STR(sta->addr));
					vty_out(vty,"%-15s ",sta->ip);
					vty_out(vty,"%-7llu ",sta->txbytes);
					vty_out(vty,"%-6llu ",sta->rxbytes);		
					if(!strcmp(sta_method,"Unknown"))
						vty_out(vty,"%-5s ",SecurityType);
					else
						vty_out(vty,"%-5s ",sta_method);
					
					vty_out(vty,"%-11s ",onlinetime);
					vty_out(vty,"%-s\n",sta->identify);
					}
				}
					
					vty_out(vty,"==============================================================================\n");
					}
				}
				

				if(bss_exist == 0)
					vty_out(vty,"<error> wtp %d is not exist!or there is no bss in wtp %d\n",wtp_id,wtp_id);
				else if(sta_exist == 0)
					vty_out(vty,"<error> there is no station\n");
				dcli_free_ac(ac);
			}	
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;
}
//weichao add
DEFUN(set_asd_sta_idle_time_switch_cmd_func,
		set_asd_sta_idle_time_switch_cmd,
		"set asd sta idle time switch (enable|disable)",
		"set \n"
		"asd\n"
		"sta info\n"
		"sta idle info"
		"idle time\n"
		"timeout switch\n"
		"default enable"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type = 1;   
	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
		
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME_SWITCH);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd sta idel time swich %s successfully!\n",type?"enable":"disable"); 
	else
		vty_out(vty,"<error> %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

//weichao add
DEFUN(set_asd_sta_idle_time_cmd_func,
		set_asd_sta_idle_time_cmd,
		"set asd sta idle time interval TIME",
		"set \n"
		"asd\n"
		"sta info\n"
		"sta idle info"
		"idle time\n"
		"timeout interval\n"
		"unit hour(default 8)"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int  time = 8;   
	ret = parse_int_ID((char*)argv[0], &time);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown format,please input number\n");
		return CMD_SUCCESS;
	}
	if(time<= 0 ||time > 32767)
	{
		vty_out(vty,"the  time you input is too large!\n");
		return CMD_SUCCESS;	
	}
		
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&time,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd sta idel time %d hour successfully!\n",time); 
	else
		vty_out(vty,"<error> %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
//for sta ipset swich in asd
DEFUN(set_asd_ipset_switch_cmd_func,
		set_asd_ipset_switch_cmd,
		"set asd  ipset switch (enable|disable)",
		"set \n"
		"asd\n"
		"sta info\n"
		"sta ipset will not notice portal"
		"\n"
		"\n"
		"default disable"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type = 0;   
	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
		
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_IPSET_SWITCH);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd ipset switch %s successfully!\n",type?"enable":"disable"); 
	else
		vty_out(vty,"<error>set asd ipset switch error! %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}


//add for bak sta update
DEFUN(set_asd_bak_sta_update_value_func,
		set_asd_bak_sta_update_value_cmd,
		"set asd bak sta update interval INTERVAL",
		"set \n"
		"asd\n"
		"back sta \n"
		"update sta \n"
		"update sta req interval\n"
		"unit minute(default 360)"
		)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int  time = 360;   
	ret = parse_int_ID((char*)argv[0], &time);
	if(ret != ASD_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown format,please input number\n");
		return CMD_SUCCESS;
	}
	if(time<= 0 ||time > 32767)
	{
		vty_out(vty,"the  time you input is too large!\n");
		return CMD_SUCCESS;	
	}
		
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_BAK_STA_UPDATE_TIME);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&time,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd bak sta update interval %d minute successfully!\n",time); 
	else
		vty_out(vty,"<error> %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}
DEFUN(show_roaming_sta_cmd_func,
		show_roaming_sta_cmd,
		"show roaming sta list by (wlan|class|wtp) [remote] [local] [PARAMETER]",
		"show\n"
		"roaming sta list\n"
		"roaming sta list\n"
		""
)
{	
	struct dcli_r_sta_info *r_sta_group = NULL;
	struct r_sta_info *r_sta_list = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	char type = 0;
	unsigned int Total_roam_sta_num = 0;
	if(strncmp(argv[0],"wlan",strlen(argv[0])) == 0)
		type = 1;
	else if(strncmp(argv[0],"class",strlen(argv[0])) == 0)
		type = 2;
	else if(strncmp(argv[0],"wtp",strlen(argv[0])) == 0)
		type = 3;
	
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		r_sta_group = show_roaming_sta_list(dcli_dbus_connection,index,localid,type,&ret,&Total_roam_sta_num,WTP_NUM+1);

		vty_out(vty,"==============================================================================\n");
		if((ret == 0) && (r_sta_group != NULL)) {
			vty_out(vty,"Total_roam_sta_num:	%d\n",Total_roam_sta_num);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			if(type == 1){
				for(i=0;i<WLAN_NUM;i++){
					if(r_sta_group[i].roaming_sta_num != 0){
						vty_out(vty,"WlanID:	%d\n",i );
						vty_out(vty,"Roaming_sta_num:	%d\n",r_sta_group[i].roaming_sta_num);
						r_sta_list = r_sta_group[i].r_sta_list;
						if(r_sta_list != NULL){
							for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
								vty_out(vty,"------------------------------------------------------------------------------\n");
								vty_out(vty,"STA_MAC			Roam_type	PreAPID		CurAPID\n");
								vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
								vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
								vty_out(vty,"%16d",r_sta_list[j].preAPID);
								vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
							}
						}
					}
				}
				dcli_free_r_sta_group(r_sta_group,WLAN_NUM);
			}
			else if(type == 2){
				for(i=0;i<3;i++){
					if(r_sta_group[i].roaming_sta_num != 0){
						vty_out(vty,"Roam_type:		%s\n",(i == 1)?"L2":"L3");
						vty_out(vty,"Roaming_sta_num:	%d\n",r_sta_group[i].roaming_sta_num);
						r_sta_list = r_sta_group[i].r_sta_list;
						if(r_sta_list != NULL){
							for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
								vty_out(vty,"------------------------------------------------------------------------------\n");
								vty_out(vty,"STA_MAC			Roam_type	PreAPID		CurAPID\n");
								vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
								vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
								vty_out(vty,"%16d",r_sta_list[j].preAPID);
								vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
							}
						}
					}
				}
				
				dcli_free_r_sta_group(r_sta_group,3);
			}
			else if(type == 3){
				for(i=0;i<WTP_NUM+1;i++){
					if(r_sta_group[i].roaming_sta_num != 0){
						vty_out(vty,"WTPID:	%d\n",i );
						vty_out(vty,"Roaming_sta_num:	%d\n",r_sta_group[i].roaming_sta_num);
						r_sta_list = r_sta_group[i].r_sta_list;
						if(r_sta_list != NULL){
							for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
								vty_out(vty,"------------------------------------------------------------------------------\n");
								vty_out(vty,"STA_MAC			Roam_type	PreAPID		CurAPID\n");
								vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
								vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
								vty_out(vty,"%16d",r_sta_list[j].preAPID);
								vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
							}
						}
					}
				}
				
				dcli_free_r_sta_group(r_sta_group,WTP_NUM);
			}
 		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
		vty_out(vty,"==============================================================================\n");
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

	 hansi_parameter:
		 r_sta_group = show_roaming_sta_list(dcli_dbus_connection,profile,localid,type,&ret,&Total_roam_sta_num,WTP_NUM);
		 
		 vty_out(vty,"==============================================================================\n");
		 if((ret == 0) && (r_sta_group != NULL)) {
			 vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			 vty_out(vty,"------------------------------------------------------------------------------\n");
			 vty_out(vty,"Total_roam_sta_num:	 %d\n",Total_roam_sta_num);
			 vty_out(vty,"------------------------------------------------------------------------------\n");
			 if(type == 1){
				 for(i=0;i<WLAN_NUM;i++){
					 if(r_sta_group[i].roaming_sta_num != 0){
						 vty_out(vty,"WlanID:	 %d\n",i );
						 vty_out(vty,"Roaming_sta_num:	 %d\n",r_sta_group[i].roaming_sta_num);
						 r_sta_list = r_sta_group[i].r_sta_list;
						 if(r_sta_list != NULL){
							 for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
								 vty_out(vty,"------------------------------------------------------------------------------\n");
								 vty_out(vty,"STA_MAC			 Roam_type	 PreAPID	 CurAPID\n");
								 vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
								 vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
								 vty_out(vty,"%16d",r_sta_list[j].preAPID);
								 vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
							 }
						 }
					 }
				 }
				 dcli_free_r_sta_group(r_sta_group,WLAN_NUM);
			 }
			 else if(type == 2){
				 for(i=0;i<3;i++){
					 if(r_sta_group[i].roaming_sta_num != 0){
						 vty_out(vty,"Roam_type:	 %s\n",(i == 1)?"L2":"L3");
						 vty_out(vty,"Roaming_sta_num:	 %d\n",r_sta_group[i].roaming_sta_num);
						 r_sta_list = r_sta_group[i].r_sta_list;
						 if(r_sta_list != NULL){
							 for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
								 vty_out(vty,"------------------------------------------------------------------------------\n");
								 vty_out(vty,"STA_MAC			 Roam_type	 PreAPID	 CurAPID\n");
								 vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
								 vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
								 vty_out(vty,"%16d",r_sta_list[j].preAPID);
								 vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
							 }
						 }
					 }
				 }
				 
				 dcli_free_r_sta_group(r_sta_group,3);
			 }
			 else if(type == 3){
				 for(i=0;i<WTP_NUM;i++){
					 if(r_sta_group[i].roaming_sta_num != 0){
						 vty_out(vty,"WTPID:	 %d\n",i );
						 vty_out(vty,"Roaming_sta_num:	 %d\n",r_sta_group[i].roaming_sta_num);
						 r_sta_list = r_sta_group[i].r_sta_list;
						 if(r_sta_list != NULL){
							 for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
								 vty_out(vty,"------------------------------------------------------------------------------\n");
								 vty_out(vty,"STA_MAC			 Roam_type	 PreAPID	 CurAPID\n");
								 vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
								 vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
								 vty_out(vty,"%16d",r_sta_list[j].preAPID);
								 vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
							 }
						 }
					 }
				 }
				 
				 dcli_free_r_sta_group(r_sta_group,WTP_NUM);
			 }
		 }	 
		 else if (ret == ASD_DBUS_ERROR)
			 cli_syslog_info("<error> failed get reply.\n");
		 else
			 vty_out(vty,"<error> ret = %d\n",ret);
		 vty_out(vty,"==============================================================================\n");
		 if(argc == 3){
			 return CMD_SUCCESS;
		 }
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	 local_hansi_parameter:
			r_sta_group = NULL;
			r_sta_group = show_roaming_sta_list(dcli_dbus_connection,profile, localid,&ret,&Total_roam_sta_num,WTP_NUM);

			vty_out(vty,"==============================================================================\n");
			if((ret == 0) && (r_sta_group != NULL)) {
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"------------------------------------------------------------------------------\n");
				vty_out(vty,"Total_roam_sta_num:	%d\n",Total_roam_sta_num);
				vty_out(vty,"------------------------------------------------------------------------------\n");
				if(type == 1){
					for(i=0;i<WLAN_NUM;i++){
						if(r_sta_group[i].roaming_sta_num != 0){
							vty_out(vty,"WlanID:	%d\n",i );
							vty_out(vty,"Roaming_sta_num:	%d\n",r_sta_group[i].roaming_sta_num);
							r_sta_list = r_sta_group[i].r_sta_list;
							if(r_sta_list != NULL){
								for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
									vty_out(vty,"------------------------------------------------------------------------------\n");
									vty_out(vty,"STA_MAC			Roam_type	PreAPID 	CurAPID\n");
									vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
									vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
									vty_out(vty,"%16d",r_sta_list[j].preAPID);
									vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
								}
							}
						}
					}
					dcli_free_r_sta_group(r_sta_group,WLAN_NUM);
				}
				else if(type == 2){
					for(i=0;i<3;i++){
						if(r_sta_group[i].roaming_sta_num != 0){
							vty_out(vty,"Roam_type: 	%s\n",(i == 1)?"L2":"L3");
							vty_out(vty,"Roaming_sta_num:	%d\n",r_sta_group[i].roaming_sta_num);
							r_sta_list = r_sta_group[i].r_sta_list;
							if(r_sta_list != NULL){
								for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
									vty_out(vty,"------------------------------------------------------------------------------\n");
									vty_out(vty,"STA_MAC			Roam_type	PreAPID 	CurAPID\n");
									vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
									vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
									vty_out(vty,"%16d",r_sta_list[j].preAPID);
									vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
								}
							}
						}
					}
					
					dcli_free_r_sta_group(r_sta_group,3);
				}
				else if(type == 3){
					for(i=0;i<WTP_NUM;i++){
						if(r_sta_group[i].roaming_sta_num != 0){
							vty_out(vty,"WTPID:	%d\n",i );
							vty_out(vty,"Roaming_sta_num:	%d\n",r_sta_group[i].roaming_sta_num);
							r_sta_list = r_sta_group[i].r_sta_list;
							if(r_sta_list != NULL){
								for(j=0;j<r_sta_group[i].roaming_sta_num;j++){
									vty_out(vty,"------------------------------------------------------------------------------\n");
									vty_out(vty,"STA_MAC			Roam_type	PreAPID 	CurAPID\n");
									vty_out(vty,MACSTR,MAC2STR(r_sta_list[j].STA_MAC));
									vty_out(vty,"%10s",(r_sta_list[j].r_type == 1)?"L2":"L3");
									vty_out(vty,"%16d",r_sta_list[j].preAPID);
									vty_out(vty,"%16d\n",r_sta_list[j].curAPID);
								}
							}
						}
					}
					
					dcli_free_r_sta_group(r_sta_group,WTP_NUM);
				}
			}	
			else if (ret == ASD_DBUS_ERROR)
				cli_syslog_info("<error> failed get reply.\n");
			else
				vty_out(vty,"<error> ret = %d\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
		}
	}
	}
	
	return CMD_SUCCESS;
}
DEFUN(show_bss_summary_cmd_func,
		show_bss_summary_cmd,
		"show bss summary by (wlan|wtp|radio) [ID] [remote] [local] [PARAMETER]",
		"show \n"
		"bss info\n"
		"seperated by wlan|wtp|radio\n"
		"wlanID/wtpID/radioGID"
		""
	)
{
	struct dcli_bss_summary_info *bss_summary = NULL;
	struct bss_summary_info *bss_list = NULL;
	unsigned int ret = 0;
	int i,j;int localid = 1;int slot_id = HostSlotId;int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	char type = 0;
	int id = 0;
	int Total_bss_num = 0;
	int num = 0;
	if(strncmp(argv[0],"wlan",strlen(argv[0])) == 0){
		type = 1;
		num = WLAN_NUM;
	}
	else if(strncmp(argv[0],"wtp",strlen(argv[0])) == 0){
		type = 2;
		num = WTP_NUM+1;
	}
	else if(strncmp(argv[0],"radio",strlen(argv[0])) == 0){
		type = 3;
		num = 1;
	}
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if(argc == 5){
		vty_out(vty,"<error>input parameter should be ID 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 1){
		if(type == 3){
			vty_out(vty,"show bss summary by %s should take the ID number!\n",argv[0]);
			return CMD_SUCCESS;
		}
	}
	if(argc == 2){
		if(strncmp(argv[1],"remote",strlen(argv[1])) == 0 || strncmp(argv[1],"local",strlen(argv[1])) == 0 || !parse_slot_hansi_id((char*)argv[1],&slot_id,&profile)){
			vty_out(vty,"<error>input parameter should be 'wtp ID' or 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		ret = parse_int_ID(argv[1],&id);
		if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error> illegal input!\n");
			return CMD_SUCCESS;		
		}
		if(type == 1 && id > 128){
			vty_out(vty,"%s ID exceeds the the maximum value of the parameter type\n");
			return CMD_SUCCESS;
		}
		else if (type == 2 && id > WTP_NUM){
			vty_out(vty,"%s ID exceeds the the maximum value of the parameter type\n");
			return CMD_SUCCESS;
		}
	}
	if(argc == 3){
		if(type == 3){
			vty_out(vty,"show by radio must take the radio global id!\n");
			return CMD_SUCCESS;
		}
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		if(strncmp(argv[2],"remote",strlen(argv[2])) == 0 || strncmp(argv[2],"local",strlen(argv[2])) == 0){
			vty_out(vty,"<error>input parameter should be 'wtp ID' or 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 

	}
	if(argc == 4){
		if (!strcmp(argv[2],"remote")){
			localid = 0;
		}else if(!strcmp(argv[2],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[2],"remote"))&&(!strcmp(argv[3],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		ret = parse_int_ID(argv[1],&id);
		if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error> illegal input!\n");
			return CMD_SUCCESS;		
		}
		if(type == 1 && id > 128){
			vty_out(vty,"%s ID exceeds the the maximum value of the parameter type\n");
			return CMD_SUCCESS;
		}
		else if (type == 2 && id > WTP_NUM){
			vty_out(vty,"%s ID exceeds the the maximum value of the parameter type\n");
			return CMD_SUCCESS;
		}
		ret = parse_slot_hansi_id((char*)argv[3],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[3], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		bss_summary = show_bss_summary(dcli_dbus_connection,index,localid,type,&ret,&Total_bss_num,id,num);

		vty_out(vty,"==============================================================================\n");
		if((ret == 0) && (bss_summary != NULL)) {
			vty_out(vty,"Total_bss_num:	%d\n",Total_bss_num);
			vty_out(vty,"------------------------------------------------------------------------------\n");
			if(type == 1){
				for(i=0;i<WLAN_NUM;i++){
					if(bss_summary[i].local_bss_num != 0){
						vty_out(vty,"WlanID:	%d\n",bss_summary[i].ID );
						vty_out(vty,"sub_bss_num:	%d\n",bss_summary[i].local_bss_num);
						bss_list = bss_summary[i].bss_list;
						vty_out(vty,"------------------------------------------------------------------------------\n");
						vty_out(vty,"BSSID			WlanID		G_R_ID		sta_num 	BssIndex\n");
						if(bss_list != NULL){
							for(j=0;j<bss_summary[i].local_bss_num;j++){
								vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
								vty_out(vty,"%10d",bss_list[j].WLANID);
								vty_out(vty,"%16d",bss_list[j].RGID);
								vty_out(vty,"%16d",bss_list[j].sta_num);
								vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
							}
						}
						vty_out(vty,"------------------------------------------------------------------------------\n");
					}
				}
				dcli_free_bss_summary_list(bss_summary,num);
			}
			else if(type == 2){
				for(i=0;i<WTP_NUM+1;i++){
					if(bss_summary[i].local_bss_num != 0){
						vty_out(vty,"WtpIndex:	%d\n",bss_summary[i].ID  );
						vty_out(vty,"sub_bss_num:	%d\n",bss_summary[i].local_bss_num);
						bss_list = bss_summary[i].bss_list;
						vty_out(vty,"------------------------------------------------------------------------------\n");
						vty_out(vty,"BSSID			WlanID		G_R_ID		sta_num 	BssIndex\n");
						if(bss_list != NULL){
							for(j=0;j<bss_summary[i].local_bss_num;j++){
								vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
								vty_out(vty,"%10d",bss_list[j].WLANID);
								vty_out(vty,"%16d",bss_list[j].RGID);
								vty_out(vty,"%16d",bss_list[j].sta_num);
								vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
							}
						}
						vty_out(vty,"------------------------------------------------------------------------------\n");
					}
				}
				
				dcli_free_bss_summary_list(bss_summary,num);
			}
			else if(type == 3){
				if(bss_summary[0].local_bss_num != 0){
					vty_out(vty,"G_R_ID:	%d\n",id );
					vty_out(vty,"sub_bss_num:	%d\n",bss_summary[0].local_bss_num);
					bss_list = bss_summary[0].bss_list;
					vty_out(vty,"------------------------------------------------------------------------------\n");
					vty_out(vty,"BSSID			WlanID		G_R_ID		sta_num 	BssIndex\n");
					if(bss_list != NULL){
						for(j=0;j<bss_summary[0].local_bss_num;j++){
							vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
							vty_out(vty,"%10d",bss_list[j].WLANID);
							vty_out(vty,"%16d",bss_list[j].RGID);
							vty_out(vty,"%16d",bss_list[j].sta_num);
							vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
						}
					}
					vty_out(vty,"------------------------------------------------------------------------------\n");
				}
				dcli_free_bss_summary_list(bss_summary,num);
			}
		}	
		else if (ret == ASD_DBUS_ERROR)
			cli_syslog_info("<error> failed get reply.\n");
		else
			vty_out(vty,"<error> ret = %d\n",ret);
		vty_out(vty,"==============================================================================\n");
	}
	
	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	hansi_parameter:
		
				bss_summary = show_bss_summary(dcli_dbus_connection,profile,localid,type,&ret,&Total_bss_num,id,num);
				vty_out(vty,"==============================================================================\n");
				if((ret == 0) && (bss_summary != NULL)) {
					vty_out(vty,"Total_bss_num: %d\n",Total_bss_num);
					vty_out(vty,"------------------------------------------------------------------------------\n");
					if(type == 1){
						for(i=0;i<WLAN_NUM;i++){
							if(bss_summary[i].local_bss_num != 0){
								vty_out(vty,"WlanID:	%d\n",i );
								vty_out(vty,"sub_bss_num:	%d\n",bss_summary[i].local_bss_num);
								bss_list = bss_summary[i].bss_list;
								vty_out(vty,"------------------------------------------------------------------------------\n");
								vty_out(vty,"BSSID			WlanID		G_R_ID		sta_num 	BssIndex\n");
								if(bss_list != NULL){
									for(j=0;j<bss_summary[i].local_bss_num;j++){
										vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
										vty_out(vty,"%10d",bss_list[j].WLANID);
										vty_out(vty,"%16d",bss_list[j].RGID);
										vty_out(vty,"%16d",bss_list[j].sta_num);
										vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
									}
								}
								vty_out(vty,"------------------------------------------------------------------------------\n");
							}
						}
						dcli_free_bss_summary_list(bss_summary,num);
					}
					else if(type == 2){
						for(i=0;i<WTP_NUM+1;i++){
							if(bss_summary[i].local_bss_num != 0){
								vty_out(vty,"WtpIndex:	%d\n",i );
								vty_out(vty,"sub_bss_num:	%d\n",bss_summary[i].local_bss_num);
								bss_list = bss_summary[i].bss_list;
								vty_out(vty,"------------------------------------------------------------------------------\n");
								vty_out(vty,"BSSID			WlanID		G_R_ID		sta_num 	BssIndex\n");
								if(bss_list != NULL){
									for(j=0;j<bss_summary[i].local_bss_num;j++){
										vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
										vty_out(vty,"%10d",bss_list[j].WLANID);
										vty_out(vty,"%16d",bss_list[j].RGID);
										vty_out(vty,"%16d",bss_list[j].sta_num);
										vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
									}
								}
								vty_out(vty,"------------------------------------------------------------------------------\n");
							}
						}
						
						dcli_free_bss_summary_list(bss_summary,num);
					}
					else if(type == 3){
						if(bss_summary[0].local_bss_num != 0){
							vty_out(vty,"G_R_ID:	%d\n",i );
							vty_out(vty,"sub_bss_num:	%d\n",bss_summary[0].local_bss_num);
							bss_list = bss_summary[0].bss_list;
							vty_out(vty,"------------------------------------------------------------------------------\n");
							vty_out(vty,"BSSID			WlanID		G_R_ID		sta_num 	BssIndex\n");
							if(bss_list != NULL){
								for(j=0;j<bss_summary[0].local_bss_num;j++){
									vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
									vty_out(vty,"%10d",bss_list[j].WLANID);
									vty_out(vty,"%16d",bss_list[j].RGID);
									vty_out(vty,"%16d",bss_list[j].sta_num);
									vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
								}
							}
							vty_out(vty,"------------------------------------------------------------------------------\n");
						}
						dcli_free_bss_summary_list(bss_summary,num);
					}
				}	
				else if (ret == ASD_DBUS_ERROR)
					cli_syslog_info("<error> failed get reply.\n");
				else
					vty_out(vty,"<error> ret = %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 4){
					return CMD_SUCCESS;
				}
			}

		}
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	local_hansi_parameter:
			   bss_summary = NULL;
			   bss_summary = show_bss_summary(dcli_dbus_connection,profile, localid,&ret,&Total_bss_num,id,num);
			   vty_out(vty,"==============================================================================\n");
			   if((ret == 0) && (bss_summary != NULL)) {
				   vty_out(vty,"Total_bss_num: %d\n",Total_bss_num);
				   vty_out(vty,"------------------------------------------------------------------------------\n");
				   if(type == 1){
					   for(i=0;i<WLAN_NUM;i++){
						   if(bss_summary[i].local_bss_num != 0){
							   vty_out(vty,"WlanID:    %d\n",i );
							   vty_out(vty,"sub_bss_num:   %d\n",bss_summary[i].local_bss_num);
							   bss_list = bss_summary[i].bss_list;
							   vty_out(vty,"------------------------------------------------------------------------------\n");
							   vty_out(vty,"BSSID		   WlanID	   G_R_ID	   sta_num	   BssIndex\n");
							   if(bss_list != NULL){
								   for(j=0;j<bss_summary[i].local_bss_num;j++){
									   vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
									   vty_out(vty,"%10d",bss_list[j].WLANID);
									   vty_out(vty,"%16d",bss_list[j].RGID);
									   vty_out(vty,"%16d",bss_list[j].sta_num);
									   vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
								   }
							   }
							   vty_out(vty,"------------------------------------------------------------------------------\n");
						   }
					   }
					   dcli_free_bss_summary_list(bss_summary,num);
				   }
				   else if(type == 2){
					   for(i=0;i<WTP_NUM+1;i++){
						   if(bss_summary[i].local_bss_num != 0){
							   vty_out(vty,"WtpIndex:  %d\n",i );
							   vty_out(vty,"sub_bss_num:   %d\n",bss_summary[i].local_bss_num);
							   bss_list = bss_summary[i].bss_list;
							   vty_out(vty,"------------------------------------------------------------------------------\n");
							   vty_out(vty,"BSSID		   WlanID	   G_R_ID	   sta_num	   BssIndex\n");
							   if(bss_list != NULL){
								   for(j=0;j<bss_summary[i].local_bss_num;j++){
									   vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
									   vty_out(vty,"%10d",bss_list[j].WLANID);
									   vty_out(vty,"%16d",bss_list[j].RGID);
									   vty_out(vty,"%16d",bss_list[j].sta_num);
									   vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
								   }
							   }
							   vty_out(vty,"------------------------------------------------------------------------------\n");
						   }
					   }
					   
					   dcli_free_bss_summary_list(bss_summary,num);
				   }
				   else if(type == 3){
					   if(bss_summary[0].local_bss_num != 0){
						   vty_out(vty,"G_R_ID:    %d\n",i );
						   vty_out(vty,"sub_bss_num:   %d\n",bss_summary[0].local_bss_num);
						   bss_list = bss_summary[0].bss_list;
						   vty_out(vty,"------------------------------------------------------------------------------\n");
						   vty_out(vty,"BSSID		   WlanID	   G_R_ID	   sta_num	   BssIndex\n");
						   if(bss_list != NULL){
							   for(j=0;j<bss_summary[0].local_bss_num;j++){
								   vty_out(vty,MACSTR,MAC2STR(bss_list[j].BSSID));
								   vty_out(vty,"%10d",bss_list[j].WLANID);
								   vty_out(vty,"%16d",bss_list[j].RGID);
								   vty_out(vty,"%16d",bss_list[j].sta_num);
								   vty_out(vty,"%16d\n",bss_list[j].BSSINDEX);
							   }
						   }
						   vty_out(vty,"------------------------------------------------------------------------------\n");
					   }
					   dcli_free_bss_summary_list(bss_summary,num);
				   }
			   }   
			   else if (ret == ASD_DBUS_ERROR)
				   cli_syslog_info("<error> failed get reply.\n");
			   else
				   vty_out(vty,"<error> ret = %d\n",ret);
			   vty_out(vty,"==============================================================================\n");
			   if(argc == 4){
				   return CMD_SUCCESS;
			   }
			}
		}
	}
	return CMD_SUCCESS;
}
DEFUN(show_bss_bssindex_cmd_func,
		show_bss_bssindex_cmd,
		"show bss BSSINDEX [detail]",
		"show \n"
		"bss info\n"
	)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret = 0;
	unsigned int Bssindex = 0;
	unsigned char detail = 0;
	char *br_ifname = NULL;
	struct dcli_bss_indexinfo bssinfo;
	memset(&bssinfo,0,sizeof(struct dcli_bss_indexinfo));
	ret = parse_int_ID(argv[0],&Bssindex);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> illegal input!\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if(!strcmp(argv[1],"detail"))
			detail = 1;
		else{
			vty_out(vty,"<error> input shoule be show bss BSSINDEX detail!\n");
			return CMD_SUCCESS;
		}
	}

	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_BSS_METHOD_SHOW_BSS_BSSINDEX);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&Bssindex,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.BSSID[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.BSSID[1]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.BSSID[2]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.BSSID[3]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.BSSID[4]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.BSSID[5]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.R_L_ID);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.R_G_ID);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.WlanID);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.SID);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.Vlanid);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.num_sta);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.hotspotid);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&br_ifname);
		memcpy(bssinfo.br_ifname,br_ifname,strlen(br_ifname));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.traffic_limit);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bssinfo.send_traffic_limit);
	}
	if(ret == ASD_BSS_NOT_EXIST)
		vty_out(vty,"bss %s is not exist!",argv[0]);
	else if(ret == 0){
		vty_out(vty,"==============================================================================\n");
			vty_out(vty,"BSSID:	"MACSTR"\n",MAC2STR(bssinfo.BSSID));
			vty_out(vty,"Radio Global ID:	%d\n",bssinfo.R_G_ID);
			vty_out(vty,"Wlan ID:		%d\n",bssinfo.WlanID);
			vty_out(vty,"Vlan ID:		%d\n",bssinfo.Vlanid);
			vty_out(vty,"Security ID:		%d\n",bssinfo.SID);
			if(detail){
				vty_out(vty,"Radio Local ID:		%d\n",bssinfo.R_L_ID);
				vty_out(vty,"Accessed sta num:	%d\n",bssinfo.num_sta);
				vty_out(vty,"Hotspot ID:		%d\n",bssinfo.hotspotid);
				vty_out(vty,"Br_ifname:		%s\n",bssinfo.br_ifname);
				vty_out(vty,"Uplink traffic limit:	%d\n",bssinfo.traffic_limit);
				vty_out(vty,"Downlink traffic limit:	%d\n",bssinfo.send_traffic_limit);
			}
			vty_out(vty,"==============================================================================\n");
	}
	else
		vty_out(vty,"ret = %d\n",ret);
	
	return CMD_SUCCESS; 
	
}

DEFUN(set_asd_1x_radius_format_cmd_func,
	    set_asd_1x_radius_format_cmd,
		"set asd radius format to (default|indonesia)",
		"set 802.1x radius format for different use\n"
		"default 802.1x radius format\n"
		"Indonesia 802.1x radius format\n"
	    )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type = 0;
	
	if (!strcmp(argv[0],"default"))
	{
		type = 0;		/* RADIUS_FORMAT_DEFAULT */
	}
	else if (!strcmp(argv[0],"indonesia"))
	{
		type = 1;		/* RADIUS_FORMAT_INDONESIA */
	}
	else
	{
		return CMD_SUCCESS;
	}
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_RADIUS_FORMAT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret==ASD_DBUS_SUCCESS)
		vty_out(vty,"set asd sta 802.1x radius format successful!\n"); 
	else
		vty_out(vty,"set failed %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS; 
}

/*yjl copy from aw3.1.2 for local forwarding.2014-2-28***************************************************/
DEFUN(delete_sta_vir_dhcp_pool_ip_range_cmd_func,
	delete_sta_vir_dhcp_pool_ip_range_cmd,
	"delete sta vir-dhcp range A.B.C.D A.B.C.D base wlan ID",
	"delete dhcp ip range\n"
	"ASD station information\n"
	"Vir-DHCP\n"
	"sta vir-dhcp ip range\n"
	"Low ip A.B.C.D\n"
	"High ip A.B.C.D\n"
	"WLAN ID"
)
{
	unsigned int ipAddrl = 0, ipAddrh = 0, ip_Nums = 0;
	unsigned int ret = 0;
	wlan_t wlanid = 0;
	unsigned int add_flag = 0;
	int index = 0;
	int localid = 1;int slot_id = HostSlotId;
	DBusConnection *dcli_dbus_connection = NULL;
	
	if (dcli_check_ipaddr(argv[0], &ipAddrl))
	{
		vty_out(vty, "%% invalid ip address : %s\n", argv[0]);
		return CMD_WARNING;
	}

	if (dcli_check_ipaddr(argv[1], &ipAddrh))
	{
		vty_out(vty, "%% invalid ip address : %s\n", argv[1]);
		return CMD_WARNING;
	}

	ret = parse_char_ID((char*)argv[2], &wlanid);
	if(ret != WID_DBUS_SUCCESS)
	{
		if(ret == WID_ILLEGAL_INPUT)
		{
			vty_out(vty,"%% illegal input:Input exceeds the maximum value of the parameter type \n");
		}
		else
		{
			vty_out(vty,"%% unknown id format\n");
		}
		return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0)
	{
		vty_out(vty,"%% wlan id should be 1 to %d\n", WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if (ipAddrl > ipAddrh) 
	{	
		vty_out(vty, "%% illegal ip range\n");
		return CMD_WARNING;
	}		

	ip_Nums = ipAddrh - ipAddrl;
	if ((ip_Nums > 0xffff))
	{
		vty_out(vty, "set ip range fail \n");
		return CMD_WARNING;
	}

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
    vty_out(vty,"localid: %d,slot_id: %d ,**********yjl add for test ****11111111111111*******\n",localid, slot_id);
	ret = dcli_set_sta_virdchp_range(dcli_dbus_connection, index, localid, ipAddrl, ipAddrh, add_flag, wlanid);
	if (ASD_DBUS_SUCCESS != ret)
	{
		vty_out(vty,"%s\n", dcli_asd_opcode2string(ret));
		//vty_out(vty,"%d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_sta_vir_dhcp_pool_ip_range_cmd_func,
	set_sta_vir_dhcp_pool_ip_range_cmd,
	"set sta vir-dhcp range A.B.C.D A.B.C.D base wlan ID",
	"set dhcp ip range\n"
	"sta vir-dhcp ip range\n"
	"Range ip\n"
	"Low ip A.B.C.D\n"
	"High ip A.B.C.D\n"
	"WLAN ID info"
)
{
	unsigned int ipAddrl = 0, ipAddrh = 0, ip_Nums = 0;
	unsigned int ret = 0;
	wlan_t wlanid = 0;
	unsigned int add_flag = 1;

	if (dcli_check_ipaddr(argv[0], &ipAddrl))
	{
		vty_out(vty, "%% invalid ip address : %s\n", argv[0]);
		return CMD_WARNING;
	}

	if (dcli_check_ipaddr(argv[1], &ipAddrh))
	{
		vty_out(vty, "%% invalid ip address : %s\n", argv[1]);
		return CMD_WARNING;
	}

	ret = parse_char_ID((char*)argv[2], &wlanid);
	if(ret != WID_DBUS_SUCCESS)
	{
		if(ret == WID_ILLEGAL_INPUT)
		{
			vty_out(vty,"%% illegal input:Input exceeds the maximum value of the parameter type \n");
		}
		else
		{
			vty_out(vty,"%% unknown id format\n");
		}
		return CMD_SUCCESS;
	}	
	if(wlanid >= WLAN_NUM || wlanid == 0)
	{
		vty_out(vty,"%% wlan id should be 1 to %d\n", WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	if (ipAddrl > ipAddrh) 
	{	
		vty_out(vty, "%% illegal ip range\n");
		return CMD_WARNING;
	}		

	ip_Nums = ipAddrh - ipAddrl;
	if ((ip_Nums > 0xffff))
	{
		vty_out(vty, "set ip range fail \n");
		return CMD_WARNING;
	}	

	int index = 0;
	int localid = 1;int slot_id = HostSlotId;
	DBusConnection *dcli_dbus_connection = NULL;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
    vty_out(vty,"localid: %d,slot_id: %d ,**********yjl add for test ****11111111111111*******\n",localid, slot_id);
	ret = dcli_set_sta_virdchp_range(dcli_dbus_connection, index, localid, ipAddrl, ipAddrh, add_flag, wlanid);
	if (ASD_DBUS_SUCCESS != ret)
	{
		vty_out(vty,"%s\n", dcli_asd_opcode2string(ret));
	}

	return CMD_SUCCESS;
}

DEFUN(set_sta_vir_dhcp_state_cmd_func,
	set_sta_vir_dhcp_pool_state_cmd,
	"set sta vir-dhcp (enable|disable) base wlan ID",
	"set dhcp service\n"
	"sta vir-dhcp service enable or disable\n"
	"WLAN ID info"
)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	char wlan_id = 0;
	int is_enable = 0;
	if (!strcmp(argv[0],"enable")||(tolower(argv[0][0]) == 'e'))
	{
		is_enable = 1;	
	
	}		
	else if (!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd'))
	{
		is_enable = 0;
	}

	ret = parse_char_ID((char*)argv[1], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int localid = 1;int slot_id = HostSlotId;
	
	DBusConnection *dcli_dbus_connection = NULL;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	vty_out(vty,"localid: %d,slot_id: %d ,**********yjl add for test ****11111111111111*******\n",localid, slot_id);

	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_VIR_DHCP_STATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&is_enable,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
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
	if(ret==0)
		vty_out(vty,"set sta vir-dhcp successful!\n"); 
	else if(ret == ASD_DHCP_POOL_NOT_EXIST){
		vty_out(vty,"sta vir-dhcp poll not exsit!\n"); 
	}
	else if(ret == ASD_WLAN_BE_ENABLE){
		vty_out(vty,"Please disable wlan first!\n"); 
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"wlan %d not exsit!\n",wlan_id); 
	}
	else
		vty_out(vty,"error %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(set_sta_tunnel_switch_state_cmd_func,
	set_sta_tunnel_switch_state_cmd,
	"set sta tunnel switch (enable|disable) base wlan ID",
	"set sta tunnel switch service\n"
	"sta sta tunnel switch service enable or disable\n"
	"WLAN ID info"
)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	char wlan_id = 0;
	int is_enable = 0;
	if (!strcmp(argv[0],"enable")||(tolower(argv[0][0]) == 'e'))
	{
		is_enable = 1;	
	
	}		
	else if (!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd'))
	{
		is_enable = 0;
	}

	ret = parse_char_ID((char*)argv[1], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int localid = 1;int slot_id = HostSlotId;
	
	DBusConnection *dcli_dbus_connection = NULL;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	vty_out(vty,"localid: %d,slot_id: %d ,**********yjl add for test ****11111111111111*******\n",localid, slot_id);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_TUNNEL_SWITCH_STATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&is_enable,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
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
	if(ret==0){
		index = 0;
	    localid = 1; slot_id = HostSlotId;
		dcli_dbus_connection = NULL;
		if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if (vty->node == LOCAL_HANSI_NODE){
			index = vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		memset(BUSNAME,0,PATH_LEN);	
	    memset(OBJPATH,0,PATH_LEN);
	    memset(INTERFACE,0,PATH_LEN);
		ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid, index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid, index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
		vty_out(vty,"BUSNAME: %s,OBJPATH: %s INTERFACE: %s,**********yjl add for test ****9999999999*******\n", BUSNAME, OBJPATH, INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WLAN_STA_TUNNEL_SWITCH_STATE);
		vty_out(vty,"BUSNAME: %s,OBJPATH: %s INTERFACE: %s,: %p**********yjl add for test ****121221212*******\n", BUSNAME, OBJPATH, INTERFACE, query);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&is_enable,
								DBUS_TYPE_BYTE,&wlan_id,
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
		if(ret == 0){
			vty_out(vty,"set sta tunnel switch successfully\n");
		}else{
			vty_out(vty,"error %d\n",ret);
		}
	}
	else if(ret == ASD_DHCP_POOL_NOT_EXIST){
		vty_out(vty,"sta vir-dhcp poll not exsit!\n"); 
	}
	else if(ret == ASD_WLAN_BE_ENABLE){
		vty_out(vty,"Please disable wlan first!\n"); 
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"wlan %d not exsit!\n",wlan_id); 
	}
	else
		vty_out(vty,"error %d\n",ret);
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(set_sta_state_cmd_func,
	set_sta_state_cmd,
	"set sta MAC (auth|deauth) base wlan ID wtp WTPID",
	"sta mac\n"
	"sta sta auth or deauth\n"
	"WLAN ID info"
)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	char wlan_id = 0;
	int is_enable = 0;
	char mac1[MAC_LEN];
	int mac[MAC_LEN];
	int wtpid = 0;
	memset(mac,0,MAC_LEN);
	sscanf(argv[0],"%X:%X:%X:%X:%X:%X",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	if (!strcmp(argv[1],"auth")||(tolower(argv[1][0]) == 'a'))
	{
		is_enable = 1;	
	
	}		
	else if (!strcmp(argv[1],"deauth")||(tolower(argv[1][0]) == 'd'))
	{
		is_enable = 0;
	}

	ret = parse_char_ID((char*)argv[2], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}		
	ret = parse_int_ID((char*)argv[3], &wtpid);
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int localid = 1;int slot_id = HostSlotId;
	DBusConnection *dcli_dbus_connection = NULL;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WLAN_STA_STATE);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&(mac1[0]),
							DBUS_TYPE_BYTE,&(mac1[1]),
							DBUS_TYPE_BYTE,&(mac1[2]),
							DBUS_TYPE_BYTE,&(mac1[3]),
							DBUS_TYPE_BYTE,&(mac1[4]),
							DBUS_TYPE_BYTE,&(mac1[5]),
							DBUS_TYPE_UINT32,&is_enable,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_UINT32,&wtpid,
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
	if(ret == 0){
		vty_out(vty,"set sta successfully\n");
	}else{
		vty_out(vty,"error %d\n",ret);
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}
/*****end***********************yjl copy from aw3.1.2 for local forwarding.2014-2-28*********************/

#if 0 /*****wangchao moved those to dcli_wireless_main.c*****/
int dcli_wlan_list_show_running_config(struct vty*vty) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	//index = vty->index;
	//localid = vty->local;
	//slot_id = vty->slotindex;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID))	{
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"WLAN LIST");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return 1;	
	}
}
#endif


#if 0
int dcli_wtp_list_show_running_config(struct vty*vty) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	//index = vty->index;
	//localid = vty->local;
	//slot_id = vty->slotindex;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"WTP LIST");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return 1;	
	}

}
#endif

#if 0
int dcli_bss_list_show_running_config(struct vty*vty) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	int localid = 1;int slot_id = HostSlotId;int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	//index = vty->index;
	//localid = vty->local;
	//slot_id = vty->slotindex;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show bss list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"BSS LIST");
		vtysh_add_show_string(_tmpstr);					
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
	} else 	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 	{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);

			dbus_message_unref(reply);
			return 1;	
		}
	}

}
#endif

#if 0

char* dcli_hansi_wlan_list_show_running_config(int localid, int slot_id,int index) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID))	{
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));	
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return NULL;	
	}
}

#endif

#if 0
char* dcli_hansi_wtp_list_show_running_config(int localid, int slot_id,int index) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));	
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return NULL;	
	}

}
#endif

#if 0
char *dcli_hansi_bss_list_show_running_config(int localid, int slot_id, int index) 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);
	/*query = dbus_message_new_method_call(
							ASD_DBUS_BUSNAME,		\
							ASD_DBUS_STA_OBJPATH , \
							ASD_DBUS_STA_INTERFACE ,		\
							ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show bss list config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));	
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} else 	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 	{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);

			dbus_message_unref(reply);
			return NULL;	
		}
	}

}
#endif

void dcli_sta_init(void) {
	install_element(VIEW_NODE,&show_sta_cmd);
	install_element(VIEW_NODE,&show_wlan_sta_cmd);
	install_element(VIEW_NODE,&show_wtp_sta_cmd);
	install_element(VIEW_NODE,&show_all_wlan_MAC_list_cmd);	
	install_element(VIEW_NODE,&show_all_wtp_MAC_list_cmd);
	install_element(VIEW_NODE,&show_all_bss_MAC_list_cmd);
	install_element(VIEW_NODE,&show_all_sta_static_info_cmd);
	install_element(VIEW_NODE,&show_sta_list_cmd);
	install_element(VIEW_NODE,&show_sta_list_by_group_cmd);
	install_element(VIEW_NODE,&show_sta_summary_cmd);
	install_element(VIEW_NODE,&show_traffic_limit_info_cmd);
	install_element(VIEW_NODE,&show_traffic_limit_info_rd_cmd);
	install_element(VIEW_NODE,&show_wtp_MAC_list_cmd);
	install_element(VIEW_NODE,&show_wlan_MAC_list_cmd);
	install_element(VIEW_NODE,&show_bss_MAC_list_cmd);
	install_element(VIEW_NODE,&show_radio_info_cmd);
	install_element(VIEW_NODE,&show_wlan_info_cmd);
	install_element(VIEW_NODE,&show_bss_info_cmd);
	install_element(VIEW_NODE,&show_sta_list_detial_cmd);
	install_element(VIEW_NODE,&show_sta_detial_bywlan_cmd);
	install_element(VIEW_NODE,&show_sta_detial_bywtp_cmd);
	install_element(VIEW_NODE,&show_roaming_sta_cmd);
	install_element(VIEW_NODE,&show_bss_summary_cmd);
	install_element(VIEW_NODE,&show_bss_bssindex_cmd);
#if 0
	install_node(&sta_node,NULL);
	install_default(STA_NODE);
	/*-----------------------------------VIEW_NODE-----------------------------------*/
	install_element(VIEW_NODE,&show_sta_cmd);														/*a1*/
	install_element(VIEW_NODE,&show_bss_info_cmd);												/*a2*/
	install_element(VIEW_NODE,&show_wlan_info_cmd);												/*a3*/
	install_element(VIEW_NODE,&show_wapi_info_cmd);												/*a4*/
	install_element(VIEW_NODE,&show_wapi_protocol_info_cmd);										/*a5*/
	install_element(VIEW_NODE,&show_radio_info_cmd);												/*a6*/
	install_element(VIEW_NODE,&extend_show_sta_cmd);												/*a7*/
	install_element(VIEW_NODE,&show_sta_mac_static_info_cmd);	/*ht add 100114*/						/*a8*/
	install_element(VIEW_NODE,&show_all_sta_static_info_cmd);	/*ht add 100114*/						/*a9*/
	install_element(VIEW_NODE,&show_sta_list_cmd);												/*a10*/
	install_element(VIEW_NODE,&show_sta_list_by_interface_cmd);	
	install_element(VIEW_NODE,&show_sta_list_by_group_cmd);										/*a11*/
	install_element(VIEW_NODE,&show_sta_summary_cmd);												/*a12*/
	install_element(VIEW_NODE,&show_wlan_sta_cmd);												/*a13*/
	install_element(VIEW_NODE,&show_wtp_sta_cmd);													/*a14*/
	install_element(VIEW_NODE,&extend_show_wtp_sta_cmd);											/*a15*/
	install_element(VIEW_NODE,&show_channel_access_time_cmd);		/*ht add */						/*a16*/
	install_element(VIEW_NODE,&show_all_wlan_MAC_list_cmd);										/*a17*/
	install_element(VIEW_NODE,&show_all_wtp_MAC_list_cmd);										/*a18*/
	install_element(VIEW_NODE,&show_all_bss_MAC_list_cmd);										/*a19*/
	install_element(VIEW_NODE,&show_wlan_MAC_list_cmd);	/*ht add 08.12.22*/						/*a20*/
	
	install_element(VIEW_NODE,&show_wtp_MAC_list_cmd);											/*a21*/
	install_element(VIEW_NODE,&show_bss_MAC_list_cmd);											/*a22*/
	install_element(VIEW_NODE,&show_wlan_wids_MAC_list_cmd); /*ht add 090715*/						/*a23*/
	install_element(VIEW_NODE,&show_mib_info_cmd);	/*	xm0616*/								/*a24*/
	install_element(VIEW_NODE,&show_wapi_mib_info_cmd);	/*	xm0623*/							/*a25*/
	install_element(VIEW_NODE,&show_traffic_limit_info_rd_cmd);	/*	xm0723*/					/*a26*/
	install_element(VIEW_NODE,&show_traffic_limit_info_cmd);	/*	xm0723*/						/*a27*/
	install_element(VIEW_NODE,&show_distinguish_information_of_all_wtp_cmd);	//mahz add 2011.1.17
	install_element(VIEW_NODE,&show_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.19
	install_element(VIEW_NODE,&show_sta_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.24
	/*-----------------------------------ENABLE_NODE-----------------------------------*/
	install_element(ENABLE_NODE,&show_sta_cmd);														/*a1*/
	install_element(ENABLE_NODE,&show_bss_info_cmd);												/*a2*/
	install_element(ENABLE_NODE,&show_wlan_info_cmd);												/*a3*/
	install_element(ENABLE_NODE,&show_wapi_info_cmd);												/*a4*/
	install_element(ENABLE_NODE,&show_wapi_protocol_info_cmd);										/*a5*/
	install_element(ENABLE_NODE,&show_radio_info_cmd);												/*a6*/
	install_element(ENABLE_NODE,&extend_show_sta_cmd);												/*a7*/
	install_element(ENABLE_NODE,&show_sta_mac_static_info_cmd);	/*ht add 100114*/						/*a8*/
	install_element(ENABLE_NODE,&show_all_sta_static_info_cmd);	/*ht add 100114*/						/*a9*/
	install_element(ENABLE_NODE,&show_sta_list_cmd);												/*a10*/
	
	install_element(ENABLE_NODE,&show_sta_list_by_group_cmd);										/*a11*/
	install_element(ENABLE_NODE,&show_sta_summary_cmd);												/*a12*/
	install_element(ENABLE_NODE,&show_wlan_sta_cmd);												/*a13*/
	install_element(ENABLE_NODE,&show_wtp_sta_cmd);													/*a14*/
	install_element(ENABLE_NODE,&extend_show_wtp_sta_cmd);											/*a15*/
	install_element(ENABLE_NODE,&show_channel_access_time_cmd);		/*ht add */						/*a16*/
	install_element(ENABLE_NODE,&show_all_wlan_MAC_list_cmd);										/*a17*/
	install_element(ENABLE_NODE,&show_all_wtp_MAC_list_cmd);										/*a18*/
	install_element(ENABLE_NODE,&show_all_bss_MAC_list_cmd);										/*a19*/
	install_element(ENABLE_NODE,&show_wlan_MAC_list_cmd);	/*ht add 08.12.22*/						/*a20*/
	install_element(ENABLE_NODE,&show_sta_list_by_interface_cmd);
	install_element(ENABLE_NODE,&show_wtp_MAC_list_cmd);											/*a21*/
	install_element(ENABLE_NODE,&show_bss_MAC_list_cmd);											/*a22*/
	install_element(ENABLE_NODE,&show_wlan_wids_MAC_list_cmd); /*ht add 090715*/						/*a23*/
	install_element(ENABLE_NODE,&show_mib_info_cmd);	/*	xm0616*/								/*a24*/
	install_element(ENABLE_NODE,&show_wapi_mib_info_cmd);	/*	xm0623*/							/*a25*/
	install_element(ENABLE_NODE,&show_traffic_limit_info_rd_cmd);	/*	xm0723*/					/*a26*/
	install_element(ENABLE_NODE,&show_traffic_limit_info_cmd);	/*	xm0723*/						/*a27*/
	
	install_element(ENABLE_NODE,&kick_sta_cmd);	
	install_element(ENABLE_NODE,&wlan_add_MAC_list_cmd);	/*ht add*/ 
	install_element(ENABLE_NODE,&wtp_add_MAC_list_cmd);
	install_element(ENABLE_NODE,&bss_add_MAC_list_cmd);
	install_element(ENABLE_NODE,&wlan_del_MAC_list_cmd);	
	install_element(ENABLE_NODE,&wtp_del_MAC_list_cmd);
	install_element(ENABLE_NODE,&bss_del_MAC_list_cmd);
	install_element(ENABLE_NODE,&wlan_use_MAC_list_cmd);
	install_element(ENABLE_NODE,&wtp_use_MAC_list_cmd);
	install_element(ENABLE_NODE,&bss_use_MAC_list_cmd);
	install_element(ENABLE_NODE,&show_distinguish_information_of_all_wtp_cmd);	//mahz add 2011.1.17
	install_element(ENABLE_NODE,&show_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.19
	install_element(ENABLE_NODE,&show_sta_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.24
	/*add for black name list by nl  2010-08-28*/
	/*================================================================*/
	install_element(ENABLE_NODE,&ac_del_MAC_list_cmd);											
	install_element(ENABLE_NODE,&ac_add_MAC_list_cmd);
	install_element(ENABLE_NODE,&ac_use_MAC_list_cmd);
	install_element(ENABLE_NODE,&show_ac_MAC_list_cmd);
	install_element(ENABLE_NODE,&show_wlan_sta_of_all_cmd);  //fengwenchao add 20110113
	/*================================================================*/
	/*-----------------------------------CONFIG_NODE-----------------------------------*/
	install_element(CONFIG_NODE,&show_sta_cmd);														/*a1*/
	install_element(CONFIG_NODE,&show_bss_info_cmd);												/*a2*/
	install_element(CONFIG_NODE,&show_wlan_info_cmd);												/*a3*/
	install_element(CONFIG_NODE,&show_wapi_info_cmd);												/*a4*/
	install_element(CONFIG_NODE,&show_wapi_protocol_info_cmd);										/*a5*/
	install_element(CONFIG_NODE,&show_radio_info_cmd);												/*a6*/
	install_element(CONFIG_NODE,&extend_show_sta_cmd);												/*a7*/
	install_element(CONFIG_NODE,&show_sta_mac_static_info_cmd);	/*ht add 100114*/						/*a8*/
	install_element(CONFIG_NODE,&show_all_sta_static_info_cmd);	/*ht add 100114*/						/*a9*/
	install_element(CONFIG_NODE,&show_sta_list_cmd);												/*a10*/
	install_element(CONFIG_NODE,&show_sta_list_by_interface_cmd);
	install_element(CONFIG_NODE,&show_sta_list_by_group_cmd);										/*a11*/
	install_element(CONFIG_NODE,&show_sta_summary_cmd);												/*a12*/
	install_element(CONFIG_NODE,&show_wlan_sta_cmd);												/*a13*/
	install_element(CONFIG_NODE,&show_wtp_sta_cmd);													/*a14*/
	install_element(CONFIG_NODE,&extend_show_wtp_sta_cmd);											/*a15*/
	install_element(CONFIG_NODE,&show_channel_access_time_cmd);	/*ht add*/							/*a16*/
	install_element(CONFIG_NODE,&show_all_wlan_MAC_list_cmd);/*ht add */								/*a17*/
	install_element(CONFIG_NODE,&show_all_wtp_MAC_list_cmd);/*ht add */								/*a18*/
	install_element(CONFIG_NODE,&show_all_bss_MAC_list_cmd);/*ht add */								/*a19*/
	install_element(CONFIG_NODE,&show_wlan_MAC_list_cmd);											/*a20*/
	
	install_element(CONFIG_NODE,&show_wtp_MAC_list_cmd);											/*a21*/
	install_element(CONFIG_NODE,&show_bss_MAC_list_cmd);											/*a22*/
	install_element(CONFIG_NODE,&show_wlan_wids_MAC_list_cmd); /*ht add 090715*/						/*a23*/
	install_element(CONFIG_NODE,&show_mib_info_cmd);	/*	xm0616*/								/*a24*/
	install_element(CONFIG_NODE,&show_wapi_mib_info_cmd);	/*	xm0623*/							/*a25*/
	install_element(CONFIG_NODE,&show_traffic_limit_info_rd_cmd);	/*	xm0723*/					/*a26*/
	install_element(CONFIG_NODE,&show_traffic_limit_info_cmd);	/*	xm0723*/						/*a27*/
	install_element(CONFIG_NODE,&show_all_sta_base_info_cmd);
	

	install_element(CONFIG_NODE,&set_wlan_extern_balance_cmd);
	install_element(CONFIG_NODE,&set_sta_static_info_traffic_limit_cmd);	/*ht add 100114*/
	install_element(CONFIG_NODE,&del_sta_static_info_cmd);	/*ht add 100114*/
	install_element(CONFIG_NODE,&kick_sta_cmd);
	install_element(CONFIG_NODE,&set_acct_id_sta_down_cmd);			//mahz add 2011.6.3
	install_element(CONFIG_NODE,&wlan_add_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&wtp_add_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&bss_add_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&wlan_del_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&wtp_del_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&bss_del_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&wlan_use_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&wtp_use_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&bss_use_MAC_list_cmd);/*ht add */
	install_element(CONFIG_NODE,&set_ac_flow_cmd);	/*	xm0714*/
	install_element(CONFIG_NODE,&set_asd_sta_arp_listen_cmd);	
	install_element(CONFIG_NODE,&sta_arp_set_cmd);
	install_element(CONFIG_NODE,&set_sta_static_arp_cmd);	
	install_element(CONFIG_NODE,&set_sta_arp_group_cmd);
	install_element(CONFIG_NODE,&show_sta_v2_cmd);
	install_element(CONFIG_NODE,&show_wlan_info_allwlan_cmd);  //fengwenchao add 20101221
	install_element(CONFIG_NODE,&show_distinguish_information_of_all_wtp_cmd);	//mahz add 2011.1.17
	install_element(CONFIG_NODE,&show_wlan_sta_of_all_cmd);  //fengwenchao add 20110113
	install_element(CONFIG_NODE,&show_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.19
	install_element(CONFIG_NODE,&show_sta_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.24
	install_element(CONFIG_NODE,&set_asd_switch_cmd);			//ht add,110308
	install_element(CONFIG_NODE,&clean_wlan_accessed_sta_cmd);	//weichao add 20110804
	/*add for black name list by nl  2010-08-28*/
	/*================================================================*/
	install_element(CONFIG_NODE,&ac_del_MAC_list_cmd);
	install_element(CONFIG_NODE,&ac_add_MAC_list_cmd);
	install_element(CONFIG_NODE,&ac_use_MAC_list_cmd);
	install_element(CONFIG_NODE,&show_ac_MAC_list_cmd);
	
    /*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
	install_element(CONFIG_NODE,&set_sta_vir_dhcp_pool_ip_range_cmd);
	install_element(CONFIG_NODE,&delete_sta_vir_dhcp_pool_ip_range_cmd);
	install_element(CONFIG_NODE,&set_sta_vir_dhcp_pool_state_cmd);
	install_element(CONFIG_NODE,&set_sta_tunnel_switch_state_cmd);
	install_element(CONFIG_NODE,&set_sta_state_cmd);
#endif
	/*================================================================*/
/************************************************************************************/
	/*-----------------------------------HANSI_NODE-----------------------------------*/
	install_node(&hansi_sta_node,NULL,"HANSI_STA_NODE");
	install_default(HANSI_STA_NODE);	
	install_element(HANSI_NODE,&show_sta_list_cmd);
	install_element(HANSI_NODE,&show_sta_list_by_group_cmd);
	install_element(HANSI_NODE,&show_sta_summary_cmd);	
	install_element(HANSI_NODE,&show_wlan_sta_cmd);	
	install_element(HANSI_NODE,&show_wtp_sta_cmd);
	install_element(HANSI_NODE,&extend_show_wtp_sta_cmd);
	install_element(HANSI_NODE,&show_channel_access_time_cmd);	/*ht add*/
	install_element(HANSI_NODE,&show_sta_cmd);
	install_element(HANSI_NODE,&show_bss_info_cmd);
	install_element(HANSI_NODE,&show_wlan_info_cmd);
	install_element(HANSI_NODE,&show_wapi_info_cmd);
	install_element(HANSI_NODE,&show_wapi_protocol_info_cmd);
	install_element(HANSI_NODE,&show_radio_info_cmd);
	install_element(HANSI_NODE,&extend_show_sta_cmd);
	install_element(HANSI_NODE,&show_all_wlan_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&show_all_wtp_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&show_all_bss_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&show_wlan_MAC_list_cmd);
	install_element(HANSI_NODE,&show_wtp_MAC_list_cmd);
	install_element(HANSI_NODE,&show_bss_MAC_list_cmd);
	install_element(HANSI_NODE,&show_mib_info_cmd);	/*	xm0616*/
	install_element(HANSI_NODE,&show_wapi_mib_info_cmd);	/*	xm0623*/
	install_element(HANSI_NODE,&show_traffic_limit_info_rd_cmd);	/*	xm0723*/
	install_element(HANSI_NODE,&show_traffic_limit_info_cmd);	/*	xm0723*/
	install_element(HANSI_NODE,&show_wlan_wids_MAC_list_cmd); /*ht add 090715*/
	//install_element(HANSI_NODE,&set_asd_switch_cmd);			//wuwl del.share mem has been removed,so cannot get msg from wsm.
	install_element(HANSI_NODE,&clean_wlan_accessed_sta_cmd);	//weichao add 20110804
	install_element(HANSI_NODE,&show_sta_list_by_interface_cmd);
	install_element(HANSI_NODE,&kick_sta_cmd);
	install_element(HANSI_NODE,&show_all_sta_base_info_cmd);
	install_element(HANSI_NODE,&set_acct_id_sta_down_cmd);			//mahz add 2011.6.3
	install_element(HANSI_NODE,&wlan_add_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&wtp_add_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&bss_add_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&wlan_del_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&wtp_del_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&bss_del_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&wlan_use_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&wtp_use_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&bss_use_MAC_list_cmd);/*ht add */
	install_element(HANSI_NODE,&set_ac_flow_cmd);	/*	xm0714*/
	install_element(HANSI_NODE,&set_wlan_extern_balance_cmd);
	install_element(HANSI_NODE,&sta_arp_set_cmd);
	install_element(HANSI_NODE,&set_asd_sta_arp_listen_cmd);	
	install_element(HANSI_NODE,&set_asd_sta_get_ip_from_dhcpsnoop_cmd);	
	install_element(HANSI_NODE,&set_sta_static_arp_cmd);	
	install_element(HANSI_NODE,&set_sta_arp_group_cmd);
	install_element(HANSI_NODE,&show_sta_v2_cmd);	
	install_element(HANSI_NODE,&show_wlan_info_allwlan_cmd);  //fengwenchao add 20101221
	install_element(HANSI_NODE,&show_distinguish_information_of_all_wtp_cmd);	//mahz add 2011.1.17
	install_element(HANSI_NODE,&show_wlan_sta_of_all_cmd);  //fengwenchao add 20110113
	install_element(HANSI_NODE,&show_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.19
	install_element(HANSI_NODE,&show_sta_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.24
	install_element(HANSI_NODE,&show_sta_mac_static_info_cmd);	/*ht add 100114*/						/*a8*/
	install_element(HANSI_NODE,&show_all_sta_static_info_cmd);	/*ht add 100114*/						/*a9*/
	install_element(HANSI_NODE,&set_sta_static_info_traffic_limit_cmd);	/*ht add 100114*/
	install_element(HANSI_NODE,&del_sta_static_info_cmd);	/*ht add 100114*/
	install_element(HANSI_NODE,&set_sta_hotspot_map_nas_cmd);
	install_element(HANSI_NODE,&del_sta_hotspot_cmd);//qiuchen add it 2012.10.23
	install_element(HANSI_NODE,&show_sta_hotspot_information_cmd);//qiuchen
	install_element(HANSI_NODE,&show_sta_list_detial_cmd);
	install_element(HANSI_NODE,&show_sta_detial_bywlan_cmd);
	install_element(HANSI_NODE,&show_sta_detial_bywtp_cmd);
	install_element(HANSI_NODE,&set_asd_sta_idle_time_cmd);
	install_element(HANSI_NODE,&set_asd_sta_idle_time_switch_cmd);
//	install_element(HANSI_NODE,&set_asd_ipset_switch_cmd);
	install_element(HANSI_NODE,&set_asd_bak_sta_update_value_cmd);

    /*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
	install_element(HANSI_NODE,&set_sta_vir_dhcp_pool_ip_range_cmd);
	install_element(HANSI_NODE,&delete_sta_vir_dhcp_pool_ip_range_cmd);
	install_element(HANSI_NODE,&set_sta_vir_dhcp_pool_state_cmd);
	install_element(HANSI_NODE,&set_sta_tunnel_switch_state_cmd);
	install_element(HANSI_NODE,&set_sta_state_cmd);
	/*add for black name list by nl  2010-08-28*/
	/*================================================================*/
	install_element(HANSI_NODE,&ac_del_MAC_list_cmd);
	install_element(HANSI_NODE,&ac_add_MAC_list_cmd);
	install_element(HANSI_NODE,&ac_use_MAC_list_cmd);
	install_element(HANSI_NODE,&show_ac_MAC_list_cmd);
	/*================================================================*/
	install_element(HANSI_NODE,&show_roaming_sta_cmd);
	install_element(HANSI_NODE,&show_bss_summary_cmd);
	install_element(HANSI_NODE,&show_bss_bssindex_cmd);
	install_element(HANSI_NODE,&set_asd_1x_radius_format_cmd);	/* add to set asd radius format */

#ifdef __ASD_STA_ACL
	/* caojia add for sta acl function */
	install_element(HANSI_NODE,&set_sta_acl_cmd);	
	install_element(HANSI_NODE,&show_sta_acl_cmd);
#endif
	/************************************************************************************/
	/*-----------------------------------LOCAL_HANSI_NODE-----------------------------------*/
	install_node(&local_hansi_sta_node,NULL,"LOCAL_HANSI_STA_NODE");
	install_default(LOCAL_HANSI_STA_NODE);	
	install_element(LOCAL_HANSI_NODE,&show_sta_list_cmd);
	install_element(LOCAL_HANSI_NODE,&show_sta_list_by_group_cmd);
	install_element(LOCAL_HANSI_NODE,&show_sta_summary_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_wlan_sta_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_wtp_sta_cmd);
	install_element(LOCAL_HANSI_NODE,&extend_show_wtp_sta_cmd);
	install_element(LOCAL_HANSI_NODE,&show_channel_access_time_cmd);	/*ht add*/
	install_element(LOCAL_HANSI_NODE,&show_sta_cmd);
	install_element(LOCAL_HANSI_NODE,&show_bss_info_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wlan_info_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wapi_info_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wapi_protocol_info_cmd);
	install_element(LOCAL_HANSI_NODE,&show_radio_info_cmd);
	install_element(LOCAL_HANSI_NODE,&extend_show_sta_cmd);
	install_element(LOCAL_HANSI_NODE,&show_all_wlan_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&show_all_wtp_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&show_all_bss_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&show_wlan_MAC_list_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wtp_MAC_list_cmd);
	install_element(LOCAL_HANSI_NODE,&show_bss_MAC_list_cmd);
	install_element(LOCAL_HANSI_NODE,&show_mib_info_cmd);	/*	xm0616*/
	install_element(LOCAL_HANSI_NODE,&show_wapi_mib_info_cmd);	/*	xm0623*/
	install_element(LOCAL_HANSI_NODE,&show_traffic_limit_info_rd_cmd);	/*	xm0723*/
	install_element(LOCAL_HANSI_NODE,&show_traffic_limit_info_cmd);	/*	xm0723*/
	install_element(LOCAL_HANSI_NODE,&show_wlan_wids_MAC_list_cmd); /*ht add 090715*/
	//install_element(LOCAL_HANSI_NODE,&set_asd_switch_cmd);			//wuwl del.share mem has been removed,so cannot get msg from wsm.
	install_element(LOCAL_HANSI_NODE,&clean_wlan_accessed_sta_cmd);	//weichao add 20110804
	install_element(LOCAL_HANSI_NODE,&show_sta_list_by_interface_cmd);
	install_element(LOCAL_HANSI_NODE,&show_all_sta_base_info_cmd);
	
	install_element(LOCAL_HANSI_NODE,&kick_sta_cmd);
	install_element(LOCAL_HANSI_NODE,&wlan_add_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&wtp_add_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&bss_add_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&wlan_del_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&wtp_del_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&bss_del_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&wlan_use_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&wtp_use_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&bss_use_MAC_list_cmd);/*ht add */
	install_element(LOCAL_HANSI_NODE,&set_ac_flow_cmd);	/*	xm0714*/
	install_element(LOCAL_HANSI_NODE,&set_wlan_extern_balance_cmd);
	install_element(LOCAL_HANSI_NODE,&sta_arp_set_cmd);
	install_element(LOCAL_HANSI_NODE,&set_asd_sta_arp_listen_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_asd_sta_get_ip_from_dhcpsnoop_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_sta_static_arp_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_sta_arp_group_cmd);
	install_element(LOCAL_HANSI_NODE,&show_sta_v2_cmd);	
	install_element(LOCAL_HANSI_NODE,&show_wlan_info_allwlan_cmd);  //fengwenchao add 20101221
	install_element(LOCAL_HANSI_NODE,&show_distinguish_information_of_all_wtp_cmd);	//mahz add 2011.1.17
	install_element(LOCAL_HANSI_NODE,&show_wlan_sta_of_all_cmd);  //fengwenchao add 20110113
	install_element(LOCAL_HANSI_NODE,&show_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.19
	install_element(LOCAL_HANSI_NODE,&show_sta_wapi_mib_info_of_all_wtp_cmd);			//mahz add 2011.1.24
	install_element(LOCAL_HANSI_NODE,&show_sta_mac_static_info_cmd);	/*ht add 100114*/						/*a8*/
	install_element(LOCAL_HANSI_NODE,&show_all_sta_static_info_cmd);	/*ht add 100114*/						/*a9*/
	install_element(LOCAL_HANSI_NODE,&set_sta_static_info_traffic_limit_cmd);	/*ht add 100114*/
	install_element(LOCAL_HANSI_NODE,&del_sta_static_info_cmd);	/*ht add 100114*/
	install_element(LOCAL_HANSI_NODE,&set_sta_hotspot_map_nas_cmd);
	install_element(LOCAL_HANSI_NODE,&del_sta_hotspot_cmd);//qiuchen add it 2012.10.23
	install_element(LOCAL_HANSI_NODE,&show_sta_hotspot_information_cmd);//qiuchen
	install_element(LOCAL_HANSI_NODE,&show_sta_list_detial_cmd);
	install_element(LOCAL_HANSI_NODE,&show_sta_detial_bywlan_cmd);
	install_element(LOCAL_HANSI_NODE,&show_sta_detial_bywtp_cmd);
	install_element(LOCAL_HANSI_NODE,&set_asd_sta_idle_time_cmd);
	install_element(LOCAL_HANSI_NODE,&set_asd_sta_idle_time_switch_cmd);
//	install_element(LOCAL_HANSI_NODE,&set_asd_ipset_switch_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_asd_bak_sta_update_value_cmd);
	
	/*add for black name list by nl  2010-08-28*/
	/*================================================================*/
	install_element(LOCAL_HANSI_NODE,&ac_del_MAC_list_cmd);
	install_element(LOCAL_HANSI_NODE,&ac_add_MAC_list_cmd);
	install_element(LOCAL_HANSI_NODE,&ac_use_MAC_list_cmd);
	install_element(LOCAL_HANSI_NODE,&show_ac_MAC_list_cmd);
	/*================================================================*/
	install_element(LOCAL_HANSI_NODE,&show_roaming_sta_cmd);
	install_element(LOCAL_HANSI_NODE,&show_bss_summary_cmd);
	install_element(LOCAL_HANSI_NODE,&show_bss_bssindex_cmd);
	return;
}

#endif
