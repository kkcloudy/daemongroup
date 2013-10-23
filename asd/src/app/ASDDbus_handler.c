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
* ASDDbus_handler.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/
#include <string.h>
#include "includes.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 

#include "asd.h"
#include "ASDCallback.h"
#include "ASD8021XOp.h"
#include "circle.h"
#include "priv_netlink.h"
#include "ASD80211Op.h"
#include "ASDAccounting.h"
#include "ASD80211AuthOp.h"
#include "ASDStaInfo.h"

#include "ASDHWInfo.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "ASDRadius/radius_client.h"
#include "config.h"
#include "ip_addr.h"
#include "ASDHWInfo.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ASDDbus_handler.h"
#include "ASDRadius/radius.h"
#include "ASDWPAOp.h"
#include "StaFlashDisconnOp.h"
#include "ASDMlme.h"
#include "ASDCallback_asd.h"
#include "config.h"
#include "ASDDbus.h"
#include "ASDEAPAuth.h"
extern int wpa_debug_level;


unsigned int ASD_STA_SUMMARY(unsigned int wtp[],unsigned wlan[]){
//	0610xm	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int i = 0, num=0, j=0, bss_num=0;
	
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL){
					if(interfaces->iface[i]->bss[j]->num_sta!=0){
						if((interfaces->iface[i]->bss[j]->WlanID<WLAN_NUM)
							&&(interfaces->iface[i]->bss[j]->Radio_G_ID/L_RADIO_NUM<WTP_NUM)){
							bss_num=interfaces->iface[i]->bss[j]->num_sta;
							wlan[interfaces->iface[i]->bss[j]->WlanID]+=bss_num;
							wtp[interfaces->iface[i]->bss[j]->Radio_G_ID/L_RADIO_NUM]+=bss_num;
						}
						num += interfaces->iface[i]->bss[j]->num_sta;	
					}
				}
			}
			i++;
		} else {
			i += 4 - i%L_RADIO_NUM;
		}
			
	}
	return num;
}

int Clear_SECURITY(unsigned char SecurityID){
	if(ASD_SECURITY[SecurityID] != NULL){		
		if(ASD_SECURITY[SecurityID]->SecurityKey != NULL){
			free(ASD_SECURITY[SecurityID]->SecurityKey);
			ASD_SECURITY[SecurityID]->keyLen = 0;
			ASD_SECURITY[SecurityID]->SecurityKey = NULL;
		}		
		if(ASD_SECURITY[SecurityID]->acct.acct_ip != NULL){
			asd_ip_secret_del(inet_addr(ASD_SECURITY[SecurityID]->acct.acct_ip));
			free(ASD_SECURITY[SecurityID]->acct.acct_ip);
			ASD_SECURITY[SecurityID]->acct.acct_ip = NULL;
		}
		if(ASD_SECURITY[SecurityID]->acct.acct_shared_secret){
			free(ASD_SECURITY[SecurityID]->acct.acct_shared_secret);
			ASD_SECURITY[SecurityID]->acct.acct_shared_secret = NULL;
		}		
		if(ASD_SECURITY[SecurityID]->auth.auth_ip != NULL){
			free(ASD_SECURITY[SecurityID]->auth.auth_ip);
			ASD_SECURITY[SecurityID]->auth.auth_ip = NULL;
		}
		if(ASD_SECURITY[SecurityID]->auth.auth_shared_secret != NULL){
			free(ASD_SECURITY[SecurityID]->auth.auth_shared_secret);
			ASD_SECURITY[SecurityID]->auth.auth_shared_secret = NULL;
		}
		if(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip!= NULL){
			asd_ip_secret_del(inet_addr(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip));
			free(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip);
			ASD_SECURITY[SecurityID]->acct.secondary_acct_ip = NULL;
		}
		if(ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret){
			free(ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret);
			ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret = NULL;
		}		
		if(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip != NULL){
			free(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip);
			ASD_SECURITY[SecurityID]->auth.secondary_auth_ip = NULL;
		}
		if(ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret != NULL){
			free(ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret);
			ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret = NULL;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.as_ip!= NULL){
			free(ASD_SECURITY[SecurityID]->wapi_as.as_ip);
			ASD_SECURITY[SecurityID]->wapi_as.as_ip_len=0;
			ASD_SECURITY[SecurityID]->wapi_as.as_ip= NULL;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.certification_path!= NULL){
			free(ASD_SECURITY[SecurityID]->wapi_as.certification_path);
			ASD_SECURITY[SecurityID]->wapi_as.certification_path_len=0;
			ASD_SECURITY[SecurityID]->wapi_as.certification_path= NULL;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path!= NULL){
			free(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path);
			ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path_len=0;
			ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path= NULL;
		}
		//mahz add 2011.3.31
		ASD_SECURITY[SecurityID]->wapi_as.multi_cert = 0;	
		if(ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path!= NULL){
			free(ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path);
			ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path_len=0;
			ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path= NULL;
		}
		//
		if(ASD_SECURITY[SecurityID]->wapi_as.unite_cert_path!= NULL){
			free(ASD_SECURITY[SecurityID]->wapi_as.unite_cert_path);
			ASD_SECURITY[SecurityID]->wapi_as.unite_cert_path_len=0;
			ASD_SECURITY[SecurityID]->wapi_as.unite_cert_path= NULL;
		}
		/*nl 09/11/03*/
		if(ASD_SECURITY[SecurityID]->wapi_config != NULL){
		free(ASD_SECURITY[SecurityID]->wapi_config);
		ASD_SECURITY[SecurityID]->wapi_config = NULL;
		}
		ASD_SECURITY[SecurityID]->wapi_radius_auth = 0;		//mahz add 2011.1.21
		//mahz add 2010.12.10
		if(ASD_SECURITY[SecurityID]->user_passwd != NULL){
			free(ASD_SECURITY[SecurityID]->user_passwd);
			ASD_SECURITY[SecurityID]->user_passwd = NULL;
		}
		//		
		ASD_SECURITY[SecurityID]->wapi_as.certification_type = 0;
		//if(ASD_SECURITY[SecurityID]->host_ip!= NULL){
		//	free(ASD_SECURITY[SecurityID]->host_ip);
		//	ASD_SECURITY[SecurityID]->host_ip= NULL;
		//}
	}
	return 0;
}

int Clear_WLAN_APPLY(unsigned char SecurityID){
	int i;
	for(i = 0; i < WLAN_NUM; i++){
		if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 1)&&(ASD_WLAN[i]->SecurityID == SecurityID)){
			ASD_WLAN[i]->SecurityID = 0;
			continue;
		}else
			continue;
	}
	ASD_WLAN_INF_OP(0, SecurityID, WID_DEL);
	return 0;
}
struct asd_stainfo* ASD_SEARCH_STA(unsigned char *sa){
	struct asd_stainfo * stainfo;
	struct sta_info *sta = NULL;
	sta = asd_sta_hash_get(sa);
	if(sta == NULL || sta->wasd == NULL)
		return NULL;
	asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SEARCH_STA\n");
	stainfo = (struct asd_stainfo *)os_zalloc(sizeof(struct asd_stainfo));
	if(stainfo==NULL){
		return NULL;  // 0608xm
	}
	stainfo->bss = sta->wasd;
	stainfo->sta = sta;
	return stainfo;
}
#if 0
struct asd_stainfo* ASD_SEARCH_STA(unsigned char *sa){
	struct asd_stainfo * stainfo;
	stainfo = (struct asd_stainfo *)os_zalloc(sizeof(struct asd_stainfo));
	if(stainfo==NULL){
		return NULL;  // 0608xm
	}
	stainfo->bss = NULL;
	stainfo->sta = NULL;
	int i = 0;	
	asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SEARCH_STA\n");
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
					continue;
				else {					
					stainfo->sta = ap_get_sta(interfaces->iface[i]->bss[j], sa);

					if(stainfo->sta == NULL)
						continue;
					else{
						stainfo->bss = interfaces->iface[i]->bss[j];
						return stainfo;
					}
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
		
	}
	
	stainfo->bss = NULL;
	stainfo->sta = NULL;
	free(stainfo);
	stainfo = NULL;
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SEARCH_STA end\n");
	return NULL;
}
#endif
///////////////////////////////////////////////////////////////////////////////
//xm 08/10/08
// Search all sta that access by wire.
int ASD_SEARCH_ALL_WIRED_STA(struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
//	printf("1\n");
	for(i = 0; i < port_max_num; i++){
		if(interfaces->iface_wired[i] != NULL){
			
//			printf("2\n");
			int j = 0 ;
			for(j = 0; j < vlan_max_num; j++){
//				printf("3\n");
				if(interfaces->iface_wired[i]->bss[j] == NULL)////////////?
					continue;
				else {
//					printf("we get bss\n");
					bss[num] = interfaces->iface_wired[i]->bss[j];
//					printf("*num++\n");
					num += 1;				
					continue;
				}
			}
		} 
	}

	//to improve effecincy ,we should create a list 
	
	return num;
}

int ASD_SEARCH_ALL_STA(struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
					continue;
				else {
					bss[num] = interfaces->iface[i]->bss[j];
					num += 1;				
					continue;
				}
			}
			i++;
		} else {
			i += 4 - i%L_RADIO_NUM;
		}
	}
	return num;
}
int ASD_GET_BSS_BYWLAN(struct asd_bss_summary_info *bss_summary,int id,int *circlenum)
{
	int total_bss_num = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0,j = 0;
	int wlanid = 0;
	struct bss_summary_info *bss_new = NULL;
	
	if(id){
		for(i=0;i<G_RADIO_NUM;i++){
			if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
				for(j=0;j<L_BSS_NUM;j++){
					if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID != id))
						continue;
					else{
						total_bss_num++;
						bss_summary[0].ID = id;
						bss_summary[0].local_bss_num++;
						bss_new = os_zalloc(sizeof(struct bss_summary_info));
						if(bss_new == NULL){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s malloc failed!\n",__func__);
							return -1;
						}
						memcpy(bss_new->BSSID,interfaces->iface[i]->bss[j]->own_addr,MAC_LEN);
						bss_new->sta_num = interfaces->iface[i]->bss[j]->num_sta;
						bss_new->RGID = i;
						bss_new->WLANID = id;
						bss_new->BSSINDEX = interfaces->iface[i]->bss[j]->BSSIndex;
						bss_new->next = bss_summary[0].bss_list;
						bss_summary[0].bss_list = bss_new;
					}
				}
			}
		}
		if(total_bss_num)
			*circlenum = 1;
	}
	else{
		for(i=0;i<G_RADIO_NUM;i++){
			if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
				for(j=0;j<L_BSS_NUM;j++){
					if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
						continue;
					else{
						total_bss_num++;
						wlanid = interfaces->iface[i]->bss[j]->WlanID;
						bss_summary[wlanid].ID = wlanid;
						bss_summary[wlanid].local_bss_num++;
						bss_new = os_zalloc(sizeof(struct bss_summary_info));
						if(bss_new == NULL){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s malloc failed!\n",__func__);
							return -1;
						}
						memcpy(bss_new->BSSID,interfaces->iface[i]->bss[j]->own_addr,MAC_LEN);
						bss_new->sta_num = interfaces->iface[i]->bss[j]->num_sta;
						bss_new->RGID = i;
						bss_new->WLANID = interfaces->iface[i]->bss[j]->WlanID;
						bss_new->BSSINDEX = interfaces->iface[i]->bss[j]->BSSIndex;
						bss_new->next = bss_summary[wlanid].bss_list;
						bss_summary[wlanid].bss_list = bss_new;
					}
				}
			}
		}
		for(i=0;i<WLAN_NUM;i++){
			if(bss_summary[i].local_bss_num != 0)
				(*circlenum)++;
		}
	}
	return total_bss_num;
}
int ASD_GET_BSS_BYWTP(struct asd_bss_summary_info *bss_summary,int id,int *circlenum)
{
	int total_bss_num = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0,j = 0;
	int wtpindex = 0;
	struct bss_summary_info *bss_new = NULL;

	
	if(id){
		for(i=id*L_RADIO_NUM;i<(id+1)*L_RADIO_NUM;i++){
			if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
				for(j=0;j<L_BSS_NUM;j++){
					if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
						continue;
					else{
						total_bss_num++;
						bss_summary[0].ID = id;
						bss_summary[0].local_bss_num++;
						bss_new = os_zalloc(sizeof(struct bss_summary_info));
						if(bss_new == NULL){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s malloc failed!\n",__func__);
							return -1;
						}
						memcpy(bss_new->BSSID,interfaces->iface[i]->bss[j]->own_addr,MAC_LEN);
						bss_new->sta_num = interfaces->iface[i]->bss[j]->num_sta;
						bss_new->RGID = i;
						bss_new->WLANID = interfaces->iface[i]->bss[j]->WlanID;
						bss_new->BSSINDEX = interfaces->iface[i]->bss[j]->BSSIndex;
						bss_new->next = bss_summary[0].bss_list;
						bss_summary[0].bss_list = bss_new;
					}
				}
			}
		}
		if(total_bss_num)
			*circlenum = 1;
	}
	else{
		for(i=0;i<G_RADIO_NUM;i++){
			if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
				for(j=0;j<L_BSS_NUM;j++){
					if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
						continue;
					else{
						total_bss_num++;
						wtpindex = i/4;
						bss_summary[wtpindex].ID = wtpindex;
						bss_summary[wtpindex].local_bss_num++;
						bss_new = os_zalloc(sizeof(struct bss_summary_info));
						if(bss_new == NULL){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s malloc failed!\n",__func__);
							return -1;
						}
						memcpy(bss_new->BSSID,interfaces->iface[i]->bss[j]->own_addr,MAC_LEN);
						bss_new->sta_num = interfaces->iface[i]->bss[j]->num_sta;
						bss_new->RGID = i;
						bss_new->WLANID = interfaces->iface[i]->bss[j]->WlanID;
						bss_new->BSSINDEX = interfaces->iface[i]->bss[j]->BSSIndex;
						bss_new->next = bss_summary[wtpindex].bss_list;
						bss_summary[wtpindex].bss_list = bss_new;
					}
				}
			}
		}
		for(i=0;i<WTP_NUM+1;i++){
			if(bss_summary[i].local_bss_num != 0)
				(*circlenum)++;
		}
	}
	return total_bss_num;
}
int ASD_GET_BSS_BYRADIO(struct asd_bss_summary_info *bss_summary,int id,int *circlenum)
{
	int total_bss_num = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int j = 0;
	struct bss_summary_info *bss_new = NULL;

	
	if(id){
		if((interfaces->iface[id] != NULL)&&(interfaces->iface[id]->bss != NULL)){
			for(j=0;j<L_BSS_NUM;j++){
				if((interfaces->iface[id]->bss[j] != NULL)&&(interfaces->iface[id]->bss[j]->WlanID != 0))
				{
					total_bss_num++;
					bss_summary[0].ID = id;
					bss_summary[0].local_bss_num++;
					bss_new = os_zalloc(sizeof(struct bss_summary_info));
					if(bss_new == NULL){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s malloc failed!\n",__func__);
						return -1;
					}
					memcpy(bss_new->BSSID,interfaces->iface[id]->bss[j]->own_addr,MAC_LEN);
					bss_new->sta_num = interfaces->iface[id]->bss[j]->num_sta;
					bss_new->RGID = id;
					bss_new->WLANID = interfaces->iface[id]->bss[j]->WlanID;
					bss_new->BSSINDEX = interfaces->iface[id]->bss[j]->BSSIndex;
					bss_new->next = bss_summary[0].bss_list;
					bss_summary[0].bss_list = bss_new;
				}
			}
		}
		if(total_bss_num)
			*circlenum = 1;
	}
	else 
		return -1;
	return total_bss_num;
}
int ASD_GET_CNUM(struct r_sta_wlan_info *r_sta_information,unsigned char type)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"********func:%s,type:%d********\n",__func__,type);
	int i = 0;
	int circlenum = 0;
	if(type == 1){
		for(i=0;i<WLAN_NUM;i++){
			if(r_sta_information[i].r_sta_list)
				circlenum++;
		}
	}else if(type == 2){
		for(i=0;i<3;i++){
			if(r_sta_information[i].r_sta_list)
				circlenum++;
		}
	}else if(type == 3){
		for(i=0;i<WTP_NUM+1;i++){
			if(r_sta_information[i].r_sta_list)
				circlenum++;
		}
	}else
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s,type:%d\n",__func__,type);
	return circlenum;	
}
int ASD_GET_R_STA_BYWLAN(struct r_sta_wlan_info *r_sta_wlan)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	unsigned char wlanid = 0;
	struct sta_info *tmp = NULL;
	struct r_sta_info *r_sta_new = NULL; 
	for(i=0;i<G_RADIO_NUM;i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j=0;j<L_BSS_NUM;j++){
			if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
				continue;
			else{
				wlanid = interfaces->iface[i]->bss[j]->WlanID;
				tmp = interfaces->iface[i]->bss[j]->sta_list;
				while(tmp != NULL){
					if(tmp->rflag != 0)
					{
						r_sta_new =(struct r_sta_info*)os_zalloc(sizeof(struct r_sta_info));
						r_sta_new->r_type = tmp->rflag;
						memcpy(r_sta_new->STA_MAC,tmp->addr,MAC_LEN);
						r_sta_new->preAPID = tmp->preAPID;
						r_sta_new->curAPID = interfaces->iface[i]->bss[j]->Radio_G_ID/4;
						r_sta_new->next = r_sta_wlan[wlanid].r_sta_list;
						r_sta_wlan[wlanid].r_sta_list = r_sta_new;
						r_sta_wlan[wlanid].roaming_sta_num++;
						num++;
					}
					tmp = tmp->next;
				}
			}
			}
		}
		
	}
	return num;
}
int ASD_GET_R_STA_BYWTP(struct r_sta_wlan_info *r_sta_wtp)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	unsigned char wtpid = 0;
	struct sta_info *tmp = NULL;
	struct r_sta_info *r_sta_new = NULL; 
	for(i=0;i<G_RADIO_NUM;i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			wtpid = i/4;
			for(j=0;j<L_BSS_NUM;j++){
			if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
				continue;
			else{
				tmp = interfaces->iface[i]->bss[j]->sta_list;
				while(tmp){
					if(tmp->rflag != 0)
					{
						r_sta_new =(struct r_sta_info*)os_zalloc(sizeof(struct r_sta_info));
						r_sta_new->r_type = tmp->rflag;
						memcpy(r_sta_new->STA_MAC,tmp->addr,MAC_LEN);
						r_sta_new->preAPID = tmp->preAPID;
						r_sta_new->curAPID = interfaces->iface[i]->bss[j]->Radio_G_ID/4;
						r_sta_new->next = r_sta_wtp[wtpid].r_sta_list;
						r_sta_wtp[wtpid].r_sta_list = r_sta_new;
						r_sta_wtp[wtpid].roaming_sta_num++;
						num++;
					}
					tmp = tmp->next;
				}
			}
			}
		}
	}
	return num;
}
int ASD_GET_R_STA_BYCLASS(struct r_sta_wlan_info *r_sta_class)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	unsigned char classtype = 0;
	struct sta_info *tmp = NULL;
	struct r_sta_info *r_sta_new = NULL; 
	for(i=0;i<G_RADIO_NUM;i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j=0;j<L_BSS_NUM;j++){
			if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
				continue;
			else{
				tmp = interfaces->iface[i]->bss[j]->sta_list;
				while(tmp){
					if(tmp->rflag != 0)
					{
						classtype = tmp->rflag;
						r_sta_new =(struct r_sta_info*)os_zalloc(sizeof(struct r_sta_info));
						r_sta_new->r_type = tmp->rflag;
						memcpy(r_sta_new->STA_MAC,tmp->addr,MAC_LEN);
						r_sta_new->preAPID = tmp->preAPID;
						r_sta_new->curAPID = interfaces->iface[i]->bss[j]->Radio_G_ID/4;
						r_sta_new->next = r_sta_class[classtype].r_sta_list;
						r_sta_class[classtype].r_sta_list = r_sta_new;
						r_sta_class[classtype].roaming_sta_num++;
						num++;
					}
					tmp = tmp->next;
				}
			}
			}
		}
	}
	return num;
}

//================================================================xm 08/10/28
int ASD_SEARCH_BSS_HAS_STA(struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else {
					if(interfaces->iface[i]->bss[j]->num_sta!=0){
						bss[num] = interfaces->iface[i]->bss[j];
						num += 1;	
					}			
					continue;
						
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
		
	}
	return num;
}

int ASD_SEARCH_ALL_BSS_NUM(struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else {
					if(interfaces->iface[i]->bss[j] != NULL){
						bss[num] = interfaces->iface[i]->bss[j];
						num += 1;	
					}			
					continue;
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
	}
	return num;
}

int ASD_SEARCH_WLAN_HAS_STA(unsigned char WlanID, struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++)
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else {					
					if(interfaces->iface[i]->bss[j]->WlanID == WlanID&&interfaces->iface[i]->bss[j]->num_sta!=0)
					{	
					bss[num] = interfaces->iface[i]->bss[j];
					num += 1;				
					}
					continue;
				}
			i++;	
		} else 
			i += 4 - i%L_RADIO_NUM;
	}
	return num;
}

int ASD_SEARCH_WTP_BSS(unsigned int WTPID, struct asd_data **bss)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0;
	int j = 0 ,num = 0;

	for(i = WTPID*L_RADIO_NUM; i < WTPID*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)) {
			for(j = 0; j < L_BSS_NUM; j++) {
				if((interfaces->iface[i]->bss[j] != NULL)&&(interfaces->iface[i]->bss[j]->WlanID != 0)) {
					bss[num] = interfaces->iface[i]->bss[j];
					num += 1;
				}
			}
		}
	}
	return num;
}


int ASD_SEARCH_WTP_HAS_STA(unsigned int WTPID, struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0;
	int j = 0 ,num = 0;

	for(i = WTPID*L_RADIO_NUM; i < WTPID*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)) {
			for(j = 0; j < L_BSS_NUM; j++) {
				if(interfaces->iface[i]->bss[j] != NULL) {
					bss[num] = interfaces->iface[i]->bss[j];
					num += 1;
				}
			}
		}
	}
	
	return num;
}

int ASD_SEARCH_WLAN_STA(unsigned char WlanID, struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else {					
					if(interfaces->iface[i]->bss[j]->WlanID == WlanID)
					{	
					bss[num] = interfaces->iface[i]->bss[j];
					num += 1;				
					}
					continue;
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
		
	}
	return num;
}

int ASD_SEARCH_BSS_BY_RADIO_BSS(unsigned int radioid, unsigned char bssid, struct asd_data **bss)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int bssindex = radioid*L_BSS_NUM + bssid - 1;
	
	if( (interfaces->iface[radioid] != NULL) 
		&& (interfaces->iface[radioid]->bss != NULL)
		&& (interfaces->iface[radioid]->bss[bssid-1] != NULL) 
		&& (interfaces->iface[radioid]->bss[bssid-1]->BSSIndex == bssindex)) {
		*bss = interfaces->iface[radioid]->bss[bssid-1];
		return 0;
	}	

	return -1;
}

//mahz add 2011.5.11
int ASD_SEARCH_BSS_BY_WLANID(unsigned int radioid, unsigned char WlanID, struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	
	if((interfaces->iface[radioid] != NULL)&&(interfaces->iface[radioid]->bss != NULL)){
		int j = 0 ;
		for(j = 0; j < L_BSS_NUM; j++){
			if(interfaces->iface[radioid]->bss[j] == NULL)
				continue;
			else {					
				if(interfaces->iface[radioid]->bss[j]->WlanID == WlanID)
				{	
					*bss = interfaces->iface[radioid]->bss[j];
					return 0;
				}
			}
		}
	} 
		
	return -1;
}
//

/*nl add 20100601*/
unsigned char FindWapiWlan(WID_WLAN ** WLAN,unsigned char *wlan_num)
{
	int j = 0;
	unsigned char wapi_wlan_num = 0;
	unsigned char security_id;
	*wlan_num = 0;
	
	for(j=0; j<WLAN_NUM;j++){
		if(ASD_WLAN[j]!= NULL){
			(*wlan_num)++ ;
			security_id = ASD_WLAN[j]->SecurityID;
			if(((security_id != 0) &&(ASD_SECURITY[security_id]!=NULL))&&
				((ASD_SECURITY[security_id]->securityType == WAPI_PSK)
				||(ASD_SECURITY[security_id]->securityType == WAPI_AUTH))){
				
				WLAN[wapi_wlan_num] = ASD_WLAN[j];
				wapi_wlan_num++;
			}
		}
	}
	return wapi_wlan_num;
	
}
/*nl add 20100607*/
unsigned int Asd_Find_Wtp(ASD_WTP_ST **WTP){
	int i = 0;
	unsigned int wtp_num = 0;
	while(i<WTP_NUM){
		if((ASD_WTP_AP[i] != NULL)
			&&(ASD_WTP_AP[i]->WTPID < WTP_NUM)
			&&ASD_WTP_AP[i]->WTPID > 0){
			WTP[wtp_num] = ASD_WTP_AP[i];
			wtp_num++;
		}
		i++;
	}
	return wtp_num;
}
#if 0
/*nl add 20100720*/
unsigned int ASD_Find_Running_Wtp(ASD_WTP_ST **WTP){
	int i = 0;
	unsigned int wtp_num = 0;
	while(i<WTP_NUM){
		if((ASD_WTP_AP[i] != NULL) 
			&&(ASD_WTP_AP[i]->WTPID < WTP_NUM)
			&&(ASD_WTP_AP[i]->state == 5)){
			WTP[wtp_num] = ASD_WTP_AP[i];
			wtp_num++;
		}
		i++;
	}
	return wtp_num;
}
#endif

/* for ID check by nl 2010-08-04*/
int ASD_CHECK_WTP_ID(u_int32_t Id){
	int ret = ASD_DBUS_SUCCESS;
	//struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	//int i = Id*L_RADIO_NUM;
	
	if(Id >= WTP_NUM)
		ret =  ASD_WTP_ID_LARGE_THAN_MAX;
	else if(ASD_WTP_AP[Id] == NULL)
		ret = ASD_WTP_NOT_EXIST;
	/*else if(interfaces->iface[i] == NULL)
		ret =  ASD_RADIO_NOT_EXIST;*/
	
	return ret ;
	
}

int ASD_CHECK_WLAN_ID(u_int32_t Id){
	int ret = ASD_DBUS_SUCCESS;
	
	if(Id >= WLAN_NUM)
		ret = ASD_WLAN_ID_LARGE_THAN_MAX;
	else if(ASD_WLAN[Id] == NULL)
		ret = ASD_WLAN_NOT_EXIST;
		
	return ret ;
	
}

int ASD_CHECK_G_RADIO_ID(u_int32_t Id){
	int ret = ASD_DBUS_SUCCESS;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;

	if(Id >= G_RADIO_NUM)
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
	else if(interfaces->iface[Id] == NULL)
		ret = ASD_RADIO_NOT_EXIST;
	
	return ret ;
	
}

int ASD_CHECK_SECURITY_ID(u_int32_t Id){
	int ret = ASD_DBUS_SUCCESS;
	
	if(Id >= WLAN_NUM)
		ret = ASD_SECURITY_LARGE_THAN_MAX;
	else if(ASD_SECURITY[Id] == NULL)
		ret = ASD_SECURITY_NOT_EXIST;
	
	return ret ;
	
}

int ASD_CHECK_BSS_ID(u_int32_t Id){
	int ret = ASD_DBUS_SUCCESS;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	//struct hostapd_data * hapd = NULL;
	struct asd_data *wasd = NULL;	
	unsigned int RadioID = Id/L_BSS_NUM;
	unsigned int BSS_L_ID = Id%L_BSS_NUM;
	
	if((interfaces->iface[RadioID] != NULL)
		&&(interfaces->iface[RadioID]->bss != NULL)
		&&(interfaces->iface[RadioID]->bss[BSS_L_ID] != NULL)){
		wasd = interfaces->iface[RadioID]->bss[BSS_L_ID];
		ret =  ASD_DBUS_SUCCESS;
	}
	else
		ret = ASD_BSS_NOT_EXIST;
	
	return ret ;
	
}

/* for ID check by nl  2010-08-04*/
int ASD_CHECK_ID(unsigned int TYPE,u_int32_t Id){
	int ret = 0;
	
	if(ASD_WTP_CHECK == TYPE)
		ret = ASD_CHECK_WTP_ID(Id);
	
	else if(ASD_WLAN_CHECK == TYPE)
		ret = ASD_CHECK_WLAN_ID(Id);/*remember changing type from char to int*/

	else if(ASD_RADIO_CHECK == TYPE)
		ret = ASD_CHECK_G_RADIO_ID(Id);

	else if(ASD_SECURITY_CHECK == TYPE)
		ret = ASD_CHECK_SECURITY_ID(Id);/*remember changing type from char to int*/

	else if(ASD_BSS_CHECK == TYPE)
		ret = ASD_CHECK_BSS_ID(Id);

	return ret ;

}

/*20100617 nl */
/*wlan[] is designed for avoiding repeatly record the situation that different radio of the same wtp bind the same wlan*/
unsigned int ASD_FIND_WLAN_BY_WTPID(unsigned int WTPID,unsigned int wlan[],WID_WLAN	**WLAN){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int i = 0, j = 0;
	unsigned int num = 0;
	unsigned char wlan_id;
	
	for(i = WTPID * 4; i<(WTPID+1)*4; ){
		if((interfaces->iface[i] != NULL)
			&&(interfaces->iface[i]->bss != NULL)){
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL){
					wlan_id = interfaces->iface[i]->bss[j]->WlanID;
					if((wlan_id<WLAN_NUM)
						&&(interfaces->iface[i]->bss[j]->Radio_G_ID/L_RADIO_NUM<WTP_NUM)){
						
						if((wlan[wlan_id]==0)
							&&(ASD_WLAN[wlan_id]!=NULL)
							&&(ASD_WLAN[wlan_id]->SecurityID!=0)
							&&(ASD_SECURITY[ASD_WLAN[wlan_id]->SecurityID]!=NULL)){
							
							wlan[interfaces->iface[i]->bss[j]->WlanID] = 1;
							WLAN[num] = ASD_WLAN[wlan_id];
							num++;
						}
						
						else{
						
						}
					}
				}
			}
			i++;
		} 
		else{
			break;
		}
	}
	return num;
}



int AsdCheckWTPID(unsigned int WTPID){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = WTPID*L_RADIO_NUM;
	if(interfaces->iface[i] == NULL)
		return 0;
	else 
		return 1;

}

int AsdCheckRadioID(unsigned int radioid){
//	xm0616
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;

	if(interfaces->iface[radioid] == NULL)
		return 0;
	else 
		return 1;
}

int update_sta_traffic_limit_info(struct asd_data *wasd,unsigned char include_vip){
/*xm0723*/
	struct sta_info *sta=NULL;
		
	if(wasd==NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s	wasd==NULL\n",__func__);
		return -1;
	}

	for(sta=wasd->sta_list;sta!=NULL;sta=sta->next){
		if(include_vip==1){
			sta->sta_traffic_limit=wasd->sta_average_traffic_limit;
			sta->sta_send_traffic_limit=wasd->sta_average_send_traffic_limit;
			if((sta->vip_flag & 0x03) != 0) {
				sta->vip_flag &= ~0x03;
				AsdStaInfoToWID(wasd, sta->addr, CANCEL_TRAFFIC_LIMIT);			//ht add 090305
			}
		}else{
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s sta "MACSTR" flag %d\n",__func__,MAC2STR(sta->addr),sta->vip_flag);
			if((sta->vip_flag & 0x03) == 0){
				sta->sta_traffic_limit=wasd->sta_average_traffic_limit;
				sta->sta_send_traffic_limit=wasd->sta_average_send_traffic_limit;
			}
		}
	}

	return 0;

}

/*fengwenchao add AXSSZFI-1374*/
int cancel_sta_traffic_limit_average_info(struct asd_data *wasd,unsigned char include_vip){
/*xm0723*/
	struct sta_info *sta=NULL;
		
	if(wasd==NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s	wasd==NULL\n",__func__);
		return -1;
	}

	for(sta=wasd->sta_list;sta!=NULL;sta=sta->next)
	{
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s sta "MACSTR" flag %d\n",__func__,MAC2STR(sta->addr),sta->vip_flag);
		if((sta->vip_flag & 0x03) == 0){
			sta->sta_traffic_limit=wasd->traffic_limit;
			sta->sta_send_traffic_limit=wasd->send_traffic_limit;
		}
	}
	return 0;
}

/*fengwenchao add end*/
struct asd_data * AsdCheckBSSIndex(unsigned int BSSIndex)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_data * wasd;
	if(BSSIndex>BSS_NUM)
	{
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s BSSIndex = %d is max than BSS_NUM(%d).\n",__func__,BSSIndex,BSS_NUM);
		return NULL;
	}
	unsigned int RadioID = BSSIndex/L_BSS_NUM;
	unsigned int BSS_L_ID = BSSIndex%L_BSS_NUM;
	if((interfaces->iface[RadioID] != NULL)&&(interfaces->iface[RadioID]->bss != NULL)&&(interfaces->iface[RadioID]->bss[BSS_L_ID] != NULL)){
		wasd = interfaces->iface[RadioID]->bss[BSS_L_ID];
		return wasd;
	}else
		return NULL;
}

int ASD_SEARCH_RADIO_STA(unsigned int RADIOID, struct asd_data **bss)
{
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int j = 0 ,num = 0;

	if((interfaces->iface[RADIOID] == NULL)||(interfaces->iface[RADIOID]->bss== NULL))
		return 0;
	for(j = 0; j < L_BSS_NUM; j++)
		if(interfaces->iface[RADIOID]->bss[j] == NULL)
			continue;
		else {
			bss[num] = interfaces->iface[RADIOID]->bss[j];
			num += 1;
		}	
	
	return num;
}


int ASD_SEARCH_WTP_STA(unsigned int WTPID, struct asd_data **bss){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0;	//WTPID*L_RADIO_NUM;
	int j = 0 ,num = 0;

	for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
		if((interfaces->iface[i] == NULL)||(interfaces->iface[i]->bss == NULL))
			continue;
		for(j = 0; j < L_BSS_NUM; j++)
			if((interfaces->iface[i]->bss[j] == NULL)||(interfaces->iface[i]->bss[j]->WlanID == 0))
				continue;
			else {
				bss[num] = interfaces->iface[i]->bss[j];
				num += 1;
			}	
	}
	
	return num;
}


int ASD_SEARCH_WTP_STA_NUM(unsigned int WTPID){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0;	//WTPID*L_RADIO_NUM;
	int j = 0 ,num = 0;

	for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
		if((interfaces->iface[i] == NULL)||(interfaces->iface[i]->bss == NULL))
			continue;
		for(j = 0; j < L_BSS_NUM; j++)
			if(interfaces->iface[i]->bss[j] == NULL)
				continue;
			else {
				if(interfaces->iface[i]->bss[j]->num_sta > 0){
					num += interfaces->iface[i]->bss[j]->num_sta;
				}
			}	
	}
	
	return num;
}


int ASD_ADD_SECURITY_PROFILE(char *name, unsigned char ID){

	ASD_SECURITY[ID] = (security_profile*)os_zalloc(sizeof(security_profile));
	if(ASD_SECURITY[ID]==NULL)
		return ASD_DBUS_SUCCESS;	//	0608xm
	ASD_SECURITY[ID]->SecurityID = ID;

	ASD_SECURITY[ID]->name = (char *)os_zalloc(strlen(name)+1);
	if(ASD_SECURITY[ID]->name!=NULL){
		memset(ASD_SECURITY[ID]->name, 0, strlen(name)+1);
		memcpy(ASD_SECURITY[ID]->name, name, strlen(name));
	}	//0608xm
	
	ASD_SECURITY[ID]->host_ip = (char *)os_zalloc(strlen("127.0.0.1")+1);
	if(ASD_SECURITY[ID]->host_ip!=NULL){
		memset(ASD_SECURITY[ID]->host_ip, 0, strlen("127.0.0.1")+1);
		memcpy(ASD_SECURITY[ID]->host_ip, "127.0.0.1", strlen("127.0.0.1"));
	}	//0608xm
	
	ASD_SECURITY[ID]->securityType = IEEE8021X;
	ASD_SECURITY[ID]->encryptionType = WEP;
	ASD_SECURITY[ID]->RadiusID = ID;
	ASD_SECURITY[ID]->extensible_auth = 0;
	ASD_SECURITY[ID]->wired_radius = 0;

	//mahz add 2010.12.9
	ASD_SECURITY[ID]->wapi_radius_auth = 0;
	ASD_SECURITY[ID]->user_passwd = NULL;
	ASD_SECURITY[ID]->hybrid_auth = 0;		//mahz add 2011.2.18
	ASD_SECURITY[ID]->fast_auth = 1;		//mahz add 2011.8.17
	ASD_SECURITY[ID]->mac_auth = 0;
	
	ASD_SECURITY[ID]->auth.auth_ip = NULL;
	ASD_SECURITY[ID]->eap_reauth_priod=3600;
	ASD_SECURITY[ID]->eap_alive_period=3600;	//weichao add 2011.09.22
	ASD_SECURITY[ID]->account_after_authorize =0;//weichao add
	ASD_SECURITY[ID]->account_after_dhcp =0;	//weichao add
	ASD_SECURITY[ID]->acct_interim_interval=0;
	ASD_SECURITY[ID]->quiet_Period=60;
	ASD_SECURITY[ID]->auth.auth_port = 1812;
	ASD_SECURITY[ID]->auth.auth_shared_secret = NULL;
	ASD_SECURITY[ID]->acct.acct_ip = NULL;
	ASD_SECURITY[ID]->acct.acct_port = 1813;
	ASD_SECURITY[ID]->acct.acct_shared_secret = NULL;	
	ASD_SECURITY[ID]->auth.secondary_auth_ip = NULL;
	ASD_SECURITY[ID]->auth.secondary_auth_port = 1812;
	ASD_SECURITY[ID]->auth.secondary_auth_shared_secret = NULL;
	ASD_SECURITY[ID]->acct.secondary_acct_ip = NULL;
	ASD_SECURITY[ID]->acct.secondary_acct_port = 1813;
	ASD_SECURITY[ID]->acct.secondary_acct_shared_secret = NULL;
	ASD_SECURITY[ID]->SecurityKey = NULL;
	ASD_SECURITY[ID]->keyLen= 0;
	ASD_SECURITY[ID]->pre_auth = 0;
	ASD_SECURITY[ID]->ap_max_inactivity = AP_MAX_INACTIVITY;
	
	ASD_SECURITY[ID]->wapi_as.as_ip= NULL;
	ASD_SECURITY[ID]->wapi_as.as_port= 3810;
	ASD_SECURITY[ID]->wapi_as.multi_cert= 0;
	ASD_SECURITY[ID]->wapi_as.certification_path= NULL;
	ASD_SECURITY[ID]->wapi_as.ae_cert_path= NULL;
	ASD_SECURITY[ID]->wapi_as.ca_cert_path= NULL;
	ASD_SECURITY[ID]->wapi_as.unite_cert_path= NULL;
	ASD_SECURITY[ID]->wapi_as.certification_type= 0;
	ASD_SECURITY[ID]->wapi_as.as_ip_len=0;
	ASD_SECURITY[ID]->wapi_as.certification_path_len=0;
	ASD_SECURITY[ID]->wapi_as.ae_cert_path_len=0;
	ASD_SECURITY[ID]->wapi_as.ca_cert_path_len=0;
	ASD_SECURITY[ID]->wapi_as.unite_cert_path_len = 0;	

	ASD_SECURITY[ID]->wapi_ucast_rekey_method=1;
	ASD_SECURITY[ID]->wapi_ucast_rekey_para_t=86400;
	ASD_SECURITY[ID]->wapi_ucast_rekey_para_p=100000;

	ASD_SECURITY[ID]->wapi_mcast_rekey_method=1;
	ASD_SECURITY[ID]->wapi_mcast_rekey_para_t=86400;
	ASD_SECURITY[ID]->wapi_mcast_rekey_para_p=100000;	//	xm0701

	ASD_SECURITY[ID]->wapi_config = NULL;	//	ht add,091030	
	ASD_SECURITY[ID]->index = 1;	//nl add,20100307
	ASD_SECURITY[ID]->wpa_group_rekey = 600;
	ASD_SECURITY[ID]->wpa_keyupdate_timeout = 1000;
	ASD_SECURITY[ID]->wpa_once_group_rekey_time = 6;
	ASD_SECURITY[ID]->traffic_limite_radius = 0;
	//qiuchen add it for master_bak radius server 2012.12.11
	ASD_SECURITY[ID]->ac_radius_name = NULL;
	ASD_SECURITY[ID]->radius_res_fail_percent = RADIUS_RES_FAIL_PERCENT;
	ASD_SECURITY[ID]->radius_res_suc_percent = RADIUS_RES_SUC_PERCENT;
	ASD_SECURITY[ID]->radius_access_test_interval = RADIUS_ACCESS_TEST_INTERVAL;
	ASD_SECURITY[ID]->radius_server_change_test_timer = RADIUS_SERVER_CHANGE_TIMER;
	ASD_SECURITY[ID]->radius_server_reuse_test_timer = RADIUS_SERVER_REUSE_TIMER;
	ASD_SECURITY[ID]->radius_heart_test_type = RADIUS_AUTH_TEST;
	ASD_SECURITY[ID]->radius_server_binding_type = RADIUS_SERVER_UNBINDED;
	ASD_SECURITY[ID]->heart_test_identifier_acct = 32768;
	ASD_SECURITY[ID]->heart_test_identifier_auth = 32768;
	ASD_SECURITY[ID]->heart_test_on = 0;
	//ASD_SECURITY[ID]->auth_server_last_use = RADIUS_DISABLE;
	//ASD_SECURITY[ID]->acct_server_last_use = RADIUS_DISABLE;
	ASD_SECURITY[ID]->acct_server_current_use = RADIUS_DISABLE;
	ASD_SECURITY[ID]->auth_server_current_use = RADIUS_DISABLE;
	ASD_SECURITY[ID]->radius_conf = NULL;
	ASD_SECURITY[ID]->radius_auth = RADIUS_AUTH_TEST;
	ASD_SECURITY[ID]->radius_acct = RADIUS_ACCT_TEST;
	ASD_SECURITY[ID]->radius_both = RADIUS_BOTH_TEST;
	//end 
	return ASD_DBUS_SUCCESS;
}

int ASD_DELETE_SECURITY_PROFILE(unsigned char ID){
	ASD_SECURITY[ID]->eap_reauth_priod=3600;	
	ASD_SECURITY[ID]->index=1;
	if(ASD_SECURITY[ID]->auth.auth_ip != NULL){
		free(ASD_SECURITY[ID]->auth.auth_ip);
		ASD_SECURITY[ID]->auth.auth_ip = NULL;
		free(ASD_SECURITY[ID]->auth.auth_shared_secret);
		ASD_SECURITY[ID]->auth.auth_shared_secret = NULL;
	}
	if(ASD_SECURITY[ID]->acct.acct_ip != NULL){
		asd_ip_secret_del(inet_addr(ASD_SECURITY[ID]->acct.acct_ip));
		free(ASD_SECURITY[ID]->acct.acct_ip);
		ASD_SECURITY[ID]->acct.acct_ip = NULL;
		free(ASD_SECURITY[ID]->acct.acct_shared_secret);
		ASD_SECURITY[ID]->acct.acct_shared_secret = NULL;
	}	
	if(ASD_SECURITY[ID]->auth.secondary_auth_ip != NULL){
		free(ASD_SECURITY[ID]->auth.secondary_auth_ip);
		ASD_SECURITY[ID]->auth.secondary_auth_ip = NULL;
		free(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret);
		ASD_SECURITY[ID]->auth.secondary_auth_shared_secret = NULL;
	}
	if(ASD_SECURITY[ID]->acct.secondary_acct_ip != NULL){
		asd_ip_secret_del(inet_addr(ASD_SECURITY[ID]->acct.secondary_acct_ip));
		free(ASD_SECURITY[ID]->acct.secondary_acct_ip);
		ASD_SECURITY[ID]->acct.secondary_acct_ip = NULL;
		free(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret);
		ASD_SECURITY[ID]->acct.secondary_acct_shared_secret = NULL;
	}
	if(ASD_SECURITY[ID]->hybrid_auth == 1)
	{	
		asd_ipset_switch--;
		if(asd_ipset_switch  == 0)
		{
			eap_clean_all_user(); 
			eap_auth_exit();
		}
		ASD_SECURITY[ID]->hybrid_auth = 0 ;
	}
	ASD_SECURITY[ID]->eap_alive_period = 3600;		//weichao add
	ASD_SECURITY[ID]->account_after_authorize =0;	//weichao add
	ASD_SECURITY[ID]->account_after_dhcp = 0;		//weichao add
	ASD_SECURITY[ID]->mac_auth = 0;				//weichao add
	ASD_SECURITY[ID]->wapi_radius_auth = 0;		//mahz add 2011.1.21
	ASD_SECURITY[ID]->hybrid_auth = 0;		//mahz add 2011.2.18
	ASD_SECURITY[ID]->traffic_limite_radius = 0;
	//mahz add 2010.12.10
	if(ASD_SECURITY[ID]->user_passwd != NULL){
		free(ASD_SECURITY[ID]->user_passwd);
		ASD_SECURITY[ID]->user_passwd = NULL;		
	}
	//
	if(ASD_SECURITY[ID]->wapi_as.as_ip!= NULL){
		free(ASD_SECURITY[ID]->wapi_as.as_ip);
		ASD_SECURITY[ID]->wapi_as.as_ip = NULL;
	}
	if(ASD_SECURITY[ID]->wapi_as.certification_path!= NULL){
		free(ASD_SECURITY[ID]->wapi_as.certification_path);
		ASD_SECURITY[ID]->wapi_as.certification_path = NULL;
	}
	if(ASD_SECURITY[ID]->wapi_as.ae_cert_path!= NULL){
		free(ASD_SECURITY[ID]->wapi_as.ae_cert_path);
		ASD_SECURITY[ID]->wapi_as.ae_cert_path = NULL;
	}
	if(ASD_SECURITY[ID]->wapi_as.ca_cert_path!= NULL){
		free(ASD_SECURITY[ID]->wapi_as.ca_cert_path);
		ASD_SECURITY[ID]->wapi_as.ca_cert_path = NULL;
	}
	
	if(ASD_SECURITY[ID]->wapi_as.unite_cert_path!= NULL){
		free(ASD_SECURITY[ID]->wapi_as.unite_cert_path);
		ASD_SECURITY[ID]->wapi_as.unite_cert_path_len=0;
		ASD_SECURITY[ID]->wapi_as.unite_cert_path= NULL;
	}

	if(ASD_SECURITY[ID]->SecurityKey!= NULL){
		free(ASD_SECURITY[ID]->SecurityKey);
		ASD_SECURITY[ID]->SecurityKey= NULL;
		ASD_SECURITY[ID]->keyLen= 0;
	}
	if(ASD_SECURITY[ID]->wapi_config != NULL){
		free(ASD_SECURITY[ID]->wapi_config);
		ASD_SECURITY[ID]->wapi_config = NULL;
	}
	
	//qiuchen add it for master_bak radius server 2012.12.11
	if(ASD_SECURITY[ID]->ac_radius_name != NULL){
		free(ASD_SECURITY[ID]->ac_radius_name);
		ASD_SECURITY[ID]->ac_radius_name = NULL;
	}
	if(ASD_SECURITY[ID]->radius_conf != NULL){
		asd_free_radius(ASD_SECURITY[ID]->radius_conf);
	}
	//end 
	free(ASD_SECURITY[ID]->name);
	ASD_SECURITY[ID]->name = NULL;
	free(ASD_SECURITY[ID]->host_ip);
	ASD_SECURITY[ID]->host_ip = NULL;
	free(ASD_SECURITY[ID]);
	ASD_SECURITY[ID] = NULL;	
	return ASD_DBUS_SUCCESS;
}

int ASD_SET_ACCT(unsigned char ID, unsigned int port, char *IP, char *secret){
	struct asd_ip_secret *secret_ip = NULL;
	if(ASD_SECURITY[ID]->acct.acct_ip != NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"ip:%d ,%s\n",inet_addr(ASD_SECURITY[ID]->acct.acct_ip),ASD_SECURITY[ID]->acct.acct_ip);
		asd_ip_secret_del(inet_addr(ASD_SECURITY[ID]->acct.acct_ip));
		free(ASD_SECURITY[ID]->acct.acct_ip);
		ASD_SECURITY[ID]->acct.acct_ip = NULL;
	}
	if(ASD_SECURITY[ID]->acct.acct_shared_secret){
		free(ASD_SECURITY[ID]->acct.acct_shared_secret);
		ASD_SECURITY[ID]->acct.acct_shared_secret = NULL;
	}
	ASD_SECURITY[ID]->acct.acct_port = port;

	ASD_SECURITY[ID]->acct.acct_ip = (char*)os_zalloc(strlen(IP)+1);
	if(ASD_SECURITY[ID]->acct.acct_ip!=NULL){
		memset(ASD_SECURITY[ID]->acct.acct_ip, 0, strlen(IP)+1);
		memcpy(ASD_SECURITY[ID]->acct.acct_ip, IP, strlen(IP));
		secret_ip = asd_ip_secret_add(inet_addr(ASD_SECURITY[ID]->acct.acct_ip));
	}	//	0608xm
	
	ASD_SECURITY[ID]->acct.acct_shared_secret = (char*)os_zalloc(strlen(secret)+1);
	if(ASD_SECURITY[ID]->acct.acct_shared_secret!=NULL){
		memset(ASD_SECURITY[ID]->acct.acct_shared_secret, 0, strlen(secret)+1);
		memcpy(ASD_SECURITY[ID]->acct.acct_shared_secret, secret, strlen(secret));
		if(secret_ip)
		{
			secret_ip->shared_secret = (u8 *)ASD_SECURITY[ID]->acct.acct_shared_secret;
			secret_ip->shared_secret_len = strlen(ASD_SECURITY[ID]->acct.acct_shared_secret);
		}
	}	//	0608xm
	if(secret_ip != NULL)
	{
		if(secret_ip->slot_value !=ASD_SECURITY[ID]->slot_value )
			secret_ip->slot_value = ASD_SECURITY[ID]->slot_value;
		if(secret_ip->inst_value !=ASD_SECURITY[ID]->inst_value )
			secret_ip->inst_value = ASD_SECURITY[ID]->inst_value;
	}
	return ASD_DBUS_SUCCESS;
}

int ASD_SET_WAPI_AUTH(unsigned char ID, unsigned int type, char *IP/*, char *path*/){
	if(ASD_SECURITY[ID]->wapi_as.as_ip!= NULL){
		free(ASD_SECURITY[ID]->wapi_as.as_ip);
		ASD_SECURITY[ID]->wapi_as.as_ip = NULL;
	}
	//if(ASD_SECURITY[ID]->wapi_as.certification_path!=NULL){
		//free(ASD_SECURITY[ID]->wapi_as.certification_path);
		//ASD_SECURITY[ID]->wapi_as.certification_path = NULL;
	//}
	ASD_SECURITY[ID]->wapi_as.as_port= 3810;

	ASD_SECURITY[ID]->wapi_as.as_ip= (char*)os_zalloc(strlen(IP)+1);
	if(ASD_SECURITY[ID]->wapi_as.as_ip!=NULL){
		memset(ASD_SECURITY[ID]->wapi_as.as_ip, 0, strlen(IP)+1);
		memcpy(ASD_SECURITY[ID]->wapi_as.as_ip, IP, strlen(IP));
		ASD_SECURITY[ID]->wapi_as.as_ip_len=strlen(IP);
	}	//	0608xm
	
	//ASD_SECURITY[ID]->wapi_as.certification_path= (char*)malloc(strlen(path)+1);
	//memset(ASD_SECURITY[ID]->wapi_as.certification_path, 0, strlen(path)+1);
	//memcpy(ASD_SECURITY[ID]->wapi_as.certification_path, path, strlen(path));
	//ASD_SECURITY[ID]->wapi_as.certification_path_len=strlen(path);
	
	if(type==0)
		ASD_SECURITY[ID]->wapi_as.certification_type=WAPI_X509;
	else if(type==1)
		ASD_SECURITY[ID]->wapi_as.certification_type=WAPI_GBW;

	asd_printf(ASD_DBUS,MSG_DEBUG,"scry id %d\n",ID);
	asd_printf(ASD_DBUS,MSG_DEBUG,"port %d\n",ASD_SECURITY[ID]->wapi_as.as_port);
	asd_printf(ASD_DBUS,MSG_DEBUG,"ip %s\n",ASD_SECURITY[ID]->wapi_as.as_ip);
	asd_printf(ASD_DBUS,MSG_DEBUG,"scry cert path %s\n",ASD_SECURITY[ID]->wapi_as.certification_path);
	asd_printf(ASD_DBUS,MSG_DEBUG,"scry cert tpye %d\n",ASD_SECURITY[ID]->wapi_as.certification_type);
	return ASD_DBUS_SUCCESS;
}

/*weichao add 20110801*/
int ASD_DEL_WAPI_CERT(unsigned char ID, unsigned int path_type){

	if(path_type==1){
		if((ASD_SECURITY[ID]->wapi_as.certification_path!=NULL)){
			free(ASD_SECURITY[ID]->wapi_as.certification_path);
			ASD_SECURITY[ID]->wapi_as.certification_path = NULL;
			ASD_SECURITY[ID]->wapi_as.certification_path_len = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"scry id %d\n",ID);
			asd_printf(ASD_DBUS,MSG_DEBUG,"delete scry AS cert path %s\n",ASD_SECURITY[ID]->wapi_as.certification_path);
		}
	}
	
	else if(path_type==2){
		if((ASD_SECURITY[ID]->wapi_as.ae_cert_path!=NULL)){
			free(ASD_SECURITY[ID]->wapi_as.ae_cert_path);
			ASD_SECURITY[ID]->wapi_as.ae_cert_path = NULL;
			ASD_SECURITY[ID]->wapi_as.ae_cert_path_len = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"scry id %d\n",ID);
			asd_printf(ASD_DBUS,MSG_DEBUG,"delete scry AE cert path %s\n",ASD_SECURITY[ID]->wapi_as.ae_cert_path);
		}
	}
	
	else if(path_type==3){
		if(ASD_SECURITY[ID]->wapi_as.multi_cert == 0) {
			return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		}
		else{
			if((ASD_SECURITY[ID]->wapi_as.ca_cert_path!=NULL)){
				free(ASD_SECURITY[ID]->wapi_as.ca_cert_path);
				ASD_SECURITY[ID]->wapi_as.ca_cert_path = NULL;
				ASD_SECURITY[ID]->wapi_as.ca_cert_path_len = 0;
				asd_printf(ASD_WAPI,MSG_DEBUG,"scry id %d\n",ID);
				asd_printf(ASD_WAPI,MSG_DEBUG,"delete scry CA cert path %s\n",ASD_SECURITY[ID]->wapi_as.ca_cert_path);
			}
		}
	}
	
	return ASD_DBUS_SUCCESS;
}

int ASD_SET_WAPI_CERT_PATH(unsigned char ID, unsigned int path_type,  char *path){

	if(path_type==1){
		if((ASD_SECURITY[ID]->wapi_as.certification_path!=NULL)){
			free(ASD_SECURITY[ID]->wapi_as.certification_path);
			ASD_SECURITY[ID]->wapi_as.certification_path = NULL;
		}
		
		ASD_SECURITY[ID]->wapi_as.certification_path= (char*)os_zalloc(strlen(path)+1);
		if(ASD_SECURITY[ID]->wapi_as.certification_path!=NULL){
			memset(ASD_SECURITY[ID]->wapi_as.certification_path, 0, strlen(path)+1);
			memcpy(ASD_SECURITY[ID]->wapi_as.certification_path, path, strlen(path));
			ASD_SECURITY[ID]->wapi_as.certification_path_len=strlen(path);
		}	//	0608xm
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"scry id %d\n",ID);
		asd_printf(ASD_DBUS,MSG_DEBUG,"scry AS cert path %s\n",ASD_SECURITY[ID]->wapi_as.certification_path);
	}
	else if(path_type==2){
		if((ASD_SECURITY[ID]->wapi_as.ae_cert_path!=NULL)){
			free(ASD_SECURITY[ID]->wapi_as.ae_cert_path);
			ASD_SECURITY[ID]->wapi_as.ae_cert_path = NULL;
		}
		
		ASD_SECURITY[ID]->wapi_as.ae_cert_path= (char*)os_zalloc(strlen(path)+1);
		if(ASD_SECURITY[ID]->wapi_as.ae_cert_path!=NULL){
			memset(ASD_SECURITY[ID]->wapi_as.ae_cert_path, 0, strlen(path)+1);
			memcpy(ASD_SECURITY[ID]->wapi_as.ae_cert_path, path, strlen(path));
			ASD_SECURITY[ID]->wapi_as.ae_cert_path_len=strlen(path);
		}	//	0608xm
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"scry id %d\n",ID);
		asd_printf(ASD_DBUS,MSG_DEBUG,"scry AE cert path %s\n",ASD_SECURITY[ID]->wapi_as.ae_cert_path);
	}
	else if(path_type==3){
		if(ASD_SECURITY[ID]->wapi_as.multi_cert == 0) {
			return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		}else{
			if((ASD_SECURITY[ID]->wapi_as.ca_cert_path!=NULL)){
				free(ASD_SECURITY[ID]->wapi_as.ca_cert_path);
				ASD_SECURITY[ID]->wapi_as.ca_cert_path = NULL;
			}
			
			ASD_SECURITY[ID]->wapi_as.ca_cert_path= (char*)os_zalloc(strlen(path)+1);
			if(ASD_SECURITY[ID]->wapi_as.ca_cert_path!=NULL){
				memset(ASD_SECURITY[ID]->wapi_as.ca_cert_path, 0, strlen(path)+1);
				memcpy(ASD_SECURITY[ID]->wapi_as.ca_cert_path, path, strlen(path));
				ASD_SECURITY[ID]->wapi_as.ca_cert_path_len=strlen(path);
			}	
		
			asd_printf(ASD_WAPI,MSG_DEBUG,"scry id %d\n",ID);
			asd_printf(ASD_WAPI,MSG_DEBUG,"scry CA cert path %s\n",ASD_SECURITY[ID]->wapi_as.ca_cert_path);
		}
	}
	if((ASD_SECURITY[ID]->wapi_as.ae_cert_path!=NULL)&&(ASD_SECURITY[ID]->wapi_as.certification_path!=NULL)){
		char buf1[128];
		char buf2[1024];
		//char buf3[1024];
		if(ASD_SECURITY[ID]->wapi_as.unite_cert_path == NULL){
			memset(buf1,0,128);
			
#ifndef _DISTRIBUTION_
			sprintf(buf1,"/mnt/wtp/user%d_%d.cer",vrrid,ID);		
#else
			sprintf(buf1,"/mnt/wtp/user%d_%d_%d.cer",local,vrrid,ID);		
#endif
			ASD_SECURITY[ID]->wapi_as.unite_cert_path= (char*)os_zalloc(strlen(buf1)+1);
			if(ASD_SECURITY[ID]->wapi_as.unite_cert_path!=NULL){
				memset(ASD_SECURITY[ID]->wapi_as.unite_cert_path, 0, strlen(buf1)+1);
				memcpy(ASD_SECURITY[ID]->wapi_as.unite_cert_path, buf1, strlen(buf1));
				ASD_SECURITY[ID]->wapi_as.unite_cert_path_len=strlen(buf1);
			}	//	0608xm
		}
		memset(buf2,0,1024);
		sprintf(buf2,"/usr/bin/cert_unite.sh %s %s %s\n",ASD_SECURITY[ID]->wapi_as.certification_path,ASD_SECURITY[ID]->wapi_as.ae_cert_path,ASD_SECURITY[ID]->wapi_as.unite_cert_path);				
		system(buf2);
//		memset(buf3,0,1024);
//		sprintf(buf3,"sudo chmod 777 %s\n",ASD_SECURITY[ID]->wapi_as.unite_cert_path);				
//		system(buf3);		
	}
	return ASD_DBUS_SUCCESS;
}



int ASD_SET_AUTH(unsigned char ID, unsigned int port, char *IP, char *secret){
	if(ASD_SECURITY[ID]->auth.auth_ip != NULL){
		free(ASD_SECURITY[ID]->auth.auth_ip);
		ASD_SECURITY[ID]->auth.auth_ip = NULL;
	}
	if(ASD_SECURITY[ID]->auth.auth_shared_secret != NULL){
		free(ASD_SECURITY[ID]->auth.auth_shared_secret);
		ASD_SECURITY[ID]->auth.auth_shared_secret = NULL;
	}
	ASD_SECURITY[ID]->auth.auth_port = port;

	ASD_SECURITY[ID]->auth.auth_ip = (char*)os_zalloc(strlen(IP)+1);
	if(ASD_SECURITY[ID]->auth.auth_ip!=NULL){
		memset(ASD_SECURITY[ID]->auth.auth_ip, 0, strlen(IP)+1);
		memcpy(ASD_SECURITY[ID]->auth.auth_ip, IP, strlen(IP));
	}	//	0608xm

	ASD_SECURITY[ID]->auth.auth_shared_secret = (char*)os_zalloc(strlen(secret)+1);
	if(ASD_SECURITY[ID]->auth.auth_shared_secret!=NULL){
		memset(ASD_SECURITY[ID]->auth.auth_shared_secret, 0, strlen(secret)+1);
		memcpy(ASD_SECURITY[ID]->auth.auth_shared_secret, secret, strlen(secret));
	}	//	0608xm
	return ASD_DBUS_SUCCESS;
}

/***********************************************************************/
//xm 08/09/02
int ASD_SECONDARY_SET_ACCT(unsigned char ID, unsigned int port, char *IP, char *secret){
	struct asd_ip_secret *secret_ip = NULL;
	if(ASD_SECURITY[ID]->acct.secondary_acct_ip!= NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"ip:%d ,%s\n",inet_addr(ASD_SECURITY[ID]->acct.secondary_acct_ip),ASD_SECURITY[ID]->acct.secondary_acct_ip);
		asd_ip_secret_del(inet_addr(ASD_SECURITY[ID]->acct.secondary_acct_ip));
		free(ASD_SECURITY[ID]->acct.secondary_acct_ip);
		ASD_SECURITY[ID]->acct.secondary_acct_ip= NULL;
	}
	if(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret!= NULL){
		free(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret);
		ASD_SECURITY[ID]->acct.secondary_acct_shared_secret= NULL;
	}
	ASD_SECURITY[ID]->acct.secondary_acct_port= port;
	
	ASD_SECURITY[ID]->acct.secondary_acct_ip= (char*)os_zalloc(strlen(IP)+1);
	if(ASD_SECURITY[ID]->acct.secondary_acct_ip!=NULL){
		memset(ASD_SECURITY[ID]->acct.secondary_acct_ip, 0, strlen(IP)+1);
		memcpy(ASD_SECURITY[ID]->acct.secondary_acct_ip, IP, strlen(IP));
		secret_ip = asd_ip_secret_add(inet_addr(ASD_SECURITY[ID]->acct.secondary_acct_ip));
	}	//	0608xm
	
	ASD_SECURITY[ID]->acct.secondary_acct_shared_secret= (char*)os_zalloc(strlen(secret)+1);
	if(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret!=NULL){
		memset(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret, 0, strlen(secret)+1);
		memcpy(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret, secret, strlen(secret));
		if(secret_ip)
		{
			secret_ip->shared_secret = (u8 *)ASD_SECURITY[ID]->acct.secondary_acct_shared_secret;
			secret_ip->shared_secret_len = strlen(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret);
		}
	}	//	0608xm	
	if(secret_ip != NULL)
	{
		if(secret_ip->slot_value !=ASD_SECURITY[ID]->slot_value )
			secret_ip->slot_value = ASD_SECURITY[ID]->slot_value;
		if(secret_ip->inst_value !=ASD_SECURITY[ID]->inst_value )
			secret_ip->inst_value = ASD_SECURITY[ID]->inst_value;
	}
	return ASD_DBUS_SUCCESS;
}



int ASD_SECONDARY_SET_AUTH(unsigned char ID, unsigned int port, char *IP, char *secret){
	if(ASD_SECURITY[ID]->auth.secondary_auth_ip!= NULL){
		free(ASD_SECURITY[ID]->auth.secondary_auth_ip);
		ASD_SECURITY[ID]->auth.secondary_auth_ip= NULL;
	}
	if(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret!= NULL){
		free(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret);
		ASD_SECURITY[ID]->auth.secondary_auth_shared_secret= NULL;
	}
	ASD_SECURITY[ID]->auth.secondary_auth_port= port;
	
	ASD_SECURITY[ID]->auth.secondary_auth_ip= (char*)os_zalloc(strlen(IP)+1);
	if(ASD_SECURITY[ID]->auth.secondary_auth_ip!=NULL){
		memset(ASD_SECURITY[ID]->auth.secondary_auth_ip, 0, strlen(IP)+1);
		memcpy(ASD_SECURITY[ID]->auth.secondary_auth_ip, IP, strlen(IP));
	}	//	0608xm
	
	ASD_SECURITY[ID]->auth.secondary_auth_shared_secret= (char*)os_zalloc(strlen(secret)+1);
	if(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret!=NULL){
		memset(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret, 0, strlen(secret)+1);
		memcpy(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret, secret, strlen(secret));
	}	//	0608xm
	
	return ASD_DBUS_SUCCESS;
}
//xm 08/09/02
/***********************************************************************/
//qiuchen
int ASD_SEC_RADIUS_CHECK(unsigned char ID){
	if(((ASD_SECURITY[ID]->securityType== IEEE8021X)&&(ASD_SECURITY[ID]->encryptionType == WEP))||(((ASD_SECURITY[ID]->securityType == WPA_E)||(ASD_SECURITY[ID]->securityType == WPA2_E)))||(ASD_SECURITY[ID]->securityType == MD5)||(ASD_SECURITY[ID]->extensible_auth == 1)||(ASD_SECURITY[ID]->wapi_radius_auth == 1)){
		if((ASD_SECURITY[ID]->auth.auth_ip == NULL)||(ASD_SECURITY[ID]->acct.acct_ip == NULL)||(ASD_SECURITY[ID]->auth.auth_shared_secret == NULL)||(ASD_SECURITY[ID]->acct.acct_shared_secret == NULL)||((ASD_SECURITY[ID]->wapi_radius_auth == 1)&&(ASD_SECURITY[ID]->user_passwd == NULL)))
			return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		if((ASD_SECURITY[ID]->auth.secondary_auth_ip == NULL) && (ASD_SECURITY[ID]->acct.secondary_acct_ip == NULL))
			return ASD_SECURITY_NONEED_RADIUS_TEST;
		if(ASD_SECURITY[ID]->radius_server_binding_type == RADIUS_SERVER_BINDED){
			if((ASD_SECURITY[ID]->acct.secondary_acct_ip == NULL)||(ASD_SECURITY[ID]->auth.secondary_auth_ip == NULL) || 
				(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret == NULL) || (ASD_SECURITY[ID]->acct.secondary_acct_shared_secret == NULL))
				return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		}
		else{
			if(((ASD_SECURITY[ID]->radius_heart_test_type == RADIUS_AUTH_TEST)&&((ASD_SECURITY[ID]->auth.secondary_auth_ip == NULL)||(ASD_SECURITY[ID]->auth.secondary_auth_shared_secret == NULL)))
				||((ASD_SECURITY[ID]->radius_heart_test_type == RADIUS_ACCT_TEST)&&((ASD_SECURITY[ID]->acct.secondary_acct_ip == NULL)||(ASD_SECURITY[ID]->acct.secondary_acct_shared_secret == NULL))))
				return ASD_SECURITY_HEART_TEST_TYPE_WRONG;
		}
		if(ASD_SECURITY[ID]->radius_server_change_test_timer/ASD_SECURITY[ID]->radius_access_test_interval < TEST_WINDOW_LEAST_SIZE ||
			ASD_SECURITY[ID]->radius_server_reuse_test_timer/ASD_SECURITY[ID]->radius_access_test_interval < TEST_WINDOW_LEAST_SIZE)
			return ASD_SECURIT_HEART_PARAMETER_WRONG;
		return ASD_DBUS_SUCCESS;
	}
	else
		return ASD_SECURITY_TYPE_WITHOUT_8021X;
}
int ASD_SECURITY_PROFILE_CHECK(unsigned char ID){
	unsigned int security_type;
	unsigned int encryption_type;
	unsigned int state;
	if(ASD_SECURITY[ID] == NULL)
		return ASD_SECURITY_NOT_EXIST;
	security_type = ASD_SECURITY[ID]->securityType;
	encryption_type = ASD_SECURITY[ID]->encryptionType;
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[ID]->securityType %d",ASD_SECURITY[ID]->securityType);
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[ID]->encryptionType %d",ASD_SECURITY[ID]->encryptionType);


	state = ASD_SECURITY[ID]->extensible_auth;
	if(((security_type == OPEN)&&((encryption_type == NONE)||(encryption_type == WEP)))
		||((security_type == SHARED)&&(encryption_type == WEP))
		||((security_type == IEEE8021X)&&(encryption_type == WEP))
		||(((security_type == WPA_P)||(security_type == WPA_E)||(security_type == WPA2_P)||(security_type == WPA2_E))&&((encryption_type == AES)||(encryption_type == TKIP)))
		||((security_type == MD5)&&(encryption_type == NONE))
		||(((security_type == WAPI_PSK)||(security_type == WAPI_AUTH))&&(encryption_type == SMS4))){
		if(((security_type == IEEE8021X)&&(encryption_type == WEP))||(((security_type == WPA_E)||(security_type == WPA2_E)))||(security_type == MD5)||(ASD_SECURITY[ID]->extensible_auth == 1)||(ASD_SECURITY[ID]->wapi_radius_auth == 1)){
			if((ASD_SECURITY[ID]->auth.auth_ip == NULL)||(ASD_SECURITY[ID]->acct.acct_ip == NULL)||(ASD_SECURITY[ID]->auth.auth_shared_secret == NULL)||(ASD_SECURITY[ID]->acct.acct_shared_secret == NULL)||((ASD_SECURITY[ID]->wapi_radius_auth == 1)&&(ASD_SECURITY[ID]->user_passwd == NULL)))
				return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
			//qiuchen add it for master_bak radius server 2012.12.11
			if(ASD_SECURITY[ID]->radius_server_binding_type == RADIUS_SERVER_BINDED){
				if(((ASD_SECURITY[ID]->auth.auth_ip != NULL)&&(ASD_SECURITY[ID]->acct.acct_ip == NULL))||
					((ASD_SECURITY[ID]->auth.auth_ip == NULL)&&(ASD_SECURITY[ID]->acct.acct_ip != NULL)))
					return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
			}
			else{
				if(((ASD_SECURITY[ID]->auth.secondary_auth_ip!= NULL)||(ASD_SECURITY[ID]->acct.secondary_acct_ip!= NULL))&& ASD_SECURITY[ID]->heart_test_on == 0)
					return ASD_SECURITY_HEART_TEST_ON;	
			}
			//end
			if((ASD_SECURITY[ID]->distribute_off == 0)&&((ASD_SECURITY[ID]->slot_value == 0) ||(ASD_SECURITY[ID]->inst_value == 0)))
				return ASD_SECURITY_RDC_NOT_EXIT;
		}
		if((security_type == WAPI_AUTH)){
			if((ASD_SECURITY[ID]->wapi_as.as_ip == NULL)||(ASD_SECURITY[ID]->wapi_as.certification_path == NULL)||(ASD_SECURITY[ID]->wapi_as.ae_cert_path == NULL))
				return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		}
		if((((security_type == OPEN)&&(encryption_type == WEP)))||(security_type == SHARED)||(security_type ==WPA_P)||(security_type ==WPA2_P)||(security_type == WAPI_PSK)){
			if(ASD_SECURITY[ID]->SecurityKey == NULL)
				return ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		}
			
	}else
		return ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
	return ASD_DBUS_SUCCESS;
}

int ASD_WLAN_INF_OP(unsigned char WLANID, unsigned int SecurityID, Operate op){
//===========================================================
asd_printf(ASD_DBUS,MSG_DEBUG," send to wid type %d  ====0/1/2/add/del/modify\n", op);
//===========================================================
	TableMsg WLAN;
	int len;
	WLAN.Op = op;
	WLAN.Type = WLAN_TYPE;
	WLAN.u.WLAN.WlanID = WLANID;
	WLAN.u.WLAN.SecurityType = ASD_SECURITY[SecurityID]->securityType;
	WLAN.u.WLAN.EncryptionType = ASD_SECURITY[SecurityID]->encryptionType;
	WLAN.u.WLAN.PreAuth = ASD_SECURITY[SecurityID]->pre_auth;
	if((WLANID != 0)&&(op == WID_MODIFY))  //fengwenchao add 20110314
	{
		WLAN.u.WLAN.WlanKey.OldSecurityIndex = ASD_WLAN[WLANID]->OldSecurityIndex ;  //fengwenchao add 20110309
		WLAN.u.WLAN.WlanKey.OldSecurityIndex_flag  = ASD_WLAN[WLANID]->OldSecurityIndex_flag;    //fengwenchao add 20110310
		WLAN.u.WLAN.ap_max_inactivity = ASD_WLAN[WLANID]->ap_max_inactivity;
		//asd_printf(ASD_DBUS,MSG_INFO,"ASD_WLAN[%d]->OldSecurityIndex = %d\n",WLANID,ASD_WLAN[WLANID]->OldSecurityIndex);	
	}
	if(((WLAN.u.WLAN.SecurityType == OPEN)&&(WLAN.u.WLAN.EncryptionType == WEP))
		||(WLAN.u.WLAN.SecurityType == SHARED)||(WLAN.u.WLAN.SecurityType == WAPI_PSK))
	{	
		if(ASD_SECURITY[SecurityID]->SecurityKey!=NULL){
			memset(WLAN.u.WLAN.WlanKey.key, 0, DEFAULT_LEN);
			memcpy(WLAN.u.WLAN.WlanKey.key, ASD_SECURITY[SecurityID]->SecurityKey, ASD_SECURITY[SecurityID]->keyLen);
			WLAN.u.WLAN.WlanKey.key_len = ASD_SECURITY[SecurityID]->keyLen;

			WLAN.u.WLAN.ascii_hex=ASD_SECURITY[SecurityID]->keyInputType;		
			WLAN.u.WLAN.WlanKey.SecurityIndex = ASD_SECURITY[SecurityID]->index;
		}
	}else if (WLAN.u.WLAN.SecurityType== WAPI_AUTH){

		if(ASD_SECURITY[SecurityID]->wapi_as.as_ip!=NULL){
			memset(WLAN.u.WLAN.as_ip, 0, DEFAULT_LEN);
			memcpy(WLAN.u.WLAN.as_ip, ASD_SECURITY[SecurityID]->wapi_as.as_ip, ASD_SECURITY[SecurityID]->wapi_as.as_ip_len);
			WLAN.u.WLAN.as_ip_len=ASD_SECURITY[SecurityID]->wapi_as.as_ip_len;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.certification_path!=NULL){
			memset(WLAN.u.WLAN.cert_path, 0, DEFAULT_LEN);
			memcpy(WLAN.u.WLAN.cert_path, ASD_SECURITY[SecurityID]->wapi_as.certification_path, ASD_SECURITY[SecurityID]->wapi_as.certification_path_len);
			WLAN.u.WLAN.cert_path_len=ASD_SECURITY[SecurityID]->wapi_as.certification_path_len;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path!=NULL){
			memset(WLAN.u.WLAN.ae_cert_path, 0, DEFAULT_LEN);
			memcpy(WLAN.u.WLAN.ae_cert_path, ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path, ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path_len);
			WLAN.u.WLAN.ae_cert_path_len=ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path_len;
		}
	}
	WLAN.u.WLAN.SecurityID = SecurityID;
	len = sizeof(WLAN);
//=========================================================
if((ASD_SECURITY[SecurityID]->SecurityKey!=NULL)
	&&(ASD_SECURITY[SecurityID]->wapi_as.as_ip!=NULL)
	&&(ASD_SECURITY[SecurityID]->wapi_as.certification_path!=NULL)
	&&(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path!=NULL)){
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->securityType:  %d\n", ASD_SECURITY[SecurityID]->securityType);
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->encryptionType:  %d\n", ASD_SECURITY[SecurityID]->encryptionType);

asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->SecurityKey:  %s\n", ASD_SECURITY[SecurityID]->SecurityKey);

asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->keyLen:  %d\n", ASD_SECURITY[SecurityID]->keyLen);
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->keyInputType:  %d\n", ASD_SECURITY[SecurityID]->keyInputType);

asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->wapi_as.as_ip:  %s\n", ASD_SECURITY[SecurityID]->wapi_as.as_ip);
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->wapi_as.as_ip_len:  %d\n", ASD_SECURITY[SecurityID]->wapi_as.as_ip_len);

asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->wapi_as.certification_path:  %s\n", ASD_SECURITY[SecurityID]->wapi_as.certification_path);
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->wapi_as.certification_path_len:  %d\n", ASD_SECURITY[SecurityID]->wapi_as.certification_path_len);

asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path:  %s\n", ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path);
asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path_len:  %d\n", ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path_len);

}
//=========================================================
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_WLAN_INF_OP1\n");
	if(sendto(TableSend, &WLAN, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
//		close(sock);
		asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_WLAN_INF_OP2\n");
		return 0;
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_WLAN_INF_OP3\n");
//	close(sock);
	return 1;

}

/*nl add for wapi 09/11/02*/
void SECURITY_WAPI_INIT(unsigned char SecurityID){

	unsigned char MultiCipher[4]={0x00,0x14,0x72,0x01};
	ASD_SECURITY[SecurityID]->wapi_config=(struct wapi_sub_security *)os_zalloc(sizeof(struct wapi_sub_security ));
	
	if(ASD_SECURITY[SecurityID]->wapi_config == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	
	ASD_SECURITY[SecurityID]->wapi_config->WapiPreauth=0;			/*default 0,no*/
	ASD_SECURITY[SecurityID]->wapi_config->MulticaseRekeyStrict=0 ;  /*default 0,no*/
	ASD_SECURITY[SecurityID]->wapi_config->CertificateUpdateCount=3;	/*default 3*/
	ASD_SECURITY[SecurityID]->wapi_config->MulticastUpdateCount=3;	/*default 3*/
	ASD_SECURITY[SecurityID]->wapi_config->UnicastUpdateCount=3;	/*default 3*/
	ASD_SECURITY[SecurityID]->wapi_config->BKLifetime=43200;	/*default 43200*/
	ASD_SECURITY[SecurityID]->wapi_config->BKReauthThreshold=70;	/*default 70*/
	ASD_SECURITY[SecurityID]->wapi_config->SATimeout=60;	/*default 60*/
	ASD_SECURITY[SecurityID]->wapi_config->UnicastCipherEnabled=1;			/*default 1,yes*/
	ASD_SECURITY[SecurityID]->wapi_config->AuthenticationSuiteEnable=1;   /*default 1,yes*/
	memcpy(ASD_SECURITY[SecurityID]->wapi_config->MulticastCipher,MultiCipher,4);


}


void SECURITY_ENCYPTION_MATCH(unsigned char SecurityID, unsigned int security_type){
	switch(security_type){
		case OPEN:
			ASD_SECURITY[SecurityID]->encryptionType = NONE;
			break;
		case SHARED:
			ASD_SECURITY[SecurityID]->encryptionType = WEP;
			break;
		case IEEE8021X:
			ASD_SECURITY[SecurityID]->encryptionType = WEP;
			break;
		case WPA_P:			
			ASD_SECURITY[SecurityID]->encryptionType = TKIP;
			break;
		case WPA_E:
			ASD_SECURITY[SecurityID]->encryptionType = TKIP;			
			break;
		case WPA2_P:			
			ASD_SECURITY[SecurityID]->encryptionType = AES;
			break;
		case WPA2_E:
			ASD_SECURITY[SecurityID]->encryptionType = AES;			
			break;		
		case MD5:
			ASD_SECURITY[SecurityID]->encryptionType = NONE;
			break;
		case WAPI_PSK:
			ASD_SECURITY[SecurityID]->encryptionType = SMS4;
			break;
		case WAPI_AUTH:
			ASD_SECURITY[SecurityID]->encryptionType = SMS4;
			break;
	}

}

void ASD_WIRED_DEFAULT_INIT(struct asd_data * wasd){
	
	int i;
	const int aCWmin = 15, aCWmax = 1024;
	const struct asd_wme_ac_params ac_bk =
		{ aCWmin, aCWmax, 7, 0, 0 }; /* background traffic */
	const struct asd_wme_ac_params ac_be =
		{ aCWmin, aCWmax, 3, 0, 0 }; /* best effort traffic */
	const struct asd_wme_ac_params ac_vi = /* video traffic */
		{ aCWmin >> 1, aCWmin, 2, 3000 / 32, 1 };
	const struct asd_wme_ac_params ac_vo = /* voice traffic */
		{ aCWmin >> 2, aCWmin >> 1, 2, 1500 / 32, 1 };
	wasd->iconf = os_zalloc(sizeof(struct asd_config));
	if(wasd->iconf == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	wasd->conf = os_zalloc(sizeof(struct asd_bss_config));
	if(wasd->conf == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	wasd->conf->radius = os_zalloc(sizeof(struct asd_radius_servers));
	if(wasd->conf->radius == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	
	asd_config_defaults_bss(wasd->conf,wasd->SecurityID);
	wasd->default_wep_key_idx = 1;
	wasd->iconf->num_bss = 1;
	wasd->iconf->bss = wasd->conf;

	wasd->iconf->beacon_int = 100;
	wasd->iconf->rts_threshold = -1; /* use driver default: 2347 */
	wasd->iconf->fragm_threshold = -1; /* user driver default: 2346 */
	wasd->iconf->send_probe_response = 0;
	wasd->iconf->bridge_packets = INTERNAL_BRIDGE_DO_NOT_CONTROL;

	wasd->num_sta = 0;
	
	os_memcpy(wasd->iconf->country, "US ", 3);

	for (i = 0; i < NUM_TX_QUEUES; i++)
		wasd->iconf->tx_queue[i].aifs = -1; /* use hw default */

	wasd->iconf->wme_ac_params[0] = ac_be;
	wasd->iconf->wme_ac_params[1] = ac_bk;
	wasd->iconf->wme_ac_params[2] = ac_vi;
	wasd->iconf->wme_ac_params[3] = ac_vo;

	return;
}

int ASD_WIRED_INIT(struct asd_data * wasd, unsigned char securityID){
	unsigned char SID;
	int pairwise = 0;
	struct asd_bss_config *bss;
	ASD_WIRED_DEFAULT_INIT(wasd);
	bss = wasd->conf;
	SID = securityID;
	wasd->SecurityID = securityID;
	if(ASD_SECURITY[SID] == NULL)
		return -1;
	
	if((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)){
		bss->ieee802_1x = 1;	
		bss->wpa = 0;
		bss->default_wep_key_len = 0;
		bss->individual_wep_key_len = 0;
		bss->wep_rekeying_period = 0;
		bss->eapol_key_index_workaround = 0;
		if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &bss->own_ip_addr)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				  SID,ASD_SECURITY[SID]->host_ip);
		}
		bss->nas_identifier = ASD_SECURITY[SID]->host_ip;

	if(ASD_SECURITY[SID]->host_ip != NULL) 	{			//ht add,081110
		if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &bss->radius->client_addr) == 0) {
			bss->radius->force_client_addr = 1;
		}else {
			asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",SID,ASD_SECURITY[SID]->host_ip);
			return -1;
		}
	}
		
		if(ASD_SECURITY[SID]->auth.auth_ip == NULL)
			return -1;
		if (asd_config_read_radius_addr(
				&bss->radius->auth_servers,
				&bss->radius->num_auth_servers, ASD_SECURITY[SID]->auth.auth_ip, 1812,
				&bss->radius->auth_server)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->auth.auth_ip);
		}
		
		bss->radius->auth_server->port = ASD_SECURITY[SID]->auth.auth_port;
		
		int len = os_strlen(ASD_SECURITY[SID]->auth.auth_shared_secret);
		if (len == 0) {
			/* RFC 2865, Ch. 3 */
			asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		bss->radius->auth_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->auth.auth_shared_secret);
		bss->radius->auth_server->shared_secret_len = len;

		if(ASD_SECURITY[SID]->acct.acct_ip == NULL)
			return -1;
		
		if (asd_config_read_radius_addr(
				&bss->radius->acct_servers,
				&bss->radius->num_acct_servers, ASD_SECURITY[SID]->acct.acct_ip, 1813,
				&bss->radius->acct_server)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->acct.acct_ip);
		}
		
		bss->radius->acct_server->port = ASD_SECURITY[SID]->acct.acct_port;
		len = os_strlen(ASD_SECURITY[SID]->acct.acct_shared_secret);
		if (len == 0) {
			/* RFC 2865, Ch. 3 */
			asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		bss->radius->acct_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->acct.acct_shared_secret);
		bss->radius->acct_server->shared_secret_len = len;
		
		if(ASD_SECURITY[SID]->auth.secondary_auth_ip != NULL){
			if (asd_config_read_radius_addr(
					&bss->radius->auth_servers,
					&bss->radius->num_auth_servers, ASD_SECURITY[SID]->auth.secondary_auth_ip, 1812,
					&bss->radius->auth_server)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
					   SID, ASD_SECURITY[SID]->auth.secondary_auth_ip);
				
			}
			
			bss->radius->auth_server->port = ASD_SECURITY[SID]->auth.secondary_auth_port;
			
			int len = os_strlen(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
			if (len == 0) {
				/* RFC 2865, Ch. 3 */
				asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: empty shared secret is not "
					   "allowed.\n", SID);
			}
			bss->radius->auth_server->shared_secret =
				(u8 *) os_strdup(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
			bss->radius->auth_server->shared_secret_len = len;
		}
		
		if(ASD_SECURITY[SID]->acct.secondary_acct_ip != NULL){
			if (asd_config_read_radius_addr(
					&bss->radius->acct_servers,
					&bss->radius->num_acct_servers, ASD_SECURITY[SID]->acct.secondary_acct_ip, 1813,
					&bss->radius->acct_server)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
					   SID, ASD_SECURITY[SID]->acct.secondary_acct_ip);
			}

			bss->radius->acct_server->port = ASD_SECURITY[SID]->acct.secondary_acct_port;
			len = os_strlen(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
			if (len == 0) {
				/* RFC 2865, Ch. 3 */
				asd_printf(ASD_DBUS,MSG_DEBUG,"Security %d: empty shared secret is not "
					   "allowed.\n", SID);
			}
			bss->radius->acct_server->shared_secret =
				(u8 *) os_strdup(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
			bss->radius->acct_server->shared_secret_len = len;
		}
		bss->radius->auth_server = bss->radius->auth_servers;
		bss->radius->acct_server = bss->radius->acct_servers;
	}

	if (bss->wpa & 1)
		pairwise |= bss->wpa_pairwise;
	if (bss->wpa & 2) {
		if (bss->rsn_pairwise == 0)
			bss->rsn_pairwise = bss->wpa_pairwise;
		pairwise |= bss->rsn_pairwise;
	}
	if (pairwise & WPA_CIPHER_TKIP)
		bss->wpa_group = WPA_CIPHER_TKIP;
	else
		bss->wpa_group = WPA_CIPHER_CCMP;
	
	if (bss->wpa && bss->ieee802_1x) {
		bss->ssid.security_policy = SECURITY_WPA;
	} else if (bss->wpa) {
		bss->ssid.security_policy = SECURITY_WPA_PSK;
	} else if (bss->ieee802_1x) {
		bss->ssid.security_policy = SECURITY_IEEE_802_1X;
		bss->ssid.wep.default_len = bss->default_wep_key_len;
	} else if (bss->ssid.wep.keys_set)
		bss->ssid.security_policy = SECURITY_STATIC_WEP;
	else
		bss->ssid.security_policy = SECURITY_PLAINTEXT;
	return 0;
}
int ASD_WIRED_SECURITY_INIT(struct asd_data * wasd){
	if (wpa_debug_level == MSG_MSGDUMP)
		wasd->conf->radius->msg_dumps = 1;
	if(ASD_WLAN[wasd->WlanID])
		ASD_WLAN[wasd->WlanID]->radius = radius_client_init(ASD_WLAN[wasd->WlanID], ASD_WLAN[wasd->WlanID]->radius_server);
	if (wasd->radius == NULL) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"RADIUS client initialization failed.\n");
		return -1;
	}

	if (asd_acl_init(wasd)) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"ACL initialization failed.\n");
		return -1;
	}

	if (ieee802_1x_init(wasd)) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"IEEE 802.1X initialization failed.\n");
		return -1;
	}
	if (accounting_init(wasd->WlanID)) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"Accounting initialization failed.\n");
		return -1;
	}
	return 0;
}

/*kick the  online sta ,because it is not allowed to access */
int bss_kick_sta(struct asd_data *bss, struct sta_info *sta)
{
#ifdef ASD_USE_PERBSS_LOCK
	pthread_mutex_lock(&(bss->asd_sta_mutex));   
#else
	pthread_mutex_lock(&asd_g_sta_mutex);
#endif

	TableMsg STA;
	int len;
	unsigned char SID = (unsigned char)bss->SecurityID;
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",__func__);
	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
	if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
		wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
		mlme_deauthenticate_indication(bss, sta, 0);
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	}

	ieee802_11_send_deauth(bss, sta->addr, 3);
	
	STA.Op = WID_DEL;
	STA.Type = STA_TYPE;
	STA.u.STA.BSSIndex = bss->BSSIndex;
	STA.u.STA.WTPID = ((bss->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC, sta->addr, ETH_ALEN);
	len = sizeof(STA);
	asd_printf(ASD_DBUS,MSG_DEBUG,"bss_kick_sta1\n");
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		asd_printf(ASD_DBUS,MSG_DEBUG,"bss_kick_sta2\n");
//		close(sock);
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"bss_kick_sta3\n");
//	close(sock);

	if(ASD_NOTICE_STA_INFO_TO_PORTAL)
		AsdStaInfoToEAG(bss,sta,WID_DEL);
	if(ASD_WLAN[bss->WlanID]!=NULL&&ASD_WLAN[bss->WlanID]->balance_switch==1&&ASD_WLAN[bss->WlanID]->balance_method==1){
		ap_free_sta(bss, sta, 1);
	}else{
		ap_free_sta(bss, sta, 0);
	}
#ifdef ASD_USE_PERBSS_LOCK
	pthread_mutex_unlock(&(bss->asd_sta_mutex));   
#else
	pthread_mutex_unlock(&asd_g_sta_mutex);   
#endif
	return 0;
}


int add_ASD_BSS_only_acl_conf(unsigned int bssindex)
{
	if((ASD_BSS[bssindex]!=NULL) && (ASD_BSS[bssindex]->acl_conf != NULL))
		return 0;

	if(ASD_BSS[bssindex] == NULL) {
		ASD_BSS[bssindex] = (WID_BSS*)os_zalloc(sizeof(WID_BSS));
		if(ASD_BSS[bssindex] == NULL)
			return -1;
		os_memset(ASD_BSS[bssindex],0,sizeof(WID_BSS));
	}
	
	ASD_BSS[bssindex]->BSSIndex = bssindex;
	ASD_BSS[bssindex]->Radio_G_ID = bssindex/L_BSS_NUM;
	ASD_BSS[bssindex]->acl_conf = (struct acl_config *)os_zalloc(sizeof(struct acl_config));
	if(ASD_BSS[bssindex]->acl_conf == NULL)
			return -1;
	
	os_memset(ASD_BSS[bssindex]->acl_conf,0,sizeof(struct acl_config));
	ASD_BSS[bssindex]->acl_conf->macaddr_acl = 0;
	ASD_BSS[bssindex]->acl_conf->accept_mac = NULL;
	ASD_BSS[bssindex]->acl_conf->num_accept_mac = 0;
	ASD_BSS[bssindex]->acl_conf->deny_mac = NULL;
	ASD_BSS[bssindex]->acl_conf->num_deny_mac = 0;

	return 0;
}


int Create_WIRED_node(int VLANID, int PORTID, unsigned char securityID){	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i;
	struct asd_data *wasd;
	struct asd_data *wasd_config = interfaces->config_wired->bss[0];
	if(interfaces->iface_wired[PORTID] == NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"interfaces->iface setup and init\n");
		interfaces->iface_wired[PORTID] = (struct asd_iface*)os_zalloc(sizeof(struct asd_iface));
		interfaces->iface_wired[PORTID]->interfaces = interfaces;
		interfaces->iface_wired[PORTID]->num_bss = 0;
		interfaces->iface_wired[PORTID]->bss = os_zalloc(vlan_max_num * sizeof(struct asd_data*));
		if(interfaces->iface_wired[PORTID]->bss == NULL ){
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			exit(1);
		}
		for(i=0;i<vlan_max_num;i++)
			interfaces->iface_wired[PORTID]->bss[i] = NULL;
		if(asd_hw_feature_init(interfaces->iface_wired[PORTID]) < 0)
			asd_printf(ASD_DBUS,MSG_DEBUG,"asd_hw_feature_init failed\n");
	}
	if(interfaces->iface_wired[PORTID]->bss[VLANID] == NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"bss setup and init\n");
		wasd = interfaces->iface_wired[PORTID]->bss[VLANID] = os_zalloc(sizeof(struct asd_data));			
		if(wasd == NULL ){
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			exit(1);
		}
		interfaces->iface_wired[PORTID]->num_bss++;
		//zhanglei  need get AC MAC
		memcpy(wasd->own_addr,wasd_config->own_addr,6);

		wasd->iface = interfaces->iface_wired[PORTID];
		wasd->VLANID = VLANID;
		wasd->PORTID = PORTID;
		wasd->info = os_zalloc(sizeof(struct stat_info));	//ht add 090223
		if(wasd->info == NULL ){
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			exit(1);
		}
		wasd->driver = interfaces->config_wired->bss[0]->driver;
		wasd->drv_priv = interfaces->config_wired->bss[0]->drv_priv;
		if(ASD_WIRED_INIT(wasd, securityID) < 0){
			asd_printf(ASD_DBUS,MSG_DEBUG,"BSS init failed\n");
			if(wasd->conf->radius->acct_server != NULL){
				free(wasd->conf->radius->acct_server);
				wasd->conf->radius->acct_server = NULL;
			}
			if(wasd->conf->radius->auth_server != NULL){
				free(wasd->conf->radius->auth_server);
				wasd->conf->radius->auth_server = NULL;
			}
			if(wasd->info != NULL) {
				free(wasd->info);
				wasd->info = NULL;
			}
			free(wasd->conf->radius);
			wasd->conf->radius = NULL;
			free(wasd->conf);
			wasd->conf = NULL;
			free(wasd->iconf);
			wasd->iconf = NULL;
			wasd->driver = NULL;
			wasd->drv_priv = NULL;
			free(wasd);
			wasd = NULL;
			interfaces->iface_wired[PORTID]->bss[VLANID] = NULL;
			return 0;
		}
		else if ((wasd->conf->ieee802_1x == 1)&&(ASD_WIRED_SECURITY_INIT(wasd) <0) ){
			asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_BSS_SECURITY_INIT failed\n");
			return 0;
		}
			
		asd_printf(ASD_DBUS,MSG_DEBUG,"%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x\n",wasd->BSSIndex,wasd->WlanID,wasd->Radio_G_ID,wasd->Radio_L_ID,
			wasd->own_addr[0],wasd->own_addr[1],wasd->own_addr[2],wasd->own_addr[3],wasd->own_addr[4],wasd->own_addr[5]);
		return 0;
			
	}else{
		asd_printf(ASD_DBUS,MSG_DEBUG,"interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] != NULL\n");
		return 0;
	}
}

int ASDReInit(){	
	//struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int count;	
	unsigned int wtp,radio,bss;
	unsigned int flag = 0;	/*xiaodawei add, 20101115*/
	int i, newlicensecount = 0;
	char WTP_COUNT_PATH_BASE[] = "/devinfo/maxwtpcount";
	char buf_base[DEFAULT_LEN];
	char strdir[DEFAULT_LEN];

	/*
	char WTP_COUNT_PATH[] = "/devinfo/maxwtpcount";
	char buf[DEFAULT_LEN];
	memset(buf,0,DEFAULT_LEN);
	char WTP_COUNT_PATH_OEM[] = "/devinfo/maxwtpcount2";
	char buf_oem[DEFAULT_LEN];
	memset(buf_oem,0,DEFAULT_LEN);	
	wtp = WTP_NUM;
	radio = G_RADIO_NUM;
	bss = BSS_NUM;
	if(read_ac_info(WTP_COUNT_PATH,buf) == 0){
		if(parse_int_ID(buf, &count)==-1)
			return ASD_DBUS_ERROR;
		else{
			if(count < WTP_NUM_AUTELAN)
				return ASD_DBUS_ERROR;
			else{
				WTP_NUM_AUTELAN = count;
			}
		}
	}else
		return ASD_DBUS_ERROR;
	
	printf("WTP_NUM_AUTELAN %d\n",WTP_NUM_AUTELAN);
	
	if(read_ac_info(WTP_COUNT_PATH_OEM,buf_oem) == 0){
		if(parse_int_ID(buf_oem, &count_oem)==-1)
			return ASD_DBUS_ERROR;
		else{
			if(count < WTP_NUM_OEM)
				return ASD_DBUS_ERROR;
			else{
				WTP_NUM_OEM = count_oem;
			}
		}
	}else
		return ASD_DBUS_ERROR;
	printf("WTP_NUM_OEM %d\n",WTP_NUM_OEM); 

	WTP_NUM = WTP_NUM_AUTELAN + WTP_NUM_OEM;

	
	printf("num= %d,autelannum= %d oemnum = %d\n",WTP_NUM,WTP_NUM_AUTELAN,WTP_NUM_OEM); 
	*/
	wtp = WTP_NUM;
	radio = G_RADIO_NUM;
	bss = BSS_NUM;

	newlicensecount = get_dir_wild_file_count("/devinfo","maxwtpcount");
	printf("newlicensecount %d\n",newlicensecount);
	if((newlicensecount >= glicensecount)){
		{
			/*xiaodawei modify, 20101029*/
			g_wtp_count = realloc(g_wtp_count,newlicensecount*(sizeof(LICENSE_TYPE *)));
			
			for(i=0; i<newlicensecount; i++)
			{
				if(i >= glicensecount){
					g_wtp_count[i] = malloc(sizeof(LICENSE_TYPE));
					g_wtp_count[i]->gmax_wtp_count = 0;
					g_wtp_count[i]->gcurrent_wtp_count = 0;
					g_wtp_count[i]->flag = 0;
				}
				memset(strdir,0,DEFAULT_LEN);
				memset(buf_base,0,DEFAULT_LEN);	

				if(i == 0)
				{
					if(read_ac_info(WTP_COUNT_PATH_BASE,buf_base) == 0)
					{
						if(parse_int_ID(buf_base, &count)==-1)
						{
							return ASD_NEED_REBOOT;
						}
						if(count < g_wtp_count[i]->gmax_wtp_count)
						{
							return ASD_NEED_REBOOT;
						}
						else
						{
							flag = g_wtp_count[i]->flag;
							if(flag!=0)
							{
								g_wtp_binding_count[flag]->gmax_wtp_count -= g_wtp_count[i]->gmax_wtp_count;
								g_wtp_count[i]->gmax_wtp_count = count;
								g_wtp_binding_count[flag]->gmax_wtp_count += g_wtp_count[i]->gmax_wtp_count;
							}
							else
							{
								g_wtp_count[i]->gmax_wtp_count = count;
							}
						}
					}
					else
					{

						return ASD_NEED_REBOOT;
					}
		
				}
				else
				{
					sprintf(strdir,"/devinfo/maxwtpcount%d",i+1);
					if(read_ac_info(strdir,buf_base) == 0)
					{
						if(parse_int_ID(buf_base, &count)==-1)
						{
							return ASD_NEED_REBOOT;
						}
						if(count < g_wtp_count[i]->gmax_wtp_count)
						{
							return ASD_NEED_REBOOT;
						}
						else
						{
							flag = g_wtp_count[i]->flag;
							if(flag!=0)
							{
								g_wtp_binding_count[flag]->gmax_wtp_count -= g_wtp_count[i]->gmax_wtp_count;
								g_wtp_count[i]->gmax_wtp_count = count;
								g_wtp_binding_count[flag]->gmax_wtp_count += g_wtp_count[i]->gmax_wtp_count;
							}
							else
							{
								g_wtp_count[i]->gmax_wtp_count = count;
							}
						}
					}
					else
					{
						return ASD_NEED_REBOOT;
					}
				}

				printf("asd################ maxwtp[%d] = %d\n",i,g_wtp_count[i]->gmax_wtp_count);
			}			
			#if 0
			WTP_NUM = 0;
			for(i=0; i<newlicensecount; i++)
			{
				WTP_NUM += g_wtp_count[i]->gmax_wtp_count;
			}
			#endif
		}
		glicensecount = newlicensecount;
	}else{
		return ASD_NEED_REBOOT;
	}
#if 0
	asd_printf(ASD_DBUS,MSG_DEBUG,"WTP_NUM %d\n",WTP_NUM);
	WTP_NUM += 1;
	G_RADIO_NUM = WTP_NUM*L_RADIO_NUM;
	BSS_NUM = G_RADIO_NUM*L_BSS_NUM;
	ASD_WTP = realloc(ASD_WTP,WTP_NUM*(sizeof(WID_WTP *)));	
	for(i=wtp;i<WTP_NUM;i++)
		ASD_WTP[i] = NULL;
	ASD_RADIO = realloc(ASD_RADIO,G_RADIO_NUM*(sizeof(WID_WTP_RADIO *)));	
	for(i=radio;i<G_RADIO_NUM;i++)
		ASD_RADIO[i] = NULL;
	ASD_BSS = realloc(ASD_BSS,BSS_NUM*(sizeof(WID_BSS *)));
	for(i=bss;i<BSS_NUM;i++)
		ASD_BSS[i] = NULL;
	ASD_WTP_AP = realloc(ASD_WTP_AP,WTP_NUM*(sizeof(ASD_WTP_ST *)));
	for(i=wtp;i<WTP_NUM;i++)
		ASD_WTP_AP[i] = NULL;	
	interfaces->count = G_RADIO_NUM;
	interfaces->iface = realloc(interfaces->iface,interfaces->count *
				     sizeof(struct asd_iface *));
	for(i=radio;i<G_RADIO_NUM;i++)
		interfaces->iface[i] = NULL;
#endif	
	return ASD_DBUS_SUCCESS;
}

/* zhangshu copy from 1.2, 2010-09-13 */
//zhaoruijia,判断当前的radio是否绑定当前的wtp
int ASD_IS_WTP_BIND_RADIO(unsigned int WTPID,unsigned int RADIOID)
{
    int i=0;
    int j=0;
    int radionum = 0;
    unsigned int radio[L_RADIO_NUM] = {0};
    for(i = (WTPID)*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
    	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
    	if((interfaces->iface[i] == NULL)||(interfaces->iface[i]->bss == NULL))
    		continue;
    	else {
    		radio[radionum]=i;
    		radionum++;
    	}

    	/*g_radio_id = wtpid *L_RADIO_NUM + l_radio_id*/
    	printf("ASD_IS_WTP_BIND_RADIO\n");
    }
    for(j = 0; j < radionum; j++) {
    
        if(radio[j] != RADIOID){
           continue;
        }
        else{
          return 1;
        }    
    }
	return 0;	

}


//zhaoruijia,判断当前wlanid是否与当前的radio绑定


int ASD_IS_RADIO_BIND_WLAN(unsigned int RADIOID,unsigned char WlanID,int bsscount)
{
   printf("@@@ASD_IS_RADIO_BIND_WLAN@@@\n");
   struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
   if((interfaces->iface[RADIOID] == NULL)||(interfaces->iface[RADIOID]->bss== NULL))
		return 0;
   
    if(interfaces->iface[RADIOID]->bss[bsscount]->WlanID == WlanID){
      
		   printf("@@@11111111111111111111@@@@@@@@\n");
		   return 1;
	    }
	else{
		 printf("@@@22222222222222222222@@@@@@@@\n");
           return 0;
	    }
   	
}

//mahz add 2010.12.9
int ASD_WAPI_RADIUS_AUTH_SET_USER_PASSWD(unsigned char ID, char *passwd){
	
	if(ASD_SECURITY[ID]->user_passwd){
		free(ASD_SECURITY[ID]->user_passwd);
		ASD_SECURITY[ID]->user_passwd = NULL;
	}
	
	ASD_SECURITY[ID]->user_passwd = (char*)os_zalloc(strlen(passwd)+1);
	if(ASD_SECURITY[ID]->user_passwd!=NULL){
		memset(ASD_SECURITY[ID]->user_passwd, 0, strlen(passwd)+1);
		memcpy(ASD_SECURITY[ID]->user_passwd, passwd, strlen(passwd));
	}	
	return ASD_DBUS_SUCCESS;
}
struct asd_ip_secret * asd_ip_secret_get(const u32 ip)
{
	struct asd_ip_secret *asd_secret = NULL;
	asd_secret = ASD_SECRET_HASH[SECRET_IP_HASH(ip)];	
	while(asd_secret!=NULL &&asd_secret->ip != ip)
		asd_secret = asd_secret->next;
	return asd_secret;
}
struct asd_ip_secret *asd_ip_secret_add(u32 ip)
{
	struct asd_ip_secret *asd_secret = NULL;
	asd_secret = asd_ip_secret_get(ip);
	if(NULL != asd_secret)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"got the ip%d , asd_secret ip:%d\n",ip,asd_secret->ip);
		return asd_secret;
	}
	asd_secret = os_malloc(sizeof(struct asd_ip_secret));
	if(NULL == asd_secret)
	{
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s: malloc error!\n",__func__);
		return NULL;
	}
	asd_secret->ip = ip;
	asd_secret->next = ASD_SECRET_HASH[SECRET_IP_HASH(ip)];
	ASD_SECRET_HASH[SECRET_IP_HASH(ip)] = asd_secret;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s add successfully!\n",__func__);
	return asd_secret;
}
void asd_ip_secret_del(u32 ip)
{
	
	struct asd_ip_secret *s = NULL;
	struct asd_ip_secret *tmp = NULL;
	s = ASD_SECRET_HASH[SECRET_IP_HASH(ip)];
	if (s == NULL) return;
	if (s->ip == ip) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ok!ip%d\n",ip);
		tmp = s;
		ASD_SECRET_HASH[SECRET_IP_HASH(ip)] = s->next;
		goto out;	
	}

	while (s->next != NULL &&s->next->ip == ip)
		s = s->next;
	if (s->next != NULL){
		tmp = s->next;
		s->next = s->next->next;
	}
	else
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR, "asd_ip_secret: could not remove IP %d\n",ip);
		return;
	}
out:
	if(tmp)
	{
		tmp->next = NULL;
		tmp->shared_secret = NULL;
		free(tmp);
		tmp = NULL;
	}
	return;
}
