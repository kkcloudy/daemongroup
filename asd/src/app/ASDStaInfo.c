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
* AsdRadioAp.c
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <netdb.h>   
#include <sys/ioctl.h>   
#include <net/if.h>

#include "includes.h"

#include "asd.h"
#include "ASDStaInfo.h"
#include "circle.h"
#include "ASDAccounting.h"
#include "ASD8021XOp.h"
#include "ASD80211Op.h"
#include "ASDRadius/radius.h"
#include "ASDWPAOp.h"
#include "ASDPreauth.h"
#include "ASDRadius/radius_client.h"
#include "ASDEapolSM.h"
#include "ASDCallback.h"
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDMlme.h"
#include "vlan_init.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"

#include "ASDDbus.h"
#include "asd_bak.h"
#include "ASDNetlinkArpOp.h"
#include "cert_auth.h"
#include "Inter_AC_Roaming.h"
#include "StaFlashDisconnOp.h"
#include "asd_iptables.h"
#include "../app/include/wai_sta.h"
#include "ASDCallback_asd.h"
#include "ASDDbus_handler.h"
#include "include/raw_socket.h"
#include "se_agent/se_agent_def.h" // for fastfwd
#include "ASDEAPAuth.h"

int ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER=10;

unsigned char	asd_get_sta_info_able = 0;
unsigned int	asd_get_sta_info_time = 10;
extern unsigned int IEEE_80211N_ENABLE;
extern unsigned char gASDLOGDEBUG;
static void asd_sta_hash_del(struct sta_info *sta);
static int ap_sta_in_other_bss(struct asd_data *wasd,
			       struct sta_info *sta, u32 flags);
static void ap_handle_session_timer(void *circle_ctx, void *timeout_ctx);
struct asd_data * 
get_kick_sta_wasd(unsigned int WTPID,unsigned char wlanid){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int i = 0, j=0;
	//unsigned int num = 0;

	for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
		if(interfaces->iface[i] == NULL)
			continue;
		for(j = 0; j < L_BSS_NUM; j++){
			if(interfaces->iface[i]->bss[j] == NULL)
				continue;
			else if(interfaces->iface[i]->bss[j]->WlanID==wlanid){
				return interfaces->iface[i]->bss[j];
			}
			else
				continue;
		}
	}
	
	return NULL;

}

int dynamic_num_blnc(struct asd_data *wasd){
	unsigned char wlanid=0;
	unsigned int  i=0,ap_min_num=0xffffffff,ap_max_num=0;
	unsigned int  kick_wtpid=0;
	struct asd_data *hd=NULL;
	unsigned char addr[6];
	struct sta_info *sta=NULL;
	
	if(wasd==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "wasd==NULL in %s\n", __func__);
		return -1;
	}

	wlanid=wasd->WlanID;
/*
	if(wasd->num_sta<=ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->wtp_triger_num){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: accept sta in wtp %u, bssindex %u\n"
			,__func__,wasd->Radio_G_ID/L_RADIO_NUM,wasd->BSSIndex);
		return 0;
	}
*/
	for(i=1;i<WTP_NUM;i++){
		
		if(1==is_wtp_apply_wlan(i,wlanid)){

			unsigned int num=0;
			
			num = get_sta_num_by_wtpid(i);
				
			if(num<ap_min_num){
				ap_min_num=num;
			}
			if(num>ap_max_num){
				ap_max_num=num;
				kick_wtpid=i;	
			}
		}
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: ap_min_num =%u. kick_wtpid=%d\n",__func__,ap_min_num,kick_wtpid);

	if(ap_min_num==0xffffffff||kick_wtpid==0){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: get ap min number or kick wtpid fail. \n",__func__);
		return 0;
	}

	if((ASD_WLAN[wasd->WlanID])&&((ap_max_num-ap_min_num)<=ASD_WLAN[wasd->WlanID]->balance_para)){
	//if(wasd->num_sta<=(ap_min_num+ASD_WLAN[wasd->WlanID]->balance_para)){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: balance now.\n",__func__);
		return 0;
	}

	hd=get_kick_sta_wasd(kick_wtpid, wlanid);

	if(hd==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: hd==NULL. \n",__func__);
		return -1;
	}

	sta=hd->sta_list;
	if(sta==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: sta==NULL. \n",__func__);
		return -1;
	}
	memcpy(addr,sta->addr,6);

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "kick sta:" MACSTR ".\n", MAC2STR(addr));
	
	char *SSID = NULL;
	if(ASD_WLAN[wlanid])
		SSID = ASD_WLAN[wlanid]->ESSID;
	if(gASDLOGDEBUG & BIT(0)){
		asd_syslog_h(6,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MAC_ADDRESS" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,5);//idletime out
	}
	ieee802_11_send_deauth(hd, addr, 3);

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: send deauth. \n",__func__);
	ap_free_sta(hd, sta, 1);
	
	AsdStaInfoToWID(hd, addr, WID_DEL);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: AsdStaInfoToWID end. \n",__func__);
	
	return 0;
}


static int mib_while_ap_free_sta(struct asd_data *wasd, struct sta_info *sta){
//	xm0703
	time_t now;

	if(wasd==NULL || sta==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "in %s: para invalid.\n", __func__);
		return -1;
	}
	
	//time(&now);
	get_sysruntime(&now);
	wasd->total_past_online_time+=now-(sta->add_time_sysruntime)+sta->sta_online_time;//qiuchen change it
	wasd->sta_assoced+=1;
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "in %s\n", __func__);
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "sta:" MACSTR " leave.\n", MAC2STR(sta->addr));
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "past online time %u\n", (int)(now-(sta->add_time_sysruntime)+sta->sta_online_time));//qiuchen change it
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "total past online time %llu\n", wasd->total_past_online_time);
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "total past assoced counter %u\n", wasd->sta_assoced);

	return 0;
}

int UpdateStaInfoToWSM(struct asd_data *wasd, const u8 *addr, Operate op){
	TableMsg STA;
	int len;
	STA.Op = op;
	STA.Type = STA_TYPE;
	
	if((wasd == NULL)||(addr == NULL) ) {
		os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
	}else {
	STA.u.STA.BSSIndex = wasd->BSSIndex;
	STA.u.STA.WTPID = ((wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC, addr, ETH_ALEN);
	}
		
	len = sizeof(STA);
	
	
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 2\n");
	//	close(sock);
		return 0;
	}
	return 0;
}

//mahz modified 2011.3.15
int UpdateStaInfoToCHILL(unsigned int BSSIndex, struct sta_info *sta, Operate op){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in func %s\n",__func__);
	TableMsg STA;
	int len;
//	int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	STA.Op = op;
	STA.Type = STA_TYPE;
	if((sta->addr == NULL) ) {
		os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
	}else {
		os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
		STA.u.STA.BSSIndex = BSSIndex;
		STA.u.STA.WTPID = ((BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
		memcpy(STA.u.STA.STAMAC, sta->addr, ETH_ALEN);
		STA.u.STA.delay = ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER;
		STA.u.STA.ipv4Address = sta->ip_addr.s_addr;
		if((WID_DEL == op)||(IDLE_STA_DEL == op)){
			STA.u.STA.tx_data_bytes = sta->txbytes;
			STA.u.STA.rx_data_bytes =sta->rxbytes;
			STA.u.STA.tx_frames = (unsigned int)sta->txpackets;
			 STA.u.STA.rx_frames = (unsigned int)sta->rxpackets;
		}
	}
		
	len = sizeof(STA);
	
	if((ASD_NOTICE_STA_INFO_TO_PORTAL==1)){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 4 \n");
		if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toEAG.addr, toEAG.addrlen) < 0){
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s sendtoCHILLI %s\n",__func__,strerror(errno));
			perror("send(wCHILLSocket)");
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 5\n");
	//		close(sock);
			return 0;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 6\n");
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA.op = %d\n",STA.Op);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"tx_data_bytes = %llu\n",STA.u.STA.tx_data_bytes );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rx_data_bytes = %llu\n",STA.u.STA.rx_data_bytes );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"tx_frames = %d\n",STA.u.STA.tx_frames );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rx_frames = %d\n",STA.u.STA.rx_frames );
	}
	return 0;
}
//weichao add
int UpdateStaInfoToEAG(unsigned int BSSIndex, struct FLASHDISCONN_STAINFO *sta, Operate op){
	TableMsg STA;
	int len;
	STA.Op = op;
	STA.Type = STA_TYPE;
	
	if((sta->addr == NULL) ) {
		os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
	}else {
		STA.u.STA.BSSIndex = BSSIndex;
		STA.u.STA.WTPID = ((BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
		memcpy(STA.u.STA.STAMAC, sta->addr, ETH_ALEN);
		STA.u.STA.delay = ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER;
		STA.u.STA.ipv4Address = sta->ipv4Address;
		STA.u.STA.tx_data_bytes = sta->txbytes;
		STA.u.STA.rx_data_bytes =sta->rxbytes;
		STA.u.STA.tx_frames = (unsigned long)sta->txpackets;
		 STA.u.STA.rx_frames = (unsigned long)sta->rxpackets;	
	}
		
	len = sizeof(STA);
	
	if((ASD_NOTICE_STA_INFO_TO_PORTAL==1)){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 4 \n");
		if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toEAG.addr, toEAG.addrlen) < 0){
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s sendtoCHILLI %s\n",__func__,strerror(errno));
			perror("send(wCHILLSocket)");
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 5\n");
	//		close(sock);
			return 0;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"UpdateStaInfoToWSM 6\n");
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA.op = %d\n",STA.Op);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"tx_data_bytes = %llu\n",STA.u.STA.tx_data_bytes );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rx_data_bytes = %llu\n",STA.u.STA.rx_data_bytes );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"tx_frames = %d\n",STA.u.STA.tx_frames );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rx_frames = %d\n",STA.u.STA.rx_frames );
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"TX: %llu\n",sta->txpackets);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"RX:%llu\n",sta->rxpackets);

	}
	return 0;
}
//weichao add
int UpdateStaInfoToFASTFWD(uint32_t ip,char *hand_cmd)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func:%s\n",__func__);
	if(ip == 0)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ip = 0, will not send to fastfwd!\n");
		return 0;
	}
	se_interative_t sta_data;
	
	memset(&sta_data,0,sizeof(sta_data));
	strncpy(sta_data.hand_cmd,hand_cmd,strlen(hand_cmd));
	sta_data.fccp_cmd.fccp_data.user_info.user_ip = ip;

	if(sendto(TipcSend, &sta_data, sizeof(sta_data), 0, (struct sockaddr *) &toFASTFWD.addr, toFASTFWD.addrlen) < 0){
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s sendtoFASTFWD %s\n",__func__,strerror(errno));
		perror("send(FASTFWD Socket)");
		return 0;
	}
	return 1;
}


int ap_for_each_sta(struct asd_data *wasd,
		    int (*cb)(struct asd_data *wasd, struct sta_info *sta,
			      void *ctx),
		    void *ctx)
{
	struct sta_info *sta;

	for (sta = wasd->sta_list; sta; sta = sta->next) {
		if (cb(wasd, sta, ctx))
			return 1;
	}

	return 0;
}


struct sta_info * ap_get_sta(struct asd_data *wasd, const u8 *sta)
{
	struct sta_info *s;

	s = wasd->sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}


static void ap_sta_list_del(struct asd_data *wasd, struct sta_info *sta)
{
	struct sta_info *tmp;

	if (wasd->sta_list == sta) {
		wasd->sta_list = sta->next;
		return;
	}

	tmp = wasd->sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove STA " MACSTR " from "
			   "list.", MAC2STR(sta->addr));
	} else
		tmp->next = sta->next;
}


void ap_sta_hash_add(struct asd_data *wasd, struct sta_info *sta)
{
	sta->hnext = wasd->sta_hash[STA_HASH(sta->addr)];
	wasd->sta_hash[STA_HASH(sta->addr)] = sta;
}


static void ap_sta_hash_del(struct asd_data *wasd, struct sta_info *sta)
{
	struct sta_info *s;

	s = wasd->sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		wasd->sta_hash[STA_HASH(sta->addr)] = s->hnext;
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
/*
	weichao modify from 1.3.11,20110802
*/
struct a_sta_info * wlan_get_sta(unsigned char wlanid, const u8 *sta)
{
	struct a_sta_info *s;
	if(ASD_WLAN[wlanid] == NULL)
		return NULL;
	
	s = ASD_WLAN[wlanid]->a_sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}
	
void wlan_sta_hash_add(unsigned char wlanid, struct a_sta_info *sta)
{
	if(ASD_WLAN[wlanid] == NULL)
		return ;

	sta->hnext = ASD_WLAN[wlanid]->a_sta_hash[STA_HASH(sta->addr)];
	ASD_WLAN[wlanid]->a_sta_hash[STA_HASH(sta->addr)] = sta;
	return;
}
	
static void wlan_sta_list_del(unsigned char wlanid, struct a_sta_info *sta)
{
	struct a_sta_info *tmp;
	if(ASD_WLAN[wlanid] == NULL)
		return ;

	if (ASD_WLAN[wlanid]->a_sta_list == sta) {
		ASD_WLAN[wlanid]->a_sta_list = sta->next;
		return ;
	}

	tmp = ASD_WLAN[wlanid]->a_sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp != NULL)
		tmp->next = sta->next;
	return ;
}
	
static void wlan_sta_hash_del(unsigned char wlanid, struct a_sta_info *sta)
{
	/*struct a_sta_info *s;*/
	struct PMK_STAINFO *s;
	if(ASD_WLAN[wlanid] == NULL)
		return ;

	s = ASD_WLAN[wlanid]->sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		ASD_WLAN[wlanid]->sta_hash[STA_HASH(sta->addr)] = s->hnext;
		return ;
	}

	while (s->hnext != NULL &&
		   os_memcmp(s->hnext->addr, sta->addr, ETH_ALEN) != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	return ;
}

void wlan_add_sta(unsigned char wlanid,const u8 *addr)
{
	struct a_sta_info *sta = NULL;
	if(ASD_WLAN[wlanid] == NULL)
		return ;

	sta = wlan_get_sta(wlanid,addr);

	if(sta)
		return ;

	sta = (struct a_sta_info *)os_malloc(sizeof(struct a_sta_info));
	os_memset(sta,0,sizeof(struct a_sta_info));
	os_memcpy(sta->addr,addr,MAC_LEN);
	sta->next = ASD_WLAN[wlanid]->a_sta_list;
	ASD_WLAN[wlanid]->a_sta_list = sta;
	wlan_sta_hash_add(wlanid,sta);
	ASD_WLAN[wlanid]->a_num_sta++;
	return ;
}

void wlan_free_stas(unsigned char wlanid)
{
	struct a_sta_info *sta, *prev;
	if(ASD_WLAN[wlanid] == NULL)
		return ;

	sta = ASD_WLAN[wlanid]->a_sta_list;

	while (sta) {
		prev = sta;
		sta = sta->next;
		wlan_sta_list_del(wlanid,prev);
		wlan_sta_hash_del(wlanid,prev);
		os_free(prev);
		prev = NULL; 
	}
	ASD_WLAN[wlanid]->a_num_sta = 0;
	return ;
}

/*ht add for ASD_STATIC_STA_TABLE,100114*/
struct sta_static_info *asd_get_static_sta(const u8 *sta)
{
	struct sta_static_info *s = NULL;

	s = STA_STATIC_TABLE[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}

void asd_static_sta_hash_add(struct sta_static_info *sta)
{
	sta->hnext = STA_STATIC_TABLE[STA_HASH(sta->addr)];
	STA_STATIC_TABLE[STA_HASH(sta->addr)] = sta;
	sta_static_num++;
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Add STA " MACSTR
		   " to sta static hash table", MAC2STR(sta->addr));
}

int asd_static_sta_hash_del(struct sta_static_info *sta,unsigned char wlanid)
{
	struct sta_static_info *s=NULL;
	struct sta_static_info *tmp=NULL;

	s = STA_STATIC_TABLE[STA_HASH(sta->addr)];
	if (s == NULL) return -1;
	
	if ((os_memcmp(s->addr, sta->addr, 6) == 0)&&(s->wlan_id == wlanid)) {
		STA_STATIC_TABLE[STA_HASH(sta->addr)] = s->hnext;
		sta_static_num--;
		tmp = s;
		s = s->hnext;
		tmp->hnext = NULL;
		free(tmp);
		tmp = NULL;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Remove STA " MACSTR
			   " from sta static hash table 1", MAC2STR(sta->addr));
		return 0;
	}

	while (s->hnext != NULL &&
		   ((os_memcmp(s->hnext->addr, sta->addr, ETH_ALEN) != 0)||(s->hnext->wlan_id != wlanid)))
		s = s->hnext;
	if (s->hnext != NULL){
		tmp = s->hnext;
		s->hnext = s->hnext->hnext;
		tmp->hnext = NULL;
		free(tmp);
		tmp = NULL;
		sta_static_num--;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Remove STA " MACSTR
			   " from sta static hash table 2", MAC2STR(sta->addr));
		return 0;
	}else{
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove STA " MACSTR
			   " from sta static hash table 1", MAC2STR(sta->addr));
		return -1;
	}
}


/*
static unsigned int th_get_deny_num(void){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
		int i = 0, num = 0;
		for(i = 0; i < G_RADIO_NUM; i++){
			if(interfaces->iface[i] != NULL){
				int j = 0 ;
				for(j = 0; j < L_BSS_NUM; j++){
					if(interfaces->iface[i]->bss[j] == NULL)
						continue;
					else {
							num+=interfaces->iface[i]->bss[j]->th_deny_num;
					}			
					continue;
							
				}
				
			} else {
				if(i%L_RADIO_NUM == 0){
					i += 3;
	
				}
				else {
					while(i%L_RADIO_NUM != 0){
						i++;
					}
					i--;
					
				}	
			}
		}
		return num;

}
*/


/*
static int recover_info_before_sta_free(struct asd_data *wasd,struct asd_data *last_wasd,struct sta_info *prev)
{	//xm add 08/01/04
	//if already has item in list,del it.
		if((ASD_WLAN[last_wasd->WlanID]!=NULL)&&(prev!=NULL)){
			
			log_node *p;
			int flag=0;
		
			if(ASD_WLAN[last_wasd->WlanID]->sta_from_which_bss_list!=NULL){
				p=ASD_WLAN[last_wasd->WlanID]->sta_from_which_bss_list;
				while(p!=NULL){
					if(0==memcmp(p->mac,prev->addr,6)) {
						if(p->from_bss_list!=NULL){
							bss_arrival_num * del,*dd;
							del=p->from_bss_list;
							while(del!=NULL){
								dd=del->next;
								free(del);
								del=NULL;
								del=dd;
							}
						}
						p->bss_index=wasd->BSSIndex;
						p->from_bss_list=prev->balence_bss_list;
						p->list_len=prev->balence_list_len;
						flag=1;
						p=NULL;
						break;
					}
					p=p->next;
				}
	
			}
	
			//create item in list
			if(ASD_WLAN[last_wasd->WlanID]->sta_from_which_bss_list==NULL||flag==0){
	
				p=(log_node *)os_malloc(sizeof(log_node));
		
				if((p!=NULL)&&(prev->balence_bss_list!=NULL)){
				
					memcpy(p->mac,prev->addr,6);
					p->bss_index=wasd->BSSIndex;
					p->from_bss_list=prev->balence_bss_list;
					p->list_len=prev->balence_list_len;
		
					p->next=ASD_WLAN[last_wasd->WlanID]->sta_from_which_bss_list;
					ASD_WLAN[last_wasd->WlanID]->sta_from_which_bss_list=p;
					ASD_WLAN[last_wasd->WlanID]->sta_list_len++;
		
				}
			}
			if(flag==1) flag=0;
		}

		return 0;
}//xm add 08/01/04

int check_bss_for_ballance(struct asd_data *wasd, bss_arrival_num *balence_bss_list)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int bssindex, g_radio;	
	struct asd_data *wasd_tmp, *last_wasd;	
	bss_arrival_num	bss_max;
	bss_arrival_num * sta_bss_list;
	if(balence_bss_list == NULL)
		return 0;
	sta_bss_list = balence_bss_list;
	bssindex = sta_bss_list->bss_index;
	g_radio = bssindex/L_BSS_NUM;

//printf("1\n");
	if((bssindex != wasd->BSSIndex)&&(interfaces->iface[g_radio] != NULL)&&(interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM]!= NULL)){		
		wasd_tmp = interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM];
		bss_max.bss_index = sta_bss_list->bss_index;		
		bss_max.sta_num = wasd_tmp->num_sta;
		last_wasd = wasd_tmp;
	}else{
		printf("something wrong in check_bss_for_probe_response g_radio = %d,l_bss = %d\n",g_radio,g_radio%4);
	}
	sta_bss_list = sta_bss_list->next;
//printf("2\n");

	while(sta_bss_list != NULL){
		bssindex = sta_bss_list->bss_index;
		g_radio = bssindex/L_BSS_NUM;
		if((bssindex != wasd->BSSIndex)&&(interfaces->iface[g_radio] != NULL)&&(interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM]!= NULL)){		
			
			wasd_tmp = interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM];
			if((bss_max.sta_num < wasd_tmp->num_sta)
				&&(ASD_WTP_AP[wasd_tmp->Radio_G_ID/L_RADIO_NUM]!=NULL)			
		  		&&(ASD_WTP_AP[wasd_tmp->Radio_G_ID/L_RADIO_NUM]->wtp_triger_num<wasd_tmp->num_sta)  //xm add 09/01/17
			){
				bss_max.bss_index = sta_bss_list->bss_index;		
				bss_max.sta_num = wasd_tmp->num_sta;				
				last_wasd = wasd_tmp;
			}
		}
		sta_bss_list = sta_bss_list->next;
	}
//printf("3\n");
	if(		(last_wasd!=NULL)
			&&(ASD_WTP_AP[last_wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)			
		  	&&(ASD_WTP_AP[last_wasd->Radio_G_ID/L_RADIO_NUM]->wtp_triger_num>=last_wasd->num_sta)
		){
		return 0;
	}//xm add
	
	if((last_wasd != NULL)&&(last_wasd->num_sta > (wasd->num_sta + ASD_WLAN[wasd->WlanID]->balance_para))){
		
		struct sta_info *sta, *prev;
		unsigned char mac[6];
		sta = last_wasd->sta_list;
		
		while (sta) {
			prev = sta;
			sta = sta->next;
			sta_bss_list = prev->balence_bss_list;
			while(sta_bss_list != NULL){
				if(sta_bss_list->bss_index == wasd->BSSIndex){

					recover_info_before_sta_free(wasd,last_wasd,prev); //xm modify 09/01/12
					
					prev->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
					unsigned char SID = (unsigned char)last_wasd->SecurityID;
					if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
						wpa_auth_sm_event(prev->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
						last_wasd, prev, 0);
						prev->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
						ieee802_1x_notify_port_enabled(prev->eapol_sm, 0);
					}
					memset(mac, 0, 6);
					memcpy(mac, prev->addr, 6);
//printf("4\n");		
					ap_free_sta(last_wasd, prev,1);
					ieee802_11_send_deauth(last_wasd, mac, 3);
					//add_mac_in_maclist(bss[i]->conf,mac,0); //add in black list		
					{//let wid know this sta is not allowed.
						TableMsg STA;
						int len;
						STA.Op = WID_DEL;
						STA.Type = STA_TYPE;
						STA.u.STA.BSSIndex = last_wasd->BSSIndex;
						STA.u.STA.WTPID = ((last_wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
						memcpy(STA.u.STA.STAMAC, mac, ETH_ALEN);
						len = sizeof(STA);
						if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
							perror("send(wASDSocket)");
						}
					}
					return 1;
				}
				sta_bss_list = sta_bss_list->next;
			}
		}
	}
	return 0;

}
*/
void ap_free_sta_without_wsm(struct asd_data *wasd, struct sta_info *sta, unsigned int state)
	{
		int set_beacon = 0;
	
		unsigned char mac[6];
		unsigned char channel=0;
		unsigned char rssi = 0;	//xiaodawei add rssi for telecom, 20110228
		int i=0;
		
		if((wasd == NULL) || (sta == NULL)){
			asd_printf(ASD_DEFAULT,MSG_INFO,"wasd or sta is NULL in %s\n",__func__);
			return ;
		}
		if(1 == check_sta_authorized(wasd,sta))
			wasd->authorized_sta_num--;
		circle_cancel_timeout(sta_auth_timer, wasd, sta);
		circle_cancel_timeout(ap_handle_timer, wasd, sta);
		circle_cancel_timeout(ap_handle_session_timer, wasd, sta);
		circle_cancel_timeout(ap_sta_idle_timeout,wasd,sta);
		circle_cancel_timeout(ap_sta_eap_auth_timer,wasd,sta);
		circle_cancel_timeout(wapi_radius_auth_send, wasd, sta);	
		if(is_secondary == 0)
			bak_del_sta(wasd,sta);
		
		for(i=0;i<6;i++)
			mac[i]=sta->addr[i];
		
		rssi = sta->rssi;	//xiaodawei add rssi for telecom, 20110228
			
		if(ASD_BSS[wasd->BSSIndex])
			ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num--;
		if(ASD_WLAN[wasd->WlanID])
			ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num--;
		//if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)							//mahz modify 2011.4.8
		//	ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num--;			//mahz modify 2011.4.8
		
		accounting_sta_stop(wasd, sta);
		
		if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL_SUCCESS(sta))){
			if(sta->ipaddr != 0)
			{	
				if(asd_ipset_switch)
					eap_connect_down(sta->ipaddr);
				else
					AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
			}
		}
		mib_while_ap_free_sta(wasd, sta);	//	xm0703
		if(STA_STATIC_FDB_ABLE && wasd->bss_iface_type && wasd->br_ifname){
			char ifname[IF_NAME_MAX]={0};
#ifndef _DISTRIBUTION_
		sprintf(ifname,"radio%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
#else
		if(local){
			sprintf(ifname,"r%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
		}else{
			sprintf(ifname,"r%d-%d-%d-%d.%d",slotid,vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
		}
#endif
			
			add_and_del_static_br_fdb(wasd->br_ifname,ifname, sta->addr,0) ;
		}
		
		ap_sta_hash_del(wasd, sta);
	//	asd_sta_hash_del(sta);
		ap_sta_list_del(wasd, sta);
	
		if (sta->aid > 0)
			wasd->sta_aid[sta->aid - 1] = NULL;
	
		wasd->num_sta--;
		if (sta->nonerp_set) {
			sta->nonerp_set = 0;
			wasd->iface->num_sta_non_erp--;
			if (wasd->iface->num_sta_non_erp == 0)
				set_beacon++;
		}
	
		if (sta->no_short_slot_time_set) {
			sta->no_short_slot_time_set = 0;
			wasd->iface->num_sta_no_short_slot_time--;
			if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
				&& wasd->iface->num_sta_no_short_slot_time == 0)
				set_beacon++;
		}
	
		if (sta->no_short_preamble_set) {
			sta->no_short_preamble_set = 0;
			wasd->iface->num_sta_no_short_preamble--;
			if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
				&& wasd->iface->num_sta_no_short_preamble == 0)
				set_beacon++;
		}
	
		if (set_beacon)
			ieee802_11_set_beacons(wasd->iface);
	
		unsigned char SID = wasd->SecurityID;
		if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
		ieee802_1x_free_alive(sta,&ASD_SECURITY[SID]->eap_alive_period);
		ieee802_1x_free_station(sta);
		wpa_auth_sta_deinit(sta->wpa_sm);
		rsn_preauth_free_station(wasd, sta);
		//radius_client_flush_auth(wasd->radius, sta->addr);
		}
		asd_printf(ASD_DEFAULT,MSG_INFO,"free sta in ap_free_sta_without_wsm\n");
		os_free(sta->last_assoc_req);
		sta->last_assoc_req=NULL;
		os_free(sta->in_addr);
		sta->in_addr=NULL;
		os_free(sta->challenge);
		sta->challenge=NULL;
		os_free(sta->add_time);
		sta->add_time=NULL;
		if(state == 1)
			dynamic_num_blnc(wasd);
	
	
		
	//add warnning when sta leave
		signal_sta_leave(mac,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID, rssi);	//xiaodawei add rssi for telecom, 20110228
	
		total_sta_unconnect_count++;
	
	//xm add 09.5.13
		if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL){
			time_t now;
			time_t now_sysrun;//qiuchen add it
			channel=sta->info_channel;//ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ra_ch[wasd->Radio_G_ID%L_RADIO_NUM];
			if((channel!=0)&&(channel<=13)){
	
				channel_time[channel].sta_num--;
				time(&now);
				channel_time[channel].end_time= now- channel_time[channel].begin_time;
				//qiuchen add it
				get_sysruntime(&now_sysrun);
				channel_time[channel].end_time_sysrun = now_sysrun - channel_time[channel].begin_time_sysrun;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"end_time sendt is %u, sendsys is %u func is %s line is %d.\n",(int)channel_time[channel].end_time,(int)channel_time[channel].end_time_sysrun,__func__,__LINE__);
				//end
				if(channel_time[channel].sta_num==0){
					
					channel_time[channel].begin_time=0;
					//qiuchen add it
					channel_time[channel].begin_time_sysrun = 0;
				}
				
			}
		}
	
		if((ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL) && (ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->wtp_deny_sta_flag == 1)
			&&(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num == ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_max_allowed_sta_num)){
				signal_de_wtp_deny_sta(wasd->Radio_G_ID/L_RADIO_NUM); //xm add	info trap helper
				ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->wtp_deny_sta_flag = 0;
		}
		if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)							//mahz add 2011.4.8
			ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num--;		//mahz add 2011.4.8
		
		os_free(sta);
		sta=NULL;
}


void ap_free_sta(struct asd_data *wasd, struct sta_info *sta, unsigned int state)
{
	int set_beacon = 0;

	time_t now_t;
	unsigned char mac[6];
	unsigned char channel=0;
	unsigned char WLANID;
	unsigned char group_id;
	unsigned char rssi;	//xiaodawei add rssi for telecom, 20110228
	int i=0;
	int j = 0; 	

	if((wasd == NULL) || (sta == NULL)){
		asd_printf(ASD_DEFAULT,MSG_INFO,"wasd or sta is NULL in %s\n",__func__);
		return ;
	}
	
	WLANID = wasd->WlanID;
	if(is_secondary == 1)
		wasd->authorized_sta_num--;
	else if(1 == check_sta_authorized(wasd,sta))
		wasd->authorized_sta_num--;
	if(sta->flags & WLAN_STA_FREE){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "sta is free!\n");
		return ;
	}else {
		sta->flags |= WLAN_STA_FREE;
	}
	//first cancle the timer
	circle_cancel_timeout(sta_auth_timer, wasd, sta);
	circle_cancel_timeout(ap_handle_timer, wasd, sta);
	circle_cancel_timeout(ap_handle_session_timer, wasd, sta);
	circle_cancel_timeout(ap_sta_eap_auth_timer,wasd,sta);
	circle_cancel_timeout(wapi_radius_auth_send, wasd, sta);	//
	circle_cancel_timeout(ap_sta_idle_timeout,wasd,sta);
	if((is_secondary == 0)&&(is_notice == 1))
		bak_del_sta(wasd,sta);

	if((ASD_WLAN[WLANID]!=NULL)\
		&&(ASD_WLAN[WLANID]->AC_Roaming_Policy == 1)\
		&&(roaming_notice == 1)){
		group_id = ASD_WLAN[WLANID]->group_id;
		if(AC_GROUP[group_id] != NULL)
		for(j = 0; j < G_AC_NUM; j++){
			int sock;
			if((AC_GROUP[group_id]->Mobility_AC[j] == NULL)||(AC_GROUP[group_id]->Mobility_AC[j]->is_conn == 0))
				continue;
			#if 0
			if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
			{	printf("tcp socket create failed\n");
				return;
			}
			
			unsigned long ul = 1;
			ioctl(sock, FIONBIO, &ul);
			while((connect(sock, (struct sockaddr *)&(AC_GROUP[group_id]->Mobility_AC[j]->ac_addr), sizeof(struct sockaddr)) == -1)){
				int n = 2;
				while(n > 0){
					fd_set fdset;
					struct timeval tv;			
					FD_ZERO(&fdset);
					FD_SET(sock,&fdset);
					
					tv.tv_sec = 2;
					tv.tv_usec = 0;
					
					if(select(sock + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
					{
						n--;
						continue;
					}
					
					if (FD_ISSET(sock, &fdset)){
						ul = 0;
						ioctl(sock, FIONBIO, &ul);
						break;			
					}
				}
				break;
			}
			#endif
			sock = AC_GROUP[group_id]->Mobility_AC[j]->sock;
			if(ac_group_add_del_sta_info(sock, group_id, sta, wasd, G_DEL)==0)
			{
				gThreadArg* tmp;
				tmp = (gThreadArg*)malloc(sizeof(gThreadArg));
				if(tmp == NULL){
					asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
					exit(1);
				}
				memset(tmp, 0, sizeof(gThreadArg));
				tmp->group_id = group_id;
				tmp->ACID = j;
				close(sock);
				AC_GROUP[group_id]->Mobility_AC[j]->sock = -1;
				AC_GROUP[group_id]->Mobility_AC[j]->is_conn = 0;
				
				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
					exit(1);
				}
				tmp->sock = sock;
				//fcntl(sock, F_SETFL, O_NONBLOCK);
				unsigned long ul = 1;
				ioctl(sock, FIONBIO, &ul);
				circle_register_timeout(0, 0,asd_synch_select, tmp, NULL);
			}
			//close(sock);
		}
	}
	
	for(i=0;i<6;i++)
		mac[i]=sta->addr[i];

	rssi = sta->rssi;	//xiaodawei add rssi for telecom, 20110228
	
	if(ASD_BSS[wasd->BSSIndex])
		ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num--;
	if(ASD_WLAN[wasd->WlanID])
		ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num--;
	//if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)							//mahz modify 2011.4.8
	//	ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num--;			//mahz modify 2011.4.8
	
	accounting_sta_stop(wasd, sta);

	mib_while_ap_free_sta(wasd, sta);	//	xm0703
	if(STA_STATIC_FDB_ABLE && wasd->bss_iface_type){
		char ifname[IF_NAME_MAX]={0};
		
#ifndef _DISTRIBUTION_
		sprintf(ifname,"radio%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
#else
		if(local)
			sprintf(ifname,"r%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
		else{
			sprintf(ifname,"r%d-%d-%d-%d.%d",slotid,vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
		}
#endif
		add_and_del_static_br_fdb(wasd->br_ifname,ifname, sta->addr,0) ;
	}
	
	if (!ap_sta_in_other_bss(wasd, sta, WLAN_STA_ASSOC) &&
	    !(sta->flags & WLAN_STA_PREAUTH))
		asd_sta_remove(wasd, sta->addr);
	
	ap_sta_hash_del(wasd, sta);
	asd_sta_hash_del(sta);
	ap_sta_list_del(wasd, sta);

	if (sta->aid > 0)
		wasd->sta_aid[sta->aid - 1] = NULL;

	wasd->num_sta--;
	if (sta->nonerp_set) {
		sta->nonerp_set = 0;
		wasd->iface->num_sta_non_erp--;
		if (wasd->iface->num_sta_non_erp == 0)
			set_beacon++;
	}

	if (sta->no_short_slot_time_set) {
		sta->no_short_slot_time_set = 0;
		wasd->iface->num_sta_no_short_slot_time--;
		if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
		    && wasd->iface->num_sta_no_short_slot_time == 0)
			set_beacon++;
	}

	if (sta->no_short_preamble_set) {
		sta->no_short_preamble_set = 0;
		wasd->iface->num_sta_no_short_preamble--;
		if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
		    && wasd->iface->num_sta_no_short_preamble == 0)
			set_beacon++;
	}

#ifdef ASD_IEEE80211N
	if(IEEE_80211N_ENABLE){
		if (sta->no_ht_gf_set) {
			sta->no_ht_gf_set = 0;
			wasd->iface->num_sta_ht_no_gf--;
		}

		if (sta->no_ht_set) {
			sta->no_ht_set = 0;
			wasd->iface->num_sta_no_ht--;
		}

		if (sta->ht_20mhz_set) {
			sta->ht_20mhz_set = 0;
			wasd->iface->num_sta_ht_20mhz--;
		}

		//if (asd_ht_operation_update(wasd->iface) > 0)
		//	set_beacon++;
	}
#endif /* ASD_IEEE80211N */

	if (set_beacon)
		ieee802_11_set_beacons(wasd->iface);

	unsigned char SID = wasd->SecurityID;
	get_sysruntime(&now_t);//qiuchen add it
	if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL_SUCCESS(sta))){
		if(sta->ipaddr != 0)
		{
			if(asd_ipset_switch)
				eap_connect_down(sta->ipaddr);
			else
				AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
		}
		if(ASD_SECURITY[SID]&&(ASD_SECURITY[SID]->securityType == OPEN)){
			wasd->no_auth_downline_time += (now_t - (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen change it
			wasd->no_auth_sta_num--;
		}
	}
	if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)
		||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)
		||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)
		||(ASD_SECURITY[SID]->extensible_auth == 1))){
		ieee802_1x_free_alive(sta,&ASD_SECURITY[SID]->eap_alive_period);
		wasd->assoc_auth_downline_time += (now_t - (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen change it
		if(	sta->sta_assoc_auth_flag == 1){
			sta->sta_assoc_auth_flag = 0;
			wasd->assoc_auth_sta_num--;
		}
		ieee802_1x_free_station(sta);
		wpa_auth_sta_deinit(sta->wpa_sm);
		rsn_preauth_free_station(wasd, sta);
//		radius_client_flush_auth(wasd->radius, sta->addr);
	}else if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WAPI_AUTH)||(ASD_SECURITY[SID]->securityType == WAPI_PSK))){
		wasd->assoc_auth_downline_time += (now_t - (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen change it
		if(	sta->sta_assoc_auth_flag == 1){
			sta->sta_assoc_auth_flag = 0;
			wasd->assoc_auth_sta_num--;
		}
		circle_cancel_timeout(wapi_retry_timer, wasd->wapi_wasd->vap_user->wapid, &(sta->wapi_sta_info));
	}else if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType == SHARED)){
		wasd->assoc_auth_downline_time += (now_t - (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen change it
		if(	sta->sta_assoc_auth_flag == 1){
			sta->sta_assoc_auth_flag = 0;
			wasd->assoc_auth_sta_num--;
		}
	}
	//qiuchen 2012.12.05
	wasd->statistics.offline_sta_time += (now_t - (sta->add_time_sysruntime)+sta->sta_online_time);
	if (ASD_AUTH_TYPE_WEP_PSK(SID))	/* WEP/PSK assoc auth */
	{
		wasd->u.weppsk.online_sta_num--;
		wasd->u.weppsk.total_offline_sta_time += (now_t - (sta->add_time_sysruntime) + sta->sta_online_time);
		asd_printf(ASD_1X,MSG_DEBUG, " online_sta_num is %d",
		  wasd->u.weppsk.online_sta_num);
	}
	else if (ASD_AUTH_TYPE_EAP(SID))	/* EAP auth */
	{
		if (wasd->u.eap_auth.autoauth.online_sta_num > 0)
		{
			wasd->u.eap_auth.autoauth.online_sta_num--;
			wasd->u.eap_auth.autoauth.total_offline_sta_time += (now_t - (sta->add_time_sysruntime) + sta->sta_online_time);
		}
			
		asd_printf(ASD_DEFAULT,MSG_DEBUG, " online_sta_num is %d",
		  wasd->u.eap_auth.autoauth.online_sta_num);
	}
	//end
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"free sta in ap_free_sta\n");
	UpdateStaInfoToFASTFWD(sta->ipaddr,SE_AGENT_CLEAR_RULE_IP);
	if((sta->ipaddr != 0)&&(asd_sta_arp_listen != 0)){
		char mac[20];		
		memset(mac,0,20);
		sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(sta->addr));
		printf("sta->in_addr %s mac %s sta->arpifname %s\n",sta->in_addr, mac,sta->arpifname);
		ipneigh_modify(RTM_DELNEIGH, 0,sta->in_addr, mac,sta->arpifname);		
	}
	os_free(sta->last_assoc_req);
	sta->last_assoc_req=NULL;
	os_free(sta->in_addr);
	sta->in_addr=NULL;
	os_free(sta->challenge);
	sta->challenge=NULL;
	os_free(sta->add_time);
	sta->add_time=NULL;
	if(state == 1)
		dynamic_num_blnc(wasd);
	
	//os_free(sta);

//add warnning when sta leave
	signal_sta_leave(mac,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID,rssi);	//xiaodawei add rssi for telecom, 20110228

	total_sta_unconnect_count++;

//xm add 09.5.13
	if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL){
		time_t now;
		time_t now_sysrun;
		channel=sta->info_channel;//ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ra_ch[wasd->Radio_G_ID%L_RADIO_NUM];
		if((channel!=0)&&(channel<=13)){

			channel_time[channel].sta_num--;
			time(&now);
			channel_time[channel].end_time= now- channel_time[channel].begin_time;
			//qiuchen add it
			get_sysruntime(&now_sysrun);
			channel_time[channel].end_time_sysrun = now_sysrun - channel_time[channel].begin_time_sysrun;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"end_time sendt is %u, sendsys is %u func is %s line is %d.\n",(int)channel_time[channel].end_time,(int)channel_time[channel].end_time_sysrun,__func__,__LINE__);
			//end
			if(channel_time[channel].sta_num==0){
				
				channel_time[channel].begin_time=0;
				//qiuchen add it
				channel_time[channel].begin_time_sysrun = 0;
			}
			
		}
	}

	if((ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL) && (ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->wtp_deny_sta_flag == 1)
		&&(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num == ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_max_allowed_sta_num)){
			signal_de_wtp_deny_sta(wasd->Radio_G_ID/L_RADIO_NUM); //xm add  info trap helper
			ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->wtp_deny_sta_flag = 0;
	}
	if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)							//mahz add 2011.4.8
		ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num--;		//mahz add 2011.4.8

	os_free(sta);
	
	sta=NULL;
}


void asd_free_stas(struct asd_data *wasd)
{
	struct sta_info *sta, *prev;

	sta = wasd->sta_list;

	while (sta) {
		prev = sta;
		if (sta->flags & WLAN_STA_AUTH) {
			mlme_deauthenticate_indication(
				wasd, sta, WLAN_REASON_UNSPECIFIED);
		}
		sta = sta->next;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Removing station " MACSTR,
			   MAC2STR(prev->addr));
		
		if(ASD_NOTICE_STA_INFO_TO_PORTAL)
			AsdStaInfoToEAG(wasd,prev,WID_DEL);
		is_notice = 0;
		ap_free_sta(wasd, prev,0);
		is_notice = 1;
	}
}


//xm0715
static int parse_long_long(char* str,unsigned long long * ll){
	char *endptr = NULL;
	char c;
	c = str[0];

	if (c>='0'&&c<='9'){
		*ll= strtoul(str,&endptr,10);
		if((endptr[0] == '\0')||(endptr[0] == '\n'))
			return 0;
		else
			return -1;
	}
	else
		return -1;
}


/*超时重发处理*/
void wapi_retry_timer(void *circle_ctx, void *timeout_ctx)
{
	int sendlen = 0;
	int waiting_groupkey = 0;
	time_t current_time = time(0);
	struct ethhdr eh;
	struct auth_sta_info_t	*pmt=NULL;
	struct wapid_interfaces *wapid ;
	struct asd_data *wasd;
	struct asd_wapi *tmp_circle ;
	//struct sta_info *sta;
	apdata_info *pap = circle_ctx;
	pmt = timeout_ctx;

	if(pap == NULL || pmt == NULL) return ;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return ;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return ;

	wasd = (struct asd_data *)tmp_circle->wasd_bk;
	if(wasd == NULL) return ;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s "MACSTR" status %d\n",__func__,MAC2STR(pmt->mac),pmt->status);
	if (pmt->status == NO_AUTH
		||pmt->status == MT_AUTHENTICATED
		||pmt->sendinfo.cur_count  == 0)
		return ;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s timeout %d\n",__func__,pmt->sendinfo.timeout);
	/*检查发送是否超时*/
	if (current_time - pmt->sendinfo.send_time >= pmt->sendinfo.timeout) 
	{
		/*检测是否达到最大重发次数*/
		if (pmt->sendinfo.cur_count >= MAX_RESEND_COUNT) 
		{
			DPrintf("resend counter is bigger than 3;pmt->status = %d\n", pmt->status);
			if(pmt->status == MT_GROUPNOTICEING)
			{
				waiting_groupkey++ ;
				pap->group_No--;//record group notice no.
			}
			if(pmt->status == MT_WAITING_AUTH_FROM_AS)
			{
				pap->wapi_mib_stats.wai_cert_handshake_fail++;
				pmt->wapi_mib_stats.wai_cert_handshake_fail++;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_cert_handshake_fail++\n\n",MAC2STR(pap->macaddr));
			}		

			if(pmt->status == MT_WAITING_SESSION)
			{
				pap->wapi_mib_stats.wai_unicast_handshake_fail++;
				pmt->wapi_mib_stats.wai_unicast_handshake_fail++;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_unicast_handshake_fail++\n\n",MAC2STR(pap->macaddr));
			}
		
			if(pmt->status == MT_WAITING_GROUPING)
			{
				pap->wapi_mib_stats.wai_multi_handshake_fail++;
				pmt->wapi_mib_stats.wai_multi_handshake_fail++;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_multi_handshake_fail++\n\n",MAC2STR(pap->macaddr));
			}

			if((pmt->status == MT_WAITING_DYNAMIC_SESSION )
				||(pmt->status == MT_GROUPNOTICEING)
				||(pmt->status == MT_SESSIONGROUPING)){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s resend ukey rekey fail. \n",__func__);
				kick_sta_mac(pmt->mac);
				reset_sta_info(pmt, pap);
				return ;
			}
			
			/*检查鉴别模式*/
			if(pmt->auth_mode == AUTH_MODE)
			{
				//notify_driver_disauthenticate_sta(pmt, __func__, __LINE__);
				kick_sta_mac(pmt->mac);
				DPrintf("notify driver to disauthenticate this sta\n");
			}
			
			/*清除重发缓冲区*/
			reset_sta_info(pmt, pap);
			return ;
		}else{
			pap->wapi_mib_stats.wai_timeout++;
			pmt->wapi_mib_stats.wai_timeout++;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_timeout++\n\n",MAC2STR(pap->macaddr));
			memcpy (eh.h_dest, pmt->mac,  ETH_ALEN);
			if ((pmt->buf0.len > 0) && (pmt->buf0.data !=NULL))
			{		
				DPrintf("Timeout. status=%d  send_count=%d \n", 
				pmt->status, pmt->sendinfo.cur_count);

				DPrintf("\nresend buf0. len=%d\n", pmt->buf0.len);
				/*检查发送的反向*/
				if (pmt->sendinfo.direction == SENDTO_AS)
				{
					DPrintf("\nresend to AS\n");
					sendlen = sendto(tmp_circle->as_udp_sk, pmt->buf0.data, pmt->buf0.len,
									0, (struct sockaddr *)&(tmp_circle->as_addr), 
									sizeof(struct sockaddr_in));
					
					if (sendlen != pmt->buf0.len)
					{
						DPrintf("\nfail to send to ASU\n");
						return ;
					}
				}
				else
				{
					DPrintf("resend buf0 to STA "MACSTR"\n", MAC2STR(pmt->mac));
					/*发送数据*/
					send_rs_data(pmt->buf0.data, pmt->buf0.len, &eh, pap);
				}
				
			}

			if ((pmt->buf1.len > 0) && (pmt->buf1.data !=NULL)) 
			{				
				DPrintf("resend buf1. len=%d\n", pmt->buf1.len);	
				DPrintf("resend buf1 to STA "MACSTR"\n", MAC2STR(pmt->mac));
				send_rs_data(pmt->buf1.data, pmt->buf1.len, &eh, pap);	
			}
			pmt->sendinfo.send_time = current_time;
			(pmt->sendinfo.cur_count) ++;
			circle_register_timeout(pmt->sendinfo.timeout, 0, wapi_retry_timer, pap, pmt);
			return ;
		}
	}
	circle_register_timeout(pmt->sendinfo.timeout, 0, wapi_retry_timer, pap, pmt);
	return ;
}


void get_flow_timer_handler(void *circle_ctx, void *timeout_ctx)
{
	unsigned int i=0;
	unsigned char wlanid=0;
	char  filename[256], filename1[256],buf[DEFAULT_LEN];
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int radio_id = 0;
	
	unsigned long long tx_byte=0;
	unsigned long long rx_byte=0;
	WID_WLAN	*wlan=NULL;
	int ii;
	
	//time_t now;

	if(timeout_ctx==NULL)
		return;
	
	wlan=(WID_WLAN*)timeout_ctx;
	
	memset(filename,0,256);
	memset(filename1,0,256);
	memset(buf,0,DEFAULT_LEN);

	wlanid=wlan->WlanID;

	//time(&now);
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s wlanid=%u\n",__func__,wlanid);
	
	for(i=1;i<WTP_NUM;i++){
		if(ASD_WTP_AP[i]!=NULL){
			for(ii=0;ii<L_RADIO_NUM;ii++){
				radio_id = L_RADIO_NUM*i + ii;
				if(interfaces->iface[radio_id] == NULL)
					continue;
#ifndef _DISTRIBUTION_				
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s radio%u-%u-%u.%u\n",__func__,vrrid,i,ii,wlanid);
				sprintf(filename,"/sys/class/net/radio%u-%u-%u.%u/statistics/tx_bytes",vrrid,i,ii,wlanid);
#else
				if(local){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s radio%u-%u-%u.%u\n",__func__,vrrid,i,ii,wlanid);
					sprintf(filename,"/sys/class/net/radio%u-%u-%u.%u/statistics/tx_bytes",vrrid,i,ii,wlanid);
				}else{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s radio%d-%u-%u-%u.%u\n",__func__,slotid,vrrid,i,ii,wlanid);
					sprintf(filename,"/sys/class/net/radio%u-%u-%u-%u.%u/statistics/tx_bytes",slotid,vrrid,i,ii,wlanid);
				}
#endif
				memset(buf,0,DEFAULT_LEN);
				if(read_ac_info(filename,buf) != 0){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"read tx_bytes file fail.\n");
					continue ;
				}
				if(parse_long_long(buf, &tx_byte)!=0){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"read tx_bytes fail.\n");
					tx_byte=0;
				}
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"tx_bytes  %llu.\n",tx_byte);
				
#ifndef _DISTRIBUTION_				
				sprintf(filename1,"/sys/class/net/radio%u-%u-%u.%u/statistics/rx_bytes",vrrid,i,ii,wlanid);
#else
				if(local){
					sprintf(filename1,"/sys/class/net/radio%u-%u-%u.%u/statistics/rx_bytes",vrrid,i,ii,wlanid);
				}else{
					sprintf(filename1,"/sys/class/net/radio%u-%u-%u-%u.%u/statistics/rx_bytes",slotid,vrrid,i,ii,wlanid);
				}
#endif
				memset(buf,0,DEFAULT_LEN);
				if(read_ac_info(filename1,buf) != 0){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"read rx_bytes file fail.\n");
					continue ;
				}
				if(parse_long_long(buf, &rx_byte)!=0){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"read rx_bytes fail.\n");
					rx_byte=0;
				}
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"rx_bytes  %llu.\n",rx_byte);
				
				ASD_WTP_AP[i]->radio_flow_info[ii].old_tx_bytes = ASD_WTP_AP[i]->radio_flow_info[ii].tx_bytes;
				ASD_WTP_AP[i]->radio_flow_info[ii].tx_bytes = tx_byte+rx_byte;
				ASD_WTP_AP[i]->radio_flow_info[ii].trans_rates = (ASD_WTP_AP[i]->radio_flow_info[ii].tx_bytes - ASD_WTP_AP[i]->radio_flow_info[ii].old_tx_bytes)*8/(1024*30);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"old_tx_bytes = %u (byte)\n",ASD_WTP_AP[i]->radio_flow_info[ii].old_tx_bytes);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"new_tx_bytes = %u (byte)\n",ASD_WTP_AP[i]->radio_flow_info[ii].tx_bytes);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"trans_rates  = %u (kbps)\n",ASD_WTP_AP[i]->radio_flow_info[ii].trans_rates);

			}
		}
	}
	
	circle_register_timeout(30, 0, get_flow_timer_handler, NULL, timeout_ctx);
	return;
	
}		//xm0715

/*
  *ht add 0907014
  */
void del_wids_mac_timer(void *circle_ctx, void *timeout_ctx)
{
	struct acl_config *conf = circle_ctx;
	struct maclist *list = timeout_ctx;

	if((conf == NULL)||(list == NULL)){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s conf == NULL || list == NULL",__func__);
		return ;
	}
	
	del_mac_in_blacklist_for_wids(conf,list->addr);

		
	return;
}



/*
  *ht add 090703
  */
void assoc_update_timer(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	pthread_mutex_lock(&(asd_g_sta_mutex));

	if (wasd != NULL) {
		
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_lock(&(wasd->asd_sta_mutex));
#endif
		wasd->assoc_req_timer_update = wasd->assoc_req;
		wasd->assoc_resp_timer_update = wasd->assoc_resp;
		wasd->assoc_success_all_timer_update = wasd->assoc_success + wasd->reassoc_success;
		circle_register_timeout(ASSOC_UPDATE_TIME, 0, assoc_update_timer, wasd, sta);
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_unlock(&(wasd->asd_sta_mutex));		
#endif
	}
	pthread_mutex_unlock(&(asd_g_sta_mutex));		
	
	return;
}

#if 0
/*
  *ht add 090917
  */
void sta_info_to_wsm_timer(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	//UpdateStaInfoToWSM(NULL,NULL,STA_INFO);
	//circle_register_timeout(asd_get_sta_info_time, 0, sta_info_to_wsm_timer, wasd, sta);
	return;
}
#endif

/*
  *ht add 090219
  */
void sta_auth_timer(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	pthread_mutex_lock(&(asd_g_sta_mutex));
#ifdef ASD_USE_PERBSS_LOCK
	pthread_mutex_lock(&(wasd->asd_sta_mutex));
#endif

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\t "MACSTR" flag:%d\n",__func__,MAC2STR(sta->addr),sta->flags);
	if ((sta->flags & WLAN_STA_ASSOC)||(sta->flags & WLAN_STA_AUTHORIZED)) {
	}else if (sta->flags & WLAN_STA_AUTH_ACK) {
		ap_free_sta(wasd,sta,0);
	}else {
		sta->flags |= WLAN_STA_AUTH_ACK;
		circle_register_timeout(10, 0, sta_auth_timer, wasd, sta);
	}
	
#ifdef ASD_USE_PERBSS_LOCK
	pthread_mutex_unlock(&(wasd->asd_sta_mutex));
#endif
	pthread_mutex_unlock(&(asd_g_sta_mutex)); 	
	return;
}
void ap_sta_eap_auth_timer(void *circle_ctx,void *timeout_ctx)
{
	struct  asd_data *wasd = circle_ctx;
	struct sta_info*sta = timeout_ctx;
	if((wasd == NULL)||(sta == NULL))
		return;
	if(sta->flags & WLAN_STA_AUTHORIZED)
		return;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s ; sta "MACSTR" is unauthorized too long\n",__func__,MAC2STR(sta->addr));	
	wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
	mlme_deauthenticate_indication(
	wasd, sta, 0);
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	ieee802_11_send_deauth(wasd, sta->addr, 3);
	AsdStaInfoToWID(wasd,sta->addr,WID_DEL);	
	if(ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method== 1)
		ap_free_sta(wasd, sta,1);
	else
		ap_free_sta(wasd, sta,0);
}
void ap_handle_timer(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	unsigned long next_time = 1;/*zhanglei chenge 0 to 1 for sta still alive*/
	sta->timeout_next = STA_NULLFUNC;/*zhanglei add for sta still alive*/

	if (sta->timeout_next == STA_REMOVE) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_INFO, "deauthenticated due to "
			       "local deauth request");
		//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
		//ap_free_sta(wasd, sta,1);
	if(ASD_WLAN[wasd->WlanID] && ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method== 1)
		ap_free_sta(wasd, sta,1);
	else
		ap_free_sta(wasd, sta,0);
		return;
	}

	if ((sta->flags & WLAN_STA_ASSOC) &&
	    (sta->timeout_next == STA_NULLFUNC ||
	     sta->timeout_next == STA_DISASSOC)) {
		int inactive_sec;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Checking STA " MACSTR " inactivity:",
			   MAC2STR(sta->addr));
		inactive_sec = asd_get_inact_sec(wasd, sta->addr);
		if (inactive_sec == -1) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not get station info "
				   "from kernel driver for " MACSTR ".",
				   MAC2STR(sta->addr));
		} else if (inactive_sec < wasd->conf->ap_max_inactivity &&
			   sta->flags & WLAN_STA_ASSOC) {
			/* station activity detected; reset timeout state */
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "  Station has been active");
			sta->timeout_next = STA_NULLFUNC;
			next_time = wasd->conf->ap_max_inactivity -
				inactive_sec;
		}
	}

	if ((sta->flags & WLAN_STA_ASSOC) &&
	    sta->timeout_next == STA_DISASSOC &&
	    !(sta->flags & WLAN_STA_PENDING_POLL)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "  Station has ACKed data poll");
		/* data nullfunc frame poll did not produce TX errors; assume
		 * station ACKed it */
		sta->timeout_next = STA_NULLFUNC;
		next_time = wasd->conf->ap_max_inactivity;
	}

	if (next_time) {
		circle_register_timeout(next_time, 0, ap_handle_timer, wasd,
				       sta);
		return;
	}

	if (sta->timeout_next == STA_NULLFUNC &&
	    (sta->flags & WLAN_STA_ASSOC)) {
		/* send data frame to poll STA and check whether this frame
		 * is ACKed */
		struct ieee80211_hdr hdr;

		asd_printf(ASD_DEFAULT,MSG_DEBUG, "  Polling STA with data frame");
		sta->flags |= WLAN_STA_PENDING_POLL;

		/* FIX: WLAN_FC_STYPE_NULLFUNC would be more appropriate, but
		 * it is apparently not retried so TX Exc events are not
		 * received for it */
		os_memset(&hdr, 0, sizeof(hdr));
		hdr.frame_control =
			IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
		hdr.frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
		hdr.frame_control |= host_to_le16(WLAN_FC_FROMDS);
		os_memcpy(hdr.IEEE80211_DA_FROMDS, sta->addr, ETH_ALEN);
		os_memcpy(hdr.IEEE80211_BSSID_FROMDS, wasd->own_addr,
			  ETH_ALEN);
		os_memcpy(hdr.IEEE80211_SA_FROMDS, wasd->own_addr, ETH_ALEN);
		hdr.seq_ctrl = seq_to_le16(wasd->seq_num++);
		if(wasd->seq_num == 4096)
			wasd->seq_num = 0;

		DataMsg msg;
		int msglen;
		msglen = Assmble_WSM_msg(&msg, wasd, &hdr,  sizeof(hdr), IEEE802_11_MGMT);

		if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
			perror("ap_handle_timer: send");
	} else if (sta->timeout_next != STA_REMOVE) {
		int deauth = sta->timeout_next == STA_DEAUTH;

		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Sending %s info to STA " MACSTR,
			   deauth ? "deauthentication" : "disassociation",
			   MAC2STR(sta->addr));

		if (deauth) {
			asd_sta_deauth(wasd, sta->addr,
					   WLAN_REASON_PREV_AUTH_NOT_VALID);
		} else {
			asd_sta_disassoc(
				wasd, sta->addr,
				WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
		}
	}

	switch (sta->timeout_next) {
	case STA_NULLFUNC:
		sta->timeout_next = STA_DISASSOC;
		circle_register_timeout(AP_DISASSOC_DELAY, 0, ap_handle_timer,
				       wasd, sta);
		break;
	case STA_DISASSOC:
		sta->flags &= ~WLAN_STA_ASSOC;
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
		if (!sta->acct_terminate_cause)
			sta->acct_terminate_cause =
				RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
		accounting_sta_stop(wasd, sta);
		if(ASD_SECURITY[wasd->SecurityID])
			ieee802_1x_free_alive(sta,&ASD_SECURITY[wasd->SecurityID]->eap_alive_period);
		ieee802_1x_free_station(sta);
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_INFO, "disassociated due to "
			       "inactivity");
		sta->timeout_next = STA_DEAUTH;
		circle_register_timeout(AP_DEAUTH_DELAY, 0, ap_handle_timer,
				       wasd, sta);
		mlme_disassociate_indication(
			wasd, sta, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
		break;
	case STA_DEAUTH:
	case STA_REMOVE:
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_INFO, "deauthenticated due to "
			       "inactivity");
		if (!sta->acct_terminate_cause)
			sta->acct_terminate_cause =
				RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
		mlme_deauthenticate_indication(
			wasd, sta,
			WLAN_REASON_PREV_AUTH_NOT_VALID);
		//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
		if(ASD_WLAN[wasd->WlanID] && ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method == 1)
			ap_free_sta(wasd, sta,1);
		else
			ap_free_sta(wasd, sta,0);
		break;
	}
}


static void ap_handle_session_timer(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	u8 addr[ETH_ALEN];
	pthread_mutex_lock(&(asd_g_sta_mutex));
#ifdef ASD_USE_PERBSS_LOCK
	pthread_mutex_lock(&(wasd->asd_sta_mutex));
#endif

	if (!(sta->flags & WLAN_STA_AUTH)){
		
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_unlock(&(wasd->asd_sta_mutex));
#endif
		pthread_mutex_unlock(&(asd_g_sta_mutex));
		return;
	}

	mlme_deauthenticate_indication(wasd, sta,
				       WLAN_REASON_PREV_AUTH_NOT_VALID);
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_INFO, "deauthenticated due to "
		       "session timeout");
	sta->acct_terminate_cause =
		RADIUS_ACCT_TERMINATE_CAUSE_SESSION_TIMEOUT;
	os_memcpy(addr, sta->addr, ETH_ALEN);
	//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
	if(ASD_WLAN[wasd->WlanID] && ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method== 1)
		ap_free_sta(wasd, sta,1);
	else
		ap_free_sta(wasd, sta,0);
	asd_sta_deauth(wasd, addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
#ifdef ASD_USE_PERBSS_LOCK
	pthread_mutex_unlock(&(wasd->asd_sta_mutex));	
#endif
	pthread_mutex_unlock(&(asd_g_sta_mutex));
}


void ap_sta_session_timeout(struct asd_data *wasd, struct sta_info *sta,
			    u32 session_timeout)
{
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG, "setting session timeout to %d "
		       "seconds", session_timeout);
	circle_cancel_timeout(ap_handle_session_timer, wasd, sta);
	circle_register_timeout(session_timeout, 0, ap_handle_session_timer,
			       wasd, sta);
}


void ap_sta_no_session_timeout(struct asd_data *wasd, struct sta_info *sta)
{
	circle_cancel_timeout(ap_handle_session_timer, wasd, sta);
}

/*
int balance_add_sta(struct asd_data *wasd)
{

	if(wasd==NULL) return -1;
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	bss_arrival_num *bss_node;
	struct sta_info *sta_node;
	unsigned int g_radio,bssindex;
	unsigned int sta_num_max,sta_num_min,sta_num,bss_has_max_sta=0;

	if(wasd->sta_list==NULL||interfaces==NULL) return -1;
	
	sta_node=wasd->sta_list;

	while(NULL!=sta_node){
		
		if(sta_node->balence_bss_list==NULL) return -1;
		bss_node=sta_node->balence_bss_list;
		bssindex = bss_node->bss_index;
		g_radio=bss_node->bss_index/L_BSS_NUM;
		if((interfaces->iface[g_radio] == NULL)
				||(interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM]== NULL))
			return -1;
		sta_num_max=sta_num_min=interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM]->num_sta;
		bss_node=bss_node->next;
		while(bss_node!=NULL){
			g_radio=bss_node->bss_index;			
			bssindex = bss_node->bss_index;
			if((interfaces->iface[g_radio] == NULL)
				||(interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM]== NULL))
				return -1;
			sta_num=interfaces->iface[g_radio]->bss[bssindex%L_BSS_NUM]->num_sta;

			if(sta_num_max<sta_num) {
				sta_num_max=sta_num;
				bss_has_max_sta=bss_node->bss_index;
			}
			if(sta_num_min>sta_num) sta_num_min=sta_num;
				
		}

		if((sta_num_max-sta_num_min)>ASD_WLAN[wasd->WlanID]->balance_para&&bss_has_max_sta==wasd->BSSIndex){
			char mac[6];
			memcpy(mac, sta_node->addr, 6);
			sta_node->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
			unsigned char SID = (unsigned char)wasd->SecurityID;
			if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
				wpa_auth_sm_event(sta_node->wpa_sm, WPA_DEAUTH);
				mlme_deauthenticate_indication(
				wasd, sta_node, 0);
				sta_node->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
				ieee802_1x_notify_port_enabled(sta_node->eapol_sm, 0);
			}
			memset(mac, 0, 6);
			memcpy(mac, sta_node->addr, 6);
			ap_free_sta(wasd, sta_node,0);
			ieee802_11_send_deauth(wasd, mac, 3);
			//add_mac_in_maclist(bss[i]->conf,mac,0); //add in black list		
			{//let wid know this sta is not allowed.
				TableMsg STA;
				int len;
				STA.Op = WID_DEL;
				STA.Type = STA_TYPE;
				STA.u.STA.BSSIndex = wasd->BSSIndex;
				STA.u.STA.WTPID = ((wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
				memcpy(STA.u.STA.STAMAC, mac, ETH_ALEN);
				len = sizeof(STA);
				if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
					perror("send(wASDSocket)");
				}
			}
			return 1;
		
		}
		sta_node=sta_node->next;
		
	}
	
	return 0;
	
}

*/


int asd_get_ip_v2(struct sta_info *sta)
{
    FILE * fp;
    char * line = NULL;
    char * res = NULL;
    char smac[18]={0};
    u32 len = 0;
    u32 read;
	char buf[128] = {0};
    sprintf(smac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(sta->addr));
	sprintf(buf,"sudo cat //proc/net/arp |grep %s > /var/run/wcpss/arp",smac);
	system(buf);
	fp = fopen("/temp/arp","r");
    if(!fp)
    {
        asd_printf(ASD_DEFAULT,MSG_DEBUG,"File /proc/net/arp can't be read!\n");
        return -1;
    }
    asd_printf(ASD_DEFAULT,MSG_DEBUG,"MAC: %s\n",smac);

    while ((read = getline(&line, &len, fp)) != -1) {
	res = strstr(line,smac);
        if(res != NULL)
        {
                strncpy(sta->in_addr,line,15);
                strcpy(sta->arpifname,line+72);
                asd_printf(ASD_DEFAULT,MSG_DEBUG,"IP : %s\n",sta->in_addr);
                asd_printf(ASD_DEFAULT,MSG_DEBUG,"arpifname : %s\n",sta->arpifname);
				os_free(line);
				line = NULL;
				fclose(fp);
				return 0;
        }
    }

	if(line) {
	    os_free(line);
		line = NULL;
	}
	fclose(fp);

    return -1;

}



struct sta_info * ap_sta_add(struct asd_data *wasd, const u8 *addr, int both)
{
	struct sta_info *sta;
	//char* nowtime;
	//time_t now;//zhanglei add
	unsigned char channel=0;
	int SID;
	sta = ap_get_sta(wasd, addr);
	
	if (sta)
		return sta;


	if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)
		ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num++;
		
	if(ASD_BSS[wasd->BSSIndex] != NULL)
		ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num++;
	if(ASD_WLAN[wasd->WlanID] != NULL)
		ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num++;

	if((ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)
		&&(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num>ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_max_allowed_sta_num)){
		signal_wtp_deny_sta(wasd->Radio_G_ID/L_RADIO_NUM); //xm add  info trap helper
		ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->wtp_deny_sta_flag = 1;			//mahz add 2011.4.13
	}

	

	

	if(((ASD_BSS[wasd->BSSIndex]==NULL)? 0:(ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num<=ASD_BSS[wasd->BSSIndex]->bss_max_allowed_sta_num))
		&&((ASD_WLAN[wasd->WlanID]==NULL)? 0:(ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num<=ASD_WLAN[wasd->WlanID]->wlan_max_allowed_sta_num))
		&&((ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]==NULL)? 0:(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num<=ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_max_allowed_sta_num))
	){
		//xm add 09.5.13
		if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL){
			channel=ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ra_ch[wasd->Radio_G_ID%L_RADIO_NUM];
			
			if((channel!=0)&&(channel<=13)){
				if(channel_time[channel].sta_num==0){
					time(&channel_time[channel].begin_time);
					channel_time[channel].end_time=0;
					//qiuchen add it
					get_sysruntime(&channel_time[channel].begin_time_sysrun);
					channel_time[channel].end_time_sysrun=0;
					//end
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"channel begin time is %u,channel begin systime is %u  func  %s  line %d.\n",(int)channel_time[channel].begin_time,(int)channel_time[channel].begin_time_sysrun,__func__,__LINE__);
				}else{
					time_t now;
					time(&now);
					channel_time[channel].end_time=now-channel_time[channel].begin_time;
					//qiuchen add it
					time_t now_sysrun;
					get_sysruntime(&now_sysrun);
					channel_time[channel].end_time_sysrun =  now_sysrun-channel_time[channel].begin_time_sysrun;
					//end
					
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"channel begin time is %u,channel begin systime is %u  func  %s  line %d.\n",(int)channel_time[channel].begin_time,(int)channel_time[channel].begin_time_sysrun,__func__,__LINE__);
				}
				channel_time[channel].sta_num++;
			}

		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "  New STA");
		if (wasd->num_sta >= wasd->conf->max_num_sta) {
			/* FIX: might try to remove some old STAs first? */
			asd_printf(ASD_DEFAULT,MSG_WARNING, "no more room for new STAs (%d/%d)",
				   wasd->num_sta, wasd->conf->max_num_sta);
			return NULL;
		}

		sta = os_zalloc(sizeof(struct sta_info));
		if (sta == NULL) {
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			return NULL;
		}	
		
		SID = wasd->SecurityID;
		if(ASD_SECURITY[SID] == NULL)
			return NULL;
		sta->security_type = ASD_SECURITY[SID]->securityType;
		if(ASD_SECURITY[SID]->hybrid_auth == 1)
		{	
			if(ASD_SECURITY[SID]->extensible_auth == 1)
				sta->security_type = HYBRID_AUTH_EAPOL;
			else if(ASD_SECURITY[SID]->mac_auth == 1)
				sta->security_type = MAC_AUTH;
			else{				
				sta->security_type = NO_NEED_AUTH;
				if(ASD_SECURITY[SID]->securityType == OPEN)
					wasd->no_auth_sta_num++;
			}	
		}
		sta->in_addr = os_zalloc(sizeof(unsigned char)*16);
		if (sta->in_addr == NULL){
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			os_free(sta);
			sta=NULL;
			return NULL;
		}	
		os_strncpy(sta->in_addr,"0.0.0.0",7);
		sta->add_time=(time_t *)os_zalloc(sizeof(time_t));
		if(sta->add_time==NULL){
			os_free(sta->in_addr);
			sta->in_addr=NULL;
			os_free(sta);
			sta=NULL;
			return NULL;  // 0608xm
		}

		time(sta->add_time);
		//qiuchen add it 2012.10.30
		get_sysruntime(&sta->add_time_sysruntime);
		sta->sta_online_time = 0;
		//end
		sta->info_channel=channel;//xm add 09.5.13
		
		sta->acct_interim_interval = wasd->conf->radius->acct_interim_interval;

		/* initialize STA info data */
		//circle_register_timeout(wasd->conf->ap_max_inactivity, 0,
		//		       ap_handle_timer, wasd, sta);
		circle_register_timeout(10, 0, sta_auth_timer, wasd, sta);		//ht add 090219
		os_memcpy(sta->addr, addr, ETH_ALEN);
		sta->next = wasd->sta_list;
		wasd->sta_list = sta;
		wasd->num_sta++;
		ap_sta_hash_add(wasd, sta);
		asd_sta_hash_add(sta);
		sta->ssid = &wasd->conf->ssid;
		//wlan_add_sta(wasd->WlanID,sta->addr);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ADD STA in ap_sta_add\n");

		sta->vip_flag=0;
		sta->sta_traffic_limit=wasd->sta_average_traffic_limit;
		sta->sta_send_traffic_limit=wasd->sta_average_send_traffic_limit;
		sta->nRate = 110;   //fengwenchao add 20110301
		//UpdateStaInfoToWSM(wasd, addr, WID_ADD);
		if(both)
			asd_sta_add("", wasd, sta->addr,0,0,NULL,0, 0);
		memset(&sta->wapi_sta_info, 0, sizeof(sta->wapi_sta_info));
		if(asd_sta_arp_listen)
			asd_get_ip_v2(sta);
		if(asd_sta_idle_time_switch)
			circle_register_timeout(asd_sta_idle_time*3600,0,ap_sta_idle_timeout,wasd,sta);
		if(is_secondary == 1)
			wasd->authorized_sta_num++;
	/*	if(ASD_NOTICE_STA_INFO_TO_PORTAL)
			sta_flash_disconn_check(ASD_FDStas, addr);*/
		return sta;
	}
	else{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"add sta fail due to max sta number.\n");
		if(ASD_BSS[wasd->BSSIndex] != NULL)
			ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num--;
		if(ASD_WLAN[wasd->WlanID] != NULL)
			ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num--;
		
		if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)
			ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num--;

		wasd->th_deny_num++;
		return NULL;
	}
}


static int ap_sta_remove(struct asd_data *wasd, struct sta_info *sta)
{

	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Removing STA " MACSTR " from kernel driver",
		   MAC2STR(sta->addr));
	if (asd_sta_remove(wasd, sta->addr) &&
	    sta->flags & WLAN_STA_ASSOC) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not remove station " MACSTR
			   " from kernel driver.", MAC2STR(sta->addr));
		return -1;
	}
	return 0;
}


static int ap_sta_in_other_bss(struct asd_data *wasd,
			       struct sta_info *sta, u32 flags)
{
	struct asd_iface *iface = wasd->iface;
	size_t i;

	for (i = 0; i < iface->num_bss; i++) {
		struct asd_data *bss = iface->bss[i];
		struct sta_info *sta2;
		/* bss should always be set during operation, but it may be
		 * NULL during reconfiguration. Assume the STA is not
		 * associated to another BSS in that case to avoid NULL pointer
		 * dereferences. */
		if (bss == wasd || bss == NULL)
			continue;
		sta2 = ap_get_sta(bss, sta->addr);
		if (sta2 && ((sta2->flags & flags) == flags))
			return 1;
	}

	return 0;
}


void ap_sta_disassociate(struct asd_data *wasd, struct sta_info *sta,
			 u16 reason)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: disassociate STA " MACSTR,
		   wasd->conf->iface, MAC2STR(sta->addr));
	if(1 == check_sta_authorized(wasd,sta))
		wasd->authorized_sta_num--;
	sta->flags &= ~WLAN_STA_ASSOC;
	if (!ap_sta_in_other_bss(wasd, sta, WLAN_STA_ASSOC))
		ap_sta_remove(wasd, sta);
	sta->timeout_next = STA_DEAUTH;
	circle_cancel_timeout(ap_handle_timer, wasd, sta);
	//circle_register_timeout(AP_MAX_INACTIVITY_AFTER_DISASSOC, 0,
	//		       ap_handle_timer, wasd, sta);
	accounting_sta_stop(wasd, sta);
	if(ASD_SECURITY[wasd->SecurityID])
		ieee802_1x_free_alive(sta,&ASD_SECURITY[wasd->SecurityID]->eap_alive_period);
	ieee802_1x_free_station(sta);

	mlme_disassociate_indication(wasd, sta, reason);
}


void ap_sta_deauthenticate(struct asd_data *wasd, struct sta_info *sta,
			   u16 reason)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: deauthenticate STA " MACSTR,
		   wasd->conf->iface, MAC2STR(sta->addr));
	if(1 == check_sta_authorized(wasd,sta))
		wasd->authorized_sta_num--;
	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
	if (!ap_sta_in_other_bss(wasd, sta, WLAN_STA_ASSOC))
		ap_sta_remove(wasd, sta);
	sta->timeout_next = STA_REMOVE;
	circle_cancel_timeout(ap_handle_timer, wasd, sta);
	//circle_register_timeout(AP_MAX_INACTIVITY_AFTER_DEAUTH, 0,
	//		       ap_handle_timer, wasd, sta);
	accounting_sta_stop(wasd, sta);
	if(ASD_SECURITY[wasd->SecurityID])
		ieee802_1x_free_alive(sta,&ASD_SECURITY[wasd->SecurityID]->eap_alive_period);
	ieee802_1x_free_station(sta);

	mlme_deauthenticate_indication(wasd, sta, reason);
}


int ap_sta_bind_vlan(struct asd_data *wasd, struct sta_info *sta,
		     int old_vlanid)
{
	const char *iface;
	struct asd_vlan *vlan = NULL;

	/*
	 * Do not proceed furthur if the vlan id remains same. We do not want
	 * duplicate dynamic vlan entries.
	 */
	if (sta->vlan_id == old_vlanid)
		return 0;

	/*
	 * During 1x reauth, if the vlan id changes, then remove the old id and
	 * proceed furthur to add the new one.
	 */
	if (old_vlanid > 0)
		vlan_remove_dynamic(wasd, old_vlanid);

	iface = wasd->conf->iface;
	if (sta->ssid->vlan[0])
		iface = sta->ssid->vlan;

	if (sta->ssid->dynamic_vlan == DYNAMIC_VLAN_DISABLED)
		sta->vlan_id = 0;
	else if (sta->vlan_id > 0) {
		vlan = wasd->conf->vlan;
		while (vlan) {
			if (vlan->vlan_id == sta->vlan_id ||
			    vlan->vlan_id == VLAN_ID_WILDCARD) {
				iface = vlan->ifname;
				break;
			}
			vlan = vlan->next;
		}
	}

	if (sta->vlan_id > 0 && vlan == NULL) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG, "could not find VLAN for "
			       "binding station to (vlan_id=%d)",
			       sta->vlan_id);
		return -1;
	} else if (sta->vlan_id > 0 && vlan->vlan_id == VLAN_ID_WILDCARD) {
		vlan = vlan_add_dynamic(wasd, vlan, sta->vlan_id);
		if (vlan == NULL) {
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE80211,
				       asd_LEVEL_DEBUG, "could not add "
				       "dynamic VLAN interface for vlan_id=%d",
				       sta->vlan_id);
			return -1;
		}

		iface = vlan->ifname;
		if (vlan_setup_encryption_dyn(wasd, sta->ssid, iface) != 0) {
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE80211,
				       asd_LEVEL_DEBUG, "could not "
				       "configure encryption for dynamic VLAN "
				       "interface for vlan_id=%d",
				       sta->vlan_id);
		}

		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG, "added new dynamic VLAN "
			       "interface '%s'", iface);
	} else if (vlan && vlan->vlan_id == sta->vlan_id) {
		if (sta->vlan_id > 0) {
			vlan->dynamic_vlan++;
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE80211,
				       asd_LEVEL_DEBUG, "updated existing "
				       "dynamic VLAN interface '%s'", iface);
		}

		/*
		 * Update encryption configuration for statically generated
		 * VLAN interface. This is only used for static WEP
		 * configuration for the case where asd did not yet know
		 * which keys are to be used when the interface was added.
		 */
		if (vlan_setup_encryption_dyn(wasd, sta->ssid, iface) != 0) {
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE80211,
				       asd_LEVEL_DEBUG, "could not "
				       "configure encryption for VLAN "
				       "interface for vlan_id=%d",
				       sta->vlan_id);
		}
	}

	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG, "binding station to interface "
		       "'%s'", iface);

	if (wpa_auth_sta_set_vlan(sta->wpa_sm, sta->vlan_id) < 0)
		asd_printf(ASD_DEFAULT,MSG_WARNING, "Failed to update VLAN-ID for WPA");

	return asd_set_sta_vlan(iface, wasd, sta->addr, sta->vlan_id);
}

void ap_free_sta_for_pmk(struct asd_data *wasd, struct sta_info *sta)
{		
		int set_beacon = 0;	
		
		if((wasd == NULL) || (sta == NULL)){
			asd_printf(ASD_DEFAULT,MSG_INFO,"wasd or sta is NULL in %s\n",__func__);
			return ;
		}
		
	
		if(1 == check_sta_authorized(wasd,sta))
			wasd->authorized_sta_num--;
		circle_cancel_timeout(sta_auth_timer, wasd, sta);
		circle_cancel_timeout(ap_handle_timer, wasd, sta);
		circle_cancel_timeout(ap_handle_session_timer, wasd, sta);
		circle_cancel_timeout(ap_sta_idle_timeout,wasd,sta);
		circle_cancel_timeout(ap_sta_eap_auth_timer,wasd,sta);
		if(ASD_BSS[wasd->BSSIndex])
			ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num--;
		if(ASD_WLAN[wasd->WlanID])
			ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num--;
		if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]!=NULL)
			ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num--;
		
		if((sta->acct_session_started)&&(sta->acct_terminate_cause == 0))
			sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		accounting_sta_stop(wasd, sta);		
		if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL == sta->security_type)){
			if(sta->ipaddr != 0)
			{
				if(asd_ipset_switch)
					eap_connect_down(sta->ipaddr);
				else
					AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
			}
		}
		ap_sta_hash_del(wasd, sta);
	//	asd_sta_hash_del(sta);
		ap_sta_list_del(wasd, sta);

		if (sta->aid > 0)
			wasd->sta_aid[sta->aid - 1] = NULL;

		wasd->num_sta--;
		if (sta->nonerp_set) {
			sta->nonerp_set = 0;
			wasd->iface->num_sta_non_erp--;
			if (wasd->iface->num_sta_non_erp == 0)
				set_beacon++;
		}

		if (sta->no_short_slot_time_set) {
			sta->no_short_slot_time_set = 0;
			wasd->iface->num_sta_no_short_slot_time--;
			if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
			    && wasd->iface->num_sta_no_short_slot_time == 0)
				set_beacon++;
		}

		if (sta->no_short_preamble_set) {
			sta->no_short_preamble_set = 0;
			wasd->iface->num_sta_no_short_preamble--;
			if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
			    && wasd->iface->num_sta_no_short_preamble == 0)
				set_beacon++;
		}

		if (set_beacon)
			ieee802_11_set_beacons(wasd->iface);

		unsigned char SID = wasd->SecurityID;
		if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)&&(ASD_SECURITY[SID]->hybrid_auth == 0))){
			if(ASD_SECURITY[wasd->SecurityID])
				ieee802_1x_free_alive(sta,&ASD_SECURITY[wasd->SecurityID]->eap_alive_period);
			ieee802_1x_free_station(sta);
			wpa_auth_sta_deinit(sta->wpa_sm);
			rsn_preauth_free_station(wasd, sta);
//			radius_client_flush_auth(wasd->radius, sta->addr);
		}
		if(STA_STATIC_FDB_ABLE && wasd->bss_iface_type){
			char ifname[IF_NAME_MAX]={0};
#ifndef _DISTRIBUTION_				
			sprintf(ifname,"radio%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
#else
			if(local){
				sprintf(ifname,"r%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
			}else{
				sprintf(ifname,"r%d-%d-%d-%d.%d",slotid,vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
			}
#endif
			add_and_del_static_br_fdb(wasd->br_ifname,ifname, sta->addr,0) ;
		}
		asd_printf(ASD_DEFAULT,MSG_INFO,"free sta in ap_free_sta_for_pmk\n");
		os_free(sta->last_assoc_req);
		sta->last_assoc_req=NULL;
		os_free(sta->in_addr);
		sta->in_addr=NULL;
		os_free(sta->challenge);
		sta->challenge=NULL;
		os_free(sta->add_time);
		sta->add_time=NULL;
		os_free(sta);
		sta=NULL;
		
}


int kick_sta_mac(unsigned char mac[]){

	if(mac==NULL)
		return -1;
	struct sta_info *sta=NULL;
	int bss_num=0;
	int i=0 , sta_flag=0;
	//struct asd_data *bss[BSS_NUM];
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return -1;
	}

	
	bss_num= ASD_SEARCH_BSS_HAS_STA(bss);
	
	for(i=0;i<bss_num;i++){
		sta = ap_get_sta(bss[i], mac);
		if(sta != NULL){
			if(1 == check_sta_authorized(bss[i],sta))
				bss[i]->authorized_sta_num--;
			sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
			sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
			unsigned char SID = (unsigned char)bss[i]->SecurityID;
			if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
				wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
				mlme_deauthenticate_indication(
				bss[i], sta, 0);
				ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
			}
			if(ASD_WLAN[bss[i]->WlanID]!=NULL&&ASD_WLAN[bss[i]->WlanID]->balance_switch == 1&&ASD_WLAN[bss[i]->WlanID]->balance_method==1)
				ap_free_sta(bss[i], sta, 1);
			else
				ap_free_sta(bss[i], sta, 0);
	/********************************************************************
	*		Let sta known it must stop connecting.
	*		reason code 3---Deauthenticated because sending STA 
	*					     is leaving (or has left) IBSS or ESS
	*		xm add 08/11/03
	*********************************************************************/
			ieee802_11_send_deauth(bss[i], mac, 3);
			//add_mac_in_maclist(bss[i]->conf,mac,0); //add in black list		
			sta_flag=1;
			{//let wid know this sta is not allowed.
				TableMsg STA;
				int len;
				STA.Op = WID_DEL;
				STA.Type = STA_TYPE;
				STA.u.STA.BSSIndex = bss[i]->BSSIndex;
				STA.u.STA.WTPID = ((bss[i]->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
				memcpy(STA.u.STA.STAMAC, mac, ETH_ALEN);
				len = sizeof(STA);
				if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
					perror("send(wASDSocket)");
				}
			}
		}
	}
	if(bss){
		free(bss);
		bss = NULL;
	}
	return 0;
}

int add_and_del_static_br_fdb(const char *brname,const char *ifname, const unsigned char * addr,int isadd)	
{
	int br_socket_fd = -1;
	
	struct ifreq ifr;
	unsigned int args[8] = {0};

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"brname %s,ifrname %s,add %d\n",brname,ifname,isadd);
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		asd_printf(ASD_DEFAULT,MSG_INFO,"Socket create failed:%s.\n",strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(br_socket_fd, SIOCGIFINDEX, &ifr) < 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO,"ioctl SIOCGIFINDEX failed:%s.\n",strerror(errno));
		close(br_socket_fd);
		return -1;
	}else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ifrname %s ifindex %d.\n",ifr.ifr_name,ifr.ifr_ifindex);
	}
	
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	args[0] = 0;
	args[1] = isadd ? BRCTL_ADD_SFDB : BRCTL_DEL_SFDB;
	args[2] = 0;
	args[3] = ifr.ifr_ifindex;
	memcpy(&args[4], addr, MAC_LEN);
	ifr.ifr_data = (void *)&args;
	//ifr.ifr_ifru.ifru_data = &args;

	if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		asd_printf(ASD_DEFAULT,MSG_INFO,"ioctl SETFDB failed:%s.\n",strerror(errno));
		close(br_socket_fd);
		return -1;
	}else
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ioctl SETFDB "MACSTR" successed.\n",MAC2STR(addr));
	
	close(br_socket_fd);
	return 0;
}
void ap_sta_idle_timeout(void *circle_ctx,void *timeout_ctx)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"*************now in func :%s************\n",__func__);
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	unsigned int BssIndex = 0;
	unsigned char SID = 0;
	u8 mac[MAC_LEN];
	if((wasd == NULL)||(sta == NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"wasd or sta is NULL in func:%s\n",__func__);
		return;
	}
	memset(mac,0,MAC_LEN);
	BssIndex = wasd->BSSIndex;
	memcpy(mac,sta->addr,MAC_LEN);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"BssIndex = %d\n",BssIndex);
		
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Removing station " MACSTR"  because of idle timeout !",
		   MAC2STR(sta->addr));
	if (sta->flags & WLAN_STA_AUTH) {
		mlme_deauthenticate_indication(
			wasd, sta, WLAN_REASON_UNSPECIFIED);
	}
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
	SID = wasd->SecurityID;
	if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
		wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
		mlme_deauthenticate_indication(
		wasd, sta, 0);
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	}
	
	if(ASD_NOTICE_STA_INFO_TO_PORTAL)
		AsdStaInfoToEAG(wasd,sta,WID_DEL);
	/*if(ASD_NOTICE_STA_INFO_TO_PORTAL)
		flashdisconn_sta_add(ASD_FDStas, sta->addr,wasd->BSSIndex,wasd->WlanID);	*/
	if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
		ap_free_sta(wasd, sta,1);
	}
	else{
		ap_free_sta(wasd, sta,0);
	}
	AsdStaInfoToWID(wasd,mac,WID_DEL);
	if(is_secondary == 1)
		bak_check_sta_req(asd_sock,BssIndex,mac);
}
int check_sta_authorized(struct asd_data *wasd,struct sta_info *sta)
{
	if(!wasd||!sta)
		return 0;
	unsigned char SID = wasd->SecurityID;
	if(!ASD_SECURITY[SID])
		return 0;
	if((ASD_SECURITY[SID]->extensible_auth == 0)&&((ASD_SECURITY[SID]->securityType == OPEN)||(ASD_SECURITY[SID]->securityType == SHARED)))
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->flags = %d\n",sta->flags);
		if(sta->flags&WLAN_STA_ASSOC)
			return 1;
	}
	/*extensible_auth  =1 ,sta can get the ip,should notice ap; but sta is not authorized*/
	else if((ASD_SECURITY[SID]->extensible_auth == 1)&&(ASD_SECURITY[SID]->hybrid_auth == 1))
	{
		if(sta->flags&WLAN_STA_ASSOC)
			return 1;
	}	
	else if(sta->flags&WLAN_STA_AUTHORIZED)
		return 1;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta :"MACSTR" has not authorized\n",MAC2STR(sta->addr));
	return 0;
}

struct sta_info *ap_get_sta_acct_id(const unsigned char *acct_id)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s   acct_id:%s\n",__func__,acct_id);
	if(acct_id == NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s : acct_id is NULL,will return!\n",__func__);
		return NULL;
	}
	struct sta_info *sta = NULL;
	struct asd_data **bss = NULL;
	int i ,bss_num;
	unsigned char SID;
	char buf[256]={0};
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}	
	bss_num= ASD_SEARCH_ALL_STA(bss);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss_num:%d\n",bss_num);
	for(i = 0 ; i < bss_num ; i++ )
	{		
		if(bss[i] == NULL)
			continue;
		SID = bss[i]->SecurityID;
		if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)))
		{}
		else
			continue;
		if(bss[i]->sta_list != NULL)
		{
			sta = bss[i]->sta_list;
			while(sta!=NULL)
			{			
				os_snprintf(buf, sizeof(buf), "%08X-%08X", sta->acct_session_id_hi, sta->acct_session_id_lo);			
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta acct_id: %s\n",buf); 	
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive acct_id:%s\n",acct_id);
				if((os_memcmp(acct_id, buf, ACCT_ID_LEN)) == 0)		
				{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"memcmp recieve acct_id:%s\n",acct_id);	
					if(bss)
					{
						free(bss);
						bss = NULL;
					}
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"get the sta :"MACSTR"\n",MAC2STR(sta->addr));
					return sta;
				}
					sta = sta->next;
			}
		}
	}
	if(bss)
	{
		free(bss);
		bss = NULL;
	}
	return NULL;
}
int ap_kick_eap_sta(struct sta_info *sta)
{
	if((sta == NULL)||(sta->eapol_sm == NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"sta is NULL,or sta is not a eap sta!\n");
		return -1;
	}
	struct asd_data *wasd = sta->eapol_sm->wasd;
	if(wasd == NULL)
		return -1;
	
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
	wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
	mlme_deauthenticate_indication(
	wasd, sta, 0);
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	//notice to ap
	ieee802_11_send_deauth(wasd, sta->addr, 3);
	AsdStaInfoToWID(wasd,sta->addr,WID_DEL);
	if(ASD_NOTICE_STA_INFO_TO_PORTAL)
		AsdStaInfoToEAG(wasd,sta,WID_DEL);
	if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
		ap_free_sta(wasd, sta, 1);
	}
	else{				
		ap_free_sta(wasd, sta, 0);
	}
	return 0;
}
/*
int radius_dm_kick_sta(const unsigned char *acct_id)
{
	if(acct_id == NULL)
		return -1;
	struct sta_info *sta = NULL;
	struct asd_data **bss = NULL;
	int i ,bss_num;
	unsigned char SID;
	char buf[128]={0};
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return -1;
	}	
	bss_num= ASD_SEARCH_BSS_HAS_STA(bss);
	for(i = 0 ; i < bss_num ; i++ )
	{		
		if(bss[i] == NULL)
			continue;
		SID = bss[i]->SecurityID;
		if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)))
		{}
		else
			continue;
		if(bss[i]->sta_list != NULL)
		{
			sta = bss[i]->sta_list;
			while(sta!=NULL)
			{			
				os_snprintf(buf, sizeof(buf), "%08X-%08X", sta->acct_session_id_hi, sta->acct_session_id_lo);			
				asd_printf(ASD_DBUS,MSG_DEBUG,"acct_id: %s\n",buf); 	
				if(os_memcmp(acct_id, buf, strlen((char *)acct_id)) == 0)
				{
					sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
					unsigned char SID = (unsigned char)bss[i]->SecurityID;
					if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)&&(ASD_SECURITY[SID]->hybrid_auth == 0))){
						wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
						bss[i], sta, 0);
						sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
						ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
					}
					//notice to ap
					ieee802_11_send_deauth(bss[i], sta->addr, 3);
					AsdStaInfoToWID(bss[i],sta->addr,WID_DEL);
					if(ASD_WLAN[bss[i]->WlanID]!=NULL&&ASD_WLAN[bss[i]->WlanID]->balance_switch == 1&&ASD_WLAN[bss[i]->WlanID]->balance_method==1){
						if(ASD_NOTICE_STA_INFO_TO_PORTAL)
							UpdateStaInfoToCHILL(bss[i]->BSSIndex,sta,WID_DEL); 
						ap_free_sta(bss[i], sta, 1);
					}
					else{				
						if(ASD_NOTICE_STA_INFO_TO_PORTAL)
							UpdateStaInfoToCHILL(bss[i]->BSSIndex,sta,WID_DEL);
						ap_free_sta(bss[i], sta, 0);
					}
					return 0;
				}
					sta = sta->next;
			}
		}
	}
	return -1;
}
*/
struct asd_data *get_wasd_by_radius_ip(const unsigned int ip,const int port)
{
	struct asd_data * wasd = NULL;	
	struct asd_data **bss = NULL;
	int i,bss_num;
	unsigned char SID = 0;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}	
	bss_num= ASD_SEARCH_ALL_STA(bss);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss_num = %d\n",bss_num);
	for(i = 0 ; i < bss_num ; i++)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"i = %d\n",i);
		if(NULL == bss[i])
			continue;
		SID = bss[i]->SecurityID;	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"SECURITY :%d\n",ASD_SECURITY[SID]->securityType);
		if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)))
		{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss ip:%d,port :%d   receive ip%d port %d\n",ip,port,bss[i]->radius->conf->acct_server->addr.u.v4.s_addr,bss[i]->radius->conf->acct_server->port);
			if(bss[i]&&bss[i]->radius&&bss[i]->radius->conf
			 &&bss[i]->radius->conf->acct_server->addr.u.v4.s_addr == ip)
			{
				wasd = bss[i];
				asd_printf(ASD_DEFAULT,MSG_DEBUG," %s :find the wasd(bssindex:%d)!\n",__func__,wasd->BSSIndex);
				break;
			}
		}
	}

	if(bss)
	{
		free(bss);
		bss = NULL;
	}
	return wasd;
}
struct sta_info * asd_sta_hash_get(const u8 *sta)
{
	struct sta_info *s = NULL;

	s = ASD_STA_HASH[STA_GLOBAL_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->g_hnext;
	return s;
}

void asd_sta_hash_add(struct sta_info *sta)
{
 	asd_sta_hash_del(sta);
 	sta->g_hnext = ASD_STA_HASH[STA_GLOBAL_HASH(sta->addr)];
	ASD_STA_HASH[STA_GLOBAL_HASH(sta->addr)] = sta;
 }


static void asd_sta_hash_del(struct sta_info *sta)
{
	struct sta_info *s = NULL;
 	s = ASD_STA_HASH[STA_GLOBAL_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		ASD_STA_HASH[STA_GLOBAL_HASH(sta->addr)] = s->g_hnext;
		return;
	}

	while (s->g_hnext != NULL &&
	       os_memcmp(s->g_hnext->addr, sta->addr, ETH_ALEN) != 0)
		s = s->g_hnext;
	if (s->g_hnext != NULL)
		s->g_hnext = s->g_hnext->g_hnext;
	else
		asd_printf(ASD_DEFAULT,MSG_ERROR, "GLOBAL HASH: could not remove STA " MACSTR
			   " from hash table", MAC2STR(sta->addr));
	
}
unsigned int sta_acct_get_key(u8 *acct_id)
{
	unsigned int index = 0;
	char tmp[ACCT_ID_LEN] = {0};
	os_memcpy(tmp,acct_id+ACCT_ID_LEN-8,8);
	index = strtoul(tmp,NULL,16);
	return index;
}
struct sta_acct_info *sta_acct_info_get(u8 *acct_id)
{
	struct sta_acct_info *acct_info = NULL;
	unsigned int index = 0;
	index = sta_acct_get_key(acct_id);
	acct_info =  ASD_ACCT_HASH[index];
	while(acct_info!=NULL &&os_memcmp(acct_info->acct_id,acct_id,ACCT_ID_LEN)!= 0)
		acct_info = acct_info->next;
	return acct_info;
}
void sta_acct_info_add(struct sta_info *sta)
{
	if(!sta)
		return;
	struct sta_acct_info *acct_info = NULL;
	u8 acct_id[ACCT_ID_LEN] = {0};
	u32 index = 0;
	os_snprintf((char *)acct_id, sizeof(acct_id), "%08X-%08X", sta->acct_session_id_hi, sta->acct_session_id_lo);			
	acct_info = sta_acct_info_get(acct_id);
	if(acct_info !=NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"acct_info:%s is exist will only init!\n",acct_info->acct_id);
		goto init;
	}
	acct_info = os_zalloc(sizeof(struct sta_acct_info));
	if(!acct_info)
	{
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s:malloc error!\n",__func__);
		return;
	}
init:	
	memset(acct_info,0,sizeof(struct sta_acct_info));
	os_snprintf((char *)acct_info->acct_id, sizeof(acct_info->acct_id), "%08X-%08X", sta->acct_session_id_hi, sta->acct_session_id_lo);			
	os_memcpy(acct_info->addr,sta->addr,MAC_LEN);
	
	index = sta->acct_session_id_lo;
	acct_info->next = ASD_ACCT_HASH[index];
	ASD_ACCT_HASH[index] = acct_info;
}
void sta_acct_info_del(u8 *acct_id)
{
	struct  sta_acct_info*s = NULL;
	unsigned int index;
	index = sta_acct_get_key( acct_id);
	s = ASD_ACCT_HASH[index];
	if (s == NULL) 
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"%s:acct_id(lo) %d is not exist in HASH!\n",__func__,index);
		return;
	}
	if (os_memcmp(s->acct_id, acct_id, ACCT_ID_LEN) == 0) {
		ASD_ACCT_HASH[index] = s->next;
		goto out;
	}
	while (s->next != NULL &&
	       os_memcmp(s->next->acct_id, acct_id, ACCT_ID_LEN) != 0)
		s = s->next;
	if (s->next != NULL)
		s->next = s->next->next;
	else{
		asd_printf(ASD_DEFAULT,MSG_ERROR, "GLOBAL HASH: could not remove acct_info %s", acct_id);
		return;
	}
out:
	if(s)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s  del acct_id %s\n",__func__,s->acct_id);
		s->next = NULL;
		os_free(s);
		s = NULL;
	}
	return;
}
int AsdStaInfoToEAG(struct asd_data *wasd, struct sta_info *sta, Operate op){
	if(!wasd||!sta)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s wasd or sta is NULL!\n",__func__);
		return -1;
	}
	if(0 == ASD_NOTICE_STA_INFO_TO_PORTAL)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"ASD_NOTICE_STA_INFO_TO_PORTAL is changed!\n");
		return -1;
	}
	EagMsg msg;
	memset(&msg,0,sizeof(msg));
	unsigned int wtpid = 0;
	unsigned int len = 0;
	wtpid = (wasd->BSSIndex)/L_BSS_NUM/L_RADIO_NUM;

	msg.Op = op;
	msg.Type = EAG_TYPE;
	os_memcpy(msg.STA.addr,sta->addr,MAC_LEN);
	msg.STA.radio_id = wasd->Radio_G_ID;
	msg.STA.ipaddr = sta->ipaddr;
	msg.STA.wlan_id = wasd->WlanID;
	msg.STA.auth_type = sta->security_type;
	msg.STA.reason = sta->acct_terminate_cause;

	if(NULL != sta->ssid)
		os_memcpy(msg.STA.essid,sta->ssid->ssid,sta->ssid->ssid_len);
	if(NULL != ASD_WTP_AP[wtpid])
		os_memcpy(msg.STA.wtp_mac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	if(NULL != ASD_BSS[wasd->BSSIndex])
		msg.STA.vlan_id = ASD_BSS[wasd->BSSIndex]->vlanid;

	msg.STA.tx_data_bytes = sta->txbytes;
	msg.STA.rx_data_bytes =sta->rxbytes;
	msg.STA.tx_frames = (unsigned long)sta->txpackets;
	msg.STA.rx_frames = (unsigned long)sta->rxpackets;	

	len = sizeof(msg);
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"AsdStaInfoToEAG \n");
	if(sendto(TableSend, &msg, len, 0, (struct sockaddr *) &toEAG.addr, toEAG.addrlen) < 0){
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s sendtoEAG %s\n",__func__,strerror(errno));
		perror("send(AsdStaInfoToEAG)");
		return -1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA.op = %d\n",msg.Op);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta: "MACSTR"\n",MAC2STR(msg.STA.addr));
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"reason = %d\n",msg.STA.reason );
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"auth_type = %d\n",msg.STA.auth_type );
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"essid : %s\n",msg.STA.essid);
	return 0;
}
