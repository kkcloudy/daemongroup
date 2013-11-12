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
* AsdRoamingSta.c
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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "includes.h"

#include "asd.h"
#include "ASDStaInfo.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "common.h"
#include "ap.h"
#include "config.h"
#include "circle.h"
#include "roaming_sta.h"
#include "syslog.h"
extern unsigned char gASDLOGDEBUG;//qiuchen
extern unsigned long gASD_AC_MANAGEMENT_IP;

struct ROAMING_STAINFO * roaming_get_sta(WID_WLAN *WLAN, const u8 *sta)
{
	struct ROAMING_STAINFO *s;

	s = WLAN->r_sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}


static void roaming_sta_list_del(WID_WLAN *WLAN, struct ROAMING_STAINFO *sta)
{
	struct ROAMING_STAINFO *tmp;

	if (WLAN->r_sta_list == sta) {
		WLAN->r_sta_list = sta->next;
		return;
	}

	tmp = WLAN->r_sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove STA " MACSTR " from "
			   "list.", MAC2STR(sta->addr));
	} else
		tmp->next = sta->next;
}


void roaming_sta_hash_add(WID_WLAN *WLAN, struct ROAMING_STAINFO *sta)
{
	sta->hnext = WLAN->r_sta_hash[STA_HASH(sta->addr)];
	WLAN->r_sta_hash[STA_HASH(sta->addr)] = sta;
}


static void roaming_sta_hash_del(WID_WLAN *WLAN, struct ROAMING_STAINFO *sta)
{
	struct ROAMING_STAINFO *s;

	s = WLAN->r_sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		WLAN->r_sta_hash[STA_HASH(sta->addr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       os_memcmp(s->hnext->addr, sta->addr, ETH_ALEN) != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		asd_printf(ASD_DEFAULT,MSG_ERROR, "AP: could not remove STA " MACSTR
			   " from hash table", MAC2STR(sta->addr));
}


void roaming_free_sta(WID_WLAN *WLAN, struct ROAMING_STAINFO *sta)
{
	asd_printf(ASD_DEFAULT,MSG_INFO,"roaming free sta\n");
	//int set_beacon = 0;
	//int i=0;
		
	RoamingStaInfoToWIFI(sta,WID_DEL);
	roaming_sta_hash_del(WLAN, sta);
	roaming_sta_list_del(WLAN, sta);
	WLAN->r_num_sta--;
	os_free(sta);

}


struct ROAMING_STAINFO * roaming_sta_add(WID_WLAN *WLAN, const u8 *addr, unsigned int BSSIndex, const u8 *BSSID)
{
	struct ROAMING_STAINFO *sta;
	sta = roaming_get_sta(WLAN, addr);
	if (sta){
		asd_printf(ASD_DEFAULT,MSG_INFO,"roaming find sta in add\n");
		return sta;
	}
	sta = os_zalloc(sizeof(struct ROAMING_STAINFO));
	if (sta == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "malloc failed");
		return NULL;
	}	
	asd_printf(ASD_DEFAULT,MSG_INFO,"roaming add sta in add\n");
	sta->need_notice_wifi = 0;
	os_memcpy(sta->addr, addr, ETH_ALEN);
	sta->BssIndex = BSSIndex;
	sta->PreBssIndex = BSSIndex;
	os_memcpy(sta->BSSID, BSSID, ETH_ALEN);
	os_memcpy(sta->PreBSSID, BSSID, ETH_ALEN);
	sta->next = WLAN->r_sta_list;
	WLAN->r_sta_list = sta;
	WLAN->r_num_sta++;
	roaming_sta_hash_add(WLAN, sta);
	return sta;
	
}

void roaming_del_all_sta(unsigned char WLANID)
{
	struct ROAMING_STAINFO *sta, *prev;
	if(ASD_WLAN[WLANID] != NULL){
		sta = ASD_WLAN[WLANID]->r_sta_list;
		while(sta){
			prev = sta;
			sta = sta->next;
			roaming_free_sta(ASD_WLAN[WLANID],prev);
		}
	}
}



struct ROAMING_STAINFO * AsdRoamingStaInfoAdd(struct asd_data *wasd, const u8 *addr){
	struct ROAMING_STAINFO *r_sta = NULL;
	struct sta_info *sta,*sta_cur;
	unsigned char WLANID = wasd->WlanID;
	unsigned int RadioID;
	unsigned char BSS_L_ID;
	unsigned char SID = wasd->SecurityID;//qiuchen
	struct asd_data *wasd_r;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	
	if(ASD_WLAN[WLANID] == NULL)
		return NULL;
	r_sta = roaming_sta_add(ASD_WLAN[WLANID],addr,wasd->BSSIndex,wasd->own_addr);
	if(r_sta == NULL){
		asd_printf(ASD_DEFAULT,MSG_ERROR,"(r_sta == NULL)\n");
		return NULL;
	}
	if(r_sta->BssIndex !=  wasd->BSSIndex){
		r_sta->need_notice_wifi = 1;
		RadioID = r_sta->BssIndex/L_BSS_NUM;
		BSS_L_ID = r_sta->BssIndex%L_BSS_NUM;
		if((interfaces->iface[RadioID]!=NULL)&&(interfaces->iface[RadioID]->bss[BSS_L_ID]!=NULL)){
			wasd_r = interfaces->iface[RadioID]->bss[BSS_L_ID];
			sta = ap_get_sta(wasd_r,addr);
			if(sta != NULL){
				asd_printf(ASD_DEFAULT,MSG_INFO,"roaming ap free sta\n");
				sta_cur = ap_get_sta(wasd,addr);
				if(sta_cur != NULL)
				{
					if((sta->ipaddr != 0)&&(sta_cur->ipaddr == 0))
					{
						sta_cur->ipaddr = sta->ipaddr;
						sta_cur->ip_addr.s_addr = sta->ip_addr.s_addr;
						memset(sta_cur->in_addr,0,16);
						memcpy(sta_cur->in_addr,sta->in_addr,strlen(sta->in_addr));
						memset(sta_cur->arpifname,0,sizeof(sta_cur->arpifname));
						memcpy(sta_cur->arpifname,sta->arpifname,sizeof(sta->arpifname));
					}
					
					/* add to support ipv6 address */
					if((asd_check_ipv6(sta->ip6_addr) != 0) && (asd_check_ipv6(sta_cur->ip6_addr) == 0))
					{
						sta_cur->ip6_addr = sta->ip6_addr;
						memset(sta_cur->arpifname,0,sizeof(sta_cur->arpifname));
						memcpy(sta_cur->arpifname,sta->arpifname,sizeof(sta->arpifname));
					}					
					sta_cur->rflag = ASD_ROAM_3;//qiuchen
					sta_cur->PreBssIndex = wasd_r->BSSIndex;
					sta_cur->preAPID = wasd_r->Radio_G_ID/4;
					memcpy(sta_cur->PreBSSID,wasd_r->own_addr,6);
					char *SSID = (char *)wasd_r->conf->ssid.ssid;
					if(gASDLOGDEBUG & BIT(1) && !(sta->logflag&BIT(1))){
						if((ASD_SECURITY[SID]) && (ASD_SECURITY[SID]->securityType == OPEN || ASD_SECURITY[SID]->securityType == SHARED) && (sta->flags & WLAN_STA_ASSOC)){
							syslog(LOG_INFO|LOG_LOCAL3,"STA_ROAM_SUCCESS:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
								MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
								wasd_r->Radio_G_ID/4,MAC2STR(wasd_r->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
								wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
							);
							sta->logflag = BIT(1);
						}
						else if((sta->flags & WLAN_STA_AUTHORIZED)){
							syslog(LOG_INFO|LOG_LOCAL3,"STA_ROAM_SUCCESS:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
								MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
								wasd_r->Radio_G_ID/4,MAC2STR(wasd_r->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
								wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
							);
							sta->logflag = BIT(1);
						}
					}
					if(gASDLOGDEBUG & BIT(0))
						asd_syslog_h(6,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MAC_ADDRESS" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,65534);
					if(gASDLOGDEBUG & BIT(0) && !(sta->logflag&BIT(0))){
						if((ASD_SECURITY[SID]) && (ASD_SECURITY[SID]->securityType == OPEN || ASD_SECURITY[SID]->securityType == SHARED) && (sta->flags & WLAN_STA_ASSOC)){
							asd_syslog_h(6,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(wasd_r->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
										((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
							sta->logflag = BIT(0);
						}
						else if(sta->flags & WLAN_STA_AUTHORIZED){
							asd_syslog_h(6,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(wasd_r->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16), \
										((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16), ((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
							sta->logflag = BIT(0);
						}
					}
				}
				AsdStaInfoToWID(wasd_r, sta->addr, WID_DEL);
				ap_free_sta(wasd_r,sta,0);
			}
		}
	}
	r_sta->BssIndex = wasd->BSSIndex;
	os_memcpy(r_sta->BSSID, wasd->own_addr, MAC_LEN);
 	return r_sta;
}

int RoamingStaInfoToWSM(struct ROAMING_STAINFO *sta, Operate op){
	TableMsg STA;
	int len;
	STA.Op = op;
	STA.Type = STA_TYPE;
	os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
	if((sta == NULL)) {
		return 0;
	}else {
		STA.u.STA.RoamingTag = 2;
		STA.u.STA.BSSIndex = sta->PreBssIndex;
		STA.u.STA.WTPID = ((sta->PreBssIndex)/L_BSS_NUM)/L_RADIO_NUM;
		memcpy(STA.u.STA.STAMAC, sta->addr, ETH_ALEN);		
		memcpy(STA.u.STA.RBSSID, sta->PreBSSID, ETH_ALEN);
	}
		
	len = sizeof(STA);
	
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 0;
}

//weichao add
int RoamingStaInfoToWIFI(struct ROAMING_STAINFO *sta, Operate op){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in fun:%s\n",__func__);
	struct asd_to_wifi_sta tmp;
	int ret = 0;	
	int fd = open("/dev/wifi0", O_RDWR);
	if(fd < 0)
	{
		return 0;
	}
	if(sta == NULL)
	{
		close(fd);//qiuchen
		return 0;
	}
	memset(&tmp, 0, sizeof(struct asd_to_wifi_sta));
	memcpy(tmp.STAMAC,sta->addr,MAC_LEN);
	memcpy(tmp.BSSID_Before,sta->PreBSSID,MAC_LEN);
	memcpy(tmp.BSSID,sta->BSSID,MAC_LEN);
	tmp.roaming_flag = 2;
	if(op == WID_ADD){
		ret = ioctl(fd,WIFI_IOC_ADD_STA,&tmp);
	}	
	if(op == WID_DEL){
		ret = ioctl(fd,WIFI_IOC_DEL_STA,&tmp);
	}
	if(ret < 0)
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s ioctl error %s\n",__func__,strerror(errno));
	close(fd);
	return 1;
}
int RoamingStaInfoToWSM_1(struct sta_info *sta, Operate op){
	TableMsg STA;
	int len;
	STA.Op = op;
	STA.Type = STA_TYPE;
	os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
	if((sta == NULL)) {
		return 0;
	}else {
		STA.u.STA.RoamingTag = 2;
		STA.u.STA.BSSIndex = sta->PreBssIndex;
		STA.u.STA.WTPID = ((sta->PreBssIndex)/L_BSS_NUM)/L_RADIO_NUM;
		memcpy(STA.u.STA.STAMAC, sta->addr, ETH_ALEN);		
		memcpy(STA.u.STA.RBSSID, sta->PreBSSID, ETH_ALEN);
	}
		
	len = sizeof(STA);
	
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 0;
}

//weichao add
int RoamingStaInfoToWIFI_1(struct sta_info *sta, Operate op){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in fun:%s\n",__func__);
	struct asd_to_wifi_sta tmp;
	int ret = 0;	
	int fd = open("/dev/wifi0", O_RDWR);
	if(fd < 0)
	{
		return 0;
	}
	if(sta == NULL)
	{
		close(fd);//qiuchen
		return 0;
	}
	memset(&tmp, 0, sizeof(struct asd_to_wifi_sta));
	memcpy(tmp.STAMAC,sta->addr,MAC_LEN);
	memcpy(tmp.BSSID_Before,sta->PreBSSID,MAC_LEN);
	memcpy(tmp.BSSID,sta->BSSID,MAC_LEN);
	tmp.roaming_flag = 2;
	if(op == WID_ADD){
		ret = ioctl(fd,WIFI_IOC_ADD_STA,&tmp);
	}	
	if(op == WID_DEL){
		ret = ioctl(fd,WIFI_IOC_DEL_STA,&tmp);
	}
	if(ret < 0)
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s ioctl error %s\n",__func__,strerror(errno));
	close(fd);
	return 1;
}


