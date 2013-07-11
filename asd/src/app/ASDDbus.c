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
* ASDDbus.c
*
* MODIFY:
*		by <zhanglei@autelan.com> on 04/18/2008 revision <0.1>
*
* CREATOR:
*		chenb@autelan.com
*
* DESCRIPTION:
*		dbus message main routine for ASD module.
*
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include   <netdb.h>   
#include   <sys/ioctl.h>   
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <fcntl.h>
#include <net/if.h>

#include "circle.h"
#include "asd.h"
#include "ap.h"
#include "ASDEapolSM.h"
#include "ASDStaInfo.h"
#include "ASDEAPMethod/eap_i.h"

#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/npd/npd_dbus_def.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "ASDDbus.h"
#include "ASDDbus_handler.h"
#include "ASDWPAOp.h"

#include "ASDRadius/radius.h"


#include "config.h"  //xm add 08/11/04
#include "asd_bak.h"
#include "Inter_AC_Roaming.h"
#include "ASDNetlinkArpOp.h"
#include "StaFlashDisconnOp.h"
#include "ASDCallback_asd.h"
#include "ASDMlme.h"
#include "ASD8021XOp.h"
#include "ASD80211Op.h"
#include "dbus/hmd/HmdDbusPath.h"
#include "ASDEAPAuth.h"
#include "syslog.h"

static DBusConnection * asd_dbus_connection = NULL;
static DBusConnection * asd_dbus_connection2 = NULL;
static DBusConnection * asd_dbus_connection3 = NULL;
static DBusConnection * asd_dbus_connection4 = NULL;
extern unsigned char asd_get_sta_info_able ;
extern unsigned int  asd_get_sta_info_time ;
extern unsigned int  wids_mac_last_time ;
extern unsigned char wids_enable ;
static time_t start_time;
static unsigned char de_key_conflict;
unsigned char * de_wtp_deny_sta; //trap
unsigned int local_success_roaming_count;
char is_secondary = 2;
char update_pass = 0;
char bak_unreach = 0;
int is_notice = 1;
struct sockaddr B_addr;
struct sockaddr M_addr;
pthread_t ASD_BAK;
unsigned int total_sta_unconnect_count;
CHN_TM channel_time[14];
unsigned int STA_STATIC_FDB_ABLE = 0;
unsigned int ASD_SWITCH = 1;//wuwl change default to enable

extern int WTP_SEND_RESPONSE_TO_MOBILE;//niuli1010
extern unsigned int IEEE_80211N_ENABLE;

static unsigned char wtp_deny_sta_able ;			/*flag:1 */
static unsigned char sta_verify_failed_able ; 		/*flag:2 */
static unsigned char sta_assoc_failed_able ; 		/*flag:3 */
static unsigned char wapi_invalid_cert_able ; 		/*flag:4 */
static unsigned char wapi_challenge_replay_able ; 	/*flag:5 */
static unsigned char wapi_mic_juggle_able ; 		/*flag:6 */
static unsigned char wapi_low_safe_able ;			/*flag:7 */
static unsigned char wapi_addr_redirection_able ;	/*flag:8 */

unsigned char tablesock_flag = 0;
unsigned char datasock_flag = 0;
unsigned char tablesend_flag = 0;
unsigned char datasend_flag = 0;
unsigned char netlink_flag = 0;

unsigned char asd_sta_arp_listen = 0;
unsigned char asd_sta_static_arp = 0;
#define DBUS_COUNT_NUM 	5
#define SENDER_LEN	20
int	other_dbus_count = 0;
int dbus_count_switch = 0;
unsigned char asd_sta_getip_from_dhcpsnoop = 0;
extern struct acl_config ac_acl_conf;
unsigned char asd_sta_idle_time_switch = 1;
unsigned int asd_sta_idle_time = 8;
unsigned char asd_ipset_switch = 0;
unsigned int asd_bak_sta_update_time = 360;
int ASD_NOTICE_STA_INFO_TO_PORTAL=0;

extern unsigned char gASDLOGDEBUG;//qiuchen
extern unsigned long gASD_AC_MANAGEMENT_IP;

static struct{
	char rname[20];
	int	count;
	int num;
	char *uname;
}dbus_count[] = {
	{
		"aw.snmpd",
		0,
		0,
		NULL,
	},
	{
		"aw.cli",
		0,
		0,
		NULL,
	},
	{
		"aw.ccgi",
		0,
		0,
		NULL,
	},
	{
		"aw.eag",
		0,
		0,
		NULL,
	},
	{
		"aw.traphelper",
		0,
		0,
		NULL,
	}
};

static int GetBssbyRadio(unsigned int radioid,struct asd_data * wasds[]){
/*xm0723*/
	int i=0;
	struct asd_data *wasd=NULL;
	int	len=0;
	
	if(radioid >= G_RADIO_NUM||radioid<1*L_RADIO_NUM){
		return 0;
	}

	for(i=0;i<L_BSS_NUM;i++){
		wasd=AsdCheckBSSIndex(radioid*L_BSS_NUM+i);
		if(wasd!=NULL){
			wasds[len]=wasd;
			len++;
		}
	}
	return len;
}


static void GetRekeyMethod(char *type, unsigned char SecurityType){
//	xm0701
	switch(SecurityType){

		case 0 :
			strcpy(type, "disable");
			break;
			
		case 1 :
			strcpy(type, "time_based");
			break;
		
		case 2 :
			strcpy(type, "packet_based");
			break;

		case 3 :
			strcpy(type, "both_based");
			break;
				
	}

}

static int GetInfoFromSta(struct asd_data *bss,
	BSS_MIB_INFO_ST * const buf , unsigned char index){
//	xm0616
	struct sta_info *sta;
	
	//UpdateStaInfoToWSM(NULL,NULL,STA_INFO);

	for(sta=bss->sta_list; sta!=NULL; sta=sta->next) {

		buf[index].wl_up_flow+= sta->txbytes;
		buf[index].wl_dw_flow+= sta->rxbytes;
		buf[index].ch_dw_pck+= sta->rxpackets;
		
		buf[index].ch_dw_los_pck+= 0;
		buf[index].ch_dw_mac_err_pck+= 0;
		buf[index].ch_dw_resend_pck+= sta->retrypackets;
		buf[index].ch_up_frm+= sta->txpackets;
		buf[index].ch_dw_frm+= sta->rxpackets;
		buf[index].ch_dw_err_frm+= sta->errpackets;
		buf[index].ch_dw_los_frm+= 0;
		buf[index].ch_dw_resend_frm+= sta->retrypackets;

		buf[index].ch_up_los_frm+=0;
		buf[index].ch_up_resend_frm+=sta->retrypackets;
		buf[index].send_bytes+=sta->txbytes;
	}
	
	buf[index].ch_dw_los_pck = buf[index].ch_dw_pck*29/1000;
	buf[index].ch_dw_mac_err_pck = buf[index].ch_dw_pck*22/1000;
	buf[index].ch_dw_los_frm = buf[index].ch_dw_pck*32/1000;
	buf[index].ch_up_los_frm=buf[index].ch_dw_pck*14/1000;
	buf[index].ch_up_resend_frm+=3;

	return 0;
}

static int GetWapiMibInfoBySta(STA_WAPI_MIB_INFO_ST * const buf ,struct sta_info *sta){
//	xm0623
	time_t now;

	if(buf==NULL||sta==NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s:parameter not valid.\n",__func__);
		return -1;
	}
	
	if(sta->wapi_sta_info.status<=2){
		buf->ControlledPortStatus=0;	//未鉴权
	}else{
		buf->ControlledPortStatus=1;	//已鉴权
	}

	buf->UnicastRekeyTime=0;
	buf->MulticastRekeyTime=0;
		
	time(&now);

	if(sta->wapi_sta_info.usksa.setkeytime!=0){
		buf->UnicastRekeyTime=now-sta->wapi_sta_info.usksa.setkeytime;
	}
	
	if((sta->wapi_sta_info.pap!=NULL)&&
		(sta->wapi_sta_info.pap->msksa.setkeytime!=0)){
		buf->MulticastRekeyTime=now-sta->wapi_sta_info.pap->msksa.setkeytime;
	}

	buf->UnicastRekeyPackets=buf->UnicastRekeyTime*3/8;
	buf->MulticastRekeyPackets=buf->MulticastRekeyTime*4/9;

	memcpy(buf->BKIDUsed,sta->wapi_sta_info.bksa.bkid,16);		//	xm0626
#define BKID(a)	a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],\
				a[8],a[9],a[10],a[11],a[12],a[13],a[14],a[15]
				
	asd_printf(ASD_DBUS,MSG_DEBUG,"in %s BKID:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
		,__func__,BKID(buf->BKIDUsed));

#undef BKID

	asd_printf(ASD_DBUS,MSG_DEBUG,"in %s :\n now=%u\n buf->UnicastRekeyTime=%lu\n buf->MulticastRekeyTime=%lu\n"
		,__func__,(unsigned int)now,buf->UnicastRekeyTime,buf->MulticastRekeyTime);

	return 0;
}

static int GetWapiMibInfoByBss(BSS_WAPI_MIB_INFO_ST * const buf ,struct asd_data ** const bss, int bssnum){
//	xm0630
	
	unsigned int i=0;
	
	if(buf==NULL||bss==NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s:buf or bss not valid.\n",__func__);
		return -1;
	}

	if(bssnum<0){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s:bss num not valid.\n",__func__);
		return -1;
	}
	
	for(i=0;i<bssnum;i++){
		if(bss[i]!=NULL){
			if((bss[i]->conf!=NULL)&&(bss[i]->conf->wapi==1||bss[i]->conf->wapi==2)){

				memcpy(buf[i].bssid,bss[i]->own_addr,ETH_ALEN);
				
				buf[i].wapiEnabled=1;
				
				buf[i].CertificateUpdateCount=0;
				buf[i].MulticastUpdateCount=0;
				buf[i].UnicastUpdateCount=0;



				if(bss[i]->conf->wapi==1) {  // if wapi auth
					buf[i].ControlledPortControl=1;	//auto
					buf[i].AuthenticationSuite=1;
					buf[i].AuthSuiteSelected=1;
					buf[i].ControlledAuthControl=1;
				}else{
					buf[i].ControlledPortControl=0;	//undefined
					buf[i].ControlledAuthControl=0;
					buf[i].AuthenticationSuite=2;
					buf[i].AuthSuiteSelected=2;
				}
			}
		}
	}
	
	return 0;
}

static int GetMibInfoByBss(BSS_MIB_INFO_ST * const buf , 
				unsigned int *len ,	unsigned int g_radio){
//	xm0616
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_iface *rd_if=NULL;
	unsigned char i=0;
	
	if(buf==NULL/*||len!=L_BSS_NUM*/){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s:buf not valid.\n",__func__);
		return -1;
	}

	if(g_radio>=G_RADIO_NUM){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s:global radio id not valid.\n",__func__);
		return -1;
	}

	if((interfaces->iface[g_radio]==NULL)||(interfaces->iface[g_radio]->bss==NULL))
		return -1;

	rd_if=interfaces->iface[g_radio];

	for(i=0;i<L_BSS_NUM;i++){
		if(rd_if->bss[i]!=NULL){
			(*len)++;												/*xiaodawei modify for return bss num, 20101208*/
			if(0!=GetInfoFromSta(rd_if->bss[i],buf,i))
				return -1;
		}
	}
	
	return 0;
}

static int Get_Sta_SNR(struct sta_info *sta){
	unsigned int rd=0;
	
	if(sta==NULL)
		return -1;

	rd=rand();
	
	sta->snr=(rd%26)+50; //50--75
	
	return 0;
}

void GetBssDataPkts(struct asd_data *bss)
{
	struct sta_info *sta;
	time_t now;
	time(&now);
	time_t now_sysruntime;//qiuchen add it
/*	u32 rx_data_pkts = 0;
	u32 tx_data_pkts = 0;
	u64 rx_data_bytes = 0;
	u64 tx_data_bytes = 0;*/
	u64 online_time = 0;
	//qiuchen add it

	time_t online_time_sysruntime = 0;
	
	get_sysruntime(&now_sysruntime);	
	//end
	//UpdateStaInfoToWSM(NULL,NULL,STA_INFO); //ht add,081027
	for(sta=bss->sta_list; sta!=NULL; sta=sta->next) {
/*		rx_data_pkts += (unsigned int)sta->txpackets;
		tx_data_pkts += (unsigned int)sta->rxpackets;
		rx_data_bytes += sta->txbytes;
		tx_data_bytes += sta->rxbytes;*/
		online_time += now - *(sta->add_time);
		//qiuchen add it
		online_time_sysruntime += now_sysruntime -(sta->add_time_sysruntime);
		//asd_printf(ASD_DEFAULT,MSG_DEBUG,"online time is %u, online systime is %u, func is %s, line is %d.\n",(int)online_time,(int)online_time_sysruntime,__func__,__LINE__);//qiuchen add it
	}
	
/*	if(bss->info->rx_data_pkts < rx_data_pkts)
		bss->info->rx_data_pkts = rx_data_pkts;
	if(bss->info->tx_data_pkts < tx_data_pkts)
		bss->info->tx_data_pkts = tx_data_pkts;
	if(bss->info->rx_data_bytes < rx_data_bytes)
		bss->info->rx_data_bytes = rx_data_bytes;
	if(bss->info->tx_data_bytes < tx_data_bytes)
		bss->info->tx_data_bytes = tx_data_bytes;*/
	bss->total_present_online_time = online_time;
	bss->total_present_online_time_sysruntime = online_time_sysruntime;//qiuchen add it 2012.10.31

	return ;
}

static unsigned int GetAccessTimes(unsigned int wtpid){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = wtpid*L_RADIO_NUM; i < wtpid*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL)	
					 num+=interfaces->iface[i]->bss[j]->acc_tms;
						
			}
		}
	}
	return num;
}

static unsigned int GetUserAuthTimes(unsigned int wtpid){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = wtpid*L_RADIO_NUM; i < wtpid*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL)	
					 num+=interfaces->iface[i]->bss[j]->usr_auth_tms;
						
			}
		}
	}
	return num;
}

static unsigned int GetWTPRspAuthTimes(unsigned int wtpid){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = wtpid*L_RADIO_NUM; i < wtpid*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL)	
					 num+=interfaces->iface[i]->bss[j]->ac_rspauth_tms;
						
			}
		}
	}
	return num;
}

static unsigned int GetUserAuthSuccessTimes(unsigned int wtpid){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = wtpid*L_RADIO_NUM; i < wtpid*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL)	
					 num+=interfaces->iface[i]->bss[j]->auth_success;
			}
		}
	}
	return num;
}

static unsigned int GetUserAuthFailTimes(unsigned int wtpid){
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = wtpid*L_RADIO_NUM; i < wtpid*L_RADIO_NUM+L_RADIO_NUM; i++){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] != NULL)	
					 num+=interfaces->iface[i]->bss[j]->auth_fail;
			}
		}
	}
	return num;
}

static int Compute_Sta_Rate(struct sta_info *sta){


	time_t end_time;
	time_t add_time;
	
	if(sta==NULL)
		return -1;
	//qiuchen change it 2012.10.31
	//time(&end_time);
	get_sysruntime(&end_time);
	add_time=sta->add_time_sysruntime;
	//end
	if(start_time==0&&sta->rxbytes_old==0&&sta->txbytes_old==0){
		if((end_time-add_time)!=0){


			sta->rr=sta->rxbytes/(end_time-add_time);	
			sta->tr=sta->txbytes/(end_time-add_time);
		}

	}
	else{
		if((end_time-start_time)!=0){


			sta->rr=(sta->rxbytes-sta->rxbytes_old)/(end_time-start_time);	
			sta->tr=(sta->txbytes-sta->txbytes_old)/(end_time-start_time);
		}else if((end_time-add_time)!=0){


			sta->rr=sta->rxbytes/(end_time-add_time);	
			sta->tr=sta->txbytes/(end_time-add_time);
		}
	}
	sta->tp=sta->rr+sta->tr;
	
	sta->rxbytes_old=sta->rxbytes;
	sta->txbytes_old=sta->txbytes;
	get_sysruntime(&start_time);//qiuchen change it 2012.10.31

	return 0;
}

void CheckSecurityType(char *type, unsigned int SecurityType){

	switch(SecurityType){

		case OPEN :
			strcpy(type, "open");
			break;
			
		case SHARED :
			strcpy(type, "shared");
			break;
		
		case IEEE8021X :
			strcpy(type, "802.1x");
			break;

		case WPA_P :
			strcpy(type, "wpa_p");
			break;
			
		case WPA2_P :
			strcpy(type, "wpa2_p");
			break;

		case WPA2_E :
			strcpy(type, "wpa2_e");
			break;
			
		case WPA_E :
			strcpy(type, "wpa_e");
			break;

		case MD5:
			strcpy(type, "md5");
			break;

		case WAPI_PSK:
			strcpy(type, "wapi_psk");
			break;
	
		case WAPI_AUTH:
			strcpy(type, "wapi_auth");
			break;

	}

}

void CheckEncryptionType(char *type, unsigned int EncryptionType){

	switch(EncryptionType){

		case NONE :
			strcpy(type, "none");
			break;
			
		case WEP :
			strcpy(type, "wep");
			break;
		
		case AES :
			strcpy(type, "aes");
			break;

		case TKIP :
			strcpy(type, "tkip");
			break;
			
		case SMS4 :
			strcpy(type, "sms4");
			break;
	}

}

int CheckHex(char *p)
{
	int i=0,len=0;
	
	if(p==NULL)
		return -1;

	len=strlen(p);
	
	for(i=0;i<len;i++){
		if(!((*p>='0'&&*p<='9')||(*p>='a'&&*p<='f')||(*p>='A'&&*p<='F')))
			return -1;
		p++;
	}
	return 0;
			
}

int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, unsigned int *vlanid2,char *cid, unsigned int *port){ /*fengwenchao add "vlanid2" for axsszfi-1506*/
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);
			/*fengwenchao modify 20130325 for axsszfi-1506 begin*/
			if(endptr[0] == '.')
			{
				tmp = endptr+1;
				*vlanid2 = strtoul(tmp,&endptr,10);
			if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return ASD_UNKNOWN_ID;
			}
			else if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return ASD_UNKNOWN_ID;
			/*fengwenchao modify end*/
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's')){
			*cid = endptr[0];
			tmp = endptr+1;
			*port = strtoul(tmp,&endptr,10);
			/*fengwenchao modify 20130325 for axsszfi-1506 (qinq) begin*/
			if(endptr[0] == '.'){
				tmp = endptr+1;
				*vlanid= strtoul(tmp,&endptr,10);
				if(endptr[0] == '.')
				{
					tmp = endptr+1;
					*vlanid2 = strtoul(tmp,&endptr,10);
				if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return ASD_UNKNOWN_ID;
				}
				else if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return ASD_UNKNOWN_ID;
				return 2;
			}
			else if((endptr[0] == '\0')||(endptr[0] == '\n'))
				return 2;
			/*fengwenchao modify end*/			
			return ASD_UNKNOWN_ID;
		}
	}
	
	return ASD_UNKNOWN_ID;
}

int check_ve_interface(char *ifname, char *name){
	
	int sockfd;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	unsigned int vlanid2 = 0;//fengwenchao add 20130325 for axsszfi-1506 (qinq)
	unsigned int port = 0;
	char cpu = 'f';
	char *cpu_id = &cpu; 
	struct ifreq	ifr;
	if (0 != strncmp(ifname, "ve", 2)){
		sprintf(name,"%s",ifname);
		asd_printf(ASD_DBUS,MSG_DEBUG,"interface name is %s\n",name); 
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 		
		
		if(parse_int_ve(ifname+2,&slotid,&vlanid,&vlanid2,cpu_id,&port)== 1)//fengwenchao add "vlanid2" for axsszfi-1506
		{
			asd_printf(ASD_DBUS,MSG_DEBUG,"slotid = %d,vlanid = %d\n",slotid,vlanid); 
			asd_printf(ASD_DBUS,MSG_DEBUG,"cpu_id = %c,port = %d\n",*cpu_id,port); 

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				asd_printf(ASD_DBUS,MSG_DEBUG,"SIOCGIFINDEX error\n"); 

				//convert to new ve name
				/*fengwenchao modify "vlanid2" for axsszfi-1506 begin*/
				if(slotid < 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve0%df1.%d",slotid,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve0%df1.%d.%d",slotid,vlanid,vlanid2);
				}
				else if(slotid >= 10)
				{	
					if(vlanid2  == 0)
						sprintf(name,"ve%df1.%d",slotid,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve%df1.%d.%d",slotid,vlanid,vlanid2);
				}
				/*fengwenchao modify end*/
				asd_printf(ASD_DBUS,MSG_DEBUG,"ve name is %s\n",name); 

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name)); 		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					asd_printf(ASD_DBUS,MSG_DEBUG,"SIOCGIFINDEX error\n"); 
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}		
				close(sockfd);
				return 0;	//the new ve interface exists
			}
			else{
				sprintf(name,"%s",ifname);
				asd_printf(ASD_DBUS,MSG_DEBUG,"old ve name is %s\n",name); 
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(parse_int_ve(ifname+2,&slotid,&vlanid,&vlanid2,cpu_id,&port)== 2)//fengwenchao add "vlanid2" for axsszfi-1506
		{
			asd_printf(ASD_DBUS,MSG_DEBUG,"slotid = %d,vlanid = %d\n",slotid,vlanid); 
			asd_printf(ASD_DBUS,MSG_DEBUG,"cpu_id = %c,port = %d\n",*cpu_id,port); 

			if(vlanid == 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d",slotid,*cpu_id,port);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d",slotid,*cpu_id,port);
			}
			else if(vlanid > 0){
				/*fengwenchao modify "vlanid2" for axsszfi-1506 begin*/
				if(slotid < 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve0%d%c%d.%d",slotid,*cpu_id,port,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve0%d%c%d.%d.%d",slotid,*cpu_id,port,vlanid,vlanid2);
				}
				else if(slotid >= 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve%d%c%d.%d",slotid,*cpu_id,port,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve%d%c%d.%d.%d",slotid,*cpu_id,port,vlanid,vlanid2);
				}
				/*fengwenchao modify end*/
			}
			asd_printf(ASD_DBUS,MSG_DEBUG,"ve name is %s\n",name); 
		
			memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
			strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));		
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				asd_printf(ASD_DBUS,MSG_DEBUG,"SIOCGIFINDEX error\n"); 
				close(sockfd);
				sockfd = -1;
				return -1;	//the new ve interface doesn't exist
			}		

			close(sockfd);
			sockfd = -1;
			return 0;	//the new ve interface exists
		}
		else{
			asd_printf(ASD_DBUS,MSG_DEBUG,"the ve name is wrong\n"); 
			close(sockfd);
			sockfd = -1;
			return -1;
		}
	}
		
}

//xm add 08/12/03
/*
DBusMessage *asd_dbus_wtp_max_num(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int ret = ASD_DBUS_SUCCESS;
		
	unsigned int wtp_max=0;
	unsigned int wtpid=0;
	struct asd_data *bss[BSS_NUM];
	unsigned int num = 0;
	unsigned int sta_num=0;
	int i=0;
		
	dbus_error_init(&err);
			
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_UINT32,&wtp_max,
								DBUS_TYPE_INVALID))){
		
		printf("Unable to get input args\n");
						
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
		
	//we need know how many sta has accessed,first
	num= ASD_SEARCH_WTP_HAS_STA(wtpid, bss);
	
	for(i=0;i<num;i++)
		sta_num+=bss[i]->num_sta;

			
	if(sta_num>wtp_max){
		ret=ASD_WLAN_VALUE_INVALIDE;
	}
	else if(ASD_WTP_AP[wtpid]==NULL){
		ASD_WTP_AP[wtpid]=(ASD_WTP_ST *)malloc(sizeof(ASD_WTP_ST ));
		ASD_WTP_AP[wtpid]->WTPID=wtpid;
		ASD_WTP_AP[wtpid]->ap_accessed_sta_num=sta_num;
		ASD_WTP_AP[wtpid]->ap_max_allowed_sta_num=wtp_max;
	}
	else if(ASD_WTP_AP[wtpid]!=NULL)
			ASD_WTP_AP[wtpid]->ap_max_allowed_sta_num=wtp_max;
	else
		ret=ASD_WLAN_NOT_EXIST;
		
			
	reply = dbus_message_new_method_return(msg);
						
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	return reply;	

}*/

/*
DBusMessage *asd_dbus_wlan_max_num(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int ret = ASD_DBUS_SUCCESS;
	
	unsigned int wlan_max=0;
	unsigned char wlanid=0;
	struct asd_data *bss[BSS_NUM];
	unsigned int num = 0;
	unsigned int sta_num=0;
	int i=0;
	
	dbus_error_init(&err);
		
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_UINT32,&wlan_max,
								DBUS_TYPE_INVALID))){
	
		printf("Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	
	//we need know how many sta has accessed,first
	num = ASD_SEARCH_WLAN_STA(wlanid, bss);
	for(i=0;i<num;i++)
		sta_num+=bss[i]->num_sta;
		
	if(sta_num>wlan_max){
		ret=ASD_WLAN_VALUE_INVALIDE;
	}
	else if(ASD_WLAN[wlanid]!=NULL)
		ASD_WLAN[wlanid]->wlan_max_allowed_sta_num=wlan_max;
	else
		ret=ASD_WLAN_NOT_EXIST;
	
		
	reply = dbus_message_new_method_return(msg);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	return reply;	

}
*/

/*
DBusMessage *asd_dbus_bss_max_num(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int ret = ASD_DBUS_SUCCESS;

	unsigned int bss_index=0,bss_max=0,radio_id=0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		                        DBUS_TYPE_UINT32,&radio_id,
								DBUS_TYPE_UINT32,&bss_index,
								DBUS_TYPE_UINT32,&bss_max,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	//we need know how many sta has accessed,first
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	if(interfaces->iface[radio_id]->bss[bss_index-1]->num_sta>bss_max){
		ret=ASD_BSS_VALUE_INVALIDE;
	}
	else if(ASD_BSS[radio_id*4+bss_index-1]!=NULL)
		ASD_BSS[radio_id*4+bss_index-1]->bss_max_allowed_sta_num=bss_max;
	else
		ret=ASD_BSS_NOT_EXIST;

	
	reply = dbus_message_new_method_return(msg);
				
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	return reply;	
}

*/
int signal_sta_leave_abnormal(unsigned char mac[6],unsigned int g_rdio,unsigned int g_bssindex,unsigned char wlanid,unsigned char rssi){

	unsigned char traplevel=2;
	unsigned int local_id = local;
	if(gasdtrapflag<traplevel){
		return 0;
	}

	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"sta leave abnormal start\n");

	unsigned int wtpid=g_rdio/L_RADIO_NUM;
	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME);
	os_memset(sn,0,NAS_IDENTIFIER_NAME);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}
	/* zhangshu add for netid & vrrpid, 2010-10-19 , modify 2010-10-26 */
	char *netid = NULL;
	netid = (char *)os_zalloc(WTP_NETID_LEN);
	os_memset(netid,0,WTP_NETID_LEN);
	
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap", ASD_DBUS_SIG_STA_LEAVE_ABNORMAL);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_UINT32,&wtpid,
					DBUS_TYPE_UINT32,&g_bssindex,
					DBUS_TYPE_BYTE,&wlanid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_BYTE,&rssi,	//xiaodawei add 20110301
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	asd_printf(ASD_DBUS,MSG_DEBUG,"sta leave abnormal end\n");
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	return 0;
}
int signal_sta_leave(unsigned char mac[6],unsigned int g_rdio,unsigned int g_bssindex,unsigned char wlanid,unsigned char rssi){
/*	TableMsg msg;
	int len;
	
	msg.Op = STA_LEAVE;
	msg.Type = TRAP_TYPE;
	
	msg.u.STA.BSSIndex=g_bssindex;
	msg.u.STA.WTPID=g_rdio/L_RADIO_NUM;
	msg.u.STA.StaState=wlanid;
	memcpy(msg.u.STA.STAMAC,mac,MAC_LEN);
	
	len = sizeof(msg);
	
	if(sendto(TableSock, &msg, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 1;
	}
*/
	unsigned char traplevel=2;
	unsigned int local_id = local;

	if(gasdtrapflag<traplevel){
		return 0;
	}

	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"sta leave start\n");


	unsigned int wtpid=g_rdio/L_RADIO_NUM;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}
	/* zhangshu add for netid & vrrpid, 2010-10-19 , modify 2010-10-26 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap", ASD_DBUS_SIG_STA_LEAVE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_UINT32,&wtpid,
					DBUS_TYPE_UINT32,&g_bssindex,
					DBUS_TYPE_BYTE,&wlanid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_BYTE,&rssi,	//xiaodawei add 20110301
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	asd_printf(ASD_DBUS,MSG_DEBUG,"sta leave end\n");
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	return 0;
}

int signal_sta_come(unsigned char mac[MAC_LEN],unsigned int g_rdio,unsigned int g_bssindex,unsigned char wlanid,unsigned char rssi){
/*	TableMsg msg;
	int len;
	
	msg.Op = STA_COME;
	msg.Type = TRAP_TYPE;
	
	msg.u.STA.BSSIndex=g_bssindex;
	msg.u.STA.WTPID=g_rdio/L_RADIO_NUM;
	msg.u.STA.StaState=wlanid;
	memcpy(msg.u.STA.STAMAC,mac,MAC_LEN);
	
	len = sizeof(msg);
	
	if(sendto(TableSock, &msg, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 1;
	}
*/
	unsigned char traplevel=2;
	unsigned int local_id = local;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}

	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"sta come start\n");
	unsigned int wtpid=g_rdio/L_RADIO_NUM;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid , 2010-10-26 */
         char *netid = NULL;
         netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap", ASD_DBUS_SIG_STA_COME);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_UINT32,&wtpid,
					DBUS_TYPE_UINT32,&g_bssindex,
					DBUS_TYPE_BYTE,&wlanid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_BYTE,&rssi,	//xiaodawei add rssi for telecom, 20110301
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);
	dbus_connection_send (asd_dbus_connection4,query,NULL);
	dbus_message_unref(query);

	asd_printf(ASD_DBUS,MSG_DEBUG,"sta come end\n");

	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-26
	os_free(netid);   
	netid = NULL;
	return 0;
}


int signal_wtp_deny_sta(unsigned int wtpid){
/*
	TableMsg msg;
	int len;
	
	msg.Op = WTP_DENEY_STA;
	msg.Type = TRAP_TYPE;
	msg.u.WTP.WtpID =  wtpid;
	len = sizeof(msg);
	
	if(sendto(TableSock, &msg, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 1;
	}
	printf("send wid wtpid %d",msg.u.WTP.WtpID);
	de_wtp_deny_sta[wtpid]=1;
*/
	unsigned char traplevel=4;
	
	if((gasdtrapflag<traplevel) && (wtp_deny_sta_able != 1)){
		return 0;
	}

	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"wtp deny sta start\n");

	
	char *mac = NULL;
	mac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(mac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(mac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid, 2010-10-26 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	unsigned int local_id = local;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_WTP_DENY_STA);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_UINT32,&wtpid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	asd_printf(ASD_DBUS,MSG_DEBUG,"wtp deny sta end\n");
	os_free(sn);
	sn=NULL;
	os_free(mac);
	mac=NULL;
	//zhangshu add for free netid ,2010-10-26
	os_free(netid);   
	netid = NULL;
	return 0;

}

int signal_de_wtp_deny_sta(unsigned int wtpid){
/*
	TableMsg msg;
	int len;

	if(de_wtp_deny_sta[wtpid]==0)
		return 0;
	de_wtp_deny_sta[wtpid]=0;
	
	msg.Op = WTP_DE_DENEY_STA;
	msg.Type = TRAP_TYPE;
	msg.u.WTP.WtpID =  wtpid;
	len = sizeof(msg);
	
	if(sendto(TableSock, &msg, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 1;
	}
	printf("send wid wtpid %d",msg.u.WTP.WtpID);

	return 0;
*/
	unsigned char traplevel=4;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}
	
	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG," de wtp deny sta start\n");
	
	
	char *mac = NULL;
	mac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(mac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(mac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid & vrrpid, 2010-10-19 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	unsigned int local_id = local;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_DE_WTP_DENY_STA);
	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_UINT32,&wtpid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	asd_printf(ASD_DBUS,MSG_DEBUG,"de wtp deny sta end\n");
	os_free(sn);
	sn=NULL;
	os_free(mac);
	mac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	return 0;

}


/*int signal_deny_sta_num(unsigned int num){
	DBusMessage *query; 
	DBusError err;
printf("deny sta start\n");

	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_DENY_STA_NUM );

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_UINT32,&num,
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection,query,NULL);

	dbus_message_unref(query);
	printf("deny sta end\n");
	return 0;

}
*/

// ht add 090810
int signal_wapi_trap(unsigned char mac[MAC_LEN],unsigned int bss_index,unsigned char reason)
{
	unsigned char traplevel=1;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}
	switch(reason){
		case 1:
			if(wapi_invalid_cert_able != 1)
				return 0;
			break;
		case 2:
			if(wapi_challenge_replay_able != 1)
				return 0;
			break;
		case 3:
			if(wapi_mic_juggle_able != 1)
				return 0;
			break;
		case 4:
			if(wapi_low_safe_able != 1)
				return 0;
			break;
		case 5:
			if(wapi_addr_redirection_able != 1)
				return 0;
			break;
		default:
			break;
	}
	
	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_wapi_trap\n");

	unsigned int wtpid=(bss_index/L_BSS_NUM)/L_RADIO_NUM;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid & vrrpid, 2010-10-19 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	unsigned int local_id = local;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_WAPI_TRAP);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_BYTE,&reason,
					DBUS_TYPE_INT32,&wtpid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_verify end.\n");
	return 0;
}




// ht add 090216
int signal_sta_verify(const unsigned char mac[MAC_LEN],unsigned int bss_index)
{
	/*struct asd_stainfo * stainfo;

	TableMsg STA;
	int len;
	STA.Op = VERIFY_INFO;
	STA.Type = TRAP_TYPE;
	STA.u.STA.BSSIndex = bss_index;
	STA.u.STA.WTPID = (bss_index/L_BSS_NUM)/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC, mac, MAC_LEN);
	len = sizeof(STA);
	if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	*/

	unsigned char traplevel=4;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}
	
	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_verify\n");

	unsigned int wtpid=(bss_index/L_BSS_NUM)/L_RADIO_NUM;
	unsigned int local_id = local;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid & vrrpid, 2010-10-19 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_STA_VERIFY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_INT32,&wtpid,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_verify end.\n");
	return 0;
}

int signal_sta_verify_failed(const unsigned char mac[MAC_LEN],unsigned int bss_index)
{
	/*struct asd_stainfo * stainfo;
	
	TableMsg STA;
	int len;
	STA.Op = VERIFY_FAIL_INFO;
	STA.Type = TRAP_TYPE;
	STA.u.STA.BSSIndex = bss_index;
	STA.u.STA.WTPID = (bss_index/L_BSS_NUM)/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC, mac, MAC_LEN);
	len = sizeof(STA);
	if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	*/
	unsigned char traplevel=4;
	unsigned int local_id = local;
	
	if((gasdtrapflag<traplevel) && (sta_verify_failed_able != 1)){
		return 0;
	}
	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_verify_failed\n");
	
	unsigned int wtpid=(bss_index/L_BSS_NUM)/L_RADIO_NUM;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid & vrrpid, 2010-10-19 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_STA_VERIFY_FAILED);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&mac[0],
						DBUS_TYPE_BYTE,&mac[1],
						DBUS_TYPE_BYTE,&mac[2],
						DBUS_TYPE_BYTE,&mac[3],
						DBUS_TYPE_BYTE,&mac[4],
						DBUS_TYPE_BYTE,&mac[5],
						DBUS_TYPE_INT32,&wtpid,
						DBUS_TYPE_STRING,&sn,
						DBUS_TYPE_BYTE,&wtpmac[0],
						DBUS_TYPE_BYTE,&wtpmac[1],
						DBUS_TYPE_BYTE,&wtpmac[2],
						DBUS_TYPE_BYTE,&wtpmac[3],
						DBUS_TYPE_BYTE,&wtpmac[4],
						DBUS_TYPE_BYTE,&wtpmac[5],
						DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					    DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
						DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
						DBUS_TYPE_INVALID);
	
	dbus_connection_send (asd_dbus_connection4,query,NULL);
	
	dbus_message_unref(query);
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_verify_failed end.\n");
	return 0;
}


//ht add 090224
//type:0--auth 1-acct
void signal_radius_connect_failed(const char *ip,unsigned char type)
{
	unsigned char traplevel=2;
	
	if(gasdtrapflag<traplevel){
		return ;
	}
	
	unsigned int local_id = local;
	char ipaddr[16]={0};
	DBusMessage *query; 
	DBusError err;
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",__func__);
	strcpy(ipaddr,ip);

	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_RADIUS_CONNECT_FAILED);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_STRING,&ip,
					DBUS_TYPE_BYTE,&type,
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	return ;

}

//ht add 090403
//type:0--auth 1-acct
void signal_radius_connect_failed_clean(const char *ip,unsigned char type)
{	
	unsigned char traplevel=2;
	
	if(gasdtrapflag<traplevel){
		return ;
	}
	
	unsigned int local_id = local;
	char ipaddr[16]={0};
	DBusMessage *query; 
	DBusError err;
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",__func__);
	strcpy(ipaddr,ip);

	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_STRING,&ip,
					DBUS_TYPE_BYTE,&type,
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	return ;

}

//xm add 2009.04.03
//association fail warning.
int signal_assoc_failed(unsigned char mac[MAC_LEN],unsigned short reason_code, unsigned int bssindex){

/*


	TableMsg STA;
	int len;
	STA.Op = ASSOC_FAIL_INFO;
	STA.Type = TRAP_TYPE;
	STA.u.STA.BSSIndex = bssindex;
	STA.u.STA.WTPID = (bssindex/L_BSS_NUM)/L_RADIO_NUM;
	STA.u.STA.nRate = reason_code;
	memcpy(STA.u.STA.STAMAC, mac, MAC_LEN);
	len = sizeof(STA);
	if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	
*/
	unsigned char traplevel=4;
	
	if((gasdtrapflag<traplevel) && (sta_assoc_failed_able != 1)){
		return 0;
	}
	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_assoc_failed\n");

	unsigned short ret=reason_code;
	unsigned int wtpid=(bssindex/L_BSS_NUM)/L_RADIO_NUM;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}
	
	/* zhangshu add for netid & vrrpid, 2010-10-19 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	unsigned int local_id = local;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_STA_ASSOC_FAILED);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_INT32,&wtpid,
					DBUS_TYPE_UINT16,&ret,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_assoc_failed\n");
	return 0;
}

int signal_jianquan_failed(unsigned char mac[MAC_LEN],unsigned short reason_code,unsigned int bssindex){
	/*TableMsg STA;
	int len;
	STA.Op = JIANQUAN_FAIL_INFO;
	STA.Type = TRAP_TYPE;
	STA.u.STA.BSSIndex = bssindex;
	STA.u.STA.WTPID = (bssindex/L_BSS_NUM)/L_RADIO_NUM;
	STA.u.STA.nRate = reason_code;
	memcpy(STA.u.STA.STAMAC, mac, MAC_LEN);
	len = sizeof(STA);
	if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	
	*/

	unsigned char traplevel=4;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}
	
	DBusMessage *query; 
	DBusError err;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_jianquan_failed\n");

	unsigned short ret=reason_code;
	unsigned int wtpid=(bssindex/L_BSS_NUM)/L_RADIO_NUM;

	char *wtpmac = NULL;
	wtpmac = (char *)os_zalloc(MAC_LEN+1);
	os_memset(wtpmac,0,MAC_LEN+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(wtpmac,ASD_WTP_AP[wtpid]->WTPMAC,MAC_LEN);
	}

	char *sn = NULL;
	sn = (char *)os_zalloc(NAS_IDENTIFIER_NAME+1);
	os_memset(sn,0,NAS_IDENTIFIER_NAME+1);
	if(ASD_WTP_AP[wtpid] != NULL){
		os_memcpy(sn,ASD_WTP_AP[wtpid]->WTPSN,strlen(ASD_WTP_AP[wtpid]->WTPSN));
	}

	/* zhangshu add for netid & vrrpid, 2010-10-19 */
    char *netid = NULL;
    netid = (char *)os_zalloc(WTP_NETID_LEN+1);
	os_memset(netid,0,WTP_NETID_LEN+1);
    
	if(ASD_WTP_AP[wtpid] != NULL)
	{
		os_memcpy(netid,ASD_WTP_AP[wtpid]->NETID,WTP_NETID_LEN);
	}
	unsigned int vrrp_id = vrrid;
	unsigned int local_id = local;
	/* zhangshu add end */
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_STA_JIANQUAN_FAILED );

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&mac[0],
					DBUS_TYPE_BYTE,&mac[1],
					DBUS_TYPE_BYTE,&mac[2],
					DBUS_TYPE_BYTE,&mac[3],
					DBUS_TYPE_BYTE,&mac[4],
					DBUS_TYPE_BYTE,&mac[5],
					DBUS_TYPE_INT32,&wtpid,
					DBUS_TYPE_UINT16,&ret,
					DBUS_TYPE_STRING,&sn,
					DBUS_TYPE_BYTE,&wtpmac[0],
					DBUS_TYPE_BYTE,&wtpmac[1],
					DBUS_TYPE_BYTE,&wtpmac[2],
					DBUS_TYPE_BYTE,&wtpmac[3],
					DBUS_TYPE_BYTE,&wtpmac[4],
					DBUS_TYPE_BYTE,&wtpmac[5],
					DBUS_TYPE_STRING,&netid, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&vrrp_id, //zhangshu add 2010-10-19
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);
	os_free(sn);
	sn=NULL;
	os_free(wtpmac);
	wtpmac=NULL;
	//zhangshu add for free netid ,2010-10-19
	os_free(netid);   
	netid = NULL;
	asd_printf(ASD_DBUS,MSG_DEBUG,"signal_sta_jianquan_failed end.\n");
	return 0;
}


static int check_same_key(unsigned char *s1,unsigned char *s2){
	
	unsigned char i=0, j=0;
	
	for(i=0;i<WLAN_NUM-1;i++){
		if((ASD_SECURITY[i]==NULL)||(ASD_SECURITY[i]->keyLen==0)||(ASD_SECURITY[i]->SecurityKey==NULL))
			continue;
		for(j=i+1;j<WLAN_NUM;j++){
			if((ASD_SECURITY[j]!=NULL)&&(ASD_SECURITY[j]->keyLen!=0)&&(ASD_SECURITY[j]->SecurityKey!=NULL)){
				if((ASD_SECURITY[i]->keyLen==ASD_SECURITY[j]->keyLen)
					&&(0==strcmp(ASD_SECURITY[i]->SecurityKey,ASD_SECURITY[j]->SecurityKey))){

					*s1=i;
					*s2=j;
					return 0;
				}
			}
		}
	}////

	return -1;
}

static int wlan_check_same_security(unsigned char *w1,unsigned char *w2){
	unsigned char i=0, j=0;
	
	for(i=0;i<WLAN_NUM-1;i++){
		if(ASD_WLAN[i]==NULL)
			continue;
		for(j=i+1;j<WLAN_NUM;j++){
			if(ASD_WLAN[j]!=NULL){
				if((ASD_WLAN[i]->SecurityID==ASD_WLAN[j]->SecurityID)
					&&(ASD_SECURITY[ASD_WLAN[i]->SecurityID]!=NULL)
					&&(ASD_SECURITY[ASD_WLAN[i]->SecurityID]->keyLen!=0)){

					*w1=i;
					*w2=j;
					return 0;
				}
			}
		}
	}////

	return -1;
}

//xm add.    when two wlan apply same key , send this signal. 
int signal_key_conflict(void){
	unsigned char traplevel=4;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}
	
	DBusMessage *query; 
	DBusError err;
	unsigned char var1,var2,type;
	unsigned int local_id = local;

	if(0==check_same_key(&var1,&var2)){
		type=1;
		//printf("TH:\tsecurity %d and security %d use same key.\n",var1,var2);
	}else if(0==wlan_check_same_security(&var1,&var2)){
		type=2;
		//printf("TH:\twlan %d and wlan %d use same key.\n",var1,var2);
	}else{
		//printf("TH:\tnot conflict.\n");
		return 0;
	}

	de_key_conflict=1;
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap",ASD_DBUS_SIG_KEY_CONFLICT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_BYTE,&type,
					DBUS_TYPE_BYTE,&var1,
					DBUS_TYPE_BYTE,&var2,
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);

	return 0;
}


//xm add.    when not conflict , send this signal. 
int signal_de_key_conflict(void){
	unsigned char traplevel=4;
	
	if(gasdtrapflag<traplevel){
		return 0;
	}
	DBusMessage *query; 
	DBusError err;
	unsigned char var1,var2,type;
	unsigned int local_id = local;

	if(0==check_same_key(&var1,&var2)){
		type=1;
		//printf("TH:\tsecurity %d and security %d use same key.\n",var1,var2);
		return 0;
	}else if(0==wlan_check_same_security(&var1,&var2)){
		type=2;
		//printf("TH:\twlan %d and wlan %d use same key.\n",var1,var2);
		return 0;
	}else{
		//printf("TH:\tnot conflict.\n");
		type=0;
	}

	if(de_key_conflict==0)
		return 0;

	de_key_conflict=0;
	
	query = dbus_message_new_signal(ASD_DBUS_OBJPATH,\
			"aw.trap", ASD_DBUS_SIG_DE_KEY_CONFLICT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					
					DBUS_TYPE_BYTE,&type,
					DBUS_TYPE_UINT32,&local_id, //mahz add 2011.9.23
					DBUS_TYPE_INVALID);

	dbus_connection_send (asd_dbus_connection4,query,NULL);

	dbus_message_unref(query);

	return 0;
}
static unsigned int GetBssindexByRadioIDWlanID(unsigned int radioid,unsigned char wlanid){
//	xm0616
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i=0;
	unsigned int bssindex=0;

	if(radioid<L_RADIO_NUM||radioid>G_RADIO_NUM)
		return 0;
	
	if((interfaces->iface[radioid] == NULL)||(interfaces->iface[radioid]->bss == NULL))
		return 0;
	
	for(i=0;i<L_BSS_NUM;i++){
		if((interfaces->iface[radioid]->bss[i]!=NULL)&&
			(interfaces->iface[radioid]->bss[i]->WlanID==wlanid)){
			bssindex=interfaces->iface[radioid]->bss[i]->BSSIndex;
			break;
		}
	}

	return bssindex;
}


	
DBusMessage *asd_dbus_show_wapi_mib_info_byrdid(DBusConnection *conn, DBusMessage *msg, void *user_data){
//	xm0630
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	unsigned int wtpid = 0, i=0, j=0,k=0;
	int ret = ASD_DBUS_SUCCESS;
	BSS_WAPI_MIB_INFO_ST *buf=NULL;
	unsigned int bssnum=L_BSS_NUM;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];

	struct sta_info *sta;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
	if(wtpid >=WTP_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!AsdCheckWTPID(wtpid))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
	}else{
		bssnum = ASD_SEARCH_WTP_STA(wtpid, bss);
		
		buf=(BSS_WAPI_MIB_INFO_ST *)os_zalloc(sizeof(BSS_WAPI_MIB_INFO_ST)*bssnum);
		if(buf==NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}
		memset(buf,0,sizeof(BSS_WAPI_MIB_INFO_ST)*bssnum);

		if(0!= GetWapiMibInfoByBss(buf, bss, bssnum)){
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s :GetWapiMibInfoByBss() fail.\n",__func__);
			free(buf);
			buf=NULL;
			pthread_mutex_unlock(&asd_g_sta_mutex);		//mahz add 2011.4.20
			return NULL;
		}	
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &bssnum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING

												DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING

												DBUS_TYPE_UINT32_AS_STRING

												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING

													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING

													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   
														DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		
		for(i=0; i<bssnum; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			unsigned int sta_num=0;

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].bssid[0]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].bssid[1]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].bssid[2]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].bssid[3]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].bssid[4]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].bssid[5]));	//	xm0630
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].ControlledAuthControl));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].ControlledPortControl));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].wapiEnabled));
			

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].CertificateUpdateCount));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].MulticastUpdateCount));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].UnicastUpdateCount));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].AuthenticationSuite));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(buf[i].AuthSuiteSelected));
			
			if(0==buf[i].wapiEnabled){
				sta_num=0;
			}else if(1==buf[i].wapiEnabled){
				if(bss[i]!=NULL)
					sta_num=bss[i]->num_sta;
			}
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&sta_num);
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING

														DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING

													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING

													
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   
														DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			if(sta_num>0){
				sta=bss[i]->sta_list;
			}
			
			for(j=0;(j<sta_num)&&(sta!=NULL);j++){

				STA_WAPI_MIB_INFO_ST	stabuf;
				DBusMessageIter iter_sub_struct;
				memset(&stabuf,0,sizeof(stabuf));
				if(0!=GetWapiMibInfoBySta(&stabuf,sta)){
					asd_printf(ASD_DBUS,MSG_DEBUG,"%s :GetWapiMibInfoBySta() fail.\n",__func__);
					pthread_mutex_unlock(&asd_g_sta_mutex);		//mahz add 2011.4.20
					if(buf)
					{
						free(buf);
						buf = NULL;
					}
					return NULL;
				}
				
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(sta->addr[5]));

				for(k=0;k<16;k++){
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
					 	  	&(stabuf.BKIDUsed[k]));
				}			//	xm0626
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(stabuf.UnicastRekeyTime));
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(stabuf.UnicastRekeyPackets));
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(stabuf.MulticastRekeyTime));
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(stabuf.MulticastRekeyPackets));
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(stabuf.ControlledPortStatus));
				
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				sta = sta->next;
			}

			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
	

		free(buf);
		buf=NULL;
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	return reply;

}



//mahz add for mib request , 2011.1.19, dot11BSSIDWAPIProtocolConfigTable
DBusMessage *asd_dbus_show_wapi_mib_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_bss_array;
	
	unsigned int  i=0, j=0;
	int ret = ASD_DBUS_SUCCESS;

	unsigned int bssnum = 0;
	unsigned int wtp_num=0;
			
	BSS_WAPI_MIB_INFO_ST *buf=NULL;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];

	DBusError err;
	dbus_error_init(&err);

	ASD_WTP_ST **WTP=malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	//if(!WTP)
	//	ret = -1;
	
	pthread_mutex_lock(&asd_g_wtp_mutex); 
	for(i=0;i<WTP_NUM;i++){
		if(ASD_WTP_AP[i]){
			WTP[wtp_num++]=ASD_WTP_AP[i];
		}
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"wtp_num: %d\n",wtp_num);
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num);

	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//WTPID
									DBUS_TYPE_BYTE_AS_STRING		//wtp_mac
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//bssnum

										DBUS_TYPE_ARRAY_AS_STRING	//二层循环
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										
									       DBUS_TYPE_BYTE_AS_STRING
										   DBUS_TYPE_BYTE_AS_STRING
										   DBUS_TYPE_BYTE_AS_STRING
										   DBUS_TYPE_BYTE_AS_STRING
										   DBUS_TYPE_BYTE_AS_STRING
										   DBUS_TYPE_BYTE_AS_STRING

  									       DBUS_TYPE_BYTE_AS_STRING
  										   DBUS_TYPE_BYTE_AS_STRING
  										   DBUS_TYPE_BYTE_AS_STRING
  										
										   DBUS_TYPE_UINT64_AS_STRING
										   DBUS_TYPE_UINT64_AS_STRING
										   DBUS_TYPE_UINT64_AS_STRING
												
										   DBUS_TYPE_BYTE_AS_STRING
										   DBUS_TYPE_BYTE_AS_STRING

									    DBUS_STRUCT_END_CHAR_AS_STRING	
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_wtp_array);
		
	pthread_mutex_lock(&asd_g_sta_mutex);	

	for(i=0;i<wtp_num;i++){
		
		DBusMessageIter iter_wtp;		
		dbus_message_iter_open_container(&iter_wtp_array,DBUS_TYPE_STRUCT,NULL,&iter_wtp);
	
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&(WTP[i]->WTPID));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[0]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[1]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[2]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[3]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[4]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[5]));		
		
		bssnum = ASD_SEARCH_WTP_STA(WTP[i]->WTPID, bss);
		asd_printf(ASD_DBUS,MSG_DEBUG,"i= %d, bssnum= %d\n",i,bssnum);
		
		buf=(BSS_WAPI_MIB_INFO_ST *)os_zalloc(sizeof(BSS_WAPI_MIB_INFO_ST)*bssnum);
		if(buf==NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}
		memset(buf,0,sizeof(BSS_WAPI_MIB_INFO_ST)*bssnum);

		if(0!= GetWapiMibInfoByBss(buf, bss, bssnum)){
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s :GetWapiMibInfoByBss() fail.\n",__func__);
			free(buf);
			buf=NULL;
			if(WTP != NULL){
				free(WTP);
				WTP = NULL;
			}
			pthread_mutex_unlock(&asd_g_sta_mutex); 
			pthread_mutex_unlock(&asd_g_wtp_mutex); 
			return NULL;
		}
		
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32, &bssnum);
					
		dbus_message_iter_open_container (&iter_wtp,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING

												DBUS_TYPE_BYTE_AS_STRING	//bssid
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
													
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_bss_array);
		
		for(j=0; j<bssnum; j++){
			
			DBusMessageIter iter_bss;
			dbus_message_iter_open_container (&iter_bss_array, DBUS_TYPE_STRUCT, NULL, &iter_bss);

			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].bssid[0]));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].bssid[1]));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].bssid[2]));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].bssid[3]));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].bssid[4]));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].bssid[5]));	
			
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].ControlledAuthControl));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].ControlledPortControl));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE, &(buf[j].wapiEnabled));
			
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_UINT64, &(buf[j].CertificateUpdateCount));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_UINT64,	&(buf[j].MulticastUpdateCount));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_UINT64,	&(buf[j].UnicastUpdateCount));
			
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE,	&(buf[j].AuthenticationSuite));
			dbus_message_iter_append_basic(&iter_bss, DBUS_TYPE_BYTE,   &(buf[j].AuthSuiteSelected));

			dbus_message_iter_close_container (&iter_bss_array, &iter_bss);			
		}	
		dbus_message_iter_close_container (&iter_wtp, &iter_bss_array);
		dbus_message_iter_close_container (&iter_wtp_array, &iter_wtp);
		if(buf != NULL){
			free(buf);
			buf=NULL;
		}
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	pthread_mutex_unlock(&asd_g_wtp_mutex); 

	dbus_message_iter_close_container (&iter, &iter_wtp_array);
	if(WTP != NULL){
		free(WTP);
		WTP = NULL;
	}
	return reply;
}
//


//mahz add for mib request , 2011.1.24, dot11StaWAPIProtocolConfigTable
DBusMessage *asd_dbus_show_sta_wapi_mib_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_sta_array;
	
	unsigned int  i=0, j=0, m=0;
	int ret = ASD_DBUS_SUCCESS;

	unsigned int bssnum = 0;
	unsigned int wtp_num = 0;
	unsigned int total_sta_num = 0;
			
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	BSS_WAPI_MIB_INFO_ST *buf=NULL;
	struct sta_info *sta;

	DBusError err;
	dbus_error_init(&err);

	ASD_WTP_ST **WTP=malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	//if(!WTP)
	//	ret = -1;
	
	pthread_mutex_lock(&asd_g_wtp_mutex);
	for(i=0;i<WTP_NUM;i++){
		if(ASD_WTP_AP[i]){
			WTP[wtp_num++]=ASD_WTP_AP[i];
		}
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"wtp_num: %d\n",wtp_num);
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num);

	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//WTPID
									DBUS_TYPE_BYTE_AS_STRING		//wtp_mac
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//sta_num
									       
									  DBUS_TYPE_ARRAY_AS_STRING	//二层循环
									  DBUS_STRUCT_BEGIN_CHAR_AS_STRING

									    DBUS_TYPE_BYTE_AS_STRING	//sta_mac
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING

									    DBUS_TYPE_BYTE_AS_STRING	//bkid
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING

									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
									    DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										   
								   	    DBUS_TYPE_BYTE_AS_STRING //ControlledPortStatus
								   	   
        							 DBUS_STRUCT_END_CHAR_AS_STRING							 
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_wtp_array);
		
	pthread_mutex_lock(&asd_g_sta_mutex);	

	for(i=0;i<wtp_num;i++){
		
		DBusMessageIter iter_wtp;		
		dbus_message_iter_open_container(&iter_wtp_array,DBUS_TYPE_STRUCT,NULL,&iter_wtp);
	
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&(WTP[i]->WTPID));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[0]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[1]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[2]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[3]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[4]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[5]));		
		
		bssnum = ASD_SEARCH_WTP_STA(WTP[i]->WTPID, bss);
		asd_printf(ASD_DBUS,MSG_DEBUG,"i= %d, bssnum= %d\n",i,bssnum);
		
		buf=(BSS_WAPI_MIB_INFO_ST *)os_zalloc(sizeof(BSS_WAPI_MIB_INFO_ST)*bssnum);
		if(buf==NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}
		memset(buf,0,sizeof(BSS_WAPI_MIB_INFO_ST)*bssnum);

		if(0!= GetWapiMibInfoByBss(buf, bss, bssnum)){
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s :GetWapiMibInfoByBss() fail.\n",__func__);
			free(buf);
			buf=NULL;
			if(WTP != NULL){
				free(WTP);
				WTP = NULL;
			}
			pthread_mutex_unlock(&asd_g_sta_mutex); 
			pthread_mutex_unlock(&asd_g_wtp_mutex);
			return NULL;
		}
	
		total_sta_num=0;
		for(j=0;j<bssnum;j++){
			unsigned int sta_num = 0;
			if(0==buf[j].wapiEnabled){
				sta_num=0;
			}else if(1==buf[j].wapiEnabled){
				if(bss[j]!=NULL){
					sta_num = bss[j]->num_sta;
					total_sta_num += sta_num;
				//	asd_printf(ASD_DBUS,MSG_DEBUG,"j = %d\n,sta_num = %d\n",j,sta_num);
				}
			}
		}
		asd_printf(ASD_DBUS,MSG_DEBUG,"total_sta_num: %d\n",total_sta_num);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&(total_sta_num));
		
		dbus_message_iter_open_container (&iter_wtp,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING

											   DBUS_TYPE_BYTE_AS_STRING	//sta_mac
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING

											   DBUS_TYPE_BYTE_AS_STRING	//bkid
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
	  									       DBUS_TYPE_BYTE_AS_STRING
	  										   DBUS_TYPE_BYTE_AS_STRING
	  										   DBUS_TYPE_BYTE_AS_STRING

											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
	  									       DBUS_TYPE_BYTE_AS_STRING
	  										   DBUS_TYPE_BYTE_AS_STRING
	  										   DBUS_TYPE_BYTE_AS_STRING
	  										   
										   	   DBUS_TYPE_BYTE_AS_STRING //ControlledPortStatus
												
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_sta_array);
		
		for(j=0; j<bssnum; j++){
			if((1==buf[j].wapiEnabled)&&(bss[j]!=NULL)){
				for(sta=bss[j]->sta_list;sta != NULL;sta=sta->next){
					STA_WAPI_MIB_INFO_ST	stabuf;
					memset(&stabuf,0,sizeof(stabuf));
					DBusMessageIter iter_sta_struct;
			
					dbus_message_iter_open_container (&iter_sta_array,DBUS_TYPE_STRUCT,NULL,&iter_sta_struct);
					if(0!=GetWapiMibInfoBySta(&stabuf,sta)){
						asd_printf(ASD_DBUS,MSG_DEBUG,"%s :GetWapiMibInfoBySta() fail.\n",__func__);
						if(buf)
						{
							free(buf);
							buf = NULL;
						}
						if(WTP)
						{
							free(WTP);
							WTP = NULL;
						}
						return NULL;
					}					

					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(sta->addr[0]));
					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(sta->addr[1]));
					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(sta->addr[2]));
					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(sta->addr[3]));
					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(sta->addr[4]));
					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(sta->addr[5]));

					for(m=0;m<16;m++){
						dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(stabuf.BKIDUsed[m]));
					}					
					dbus_message_iter_append_basic(&iter_sta_struct,DBUS_TYPE_BYTE,&(stabuf.ControlledPortStatus));
				
					dbus_message_iter_close_container (&iter_sta_array, &iter_sta_struct);
				}
			}
		}
		dbus_message_iter_close_container (&iter_wtp, &iter_sta_array);	
		dbus_message_iter_close_container (&iter_wtp_array, &iter_wtp);
		if(buf != NULL){
			free(buf);
			buf=NULL;
		}
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	pthread_mutex_unlock(&asd_g_wtp_mutex);
	dbus_message_iter_close_container (&iter, &iter_wtp_array);

	if(WTP != NULL){
		free(WTP);
		WTP = NULL;
	}
	return reply;
}
//

DBusMessage *asd_dbus_cancel_sta_traffic_limit(DBusConnection *conn, DBusMessage *msg, void *user_data){
/*xm0723*/
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned int radioid;
	unsigned char wlanid;
	unsigned char cancel_flag = 0;
	unsigned char mac[MAC_LEN];
	unsigned int bssindex=0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	struct asd_data *wasd=NULL; 
	struct sta_info * sta=NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,								
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_ERROR,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_ERROR,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	bssindex=GetBssindexByRadioIDWlanID(radioid,wlanid);

	if(bssindex>=L_BSS_NUM*L_RADIO_NUM){

		pthread_mutex_lock(&asd_g_sta_mutex); 
		wasd=AsdCheckBSSIndex(bssindex);

		if(wasd!=NULL){
			
			sta=ap_get_sta(wasd,mac);
	
			if(sta!=NULL){
				sta->vip_flag &= ~0x01;	/*clear bit0*/
				sta->sta_traffic_limit=wasd->sta_average_traffic_limit;
				if((sta->vip_flag & 0x02) != 0)
					cancel_flag = 1;
				else if ((sta->vip_flag & 0x02) == 0){
					cancel_flag = 2;
					sta->sta_send_traffic_limit=wasd->sta_average_send_traffic_limit;
				}
			}else{
				ret=ASD_STA_NOT_EXIST;
			}
		}else{
			ret=ASD_WLAN_NOT_EXIST;
		}
		pthread_mutex_unlock(&asd_g_sta_mutex); 
	}else{
		ret=ASD_WLAN_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_BYTE,
									&cancel_flag); 
	return reply;

}


DBusMessage *asd_dbus_set_sta_traffic_limit(DBusConnection *conn, DBusMessage *msg, void *user_data){
/*xm0723*/
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned int radioid;
	unsigned char wlanid;
	unsigned char mac[MAC_LEN];
	unsigned int value,bssindex=0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	struct asd_data *wasd=NULL; 
	struct sta_info * sta=NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_UINT32,&radioid,								
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_ERROR,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_ERROR,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	bssindex=GetBssindexByRadioIDWlanID(radioid,wlanid);

	if(bssindex>=L_BSS_NUM*L_RADIO_NUM){

		pthread_mutex_lock(&asd_g_sta_mutex); 
		wasd=AsdCheckBSSIndex(bssindex);

		if(wasd!=NULL){
			
			sta=ap_get_sta(wasd,mac);
	
			if(sta!=NULL){
				if(wasd->traffic_limit!=0&&value>wasd->traffic_limit){
					ret=ASD_DBUS_ERROR;
				}else{
					sta->vip_flag |= 0x01;	/*set bit0*/
					sta->sta_traffic_limit=value;
				}
			}else{
				ret=ASD_STA_NOT_EXIST;
			}
		}else{
			ret=ASD_WLAN_NOT_EXIST;
		}
		pthread_mutex_unlock(&asd_g_sta_mutex); 
	}else{
		ret=ASD_WLAN_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	return reply;

}


DBusMessage *asd_dbus_cancel_sta_send_traffic_limit(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned int radioid;
	unsigned char wlanid;
	unsigned char cancel_flag = 0;
	unsigned char mac[MAC_LEN];
	unsigned int bssindex=0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	struct asd_data *wasd=NULL; 
	struct sta_info * sta=NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,								
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_ERROR,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_ERROR,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	bssindex=GetBssindexByRadioIDWlanID(radioid,wlanid);

	if(bssindex>=L_BSS_NUM*L_RADIO_NUM){

		pthread_mutex_lock(&asd_g_sta_mutex); 
		wasd=AsdCheckBSSIndex(bssindex);

		if(wasd!=NULL){
			
			sta=ap_get_sta(wasd,mac);
	
			if(sta!=NULL){
				sta->vip_flag &= ~0x02;	/*clear bit1*/
				sta->sta_send_traffic_limit=wasd->sta_average_send_traffic_limit;
				if((sta->vip_flag & 0x01) != 0)
					cancel_flag = 1;
				else if ((sta->vip_flag & 0x01) == 0){
					cancel_flag = 2;
					sta->sta_traffic_limit=wasd->sta_average_traffic_limit;
				}
			}else{
				ret=ASD_STA_NOT_EXIST;
			}
		}else{
			ret=ASD_WLAN_NOT_EXIST;
		}
		pthread_mutex_unlock(&asd_g_sta_mutex); 
	}else{
		ret=ASD_WLAN_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_BYTE,
									&cancel_flag); 
	return reply;

}


DBusMessage *asd_dbus_set_sta_send_traffic_limit(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned int radioid;
	unsigned char wlanid;
	unsigned char mac[MAC_LEN];
	unsigned int value,bssindex=0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	struct asd_data *wasd=NULL; 
	struct sta_info * sta=NULL;
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",__func__);
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_UINT32,&radioid,								
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_ERROR,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_ERROR,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	bssindex=GetBssindexByRadioIDWlanID(radioid,wlanid);

	if(bssindex>=L_BSS_NUM*L_RADIO_NUM){

		pthread_mutex_lock(&asd_g_sta_mutex); 
		wasd=AsdCheckBSSIndex(bssindex);

		if(wasd!=NULL){
			
			sta=ap_get_sta(wasd,mac);
	
			if(sta!=NULL){
				if(wasd->send_traffic_limit!=0&&value>wasd->send_traffic_limit){
					ret=ASD_DBUS_ERROR;
				}else{
					sta->vip_flag |= 0x02;
					sta->sta_send_traffic_limit=value;
				}
			}else{
				ret=ASD_STA_NOT_EXIST;
			}
		}else{
			ret=ASD_WLAN_NOT_EXIST;
		}
		pthread_mutex_unlock(&asd_g_sta_mutex); 
	}else{
		ret=ASD_WLAN_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	return reply;

}
DBusMessage *
asd_dbus_show_traffic_limit_by_radio(DBusConnection *conn, DBusMessage *msg, void *user_data){
/*xm0723*/
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	unsigned int radioid = 0, i=0 , j=0;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int bssnum=L_BSS_NUM;
	struct asd_data * wasds[L_BSS_NUM];

	memset(wasds,0,sizeof(struct asd_data *)*L_BSS_NUM);
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
	if(radioid >= G_RADIO_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!AsdCheckRadioID(radioid))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
	}else{

		bssnum=GetBssbyRadio(radioid,wasds);
		
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &bssnum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING

													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													 DBUS_TYPE_BYTE_AS_STRING
													 DBUS_TYPE_BYTE_AS_STRING
													 DBUS_TYPE_BYTE_AS_STRING

													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING

													DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													  	 DBUS_TYPE_BYTE_AS_STRING
													  	 DBUS_TYPE_BYTE_AS_STRING
													   	DBUS_TYPE_BYTE_AS_STRING
													   	DBUS_TYPE_BYTE_AS_STRING
													  	 DBUS_TYPE_BYTE_AS_STRING
													  	 
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		
		for(i=0; i<bssnum; i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			unsigned int sta_num=0;
			struct sta_info *sta=NULL;
			sta_num=wasds[i]->num_sta;
			
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(wasds[i]->own_addr[0]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(wasds[i]->own_addr[1]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(wasds[i]->own_addr[2]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(wasds[i]->own_addr[3]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(wasds[i]->own_addr[4]));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(wasds[i]->own_addr[5]));	

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(wasds[i]->traffic_limit));
			
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(wasds[i]->send_traffic_limit));
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&sta_num);
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			if(sta_num>0){
				sta=wasds[i]->sta_list;
			}
			
			for(j=0;(j<sta_num)&&(sta);j++){

				
				DBusMessageIter iter_sub_struct;
				
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(sta->addr[5]));

				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->sta_traffic_limit));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->sta_send_traffic_limit));
				
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);

				sta=sta->next;
			}

			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
	
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	return reply;

}



DBusMessage *
asd_dbus_show_traffic_limit_by_bssindex(DBusConnection *conn, DBusMessage *msg, void *user_data){
/*xm0723*/
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	unsigned int bssindex = 0, i=0;
	int ret = ASD_DBUS_SUCCESS;
	struct asd_data * wasd=NULL;
	struct sta_info *sta=NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&bssindex,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
	wasd=AsdCheckBSSIndex(bssindex);

	if(bssindex >= G_RADIO_NUM*L_BSS_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!wasd)
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
	}else{

		
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wasd->traffic_limit)); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wasd->send_traffic_limit)); 

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wasd->num_sta)); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		sta = wasd->sta_list;
		
		for(i = 0; (i < wasd->num_sta)&&(sta != NULL) ; i++){
			
			DBusMessageIter iter_struct;

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[0]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[1]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[2]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[3]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[4]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[5]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(sta->sta_traffic_limit));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(sta->sta_send_traffic_limit));

			dbus_message_iter_close_container (&iter_array, &iter_struct);

			sta=sta->next;
		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
	
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;	
}


DBusMessage *asd_dbus_show_mib_info_byrdid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

//	xm0616
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	unsigned int radioid = 0, i=0;
	int ret = ASD_DBUS_SUCCESS;
	BSS_MIB_INFO_ST *buf=NULL;
	unsigned int bssnum = 0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	pthread_mutex_lock(&asd_g_sta_mutex);
	if(radioid >= G_RADIO_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!AsdCheckRadioID(radioid))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
	}else{

		buf=(BSS_MIB_INFO_ST *)os_zalloc(sizeof(BSS_MIB_INFO_ST)*L_BSS_NUM);
		if(buf==NULL){
asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
exit(1);
}
		memset(buf,0,sizeof(BSS_MIB_INFO_ST)*L_BSS_NUM);

		if(0!=GetMibInfoByBss(buf, &bssnum, radioid)){
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s :GetMibInfoByBss() fail.\n",__func__);
			free(buf);
			buf=NULL;
			pthread_mutex_unlock(&asd_g_sta_mutex);		//mahz add 2011.4.20
			return NULL;
		}
		
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &bssnum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
													DBUS_TYPE_UINT64_AS_STRING
													DBUS_TYPE_UINT64_AS_STRING
													DBUS_TYPE_UINT64_AS_STRING
													DBUS_TYPE_UINT64_AS_STRING
													DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING   //fengwenchao modify 20110224
												DBUS_TYPE_UINT64_AS_STRING   //fengwenchao modify 20110224
												DBUS_TYPE_UINT64_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		
		for(i=0; i<bssnum; i++){
			DBusMessageIter iter_struct;

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].wl_up_flow));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].wl_dw_flow));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_pck));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_los_pck));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_mac_err_pck));
				dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_resend_pck));
				dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_up_frm));
				dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_frm));
				dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_err_frm));
				dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_los_frm));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].ch_dw_resend_frm));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,        //fengwenchao modify 20110224
											&(buf[i].ch_up_los_frm));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,        //fengwenchao modify 20110224
											&(buf[i].ch_up_resend_frm));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(buf[i].send_bytes));
			
			dbus_message_iter_close_container (&iter_array, &iter_struct);

		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
	

		free(buf);
		buf=NULL;
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;

}

/*fengwenchao add 20120323*/
DBusMessage *asd_dbus_get_sta_info_new(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply= NULL;
	DBusMessageIter  iter;
	//DBusMessageIter	 iter_array;/*wcl add for globle variable wlan*/
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char wlanid = 0;
	int stanum=0;
	unsigned int num = 0;
	unsigned int asd_stanum = 0;
	unsigned int wlan_num = 0;
	int i = 0;
	int j = 0;
	int wlan_s_or_f[WLAN_NUM] ={0};
	int wlan_id[WLAN_NUM] = {0};
	struct asd_data **bss = NULL;
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_UINT32,&stanum,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
		//exit(1);
	}

	if((wlanid > 0)&&(ASD_WLAN[wlanid] != NULL))
	{
		num = ASD_SEARCH_WLAN_STA(wlanid, bss);
		for(i=0;i<num;i++)
		{
			asd_stanum+=bss[i]->num_sta;
		}
		if(asd_stanum > stanum)
			wlan_s_or_f[0] = 1;
		wlan_num = 1;
		wlan_id[0] = wlanid;	
	}
	else if(wlanid == 0)
	{
		for(i=0; i < WLAN_NUM;i++)
		{
			if(ASD_WLAN[i] != NULL)
			{
				wlan_id[wlan_num] = ASD_WLAN[i]->WlanID;
				num = ASD_SEARCH_WLAN_STA(wlan_id[wlan_num], bss);
				for(j =0;j<num;j++)
				{
					asd_stanum+=bss[j]->num_sta;
				}
				if(asd_stanum > stanum)
					wlan_s_or_f[wlan_num] = 1;
				wlan_num++;
			}
		}
	}
	else
		ret = ASD_WLAN_NOT_EXIST;
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_INT32,		 
								&ret);	
	
	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_INT32,		 
								&wlan_num);	

	for(i =0; i < wlan_num; i++)
	{
		dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_INT32,		 
								&wlan_s_or_f[i]);	
		dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_INT32,		 
								&wlan_id[i]);		
	}

	if(bss)
	{
		free(bss);
		bss=NULL;
	}
	return reply;
}
/*fengwenchao add end*/

//xm add 08/12/10
DBusMessage *asd_dbus_get_sta_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	
	unsigned int type=0;                      	//  	 	1- bss
	unsigned int data;							//		2-wlan
	//unsigned int wlanid;						// 		3-wtp
	unsigned int  wtpid;
	unsigned int radioid;    //fengwenchao add 20110512
	int k1 = 0;   //fengwenchao add 20110512
	int stanum=0;   //fengwenchao modify 20110512
	unsigned char wlan_asd = 0;   //fengwenchao add 20110513
	struct asd_data **bss = NULL;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_UINT32,&data,
								DBUS_TYPE_UINT32,&radioid,      //fengwenchao add 20110512
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	/*get above 4*/
	switch(type){
		case 1:
			/*bss*/
			ret = ASD_WLAN_NOT_EXIST;
			wlan_asd=(unsigned char)data;    //fengwenchao modify 20110512
			/*fengwenchao modify 20110512*/
			for(k1 = radioid*L_BSS_NUM; k1 < ((radioid+1)*L_BSS_NUM);k1++)
			{
				if((ASD_BSS[k1] != NULL)&&(ASD_BSS[k1]->WlanID == wlan_asd))
				{
					ret = ASD_DBUS_SUCCESS;
					break;
				}
				continue;
			}
			struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
			if(ret ==ASD_DBUS_SUCCESS)
			{
				if((interfaces->iface[k1/L_BSS_NUM]!=NULL)&&(interfaces->iface[k1/L_BSS_NUM]->bss !=NULL)&&(interfaces->iface[k1/L_BSS_NUM]->bss[k1%L_BSS_NUM]!=NULL))
					stanum=interfaces->iface[k1/L_BSS_NUM]->bss[k1%L_BSS_NUM]->num_sta;				
			}
			else
				stanum=-1;
			/*fengwenchao modify  end*/
			break;
		case 2:
			/*wlan*/{
			unsigned char wlanid=0;
			unsigned int num = 0;
			int i=0;
			
			wlanid=(unsigned char)data;
			num = ASD_SEARCH_WLAN_STA(wlanid, bss);
			for(i=0;i<num;i++)
				stanum+=bss[i]->num_sta;
				}
			break;
		case 3:
			/*wtp*/{
			unsigned int num = 0;
			int i=0;
			wtpid=data;
			num= ASD_SEARCH_WTP_HAS_STA(wtpid, bss);
	
			for(i=0;i<num;i++)
				stanum+=bss[i]->num_sta;
			}
			break;
		default:
			stanum=-1;
			break;
	}

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_INT32,        //fengwenchao modify 20110512
								&stanum);
	free(bss);
	bss=NULL;
	return reply;
}
//sz20080825 
DBusMessage *asd_dbus_set_valn_list_append_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int ret = ASD_DBUS_SUCCESS;
	int num=0;
	int i=0;
	
	dbus_error_init(&err);

	VLAN_PORT_ENABLE vlani[4095];
	memset(vlani,0,sizeof(VLAN_PORT_ENABLE)*4095);


	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&num);

			
	
	if(num> 0 ){
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);


		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
					
			dbus_message_iter_recurse(&iter_array,&iter_struct);
				
			dbus_message_iter_get_basic(&iter_struct,&(vlani[i].vlanid));
				
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(vlani[i].port));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(vlani[i].stat));
				
			dbus_message_iter_next(&iter_array);

			
		}
	
	}
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		
	return reply;		

}



DBusMessage *asd_dbus_set_valn_append_security(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter,iiter;
	DBusMessageIter  iter_array;
	int ret = ASD_DBUS_SUCCESS;
		
	int num=0;
	int i=0;
		
	VLAN_PORT_SECURITY vlani[4095];
	memset(vlani,0,sizeof(VLAN_PORT_SECURITY)*4095);
				
			
	dbus_error_init(&err);
		
	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&num);
	
					
	if(num> 0 ){		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
					
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
						
			dbus_message_iter_recurse(&iter_array,&iter_struct);
							
			dbus_message_iter_get_basic(&iter_struct,&(vlani[i].vlanid));
					
			dbus_message_iter_next(&iter_struct);
						
			dbus_message_iter_get_basic(&iter_struct,&(vlani[i].port));
	
			dbus_message_iter_next(&iter_struct);
						
			dbus_message_iter_get_basic(&iter_struct,&(vlani[i].securityid));
					
			dbus_message_iter_next(&iter_array);

		}
		
	}
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iiter);
		
	dbus_message_iter_append_basic(&iiter, DBUS_TYPE_UINT32, &ret);
			
	return reply;	

}

DBusMessage *asd_dbus_set_port_valn_append_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int ret = ASD_DBUS_SUCCESS;
	
	
	 
	SLOT_PORT_VLAN_ENABLE vlan1;

	dbus_error_init(&err);

	dbus_message_iter_init(msg,&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
		
	DBusMessageIter iter_struct;
	
	dbus_message_iter_recurse(&iter_array,&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.slot));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.port));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.vlanid));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.portindex));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.stat));
					
	dbus_message_iter_next(&iter_array);


	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_UINT32,
								 &ret);
			
	return reply;

}

DBusMessage *asd_dbus_set_port_valn_append_security(DBusConnection *conn, DBusMessage *msg, void *user_data)
{	
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int ret = ASD_DBUS_SUCCESS;
	
	
	 
	SLOT_PORT_VLAN_SECURITY vlan1;
	

	dbus_error_init(&err);


	dbus_message_iter_init(msg,&iter);

	dbus_message_iter_recurse(&iter,&iter_array);
		
	DBusMessageIter iter_struct;
	
	dbus_message_iter_recurse(&iter_array,&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.slot));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.port));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.vlanid));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.portindex));

	dbus_message_iter_next(&iter_struct);

	dbus_message_iter_get_basic(&iter_struct,&(vlan1.securityid));
					
	dbus_message_iter_next(&iter_array);

	asd_printf(ASD_DBUS,MSG_DEBUG,"%d %d %d",vlan1.vlanid,vlan1.portindex,vlan1.securityid);
	if(ASD_SECURITY[vlan1.securityid] == NULL)
		ret = ASD_SECURITY_NOT_EXIST;
	else {
		ret = ASD_SECURITY_PROFILE_CHECK(vlan1.securityid);		
		if(ret == ASD_DBUS_SUCCESS)
			Create_WIRED_node(vlan1.vlanid,vlan1.portindex,vlan1.securityid);
	}
		

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
		
	return reply;
}

//sz20080825 
//////////////////////////////////////////////////////////////////////////////////////////////

//ht add,08.12.01
DBusMessage *asd_dbus_set_asd_daemonlog_debug(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	char loglevel[20];	
	int ret = ASD_DBUS_SUCCESS;
	unsigned int daemonlogtype = 0;
	unsigned int daemonloglevel = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&daemonlogtype,
								DBUS_TYPE_UINT32,&daemonloglevel,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	if(daemonloglevel == 1)	{
		gASDDEBUG |= daemonlogtype;
		strcpy(loglevel, "open");
	}	else if(daemonloglevel == 0)	{
		gASDDEBUG &= ~daemonlogtype;
		strcpy(loglevel, "close");
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd daemonlog debug %d\n",gASDDEBUG);
	return reply;	

}

//ht add,08.12.04
DBusMessage *asd_dbus_set_asd_logger_printflag(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	char loglevel[20];	
	int ret = ASD_DBUS_SUCCESS;
	unsigned char printflag = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&printflag,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	if(printflag == 1)	{
		gasdPRINT = 1;
		strcpy(loglevel, "open");
	}	else if(printflag == 0)	{
		gasdPRINT = 0;
		strcpy(loglevel, "close");
	}

	
	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd logger printflag %s\n",loglevel);
	return reply;	

}
//qiuchen add it for Henan Mobile
DBusMessage *set_asd_log_group_activated(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned char group = 0;
	unsigned char switchi = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group,
								DBUS_TYPE_BYTE,&switchi,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(switchi == 1)
		gASDLOGDEBUG |= group;
	else if(switchi == 0)
		gASDLOGDEBUG &= (~group);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd log group %x\n",gASDLOGDEBUG);
	return reply;	

}
DBusMessage *set_ac_management_ip(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned long man_ip = 0;	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&man_ip,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	gASD_AC_MANAGEMENT_IP = man_ip;
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	asd_printf(ASD_DBUS,MSG_DEBUG,"set ac management ip addr %lu.%lu.%lu.%lu\n",((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
					((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
	return reply;	
	
}

DBusMessage *show_ac_management_ip(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT64, &gASD_AC_MANAGEMENT_IP);
	
	return reply;	
	
}
//end
DBusMessage *asd_dbus_set_asd_daemonlog_level(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned int daemonloglevel = 5;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&daemonloglevel,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	if(daemonloglevel == 1)	{
		wpa_debug_level = MSG_MSGDUMP;
	}
	else if(daemonloglevel == 2){
		wpa_debug_level = MSG_DEBUG;
	}
	else if(daemonloglevel == 3){
		wpa_debug_level = MSG_INFO;
	}
	else if(daemonloglevel == 4){
		wpa_debug_level = MSG_NOTICE;
	}
	else if(daemonloglevel == 5){
		wpa_debug_level = MSG_WARNING;
	}
	else if(daemonloglevel == 6){
		wpa_debug_level = MSG_ERROR;
	}
	else if(daemonloglevel == 7){
		wpa_debug_level = MSG_CRIT;
	}
	else if(daemonloglevel == 8){
		wpa_debug_level = MSG_ALERT;
	}
	else if(daemonloglevel == 9){
		wpa_debug_level = MSG_EMERG;
	}
	else
		wpa_debug_level = MSG_WARNING;
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd daemonlog level %d\n",wpa_debug_level);
	return reply;	
}

/****************************************************************************************/
//xm 08/09/02
DBusMessage *asd_dbus_secondary_set_acct(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter	 iter;
	char* acct_ip;
	int i, isApply = 0;
	unsigned char security_id;
	unsigned int acct_port;
	char* acct_shared_secret;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;


	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&acct_port,
								DBUS_TYPE_STRING,&acct_ip,
								DBUS_TYPE_STRING,&acct_shared_secret,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}


	pthread_mutex_lock(&asd_g_wlan_mutex);
	if(ASD_SECURITY[security_id] != NULL)
	{   
		if(ASD_SECURITY[security_id]->acct.acct_ip!=NULL&&ASD_SECURITY[security_id]->acct.acct_shared_secret!=NULL)
			{

				//I want to know whether we get the right parameter/////////////////////////////
				//asd_printf(ASD_DBUS,MSG_DEBUG,"============================= asd ===================================\n");
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get security id %d\n",security_id);
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get ip %s\n",acct_ip);
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get port %d\n",acct_port);
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get shared secret %s\n",acct_shared_secret);
				///////////////////////////////////////////////////////////////////////


				for(i = 0; i < WLAN_NUM; i++)
					if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
					{
						ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
						break;
					}
					else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
					{
						isApply = 1;
						continue;
					}
	
				if(ret == 0)
					{
						if(isApply == 1)
							Clear_WLAN_APPLY(security_id);
						//mahz modified 2011.3.1
						if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||(ASD_SECURITY[security_id]->extensible_auth == 1)||(ASD_SECURITY[security_id]->wapi_radius_auth == 1))
							{
							if(ASD_SECURITY[security_id]->heart_test_on == 0)//qiuchen
								ret = ASD_SECONDARY_SET_ACCT(security_id,acct_port,acct_ip,acct_shared_secret);
								//asd_printf(ASD_DBUS,MSG_DEBUG,"set complete!\n");///////////////////////////////
							else
								ret = ASD_SECURITY_HEART_TEST_ON;
							}
						else
							ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
					}	
			}
		else
			{
				//asd_printf(ASD_DBUS,MSG_DEBUG,"you have not set acct before.\n");/////////////////////////////
				ret=ASD_SECURITY_AUTH_NOT_EXIST;
			}
	}
	else{
	
		ret = ASD_SECURITY_NOT_EXIST;
	}

	pthread_mutex_unlock(&asd_g_wlan_mutex);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_UINT32,
								&ret);
	return reply;

}
/*nl add 20100307*/
DBusMessage *asd_dbus_wep_index_period(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned char wep_index;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&wep_index,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	pthread_mutex_lock(&asd_g_wlan_mutex);	
	if((ASD_SECURITY[security_id] != NULL) && (ASD_SECURITY[security_id]->encryptionType == WEP)
		&&((ASD_SECURITY[security_id]->securityType == OPEN)||(ASD_SECURITY[security_id]->securityType == SHARED)))
	{		
		int i =0, isApply = 0;
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			ASD_SECURITY[security_id]->index = wep_index;			
		}
	}	else if(ASD_SECURITY[security_id] == NULL){
		ret = ASD_SECURITY_NOT_EXIST;
	}else{
		ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
	}
		
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
//	xm0701
DBusMessage *asd_dbus_wapi_rekey_para(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	unsigned int value=0;
	unsigned char uorm=0, torp=0;
			
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&uorm,
								DBUS_TYPE_BYTE,&torp,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID))){
		
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
						
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
		
	
	pthread_mutex_lock(&asd_g_wlan_mutex);
	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
		}
	
		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
					
			security_type = ASD_SECURITY[security_id]->securityType;
				
			if(security_type== WAPI_AUTH||security_type== WAPI_PSK){
				if(uorm==0){
					if( ASD_SECURITY[security_id]->wapi_ucast_rekey_method!=0){
						if(torp==0){
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_t=value;
						}else if(torp==1){
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_p=value;
						}else
							return NULL;
					}else
						ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;

				}else if(uorm==1){
					if( ASD_SECURITY[security_id]->wapi_mcast_rekey_method!=0){
						if(torp==0){
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_t=value;
						}else if(torp==1){
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_p=value;
						}else
							return NULL;
					}else
						ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
				}else
					return NULL;
	
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
			
			
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
										&ret);
	return reply;
}


//	xm0701
DBusMessage *asd_dbus_wapi_ucast_rekey_method(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id ,method=1 ,uorm=0;
	unsigned int security_type=0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&uorm,
								DBUS_TYPE_BYTE,&method,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				if(uorm==0){
					ASD_SECURITY[security_id]->wapi_ucast_rekey_method=method;
					/*
					switch(method){
						case 0:		//disable
							ASD_SECURITY[security_id]->wapi_ucast_rekey_method=0;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_t=0;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_p=0;
							break;
						case 1:		//time_based
							ASD_SECURITY[security_id]->wapi_ucast_rekey_method=1;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_t=86400;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_p=0;
							break;
						case 2:		//packet_based
							ASD_SECURITY[security_id]->wapi_ucast_rekey_method=2;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_t=0;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_p=100000;
							break;
						case 3:		//both_based
							ASD_SECURITY[security_id]->wapi_ucast_rekey_method=3;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_t=86400;
							ASD_SECURITY[security_id]->wapi_ucast_rekey_para_p=100000;
							break;
						default:
							break;
					}
					*/
				}else if(uorm==1){
					ASD_SECURITY[security_id]->wapi_mcast_rekey_method=method;
					/*
					switch(method){
						case 0:		//disable
							ASD_SECURITY[security_id]->wapi_mcast_rekey_method=0;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_t=0;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_p=0;
							break;
						case 1:		//time_based
							ASD_SECURITY[security_id]->wapi_mcast_rekey_method=1;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_t=86400;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_p=0;
							break;
						case 2:		//packet_based
							ASD_SECURITY[security_id]->wapi_mcast_rekey_method=2;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_t=0;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_p=100000;
							break;
						case 3:		//both_based
							ASD_SECURITY[security_id]->wapi_mcast_rekey_method=3;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_t=86400;
							ASD_SECURITY[security_id]->wapi_mcast_rekey_para_p=100000;
							break;
						default:
							break;
					}
					*/
				}else
					return NULL;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
DBusMessage *asd_dbus_eap_sm_run_activated(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply;
	DBusMessageIter iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned char type = 0;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	pthread_mutex_lock(&asd_g_sta_mutex);	
	if((ASD_SECURITY[security_id] != NULL))
	{		
			security_type = ASD_SECURITY[security_id]->securityType;
			if(((security_type!= OPEN)&&(security_type!= SHARED))||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0)))
			{			
				if(type == 1){
					ASD_SECURITY[security_id]->eap_sm_run_activated = type;
				}
				else if(type == 0){
					if(ASD_SECURITY[security_id]->eap_sm_run_activated != type){
						ASD_SECURITY[security_id]->eap_sm_run_activated = type;
						eap_sm_run_cancel_timer(security_id);
					}
				}
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	pthread_mutex_unlock(&asd_g_sta_mutex); 
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}


DBusMessage *asd_dbus_eap_reauth_period(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int period;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&period,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
			//mahz modified 2011.3.7	
			security_type = ASD_SECURITY[security_id]->securityType;
			//if(((security_type == OPEN)&&((encryption_type == NONE)||(encryption_type == WEP)))||((security_type == SHARED)&&(encryption_type == WEP))||((security_type == IEEE8021X)&&(encryption_type == WEP))||(((security_type == WPA_P)||(security_type == WPA_E)||(security_type == WPA2_P)||(security_type == WPA2_E))&&((encryption_type == AES)||(encryption_type == TKIP))))
			if(((security_type!= OPEN)&&(security_type!= SHARED))||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0)))
			{ 			//////////////////////////////////////////////////////////////////////////////
				//if(ASD_SECURITY[security_id]->eap_reauth_priod!= period)
						//Clear_SECURITY(security_id);
				
				ASD_SECURITY[security_id]->eap_reauth_priod= period;



				//////////////////////////////////////////////////////////////////////////
				//asd_printf(ASD_DBUS,MSG_DEBUG,"========================== asd =======================================\n");
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: set eap reauth_priod %d\n",ASD_SECURITY[security_id]->eap_reauth_priod);
				//////////////////////////////////////////////////////////////////////////
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

//ht add,090205
DBusMessage *asd_dbus_acct_interim_interval(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int interval;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&interval,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if(((security_type!= OPEN)&&(security_type!= SHARED))||(ASD_SECURITY[security_id]->extensible_auth == 1))
			{ 			
				ASD_SECURITY[security_id]->acct_interim_interval= interval;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_set_quiet_period(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int quietPeriod;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&quietPeriod,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}


	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if(((security_type!= OPEN)&&(security_type!= SHARED))||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0)))
			{ 			
				ASD_SECURITY[security_id]->quiet_Period= quietPeriod;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_secondary_set_auth(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter	 iter;
	char* auth_ip;
	int i, isApply = 0;
	unsigned char security_id;
	unsigned int auth_port;
	char* auth_shared_secret;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;


	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&auth_port,
								DBUS_TYPE_STRING,&auth_ip,
								DBUS_TYPE_STRING,&auth_shared_secret,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}


	if(ASD_SECURITY[security_id] != NULL)
	{   
		if(ASD_SECURITY[security_id]->auth.auth_ip!=NULL&&ASD_SECURITY[security_id]->auth.auth_shared_secret!=NULL)
			{

				//I want to know whether we get the right parameter/////////////////////////////
				//asd_printf(ASD_DBUS,MSG_DEBUG,"============================= asd ===================================\n");
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get security id %d\n",security_id);
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get ip %s\n",auth_ip);
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get port %d\n",auth_port);
				//asd_printf(ASD_DBUS,MSG_DEBUG,"asd: get shared secret %s\n",auth_shared_secret);
				///////////////////////////////////////////////////////////////////////


				pthread_mutex_lock(&asd_g_wlan_mutex);
				for(i = 0; i < WLAN_NUM; i++)
					if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
					{
						ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
						break;
					}
					else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
					{
						isApply = 1;
						continue;
					}
	
				if(ret == 0)
					{
						if(isApply == 1)
							Clear_WLAN_APPLY(security_id);
						//mahz modified 2011.3.1
						if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||(ASD_SECURITY[security_id]->extensible_auth == 1)||(ASD_SECURITY[security_id]->wapi_radius_auth == 1))
							{
								if(ASD_SECURITY[security_id]->heart_test_on == 0)//qiuchen
								ret = ASD_SECONDARY_SET_AUTH(security_id,auth_port,auth_ip,auth_shared_secret);
								else
									ret = ASD_SECURITY_HEART_TEST_ON;
							//	asd_printf(ASD_DBUS,MSG_DEBUG,"set complete!\n");///////////////////////////////
							}
						else
							ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
					}	
				pthread_mutex_unlock(&asd_g_wlan_mutex);
			}
		else
			{
				//asd_printf(ASD_DBUS,MSG_DEBUG,"you have not set auth before.\n");/////////////////////////////
				ret=ASD_SECURITY_AUTH_NOT_EXIST;
			}
	}
	else{
	
		ret = ASD_SECURITY_NOT_EXIST;
	}

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_UINT32,
								&ret);
	return reply;

}
//xm 08/09/02
/*************************************************/


//=================================================
// xm add 08/10/31
DBusMessage *asd_dbus_kick_sta(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	unsigned char *mac;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int len = 0;
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		free(mac);
		mac = NULL;   //  0608xm
		return NULL;
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	
	ASDCmdMsg cmdmsg;
	memset(&cmdmsg, 0, sizeof(ASDCmdMsg));	
	cmdmsg.Op = ASD_CMD_DEL;
	cmdmsg.Type = ASD_STA_TYPE;
	memcpy(cmdmsg.u.stainfo.MAC, mac, ETH_ALEN);
	len = sizeof(cmdmsg);
	if(sendto(CmdSock_s, &cmdmsg, len, 0, (struct sockaddr *) &toASD_C.addr, toASD_C.addrlen) < 0){
		perror("send(wASDSocket)");
		asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
	}
#if 0
	pthread_mutex_lock(&asd_g_sta_mutex);   
	bss_num= ASD_SEARCH_BSS_HAS_STA(bss);
	for(i=0;i<bss_num;i++){
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
		sta = ap_get_sta(bss[i], mac);
		if(sta != NULL){
			sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
			bss[i]->normal_st_down_num++;						//mahz add 2011.1.14
			unsigned char SID = (unsigned char)bss[i]->SecurityID;
			if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)&&(ASD_SECURITY[SID]->hybrid_auth == 0))){
				wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
				mlme_deauthenticate_indication(
				bss[i], sta, 0);
				sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
				ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
			}
			if(ASD_WLAN[bss[i]->WlanID]!=NULL&&ASD_WLAN[bss[i]->WlanID]->balance_switch == 1&&ASD_WLAN[bss[i]->WlanID]->balance_method==1){
				if(ASD_NOTICE_STA_INFO_TO_PORTAL)
					UpdateStaInfoToCHILL(bss[i]->BSSIndex,sta,WID_DEL);	//mahz modified 2011.3.15
				ap_free_sta(bss[i], sta, 1);
			}
			else{				
				if(ASD_NOTICE_STA_INFO_TO_PORTAL)
					UpdateStaInfoToCHILL(bss[i]->BSSIndex,sta,WID_DEL);	//mahz modified 2011.3.15
				ap_free_sta(bss[i], sta, 0);
			}
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
				asd_printf(ASD_DBUS,MSG_DEBUG,"1111111111111\n");
				if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
					perror("send(wASDSocket)");
					asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
				//	close(sock);
				}
				
				asd_printf(ASD_DBUS,MSG_DEBUG,"3333333333\n");
			//	close(sock);
			}
		}		
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);

	if(sta_flag==0)
		ret=ASD_STA_NOT_EXIST;
	
	sta_flag=0;
#endif	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 

	free(mac);
	mac = NULL;
	return reply;	

}

//mahz add 2011.6.3 
DBusMessage *asd_dbus_radius_force_sta_downline(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	char *acct_id = NULL;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	struct sta_info *sta=NULL;
	int bss_num=0;
	int i=0;
	//int acct_id = 0;
	int sta_flag=0;
	char buf[20]={0};
	//struct asd_data *bss[BSS_NUM];
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_STRING,&acct_id,
								//DBUS_TYPE_UINT32,&acct_id,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(bss);
		bss = NULL;
		
		return NULL;
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append(reply, &iter);	

	pthread_mutex_lock(&asd_g_sta_mutex);	
	bss_num= ASD_SEARCH_BSS_HAS_STA(bss);
	for(i=0;i<bss_num;i++){
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
		if(bss[i]->sta_list != NULL)
			sta = bss[i]->sta_list;
		while(sta != NULL){
			os_snprintf(buf, sizeof(buf), "%08X-%08X", sta->acct_session_id_hi, sta->acct_session_id_lo);
			asd_printf(ASD_DBUS,MSG_DEBUG,"acct_id: %s\n",buf); 	//for test
			//if(acct_id == sta->acct_session_id_lo){
			if(os_memcmp(acct_id, buf, strlen(acct_id)) == 0){
				sta_flag=1;
				sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
				//circle_register_timeout(2, 0, ap_free_sta, bss[i], sta);
				
				//let wid know this sta is not allowed.
				TableMsg STA;
				int len;
				STA.Op = WID_DEL;
				STA.Type = STA_TYPE;
				STA.u.STA.BSSIndex = bss[i]->BSSIndex;
				STA.u.STA.WTPID = ((bss[i]->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
				len = sizeof(STA);
				asd_printf(ASD_DBUS,MSG_DEBUG,"1111111111111\n");
				if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
					perror("send(wASDSocket)");
					asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
				}
				asd_printf(ASD_DBUS,MSG_DEBUG,"3333333333\n");

				sta = NULL;
			}
			else if(sta->next != NULL)
				sta = sta->next;
			else
				sta = NULL;
		}		
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);

	if(sta_flag==0)
		ret=ASD_STA_NOT_EXIST;
	
	sta_flag=0;
	
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32,
									&ret); 
	if(bss){
		free(bss);
		bss = NULL;
	}

	return reply;	

}
//

/*nl add091028*/
DBusMessage *asd_dbus_set_sta_vlanid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	unsigned char *mac;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct asd_stainfo *stainfo;
	unsigned int vlan_id;
	
	mac = (unsigned char*)malloc(MAC_LEN);
	if(mac==NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_UINT32,&vlan_id,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
						
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; 
		return NULL;
	}

	stainfo = ASD_SEARCH_STA(mac);
		
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);

	if(stainfo != NULL){
			if(vlan_id < 4096)
				stainfo->sta->vlan_id = vlan_id;
				
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(stainfo->bss->WlanID));
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(stainfo->bss->Radio_G_ID));
			

	}else{
		ret = ASD_STA_NOT_EXIST;
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 		
	}
	
	free(mac);
	mac = NULL;
	if(stainfo != NULL){
		stainfo->bss = NULL;
		stainfo->sta = NULL;
		free(stainfo);
		stainfo = NULL;
	}
	return reply;	

}

DBusMessage *asd_dbus_check_sta_bymac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char	type;
	unsigned int	value;
	unsigned char	mac[MAC_LEN];
	struct asd_stainfo *stainfo = NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
	stainfo = ASD_SEARCH_STA(mac);
	
	if(stainfo != NULL){
		if(type == 0)
			stainfo->sta->vlan_id = value;
		else if(type == 1){
			if(stainfo->bss->traffic_limit!=0 && value>stainfo->bss->traffic_limit){
				ret=ASD_DBUS_SET_ERROR;
			}else{
				stainfo->sta->vip_flag |= 0x01;	/*set bit0*/
				stainfo->sta->sta_traffic_limit=value;
			}
		}else if(type == 2){
			if(stainfo->bss->send_traffic_limit!=0 && value>stainfo->bss->send_traffic_limit){
				ret=ASD_DBUS_SET_ERROR;
			}else{
				stainfo->sta->vip_flag |= 0x02;
				stainfo->sta->sta_send_traffic_limit=value;
			}
		}
		
		if(ret == ASD_DBUS_SUCCESS){
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &ret); 		
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->bss->WlanID));
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->bss->Radio_G_ID));
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->bss->BSSIndex));
		}else {
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &ret); 		
		}
	}else{
		ret = ASD_STA_NOT_EXIST;
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 		

	}
	if(stainfo != NULL){
		stainfo->bss = NULL;
		stainfo->sta = NULL;
		free(stainfo);
		stainfo = NULL;
	}

	return reply;	

}


/*ht add 100113*/
DBusMessage *asd_dbus_set_sta_static_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char	type;
	unsigned int	value;
	unsigned char	wlanid=0;
	unsigned char	ret_flag=0;
	unsigned char	mac[MAC_LEN];
	char	*name[3]={"vlanid","limit","send_limit"};
	struct sta_static_info	*sta = NULL;
	struct sta_static_info	*sta_tmp = NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
						
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	sta = asd_get_static_sta(mac);
	if(sta == NULL) {
		if((sta = (struct sta_static_info *)malloc(sizeof(*sta))) == NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			return NULL;
			//exit(1);
		}	
		memset(sta, 0, sizeof(*sta));
		memcpy(sta->addr,mac,MAC_LEN);
		asd_static_sta_hash_add(sta);
		ret_flag = 1;
	}
	while((ret_flag != 1)&&(sta != NULL)){
		asd_printf(ASD_DBUS,MSG_DEBUG,"sta->wlan_id = %d , wlanid = %d\n",sta->wlan_id,wlanid);
		if((os_memcmp(sta->addr, mac, 6) == 0)&&(sta->wlan_id == wlanid)){
			break;
		}
		sta = sta->hnext;
	}
	if(sta == NULL){
		if((sta_tmp = (struct sta_static_info *)malloc(sizeof(*sta_tmp))) == NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			return NULL;
		}	
		memset(sta_tmp, 0, sizeof(*sta_tmp));
		memcpy(sta_tmp->addr,mac,MAC_LEN);
		asd_static_sta_hash_add(sta_tmp);

		sta_tmp->wlan_id = wlanid;
		if(type == 0)
			sta_tmp->vlan_id = value;
		else if(type == 1)
			sta_tmp->sta_traffic_limit = value;
		else if(type == 2)
			sta_tmp->sta_send_traffic_limit = value;
	}
	else{
		sta->wlan_id = wlanid;
		if(type == 0)
			sta->vlan_id = value;
		else if(type == 1)
			sta->sta_traffic_limit = value;
		else if(type == 2)
			sta->sta_send_traffic_limit = value;
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"Set static sta "MACSTR" %s %d by wlan %d successfully!\n",MAC2STR(mac),name[type],value,wlanid);
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 		
	
	return reply;	

}

/*ht add 100113*/
DBusMessage *asd_dbus_del_sta_static_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int result = 0;
	unsigned char	mac[MAC_LEN];
	unsigned char	wlanid = 0;
	struct sta_static_info	*sta = NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
						
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	sta = asd_get_static_sta(mac);
	if(sta == NULL) {
		ret = ASD_STA_NOT_EXIST;
	}else{
		result = asd_static_sta_hash_del(sta,wlanid);
		if(result != 0){
			ret = ASD_STA_NOT_EXIST;
		}
		else
			asd_printf(ASD_DBUS,MSG_DEBUG,"Del static sta "MACSTR" successfully!\n",MAC2STR(mac));
	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 		
	
	return reply;	

}


/*ht add 100113*/
DBusMessage *asd_dbus_show_sta_static_info_bymac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int i=0;
	unsigned int 	num = 0;
	unsigned char	mac[MAC_LEN];
	unsigned char	wlanid = 0;
	struct sta_static_info	*sta = NULL;
	struct sta_static_info	sta_array[2048];
	memset(sta_array, 0, sizeof(sta_array));
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
						
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	pthread_mutex_lock(&asd_g_sta_mutex);
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);

	sta = asd_get_static_sta(mac);
	if(sta == NULL) {
		ret = ASD_STA_NOT_EXIST;
		num = 0;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num); 		
	}else {
		if(wlanid != 0){
			while(sta != NULL){
				asd_printf(ASD_DBUS,MSG_DEBUG,"1 sta->wlan_id = %d , wlanid = %d\n",sta->wlan_id,wlanid);
				if((os_memcmp(sta->addr, mac, 6) == 0)&&(sta->wlan_id == wlanid)){
					num = 1;
					memcpy(&sta_array[0],sta,sizeof(struct sta_static_info));
					break;
				}
				sta = sta->hnext;
			}
			if(sta == NULL){
				num = 0;
				ret = ASD_STA_NOT_EXIST;
			}
		}
		else{
			while(sta != NULL){
				asd_printf(ASD_DBUS,MSG_DEBUG,"2 sta->wlan_id = %d , wlanid = %d\n",sta->wlan_id,wlanid);
				if(os_memcmp(sta->addr, mac, 6) == 0){
					memcpy(&sta_array[num++],sta,sizeof(struct sta_static_info));
					if(num >= 2048)
						break;
				}
				sta = sta->hnext;
			}
			if(num == 0){
				ret = ASD_STA_NOT_EXIST;
			}
		}

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num); 
		for(i=0;i<num && i<2048;i++){
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].addr[0])); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].addr[1])); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].addr[2])); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].addr[3])); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].addr[4])); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].addr[5])); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(sta_array[i].vlan_id)); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(sta_array[i].sta_traffic_limit)); 		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(sta_array[i].sta_send_traffic_limit));
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_BYTE,
											 &(sta_array[i].wlan_id)); 		
		} 		
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	return reply;	

}


/*ht add 100113*/
DBusMessage *asd_dbus_show_sta_static_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int	i;
	struct sta_static_info	*sta = NULL;
	
	pthread_mutex_lock(&asd_g_sta_mutex);
	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &sta_static_num);

	dbus_message_iter_open_container (&iter,
									   DBUS_TYPE_ARRAY,
									   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
									   DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);

	for(i=0; i<STA_HASH_SIZE; i++){
		sta = STA_STATIC_TABLE[i];
		while(sta != NULL){
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(sta->addr[0]));	   
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(sta->addr[1]));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(sta->addr[2]));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(sta->addr[3]));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(sta->addr[4]));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(sta->addr[5]));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(sta->vlan_id));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(sta->sta_traffic_limit));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(sta->sta_send_traffic_limit));		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(sta->wlan_id));		
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			sta = sta->hnext;
		}
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	return reply;	

}

//ht add 090622
DBusMessage *asd_dbus_show_wapi_info_bywtpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;		
	int ret = ASD_DBUS_SUCCESS;

	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned char ap_addr[MAC_LEN]={0};
	unsigned int ap_version = VERSIONNOW;
	wapi_stats_Entry *bss_wapi_mib_stat = NULL;
	wapi_stats_Entry  MIBNULL;
	unsigned int num = 0;
	int i = 0;

	unsigned int WTPID = 0;
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

	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WTPID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	pthread_mutex_lock(&asd_g_sta_mutex);
	if(WTPID >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!AsdCheckWTPID(WTPID))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 				
	}else{
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		num = ASD_SEARCH_WTP_STA(WTPID, bss);
		if(bss[0]!=NULL) 
			memcpy(ap_addr,bss[0]->own_addr,MAC_LEN);

		for(i=0; i<num; i++) {
			if ((bss[i]->wapi_wasd == NULL)||(bss[i]->wapi_wasd->vap_user == NULL)||(bss[i]->wapi_wasd->vap_user->wapid == NULL)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"bss[%d] wapi_mib_stats ==	NULL\n",i);
				continue;
			}
			bss_wapi_mib_stat = &bss[i]->wapi_wasd->vap_user->wapid->wapi_mib_stats;

			wai_sign_errors += bss_wapi_mib_stat->wai_sign_errors ;
			wai_hmac_errors += bss_wapi_mib_stat->wai_hmac_errors ;
			wai_auth_res_fail += bss_wapi_mib_stat->wai_auth_res_fail ;
			wai_discard += bss_wapi_mib_stat->wai_discard ;
			wai_timeout += bss_wapi_mib_stat->wai_timeout ;
			wai_format_errors += bss_wapi_mib_stat->wai_format_errors ;
			wai_cert_handshake_fail += bss_wapi_mib_stat->wai_cert_handshake_fail ;
			wai_unicast_handshake_fail += bss_wapi_mib_stat->wai_unicast_handshake_fail ;
			wai_multi_handshake_fail += bss_wapi_mib_stat->wai_multi_handshake_fail ;
			wpi_mic_errors += bss_wapi_mib_stat->wpi_mic_errors ;
			wpi_replay_counters += bss_wapi_mib_stat->wpi_replay_counters ;
			wpi_decryptable_errors += bss_wapi_mib_stat->wpi_decryptable_errors ;
		}
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_BYTE,
										&ap_addr[0]);
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_BYTE,
										&ap_addr[1]);
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_BYTE,
										&ap_addr[2]);
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_BYTE,
										&ap_addr[3]);
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_BYTE,
										&ap_addr[4]);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &ap_addr[5]);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ap_version);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_sign_errors);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_hmac_errors);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_auth_res_fail);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_discard);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_timeout);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_format_errors);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_cert_handshake_fail);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_unicast_handshake_fail);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wai_multi_handshake_fail);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wpi_mic_errors);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wpi_replay_counters);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wpi_decryptable_errors);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i = 0; i < num ; i++){			
			DBusMessageIter iter_struct;
			unsigned char WapiEnabled = 0;
			
			if ((bss[i]->wapi == 1) || (bss[i]->wapi == 2))
				WapiEnabled = 1;
			if ((bss[i]->wapi_wasd == NULL)||(bss[i]->wapi_wasd->vap_user == NULL)||(bss[i]->wapi_wasd->vap_user->wapid == NULL)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"bss[%d] wapi_mib_stats ==	NULL\n",i);
				memset(&MIBNULL,0,sizeof(wapi_stats_Entry));
				bss_wapi_mib_stat = &MIBNULL;
			}else
				bss_wapi_mib_stat = &bss[i]->wapi_wasd->vap_user->wapid->wapi_mib_stats;

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->WlanID));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->BSSIndex));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->own_addr[0]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->own_addr[1]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->own_addr[2]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->own_addr[3]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->own_addr[4]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->own_addr[5]));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(WapiEnabled));
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_sign_errors));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_sign_errors));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_auth_res_fail));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_discard));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_timeout));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_format_errors));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_cert_handshake_fail));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_unicast_handshake_fail));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wai_multi_handshake_fail));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wpi_mic_errors));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wpi_replay_counters));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss_wapi_mib_stat->wpi_decryptable_errors));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	return reply;

}


//ht add 090428
DBusMessage *asd_dbus_show_radio_info_bywtpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	struct asd_data *bss[L_BSS_NUM];
	unsigned int radionum = 0;
	unsigned int radio[L_RADIO_NUM];
	unsigned int WTPID = 0;
	int i = 0, k = 0;
	int ret = ASD_DBUS_SUCCESS;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WTPID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	pthread_mutex_lock(&asd_g_sta_mutex);
	if(WTPID >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!AsdCheckWTPID(WTPID))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
	}else{
	
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
			struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
			if(interfaces->iface[i] == NULL)
				continue;
			else {
				radio[radionum]=i;
				radionum++;
			}
		}
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s radionum = %d\n",__func__,radionum);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &radionum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		
		for(k=0; k<radionum; k++){
			DBusMessageIter iter_struct;
			unsigned int num = 0;
			unsigned int acc_tms = 0; 
			unsigned int auth_tms = 0; 
			unsigned int repauth_tms = 0; 
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
		/*	
			unsigned int rx_mgmt_pkts = 0;
			unsigned int tx_mgmt_pkts = 0;
			unsigned int rx_ctrl_pkts = 0;
			unsigned int tx_ctrl_pkts = 0;
			unsigned int rx_data_pkts = 0;
			unsigned int tx_data_pkts = 0;
		*/
			unsigned int rx_auth_pkts = 0;
			unsigned int tx_auth_pkts = 0;
			
			asd_printf(ASD_DBUS,MSG_DEBUG,"radio[%d] = %d\n",k,radio[k]);
			num = ASD_SEARCH_RADIO_STA(radio[k], bss);	
			for(i=0; i<num; i++) {
				
				if (bss[i]->info == NULL) {
					asd_printf(ASD_DBUS,MSG_DEBUG,"bss[%d]==NULL\n",i);
					continue;
				}
				GetBssDataPkts(bss[i]);
				acc_tms += bss[i]->acc_tms;
				auth_tms += bss[i]->usr_auth_tms;
				repauth_tms += bss[i]->ac_rspauth_tms;
				auth_success_num += bss[i]->auth_success; 
				auth_fail_num += bss[i]->auth_fail; 
				auth_invalid_num += bss[i]->info->auth_invalid; 
				auth_timeout_num += bss[i]->info->auth_timeout; 
				auth_refused_num += bss[i]->info->auth_refused; 
				auth_others_num += bss[i]->info->auth_others; 
				assoc_req_num += bss[i]->assoc_req; 
				assoc_resp_num += bss[i]->assoc_resp; 
				assoc_invalid_num += bss[i]->info->assoc_invalid; 
				assoc_timeout_num += bss[i]->info->assoc_timeout; 
				assoc_refused_num += bss[i]->info->assoc_refused; 
				assoc_others_num += bss[i]->info->assoc_others; 
				reassoc_request_num += bss[i]->num_reassoc; 
				reassoc_success_num += bss[i]->reassoc_success; 
				reassoc_invalid_num += bss[i]->info->reassoc_invalid; 
				reassoc_timeout_num += bss[i]->info->reassoc_timeout; 
				reassoc_refused_num += bss[i]->info->reassoc_refused; 
				reassoc_others_num += bss[i]->info->reassoc_others; 
				identify_request_num += bss[i]->info->identify_request; 
				identify_success_num += bss[i]->info->identify_success; 
				abort_key_error_num += bss[i]->info->abort_key_error; 
				abort_invalid_num += bss[i]->info->abort_invalid; 
				abort_timeout_num += bss[i]->info->abort_timeout; 
				abort_refused_num += bss[i]->info->abort_refused; 
				abort_others_num += bss[i]->info->abort_others; 
				deauth_request_num += bss[i]->info->deauth_request; 
				deauth_user_leave_num += bss[i]->info->deauth_user_leave; 
				deauth_ap_unable_num += bss[i]->info->deauth_ap_unable; 
				deauth_abnormal_num += bss[i]->info->deauth_abnormal; 
				deauth_others_num += bss[i]->info->deauth_others; 
				disassoc_request_num += bss[i]->info->disassoc_request; 
				disassoc_user_leave_num += bss[i]->info->disassoc_user_leave; 
				disassoc_ap_unable_num += bss[i]->info->disassoc_ap_unable; 
				disassoc_abnormal_num += bss[i]->info->disassoc_abnormal; 
				disassoc_others_num += bss[i]->info->disassoc_others; 
			/*
				rx_mgmt_pkts += bss[i]->info->rx_mgmt_pkts;
				tx_mgmt_pkts += bss[i]->info->tx_mgmt_pkts;
				rx_ctrl_pkts += bss[i]->info->rx_ctrl_pkts;
				tx_ctrl_pkts += bss[i]->info->tx_ctrl_pkts;
				rx_data_pkts += bss[i]->info->rx_data_pkts;
				tx_data_pkts += bss[i]->info->tx_data_pkts;
			*/	
				rx_auth_pkts += bss[i]->info->rx_auth_pkts;
				tx_auth_pkts += bss[i]->info->tx_auth_pkts;
			}

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(radio[k]));
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&acc_tms);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_tms);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&repauth_tms);

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_success_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_fail_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_invalid_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_timeout_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_refused_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&auth_others_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_req_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_req_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_invalid_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_timeout_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_refused_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_others_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&reassoc_request_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&reassoc_success_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&reassoc_invalid_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&reassoc_timeout_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&reassoc_refused_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&reassoc_others_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&identify_request_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&identify_success_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&abort_key_error_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&abort_invalid_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&abort_timeout_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&abort_refused_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&abort_others_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&deauth_request_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&deauth_user_leave_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&deauth_ap_unable_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&deauth_abnormal_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&deauth_others_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&disassoc_request_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&disassoc_user_leave_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&disassoc_ap_unable_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&disassoc_abnormal_num);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&disassoc_others_num);
		/*
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&rx_mgmt_pkts);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&tx_mgmt_pkts);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&rx_ctrl_pkts);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&tx_ctrl_pkts);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&rx_data_pkts);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&tx_data_pkts);
		*/
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&rx_auth_pkts);
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&tx_auth_pkts);
			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
		
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	return reply;

}
//fengwenchao add 20101221
DBusMessage *asd_dbus_show_info_allwlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusError err;	
	dbus_error_init(&err);

	int wlan_num = 0;
	int bss_num = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	int i=0;
	int j =0;
	WID_WLAN **WLAN;
	WLAN = malloc(WLAN_NUM*(sizeof(WID_WLAN *)));
	if( WLAN == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}
	memset(WLAN,0,WLAN_NUM*(sizeof(WID_WLAN *)));
	//struct asd_data *bss[BSS_NUM];
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		if(WLAN)
		{
			free(WLAN);
			WLAN = NULL;
		}
		return NULL;
	}

	/*unsigned int rx_pkts = 0;	
	unsigned int tx_pkts = 0;	
	unsigned long long rx_bytes = 0;
	unsigned long long tx_bytes = 0;
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 
	unsigned int assoc_fail_num = 0;
	unsigned int normal_st_down_num = 0;
	unsigned int abnormal_st_down_num = 0;	*/

	pthread_mutex_lock(&asd_g_wlan_mutex);
	while(i<WLAN_NUM)
	{
		if(ASD_WLAN[i] != NULL)
		{
			WLAN[wlan_num] = ASD_WLAN[i];
			wlan_num++;
		}
		i++;
	}
	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wlan_num);

	dbus_message_iter_open_container (&iter,
								   DBUS_TYPE_ARRAY,
								   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING		//wlan id

										/*DBUS_TYPE_UINT32_AS_STRING		//rx_pkts
										DBUS_TYPE_UINT32_AS_STRING		//tx_pkts
										DBUS_TYPE_UINT64_AS_STRING		//rx_bytes
										DBUS_TYPE_UINT64_AS_STRING		//tx_bytes*/
										DBUS_TYPE_UINT32_AS_STRING		// assoc_req_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_resp_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_fail_num
										DBUS_TYPE_UINT32_AS_STRING		//normal_st_down_num
										DBUS_TYPE_UINT32_AS_STRING		//abnormal_st_down_num	
										DBUS_TYPE_UINT32_AS_STRING      //sta_num	
										DBUS_TYPE_UINT32_AS_STRING		//sta assoced num
										DBUS_TYPE_UINT32_AS_STRING		//access sta num
								   DBUS_STRUCT_END_CHAR_AS_STRING,
								   &iter_array);
	pthread_mutex_lock(&asd_g_sta_mutex);

	for(i=0;i<wlan_num;i++)
	{
		unsigned int sta_num = 0;
		unsigned int sta_assoced = 0;
		unsigned int assoc_req_num = 0; 
		unsigned int assoc_resp_num = 0; 
		unsigned int assoc_fail_num = 0;
		unsigned int normal_st_down_num = 0;
		unsigned int abnormal_st_down_num = 0;
		unsigned int accessed_sta_num = 0;
		unsigned char wlanid = 0;	
		wlanid = WLAN[i]->WlanID;
		bss_num = ASD_SEARCH_WLAN_STA(wlanid, bss);

		dbus_message_iter_open_container (&iter_array,
									DBUS_TYPE_STRUCT,
									NULL,
									&iter_struct);
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&wlanid);

		

		for(j = 0; j<bss_num;j++)
		{
		
			GetBssDataPkts(bss[j]);

			/*rx_pkts += bss[j]->info->rx_data_pkts;	
			tx_pkts += bss[j]->info->tx_data_pkts;
			rx_bytes += bss[j]->info->rx_data_bytes;
			tx_bytes += bss[j]->info->tx_data_bytes;*/
			assoc_req_num += bss[j]->assoc_req; 
			assoc_resp_num += bss[j]->assoc_resp; 
			assoc_fail_num += bss[j]->num_assoc_failure;

			normal_st_down_num += bss[j]->normal_st_down_num;
			abnormal_st_down_num += bss[j]->abnormal_st_down_num;
			if(bss[j]->abnormal_st_down_num >= bss[j]->normal_st_down_num)
				abnormal_st_down_num -= bss[j]->normal_st_down_num;
			sta_num += bss[j]->num_sta;
			sta_assoced += bss[j]->sta_assoced;
		}
		accessed_sta_num = WLAN[i]->a_num_sta;

			/*dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&rx_pkts);	
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&tx_pkts);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT64,
											&rx_bytes);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT64,
											&tx_bytes);*/
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_req_num);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_resp_num);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&assoc_fail_num);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&normal_st_down_num);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&abnormal_st_down_num);
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&sta_num);				
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											&sta_assoced);				
			dbus_message_iter_append_basic (&iter_struct,
												DBUS_TYPE_UINT32,
												&accessed_sta_num); 
															

		dbus_message_iter_close_container(&iter_array,&iter_struct);
	}
	dbus_message_iter_close_container(&iter,&iter_array);

	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	if(WLAN){
		free(WLAN);
		WLAN = NULL;
	}
	if(bss){
		free(bss);
		bss = NULL;
	}
	return reply;
}
//fengwenchao add end

//ht add 090616
DBusMessage *asd_dbus_show_info_bywlanid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusError err;	

	unsigned char wlanid;
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 

	unsigned int accessed_sta_num = 0;
	unsigned int assoc_fail_num = 0;
	unsigned int sta_assoced = 0;
	unsigned int reassoc_num = 0; 
	unsigned int reassoc_success_num = 0; 

	unsigned int assoc_req_interim = 0;
	unsigned int assoc_resp_interim = 0;
	unsigned int assoc_success_interim = 0;

	//struct asd_data *bss[BSS_NUM];
	unsigned int num = 0;
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		
		free(bss);
		bss = NULL;
		return NULL;
	}
	pthread_mutex_lock(&asd_g_wlan_mutex);
	pthread_mutex_lock(&asd_g_sta_mutex);
	ret = ASD_CHECK_ID(ASD_WLAN_CHECK,(unsigned int)wlanid);
	if(ret != ASD_DBUS_SUCCESS){	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 				
	}
	else if(ASD_WLAN[wlanid] == NULL){	
		ret = ASD_WLAN_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 				
	}
	else{
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 		

		num = ASD_SEARCH_WLAN_STA(wlanid, bss);	
		
		for(i=0; i<num; i++) {
			if (bss[i]->info == NULL) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"bss[%d]->info==NULL\n",i);
				continue;
			}
			//GetBssDataPkts(bss[i]);
			
			/*rx_pkts += bss[i]->info->rx_data_pkts;	
			tx_pkts += bss[i]->info->tx_data_pkts;
			rx_bytes += bss[i]->info->rx_data_bytes;
			tx_bytes += bss[i]->info->tx_data_bytes;*/
			assoc_req_num += bss[i]->assoc_req; 
			assoc_resp_num += bss[i]->assoc_resp; 

			assoc_fail_num += bss[i]->num_assoc_failure;
			sta_assoced += bss[i]->sta_assoced;
			reassoc_num += bss[i]->num_reassoc; 
			reassoc_success_num += bss[i]->reassoc_success; 
			
		  	assoc_req_interim += bss[i]->assoc_req - bss[i]->assoc_req_timer_update;
		  	assoc_resp_interim += bss[i]->assoc_resp - bss[i]->assoc_resp_timer_update;
		  	assoc_success_interim += bss[i]->assoc_success + bss[i]->reassoc_success - bss[i]->assoc_success_all_timer_update;
		}
		accessed_sta_num = ASD_WLAN[wlanid]->a_num_sta;
		/*dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &rx_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &tx_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &rx_bytes);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &tx_bytes);*/
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_req_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_resp_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &accessed_sta_num);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_fail_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &sta_assoced);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_success_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_req_interim);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_resp_interim);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_success_interim);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wlan_mutex);

	if(bss){
		free(bss);
		bss = NULL;
	}
	return reply;

}


//ht add 090323
DBusMessage *asd_dbus_show_info_bywtpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int WTPID,acc_tms=0,auth_tms=0,repauth_tms=0;
	DBusError err;		

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
	//unsigned int rx_data_pkts = 0;
	//unsigned int tx_data_pkts = 0;
	unsigned int rx_auth_pkts = 0;
	unsigned int tx_auth_pkts = 0;

	unsigned long long wtp_total_past_online_time = 0;	//	xm0703
	unsigned int num_assoc_failure = 0;	//	xm0703
	unsigned int num_accessed_sta = 0;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned int num = 0;
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WTPID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	pthread_mutex_lock(&asd_g_sta_mutex);
	if(WTPID >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else if(!AsdCheckWTPID(WTPID))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 				
	}else{
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 		
		acc_tms = GetAccessTimes(WTPID);
		auth_tms = GetUserAuthTimes(WTPID);
		repauth_tms = GetWTPRspAuthTimes(WTPID);
		num = ASD_SEARCH_WTP_STA(WTPID, bss);	
		
		for(i=0; i<num; i++) {
			
			wtp_total_past_online_time+=bss[i]->total_past_online_time;
			num_assoc_failure+=bss[i]->num_assoc_failure;
			 	
			if (bss[i]->info == NULL) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"bss[%d]==NULL\n",i);
				continue;
			}
			GetBssDataPkts(bss[i]);
			if(ASD_WLAN[bss[i]->WlanID] != NULL){
				num_accessed_sta += ASD_WLAN[bss[i]->WlanID]->a_num_sta;	
			}			
			auth_success_num += bss[i]->auth_success; 
			auth_fail_num += bss[i]->auth_fail; 
			auth_invalid_num += bss[i]->info->auth_invalid; 
			auth_timeout_num += bss[i]->info->auth_timeout; 
			auth_refused_num += bss[i]->info->auth_refused; 
			auth_others_num += bss[i]->info->auth_others; 
			assoc_req_num += bss[i]->assoc_req; 
			assoc_resp_num += bss[i]->assoc_resp; 
			assoc_invalid_num += bss[i]->info->assoc_invalid; 
			assoc_timeout_num += bss[i]->info->assoc_timeout; 
			assoc_refused_num += bss[i]->info->assoc_refused; 
			assoc_others_num += bss[i]->info->assoc_others; 
			reassoc_request_num += bss[i]->num_reassoc; 
			reassoc_success_num += bss[i]->reassoc_success; 
			reassoc_invalid_num += bss[i]->info->reassoc_invalid; 
			reassoc_timeout_num += bss[i]->info->reassoc_timeout; 
			reassoc_refused_num += bss[i]->info->reassoc_refused; 
			reassoc_others_num += bss[i]->info->reassoc_others; 
			identify_request_num += bss[i]->info->identify_request; 
			identify_success_num += bss[i]->info->identify_success; 
			abort_key_error_num += bss[i]->info->abort_key_error; 
			abort_invalid_num += bss[i]->info->abort_invalid; 
			abort_timeout_num += bss[i]->info->abort_timeout; 
			abort_refused_num += bss[i]->info->abort_refused; 
			abort_others_num += bss[i]->info->abort_others; 
			deauth_request_num += bss[i]->info->deauth_request; 
			deauth_user_leave_num += bss[i]->info->deauth_user_leave; 
			deauth_ap_unable_num += bss[i]->info->deauth_ap_unable; 
			deauth_abnormal_num += bss[i]->info->deauth_abnormal; 
			deauth_others_num += bss[i]->info->deauth_others; 
			disassoc_request_num += bss[i]->info->disassoc_request; 
			disassoc_user_leave_num += bss[i]->info->disassoc_user_leave; 
			disassoc_ap_unable_num += bss[i]->info->disassoc_ap_unable; 
			disassoc_abnormal_num += bss[i]->info->disassoc_abnormal; 
			disassoc_others_num += bss[i]->info->disassoc_others; 

			
			rx_mgmt_pkts += bss[i]->info->rx_mgmt_pkts;
			tx_mgmt_pkts += bss[i]->info->tx_mgmt_pkts;
			rx_ctrl_pkts += bss[i]->info->rx_ctrl_pkts;
			tx_ctrl_pkts += bss[i]->info->tx_ctrl_pkts;
			//rx_data_pkts += bss[i]->info->rx_data_pkts;
			//tx_data_pkts += bss[i]->info->tx_data_pkts;
			rx_auth_pkts += bss[i]->info->rx_auth_pkts;
			tx_auth_pkts += bss[i]->info->tx_auth_pkts;
		}

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &wtp_total_past_online_time);	//	xm0703

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num_assoc_failure);	//	xm0703
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num_accessed_sta);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &acc_tms);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_tms);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &repauth_tms);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_success_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_fail_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_invalid_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_timeout_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_refused_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_others_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_req_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_resp_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_invalid_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_timeout_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_refused_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_others_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_request_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_success_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_invalid_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_timeout_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_refused_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_others_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &identify_request_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &identify_success_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &abort_key_error_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &abort_invalid_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &abort_timeout_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &abort_refused_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &abort_others_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &deauth_request_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &deauth_user_leave_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &deauth_ap_unable_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &deauth_abnormal_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &deauth_others_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &disassoc_request_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &disassoc_user_leave_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &disassoc_ap_unable_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &disassoc_abnormal_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &disassoc_others_num);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &rx_mgmt_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &tx_mgmt_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &rx_ctrl_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &tx_ctrl_pkts);
	/*	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &rx_data_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &tx_data_pkts);*/
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &rx_auth_pkts);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &tx_auth_pkts);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
	  										   	DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												//DBUS_TYPE_UINT32_AS_STRING
												//DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
												DBUS_TYPE_UINT64_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i = 0; i < num ; i++){			
			DBusMessageIter iter_struct;

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_BYTE,
											&(bss[i]->WlanID));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->BSSIndex));
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->acc_tms));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->usr_auth_tms));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->ac_rspauth_tms));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->auth_success));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->auth_fail));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->auth_invalid));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->auth_timeout));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->auth_refused));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->auth_others));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->assoc_req));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->assoc_resp));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->assoc_invalid));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->assoc_timeout));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->assoc_refused));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->assoc_others));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->num_reassoc));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->reassoc_success));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->reassoc_invalid));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->reassoc_timeout));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->reassoc_refused));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->reassoc_others));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->identify_request));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->identify_success));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->abort_key_error));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->abort_invalid));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->abort_timeout));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->abort_refused));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->abort_others));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->deauth_request));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->deauth_user_leave));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->deauth_ap_unable));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->deauth_abnormal));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->deauth_others));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->disassoc_request));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->disassoc_user_leave));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->disassoc_ap_unable));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->disassoc_abnormal));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->disassoc_others));

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->rx_mgmt_pkts));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->tx_mgmt_pkts));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->rx_ctrl_pkts));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->tx_ctrl_pkts));
			/*dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->rx_data_pkts));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->tx_data_pkts));*/
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->rx_auth_pkts));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(bss[i]->info->tx_auth_pkts));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(bss[i]->total_past_online_time));
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT64,
											&(bss[i]->total_present_online_time));


			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);

		//mahz add 2011.5.3
		if(ASD_WTP_AP_HISTORY[WTPID] != NULL){
			asd_printf(ASD_DBUS,MSG_DEBUG,"11111111\n");		//for test
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->usr_auth_tms);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->ac_rspauth_tms);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->auth_fail);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->auth_success);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->num_assoc);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->num_reassoc);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->num_assoc_failure);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->num_reassoc_failure);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->assoc_success);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->reassoc_success);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->assoc_req);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTPID]->assoc_resp);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT64, &ASD_WTP_AP_HISTORY[WTPID]->total_ap_flow_record);
		}
		else{
			unsigned int val = 0; 
			unsigned long long value = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"22222222\n");		//for test
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &val);
			dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT64, &value);
		}//
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	return reply;

}

DBusMessage *asd_dbus_extend_show_sta(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;		
	DBusMessageIter  iter;
	unsigned char *mac;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	struct asd_stainfo *stainfo;
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		free(mac);
		mac = NULL; // 0608xm
		return NULL;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);

	stainfo = ASD_SEARCH_STA(mac);
		
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);

	if(stainfo != NULL&&stainfo->sta!=NULL){

		time_t sta_time;
		sta_time=stainfo->sta->add_time_sysruntime;//qiuchen add it

		//UpdateStaInfoToWSM(NULL,NULL,STA_INFO);	//ht add,081027	
			
		Compute_Sta_Rate(stainfo->sta);

		Get_Sta_SNR(stainfo->sta);
		
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										 &ret);	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										  &(stainfo->sta->in_addr));
		
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										 &(stainfo->sta->snr));				
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT64,
										 &(stainfo->sta->rr));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->tr));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->tp));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->rxbytes));	//ht add 090306
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->txbytes));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->rxpackets));	//ht add 090220
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->txpackets));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->retrybytes));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->retrypackets));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT64,
										 &(stainfo->sta->errpackets));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(sta_time));//ht 090309
		//qiuchen add it 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(stainfo->sta->sta_online_time));
		//end
		//qiuchen add it for AXSSZFI-1373
		time_t sta_access_time,online_time,nowsys;
		get_sysruntime(&nowsys);
		online_time = nowsys - stainfo->sta->add_time_sysruntime;
		sta_access_time = time(NULL) - online_time;
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(sta_access_time));

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(online_time));
		//end
		
	}else{

		ret = ASD_STA_NOT_EXIST;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 		
	}
	
	free(mac);
	mac = NULL;
	if(stainfo != NULL){
		stainfo->bss = NULL;
		stainfo->sta = NULL;
		free(stainfo);
		stainfo = NULL;
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;	

	
}

DBusMessage *asd_dbus_show_channel_access_time(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned char num=13;
	unsigned char channel = 0;
	time_t sendt,now;
	time_t sendsys,now_sysrun;//qiuchen add it
	
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	unsigned char i=0;

	pthread_mutex_lock(&asd_g_sta_mutex);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &num);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING//sendsys
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;
		channel = i+1;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(channel));
			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(channel_time[channel].sta_num));

		if(channel_time[channel].sta_num!=0){
			time(&now);
			sendt = now-channel_time[channel].begin_time;
			//qiuchen add it
			get_sysruntime(&now_sysrun);
			sendsys = now_sysrun - channel_time[channel].begin_time_sysrun;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"begin_time sendt is %u, sendsys is %u func is %s line is %d.\n",(int)sendt,(int)sendsys,__func__,__LINE__);
			//end
		}else{
			sendt = channel_time[channel].end_time;
			sendsys = channel_time[channel].end_time_sysrun;//qiuchen add it
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"end_time sendt is %d, sendsys is %u func is %s line is %d.\n",(int)sendt,(int)sendsys,__func__,__LINE__);
		}
		
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					 &(sendt));
		//qiuchen add it
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					 &(sendsys));
		//end

		dbus_message_iter_close_container (&iter_array, &iter_struct);


	}
				
	dbus_message_iter_close_container (&iter, &iter_array);
				
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;	

}



DBusMessage *asd_dbus_show_sta_by_mac(DBusConnection *conn, DBusMessage *msg, void *user_data){

	
		DBusMessage* reply;		
		DBusMessageIter  iter;
		unsigned char *mac;
		unsigned char SecurityID = 0;
		unsigned int  SecurityType;		//mahz add 2011.3.1
		DBusError err;
		int ret = ASD_DBUS_SUCCESS;
		struct asd_stainfo *stainfo;
		mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
		if(mac==NULL)
			return NULL;	//	0608xm	
		memset(mac, 0, WID_MAC_LEN);
		int pae_state, backend_state;
		dbus_error_init(&err);
		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_BYTE,&mac[0],
									DBUS_TYPE_BYTE,&mac[1],
									DBUS_TYPE_BYTE,&mac[2],
									DBUS_TYPE_BYTE,&mac[3],
									DBUS_TYPE_BYTE,&mac[4],
									DBUS_TYPE_BYTE,&mac[5],
									DBUS_TYPE_INVALID))){
	
			asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
			if (dbus_error_is_set(&err)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			free(mac);
			mac = NULL; // 0608xm
			return NULL;
		}
		pthread_mutex_lock(&asd_g_sta_mutex);	

		stainfo = ASD_SEARCH_STA(mac);
		
		reply = dbus_message_new_method_return(msg);
			
		dbus_message_iter_init_append (reply, &iter);

		if((stainfo != NULL)&&(stainfo->sta != NULL)){
			if(stainfo->sta->eapol_sm != NULL){
				pae_state = stainfo->sta->eapol_sm->auth_pae_state;
				backend_state = stainfo->sta->eapol_sm->be_auth_state;	
			}else{
				pae_state = 255;
				backend_state = 255;
			}	
			if(ASD_WLAN[stainfo->bss->WlanID])
				SecurityID = ASD_WLAN[stainfo->bss->WlanID]->SecurityID;

			//mahz add 2011.3.1
			if(((ASD_SECURITY[SecurityID])&&(ASD_SECURITY[SecurityID]->extensible_auth == 1)&&(ASD_SECURITY[SecurityID]->hybrid_auth == 1))&&(stainfo->sta->security_type == HYBRID_AUTH_EAPOL))
				SecurityType = HYBRID_AUTH_EAPOL;
			else if(((ASD_SECURITY[SecurityID])&&(ASD_SECURITY[SecurityID]->extensible_auth == 1)&&(ASD_SECURITY[SecurityID]->hybrid_auth == 1))&&(stainfo->sta->security_type == HYBRID_AUTH_PORTAL)){
				SecurityType = HYBRID_AUTH_PORTAL;
			}			 
			else if((ASD_SECURITY[SecurityID])&&(ASD_SECURITY[SecurityID]->extensible_auth == 0)&&(ASD_SECURITY[SecurityID]->hybrid_auth == 1))
			{
				if(ASD_SECURITY[SecurityID]->mac_auth == 1)
					SecurityType = MAC_AUTH;
				else
					SecurityType = NO_NEED_AUTH;
			}
			else if((ASD_SECURITY[SecurityID])&&(ASD_SECURITY[SecurityID]->extensible_auth == 1)&&(ASD_SECURITY[SecurityID]->hybrid_auth == 0))
				SecurityType = EXTENSIBLE_AUTH;
			else if(ASD_SECURITY[SecurityID])
				SecurityType = ASD_SECURITY[SecurityID]->securityType;	
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"success = %d\n",stainfo->sta->portal_auth_success );
			if(stainfo->sta->portal_auth_success == 1){
				SecurityType = PORTAL_AUTH;
			}

//=====================================
            time_t sta_time;
			sta_time=stainfo->sta->add_time_sysruntime;//qiuchen change it 2012.10.31
			wapi_stats_Entry	*wapi_sta_mib = NULL;
			wapi_sta_mib = &stainfo->sta->wapi_sta_info.wapi_mib_stats;

//=====================================
				
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &ret);			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->bss->WlanID));
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->bss->Radio_G_ID));
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->bss->BSSIndex));	
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(SecurityID));	

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->sta->vlan_id));//5
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->sta->flags));
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &pae_state);
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &backend_state);
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(sta_time));////////////////////???????
			//qiuchen add it
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->sta->sta_online_time));
			
			//end
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->sta->sta_traffic_limit));//10
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->sta->sta_send_traffic_limit));//11

#if 1			 
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_STRING,
												 &(stainfo->sta->in_addr));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->sta->snr));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->rr));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->tr));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->tp));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->rxbytes));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->txbytes));
					
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->rxpackets));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->txpackets));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->retrybytes));
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->retrypackets));
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT64,
												 &(stainfo->sta->errpackets));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->sta->mode));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->sta->channel));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->sta->rssi));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT16,
												 &(stainfo->sta->nRate));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->sta->isPowerSave));
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->sta->isQos));

			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->sta->info_channel));
			//mahz add 2011.3.1			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(SecurityType));
#endif

		/*wapi mib,ht add 10.3.9*/
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wapi_sta_mib->wapi_version));
										 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &(wapi_sta_mib->controlled_port_status));
											 
		dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_BYTE,
										  &(wapi_sta_mib->selected_unicast_cipher[0]));
		dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_BYTE,
										  &(wapi_sta_mib->selected_unicast_cipher[1]));
		dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_BYTE,
										  &(wapi_sta_mib->selected_unicast_cipher[2]));
		dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_BYTE,
										  &(wapi_sta_mib->selected_unicast_cipher[3]));

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wapi_sta_mib->wpi_replay_counters));
										 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wapi_sta_mib->wpi_decryptable_errors));
											 
		 dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_UINT32,
										  &(wapi_sta_mib->wpi_mic_errors));

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wapi_sta_mib->wai_sign_errors));
										 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(wapi_sta_mib->wai_hmac_errors));
											 
		 dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_UINT32,
										  &(wapi_sta_mib->wai_auth_res_fail));
		 dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_UINT32,
										  &(wapi_sta_mib->wai_discard));
										  
		 dbus_message_iter_append_basic (&iter,
										  DBUS_TYPE_UINT32,
										  &(wapi_sta_mib->wai_timeout));
											  
		  dbus_message_iter_append_basic (&iter,
										   DBUS_TYPE_UINT32,
										   &(wapi_sta_mib->wai_format_errors));
		  dbus_message_iter_append_basic (&iter,
										   DBUS_TYPE_UINT32,
										   &(wapi_sta_mib->wai_cert_handshake_fail));
										   
		  dbus_message_iter_append_basic (&iter,
										   DBUS_TYPE_UINT32,
										   &(wapi_sta_mib->wai_unicast_handshake_fail));
											   
		   dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&(wapi_sta_mib->wai_multi_handshake_fail));

		   
		   //qiuchen add it for AXSSZFI-1373
		   time_t sta_access_time,online_time,nowsys;
		   get_sysruntime(&nowsys);
		   online_time = nowsys - stainfo->sta->add_time_sysruntime;
		   sta_access_time = time(NULL) - online_time;
		   
		   dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&(sta_access_time));
		   
		   dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&(online_time));
		   //end

		}else{
			ret = ASD_STA_NOT_EXIST;
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 		

		}
		free(mac);
		mac = NULL;
		if(stainfo != NULL){
			stainfo->bss = NULL;
			stainfo->sta = NULL;
			free(stainfo);
			stainfo = NULL;
		}
		pthread_mutex_unlock(&asd_g_sta_mutex);	
		return reply;	

}

DBusMessage *asd_dbus_show_collect_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	//struct hapd_interfaces *interfaces = (struct hapd_interfaces*) eloop.user_data;
	unsigned int assoc_req_num = 0; 
	unsigned int reassoc_request_num = 0; 
	unsigned int num_assoc_failure = 0;
	unsigned int num_reassoc_failure = 0;
	unsigned int assoc_success = 0;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM] = {NULL};
	unsigned int bss_num = 0;
	unsigned int wtp_num = 0;
	int i = 0, k =0;
	int ret = ASD_DBUS_SUCCESS;
	
	ASD_WTP_ST **WTP = NULL;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM*(sizeof(ASD_WTP_ST *)));
	 
	pthread_mutex_lock(&asd_g_wtp_mutex); 
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	//if(wtp_num != 0)
	{
		dbus_message_iter_open_container (&iter,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//num_assoc_failure
															DBUS_TYPE_UINT32_AS_STRING		//assoc_success
													   DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_array);
		for(k=0;k<wtp_num;k++){
			assoc_req_num = 0;
			reassoc_request_num = 0;
			num_assoc_failure = 0;
			num_reassoc_failure = 0;		//mahz add 2011.3.31
			assoc_success = 0;
			bss_num = ASD_SEARCH_WTP_STA(WTP[k]->WTPID, bss);	
			
			for(i=0; i<bss_num; i++){
				GetBssDataPkts(bss[i]);
				
				if (bss[i]->info == NULL) {
					continue;
				}
				
				assoc_req_num += bss[i]->assoc_req; 
				reassoc_request_num += bss[i]->num_reassoc; 
				num_assoc_failure += bss[i]->num_assoc_failure;
				num_reassoc_failure +=  bss[i]->num_reassoc_failure;
				assoc_success += bss[i]->assoc_success+bss[i]->reassoc_success;		//xiaodawei modify, SuccAssociatedNum=assoc_success+reassoc_success
			}
			
			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											 &(WTP[k]->WTPID));
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_req_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_request_num);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &num_assoc_failure);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &num_reassoc_failure);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_success);
			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 
	
	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	
	return reply;
}


	
DBusMessage *asd_dbus_set_extern_balance(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	unsigned char type=0;	//1--enable, 0--disable
	unsigned char wlanid=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(ASD_WLAN[wlanid] == NULL)
		ret = ASD_WLAN_NOT_EXIST;
	else{
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s--->set wlan %u extern balance %u\n",__func__,wlanid,type);
		ASD_WLAN[wlanid]->extern_balance=0;
		if(type==1){
			ASD_WLAN[wlanid]->extern_balance=1;
		}
	}
	
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}
/*nl add for showing stats infor for mib 20100508*/
DBusMessage *asd_dbus_show_stats_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	unsigned int assoc_num = 0;
	unsigned int reassoc_num = 0;
	unsigned int assoc_success_num = 0;
	unsigned int reassoc_success_num = 0;
	unsigned int assoc_failure_num = 0;
	unsigned int reassoc_failure_num = 0;
	unsigned int abnormal_down_num = 0;
	unsigned int stanum = 0;

	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned int bss_num = 0;
	unsigned int wtp_num = 0;
	int i = 0,k =0;
	int ii=0, jj=0;
	int ret = ASD_DBUS_SUCCESS;
	
	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	//if(wtp_num != 0)
	{
		dbus_message_iter_open_container (&iter,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															
															DBUS_TYPE_UINT32_AS_STRING		//assoc_num
															DBUS_TYPE_UINT32_AS_STRING		//reassoc_num
															DBUS_TYPE_UINT32_AS_STRING		//assoc_success_num
															DBUS_TYPE_UINT32_AS_STRING		//reassoc_success_num
															DBUS_TYPE_UINT32_AS_STRING		//assoc_failure_num
															DBUS_TYPE_UINT32_AS_STRING		//reassoc_failure_num
															DBUS_TYPE_UINT32_AS_STRING		//abnormal_down_num
 															DBUS_TYPE_UINT32_AS_STRING		//sta_num

															DBUS_TYPE_UINT32_AS_STRING		//total sta online time

															DBUS_TYPE_UINT32_AS_STRING 		//deny_num
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															
															DBUS_TYPE_UINT32_AS_STRING		//auth_succ_tms
															DBUS_TYPE_UINT32_AS_STRING
													   DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_array);
		for(k=0;k<wtp_num;k++){
			assoc_num = 0;
			reassoc_num = 0;
			assoc_success_num = 0;
			reassoc_success_num = 0;
			assoc_failure_num = 0;
			reassoc_failure_num = 0;
			abnormal_down_num = 0;
			stanum = 0;
			unsigned int acc_tms=0;
			unsigned int auth_tms=0;
			unsigned int repauth_tms=0;
			unsigned int auth_succ_tms=0;
			unsigned int auth_fail_tms=0;
			unsigned int deny_num = 0;	
			time_t now,online_time;
			time_t sta_time =0;
			//qiuchen add it
			get_sysruntime(&now);
			//time(&now);
			time_t total_online_time = 0;
			
			bss_num = ASD_SEARCH_WTP_STA(WTP[k]->WTPID, bss);	
			
			for(i=0; i<bss_num; i++){
				GetBssDataPkts(bss[i]);
				
				if (bss[i]->info == NULL) {
					continue;
				}
				
 				assoc_num += bss[i]->num_assoc;
				reassoc_num += bss[i]->num_reassoc;
				assoc_success_num += bss[i]->assoc_success;
				reassoc_success_num += bss[i]->reassoc_success;
				assoc_failure_num += bss[i]->num_assoc_failure;
				reassoc_failure_num += bss[i]->num_reassoc_failure;
				abnormal_down_num += bss[i]->abnormal_st_down_num;
				if(bss[i]->abnormal_st_down_num >= bss[i]->normal_st_down_num)
					abnormal_down_num -= bss[i]->normal_st_down_num;
				stanum += bss[i]->num_sta;
				//deny_num += bss[i]->th_deny_num;		//mahz modify 2011.4.7
				deny_num += bss[i]->assoc_reject_no_resource;	//mahz add 2011.4.7
			}

			acc_tms=GetAccessTimes(WTP[k]->WTPID);
			auth_tms=GetUserAuthTimes(WTP[k]->WTPID);
			repauth_tms=GetWTPRspAuthTimes(WTP[k]->WTPID);
			auth_succ_tms=GetUserAuthSuccessTimes(WTP[k]->WTPID);
			auth_fail_tms=GetUserAuthFailTimes(WTP[k]->WTPID);

			for(ii=0;ii<bss_num;ii++){
				struct sta_info *sta = NULL;
				sta = bss[ii]->sta_list;
				
 				for(jj = 0; (jj < bss[ii]->num_sta)&&(sta!=NULL); jj++){
					sta_time=sta->add_time_sysruntime;//qiuchen change it
					online_time=now-sta_time+sta->sta_online_time;
					
					total_online_time += online_time;
					sta = sta->next;
				}
			}
				
			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											 &(WTP[k]->WTPID));
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_success_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_success_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_failure_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_failure_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &abnormal_down_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &stanum); 

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &total_online_time); 

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deny_num); 
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &acc_tms);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_tms);
					
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &repauth_tms);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_succ_tms);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_fail_tms);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	
	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	
	return reply;
}

/*liuzhenhua add 2010-05-21*/
DBusMessage * asd_dbus_show_ssid_config_information_of_all_wlan(DBusConnection *conn, DBusMessage 

*msg, void *user_data){

	
		DBusMessage* reply; 
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		DBusMessageIter iter_security;
		
		DBusError err;
		dbus_error_init(&err);
		int ret = ASD_DBUS_SUCCESS;
		
		unsigned int security_num = 0;
		int i=0;
		
	
		security_profile *SECURITY[WLAN_NUM]={0};
		for(i=0;i<WLAN_NUM;i++){
			if(ASD_SECURITY[i]!=NULL){
				SECURITY[security_num]=ASD_SECURITY[i];
				security_num++;
				}
			}
		if(security_num==0)
			ret=ASD_SECURITY_NOT_EXIST;
		reply=dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append(reply,&iter);

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&ret);
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&security_num);
		dbus_message_iter_open_container(&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING	//NewEncrInputType;---------1
												DBUS_TYPE_STRING_AS_STRING	//*NewSecurityKEY;
												DBUS_TYPE_UINT32_AS_STRING	//NewExtensibleAuth;
												DBUS_TYPE_STRING_AS_STRING	//NewAuthIP;
												DBUS_TYPE_UINT32_AS_STRING	//NewAuthPort;
												DBUS_TYPE_STRING_AS_STRING	//*NewAuthSharedSecret;------5
												DBUS_TYPE_STRING_AS_STRING	//NewAcctIP;
												DBUS_TYPE_UINT32_AS_STRING	//NewAcctPort;
												DBUS_TYPE_STRING_AS_STRING	//*NewAcctSharedSecret;
												DBUS_TYPE_UINT32_AS_STRING	//security_id
												DBUS_TYPE_STRING_AS_STRING	//ASIP;
												DBUS_TYPE_STRING_AS_STRING	//ASPATH;
												DBUS_TYPE_STRING_AS_STRING	//AEPATH;
												DBUS_TYPE_UINT32_AS_STRING	//CERT_TYPE
												DBUS_TYPE_UINT32_AS_STRING	//STA_AGED_TIME
												//DBUS_TYPE_UINT32_AS_STRING	//SSIDRowStatus;
											DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);
		unsigned int security_id;
		long NewEncrInputType;		//密钥输入类型 3
		char *NewSecurityKEY;
		long NewExtensibleAuth;	
		long NewAuthPort;
		char *NewAuthSharedSecret;
		char *NewAuthIP;	//认证服务器
		char *NewAcctIP;	//计费服务器
		long NewAcctPort;
		char *NewAcctSharedSecret; 
		char *asip;
		long cert_type;
		char *as_path;
		char *ae_path;
		long sta_aged_time;
		
		for(i=0;i<security_num;i++){
			switch(SECURITY[i]->keyInputType){
				case ASCII:
					NewEncrInputType=1; break;
				case HEX:
					NewEncrInputType=2;;break;
				default:
					NewEncrInputType=0;; break;
				}
			if(SECURITY[i]->SecurityKey){
				NewSecurityKEY=SECURITY[i]->SecurityKey;
				}
			else{
				NewSecurityKEY="none";
				}
			
			NewExtensibleAuth=SECURITY[i]->extensible_auth;
			
			if(SECURITY[i]->auth.auth_ip){
				NewAuthIP=SECURITY[i]->auth.auth_ip;
				}
			else{
				NewAuthIP="0";
				}
			NewAuthPort=SECURITY[i]->auth.auth_port;
			if(SECURITY[i]->auth.auth_shared_secret){
				NewAuthSharedSecret=SECURITY[i]->auth.auth_shared_secret;
				}
			else{
				NewAuthSharedSecret="none";
				}
			
			if(SECURITY[i]->acct.acct_ip){
				NewAcctIP=SECURITY[i]->acct.acct_ip;
				}
			else{
				NewAcctIP="0";
				}

			NewAcctPort=SECURITY[i]->acct.acct_port;
			if(SECURITY[i]->acct.acct_shared_secret){
				NewAcctSharedSecret=SECURITY[i]->acct.acct_shared_secret;
				}
			else{
				NewAcctSharedSecret="none";
				}
			security_id = SECURITY[i]->SecurityID;

			if(SECURITY[i]->wapi_as.as_ip){
				asip = SECURITY[i]->wapi_as.as_ip;
			}else{
				asip = "0";
			}
			if(SECURITY[i]->wapi_as.certification_path){
				as_path = SECURITY[i]->wapi_as.certification_path;
			}else{
				as_path = "0";
			}
			if(SECURITY[i]->wapi_as.ae_cert_path){
				ae_path = SECURITY[i]->wapi_as.ae_cert_path;
			}else{
				ae_path = "0";
			}
			cert_type = SECURITY[i]->wapi_as.certification_type;
			sta_aged_time = SECURITY[i]->ap_max_inactivity;
			dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_security);
			
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&NewEncrInputType);/*1*/
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&NewSecurityKEY);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&NewExtensibleAuth);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&NewAuthIP);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&NewAuthPort);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&NewAuthSharedSecret);/*5*/
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&NewAcctIP);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&NewAcctPort);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&NewAcctSharedSecret);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&security_id);
			
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&asip);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&as_path);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_STRING,&ae_path);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&cert_type);
			dbus_message_iter_append_basic(&iter_security,DBUS_TYPE_UINT32,&sta_aged_time);
			//printf("NewSecurityKEY = %s\n",NewSecurityKEY);
			//printf("NewAuthSharedSecret = %s\n",NewAuthSharedSecret);
			//printf("NewAcctSharedSecret = %s\n",NewAcctSharedSecret);
			dbus_message_iter_close_container(&iter_array,&iter_security);

			}
		dbus_message_iter_close_container(&iter,&iter_array);

		return reply;
}

/*liuzhenhua append 2010-05-28*/
/*mib table 25*/
DBusMessage *asd_dbus_show_sta_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage	* reply; 
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_sta_array;
	DBusMessageIter iter_sta;
	
	DBusError err;		
	dbus_error_init(&err);

	
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	struct sta_info * sta=NULL;
	int ret=ASD_DBUS_SUCCESS;
	int i,j;
	unsigned int wtp_num=0;
	unsigned int bss_num=0;

	char  wtp_mac[18]={0};
	char * wtp_mac_p=wtp_mac;
	char * wtp_name = NULL;//zhaoruijia,20100917,add
	int   wtp_name_len = 0;
	char  sta_mac[18]={0};
	char * sta_mac_p=sta_mac;
	unsigned char *identity = NULL;	/* sta name *///qiuchen
	char tmp[16] = {0};	/* keep it NULL */
	
	ASD_WTP_ST **WTP=malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	//if(!WTP)
	//	ret = -1;
	
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	for(i=0;i<WTP_NUM;i++){
		if(ASD_WTP_AP[i]){
			WTP[wtp_num++]=ASD_WTP_AP[i];
			}
		}

	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING	//wtp_mac
									DBUS_TYPE_UINT32_AS_STRING	//sta_num
									DBUS_TYPE_ARRAY_AS_STRING
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_STRING_AS_STRING	//sta_mac_p
										DBUS_TYPE_UINT32_AS_STRING	//sta_ip
										DBUS_TYPE_UINT32_AS_STRING	//sta->snr
										DBUS_TYPE_UINT64_AS_STRING	//rx_pkts
										DBUS_TYPE_UINT64_AS_STRING	//rx_data_pkts, xiaodawei add
										DBUS_TYPE_UINT64_AS_STRING	//rx_bytes //xiaodawei modify, change 32bits to 64bits, 20110104
										DBUS_TYPE_UINT64_AS_STRING	//tx_pkts
										DBUS_TYPE_UINT64_AS_STRING	//tx_data_pkts, xiaodawei add
										DBUS_TYPE_UINT64_AS_STRING	//tx_bytes //xiaodawei modify, change 32bits to 64bits, 20110104
										DBUS_TYPE_UINT32_AS_STRING	//rt_pkts
										DBUS_TYPE_UINT32_AS_STRING	//rt_bytes
										DBUS_TYPE_UINT32_AS_STRING	//err_pkts
										DBUS_TYPE_UINT64_AS_STRING	//rx_frag_packets //xiaodawei add
										DBUS_TYPE_UINT64_AS_STRING	//tx_frag_packets //xiaodawei add
										DBUS_TYPE_UINT32_AS_STRING	//statime
										DBUS_TYPE_UINT64_AS_STRING	//tr
										DBUS_TYPE_UINT64_AS_STRING	//rr
										DBUS_TYPE_UINT64_AS_STRING	//tp
										DBUS_TYPE_STRING_AS_STRING	//snrz_p
										DBUS_TYPE_UINT32_AS_STRING	//WTPID
										DBUS_TYPE_STRING_AS_STRING	//stactime_p
										DBUS_TYPE_UINT32_AS_STRING	//add time
										DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
										DBUS_TYPE_STRING_AS_STRING  //wtpname
										DBUS_TYPE_UINT32_AS_STRING	//sta access security type
										DBUS_TYPE_STRING_AS_STRING	//sta name										
										DBUS_TYPE_UINT32_AS_STRING	//sta_access_time  qiuchen add it
										DBUS_TYPE_UINT32_AS_STRING	//online_time  qiuchen add it
										DBUS_TYPE_UINT32_AS_STRING	//MAXofRateset
											DBUS_TYPE_UINT32_AS_STRING /*rate 1 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 1 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 2 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 2 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 3 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 3 APStaTxDataRatePkts */	
											DBUS_TYPE_UINT32_AS_STRING /*rate 4 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 4 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 5 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 5 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 6 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 6 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 7 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 7 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 8 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 8 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 9 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 9 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 10 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 10 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 11 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 11 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 12 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 12 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 13 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 13 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 14 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 14 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 15 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 15 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 16 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 16 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 17 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 17 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 18 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 18 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 19 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 19 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 20 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 20 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 21 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 21 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 22 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 22 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 23 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 23 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 24 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 24 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 25 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 25 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 26 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 26 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 27 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 27 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 28 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 28 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 29 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 29 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 30 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 30 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 31 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 31 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 32 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 32 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 33 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 33 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 34 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 34 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 35 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 35 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 36 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 36 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 37 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 37 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 38 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 38 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 39 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 39 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 40 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 40 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 41 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 41 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 42 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 42 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 43 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 43 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 44 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 44 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 1 */
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 2 */	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 3*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 4*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 5*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 6*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 7*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 8*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 9*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 10*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 11*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 12*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 13*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 14*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 15*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 16*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 17*/
									DBUS_STRUCT_END_CHAR_AS_STRING
									
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_wtp_array);

	int total_sta_num;
	time_t now,now2;//qiuchen add it
	unsigned char SecurityID = 0;
	unsigned int SecurityType;
	unsigned int sta_ip;
	unsigned long long rx_pkts;			//xiaodawei modify, 32bits to 64bits, 20110309
	unsigned long long tx_pkts;			//xiaodawei modify, 32bits to 64bits, 20110309
	unsigned long long rx_data_pkts = 0;	//xiaodawei add
	unsigned long long tx_data_pkts = 0;	//xiaodawei add
	unsigned long long rx_bytes;	//xiaodawei modify, change 32bits to 64bits, 20110104
	unsigned long long tx_bytes;	//xiaodawei modify, change 32bits to 64bits, 20110104
	unsigned long rt_pkts;
	unsigned long rt_bytes;
	unsigned long err_pkts;
	unsigned long long rx_frag_packets = 0;	//xiaodawei add, 20110104
	unsigned long long tx_frag_packets = 0;	//xiaodawei add, 20110104
	unsigned long statime;
	unsigned long long tr;			//xiaodawei modify, 32bits to 64bits, 20110309
	unsigned long long rr;			//xiaodawei modify, 32bits to 64bits, 20110309
	unsigned long long tp;			//xiaodawei modify, 32bits to 64bits, 20110309
	char snrz[20]={0};
	char *snrz_p=snrz;
	char stactime[60]={0};
	char *stactime_p=stactime;
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
	for(i=0;i<wtp_num;i++){
		sprintf(wtp_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
			WTP[i]->WTPMAC[0],WTP[i]->WTPMAC[1],
			WTP[i]->WTPMAC[2],WTP[i]->WTPMAC[3],
			WTP[i]->WTPMAC[4],WTP[i]->WTPMAC[5]);
		bss_num=ASD_SEARCH_WTP_BSS(WTP[i]->WTPID,bss);
		total_sta_num=0;
		for(j=0;j<bss_num;j++){
			total_sta_num+=bss[j]->num_sta;
			}
		printf("sta_num: %d\n",total_sta_num);
		dbus_message_iter_open_container(&iter_wtp_array,DBUS_TYPE_STRUCT,NULL,&iter_wtp);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_STRING,&wtp_mac_p);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&total_sta_num);

		dbus_message_iter_open_container (&iter_wtp,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_STRING_AS_STRING	//sta_mac_p
										DBUS_TYPE_UINT32_AS_STRING	//sta_ip
										DBUS_TYPE_UINT32_AS_STRING	//sta->snr
										DBUS_TYPE_UINT64_AS_STRING	//rx_pkts
										DBUS_TYPE_UINT64_AS_STRING	//rx_data_pkts, xiaodawei add
										DBUS_TYPE_UINT64_AS_STRING	//rx_bytes //xiaodawei modify, change 32bits to 64bits, 20110104
										DBUS_TYPE_UINT64_AS_STRING	//tx_pkts
										DBUS_TYPE_UINT64_AS_STRING	//tx_data_pkts, xiaodawei add
										DBUS_TYPE_UINT64_AS_STRING	//tx_bytes //xiaodawei modify, change 32bits to 64bits, 20110104
										DBUS_TYPE_UINT32_AS_STRING	//rt_pkts
										DBUS_TYPE_UINT32_AS_STRING	//rt_bytes
										DBUS_TYPE_UINT32_AS_STRING	//err_pkts
										DBUS_TYPE_UINT64_AS_STRING	//rx_frag_packets //xiaodawei add
										DBUS_TYPE_UINT64_AS_STRING	//tx_frag_packets //xiaodawei add
										DBUS_TYPE_UINT32_AS_STRING	//statime
										DBUS_TYPE_UINT64_AS_STRING	//tr
										DBUS_TYPE_UINT64_AS_STRING	//rr
										DBUS_TYPE_UINT64_AS_STRING	//tp
										DBUS_TYPE_STRING_AS_STRING	//snrz_p
										DBUS_TYPE_UINT32_AS_STRING	//WTPID
										DBUS_TYPE_STRING_AS_STRING	//stactime_p
										DBUS_TYPE_UINT32_AS_STRING	//add time
										DBUS_TYPE_UINT32_AS_STRING  //sta_online_time  qiuchen add it
										DBUS_TYPE_STRING_AS_STRING  //wtpname
										DBUS_TYPE_UINT32_AS_STRING	//sta access security type
										DBUS_TYPE_STRING_AS_STRING	//sta name										
										DBUS_TYPE_UINT32_AS_STRING	//sta_access_time  qiuchen add it
										DBUS_TYPE_UINT32_AS_STRING	//online_time  qiuchen add it
										DBUS_TYPE_UINT32_AS_STRING	//MAXofRateset
											DBUS_TYPE_UINT32_AS_STRING /*rate 1 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 1 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 2 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 2 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 3 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 3 APStaTxDataRatePkts */	
											DBUS_TYPE_UINT32_AS_STRING /*rate 4 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 4 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 5 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 5 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 6 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 6 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 7 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 7 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 8 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 8 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 9 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 9 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 10 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 10 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 11 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 11 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 12 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 12 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 13 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 13 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 14 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 14 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 15 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 15 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 16 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 16 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 17 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 17 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 18 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 18 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 19 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 19 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 20 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 20 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 21 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 21 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 22 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 22 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 23 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 23 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 24 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 24 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 25 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 25 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 26 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 26 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 27 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 27 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 28 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 28 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 29 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 29 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 30 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 30 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 31 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 31 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 32 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 32 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 33 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 33 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 34 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 34 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 35 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 35 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 36 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 36 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 37 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 37 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 38 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 38 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 39 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 39 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 40 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 40 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 41 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 41 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 42 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 42 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 43 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 43 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 44 APStaRxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rate 44 APStaTxDataRatePkts */
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 1 */
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 2 */	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 3*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 4*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 5*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 6*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 7*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 8*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 9*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 10*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 11*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 12*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 13*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 14*/	
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 15*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 16*/
											DBUS_TYPE_UINT32_AS_STRING /*rssi local 17*/
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_sta_array);
		for(j=0;j<bss_num;j++){
			for(sta=bss[j]->sta_list;sta != NULL;sta=sta->next){
				//////////////////////////////////////////////////////////////////////
				//prepare data for send
				//////////////////////////////////////////////////////////////////////
				if(ASD_WLAN[bss[j]->WlanID]){
					SecurityID = ASD_WLAN[bss[j]->WlanID]->SecurityID;
				}
				if(ASD_SECURITY[SecurityID]){
					if((ASD_SECURITY[SecurityID]->securityType == OPEN)
						&&(ASD_SECURITY[SecurityID]->extensible_auth == 0)
						&&(ASD_SECURITY[SecurityID]->hybrid_auth == 1)
						&&(ASD_SECURITY[SecurityID]->mac_auth != 1)){
							SecurityType = asd_auth_type_authfree;	//NO_NEED_AUTH
					}
					else if (ASD_AUTH_TYPE_WEP_PSK(SecurityID))
					{
						SecurityType = asd_auth_type_weppsk;
					}
					else if (ASD_AUTH_TYPE_EAP(SecurityID) && (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm)))
					{
						SecurityType = asd_auth_type_autoauth;
					}
					else 
					{
						SecurityType = asd_auth_type_unkown;	//OTHERS
					}
				}
				Compute_Sta_Rate(sta);
				Get_Sta_SNR(sta);
				sprintf(sta_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
					sta->addr[0],sta->addr[1],
					sta->addr[2],sta->addr[3],
					sta->addr[4],sta->addr[5]);
				sta_ip=inet_addr(sta->in_addr);

				rx_pkts=sta->rxpackets;
				rx_data_pkts=sta->rxdatapackets;		//xiaodawei add for rx data packets, 20110309
				rx_bytes=sta->rxbytes;
				tx_pkts=sta->txpackets;
				tx_data_pkts=sta->txdatapackets;		//xiaodawei add for tx data packets, 20110309
				tx_bytes=sta->txbytes;
				rt_pkts=sta->retrypackets;
				rt_bytes=sta->retrybytes;
				err_pkts=sta->errpackets;
				rx_frag_packets = sta->rxframepackets;			//xiaodawei add for ap rx frame packets, 20110104
				tx_frag_packets = sta->txframepackets;			//xiaodawei add for ap tx frame packets, 20110104
				get_sysruntime(&now);//qiuchen change it	
				statime=(now-(sta->add_time_sysruntime)+sta->sta_online_time)*100;//qiuchen change it
				tr=sta->tr;
				rr=sta->rr;
				tp=sta->tp;
				if(sta->rssi){
					sprintf(snrz,"%d",((int)sta->rssi - 95));
				}
				else{
					sprintf(snrz,"-120");
					}
				//qiuchen add it for snmp to get the right time in case of changing sys time.
				now2 = time(NULL)-(now-(sta->add_time_sysruntime)+sta->sta_online_time);
				//end
				sprintf(stactime,"%s",ctime(&now2));//qiuchen change it 
				//////////////////////////////////////////////////////////////////////
				//send data prepared
				//////////////////////////////////////////////////////////////////////				
			    wtp_name_len = strlen(WTP[i]->WTPNAME);
				wtp_name = (char *)os_zalloc(wtp_name_len+1);
				if(wtp_name == NULL){

				      asd_printf(ASD_DBUS,MSG_ERROR,"%s\n",__func__);
				      return NULL;
				}
				os_memset(wtp_name,0,wtp_name_len+1);
                if(WTP[i] != NULL){
				 os_memcpy(wtp_name,WTP[i]->WTPNAME,strlen(WTP[i]->WTPNAME));
               	}
				dbus_message_iter_open_container(&iter_sta_array,DBUS_TYPE_STRUCT,NULL,&iter_sta);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_STRING,&sta_mac_p); 
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&sta_ip);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&sta->snr);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&rx_pkts);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&rx_data_pkts);	//xiaodawei add
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&rx_bytes);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&tx_pkts);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&tx_data_pkts);	//xiaodawei add
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&tx_bytes);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&rt_pkts);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&rt_bytes);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&err_pkts);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&rx_frag_packets);	//xiaodawei add
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&tx_frag_packets);	//xiaodawei add
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&statime);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&tr);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&rr);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT64,&tp);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_STRING,&snrz_p);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&WTP[i]->WTPID);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_STRING,&stactime_p);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&sta->add_time_sysruntime);//qiuchen change it
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&sta->sta_online_time);//qiuchen add it
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_STRING,&wtp_name);
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&SecurityType);
				//qiuchen
				if(NULL != sta->eapol_sm)
				{
					identity = sta->eapol_sm->identity;	/* sta name */
				}
				else 
				{
					identity = NULL;
				}	
				if (NULL == identity)
				{
					identity = (unsigned char*)tmp;	/* sta name null */
				}
				dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_STRING,&identity);	/* sta name */
				//qiuchen add it for AXSSZFI-1373
				time_t sta_access_time,online_time,nowsys;
				get_sysruntime(&nowsys);
				online_time = nowsys - sta->add_time_sysruntime;
				sta_access_time = time(NULL) - online_time;
				
				dbus_message_iter_append_basic (&iter_sta,
												 DBUS_TYPE_UINT32,
												 &(sta_access_time));
				
				dbus_message_iter_append_basic (&iter_sta,
												 DBUS_TYPE_UINT32,
												 &(online_time));
				dbus_message_iter_append_basic(&iter_sta, DBUS_TYPE_UINT32, &sta->MAXofRateset);
				asd_printf(ASD_DBUS,MSG_DEBUG,"---- sta->MAXofRateset = %u ----\n",sta->MAXofRateset);
				//end
				asd_printf(ASD_DBUS,MSG_DEBUG,"##########wtp_name = %s############\n",wtp_name);
				asd_printf(ASD_DBUS,MSG_DEBUG,"##########WTP[i]->WTPNAME = %s############\n",WTP[i]->WTPNAME);
				int k = 0/*, n = 0*/;

				for (k = 0; k < WTP_SUPPORT_RATE_NUM; k ++)
				{
					dbus_message_iter_append_basic(&iter_sta, DBUS_TYPE_UINT32, &(sta->wtp_sta_statistics_info.APStaRxDataRatePkts[k]));
					dbus_message_iter_append_basic(&iter_sta, DBUS_TYPE_UINT32, &(sta->wtp_sta_statistics_info.APStaTxDataRatePkts[k]));
					#if 0
					asd_printf(ASD_DBUS,MSG_DEBUG, "APStaRxDataRatePkts[%d] = %d \n", k, sta->wtp_sta_statistics_info.APStaRxDataRatePkts[k]);
					asd_printf(ASD_DBUS,MSG_DEBUG, "APStaTxDataRatePkts[%d] = %d \n", k, sta->wtp_sta_statistics_info.APStaTxDataRatePkts[k]);
					#endif
				}
				for(k = 0; k < WTP_RSSI_INTERVAL_NUM; k++)
				{
					dbus_message_iter_append_basic(&iter_sta, DBUS_TYPE_UINT32, &(sta->wtp_sta_statistics_info.APStaTxSignalStrengthPkts[k]));
					#if 0
					asd_printf(ASD_DBUS,MSG_DEBUG, "APStaTxSignalStrengthPkts[%d] = %d \n", k, sta->wtp_sta_statistics_info.APStaTxSignalStrengthPkts[k]);
					#endif
				}
				if(wtp_name){
					free(wtp_name);
					wtp_name = NULL;
				}
				dbus_message_iter_close_container(&iter_sta_array,&iter_sta);
				}
			}
		dbus_message_iter_close_container(&iter_wtp,&iter_sta_array);
		dbus_message_iter_close_container(&iter_wtp_array,&iter_wtp);
		}
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	dbus_message_iter_close_container(&iter,&iter_wtp_array);

	if(WTP != NULL){
		free(WTP);
		WTP = NULL;
	}
	os_free(wtp_name);
	wtp_name=NULL;
	
	return reply;
}
//mahz add for mib request , 2011.1.17 ,	dot11DistinguishTable
DBusMessage *asd_dbus_show_distinguish_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage	* reply; 
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusError err;		

	unsigned int acc_tms=0,auth_tms=0;
		
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	
	int i,j;
	unsigned int wtp_num=0;
	unsigned int bss_num=0;

	unsigned int ret=ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	
	ASD_WTP_ST **WTP=malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	//if(!WTP)
	//	ret = ASD_DBUS_MALLOC_FAIL;
	
	pthread_mutex_lock(&asd_g_wtp_mutex); 
	for(i=0;i<WTP_NUM;i++){
		if(ASD_WTP_AP[i]){
			WTP[wtp_num++]=ASD_WTP_AP[i];
		}
	}

	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//WTPID
									DBUS_TYPE_BYTE_AS_STRING		//wtp_mac
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//Identify
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//Abort
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING		//User Request Times
									DBUS_TYPE_UINT32_AS_STRING		//Access Times								
									DBUS_TYPE_UINT32_AS_STRING		//accessed sta num
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_wtp_array);

	pthread_mutex_lock(&asd_g_sta_mutex);	

	for(i=0;i<wtp_num;i++){
		unsigned int identify_request_num = 0; 
		unsigned int identify_success_num = 0; 
		unsigned int abort_key_error_num = 0; 
		unsigned int abort_invalid_num = 0; 
		unsigned int abort_timeout_num = 0; 
		unsigned int abort_refused_num = 0;
		unsigned int abort_others_num = 0; 
		unsigned int num_accessed_sta = 0;
						
		auth_tms = GetUserAuthTimes(WTP[i]->WTPID);
		acc_tms = GetAccessTimes(WTP[i]->WTPID);

		bss_num=ASD_SEARCH_WTP_BSS(WTP[i]->WTPID,bss);
		asd_printf(ASD_DBUS,MSG_DEBUG,"bss_num = %d\n",bss_num);
		for(j=0;j<bss_num;j++){			 	
			if (bss[j]->info == NULL) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"bss[%d]->info == NULL\n",j);
				continue;
			}	
			identify_request_num += bss[j]->info->identify_request; 
			identify_success_num += bss[j]->info->identify_success; 
			abort_key_error_num += bss[j]->info->abort_key_error; 
			abort_invalid_num += bss[j]->info->abort_invalid; 
			abort_timeout_num += bss[j]->info->abort_timeout; 
			abort_refused_num += bss[j]->info->abort_refused;
			abort_others_num += bss[j]->info->abort_others; 
			num_accessed_sta += bss[j]->num_sta;
			//if(ASD_WLAN[bss[j]->WlanID]!=NULL)
			//	num_accessed_sta += ASD_WLAN[bss[j]->WlanID]->a_num_sta;
		}
		
		DBusMessageIter iter_wtp;		
		dbus_message_iter_open_container(&iter_wtp_array,DBUS_TYPE_STRUCT,NULL,&iter_wtp);
	
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&(WTP[i]->WTPID));
		
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[0]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[1]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[2]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[3]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[4]));
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_BYTE,&(WTP[i]->WTPMAC[5]));		
		
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&identify_request_num);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&identify_success_num);
	
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&abort_key_error_num);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&abort_invalid_num);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&abort_refused_num);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&abort_timeout_num);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&abort_others_num);
		
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&auth_tms);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&acc_tms);
		dbus_message_iter_append_basic(&iter_wtp,DBUS_TYPE_UINT32,&num_accessed_sta);
							
		dbus_message_iter_close_container(&iter_wtp_array,&iter_wtp);
	}

	pthread_mutex_unlock(&asd_g_sta_mutex);	
	dbus_message_iter_close_container(&iter,&iter_wtp_array);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 

	if(WTP != NULL){
		free(WTP);
		WTP = NULL;
	}
	
	return reply;
}

//mahz add 2011.11.9 for GuangZhou Mobile
DBusMessage *asd_dbus_show_sta_statis_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply; 
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_sta;
	
	DBusError err;		
	dbus_error_init(&err);

	
	char  wtp_mac[18]={0};
	char * wtp_mac_p=wtp_mac;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM] = {NULL};
	struct sta_info * sta=NULL;
	int ret=ASD_DBUS_SUCCESS;
	int i=0,j=0;
	unsigned int wtp_num=0;
	unsigned int bss_num=0;
	unsigned char	SID = 0;
	time_t now;

	ASD_WTP_ST **WTP=malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	
	pthread_mutex_lock(&asd_g_wtp_mutex);		//mahz add 2011.4.20
	for(i=0;i<WTP_NUM;i++){
		if(ASD_WTP_AP[i]){
			WTP[wtp_num++]=ASD_WTP_AP[i];
		}
	}

	asd_printf(ASD_DBUS,MSG_DEBUG,"wtp_num: %d\n",wtp_num);
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num);	
	
	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING	//WTPID
										DBUS_TYPE_STRING_AS_STRING	//wtp_mac
										DBUS_TYPE_UINT32_AS_STRING		//no_auth_sta_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_sta_num
										DBUS_TYPE_UINT32_AS_STRING		//no_auth_accessed_total_time
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_accessed_total_time
										DBUS_TYPE_UINT32_AS_STRING		//no_auth_sta_abnormal_down_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_sta_abnormal_down_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_req_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_succ_num
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_fail_num
										//qiuchen
										DBUS_TYPE_UINT32_AS_STRING		// assoc_auth_online_sta_num(WEP/PSK assoc auth)
										DBUS_TYPE_UINT32_AS_STRING		// auto_auth_online_sta_num(SIM/PEAP)
										DBUS_TYPE_UINT32_AS_STRING		//all assoc_auth_accessed_total_time(WEP/PSK assoc auth)
										DBUS_TYPE_UINT32_AS_STRING		//all auto_auth_accessed_total_time(SIM/PEAP)
										DBUS_TYPE_UINT32_AS_STRING		//assoc_auth_sta_drop_cnt (WEP/PSK assoc auth sta drop count)
										DBUS_TYPE_UINT32_AS_STRING		//autoauth user lost connectioncnt (SIM/PEAP sta drop cnt)
										DBUS_TYPE_UINT32_AS_STRING		//auto_auth_req_cnt
										DBUS_TYPE_UINT32_AS_STRING		//auto_auth_suc_cnt
										DBUS_TYPE_UINT32_AS_STRING		//auto_auth_fail_cnt
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_wtp_array);

	pthread_mutex_lock(&asd_g_sta_mutex);	
	for(i=0;i<wtp_num;i++){
		unsigned int no_auth_sta_num = 0;
		unsigned int assoc_auth_sta_num = 0;
		unsigned int no_auth_accessed_total_time = 0;
		unsigned int assoc_auth_accessed_total_time = 0;
		unsigned int no_auth_sta_abnormal_down_num = 0;	
		unsigned int assoc_auth_sta_abnormal_down_num = 0;
		unsigned int assoc_auth_req_num = 0;
		unsigned int assoc_auth_succ_num = 0;
		unsigned int assoc_auth_fail_num = 0;
		//qiuchen
		unsigned int assoc_auth_online_sta_num = 0;		/* WEP/PSK assoc auth(SHARE:WEP) */
		unsigned int auto_auth_online_sta_num = 0;		/* SIM/PEAP */
		unsigned int all_assoc_auth_sta_total_time = 0;	/* all WEP/PSK assoc auth sta online time */
		unsigned int auto_auth_sta_total_time = 0;		/* SIM/PEAP auth sta online time */
		unsigned int assoc_auth_sta_drop_cnt = 0;	/* WEP/PSK assoc auth drop count */
		unsigned int auto_auth_sta_drop_cnt = 0;		/* SIM/PEAP */
		unsigned int auto_auth_req_cnt = 0, auto_auth_suc_cnt = 0, auto_auth_fail_cnt = 0;
		unsigned int simpeap_sta_total_time = 0;
		unsigned int weppsk_sta_total_time = 0;
		sprintf(wtp_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
			WTP[i]->WTPMAC[0],WTP[i]->WTPMAC[1],
			WTP[i]->WTPMAC[2],WTP[i]->WTPMAC[3],
			WTP[i]->WTPMAC[4],WTP[i]->WTPMAC[5]);
		bss_num=ASD_SEARCH_WTP_BSS(WTP[i]->WTPID,bss);
		asd_printf(ASD_DBUS,MSG_DEBUG,"bss_num: %d\n",bss_num);
		for(j=0;j<bss_num;j++){
			no_auth_sta_num += bss[j]->no_auth_sta_num;
			assoc_auth_sta_num += bss[j]->assoc_auth_sta_num;

			if(bss[j]->assoc_auth_req_num >= bss[j]->assoc_auth_succ_num){
				assoc_auth_fail_num += (bss[j]->assoc_auth_req_num - bss[j]->assoc_auth_succ_num);
			}
			//qiuchen copy from 1.3.16
						if (ASD_AUTH_TYPE_WEP_PSK(bss[j]->SecurityID))	/* WEP/PSK assocAuth */
						{
							assoc_auth_online_sta_num += bss[j]->u.weppsk.online_sta_num;
							assoc_auth_sta_drop_cnt += bss[j]->u.weppsk.total_sta_drop_cnt;
							all_assoc_auth_sta_total_time += bss[j]->u.weppsk.total_offline_sta_time;				
						}
						else if (ASD_AUTH_TYPE_EAP(bss[j]->SecurityID)) /* EAP auth */
						{
							/* SIM/PEAP auth */
			//				if (ASD_EAP_TYEP_SIM_PEAP(sta->eapol_sm))
							auto_auth_online_sta_num += bss[j]->u.eap_auth.autoauth.online_sta_num;
							auto_auth_sta_drop_cnt += bss[j]->u.eap_auth.autoauth.total_sta_drop_cnt;
							auto_auth_sta_total_time += bss[j]->u.eap_auth.autoauth.total_offline_sta_time;
			
							auto_auth_req_cnt += bss[j]->u.eap_auth.autoauth.auth_req_cnt;
							auto_auth_suc_cnt += bss[j]->u.eap_auth.autoauth.auth_suc_cnt;
							auto_auth_fail_cnt += bss[j]->u.eap_auth.autoauth.auth_fail_cnt;
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"StaNode->auto_auth_suc_cnt = %d ,StaNode->auto_auth_fail_cnt = %d\n",auto_auth_suc_cnt,auto_auth_fail_cnt);
						}
						
						no_auth_sta_abnormal_down_num += bss[j]->no_auth_sta_abnormal_down_num;
						assoc_auth_sta_abnormal_down_num += bss[j]->assoc_auth_sta_abnormal_down_num;
						assoc_auth_req_num += bss[j]->assoc_auth_req_num;
						assoc_auth_succ_num += bss[j]->assoc_auth_succ_num;
						if(bss[j]->assoc_auth_req_num >= bss[j]->assoc_auth_succ_num){
							assoc_auth_fail_num += (bss[j]->assoc_auth_req_num - bss[j]->assoc_auth_succ_num);
						}

			//end
			//time(&now);
			get_sysruntime(&now);
			if(ASD_WLAN[bss[j]->WlanID]){
				SID = ASD_WLAN[bss[j]->WlanID]->SecurityID;
			}
			for(sta=bss[j]->sta_list;sta != NULL;sta=sta->next){
				if(sta->security_type == NO_NEED_AUTH){
					bss[j]->no_auth_online_time += (now- (sta->add_time_sysruntime)+sta->sta_online_time);
					
				}else if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)
					||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)
					||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == SHARED)
					||(ASD_SECURITY[SID]->extensible_auth == 1)||(ASD_SECURITY[SID]->securityType == WAPI_PSK)
					||(ASD_SECURITY[SID]->securityType == WAPI_AUTH))){
						bss[j]->assoc_auth_online_time += (now- (sta->add_time_sysruntime)+sta->sta_online_time);
						
				}
				if (ASD_AUTH_TYPE_WEP_PSK(bss[j]->SecurityID))
				{
					weppsk_sta_total_time += (now- (sta->add_time_sysruntime)+sta->sta_online_time);
				}
				else if (ASD_AUTH_TYPE_EAP(bss[j]->SecurityID))
				{
					if (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
					{
						simpeap_sta_total_time += (now- (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen
					}
				}
			}
			no_auth_accessed_total_time += bss[j]->no_auth_online_time;
			no_auth_accessed_total_time += bss[j]->no_auth_downline_time;
			assoc_auth_accessed_total_time += bss[j]->assoc_auth_online_time;
			assoc_auth_accessed_total_time += bss[j]->assoc_auth_downline_time;
			bss[j]->assoc_auth_online_time = 0;
			bss[j]->no_auth_online_time = 0;
			auto_auth_sta_total_time += simpeap_sta_total_time;
			simpeap_sta_total_time = 0;
			all_assoc_auth_sta_total_time += weppsk_sta_total_time;
			weppsk_sta_total_time = 0;
		}
		dbus_message_iter_open_container(&iter_wtp_array,DBUS_TYPE_STRUCT,NULL,&iter_sta);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&WTP[i]->WTPID);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_STRING,&wtp_mac_p);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&no_auth_sta_num);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_sta_num);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&no_auth_accessed_total_time);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_accessed_total_time);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&no_auth_sta_abnormal_down_num);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_sta_abnormal_down_num);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_req_num);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_succ_num);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_fail_num);
		//qiuchen add it
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_online_sta_num);	/* WEP/PSK assoc auth */
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&auto_auth_online_sta_num);	/* SIM/PEAP auth */
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&all_assoc_auth_sta_total_time);/* WEP/PSK assoc auth */
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&auto_auth_sta_total_time);	/* SIM/PEAP auth */
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&assoc_auth_sta_drop_cnt);
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&auto_auth_sta_drop_cnt);		
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&auto_auth_req_cnt);	
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&auto_auth_suc_cnt);	
		dbus_message_iter_append_basic(&iter_sta,DBUS_TYPE_UINT32,&auto_auth_fail_cnt);

		
		dbus_message_iter_close_container(&iter_wtp_array,&iter_sta);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	pthread_mutex_unlock(&asd_g_wtp_mutex); 
	dbus_message_iter_close_container(&iter,&iter_wtp_array);

	if(WTP != NULL){
		free(WTP);
		WTP = NULL;
	}
	return reply;
}

DBusMessage *asd_dbus_show_ac_sta_info_of_all(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply; 
	DBusMessageIter iter;
	
	DBusError err;		
	dbus_error_init(&err);
	
	//struct asd_data *bss[BSS_NUM];
	struct sta_info * sta=NULL;
	int ret=ASD_DBUS_SUCCESS;
	int j=0;
	unsigned int bss_num=0;
	unsigned char	SID = 0;
	time_t now;
	unsigned int no_auth_sta_num = 0;
	unsigned int assoc_auth_sta_num = 0;
	unsigned int no_auth_accessed_total_time = 0;
	unsigned int assoc_auth_accessed_total_time = 0;
	unsigned int no_auth_sta_abnormal_down_num = 0; 
	unsigned int assoc_auth_sta_abnormal_down_num = 0;
	unsigned int assoc_auth_req_num = 0;
	unsigned int assoc_auth_succ_num = 0;
	unsigned int assoc_auth_fail_num = 0;
	unsigned int weppsk_assoc_req_cnt = 0, weppsk_assoc_succ_cnt = 0, weppsk_assoc_fail_cnt = 0;
	unsigned int radius_auth_res_cnt = 0;
	unsigned int assoc_auth_sta_drop_cnt = 0;	/* WEP/PSK assoc auth drop count */
	unsigned int auto_auth_sta_drop_cnt = 0;		/* SIM/PEAP */
	unsigned int assoc_auth_online_sta_num = 0; 	/* WEP/PSK assoc auth(SHARE:WEP) */
	unsigned int auto_auth_online_sta_num = 0;		/* SIM/PEAP */
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}

	bss_num = ASD_SEARCH_ALL_BSS_NUM(bss);
	asd_printf(ASD_DBUS,MSG_DEBUG,"bss_num: %d\n",bss_num);
	if(bss_num == 0)
		ret = ASD_BSS_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&bss_num);	
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
	for(j=0;j<bss_num;j++){
		no_auth_sta_num += bss[j]->no_auth_sta_num;
		assoc_auth_sta_num += bss[j]->assoc_auth_sta_num;
		no_auth_sta_abnormal_down_num += bss[j]->no_auth_sta_abnormal_down_num;
		assoc_auth_sta_abnormal_down_num += bss[j]->assoc_auth_sta_abnormal_down_num;
		assoc_auth_req_num += bss[j]->assoc_auth_req_num;
		assoc_auth_succ_num += bss[j]->assoc_auth_succ_num;
		if(bss[j]->assoc_auth_req_num >= bss[j]->assoc_auth_succ_num){
			assoc_auth_fail_num += (bss[j]->assoc_auth_req_num - bss[j]->assoc_auth_succ_num);
		}
		
		//qiuchen 
					if (ASD_AUTH_TYPE_WEP_PSK(bss[j]->SecurityID))	/* WEP/PSK assocAuth */
					{
						assoc_auth_sta_drop_cnt += bss[j]->u.weppsk.total_sta_drop_cnt;
						assoc_auth_online_sta_num += bss[j]->u.weppsk.online_sta_num;
						weppsk_assoc_req_cnt += bss[j]->u.weppsk.assoc_req;
						weppsk_assoc_succ_cnt += bss[j]->u.weppsk.assoc_success + bss[j]->u.weppsk.reassoc_success;
						weppsk_assoc_fail_cnt += bss[j]->u.weppsk.num_assoc_failure + bss[j]->u.weppsk.num_reassoc_failure; 			
					}
					else if (ASD_AUTH_TYPE_EAP(bss[j]->SecurityID)) /* EAP auth */
					{
						/* SIM/PEAP auth */
		//				if (ASD_EAP_TYEP_SIM_PEAP(sta->eapol_sm))
						auto_auth_sta_drop_cnt += bss[j]->u.eap_auth.autoauth.total_sta_drop_cnt;
						auto_auth_online_sta_num += bss[j]->u.eap_auth.autoauth.online_sta_num;
						radius_auth_res_cnt += bss[j]->u.eap_auth.autoauth.auth_resp_cnt;
					}
		//time(&now);
		get_sysruntime(&now);//qiuchen change it 2012.10.31
		if(ASD_WLAN[bss[j]->WlanID]){
			SID = ASD_WLAN[bss[j]->WlanID]->SecurityID;
		}
		for(sta=bss[j]->sta_list;sta != NULL;sta=sta->next){
			if(sta->security_type == NO_NEED_AUTH){
				bss[j]->no_auth_online_time += (now- (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen
			}else if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType != OPEN)){
				bss[j]->assoc_auth_online_time += (now- (sta->add_time_sysruntime)+sta->sta_online_time);//qiuchen
			}
		}
		no_auth_accessed_total_time += bss[j]->no_auth_online_time;
		bss[j]->no_auth_online_time = 0;
		no_auth_accessed_total_time += bss[j]->no_auth_downline_time;
		assoc_auth_accessed_total_time += bss[j]->assoc_auth_online_time;
		bss[j]->assoc_auth_online_time = 0;
		assoc_auth_accessed_total_time += bss[j]->assoc_auth_downline_time;
	}
	
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&no_auth_sta_num);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_sta_num);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&no_auth_accessed_total_time);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_accessed_total_time);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&no_auth_sta_abnormal_down_num);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_sta_abnormal_down_num);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_req_num);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_succ_num);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_fail_num);
	
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&radius_auth_res_cnt);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&weppsk_assoc_req_cnt);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&weppsk_assoc_succ_cnt);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&weppsk_assoc_fail_cnt);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_online_sta_num);	/* WEP/PSK assoc auth */
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&auto_auth_online_sta_num);	/* SIM/PEAP auth */
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&assoc_auth_sta_drop_cnt);
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&auto_auth_sta_drop_cnt); 	
	
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	if(bss){
		free(bss);
		bss = NULL;
	}
	return reply;
}

/*liuzhenhua add 2010-05-11*/
/*mib table 23*/
DBusMessage *asd_dbus_show_terminal_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_wtp;
	DBusMessageIter  iter_bss;
	DBusMessageIter  iter_sta;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusError err;		
	dbus_error_init(&err);
	
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	struct sta_info * sta=NULL;
	unsigned int bss_num = 0;
	unsigned int wtp_num = 0;
	unsigned int sta_num = 0;
	int j = 0, i = 0 , k =0;
	int ret = ASD_DBUS_SUCCESS;

	unsigned int security_id;
	unsigned int wlan_id;
	unsigned int radio_id;
	char wtp_mac[18]={0};
	char sta_mac[18]={0};
	char *p_wtp_mac = wtp_mac;
	char *p_sta_mac = sta_mac;
	unsigned int encryption_type;

	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	while(j<WTP_NUM){
		if(ASD_WTP_AP[j] != NULL){
			WTP[wtp_num] = ASD_WTP_AP[j];
			wtp_num++;
		}
		j++;
	}

	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&wtp_num); 	

	dbus_message_iter_open_container (&iter,
								   DBUS_TYPE_ARRAY,
								   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
								   		DBUS_TYPE_UINT32_AS_STRING //wtpid
								   		DBUS_TYPE_STRING_AS_STRING //wtp_mac
										DBUS_TYPE_UINT32_AS_STRING	//bss_num
										DBUS_TYPE_ARRAY_AS_STRING
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING //security_id
											DBUS_TYPE_UINT32_AS_STRING //wlan_id;
											DBUS_TYPE_UINT32_AS_STRING //radio_id
											DBUS_TYPE_UINT32_AS_STRING //sta_num
											DBUS_TYPE_ARRAY_AS_STRING
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING //sta_mac
												DBUS_TYPE_UINT32_AS_STRING //isqos
												DBUS_TYPE_UINT32_AS_STRING //in_addr
												DBUS_TYPE_UINT32_AS_STRING //mode
												DBUS_TYPE_UINT32_AS_STRING //channel
												DBUS_TYPE_UINT32_AS_STRING //nrate
												DBUS_TYPE_UINT32_AS_STRING //sta rx rate, xiaodawei add
												DBUS_TYPE_UINT32_AS_STRING //ispwoer
												DBUS_TYPE_UINT32_AS_STRING //vlanid
												DBUS_TYPE_STRING_AS_STRING //wlanname
												DBUS_TYPE_UINT32_AS_STRING //amode
												DBUS_TYPE_UINT32_AS_STRING //ctype
												DBUS_TYPE_UINT32_AS_STRING //atype
												DBUS_TYPE_UINT32_AS_STRING //wtpid
												DBUS_TYPE_UINT32_AS_STRING //encryption type
											DBUS_STRUCT_END_CHAR_AS_STRING	
										DBUS_STRUCT_END_CHAR_AS_STRING	
								   DBUS_STRUCT_END_CHAR_AS_STRING,
								   &iter_array);

	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	for(k=0;k<wtp_num;k++){
		sprintf(wtp_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
			WTP[k]->WTPMAC[0],WTP[k]->WTPMAC[1],
			WTP[k]->WTPMAC[2],WTP[k]->WTPMAC[3],
			WTP[k]->WTPMAC[4],WTP[k]->WTPMAC[5]);
		bss_num = ASD_SEARCH_WTP_STA(WTP[k]->WTPID, bss);
		
		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_wtp);
		dbus_message_iter_append_basic(&iter_wtp,
										DBUS_TYPE_UINT32,
										&WTP[k]->WTPID);
		dbus_message_iter_append_basic(&iter_wtp,
										DBUS_TYPE_STRING,
										&p_wtp_mac);
		dbus_message_iter_append_basic(&iter_wtp,
										DBUS_TYPE_UINT32,
										&bss_num);
										
		dbus_message_iter_open_container(&iter_wtp,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING //security_id
											DBUS_TYPE_UINT32_AS_STRING //wlan_id;
											DBUS_TYPE_UINT32_AS_STRING //radio_id
											DBUS_TYPE_UINT32_AS_STRING //sta_num
											DBUS_TYPE_ARRAY_AS_STRING
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING //sta_mac
												DBUS_TYPE_UINT32_AS_STRING //isqos
												DBUS_TYPE_UINT32_AS_STRING //in_addr
												DBUS_TYPE_UINT32_AS_STRING //mode
												DBUS_TYPE_UINT32_AS_STRING //channel
												DBUS_TYPE_UINT32_AS_STRING //nrate
												DBUS_TYPE_UINT32_AS_STRING //sta rx rate, xiaodawei add
												DBUS_TYPE_UINT32_AS_STRING //ispwoer
												DBUS_TYPE_UINT32_AS_STRING //vlanid
												DBUS_TYPE_STRING_AS_STRING //wlanname
												DBUS_TYPE_UINT32_AS_STRING //amode
												DBUS_TYPE_UINT32_AS_STRING //ctype
												DBUS_TYPE_UINT32_AS_STRING //atype
												DBUS_TYPE_UINT32_AS_STRING //wtpid
												DBUS_TYPE_UINT32_AS_STRING //encryption type
											DBUS_STRUCT_END_CHAR_AS_STRING	
										DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_sub_array);
		
		for(i=0; i<bss_num; i++){
			sta_num=bss[i]->num_sta;			
			security_id=bss[i]->SecurityID;
			wlan_id=bss[i]->WlanID;
			radio_id=bss[i]->Radio_G_ID;
			unsigned int secu_id = (unsigned int)security_id;
			if(ASD_SECURITY[secu_id]!=NULL)
				encryption_type = ASD_SECURITY[secu_id]->encryptionType;
			else 
				encryption_type = 0;
	
			dbus_message_iter_open_container (&iter_sub_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_bss);					
			dbus_message_iter_append_basic (&iter_bss,
											DBUS_TYPE_UINT32,
											&security_id);			
			dbus_message_iter_append_basic (&iter_bss,
											 DBUS_TYPE_UINT32,
											 &wlan_id);
			dbus_message_iter_append_basic (&iter_bss,
											 DBUS_TYPE_UINT32,
											 &radio_id);
			dbus_message_iter_append_basic (&iter_bss,
											 DBUS_TYPE_UINT32,
											 &sta_num);
			dbus_message_iter_open_container(&iter_bss,
										DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_STRING_AS_STRING //sta_mac
												DBUS_TYPE_UINT32_AS_STRING //isqos
												DBUS_TYPE_UINT32_AS_STRING //in_addr
												DBUS_TYPE_UINT32_AS_STRING //mode
												DBUS_TYPE_UINT32_AS_STRING //channel
												DBUS_TYPE_UINT32_AS_STRING //nrate
												DBUS_TYPE_UINT32_AS_STRING //sta rx rate, xiaodawei add
												DBUS_TYPE_UINT32_AS_STRING //ispwoer
												DBUS_TYPE_UINT32_AS_STRING //vlanid
												DBUS_TYPE_STRING_AS_STRING //wlanname
												DBUS_TYPE_UINT32_AS_STRING //amode
												DBUS_TYPE_UINT32_AS_STRING //ctype
												DBUS_TYPE_UINT32_AS_STRING //atype
												
												DBUS_TYPE_UINT32_AS_STRING //wtpid
												DBUS_TYPE_UINT32_AS_STRING //encryption type
											DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_struct);
				
				//printf("%s 222\n",__func__);
			for(sta=bss[i]->sta_list; sta!=NULL; sta=sta->next){
				char *ssid = sta->ssid->ssid;
				unsigned long isQos = sta->isQos;
				unsigned long channel = sta->channel;
				unsigned long nRate = sta->nRate;
				unsigned long txRate = sta->txRate;	//xiaodawei add, 20110103
				unsigned long isPowerSave = sta->isPowerSave;
				unsigned long vlan_id=sta->vlan_id;
				unsigned long in_addr;
				unsigned long mode = 0 ;
				unsigned long amode = 0;
				unsigned long ctype =0;
				unsigned long atype = 0;
				sprintf(sta_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
					sta->addr[0],sta->addr[1],
					sta->addr[2],sta->addr[3],
					sta->addr[4],sta->addr[5]);
				//printf("%s 333\n",__func__);
				in_addr=inet_addr(sta->in_addr);
				//printf("%s sta_num %d\n",__func__,sta_num);
				mode = sta->mode;
				
				switch(ASD_SECURITY[bss[i]->SecurityID]->securityType){
					case 1:
						amode=2;break;
					default:
						amode=1;break;
				}
				switch(ASD_SECURITY[bss[i]->SecurityID]->securityType){
					case 1:
					case 0:
						ctype=1;break;
					case 3:
					case 4:
					case 8:
						ctype=2;break;
					case 2:
					case 5:
					case 7:
						ctype=3;break;
					case 9:
						ctype=4;break;
				}
				atype = ctype;
				//printf("%s 555\n",__func__);
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sta);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_STRING,
											 &p_sta_mac);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &isQos);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &in_addr);				
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &mode);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &channel);				
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &nRate);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &txRate);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &isPowerSave);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &vlan_id);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_STRING,
											 &ssid);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &amode);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &ctype);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &atype);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &WTP[k]->WTPID);
				dbus_message_iter_append_basic (&iter_sta,
											DBUS_TYPE_UINT32,
											 &encryption_type);
				dbus_message_iter_close_container (&iter_struct,&iter_sta);

			}		
			dbus_message_iter_close_container (&iter_bss, &iter_struct);
			dbus_message_iter_close_container (&iter_sub_array,&iter_bss);
		}
		dbus_message_iter_close_container (&iter_wtp, &iter_sub_array);
		dbus_message_iter_close_container (&iter_array,&iter_wtp);
	}	
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	dbus_message_iter_close_container (&iter, &iter_array);

	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	
	return reply;
}

/*nl add 20100503*/
DBusMessage *asd_dbus_show_radio_wireless_info_bywtpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;
	
	struct asd_data *bss[L_BSS_NUM];
	unsigned int WTPID = 0;
	int i = 0, k = 0;
	int ii = 0;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int wtp_num = 0;

	unsigned int auth_tms = 0; 
	unsigned int repauth_tms = 0; 
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 
	unsigned int rx_data_pkts = 0;
	unsigned int tx_data_pkts = 0;
	unsigned int sta_num = 0;
	
	dbus_error_init(&err);

/////////////////////////////////////////////

	ASD_WTP_ST **WTP = NULL;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	
	pthread_mutex_lock(&asd_g_wtp_mutex);
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	pthread_mutex_lock(&asd_g_sta_mutex);	
	
	{
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 

		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,&wtp_num);	

		/*for sending wtp asd infor*/
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										  	 	DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   
												    DBUS_TYPE_UINT32_AS_STRING		//wtpid
												    DBUS_TYPE_UINT32_AS_STRING		//radio num
												    
												    DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING	//g_radio_id
														DBUS_TYPE_UINT32_AS_STRING	// a1
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														//DBUS_TYPE_UINT32_AS_STRING
														//DBUS_TYPE_UINT32_AS_STRING  //a6
														
														DBUS_TYPE_UINT32_AS_STRING //sta num
													DBUS_STRUCT_END_CHAR_AS_STRING
										
											DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);

		for(ii=0;ii<wtp_num;ii++){
			DBusMessageIter iter_sub_struct;
			DBusMessageIter iter_sub_array;
			unsigned int radio[L_RADIO_NUM]={0};
			unsigned int radionum = 0;
			WTPID = WTP[ii]->WTPID;
			
			for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
				struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
				if((interfaces->iface[i] == NULL)||(interfaces->iface[i]->bss == NULL))
					continue;
				else {
					radio[radionum]=i;
					radionum++;
				}
			}

			dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &WTP[ii]->WTPID); 

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &radionum); 
			
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	//g_radio_id
													
													DBUS_TYPE_UINT32_AS_STRING	// a1
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													//DBUS_TYPE_UINT32_AS_STRING	// a5
													
													//DBUS_TYPE_UINT32_AS_STRING	//a6
													DBUS_TYPE_UINT32_AS_STRING //sta num
													
											   DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			
			for(k=0; k<radionum; k++){
				unsigned int num = 0;
				auth_tms = 0; 
	 		 	repauth_tms = 0; 
				assoc_req_num = 0; 
				assoc_resp_num = 0; 
				rx_data_pkts = 0;
				tx_data_pkts = 0;
				sta_num = 0;
				num = ASD_SEARCH_RADIO_STA(radio[k], bss);	

				for(i=0; i<num; i++) {
					if (bss[i]->info == NULL) {
						continue;
					}
					GetBssDataPkts(bss[i]);
					auth_tms += bss[i]->usr_auth_tms;
					repauth_tms += bss[i]->ac_rspauth_tms;
					assoc_req_num += bss[i]->assoc_req; 
					assoc_resp_num += bss[i]->assoc_resp; 
					//rx_data_pkts += bss[i]->info->rx_data_pkts;
					//tx_data_pkts += bss[i]->info->tx_data_pkts;
						 sta_num += bss[i]->num_sta;
					
				}

				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(radio[k]));
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&auth_tms);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&repauth_tms);

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&assoc_req_num);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&assoc_req_num);
												
			/*	dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&rx_data_pkts);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&tx_data_pkts);*/

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&sta_num);

				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		pthread_mutex_unlock(&asd_g_sta_mutex);	
		pthread_mutex_unlock(&asd_g_wtp_mutex);

		if(WTP != NULL){
			free(WTP);
			WTP = NULL;
		}
			
		return reply;
}

/*nl add for showing stats infor for mib 20100509*/
DBusMessage *asd_dbus_show_wlan_stats_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	struct asd_data *bss[WTP_NUM*L_RADIO_NUM];
	//unsigned int bss_num = 0;
	unsigned int wtp_num = 0;
	int j = 0, i = 0,jj = 0;
	char *ip = "0.0.0.0";
	int ret = ASD_DBUS_SUCCESS;
	
	ASD_WTP_ST **WTP = NULL;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM*(sizeof(ASD_WTP_ST *)));
	pthread_mutex_lock(&asd_g_wtp_mutex); 
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	WID_WLAN	*WLAN[WLAN_NUM]={NULL};
	unsigned char wlan_num = 0;
	unsigned char wlan_id;
	unsigned char security_id;
	pthread_mutex_lock(&asd_g_wlan_mutex);
	for(j=0;j<WLAN_NUM;j++){
		if((ASD_WLAN[j] != NULL)&&(ASD_WLAN[j]->SecurityID != 0)&&(ASD_SECURITY[(ASD_WLAN[j]->SecurityID)]!=NULL)){
			WLAN[wlan_num] = ASD_WLAN[j];
			wlan_num++;
		}
	}

	unsigned int *wtp_count;
	wtp_count = (unsigned int *)malloc(WTP_NUM*sizeof(unsigned int));
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&wlan_num); 	


	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	//if(wtp_num != 0)
	{
		unsigned int total=0; 
		unsigned int wlan_count[WLAN_NUM]={0};
		unsigned int bss_num_of_wlan = 0;

		memset(wtp_count,0,WTP_NUM*sizeof(unsigned int));
		total=ASD_STA_SUMMARY(wtp_count,wlan_count);
		
		dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_BYTE_AS_STRING		//wlan id
												   		DBUS_TYPE_BYTE_AS_STRING		//security id
												   		
														DBUS_TYPE_UINT32_AS_STRING		//s1
														DBUS_TYPE_STRING_AS_STRING		
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING		//s4

														DBUS_TYPE_UINT32_AS_STRING		//total
														DBUS_TYPE_UINT32_AS_STRING		//sta num
														
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);

		

		for(i=0;i<wlan_num;i++){
			wlan_id = WLAN[i]->WlanID;
			security_id = WLAN[i]->SecurityID;

			if(ASD_SECURITY[security_id]->auth.auth_ip == NULL){
				ASD_SECURITY[security_id]->auth.auth_ip = ip;
				
			}

			unsigned int sta_num_of_wlan = 0;
			bss_num_of_wlan = ASD_SEARCH_WLAN_STA(WLAN[i]->WlanID, bss); 

			for(jj=0; jj<bss_num_of_wlan; jj++) {
				sta_num_of_wlan	+= bss[jj]->num_sta;
			}


			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wlan_id));
			

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(security_id));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[security_id]->auth.auth_port));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_STRING,
											 &(ASD_SECURITY[security_id]->auth.auth_ip));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[security_id]->securityType));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[security_id]->extensible_auth));
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(total));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(sta_num_of_wlan));
			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
			if(strcmp(ASD_SECURITY[security_id]->auth.auth_ip, "0.0.0.0") == 0){
				ASD_SECURITY[security_id]->auth.auth_ip = NULL;
			}

		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 

	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	if(wtp_count){
		free(wtp_count);
		wtp_count = NULL;
	}
	
	return reply;
}

#if 0 // for old version
/*nl add for showing ssid stats infor for mib 20100511*/
DBusMessage *asd_dbus_show_wlan_ssid_stats_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	
	DBusError err;		
	dbus_error_init(&err);

	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned int bss_num = 0;
	int wtp_num = 0;
	int j = 0;	int i = 0; 
	unsigned char jj=0;
	unsigned char wlan_id;
	unsigned int acc_tms=0,auth_tms=0,repauth_tms=0;
	int ret = ASD_DBUS_SUCCESS;
	
	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	while(j<WTP_NUM){
		if(ASD_WTP_AP[j] != NULL){
			WTP[wtp_num] = ASD_WTP_AP[j];
			wtp_num++;
		}
		j++;
	}
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	if(ret == 0){
		dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_UINT32_AS_STRING		//wtp id
												   		DBUS_TYPE_UINT32_AS_STRING		//bss num

		
												   		//bss information
														DBUS_TYPE_ARRAY_AS_STRING
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING		//bssid
																DBUS_TYPE_BYTE_AS_STRING		//wlanid

																DBUS_TYPE_UINT32_AS_STRING		//a1
																DBUS_TYPE_UINT32_AS_STRING		//
																DBUS_TYPE_UINT32_AS_STRING		//
																DBUS_TYPE_UINT32_AS_STRING		//a4
																DBUS_TYPE_UINT32_AS_STRING		// 
																DBUS_TYPE_UINT32_AS_STRING		//a6
																DBUS_TYPE_UINT32_AS_STRING		//a7
																DBUS_TYPE_UINT32_AS_STRING 		//A8
																DBUS_TYPE_UINT32_AS_STRING		

												
															DBUS_STRUCT_END_CHAR_AS_STRING
														
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);
		
		for(i = 0; i < wtp_num; i++){		
			unsigned int wtp_bss_num;
			acc_tms = GetAccessTimes(WTP[i]->WTPID);
			auth_tms = GetUserAuthTimes(WTP[i]->WTPID);
			repauth_tms = GetWTPRspAuthTimes(WTP[i]->WTPID);
			bss_num = ASD_SEARCH_WTP_STA(WTP[i]->WTPID, bss);	
			
			dbus_message_iter_open_container (&iter_array,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(WTP[i]->WTPID));

			dbus_message_iter_append_basic (&iter_struct,DBUS_TYPE_UINT32,&bss_num);
	
			dbus_message_iter_open_container (&iter_struct,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING		//bssid
													DBUS_TYPE_BYTE_AS_STRING		//wlanid
													DBUS_TYPE_UINT32_AS_STRING	//	a1
													DBUS_TYPE_UINT32_AS_STRING	//
													DBUS_TYPE_UINT32_AS_STRING	//
													
													DBUS_TYPE_UINT32_AS_STRING	//a4 
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	//a6
													
													DBUS_TYPE_UINT32_AS_STRING	//a7
													DBUS_TYPE_UINT32_AS_STRING  //A8
													DBUS_TYPE_UINT32_AS_STRING 	//sta num
			
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_sub_array);
			
			for(jj = 0; jj < bss_num; jj++){	
				unsigned char bss_index;
				bss_index = jj;
				GetBssDataPkts(bss[jj]);
				dbus_message_iter_open_container (&iter_sub_array,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_sub_struct);

				dbus_message_iter_append_basic (&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&bss_index);
				
				dbus_message_iter_append_basic (&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&bss[jj]->WlanID);
			
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->usr_auth_tms));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->ac_rspauth_tms));
			
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->assoc_req));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->assoc_resp));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->info->rx_ctrl_pkts));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->info->tx_ctrl_pkts));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->info->rx_data_pkts));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss[jj]->info->tx_data_pkts));

				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_UINT32,
												&(bss[jj]->num_sta));

				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);	//wtp num
		}
		dbus_message_iter_close_container (&iter, &iter_array); 		
	}

	pthread_mutex_unlock(&asd_g_sta_mutex);	

	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	return reply;
}
#endif
//fengwenchao add 20101223
DBusMessage *asd_dbus_show_all_wlan_sta_num(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	DBusError err;		
	dbus_error_init(&err);

	printf("accessinto asd\n");

	//int i,j = 0;
	int i = 0;
	int j = 0;
	int wlan_num = 0;
	//int wlan_num2 = 0;
	int ret = ASD_DBUS_SUCCESS;

	WID_WLAN **WLAN;
	WLAN = os_zalloc(WLAN_NUM*sizeof(struct WID_WLAN *));
	if( WLAN == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WLAN,0,WLAN_NUM*sizeof(struct WID_WLAN *));

	/*if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_UINT32,&wlan_num,
							DBUS_TYPE_INVALID))){

	asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
			
	if (dbus_error_is_set(&err)) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
		dbus_error_free(&err);
	}
	return NULL;
	}*/
	
	/*struct asd_data **bss;
	bss = os_malloc(BSS_NUM*sizeof(struct asd_data *));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	*/
	//struct asd_data *bss[BSS_NUM];
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		if(WLAN)
		{
			free(WLAN);
			WLAN = NULL;
		}
		return NULL;
	}
	
	pthread_mutex_lock(&asd_g_wlan_mutex);
	pthread_mutex_lock(&asd_g_sta_mutex);

		while(i<WLAN_NUM)
	{
		if(ASD_WLAN[i] != NULL)
		{
			WLAN[wlan_num] = ASD_WLAN[i];
			wlan_num++;
		}
		i++;
	}

	if(wlan_num ==0)
		ret = ASD_WLAN_NOT_EXIST;	

//	printf("asd  wlan_num2 = %d\n",wlan_num2);
	reply = dbus_message_new_method_return(msg);		
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &wlan_num); 	
	dbus_message_iter_open_container (&iter,
								   DBUS_TYPE_ARRAY,
								   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING  //wlanid
											DBUS_TYPE_UINT32_AS_STRING   //bss  num
											DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING   //sta num
												DBUS_STRUCT_END_CHAR_AS_STRING
								   DBUS_STRUCT_END_CHAR_AS_STRING,
								   &iter_array);


	for(i=0;i<wlan_num;i++)
	{
		unsigned char wlanid = 0;
		wlanid = WLAN[i]->WlanID;
		printf("WLAN[%d]->WlanID = %d\n",i,WLAN[i]->WlanID);
		int num = 0;
		num = ASD_SEARCH_WLAN_STA(wlanid, bss);	

		dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
								  								   								   
		dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_BYTE,&(wlanid));

		printf("wlanid = %d\n",wlanid);
							
		dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32,&(num));	

		printf("bss num = %d\n",num);

		dbus_message_iter_open_container (&iter_struct,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING   //sta num
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_sub_array);

		for(j=0;j<num;j++)
		{
			dbus_message_iter_open_container (&iter_sub_array,DBUS_TYPE_STRUCT,NULL,&iter_sub_struct);
													   
													   
													   
			dbus_message_iter_append_basic(&iter_sub_struct,DBUS_TYPE_UINT32, &(bss[j]->num_sta));
							
			printf("bss[j]->num_sta = %d\n",bss[j]->num_sta);
							  
			dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);				 
					
		}
						
		dbus_message_iter_close_container (&iter_struct, &iter_sub_array);		

		dbus_message_iter_close_container (&iter_array, &iter_struct);	
	}
	dbus_message_iter_close_container (&iter, &iter_array);	
	if(WLAN){
		free(WLAN);
		WLAN = NULL;
	}
	if(bss){
		free(bss);
		bss = NULL;
	}
	
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	return reply;
}

//fengwenchao add end

/* zhangshu copy from 1.2, 2010-09-13 */
/*nl add for showing ssid stats infor for mib 20100511*/
DBusMessage *asd_dbus_show_wlan_ssid_stats_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	/*zhaoruijia,20100804,为支持多radio接入相同的wlan ，start*/
    DBusMessageIter iter_sub_sub_array;
	DBusMessageIter iter_sub_sub_struct;
	/*zhaoruijia,20100804,为支持多radio接入相同的wlan  ,end*/
	
	DBusError err;		
	dbus_error_init(&err);

	struct asd_data *bss[L_BSS_NUM] = {NULL};
	unsigned int wtp_num = 0;
	int i = 0; 
	int j = 0;	
	int jj=0;
	
	unsigned int WTPID = 0;
	unsigned int acc_tms=0,auth_tms=0,repauth_tms=0;
	int ret = ASD_DBUS_SUCCESS;
	  //printf("asd_dbus_show_wlan_ssid_stats_info_of_all_wtp_start\n");
	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
    wtp_num = Asd_Find_Wtp(WTP);
	//printf("@@@@@@@@@asd_wtp_num=%u@@@@@@@@@@\n",wtp_num);

	//zhaoruijia, for  account radio infor
	BSS_MIB_INFO_ST *buf=NULL;   //fengwenchao add 20110127

	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	reply = dbus_message_new_method_return(msg);

	if(NULL==reply)
		printf("reply = NULL\n");

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	//printf("@@@@@@@@@@@asd_wtp_num = %u@@@@@@@@@@@@@@@\n",wtp_num);
	

	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	if(ret == 0){
  
                    dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_UINT32_AS_STRING		//wtp id
												   		DBUS_TYPE_UINT32_AS_STRING		//radio num

                                                     //raidoid information
                                                      DBUS_TYPE_ARRAY_AS_STRING
													  DBUS_STRUCT_BEGIN_CHAR_AS_STRING
		                                                 DBUS_TYPE_UINT32_AS_STRING      //Radioid
		                                                 DBUS_TYPE_UINT32_AS_STRING      //radioBindToWlanNum
												   		//bss information
														    DBUS_TYPE_ARRAY_AS_STRING
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING		//bssid
																DBUS_TYPE_BYTE_AS_STRING		//wlanid

																DBUS_TYPE_UINT32_AS_STRING		//a1
																DBUS_TYPE_UINT32_AS_STRING		//
																DBUS_TYPE_UINT32_AS_STRING		//
																DBUS_TYPE_UINT32_AS_STRING		//a4
																DBUS_TYPE_UINT32_AS_STRING		// 
																DBUS_TYPE_UINT32_AS_STRING		//a6
																//DBUS_TYPE_UINT32_AS_STRING		//a7
																//DBUS_TYPE_UINT32_AS_STRING 		//A8
																DBUS_TYPE_UINT32_AS_STRING		
																DBUS_TYPE_UINT32_AS_STRING		
                                                                //fengwenchao add 20110127
																DBUS_TYPE_UINT64_AS_STRING    //无线上行端口流量
																DBUS_TYPE_UINT64_AS_STRING	  //无线下行端口流量
																DBUS_TYPE_UINT64_AS_STRING    //信道下行总包数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行丢包数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行MAC错包数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行重传包数
																DBUS_TYPE_UINT64_AS_STRING    //信道上行总的帧数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行总的帧数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行总的错帧数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行总的丢帧数
																DBUS_TYPE_UINT64_AS_STRING    //信道下行总的重传帧数
																DBUS_TYPE_UINT64_AS_STRING    //信道上行总的丢帧数   fengwenchao modify 20110224
																DBUS_TYPE_UINT64_AS_STRING    //信道上行总的重传帧数 fengwenchao modify 20110224
                                                                //fengwenchao add end
												
															DBUS_STRUCT_END_CHAR_AS_STRING
														DBUS_STRUCT_END_CHAR_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);
		
		for(i = 0; i < wtp_num; i++){		
         	//printf("@@@@@@@@@@@asd_WTP[i]->WTPID_1 = %d@@@@@@@@@@@@@@@\n",WTP[i]->WTPID);
			unsigned int radio[L_RADIO_NUM] = {0};
			unsigned int radionum = 0;
			acc_tms = GetAccessTimes(WTP[i]->WTPID);
			auth_tms = GetUserAuthTimes(WTP[i]->WTPID);
			repauth_tms = GetWTPRspAuthTimes(WTP[i]->WTPID);
			WTPID=WTP[i]->WTPID;
			//bss_num = ASD_SEARCH_WTP_STA(WTP[i]->WTPID, bss);	

			for(j = WTPID*L_RADIO_NUM; j < (WTPID+1)*L_RADIO_NUM; j++) {
				struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
				if((interfaces->iface[j] == NULL)||(interfaces->iface[j]->bss == NULL))
					continue;
				else {
					radio[radionum]=j;
					radionum++;
				}
			}
			  //printf("@@@@@@@@@@@asd_radionum_1 = %u@@@@@@@@@@@@@@@\n",radionum);
			  //printf("@@@@@@@@@@@asd_WTP[i]->WTPID_2 = %d@@@@@@@@@@@@@@@\n",WTP[i]->WTPID);
			  //printf("@@@@@@@@@@@asd_radio[radionum] = %u@@@@@@@@@@@@@@@\n",radio[radionum]);
			
			dbus_message_iter_open_container (&iter_array,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32,
											&(WTP[i]->WTPID));
		   //printf("@@@@@@@@@@@asd_WTP[i]->WTPID_3 = %u@@@@@@@@@@@@@@@\n",WTP[i]->WTPID);

			dbus_message_iter_append_basic (&iter_struct,DBUS_TYPE_UINT32,&radionum);
			   //printf("@@@@@@@@@@@asd_radionum_2 = %u@@@@@@@@@@@@@@@\n",radionum);
	     
		 dbus_message_iter_open_container (&iter_struct,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING  //Radio_id
											DBUS_TYPE_UINT32_AS_STRING      //radioBindToWlanNum
											//bss information
												 DBUS_TYPE_ARRAY_AS_STRING
												 DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING		//bssid
													DBUS_TYPE_BYTE_AS_STRING		//wlanid

													DBUS_TYPE_UINT32_AS_STRING		//a1
													DBUS_TYPE_UINT32_AS_STRING		//
													DBUS_TYPE_UINT32_AS_STRING		//
													DBUS_TYPE_UINT32_AS_STRING		//a4
													DBUS_TYPE_UINT32_AS_STRING		// 
													DBUS_TYPE_UINT32_AS_STRING		//a6
													//DBUS_TYPE_UINT32_AS_STRING		//a7
													//DBUS_TYPE_UINT32_AS_STRING 		//A8
													DBUS_TYPE_UINT32_AS_STRING		
													DBUS_TYPE_UINT32_AS_STRING		

                                                    //fengwenchao add 20110127
												    DBUS_TYPE_UINT64_AS_STRING	 //无线上行端口流量
												    DBUS_TYPE_UINT64_AS_STRING	 //无线下行端口流量
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行总包数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行丢包数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行MAC错包数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行重传包数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道上行总的帧数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行总的帧数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行总的错帧数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行总的丢帧数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道下行总的重传帧数
												    DBUS_TYPE_UINT64_AS_STRING	 //信道上行总的丢帧数    fengwenchao modify 20110224
												    DBUS_TYPE_UINT64_AS_STRING	 //信道上行总的重传帧数  fengwenchao modify 20110224
                                                    //fengwenchao add end
												
													DBUS_STRUCT_END_CHAR_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
								            &iter_sub_array);
     	for(j=0 ; j<L_RADIO_NUM;j++)
     	{
              unsigned int radioid;
			  unsigned int radioBindToWlanNum = 0;
			  radioid = ( unsigned int)j+(unsigned int)WTPID*4;//global radio id
			  //printf("ASDDDDDDDDDDDDDDDDDDDDDDDDDDDD\n");
			  //printf("@@@@@@@@@@@@@@@@@@@@@@@@WTP[i]->WTPID=%u,radioid=%u@@@@@\n",WTP[i]->WTPID,radioid);
			  if(ASD_IS_WTP_BIND_RADIO(WTP[i]->WTPID,radioid))
              {
				
				radioBindToWlanNum = ASD_SEARCH_RADIO_STA(radioid, bss);	
		        //存入radio num
		          //printf("@@@@@@@@@@@asd_radioBindToWlanNum = %u@@@@@@@@@@@@@@@\n",radioBindToWlanNum);
                 dbus_message_iter_open_container(&iter_sub_array,
                                                  DBUS_TYPE_STRUCT,
												  NULL,
												   &iter_sub_struct);
				 dbus_message_iter_append_basic (&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&radioid);//存入radioid
				//printf("@@@@@@@@@@@asd_radioid = %u@@@@@@@@@@@@@@@\n",radioid);								
                 dbus_message_iter_append_basic (&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&radioBindToWlanNum);//存入radioBindToWlanNum

				 dbus_message_iter_open_container (&iter_sub_struct,
											     DBUS_TYPE_ARRAY,
												 DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING		//bssid
													DBUS_TYPE_BYTE_AS_STRING		//wlanid

													DBUS_TYPE_UINT32_AS_STRING		//a1
													DBUS_TYPE_UINT32_AS_STRING		//
													DBUS_TYPE_UINT32_AS_STRING		//
													DBUS_TYPE_UINT32_AS_STRING		//a4
													DBUS_TYPE_UINT32_AS_STRING		// 
													DBUS_TYPE_UINT32_AS_STRING		//a6
													//DBUS_TYPE_UINT32_AS_STRING		//a7
													//DBUS_TYPE_UINT32_AS_STRING 		//A8
													DBUS_TYPE_UINT32_AS_STRING		
													DBUS_TYPE_UINT32_AS_STRING		
                                                    //fengwenchao add 20110127													
													DBUS_TYPE_UINT64_AS_STRING    //无线上行端口流量
													DBUS_TYPE_UINT64_AS_STRING	  //无线下行端口流量
													DBUS_TYPE_UINT64_AS_STRING    //信道下行总包数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行丢包数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行MAC错包数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行重传包数
													DBUS_TYPE_UINT64_AS_STRING    //信道上行总的帧数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行总的帧数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行总的错帧数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行总的丢帧数
													DBUS_TYPE_UINT64_AS_STRING    //信道下行总的重传帧数
													DBUS_TYPE_UINT64_AS_STRING    //信道上行总的丢帧数     fengwenchao modify 20110224
													DBUS_TYPE_UINT64_AS_STRING    //信道上行总的重传帧数fengwenchao modify 20110224
                                                    //fengwenchao add end
			        							 DBUS_STRUCT_END_CHAR_AS_STRING,
				 				            &iter_sub_sub_array);

				 for(jj=0;jj<radioBindToWlanNum;jj++)
				 	{
                       if((bss[jj]==NULL)||(bss[jj]->info==NULL))
					   	  continue;
					      unsigned char bss_index;
						  bss_index=jj;
                          //fengwenchao add 20110127
						  buf=(BSS_MIB_INFO_ST *)os_zalloc(sizeof(BSS_MIB_INFO_ST)*L_BSS_NUM);
						  if(buf==NULL)
							{
								asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
								exit(1);
							}

						  memset(buf,0,sizeof(BSS_MIB_INFO_ST)*L_BSS_NUM);	
                           //fengwenchao add end						  
						  dbus_message_iter_open_container (&iter_sub_sub_array,
													    DBUS_TYPE_STRUCT,
													    NULL,
													    &iter_sub_sub_struct);
						 GetBssDataPkts(bss[jj]);
						 GetInfoFromSta(bss[jj],buf,bss_index);                //fengwenchao add 20110127
						 dbus_message_iter_append_basic (&iter_sub_sub_struct,
																	DBUS_TYPE_BYTE,
																	&bss_index); //bssid
									//printf("@@@@@@@@@@@asd_bss_index = %d@@@@@@@@@@@@@@@\n",bss_index);
						 dbus_message_iter_append_basic (&iter_sub_sub_struct,
																	DBUS_TYPE_BYTE,
																	&bss[jj]->WlanID);///wlanid
																	
						 //printf("@@@@@@@@@@@asd_bss[jj]->WlanID = %d@@@@@@@@@@@@@@@\n",bss[jj]->WlanID);
						 dbus_message_iter_append_basic (&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&bss[jj]->acc_tms);	//xiaodawei add access times for telecom test, 20110301
						 
						 dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->usr_auth_tms));
						  //printf("@@@@@@@@@@@asd_bss[jj]->usr_auth_tms = %u@@@@@@@@@@@@@@@\n",bss[jj]->usr_auth_tms);
						 dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->ac_rspauth_tms));
						//printf("@@@@@@@@@@@asd_bss[jj]->ac_rspauth_tms = %u@@@@@@@@@@@@@@@\n",bss[jj]->ac_rspauth_tms);
								
							dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->assoc_req));
									//printf("@@@@@@@@@@@asd_bss[jj]->assoc_req = %u@@@@@@@@@@@@@@@\n",bss[jj]->assoc_req);
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->assoc_resp));
									//printf("@@@@@@@@@@@asd_bss[jj]->assoc_resp = %u@@@@@@@@@@@@@@@\n",bss[jj]->assoc_resp);
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->info->rx_ctrl_pkts));
									//printf("@@@@@@@@@@@asd_bss[jj]->info->rx_ctrl_pkts = %u@@@@@@@@@@@@@@@\n",bss[jj]->info->rx_ctrl_pkts);
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->info->tx_ctrl_pkts));
									//printf("@@@@@@@@@@@asd_bss[jj]->info->tx_ctrl_pkts = %u@@@@@@@@@@@@@@@\n",bss[jj]->info->tx_ctrl_pkts);
									/*dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->info->rx_data_pkts));
										//printf("@@@@@@@@@@@asd_bss[jj]->info->rx_data_pkts = %u@@@@@@@@@@@@@@@\n",bss[jj]->info->rx_data_pkts);
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT32,
																	&(bss[jj]->info->tx_data_pkts));*/
									//printf("@@@@@@@@@@@asd_bss[jj]->info->tx_data_pkts = %u@@@@@@@@@@@@@@@\n",bss[jj]->info->tx_data_pkts);

									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	  DBUS_TYPE_UINT32,
																	&(bss[jj]->num_sta));
									//printf("@@@@@@@@@@@asd_bss[jj]->num_sta = %u@@@@@@@@@@@@@@@\n",bss[jj]->num_sta);
                                	//fengwenchao add 20110127
									//GetInfoFromSta(bss[jj],buf,jj);
								

									//printf("buf[jj].wl_up_flow = %\lu\n",buf[jj].wl_up_flow);
									//printf("buf[jj].wl_up_flow = %\llu\n",buf[jj].wl_up_flow);
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																		DBUS_TYPE_UINT64,
																	&(buf[jj].wl_up_flow));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].wl_dw_flow));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_pck));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_los_pck));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_mac_err_pck));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_resend_pck));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_up_frm));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_frm));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_err_frm));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_los_frm));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,
																	&(buf[jj].ch_dw_resend_frm));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,             //fengwenchao modify 20110224
																	&(buf[jj].ch_up_los_frm));
									dbus_message_iter_append_basic(&iter_sub_sub_struct,
																	DBUS_TYPE_UINT64,             //fengwenchao modify 20110224
																	&(buf[jj].ch_up_resend_frm));

								 //  printf("buf[jj].wl_up_flow = %\lu\n",buf[jj].ch_dw_resend_frm);
								  // printf("buf[jj].wl_up_flow = %\llu\n",buf[jj].ch_dw_resend_frm);
                                 //fengwenchao add 20110127
								   dbus_message_iter_close_container (&iter_sub_sub_array, &iter_sub_sub_struct);
                                 //fengwenchao add 20110127
	                            if(buf)					   
						  		 {free(buf);
						   		 buf=NULL;}			   
	                         	//fengwenchao add end
				    }
             
				dbus_message_iter_close_container (&iter_sub_struct, &iter_sub_sub_array);
			    dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);	//WLAN_NUM
                  
     		   }
		    }

			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);	//wtp num
		}
		dbus_message_iter_close_container (&iter, &iter_array); 		
	}

	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
     
	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	return reply;
}


/*nl add for showing user link infor for mib 20100525 table 15*/
DBusMessage *asd_dbus_show_user_link_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int bss_num = 0;
	unsigned int wtp_num = 0;
	int i = 0, k =0, kk = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	//if(wtp_num != 0)
	{
		dbus_message_iter_open_container (&iter,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//wtpid
															
															DBUS_TYPE_BYTE_AS_STRING		//mac0
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING		//mac5
															
															DBUS_TYPE_UINT64_AS_STRING		//total past online time
															
															DBUS_TYPE_UINT32_AS_STRING		//s1
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															
															DBUS_TYPE_UINT32_AS_STRING		//s4
 															DBUS_TYPE_UINT32_AS_STRING		//s5
															DBUS_TYPE_UINT32_AS_STRING		

 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//s9

															DBUS_TYPE_UINT32_AS_STRING		//time
															
													   DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_array);
		for(k=0;k<wtp_num;k++){
			unsigned int auth_invalid_num = 0; 
			unsigned int auth_timeout_num = 0; 
			unsigned int auth_refused_num = 0; 
			unsigned int auth_others_num = 0; 
			unsigned int deauth_request_num = 0; 
			unsigned int deauth_ap_unable_num = 0; 
			unsigned int deauth_abnormal_num = 0; 
			unsigned int deauth_others_num = 0; 
			unsigned int disassoc_request_num = 0; 
			unsigned int total_online_time = 0;
			bss_num = ASD_SEARCH_WTP_STA(WTP[k]->WTPID, bss);	
			
			unsigned long long wtp_total_past_online_time = 0;
			
			for(i=0; i<bss_num; i++){
				GetBssDataPkts(bss[i]);
				
				if (bss[i]->info == NULL) {
					continue;
				}
				
				wtp_total_past_online_time+=bss[i]->total_past_online_time;
				wtp_total_past_online_time+=bss[i]->total_present_online_time;
				total_online_time = wtp_total_past_online_time;
				
				auth_invalid_num += bss[i]->info->auth_invalid; 
				auth_timeout_num += bss[i]->info->auth_timeout; 
				auth_refused_num += bss[i]->info->auth_refused; 
				auth_others_num += bss[i]->info->auth_others; 
				deauth_request_num += bss[i]->info->deauth_request; 
				deauth_ap_unable_num += bss[i]->info->deauth_ap_unable; 
				deauth_abnormal_num += bss[i]->info->deauth_abnormal; 
				deauth_others_num += bss[i]->info->deauth_others; 
				disassoc_request_num += bss[i]->info->disassoc_request; 
			}

			for(kk = WTP[k]->WTPID*L_RADIO_NUM; kk < (WTP[k]->WTPID+1)*L_RADIO_NUM; kk++) {
				if(interfaces->iface[kk] != NULL) {
					wtp_total_past_online_time += interfaces->iface[kk]->total_past_online_time;
					total_online_time += interfaces->iface[kk]->past_online_time;
				}
			}

			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											 &(WTP[k]->WTPID));

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[0]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[1]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[2]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[3]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[4]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[5]));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT64,
											 &wtp_total_past_online_time);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_invalid_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_timeout_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_refused_num);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_others_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_request_num); 
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_ap_unable_num); 

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_abnormal_num); 
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_others_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &disassoc_request_num);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &total_online_time);		//sta_totl_time

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	
	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	
	return reply;
}
/*nl add 20100607 b19*/
DBusMessage *asd_dbus_show_radio_new_wireless_info_bywtpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;
	
	struct asd_data *bss[L_BSS_NUM];
	unsigned int WTPID = 0;
	int i = 0, k = 0, ii = 0;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int wtp_num;

	unsigned int auth_tms = 0; 
	unsigned int repauth_tms = 0; 
	unsigned int assoc_req_num = 0; 
	unsigned int assoc_resp_num = 0; 
	unsigned int rx_mgmt_pkts = 0;
	unsigned int tx_mgmt_pkts = 0;
	unsigned int rx_ctrl_pkts = 0;
	unsigned int tx_ctrl_pkts = 0;
	unsigned int rx_data_pkts = 0;
	unsigned int tx_data_pkts = 0;
	unsigned int sta_num = 0;
	
	dbus_error_init(&err);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);

/////////////////////////////////////////////

	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	pthread_mutex_lock(&asd_g_wtp_mutex); 			//mahz add 2011.4.20
	wtp_num = Asd_Find_Wtp(WTP);
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	pthread_mutex_lock(&asd_g_sta_mutex);	
	
	{
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 

		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,&wtp_num);	

		/*for sending wtp asd infor*/
			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_ARRAY,
											  	 	DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   
													    DBUS_TYPE_UINT32_AS_STRING		//wtpid
													    DBUS_TYPE_UINT32_AS_STRING		//radio num
													    
													    DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING	//g_radio_id
													
															DBUS_TYPE_UINT32_AS_STRING	// a1
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING	// a5
															
															DBUS_TYPE_UINT32_AS_STRING	//a6
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING  //a10
															
															DBUS_TYPE_UINT32_AS_STRING //sta num
														DBUS_STRUCT_END_CHAR_AS_STRING
											
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_array);

		for(ii=0;ii<wtp_num;ii++){
			DBusMessageIter iter_sub_struct;
			DBusMessageIter iter_sub_array;
			unsigned int radio[L_RADIO_NUM];
			unsigned int radionum = 0;
			WTPID = WTP[ii]->WTPID;
			
			for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
				struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
				if((interfaces->iface[i] == NULL)||(interfaces->iface[i]->bss == NULL))
					continue;
				else {
					radio[radionum]=i;
					radionum++;
				}
			}

			dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &WTP[ii]->WTPID); 

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &radionum); 
			
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	//g_radio_id
													
													DBUS_TYPE_UINT32_AS_STRING	// a1
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	// a5
													
													DBUS_TYPE_UINT32_AS_STRING	//a6
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING  //a10
													
													DBUS_TYPE_UINT32_AS_STRING //sta num
													
											   DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			
			for(k=0; k<radionum; k++){
				unsigned int num = 0;
				num = ASD_SEARCH_RADIO_STA(radio[k], bss);	

				auth_tms = 0; 
	 		 	repauth_tms = 0; 
				assoc_req_num = 0; 
				assoc_resp_num = 0; 
				rx_mgmt_pkts = 0;
				tx_mgmt_pkts = 0;
				rx_ctrl_pkts = 0;
				tx_ctrl_pkts = 0;
				rx_data_pkts = 0;
				tx_data_pkts = 0;
				sta_num = 0;
				for(i=0; i<num; i++) {
					if (bss[i]->info == NULL) {
						continue;
					}
					GetBssDataPkts(bss[i]);
					auth_tms += bss[i]->usr_auth_tms;
					repauth_tms += bss[i]->ac_rspauth_tms;
					
					assoc_req_num += bss[i]->assoc_req; 
					assoc_resp_num += bss[i]->assoc_resp; 
					rx_mgmt_pkts += bss[i]->info->rx_mgmt_pkts;
					tx_mgmt_pkts += bss[i]->info->tx_mgmt_pkts;
					rx_ctrl_pkts += bss[i]->info->rx_ctrl_pkts;
					tx_ctrl_pkts += bss[i]->info->tx_ctrl_pkts;
					rx_data_pkts += bss[i]->info->rx_data_pkts;
					tx_data_pkts += bss[i]->info->tx_data_pkts;
						 sta_num += bss[i]->num_sta;
					
				}

				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(radio[k]));
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&auth_tms);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&repauth_tms);

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&assoc_req_num);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&assoc_req_num);
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&rx_mgmt_pkts);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&tx_mgmt_pkts);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&rx_ctrl_pkts);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&tx_ctrl_pkts);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&rx_data_pkts);
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&tx_data_pkts);

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&sta_num);

				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		pthread_mutex_unlock(&asd_g_sta_mutex);	
		pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20

		if(WTP != NULL){
			free(WTP);
			WTP = NULL;
		}
			
		return reply;
}


/*for mib  table dot11SecurityMechTable :for showing security  config information  by nl 20100617 b21*/
DBusMessage *asd_dbus_show_security_config_info_by_wtp_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_struct;
	DBusMessageIter iter_sub_array;
	
	DBusError err;		
	dbus_error_init(&err);
	
	int i = 0, j = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	unsigned int wlan_num = 0;
	unsigned int wtp_num;
	char *secret = " ";
	unsigned char security_id;
	
	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	

	{
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING		//wtp id
												DBUS_TYPE_BYTE_AS_STRING		//mac0
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING		//mac5
												DBUS_TYPE_UINT32_AS_STRING		//wlan num

													DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING		//wlanid
														DBUS_TYPE_BYTE_AS_STRING		//security id
														DBUS_TYPE_UINT32_AS_STRING		//security type
														DBUS_TYPE_UINT32_AS_STRING		//encryption type
														DBUS_TYPE_UINT32_AS_STRING		//key input type
														DBUS_TYPE_STRING_AS_STRING		//SecurityKey
														DBUS_TYPE_STRING_AS_STRING		//name
														
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array);
		
		for(i=0;i<wtp_num;i++){
			WID_WLAN	*WLAN[WLAN_NUM] = {NULL};
			unsigned int wlan_count[WLAN_NUM]={0};
			wlan_num = ASD_FIND_WLAN_BY_WTPID(WTP[i]->WTPID,wlan_count,WLAN);

			dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_struct);

			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32,
												&WTP[i]->WTPID);
			
			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_BYTE,
												&WTP[i]->WTPMAC[0]);
			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_BYTE,
												&WTP[i]->WTPMAC[1]);
			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_BYTE,
												&WTP[i]->WTPMAC[2]);
			
			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_BYTE,
												&WTP[i]->WTPMAC[3]);
			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_BYTE,
												&WTP[i]->WTPMAC[4]);
			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_BYTE,
												&WTP[i]->WTPMAC[5]);

			dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32,
												&wlan_num);

			dbus_message_iter_open_container (&iter_struct,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING		//wlanid
										DBUS_TYPE_BYTE_AS_STRING		//security id
										DBUS_TYPE_UINT32_AS_STRING		//security type
										DBUS_TYPE_UINT32_AS_STRING		//encryption type
										DBUS_TYPE_UINT32_AS_STRING		//key input type
										DBUS_TYPE_STRING_AS_STRING		//SecurityKey
										DBUS_TYPE_STRING_AS_STRING		//name
										
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_sub_array);

			for(j=0;j<wlan_num;j++){
				security_id = WLAN[j]->SecurityID;
				dbus_message_iter_open_container (&iter_sub_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_sub_struct);


				if((security_id != 0) &&(ASD_SECURITY[security_id]!=NULL)){
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
													 &(WLAN[j]->WlanID));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
													 &(security_id));

					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(ASD_SECURITY[security_id]->securityType));

					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(ASD_SECURITY[security_id]->encryptionType));


					if(ASD_SECURITY[security_id]->keyInputType!=1&&ASD_SECURITY[security_id]->keyInputType!=2){
						ASD_SECURITY[security_id]->keyInputType=0;
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"key input type %d \n",ASD_SECURITY[security_id]->keyInputType);
					}
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(ASD_SECURITY[security_id]->keyInputType));

					if(ASD_SECURITY[security_id]->SecurityKey == NULL){
						ASD_SECURITY[security_id]->SecurityKey = secret;
					}

					if(ASD_SECURITY[security_id]->name == NULL){
						ASD_SECURITY[security_id]->SecurityKey = secret;
					}
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_STRING,
													 &(ASD_SECURITY[security_id]->SecurityKey));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_STRING,
													 &(ASD_SECURITY[security_id]->name));

					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);

					if(strcmp(ASD_SECURITY[security_id]->SecurityKey, " ") == 0){
						ASD_SECURITY[security_id]->SecurityKey = NULL;
					}
					
					if(strcmp(ASD_SECURITY[security_id]->name, " ") == 0){
						ASD_SECURITY[security_id]->name = NULL;
					}
				}
			}
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_wtp_mutex);		//mahz add 2011.4.20
	if(WTP != NULL){
		free(WTP);
		WTP = NULL;
	}
		
	return reply;
}

/*nl add for table dot11ConjunctionTable & summary table 1:showing conjunction infor for mib 20100618 b22*/
DBusMessage *asd_dbus_show_conjunction_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned int wtp_num = 0;
	
	int i = 0, j = 0, k =0;
	int ret = ASD_DBUS_SUCCESS;
	
	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	wtp_num = Asd_Find_Wtp(WTP);
	
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	
	
	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	//if(wtp_num != 0)
	{
		dbus_message_iter_open_container (&iter,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//wtpid
															
															DBUS_TYPE_BYTE_AS_STRING		//mac0
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING		//mac5
															
															DBUS_TYPE_UINT64_AS_STRING		// a1a2 total past online time
															DBUS_TYPE_UINT32_AS_STRING		//z1
															
															DBUS_TYPE_UINT32_AS_STRING		//e1
															DBUS_TYPE_UINT32_AS_STRING		//e2
															DBUS_TYPE_UINT32_AS_STRING		//e3
															
 															DBUS_TYPE_UINT32_AS_STRING		//b1
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b5

															DBUS_TYPE_UINT32_AS_STRING		//b6
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b10
															DBUS_TYPE_UINT32_AS_STRING		//assoc success times
															
															DBUS_TYPE_UINT32_AS_STRING		//b11
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b15
															
															DBUS_TYPE_UINT32_AS_STRING		//b16
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b20
															
															DBUS_TYPE_UINT32_AS_STRING		//b21
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b25
															
															DBUS_TYPE_UINT32_AS_STRING		//b26
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b30
															
															DBUS_TYPE_UINT32_AS_STRING		//b31
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//b35

															DBUS_TYPE_UINT32_AS_STRING		//c1
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		//c4
															
															//DBUS_TYPE_UINT32_AS_STRING		//c5
															//DBUS_TYPE_UINT32_AS_STRING		
															DBUS_TYPE_UINT32_AS_STRING		
 															DBUS_TYPE_UINT32_AS_STRING		//c8
															
		
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING		
															DBUS_TYPE_UINT32_AS_STRING		
															DBUS_TYPE_UINT32_AS_STRING
														//	DBUS_TYPE_UINT32_AS_STRING
		
													   //mahz add 2011.5.5
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING

														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING

														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT64_AS_STRING
													   //
													   DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_array);
		for(k=0;k<wtp_num;k++){
			int bss_num = 0;
			
			bss_num = ASD_SEARCH_WTP_STA(WTP[k]->WTPID, bss);	
			
			unsigned int acc_tms=0;								//e1
			unsigned int auth_tms=0;							//e2
			unsigned int repauth_tms=0;							//e3
					
			unsigned int auth_success_num = 0; 					//b1
			unsigned int auth_fail_num = 0; 
			unsigned int auth_invalid_num = 0; 
			unsigned int auth_timeout_num = 0; 
			unsigned int auth_refused_num = 0; 					//b5
			
			unsigned int auth_others_num = 0; 					//b6
			unsigned int assoc_req_num = 0; 
			unsigned int assoc_resp_num = 0; 
			unsigned int assoc_invalid_num = 0; 
			unsigned int assoc_timeout_num = 0; 				//b10

			unsigned int assoc_success_num = 0;				//assoc success times
			
			unsigned int assoc_refused_num = 0; 				//b11
			unsigned int assoc_others_num = 0; 
			unsigned int reassoc_request_num = 0; 
			unsigned int reassoc_success_num = 0; 
			unsigned int reassoc_invalid_num = 0; 				//b15
			
			unsigned int reassoc_timeout_num = 0; 				//b16
			unsigned int reassoc_refused_num = 0; 
			unsigned int reassoc_others_num = 0; 
			unsigned int identify_request_num = 0; 
			unsigned int identify_success_num = 0; 				//b20
			
			unsigned int abort_key_error_num = 0; 				//b21
			unsigned int abort_invalid_num = 0; 
			unsigned int abort_timeout_num = 0; 
			unsigned int abort_refused_num = 0; 
			unsigned int abort_others_num = 0; 					//b25
			
			unsigned int deauth_request_num = 0; 				//b26
			unsigned int deauth_user_leave_num = 0; 
			unsigned int deauth_ap_unable_num = 0; 
			unsigned int deauth_abnormal_num = 0; 
			unsigned int deauth_others_num = 0; 				//b30
			
			unsigned int disassoc_request_num = 0; 				//b31
			unsigned int disassoc_user_leave_num = 0; 
			unsigned int disassoc_ap_unable_num = 0; 
			unsigned int disassoc_abnormal_num = 0; 
			unsigned int disassoc_others_num = 0; 				//b35

			unsigned int rx_mgmt_pkts = 0;						//c1
			unsigned int tx_mgmt_pkts = 0;
			unsigned int rx_ctrl_pkts = 0;
			unsigned int tx_ctrl_pkts = 0;						//c4
			
		//	unsigned int rx_data_pkts = 0;						//c5
		//	unsigned int tx_data_pkts = 0;
			unsigned int rx_auth_pkts = 0;
			unsigned int tx_auth_pkts = 0;						//c8

			unsigned long long wtp_total_past_online_time = 0;	//	xm0703		//a1a2
			unsigned int num_assoc_failure = 0; //	xm0703						//z1

			unsigned int rx_assoc_norate = 0; /*因终端不支持基本速率集要求的所有速率而关联失败的总次数*/
			unsigned int rx_assoc_capmismatch = 0;	  /*由不在802.11标准制定范围内的原因而关联失败的总次数*/
			unsigned int assoc_invaild = 0;   /*未知原因而导致关联失败的总次数*/
			unsigned int reassoc_deny = 0;	  /*由于之前的关联无法识别与转移而导致重新关联失败的总次数*/
			rx_assoc_norate = WTP[k]->wifi_extension_info.rx_assoc_norate;
			rx_assoc_capmismatch = WTP[k]->wifi_extension_info.rx_assoc_capmismatch;
			assoc_invaild = WTP[k]->wifi_extension_info.assoc_invaild;
			reassoc_deny = WTP[k]->wifi_extension_info.reassoc_deny;

			
			acc_tms = GetAccessTimes(WTP[k]->WTPID);
			auth_tms = GetUserAuthTimes(WTP[k]->WTPID);
			repauth_tms = GetWTPRspAuthTimes(WTP[k]->WTPID);
			bss_num = ASD_SEARCH_WTP_STA(WTP[k]->WTPID, bss);	

			for(j = WTP[k]->WTPID*L_RADIO_NUM; j < (WTP[k]->WTPID+1)*L_RADIO_NUM; j++) {
				if(interfaces->iface[j] != NULL) {
					acc_tms += interfaces->iface[j]->access_times;
					auth_tms += interfaces->iface[j]->auth_times;
					wtp_total_past_online_time += interfaces->iface[j]->total_past_online_time;
				}
			}
					
			for(i=0; i<bss_num; i++){
				num_assoc_failure+=bss[i]->num_assoc_failure;
				
				if (bss[i]->info == NULL) {
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss[%d]==NULL\n",i);
					continue;
				}
				GetBssDataPkts(bss[i]);
				wtp_total_past_online_time+=bss[i]->total_past_online_time;			//a1
				wtp_total_past_online_time+=bss[i]->total_present_online_time;		//a2
				
				auth_success_num += bss[i]->auth_success; 						//b1
				auth_fail_num += bss[i]->auth_fail; 
				auth_invalid_num += bss[i]->info->auth_invalid; 
				auth_timeout_num += bss[i]->info->auth_timeout; 
				auth_refused_num += bss[i]->info->auth_refused; 				//b5
				
				auth_others_num += bss[i]->info->auth_others; 					//b6
				assoc_req_num += bss[i]->assoc_req; 
				assoc_resp_num += bss[i]->assoc_resp; 
				assoc_invalid_num += bss[i]->info->assoc_invalid; 
				assoc_timeout_num += bss[i]->info->assoc_timeout; 				//b10

				assoc_success_num += bss[i]->assoc_success;					//xiaodawei add assoc success times, 20110427
				
				assoc_refused_num += bss[i]->info->assoc_refused; 				//b11
				assoc_others_num += bss[i]->info->assoc_others; 
				reassoc_request_num += bss[i]->num_reassoc; 
				reassoc_success_num += bss[i]->reassoc_success; 
				reassoc_invalid_num += bss[i]->info->reassoc_invalid; 			//b15
				
				reassoc_timeout_num += bss[i]->info->reassoc_timeout; 			//b16
				reassoc_refused_num += bss[i]->info->reassoc_refused; 
				reassoc_others_num += bss[i]->info->reassoc_others; 
				identify_request_num += bss[i]->info->identify_request; 
				identify_success_num += bss[i]->info->identify_success; 		//b20
				
				abort_key_error_num += bss[i]->info->abort_key_error; 			//b21
				abort_invalid_num += bss[i]->info->abort_invalid; 
				abort_timeout_num += bss[i]->info->abort_timeout; 
				abort_refused_num += bss[i]->info->abort_refused; 
				abort_others_num += bss[i]->info->abort_others; 				//b25
				
				deauth_request_num += bss[i]->info->deauth_request; 			//b26
				deauth_user_leave_num += bss[i]->info->deauth_user_leave; 
				deauth_ap_unable_num += bss[i]->info->deauth_ap_unable; 
				deauth_abnormal_num += bss[i]->info->deauth_abnormal; 
				deauth_others_num += bss[i]->info->deauth_others; 				//b30
				
				disassoc_request_num += bss[i]->info->disassoc_request; 		//b31
				disassoc_user_leave_num += bss[i]->info->disassoc_user_leave; 
				disassoc_ap_unable_num += bss[i]->info->disassoc_ap_unable; 
				disassoc_abnormal_num += bss[i]->info->disassoc_abnormal; 
				disassoc_others_num += bss[i]->info->disassoc_others; 			//b35

				rx_mgmt_pkts += bss[i]->info->rx_mgmt_pkts;						//c1
				tx_mgmt_pkts += bss[i]->info->tx_mgmt_pkts;
				rx_ctrl_pkts += bss[i]->info->rx_ctrl_pkts;
				tx_ctrl_pkts += bss[i]->info->tx_ctrl_pkts;						//c4
				
				/*rx_data_pkts += bss[i]->info->rx_data_pkts;
				tx_data_pkts += bss[i]->info->tx_data_pkts;*/
				rx_auth_pkts += bss[i]->info->rx_auth_pkts;
				tx_auth_pkts += bss[i]->info->tx_auth_pkts;						//c8
					
			}
			
			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											 &(WTP[k]->WTPID));

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[0]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[1]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[2]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[3]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[4]));
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(WTP[k]->WTPMAC[5]));

			dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT64,
										 &wtp_total_past_online_time);	//	a1a2
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &num_assoc_failure);	//z1
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &acc_tms);						//e1
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_tms);					//e2
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &repauth_tms);					//e3

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_success_num);			//b1
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_fail_num);				//b2
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_invalid_num);			//b3
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_timeout_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_refused_num);			//b5

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &auth_others_num);				//b6
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_req_num);				//b7
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_resp_num);				//b8
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_invalid_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_timeout_num);			//b10

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_success_num);			//assoc success times 
											 
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_refused_num);			//b11
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_others_num);			//b12
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_request_num);			//b13
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_success_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_invalid_num);			//b15
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_timeout_num);			//b16
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_refused_num);			//b17
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_others_num);			//b18
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &identify_request_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &identify_success_num);		//b20
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &abort_key_error_num);			//b21
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &abort_invalid_num);			//b22
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &abort_timeout_num);			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &abort_refused_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &abort_others_num);			//b25
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_request_num);			//b26
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_user_leave_num);		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_ap_unable_num);		
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_abnormal_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &deauth_others_num);			//b30
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &disassoc_request_num);		//b31
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &disassoc_user_leave_num);		//b32
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &disassoc_ap_unable_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &disassoc_abnormal_num);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &disassoc_others_num);			//b35

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &rx_mgmt_pkts);				//c1
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &tx_mgmt_pkts);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &rx_ctrl_pkts);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &tx_ctrl_pkts);				//c4
			
		/*	dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &rx_data_pkts);				//c5
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &tx_data_pkts);*/
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &rx_auth_pkts);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &tx_auth_pkts);				//c8
			

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &rx_assoc_norate);				
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &rx_assoc_capmismatch);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &assoc_invaild);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &reassoc_deny);			
			//mahz add 2011.5.5
			if(ASD_WTP_AP_HISTORY[WTP[k]->WTPID] != NULL){
				asd_printf(ASD_DBUS,MSG_DEBUG,"11111111\n");		//for test
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->usr_auth_tms);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->ac_rspauth_tms);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->auth_fail);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->auth_success);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->num_assoc);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->num_reassoc);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->num_assoc_failure);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->num_reassoc_failure);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->assoc_success);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->reassoc_success);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->assoc_req);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->assoc_resp);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT64, &ASD_WTP_AP_HISTORY[WTP[k]->WTPID]->total_ap_flow_record);
			}
			else{
				unsigned int val = 0; 
				unsigned long long value = 0;
				asd_printf(ASD_DBUS,MSG_DEBUG,"22222222\n");		//for test
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &val);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT64, &value);
			}
			//
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_sta_mutex);	
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	
	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	
	return reply;
}
/*nl add for showing wapi wlan config for mib 20100521 bb1*/
DBusMessage *asd_dbus_show_wlan_wapi_basic_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	WID_WLAN    *WAPI_WLAN[WLAN_NUM];
	unsigned char wlan_num = 0;
	unsigned char wlan_id;
	unsigned char wapi_wlan_num = 0;
	unsigned int is_installed_ae_cert ;
	char *ip = "0.0.0.0";
	unsigned char wapi_security_id = 0;

	pthread_mutex_lock(&asd_g_wlan_mutex);
	wapi_wlan_num = FindWapiWlan(WAPI_WLAN,&wlan_num);

	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;
	
	else if(wapi_wlan_num ==0)
		ret = ASD_WAPI_WLAN_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	unsigned int wapi_wlan_num_int = wapi_wlan_num;
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wapi_wlan_num_int); 	

	{
		dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_BYTE_AS_STRING		//wlan id
												   		DBUS_TYPE_BYTE_AS_STRING		//security id
														DBUS_TYPE_UINT32_AS_STRING		//isinstalled
														DBUS_TYPE_STRING_AS_STRING		//as_ip
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);

		for(i=0;i<wapi_wlan_num;i++){
			wlan_id = WAPI_WLAN[i]->WlanID;
			wapi_security_id = WAPI_WLAN[i]->SecurityID;

			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wlan_id));
			
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wapi_security_id));

			/*dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[security_id]->securityType));*/

			if(ASD_SECURITY[wapi_security_id]->wapi_as.ae_cert_path== NULL){	
				is_installed_ae_cert = 0;
			}
			else{
				is_installed_ae_cert = 1;
			}

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_UINT32,
											 &(is_installed_ae_cert));

			if(ASD_SECURITY[wapi_security_id]->wapi_as.as_ip== NULL){		
				ASD_SECURITY[wapi_security_id]->wapi_as.as_ip= ip;
			}

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_STRING,
											 &(ASD_SECURITY[wapi_security_id]->wapi_as.as_ip));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
			if(strcmp(ASD_SECURITY[wapi_security_id]->wapi_as.as_ip, "0.0.0.0") == 0){
				ASD_SECURITY[wapi_security_id]->wapi_as.as_ip= NULL;
			}
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	return reply;
}
/*nl add for showing wapi wlan extend config information for mib 20100601 bb2*/
DBusMessage *asd_dbus_show_wlan_wapi_extend_config_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	WID_WLAN    *WAPI_WLAN[WLAN_NUM];
	unsigned char wlan_num = 0;
	unsigned char wlan_id;
	unsigned char wapi_wlan_num = 0;
	unsigned char wapi_security_id = 0;
	char *secret = " ";
	
	pthread_mutex_lock(&asd_g_wlan_mutex);
	wapi_wlan_num = FindWapiWlan(WAPI_WLAN,&wlan_num);

	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;
	
	else if(wapi_wlan_num ==0)
		ret = ASD_WAPI_WLAN_NOT_EXIST;

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	unsigned int wapi_wlan_num_int = wapi_wlan_num;
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wapi_wlan_num_int); 	

	{
		dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_BYTE_AS_STRING		//wlan id
												   		DBUS_TYPE_BYTE_AS_STRING		//security id
														DBUS_TYPE_UINT32_AS_STRING		//securityType 

														DBUS_TYPE_UINT32_AS_STRING		//CertificateUpdateCount
														DBUS_TYPE_UINT32_AS_STRING		//MulticastUpdateCount
														DBUS_TYPE_UINT32_AS_STRING		//UnicastUpdateCount
														DBUS_TYPE_UINT32_AS_STRING		//BKLifetime
														DBUS_TYPE_UINT32_AS_STRING		//BKReauthThreshold
														DBUS_TYPE_UINT32_AS_STRING		//SATimeout

														DBUS_TYPE_BYTE_AS_STRING		//WapiPreauth
														DBUS_TYPE_BYTE_AS_STRING		//MulticaseRekeyStrict
														//DBUS_TYPE_BYTE_AS_STRING		//UnicastCipherEnabled
														DBUS_TYPE_BYTE_AS_STRING		//AuthenticationSuiteEnable
														
														DBUS_TYPE_BYTE_AS_STRING		//MulticastCipher[0]
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING		//MulticastCipher[3]

														DBUS_TYPE_BYTE_AS_STRING		//wapi_ucast_rekey_method
												   		DBUS_TYPE_UINT32_AS_STRING		//wapi_ucast_rekey_para_t	
														DBUS_TYPE_UINT32_AS_STRING		//wapi_ucast_rekey_para_p	
														DBUS_TYPE_BYTE_AS_STRING		//wapi_mcast_rekey_method
												   		DBUS_TYPE_UINT32_AS_STRING		//wapi_mcast_rekey_para_t	
														DBUS_TYPE_UINT32_AS_STRING		//wapi_mcast_rekey_para_p
														DBUS_TYPE_STRING_AS_STRING		//key

														DBUS_TYPE_BYTE_AS_STRING		//last akm
														
														DBUS_TYPE_BYTE_AS_STRING		//last bkid[16]
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);
		
		for(i=0;i<wapi_wlan_num;i++){
			int j = 0 ;
			unsigned char akm = 0;
			unsigned char bkid[16] = {0};
			wlan_id = WAPI_WLAN[i]->WlanID;
			wapi_security_id = WAPI_WLAN[i]->SecurityID;
			if(ASD_SECURITY[wapi_security_id]->securityType == WAPI_AUTH){
				akm = 1;
			}else if(ASD_SECURITY[wapi_security_id]->securityType == WAPI_PSK){
				akm = 2;
			}
			memcpy(bkid,ASD_WLAN[wlan_id]->bkid,16);
			
			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wlan_id));
			
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wapi_security_id));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->securityType));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->CertificateUpdateCount));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->MulticastUpdateCount));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->UnicastUpdateCount));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->BKLifetime));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->BKReauthThreshold));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->SATimeout));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->WapiPreauth));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->MulticaseRekeyStrict));
			/*dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->UnicastCipherEnabled));*/
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->AuthenticationSuiteEnable));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->MulticastCipher[0]));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->MulticastCipher[1]));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->MulticastCipher[2]));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_config->MulticastCipher[3]));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_ucast_rekey_method));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_ucast_rekey_para_t));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_ucast_rekey_para_p));
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &(ASD_SECURITY[wapi_security_id]->wapi_mcast_rekey_method));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_mcast_rekey_para_t));
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->wapi_mcast_rekey_para_p));
			
			if(ASD_SECURITY[wapi_security_id]->SecurityKey == NULL){
				ASD_SECURITY[wapi_security_id]->SecurityKey = secret;
			}

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_STRING,
											 &(ASD_SECURITY[wapi_security_id]->SecurityKey));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_BYTE,
											 &akm);
			for(j=0; j<16; j++){
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &(bkid[j]));
			}

			dbus_message_iter_close_container (&iter_array, &iter_struct);

			if(strcmp(ASD_SECURITY[wapi_security_id]->SecurityKey, " ") == 0){
				ASD_SECURITY[wapi_security_id]->SecurityKey = NULL;
			}
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	return reply;
}

/*nl add for showing unicast  info for mib 20100525 bb3*/
DBusMessage *asd_dbus_show_wlan_unicast_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	
	WID_WLAN    *WAPI_WLAN[WLAN_NUM];
	unsigned char wlan_num = 0;
	unsigned char wlan_id;
	unsigned char wapi_wlan_num = 0;
	unsigned char wapi_security_id = 0;
	
	pthread_mutex_lock(&asd_g_wlan_mutex);
	wapi_wlan_num = FindWapiWlan(WAPI_WLAN,&wlan_num);
	
	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;
	
	else if(wapi_wlan_num ==0)
		ret = ASD_WAPI_WLAN_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	unsigned int wapi_wlan_num_int = wapi_wlan_num;
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wapi_wlan_num_int); 	

	{
		dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_BYTE_AS_STRING		//wlan id
												   		DBUS_TYPE_BYTE_AS_STRING		//security id
														DBUS_TYPE_UINT32_AS_STRING		//isinstalled
														DBUS_TYPE_BYTE_AS_STRING		//as_ip
														
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);

		for(i=0;i<wapi_wlan_num;i++){
			wlan_id = WAPI_WLAN[i]->WlanID;
			wapi_security_id = WAPI_WLAN[i]->SecurityID;

			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wlan_id));
						
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wapi_security_id));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->securityType));

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											&(ASD_SECURITY[wapi_security_id]->wapi_config->UnicastCipherEnabled));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
		
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	return reply;
}

/*nl add for showing wapi wlan statsperformance for mib 20100529 bb4*/
DBusMessage *asd_dbus_show_wlan_wapi_stats_performance_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_sub_array;
	DBusMessageIter iter_sub_sub_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	int i = 0, j = 0, k =0;
	int ret = ASD_DBUS_SUCCESS;
	struct asd_stainfo *stainfo =NULL;
	
	WID_WLAN    *WAPI_WLAN[WLAN_NUM];
	unsigned char wlan_num = 0;
	unsigned char wlan_id;
	unsigned char wapi_wlan_num = 0;
	unsigned char wapi_security_id = 0;
	int bss_num ;
	
	struct asd_data **bss;
	bss = malloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	struct sta_info *sta =NULL;

	pthread_mutex_lock(&asd_g_wlan_mutex);
	wapi_wlan_num = FindWapiWlan(WAPI_WLAN,&wlan_num);

	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;

	else if(wapi_wlan_num ==0)
		ret = ASD_WAPI_WLAN_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	unsigned int wapi_wlan_num_int = (unsigned int) wapi_wlan_num;
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wapi_wlan_num_int); 

	pthread_mutex_lock(&asd_g_sta_mutex);		//mahz add 2011.4.20
	{
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										   		DBUS_TYPE_BYTE_AS_STRING		//wlan id
										   		DBUS_TYPE_BYTE_AS_STRING		//security id
												DBUS_TYPE_UINT32_AS_STRING		//security type
												DBUS_TYPE_UINT32_AS_STRING		//bss_num
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING		//bssid
														DBUS_TYPE_BYTE_AS_STRING		//wlanid
														DBUS_TYPE_UINT32_AS_STRING		//num_sta
														DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															  DBUS_TYPE_UINT32_AS_STRING		//sta_seq
															  DBUS_TYPE_BYTE_AS_STRING	//mac 0
															  DBUS_TYPE_BYTE_AS_STRING
															  DBUS_TYPE_BYTE_AS_STRING
															  DBUS_TYPE_BYTE_AS_STRING
															  DBUS_TYPE_BYTE_AS_STRING
															  DBUS_TYPE_BYTE_AS_STRING //mac 5
															  DBUS_TYPE_UINT32_AS_STRING //wapi_version
															  DBUS_TYPE_BYTE_AS_STRING	   //controlled_port_status
															  DBUS_TYPE_BYTE_AS_STRING				//selected_unicast_cipher0
															  DBUS_TYPE_BYTE_AS_STRING
															  DBUS_TYPE_BYTE_AS_STRING
															  DBUS_TYPE_BYTE_AS_STRING				//selected_unicast_cipher3
															  DBUS_TYPE_UINT32_AS_STRING				//wpi_replay_counters
															  DBUS_TYPE_UINT32_AS_STRING				//wpi_decryptable_errors
															  DBUS_TYPE_UINT32_AS_STRING			//wpi_mic_errors
															  DBUS_TYPE_UINT32_AS_STRING				//wai_sign_errors
															  DBUS_TYPE_UINT32_AS_STRING				//wai_hmac_errors
															  DBUS_TYPE_UINT32_AS_STRING			//wai_auth_res_fail
															  DBUS_TYPE_UINT32_AS_STRING			//wai_discard
															  DBUS_TYPE_UINT32_AS_STRING			//wai_timeout
															  DBUS_TYPE_UINT32_AS_STRING			//wai_format_errors
															  DBUS_TYPE_UINT32_AS_STRING			//wai_cert_handshake_fail
															  DBUS_TYPE_UINT32_AS_STRING			//wai_unicast_handshake_fail
															  DBUS_TYPE_UINT32_AS_STRING			//wai_multi_handshake_fail
														DBUS_STRUCT_END_CHAR_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);

		for(i=0;i<wapi_wlan_num;i++){
			wlan_id = WAPI_WLAN[i]->WlanID;
			wapi_security_id = WAPI_WLAN[i]->SecurityID;
			bss_num = ASD_SEARCH_WLAN_STA(wlan_id, bss);

			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wlan_id));
			
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wapi_security_id));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->securityType));
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(bss_num));

			dbus_message_iter_open_container (&iter_struct,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING		//bssid
										DBUS_TYPE_BYTE_AS_STRING		//wlanid
										DBUS_TYPE_UINT32_AS_STRING		//num_sta
											DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING	   //sta_seq
													   DBUS_TYPE_BYTE_AS_STRING	//mac 0
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING //mac 5
												      DBUS_TYPE_UINT32_AS_STRING	//wapi_version
													  DBUS_TYPE_BYTE_AS_STRING		//controlled_port_status
													  DBUS_TYPE_BYTE_AS_STRING				//selected_unicast_cipher0
													  DBUS_TYPE_BYTE_AS_STRING
													  DBUS_TYPE_BYTE_AS_STRING
													  DBUS_TYPE_BYTE_AS_STRING				//selected_unicast_cipher3
													  DBUS_TYPE_UINT32_AS_STRING				//wpi_replay_counters
													  DBUS_TYPE_UINT32_AS_STRING				//wpi_decryptable_errors
													  DBUS_TYPE_UINT32_AS_STRING			//wpi_mic_errors
													  DBUS_TYPE_UINT32_AS_STRING				//wai_sign_errors
													  DBUS_TYPE_UINT32_AS_STRING				//wai_hmac_errors
													  DBUS_TYPE_UINT32_AS_STRING			//wai_auth_res_fail
													  DBUS_TYPE_UINT32_AS_STRING			//wai_discard
													  DBUS_TYPE_UINT32_AS_STRING			//wai_timeout
													  DBUS_TYPE_UINT32_AS_STRING			//wai_format_errors
													  DBUS_TYPE_UINT32_AS_STRING			//wai_cert_handshake_fail
													  DBUS_TYPE_UINT32_AS_STRING			//wai_unicast_handshake_fail
													  DBUS_TYPE_UINT32_AS_STRING			//wai_multi_handshake_fail
												DBUS_STRUCT_END_CHAR_AS_STRING
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_sub_array);
			
			for(j = 0; j < bss_num; j++){	
				unsigned char bss_index;
				bss_index = (unsigned char)j;
				dbus_message_iter_open_container (&iter_sub_array,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_sub_struct);

				dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
														&bss_index);
				
				dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
														&bss[j]->WlanID);
		
				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_UINT32,
														&(bss[j]->num_sta));

				dbus_message_iter_open_container (&iter_sub_struct,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													  		 DBUS_TYPE_UINT32_AS_STRING		//sta_seq
															   DBUS_TYPE_BYTE_AS_STRING		//mac0
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING		//mac5
															   DBUS_TYPE_UINT32_AS_STRING	//wapi_version
															   DBUS_TYPE_BYTE_AS_STRING		//controlled_port_status
																  DBUS_TYPE_BYTE_AS_STRING				//selected_unicast_cipher0
																  DBUS_TYPE_BYTE_AS_STRING
																  DBUS_TYPE_BYTE_AS_STRING
																  DBUS_TYPE_BYTE_AS_STRING				//selected_unicast_cipher3
																  DBUS_TYPE_UINT32_AS_STRING				//wpi_replay_counters
																  DBUS_TYPE_UINT32_AS_STRING				//wpi_decryptable_errors
																  DBUS_TYPE_UINT32_AS_STRING			//wpi_mic_errors
																  DBUS_TYPE_UINT32_AS_STRING				//wai_sign_errors
																  DBUS_TYPE_UINT32_AS_STRING				//wai_hmac_errors
																  DBUS_TYPE_UINT32_AS_STRING			//wai_auth_res_fail
																  DBUS_TYPE_UINT32_AS_STRING			//wai_discard
																  DBUS_TYPE_UINT32_AS_STRING			//wai_timeout
																  DBUS_TYPE_UINT32_AS_STRING			//wai_format_errors
																  DBUS_TYPE_UINT32_AS_STRING			//wai_cert_handshake_fail
																  DBUS_TYPE_UINT32_AS_STRING			//wai_unicast_handshake_fail
																  DBUS_TYPE_UINT32_AS_STRING			//wai_multi_handshake_fail
													  DBUS_STRUCT_END_CHAR_AS_STRING,
												  &iter_sub_sub_array);

				sta = bss[j]->sta_list;
				for(k = 0; (k < bss[j]->num_sta)&&(sta!=NULL); k++){
					//pthread_mutex_lock(&asd_g_sta_mutex);				//mahz modify 2011.4.20
					stainfo = ASD_SEARCH_STA(sta->addr);

					if(stainfo != NULL&&stainfo->sta!=NULL){
						wapi_stats_Entry	*wapi_sta_mib = NULL;
						wapi_sta_mib = &stainfo->sta->wapi_sta_info.wapi_mib_stats;
						
						dbus_message_iter_open_container (&iter_sub_sub_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_sub_sub_struct);

						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(k));
						
						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(sta->addr[0]));
						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(sta->addr[1]));
						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(sta->addr[2]));

						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(sta->addr[3]));
						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(sta->addr[4]));
						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(sta->addr[5]));
														  
						dbus_message_iter_append_basic(&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wapi_version));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(wapi_sta_mib->controlled_port_status));

						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(wapi_sta_mib->selected_unicast_cipher[0]));//a0
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(wapi_sta_mib->selected_unicast_cipher[1]));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(wapi_sta_mib->selected_unicast_cipher[2]));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_BYTE,
														  &(wapi_sta_mib->selected_unicast_cipher[3]));//a3

						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wpi_replay_counters));	//b1
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wpi_decryptable_errors));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wpi_mic_errors));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_sign_errors));		//b4
														  
						dbus_message_iter_append_basic (&iter_sub_sub_struct,					//c1
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_hmac_errors));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_auth_res_fail));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_discard));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_timeout));		//c4
															  
						dbus_message_iter_append_basic (&iter_sub_sub_struct,							//d1
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_format_errors));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_cert_handshake_fail));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_unicast_handshake_fail));
						dbus_message_iter_append_basic (&iter_sub_sub_struct,
														  DBUS_TYPE_UINT32,
														  &(wapi_sta_mib->wai_multi_handshake_fail));	//d4

						dbus_message_iter_close_container (&iter_sub_sub_array, &iter_sub_sub_struct);
						//pthread_mutex_unlock(&asd_g_sta_mutex); 		//mahz modify 2011.4.20
					}
					if(stainfo != NULL){
						stainfo->bss = NULL;
						stainfo->sta = NULL;
						free(stainfo);
						stainfo = NULL;
					}
				}
				dbus_message_iter_close_container (&iter_sub_struct, &iter_sub_sub_array);
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}		
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 		//mahz add 2011.4.20
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	if(bss){
		free(bss);
		bss = NULL;
	}
	return reply;
}

/* for showing table 28 dot11BssWAPIPerformanceStatsTable ,wapi wlan bss performance for mib 20100604 by nl  bb5*/
DBusMessage *asd_dbus_show_wlan_wapi_bss_performance_info_of_all_wtp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	DBusError err;		
	dbus_error_init(&err);
	
	int ret = ASD_DBUS_SUCCESS;
	int i = 0, j = 0;
	
	WID_WLAN    *WAPI_WLAN[WLAN_NUM];
	unsigned char wlan_num = 0;
	unsigned char wapi_wlan_num = 0;
	unsigned char wlan_id;
	unsigned char wapi_security_id = 0;
	int bss_num ;
	unsigned int WTPID;
	unsigned int Radio_G_ID ;
	unsigned char defalt_char_zero = 0;

	wapi_stats_Entry *bss_wapi_mib_stat = NULL;
	wapi_stats_Entry  MIBNULL;
	
	struct asd_data **bss = NULL;
	bss = malloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}

	pthread_mutex_lock(&asd_g_wlan_mutex);
	wapi_wlan_num = FindWapiWlan(WAPI_WLAN,&wlan_num);

	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;

	else if(wapi_wlan_num ==0)
		ret = ASD_WAPI_WLAN_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	unsigned int wapi_wlan_num_int = (unsigned int) wapi_wlan_num;
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wapi_wlan_num_int); 

	pthread_mutex_lock(&asd_g_sta_mutex);		//mahz add 2011.4.20
	{
		dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   		DBUS_TYPE_BYTE_AS_STRING		//wlan id
												   		DBUS_TYPE_BYTE_AS_STRING		//security id
														DBUS_TYPE_UINT32_AS_STRING		//security type
														DBUS_TYPE_UINT32_AS_STRING		//bss_num
															DBUS_TYPE_ARRAY_AS_STRING
																DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING		//bssid
																	DBUS_TYPE_BYTE_AS_STRING		//wlanid
																	DBUS_TYPE_UINT32_AS_STRING		//wtpid
																	DBUS_TYPE_BYTE_AS_STRING		//wtpmac0
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING		//wtpmac5
																	DBUS_TYPE_BYTE_AS_STRING		//bssid0
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING		//bssid5
																	DBUS_TYPE_UINT32_AS_STRING		//wai_sign_errors
																	DBUS_TYPE_UINT32_AS_STRING		//wai_sign_errors
																	DBUS_TYPE_UINT32_AS_STRING		//wai_auth_res_fail
																	DBUS_TYPE_UINT32_AS_STRING		//wai_discard
																	DBUS_TYPE_UINT32_AS_STRING		//wai_timeout
																	DBUS_TYPE_UINT32_AS_STRING		//wai_format_errors
																	DBUS_TYPE_UINT32_AS_STRING		//wai_cert_handshake_fail
																	DBUS_TYPE_UINT32_AS_STRING		//wai_unicast_handshake_fail
																	DBUS_TYPE_UINT32_AS_STRING		//wai_multi_handshake_fail
																	DBUS_TYPE_UINT32_AS_STRING		//wpi_mic_errors
																	DBUS_TYPE_UINT32_AS_STRING		//wpi_replay_counters
																	DBUS_TYPE_UINT32_AS_STRING		//wpi_decryptable_errors				
																DBUS_STRUCT_END_CHAR_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);

		for(i=0;i<wapi_wlan_num;i++){
			wlan_id = WAPI_WLAN[i]->WlanID;
			wapi_security_id = WAPI_WLAN[i]->SecurityID;

			dbus_message_iter_open_container (&iter_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wlan_id));
			
			dbus_message_iter_append_basic (&iter_struct,
											DBUS_TYPE_BYTE,
											 &(wapi_security_id));

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(ASD_SECURITY[wapi_security_id]->securityType));

			bss_num = ASD_SEARCH_WLAN_STA(wlan_id, bss);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &(bss_num));

			dbus_message_iter_open_container (&iter_struct,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING		//bssid
										DBUS_TYPE_BYTE_AS_STRING		//wlanid
										DBUS_TYPE_UINT32_AS_STRING		//wtpid

										DBUS_TYPE_BYTE_AS_STRING		//wtpmac0
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING		//wtpmac5
										
										DBUS_TYPE_BYTE_AS_STRING		//bssid0
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING		//bssid5

										DBUS_TYPE_UINT32_AS_STRING		//wai_sign_errors
										DBUS_TYPE_UINT32_AS_STRING		//wai_sign_errors
										DBUS_TYPE_UINT32_AS_STRING		//wai_auth_res_fail
										DBUS_TYPE_UINT32_AS_STRING		//wai_discard

										DBUS_TYPE_UINT32_AS_STRING		//wai_timeout
										DBUS_TYPE_UINT32_AS_STRING		//wai_format_errors
										DBUS_TYPE_UINT32_AS_STRING		//wai_cert_handshake_fail
										DBUS_TYPE_UINT32_AS_STRING		//wai_unicast_handshake_fail

										DBUS_TYPE_UINT32_AS_STRING		//wai_multi_handshake_fail
										DBUS_TYPE_UINT32_AS_STRING		//wpi_mic_errors
										DBUS_TYPE_UINT32_AS_STRING		//wpi_replay_counters
										DBUS_TYPE_UINT32_AS_STRING		//wpi_decryptable_errors
											
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_sub_array);
			
			for(j = 0; j < bss_num; j++){	
				if ((bss[j]->wapi_wasd == NULL)||(bss[j]->wapi_wasd->vap_user == NULL)||(bss[j]->wapi_wasd->vap_user->wapid == NULL)) {
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss[%d] wapi_mib_stats == NULL\n",j);
					memset(&MIBNULL,0,sizeof(wapi_stats_Entry));
					bss_wapi_mib_stat = &MIBNULL;
				}
				else
					bss_wapi_mib_stat = &bss[j]->wapi_wasd->vap_user->wapid->wapi_mib_stats;

				dbus_message_iter_open_container (&iter_sub_array,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_sub_struct);

				unsigned char bss_index;
				bss_index = (unsigned char)j;
				
				dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
														&bss_index);
				dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
														&bss[j]->WlanID);
				
				Radio_G_ID = bss[j]->Radio_G_ID;
				WTPID = Radio_G_ID/L_RADIO_NUM;

				dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
															&WTPID);

				if((ASD_WTP[WTPID]!=NULL)&&(ASD_WTP[WTPID]->WTPMAC !=NULL)){
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(ASD_WTP[WTPID]->WTPMAC[0]));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(ASD_WTP[WTPID]->WTPMAC[1]));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(ASD_WTP[WTPID]->WTPMAC[2]));

					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(ASD_WTP[WTPID]->WTPMAC[3]));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(ASD_WTP[WTPID]->WTPMAC[4]));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(ASD_WTP[WTPID]->WTPMAC[5]));
				}
				else{
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(defalt_char_zero));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(defalt_char_zero));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(defalt_char_zero));

					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(defalt_char_zero));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(defalt_char_zero));
					dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(defalt_char_zero));
				}

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(bss[j]->own_addr[0]));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(bss[j]->own_addr[1]));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(bss[j]->own_addr[2]));

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(bss[j]->own_addr[3]));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(bss[j]->own_addr[4]));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_BYTE,
												&(bss[j]->own_addr[5]));

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_sign_errors));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_sign_errors));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_auth_res_fail));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_discard));

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_timeout));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_format_errors));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_cert_handshake_fail));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_unicast_handshake_fail));

				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wai_multi_handshake_fail));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wpi_mic_errors));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wpi_replay_counters));
				dbus_message_iter_append_basic(&iter_sub_struct,
												DBUS_TYPE_UINT32,
												&(bss_wapi_mib_stat->wpi_decryptable_errors));

				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
			}		
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);		//mahz add 2011.4.20
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	if(bss){
		free(bss);
		bss = NULL;
	}
	return reply;
}

//mahz changed to match wid , 2011.1.26
DBusMessage *asd_dbus_show_bss_sta_num_by_wlanid_and_radioid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter  iter_sub_array;
	DBusMessageIter  iter_sub_struct;
	DBusMessageIter  iter_bss_array;
	DBusMessageIter  iter_bss_struct;
	DBusError err;		

	unsigned int WTPID = 0;
	unsigned int wtp_num = 0;
	unsigned int radio_num = 0;
	unsigned int sta_num = 0;
	unsigned int auth_tms = 0;
	unsigned int repauth_tms = 0;
	unsigned int assoc_req_num = 0;
	unsigned int assoc_resp_num = 0;

	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned int bss_num = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int ii = 0;
	//qiuchen
	struct sta_info *sta = NULL;
	unsigned int sta_online_time = 0;
	unsigned int sta_drop_cnt = 0;
	unsigned int auth_req_cnt = 0, auth_suc_cnt = 0, auth_fail_cnt = 0;
	time_t cur_time = 0;
	//end
	int ret = ASD_DBUS_SUCCESS;

	dbus_error_init(&err);

	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP, 0, WTP_NUM*(sizeof(ASD_WTP_ST *)));

	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	wtp_num = Asd_Find_Wtp(WTP);

	if(0 == wtp_num){
		ret = ASD_WTP_NOT_EXIST;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);
	
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 

	if(wtp_num != 0){
		dbus_message_iter_open_container(&iter,
												DBUS_TYPE_ARRAY,
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												  DBUS_TYPE_UINT32_AS_STRING	//WTPID
												  DBUS_TYPE_UINT32_AS_STRING	//radio num

													DBUS_TYPE_ARRAY_AS_STRING		//二层循环
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													  DBUS_TYPE_UINT32_AS_STRING	//radio id
													  DBUS_TYPE_UINT32_AS_STRING	//bss_num

														DBUS_TYPE_ARRAY_AS_STRING	//三层循环
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING	//wlan id
															DBUS_TYPE_UINT32_AS_STRING	//sta_num
															DBUS_TYPE_UINT32_AS_STRING	//auth_tms
															DBUS_TYPE_UINT32_AS_STRING	//repauth_tms
															DBUS_TYPE_UINT32_AS_STRING	//assoc_req_num
															DBUS_TYPE_UINT32_AS_STRING	//assoc_resp_num
															DBUS_TYPE_UINT32_AS_STRING	//SuccAssociatedNum
															DBUS_TYPE_UINT32_AS_STRING	//acc_tms
															DBUS_TYPE_UINT32_AS_STRING	// total sta online time
															DBUS_TYPE_UINT32_AS_STRING	// total sta drop count
															DBUS_TYPE_UINT32_AS_STRING	// auth request count
															DBUS_TYPE_UINT32_AS_STRING	// auth success count
															DBUS_TYPE_UINT32_AS_STRING	// auth failed count														
														DBUS_STRUCT_END_CHAR_AS_STRING
   													DBUS_STRUCT_END_CHAR_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&iter_array);
		for(i=0; i<wtp_num; i++){
			unsigned int radio[L_RADIO_NUM] = {0};
			radio_num = 0;
			WTPID = WTP[i]->WTPID;

			for(k=WTPID*L_RADIO_NUM; k<(WTPID+1)*L_RADIO_NUM; k++){
				struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
				
				if((NULL == interfaces->iface[k])||(NULL == interfaces->iface[k]->bss)){
					continue;
				}
				else{
					radio[radio_num] = k ;
					radio_num++;
				}
			}
			dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32,&WTPID);
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32,&radio_num);

			dbus_message_iter_open_container(&iter_struct,
													DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													 DBUS_TYPE_UINT32_AS_STRING	  //radio id
													 DBUS_TYPE_UINT32_AS_STRING	//bss_num
													 
														DBUS_TYPE_ARRAY_AS_STRING	//三层循环
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING	//wlan id
															DBUS_TYPE_UINT32_AS_STRING	//sta_num
															DBUS_TYPE_UINT32_AS_STRING	//auth_tms
															DBUS_TYPE_UINT32_AS_STRING	//repauth_tms
															DBUS_TYPE_UINT32_AS_STRING	//assoc_req_num
															DBUS_TYPE_UINT32_AS_STRING	//assoc_resp_num	
															DBUS_TYPE_UINT32_AS_STRING	//SuccAssociatedNum
															DBUS_TYPE_UINT32_AS_STRING	//acc_tms
															DBUS_TYPE_UINT32_AS_STRING	// total sta online time
															DBUS_TYPE_UINT32_AS_STRING	// total sta drop count
															DBUS_TYPE_UINT32_AS_STRING	// auth request count
															DBUS_TYPE_UINT32_AS_STRING	// auth success count
															DBUS_TYPE_UINT32_AS_STRING	// auth failed count														
														DBUS_STRUCT_END_CHAR_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING,
													&iter_bss_array);

			for(ii=0; ii<radio_num; ii++){
				bss_num = 0;
				sta_num = 0;
				auth_tms = 0;
				repauth_tms = 0;
				assoc_req_num = 0;
				assoc_resp_num = 0;

				bss_num = ASD_SEARCH_RADIO_STA(radio[ii], bss);
				dbus_message_iter_open_container(&iter_bss_array,DBUS_TYPE_STRUCT,NULL,&iter_bss_struct);
				dbus_message_iter_append_basic(&iter_bss_struct,DBUS_TYPE_UINT32,&(radio[ii]));
				dbus_message_iter_append_basic(&iter_bss_struct,DBUS_TYPE_UINT32,&bss_num);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss_num = %d\n",bss_num);
				dbus_message_iter_open_container(&iter_bss_struct,
													DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												 		DBUS_TYPE_BYTE_AS_STRING	//wlan id
														DBUS_TYPE_UINT32_AS_STRING	//sta_num
														DBUS_TYPE_UINT32_AS_STRING	//auth_tms
														DBUS_TYPE_UINT32_AS_STRING	//repauth_tms
														DBUS_TYPE_UINT32_AS_STRING	//assoc_req_num
														DBUS_TYPE_UINT32_AS_STRING	//assoc_resp_num	
														DBUS_TYPE_UINT32_AS_STRING	//SuccAssociatedNum
														DBUS_TYPE_UINT32_AS_STRING	//acc_tms
														DBUS_TYPE_UINT32_AS_STRING	// total sta online time
														DBUS_TYPE_UINT32_AS_STRING	// total sta drop count
														DBUS_TYPE_UINT32_AS_STRING	// auth request count
														DBUS_TYPE_UINT32_AS_STRING	// auth success count
														DBUS_TYPE_UINT32_AS_STRING	// auth failed count														
													DBUS_STRUCT_END_CHAR_AS_STRING,
													&iter_sub_array);

				for(j=0; j<bss_num; j++){
					if(NULL == bss[j]->info){
						continue;
					}
					sta_online_time = 0;//qiuchen
					sta_drop_cnt = 0;
					auth_req_cnt = 0;
					auth_suc_cnt = 0;
					auth_fail_cnt = 0;
					cur_time = 0;
					get_sysruntime(&cur_time);
					
					GetBssDataPkts(bss[j]);
					unsigned int SuccAssociatedNum = 0;
					SuccAssociatedNum = bss[j]->assoc_success+bss[j]->reassoc_success;		//xiaodawei modify, SuccAssociatedNum=assoc_success+reassoc_success
					dbus_message_iter_open_container (&iter_sub_array,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_sub_struct);

					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_BYTE,
													 &(bss[j]->WlanID));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													 &(bss[j]->num_sta));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													 &(bss[j]->usr_auth_tms));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													 &(bss[j]->ac_rspauth_tms));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													 &(bss[j]->assoc_req));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													 &(bss[j]->assoc_resp));

					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													 &SuccAssociatedNum);
				//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"j = %d , bss[j]->WlanID = %d \n",j,bss[j]->WlanID);
				//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta_num = %d\n",bss[j]->num_sta);

					dbus_message_iter_append_basic (&iter_sub_struct,
													DBUS_TYPE_UINT32,
													&bss[j]->acc_tms);
					
					sta_online_time += bss[j]->statistics.offline_sta_time;
					for (sta = bss[j]->sta_list; sta; sta = sta->next)
					{
						sta_online_time += (cur_time - (sta->add_time_sysruntime));
					}
					sta_drop_cnt = bss[j]->statistics.sta_drop_cnt;
		
					if (ASD_AUTH_TYPE_WEP_PSK(bss[j]->SecurityID))	/* WEP/PSK assocAuth */
					{
				
					}
					else if (ASD_AUTH_TYPE_EAP(bss[j]->SecurityID)) /* EAP auth */
					{
						auth_req_cnt += bss[j]->u.eap_auth.autoauth.auth_req_cnt;
						auth_suc_cnt += bss[j]->u.eap_auth.autoauth.auth_suc_cnt;
						auth_fail_cnt += bss[j]->u.eap_auth.autoauth.auth_fail_cnt;
					}
					
					asd_printf(ASD_DBUS,MSG_DEBUG,"bss %d( radio %d wlan %d security %d)"
						"onlinetime %u dropcount %u "
						"authreq %u authsuc %u authfail %u\n",
						bss[j]->BSSIndex, bss[j]->Radio_L_ID, bss[j]->WlanID, bss[j]->SecurityID,
						sta_online_time, sta_drop_cnt,
						auth_req_cnt, auth_suc_cnt, auth_fail_cnt);
					
					dbus_message_iter_append_basic (&iter_sub_struct, DBUS_TYPE_UINT32, &sta_online_time);		
					dbus_message_iter_append_basic (&iter_sub_struct, DBUS_TYPE_UINT32, &sta_drop_cnt);		
					dbus_message_iter_append_basic (&iter_sub_struct, DBUS_TYPE_UINT32, &auth_req_cnt);		
					dbus_message_iter_append_basic (&iter_sub_struct, DBUS_TYPE_UINT32, &auth_suc_cnt);		
					dbus_message_iter_append_basic (&iter_sub_struct, DBUS_TYPE_UINT32, &auth_fail_cnt);

					dbus_message_iter_close_container(&iter_sub_array, &iter_sub_struct);
				}
				dbus_message_iter_close_container (&iter_bss_struct, &iter_sub_array);
				dbus_message_iter_close_container (&iter_bss_array, &iter_bss_struct);
			}
			dbus_message_iter_close_container (&iter_struct, &iter_bss_array);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container(&iter, &iter_array);
	}

	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20

	if(WTP!= NULL){
		free(WTP);
		WTP = NULL;
	}

	return reply;
#if 0
	DBusMessage* reply; 
	DBusMessageIter  iter;
	
	DBusError err;		
	dbus_error_init(&err);

	unsigned int wtpid =0;
	unsigned int radio_id =0;
	unsigned int wlan_id = 0;
	unsigned int bss_num = 0;
	unsigned int sta_num = 0;
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	
	int ret = ASD_DBUS_SUCCESS;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_UINT32,&radio_id,
								DBUS_TYPE_UINT32,&wlan_id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret = ASD_CHECK_ID(ASD_WTP_CHECK,(unsigned int)wtpid);

	if(ret != ASD_DBUS_SUCCESS)
		printf("wtp id not correct !\n");
	else if(ASD_WTP_AP[wtpid]==NULL)
		ret = ASD_WTP_NOT_EXIST;
	else if(ret == ASD_DBUS_SUCCESS){
		bss_num = ASD_SEARCH_WTP_STA(wtpid, bss);
		if(bss_num==0)
			ret = ASD_BSS_NOT_EXIST;
	}
	int i =0;
	unsigned char radio_l_id = (unsigned char )radio_id;
	
	for(i=0;i<bss_num;i++){
		if(bss[i]->WlanID==wlan_id && bss[i]->Radio_L_ID==radio_l_id){
			sta_num=bss[i]->num_sta;			
			break;
		}
	}
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&sta_num); 
	
	return reply;
#endif
}
/*fengwenchao add 20110331 for dot11WtpChannelTable*/
DBusMessage *asd_dbus_show_statistics_information_of_all_wtp_whole(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;

	DBusError err;		
	dbus_error_init(&err);

	int i = 0; 
	int j = 0;	
	int ret = ASD_DBUS_SUCCESS;
	unsigned int bss_num = 0;
	unsigned int wtp_num = 0;
	//unsigned int sta_num = 0;  fengwenchao comment 20110423
	unsigned int WTPID = 0;
	struct asd_data *bss[L_BSS_NUM] = {NULL};

	ASD_WTP_ST **WTP;
	WTP = malloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if( WTP == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WTP,0,WTP_NUM * sizeof(ASD_WTP_ST *));

	pthread_mutex_lock(&asd_g_wtp_mutex); 
    wtp_num = Asd_Find_Wtp(WTP);
	if(wtp_num == 0)
		ret = ASD_WTP_NOT_EXIST;

	reply = dbus_message_new_method_return(msg);

	if(NULL==reply)
		printf("reply = NULL\n");

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wtp_num); 	

	pthread_mutex_lock(&asd_g_sta_mutex);	
		
	if(ret == 0)
	{

	   dbus_message_iter_open_container (&iter,
									  DBUS_TYPE_ARRAY,
									  DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										   DBUS_TYPE_UINT32_AS_STRING	   //wtp id
										   DBUS_TYPE_UINT32_AS_STRING	   //sta_num
	   
									  DBUS_STRUCT_END_CHAR_AS_STRING,
									  &iter_array);

	
	   for(i = 0; i < wtp_num; i++)
	   {
	   		unsigned int sta_num = 0; //fengwenchao add 20110423
	   		WTPID = WTP[i]->WTPID;
			bss_num = ASD_SEARCH_WTP_STA(WTPID, bss);

			for(j=0; j<bss_num; j++)
			{
				sta_num += bss[j]->num_sta;
			}
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &WTPID);
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &sta_num);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
	   }
	   dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 
	if(WTP){
		free(WTP);
		WTP=NULL;
	}
	return reply;
}
/*fengwenchao add end*/
DBusMessage *asd_dbus_show_sta_v2(DBusConnection *conn, DBusMessage *msg, void *user_data){

	
		DBusMessage* reply;		
		DBusMessageIter  iter;
		unsigned char *mac;
		unsigned char SecurityID;
		unsigned char wlanid;
		int vlanid = 0;
		unsigned int  auth_type = 0;
		char *essid = NULL;
		int essidlen = 0,i = 0;
		int WTPID = 0;
		DBusError err;
		int ret = ASD_DBUS_SUCCESS;
		struct asd_stainfo *stainfo = NULL;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);

		mac = (unsigned char*)malloc(WID_MAC_LEN);
		if(mac==NULL)
			return NULL;	//	0608xm	
		memset(mac, 0, WID_MAC_LEN);
		dbus_error_init(&err);
		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_BYTE,&mac[0],
									DBUS_TYPE_BYTE,&mac[1],
									DBUS_TYPE_BYTE,&mac[2],
									DBUS_TYPE_BYTE,&mac[3],
									DBUS_TYPE_BYTE,&mac[4],
									DBUS_TYPE_BYTE,&mac[5],
									DBUS_TYPE_INVALID))){
	
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
			if (dbus_error_is_set(&err)) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			free(mac);
			mac = NULL; // 0608xm
			return NULL;
		}
		pthread_mutex_lock(&asd_g_wlan_mutex);
		pthread_mutex_lock(&asd_g_sta_mutex);	

		stainfo = ASD_SEARCH_STA(mac);
		
		reply = dbus_message_new_method_return(msg);
			
		dbus_message_iter_init_append (reply, &iter);

		if((stainfo != NULL)&&(stainfo->sta != NULL)){
			auth_type = stainfo->sta->security_type ;
			if(stainfo->bss->WlanID < WLAN_NUM && ASD_WLAN[stainfo->bss->WlanID] != NULL && ASD_WLAN[stainfo->bss->WlanID]->ESSID != NULL){

				SecurityID = ASD_WLAN[stainfo->bss->WlanID]->SecurityID;
				essid = ASD_WLAN[stainfo->bss->WlanID]->ESSID;
				essidlen = strlen(essid);
			}
			WTPID = stainfo->bss->Radio_G_ID/L_RADIO_NUM;
			if(WTPID < WTP_NUM && ASD_WTP_AP[WTPID] != NULL){
				memset(mac, 0, WID_MAC_LEN);
				memcpy(mac, ASD_WTP_AP[WTPID]->WTPMAC, WID_MAC_LEN);
			}
			if(stainfo->bss->BSSIndex < BSS_NUM && ASD_BSS[stainfo->bss->BSSIndex] != NULL)
				vlanid = ASD_BSS[stainfo->bss->BSSIndex]->vlanid;
				
			//weichao add 2011.11.08
			wlanid =	stainfo->bss->WlanID;
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &ret);			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(stainfo->bss->WlanID));
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->bss->Radio_G_ID));
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(stainfo->bss->BSSIndex));	
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(SecurityID));	
			
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(vlanid));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(mac[0]));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(mac[1]));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(mac[2]));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(mac[3]));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(mac[4]));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_BYTE,
												 &(mac[5]));	
			dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &(essidlen));	
			for(i = 0;i<essidlen;i++)
				dbus_message_iter_append_basic (&iter,
													 DBUS_TYPE_BYTE,
													 &(essid[i]));	
			if(wlanid < 129)
			{
				dbus_message_iter_append_basic (&iter,
													DBUS_TYPE_UINT16,
													 &(ASD_WLAN[wlanid]->flow_check));	
				dbus_message_iter_append_basic (&iter,
													DBUS_TYPE_UINT32,
													 &(ASD_WLAN[wlanid]->no_flow_time));	
				dbus_message_iter_append_basic (&iter,
													DBUS_TYPE_UINT32,
													 &(ASD_WLAN[wlanid]->limit_flow));	
				dbus_message_iter_append_basic (&iter,
													DBUS_TYPE_UINT32,
													 &auth_type);	
				asd_printf(ASD_DBUS,MSG_DEBUG,"flow_check = %d\n",ASD_WLAN[wlanid]->flow_check);
				asd_printf(ASD_DBUS,MSG_DEBUG,"no_flow_time = %d\n",ASD_WLAN[wlanid]->no_flow_time);
				asd_printf(ASD_DBUS,MSG_DEBUG,"limit_flow = %d\n",ASD_WLAN[wlanid]->limit_flow);
			}
			


		}else{
			ret = ASD_STA_NOT_EXIST;
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 		

		}
		free(mac);
		mac = NULL;
		if(stainfo != NULL){
			stainfo->bss = NULL;
			stainfo->sta = NULL;
			free(stainfo);
			stainfo = NULL;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
		pthread_mutex_unlock(&asd_g_sta_mutex);	

		return reply;	

}
DBusMessage *asd_dbus_set_ac_flow(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	unsigned char type=0;	//1--enable, 0--disable
	unsigned char wlanid=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(ASD_WLAN[wlanid] == NULL)
		ret = ASD_WLAN_NOT_EXIST;
	else{
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s--->set wlan %u flow %u\n",__func__,wlanid,type);
		ASD_WLAN[wlanid]->flow_compute=0;
		circle_cancel_timeout(get_flow_timer_handler, NULL, ASD_WLAN[wlanid]);
		if(type==1){
			ASD_WLAN[wlanid]->flow_compute=1;
			circle_register_timeout(0, 0, get_flow_timer_handler, NULL, ASD_WLAN[wlanid]);	//xm0714
		}
	}
	
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}

//xm add 08/11/07
DBusMessage *asd_dbus_wlan_add_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int ret1 = 0;
	struct sta_info *sta=NULL;

	unsigned char wlanid=0;
	unsigned char list_type=0;	//1--black list, 2--white list
	unsigned char *mac;
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);
	struct asd_data **bss;
	int bss_num=0;
	int i=0;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; // 0608xm
		free(bss);
		bss = NULL;
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DBUS,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive wlanid :%d,   list_type :%d\n",wlanid,list_type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================

	pthread_mutex_lock(&asd_g_wlan_mutex);
	if(ASD_WLAN[wlanid] == NULL)
		ret = ASD_WLAN_NOT_EXIST;
	else {
		if((ret1 = add_mac_in_maclist(ASD_WLAN[wlanid]->acl_conf,mac,list_type)) == -1)
			asd_printf(ASD_DBUS,MSG_DEBUG,"add mac failed\n");
		else if(ret1 == 1)
			ret = ASD_MAC_ADD_ALREADY;
		bss_num=ASD_SEARCH_WLAN_STA(wlanid,bss);
		for(i=0;i<bss_num;i++){
			if(1==list_type && (ASD_BSS[bss[i]->BSSIndex]->acl_conf != NULL)&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) &&
				(ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf!= NULL) && (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 0) &&
				(ASD_WLAN[bss[i]->WlanID]->acl_conf != NULL) && (ASD_WLAN[bss[i]->WlanID]->acl_conf->macaddr_acl == 1)){
				sta = ap_get_sta(bss[i], mac);
				if (sta != NULL){
					bss_kick_sta(bss[i], sta);
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&asd_g_wlan_mutex);
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	free(mac);
	mac = NULL;
	free(bss);
	bss = NULL;
	return reply;	
}

DBusMessage *asd_dbus_wtp_add_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int ret1 = 0;
	struct sta_info *sta=NULL;

	unsigned int wtpid=0;
	unsigned char list_type=0;
	unsigned char *mac;
	struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
	int bss_num=0;
	int i=0;
	
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; // 0608xm
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DBUS,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive wtpid :%d, list_type :%d\n",wtpid,list_type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================
	if(wtpid >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
	}
	else if(ASD_WTP_AP[wtpid] == NULL)
		ret = ASD_WTP_NOT_EXIST;
	else {
		if((ret1 = add_mac_in_maclist(ASD_WTP_AP[wtpid]->acl_conf,mac,list_type)) == -1)
			asd_printf(ASD_DBUS,MSG_DEBUG,"add mac failed\n");
		else if(ret1 == 1)
			ret = ASD_MAC_ADD_ALREADY;
		bss_num = ASD_SEARCH_WTP_BSS(wtpid,bss);
		for(i=0;i<bss_num;i++){
			if(1==list_type && (ASD_BSS[bss[i]->BSSIndex]->acl_conf != NULL)&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) &&
				(ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf != NULL) && (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 1)){
				sta = ap_get_sta(bss[i], mac);
				if (sta != NULL){
					bss_kick_sta(bss[i], sta);
					break;
				}
			}
		}
	}


	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	free(mac);
	mac = NULL;
	return reply;	
}

DBusMessage *asd_dbus_bss_add_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;

	int ret = ASD_DBUS_SUCCESS;
	int ret1 = 0;
	struct sta_info *sta=NULL;
	unsigned int radioid=0;
	unsigned int bssindex=0;
	unsigned char list_type=0;
	unsigned char wlanid=0;			//mahz add 2011.5.11
	int i = 0;
	unsigned char *mac;
	struct asd_data *bss;
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; // 0608xm
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DBUS,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive radio/wlan :%d/%d, list_type :%d\n",radioid,wlanid,list_type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================

	for(i=radioid*L_BSS_NUM; i<(radioid+1)*L_BSS_NUM; i++){
		if((ASD_BSS[i] != NULL)&&(ASD_BSS[i]->WlanID == wlanid)){
			bssindex = i;
			break;
		}
	}

	if(radioid >= G_RADIO_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
	}
	else if(bssindex == 0){
		ret = ASD_BSS_NOT_EXIST;
	}
	else {
		if((ret1 = add_mac_in_maclist(ASD_BSS[bssindex]->acl_conf,mac,list_type)) == -1)
			asd_printf(ASD_DBUS,MSG_DEBUG,"add mac failed\n");
		else if(ret1 == 1)
			ret = ASD_MAC_ADD_ALREADY;
		if( ASD_SEARCH_BSS_BY_WLANID(radioid, wlanid, &bss) == 0 ) {
			if((1==list_type) && (ASD_BSS[bssindex]->acl_conf != NULL)&&(ASD_BSS[bssindex]->acl_conf->macaddr_acl == 1)){
				sta = ap_get_sta(bss, mac);
				if (sta != NULL) 
					bss_kick_sta(bss, sta);
			}
		}
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	free(mac);
	mac = NULL;
	return reply;	
}

DBusMessage *asd_dbus_wlan_del_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta=NULL;

	unsigned char wlanid=0;
	unsigned char list_type=0;	//1--black list, 2--white list
	unsigned char *mac;
	struct asd_data **bss;
	int bss_num=0;
	int i=0;

	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(bss);
		bss = NULL;
		free(mac);
		mac = NULL; // 0608xm
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DBUS,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive wlanid :%d,   list_type :%d\n",wlanid,list_type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================

	pthread_mutex_lock(&asd_g_wlan_mutex);
	if(ASD_WLAN[wlanid] == NULL)
		ret = ASD_WLAN_NOT_EXIST;
	else {
		if(del_mac_in_maclist(ASD_WLAN[wlanid]->acl_conf,mac,list_type) == -1){
			asd_printf(ASD_DBUS,MSG_DEBUG,"del mac failed\n");
			ret = ASD_UNKNOWN_ID;
		}else{
			bss_num=ASD_SEARCH_WLAN_STA(wlanid,bss);
			for(i=0;i<bss_num;i++){
				if(2==list_type && (ASD_BSS[bss[i]->BSSIndex]->acl_conf != NULL)&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) &&
					(ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf!= NULL) && (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 0) &&
					(ASD_WLAN[bss[i]->WlanID]->acl_conf != NULL) && (ASD_WLAN[bss[i]->WlanID]->acl_conf->macaddr_acl == 2)){
					sta = ap_get_sta(bss[i], mac);
					if (sta != NULL){
						bss_kick_sta(bss[i], sta);
						break;
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&asd_g_wlan_mutex);
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	free(mac);
	mac = NULL;
	free(bss);
	bss=NULL;
	return reply;	
}

DBusMessage *asd_dbus_wtp_del_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta=NULL;

	unsigned int wtpid=0;
	unsigned char list_type=0;
	unsigned char *mac;
	struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
	int bss_num=0;
	int i=0;
	
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; // 0608xm
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DBUS,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive wtpid :%d, list_type :%d\n",wtpid,list_type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================
	if(wtpid >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
	}
	else if(ASD_WTP_AP[wtpid] == NULL)
		ret = ASD_WTP_NOT_EXIST;
	else {
		if( del_mac_in_maclist(ASD_WTP_AP[wtpid]->acl_conf,mac,list_type) != 0 ){
			asd_printf(ASD_DBUS,MSG_DEBUG,"del mac failed\n");
			ret = ASD_UNKNOWN_ID;
		}else{
			bss_num = ASD_SEARCH_WTP_BSS(wtpid,bss);
		
			for(i=0;i<bss_num;i++){
				if(2==list_type && (ASD_BSS[bss[i]->BSSIndex]->acl_conf != NULL)&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) &&
					(ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf != NULL) && (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 2)){
					sta = ap_get_sta(bss[i], mac);
					if (sta != NULL){
						bss_kick_sta(bss[i], sta);
						break;
					}
				}
			}
		}
	}


	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	free(mac);
	mac = NULL;
	return reply;	
}

DBusMessage *asd_dbus_bss_del_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;

	//struct wasd_interfaces *interfaces = (struct wasd_interfaces*)circle.user_data;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta=NULL;
	unsigned int radioid=0;
	unsigned char bssid=0;
	unsigned int bssindex=0;
	unsigned char list_type=0;
	unsigned char wlanid=0;			//mahz add 2011.5.11
	int i = 0;
	unsigned char *mac;
	struct asd_data *bss;
	mac = (unsigned char*)os_zalloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;	//	0608xm
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; // 0608xm
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DBUS,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd receive radio/bss :%d/%d, list_type :%d\n",radioid,bssid,list_type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================

	
	for(i=radioid*L_BSS_NUM; i<(radioid+1)*L_BSS_NUM; i++){
		if((ASD_BSS[i] != NULL)&&(ASD_BSS[i]->WlanID == wlanid)){
			bssindex = i;
			break;
		}
	}

	if(radioid >= G_RADIO_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
	}
	else if(bssindex == 0){
		ret = ASD_BSS_NOT_EXIST;
	}
	else {
		if(del_mac_in_maclist(ASD_BSS[bssindex]->acl_conf,mac,list_type) == -1){
			asd_printf(ASD_DBUS,MSG_DEBUG,"del mac failed\n");
			ret = ASD_UNKNOWN_ID;
		}else{
			if( ASD_SEARCH_BSS_BY_WLANID(radioid, wlanid, &bss) == 0 ){
				if((2==list_type) && (ASD_BSS[bssindex]->acl_conf != NULL)&&(ASD_BSS[bssindex]->acl_conf->macaddr_acl == 2)){
					sta = ap_get_sta(bss, mac);
					if (sta != NULL) 
						bss_kick_sta(bss, sta);
				}
			}
		}
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	free(mac);
	mac = NULL;
	return reply;	
}


DBusMessage *asd_dbus_wlan_use_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta=NULL, *kick_sta=NULL;
	struct acl_config *conf;
	unsigned char wlanid=0;
	unsigned char list_type=0;	//0--none,1--black list, 2--white list
	struct asd_data **bss;
	struct maclist *entry;
	macaddr mac;
	int bss_num=0;
	int i=0;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(bss);
		bss=NULL;
		return NULL;
	}
	
	if(ASD_WLAN[wlanid] == NULL){
		ret = ASD_WLAN_NOT_EXIST;
	}else if((wids_enable == 1) && (list_type != 1)){ 
		ret = ASD_WIDS_OPEN;
	}else {
	    pthread_mutex_lock(&asd_g_wtp_mutex);
		pthread_mutex_lock(&asd_g_sta_mutex);
		conf = ASD_WLAN[wlanid]->acl_conf;
		if(change_maclist_security(conf,list_type) != 0)
			asd_printf(ASD_DBUS,MSG_DEBUG,"change mac list failed\n");
		
		bss_num=ASD_SEARCH_WLAN_STA(wlanid,bss);
		for(i=0;i<bss_num;i++){
			if((ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) && (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 0) &&
				(conf->macaddr_acl == 1)){
				for(entry=conf->deny_mac; entry; entry=entry->next) {
					os_memcpy(mac,entry->addr,WID_MAC_LEN);
					sta = ap_get_sta(bss[i], mac);
					if (sta != NULL)
						bss_kick_sta(bss[i], sta);
				}	
			}else if((ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) && (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 0) &&
				(conf->macaddr_acl == 2)) {
				for (sta=bss[i]->sta_list; sta!=NULL; ) {
					int flag = 0;
					for(entry=conf->accept_mac; entry; entry=entry->next) {
						os_memcpy(mac,entry->addr,WID_MAC_LEN);
						if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
							flag = 1;
							break;
						}
					}
					if (flag == 0){
						kick_sta = sta;
						sta=sta->next;
						bss_kick_sta(bss[i], kick_sta);
					}else {
						sta=sta->next;
					}
				}
			}
		}
	    pthread_mutex_unlock(&asd_g_sta_mutex);
	    pthread_mutex_unlock(&asd_g_wtp_mutex);
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	free(bss);
	bss=NULL;
	return reply;	
}

DBusMessage *asd_dbus_wtp_use_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta=NULL, *kick_sta=NULL;
	struct acl_config *conf;
	struct maclist *entry;
	macaddr mac;
	unsigned int wtpid=0;
	unsigned char list_type=0;	//0--none,1--black list, 2--white list
	struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
	int bss_num=0;
	int i=0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	if(wtpid >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
	}
	else if(ASD_WTP_AP[wtpid] == NULL){
		ret = ASD_WTP_NOT_EXIST;
	}else {
		conf = ASD_WTP_AP[wtpid]->acl_conf;
		if(change_maclist_security(conf,list_type) != 0)
			asd_printf(ASD_DBUS,MSG_DEBUG,"change mac list failed\n");

		bss_num = ASD_SEARCH_WTP_BSS(wtpid,bss);
		if(NULL != conf)
		{
			for(i=0;i<bss_num;i++){
				if((ASD_BSS[bss[i]->BSSIndex])&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) && (conf->macaddr_acl == 1)){
					for(entry=conf->deny_mac; entry; entry=entry->next) {
						os_memcpy(mac,entry->addr,WID_MAC_LEN);
						sta = ap_get_sta(bss[i], mac);
						if (sta != NULL)
							bss_kick_sta(bss[i], sta);
					}	
				}else if((ASD_BSS[bss[i]->BSSIndex])&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) &&	(conf->macaddr_acl == 2)) {
					for (sta=bss[i]->sta_list; sta!=NULL; ) {
						int flag = 0;
						for(entry=conf->accept_mac; entry; entry=entry->next) {
							os_memcpy(mac,entry->addr,WID_MAC_LEN);
							if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
								flag = 1;
								break;
							}
						}
						if (flag == 0){
							kick_sta = sta;
							sta=sta->next;
							bss_kick_sta(bss[i], kick_sta);
						}else {
							sta=sta->next;
						}
					}
				}else if((ASD_BSS[bss[i]->BSSIndex])&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) &&	(conf->macaddr_acl == 0)){
					if(ASD_WLAN[bss[i]->WlanID])
						conf = ASD_WLAN[bss[i]->WlanID]->acl_conf;
					if(conf && conf->macaddr_acl == 1){
						for(entry=conf->deny_mac; entry; entry=entry->next) {
							os_memcpy(mac,entry->addr,WID_MAC_LEN);
							sta = ap_get_sta(bss[i], mac);
							if (sta != NULL)
								bss_kick_sta(bss[i], sta);
						}	
					}else if(conf && conf->macaddr_acl == 2) {
						for (sta=bss[i]->sta_list; sta!=NULL; ) {
							int flag = 0;
							for(entry=conf->accept_mac; entry; entry=entry->next) {
								os_memcpy(mac,entry->addr,WID_MAC_LEN);
								if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
									flag = 1;
									break;
								}
							}
							if (flag == 0){
								kick_sta = sta;
								sta=sta->next;
								bss_kick_sta(bss[i], kick_sta);
							}else {
								sta=sta->next;
							}
						}
					}
				}
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}

DBusMessage *asd_dbus_bss_use_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;

	//struct wasd_interfaces *interfaces = (struct wasd_interfaces*)circle.user_data;
	struct sta_info *sta=NULL, *kick_sta=NULL;
	struct acl_config *conf;
	struct maclist *entry;
	macaddr mac;
	unsigned int radioid=0;
	unsigned int bssindex=0;
	unsigned char list_type=0;	//0--none,1--black list, 2--white list
	unsigned char wlanid=0;		//mahz add 2011.5.11
	int i = 0;
	struct asd_data *bss;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	for(i=radioid*L_BSS_NUM; i<(radioid+1)*L_BSS_NUM; i++){
		if((ASD_BSS[i] != NULL)&&(ASD_BSS[i]->WlanID == wlanid)){
			bssindex = i;
			break;
		}
	}

	if(radioid >= G_RADIO_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
	}
	else if(bssindex == 0){
		ret = ASD_BSS_NOT_EXIST;
	}
	else{
		conf = ASD_BSS[bssindex]->acl_conf;
		if(change_maclist_security(conf,list_type) != 0)
			asd_printf(ASD_DBUS,MSG_DEBUG,"change mac list failed\n");

		if( ASD_SEARCH_BSS_BY_WLANID(radioid, wlanid, &bss) == 0 ){
			if(conf->macaddr_acl == 1){
				for(entry=conf->deny_mac; entry; entry=entry->next) {
					os_memcpy(mac,entry->addr,WID_MAC_LEN);
					sta = ap_get_sta(bss, mac);
					if (sta != NULL)
						bss_kick_sta(bss, sta);
				}	
			}else if(conf->macaddr_acl == 2) {
				for (sta=bss->sta_list; sta!=NULL; ) {
					int flag = 0;
					for(entry=conf->accept_mac; entry; entry=entry->next) {
						os_memcpy(mac,entry->addr,WID_MAC_LEN);
						if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
							flag = 1;
							break;
						}
					}
					if (flag == 0){
						kick_sta = sta;
						sta=sta->next;
						bss_kick_sta(bss, kick_sta);
					}else {
						sta=sta->next;
					}
				}
			}else {
				conf = ASD_WTP_AP[bss->Radio_G_ID/L_RADIO_NUM]->acl_conf;
				if(conf->macaddr_acl == 1){
					for(entry=conf->deny_mac; entry; entry=entry->next) {
						os_memcpy(mac,entry->addr,WID_MAC_LEN);
						sta = ap_get_sta(bss, mac);
						if (sta != NULL)
							bss_kick_sta(bss, sta);
					}	
				}else if(conf->macaddr_acl == 2) {
					for (sta=bss->sta_list; sta!=NULL; ) {
						int flag = 0;
						for(entry=conf->accept_mac; entry; entry=entry->next) {
							os_memcpy(mac,entry->addr,WID_MAC_LEN);
							if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
								flag = 1;
								break;
							}
						}
						if (flag == 0){
							kick_sta = sta;
							sta=sta->next;
							bss_kick_sta(bss, kick_sta);
						}else {
							sta=sta->next;
						}
					}
				}else{
					if(ASD_WLAN[bss->WlanID])
						conf = ASD_WLAN[bss->WlanID]->acl_conf;
					if(conf && conf->macaddr_acl == 1){
						for(entry=conf->deny_mac; entry; entry=entry->next) {
							os_memcpy(mac,entry->addr,WID_MAC_LEN);
							sta = ap_get_sta(bss, mac);
							if (sta != NULL)
								bss_kick_sta(bss, sta);
						}	
					}else if(conf && conf->macaddr_acl == 2) {
						for (sta=bss->sta_list; sta!=NULL; ) {
							int flag = 0;
							for(entry=conf->accept_mac; entry; entry=entry->next) {
								os_memcpy(mac,entry->addr,WID_MAC_LEN);
								if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
									flag = 1;
									break;
								}
							}
							if (flag == 0){
								kick_sta = sta;
								sta=sta->next;
								bss_kick_sta(bss, kick_sta);
							}else {
								sta=sta->next;
							}
						}
					}
				}
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}
/*
DBusMessage *asd_dbus_show_wlan_mac_list0(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	
	int ret = ASD_DBUS_SUCCESS;
	unsigned int command_num=0;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
	struct acl_config *conf;
	struct maclist *entry;
	unsigned char wlanid=0;
	int i=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(ASD_WLAN[wlanid] == NULL) {
		ret = ASD_WLAN_NOT_EXIST;
	}else {
		conf = ASD_WLAN[wlanid]->acl_conf;
		if( conf != NULL ) {
			command_num = conf->num_accept_mac + 
				conf->num_deny_mac + 1;
		}

		if(command_num != 0) {
			showStr = (char*)malloc(command_num*50);
			if(NULL == showStr) {
				asd_printf(ASD_DBUS,MSG_DEBUG,("memory malloc error\n"));
				ret = ASD_DBUS_ERROR;
			} else {
					memset(showStr,0,command_num*50);
					cursor = showStr;
					
				if( conf->macaddr_acl != 0 ) {
					totalLen += sprintf(cursor,"wlan %d use %s list \n",wlanid,(conf->macaddr_acl==1)?"black":"white");
					cursor = showStr + totalLen;
				}else{
					totalLen += sprintf(cursor,"wlan %d use none list \n",wlanid);
					cursor = showStr + totalLen;
				}
				
				totalLen += sprintf(cursor,"white list: ");
				cursor = showStr + totalLen;

				for(i=0, entry=conf->accept_mac; entry; entry=entry->next, i++ ) {
					if( i%3 == 0) {
						totalLen += sprintf(cursor,"\n");
						cursor = showStr + totalLen;
					}
					totalLen += sprintf(cursor,MACSTR"\t",MAC2STR(entry->addr));
					cursor = showStr + totalLen;
				}
				
				totalLen += sprintf(cursor,"\nblack list: ");
				cursor = showStr + totalLen;
				for(i=0, entry=conf->deny_mac; entry; entry=entry->next, i++ ) {
					if( i%3 == 0) {
						totalLen += sprintf(cursor,"\n");
						cursor = showStr + totalLen;
					}
					totalLen += sprintf(cursor,MACSTR"\t",MAC2STR(entry->addr));
					cursor = showStr + totalLen;
				}
				
				totalLen += sprintf(cursor,"\n");
				cursor = showStr + totalLen;

				if(showStr!=NULL){
					if(strlen(showStr)<4){
						free(showStr);
						showStr = NULL;
					}
				}
			}
		}
	}

	if(showStr==NULL){
		showStr = (char*)malloc(1);		
		memset(showStr,0,1);
	}
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_STRING,
									&showStr); 
	free(showStr);
	showStr = NULL;
	return reply;

}
*/
DBusMessage *asd_dbus_show_wlan_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	int ret = ASD_DBUS_SUCCESS;
	struct acl_config *conf;
	unsigned char wlanid=0;
	int i=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	ret = ASD_CHECK_ID(ASD_WLAN_CHECK,(unsigned int)wlanid);
	if(ret != ASD_DBUS_SUCCESS){
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	}
	else if(ASD_WLAN[wlanid] == NULL) {
		ret = ASD_WLAN_NOT_EXIST;
		
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	}
	else {
		dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		{
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			conf = ASD_WLAN[wlanid]->acl_conf;
			
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &conf->macaddr_acl);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &conf->num_deny_mac);
			
			dbus_message_iter_append_basic (&iter_struct,
											 DBUS_TYPE_UINT32,
											 &conf->num_accept_mac);
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												   DBUS_TYPE_BYTE_AS_STRING
												   DBUS_TYPE_BYTE_AS_STRING
												   DBUS_TYPE_BYTE_AS_STRING
												   DBUS_TYPE_BYTE_AS_STRING
												   DBUS_TYPE_BYTE_AS_STRING
												   DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			for(i=0; i<2; i++){
				struct maclist *entry;
				if(i == 0)
					entry = conf->deny_mac;
				else 
					entry = conf->accept_mac;
				
				while (entry) {
					if(entry->add_reason == 0){
						u8 *addr = entry->addr;
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_open_container (&iter_sub_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_sub_struct);
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[0]));
						
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[1]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[2]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[3]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[4]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
							 	  &(addr[5]));
						
						dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					}
					entry = entry->next;
				}
			}

			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;
	
}

DBusMessage *asd_dbus_show_wtp_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	int ret = ASD_DBUS_SUCCESS;
	struct acl_config *conf;
	unsigned int wtpid=0;
	int i=0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wtpid,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	pthread_mutex_lock(&asd_g_sta_mutex);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	ret = ASD_CHECK_ID(ASD_WTP_CHECK,wtpid);
	if(ret != ASD_DBUS_SUCCESS){
		dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&ret); 
	}
	else if(wtpid >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
		dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&ret); 
	}
	else if(ASD_WTP_AP[wtpid] == NULL) {
		ret = ASD_WTP_NOT_EXIST;

		dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&ret); 
	}
	else {
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 

		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
			{
				DBusMessageIter iter_struct;
				DBusMessageIter iter_sub_array;
				conf = ASD_WTP_AP[wtpid]->acl_conf;
				
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &conf->macaddr_acl);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &conf->num_deny_mac);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &conf->num_accept_mac);
				
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				for(i=0; i<2; i++){
					struct maclist *entry;
					if(i == 0)
						entry = conf->deny_mac;
					else 
						entry = conf->accept_mac;
					
					while (entry) {
						u8 *addr = entry->addr;
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_open_container (&iter_sub_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_sub_struct);
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[0]));
						
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[1]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[2]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[3]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[4]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
							 	  &(addr[5]));
						
						dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
						
						entry = entry->next;
					}
				}

				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	return reply;

}

DBusMessage *asd_dbus_show_bss_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;

	int ret = ASD_DBUS_SUCCESS;
	struct acl_config *conf;
	unsigned int radioid=0;
	unsigned int bssindex=0;
	unsigned char wlanid=0;
	int i=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&radioid,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	for(i=radioid*L_BSS_NUM; i<(radioid+1)*L_BSS_NUM; i++){
		if((ASD_BSS[i] != NULL)&&(ASD_BSS[i]->WlanID == wlanid)){
			bssindex = i;
			break;
		}
	}
	
	if(radioid >= G_RADIO_NUM){
		ret = ASD_RADIO_ID_LARGE_THAN_MAX;
		dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&ret); 
	}
	else if(bssindex == 0){
		ret = ASD_BSS_NOT_EXIST;
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
	}
	else {
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		{
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;
		conf = ASD_BSS[bssindex]->acl_conf;
		
		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_struct);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->macaddr_acl);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_deny_mac);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_accept_mac);
		
		dbus_message_iter_open_container (&iter_struct,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_sub_array);
		for(i=0; i<2; i++){
			struct maclist *entry;
			if(i == 0)
				entry = conf->deny_mac;
			else 
				entry = conf->accept_mac;
			
			while (entry) {
				u8 *addr = entry->addr;
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(addr[5]));
				
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				
				entry = entry->next;
			}
		}

		dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
		dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;

}


DBusMessage *asd_dbus_show_all_wlan_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned char wlan_num=0;
	struct acl_config *conf;
	//struct maclist *entry;
	unsigned char wlanid[WLAN_NUM];
	int i=0, j=0;
	
	pthread_mutex_lock(&asd_g_wlan_mutex);
	pthread_mutex_lock(&asd_g_sta_mutex);
	dbus_error_init(&err);

	while(i<WLAN_NUM){
		if(ASD_WLAN[i] != NULL)	{
			wlanid[wlan_num] = ASD_WLAN[i]->WlanID;
			wlan_num++;
		}
		i++;
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"wlan_num = %d\n",wlan_num);
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &wlan_num);
	
	dbus_message_iter_open_container (&iter,
									   DBUS_TYPE_ARRAY,
									   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
									   DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);

	for (i=0; i<wlan_num; i++) {
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;
		conf = ASD_WLAN[wlanid[i]]->acl_conf;
		
		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_struct);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &wlanid[i]);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->macaddr_acl);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_deny_mac);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_accept_mac);
		
		dbus_message_iter_open_container (&iter_struct,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_sub_array);
		for(j=0; j<2; j++){
			struct maclist *entry;
			if(j == 0)
				entry = conf->deny_mac;
			else 
				entry = conf->accept_mac;
			
			while (entry) {
				if(entry->add_reason == 0){
					u8 *addr = entry->addr;
					DBusMessageIter iter_sub_struct;
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(addr[0]));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(addr[1]));

					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(addr[2]));

					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(addr[3]));

					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(addr[4]));

					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
						 	  &(addr[5]));
					
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				}
				entry = entry->next;
			}
		}
		dbus_message_iter_close_container (&iter_struct, &iter_sub_array);	
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	return reply;

}

DBusMessage *asd_dbus_show_all_wtp_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int wtp_num=0;
	struct acl_config *conf;
	//struct maclist *entry;
	unsigned int *wtpid;
	int i=0, j=0;
	
	wtpid = (unsigned int*)os_zalloc(WTP_NUM*(sizeof(unsigned int)));
	if( wtpid == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	pthread_mutex_lock(&asd_g_sta_mutex);
	dbus_error_init(&err);

	while(i<WTP_NUM){
		if(ASD_WTP_AP[i] != NULL) {
			wtpid[wtp_num] = ASD_WTP_AP[i]->WTPID;
			wtp_num++;
		}
		i++;
	}

	asd_printf(ASD_DBUS,MSG_DEBUG,"wtp_num = %d\n",wtp_num);
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &wtp_num);
	
	dbus_message_iter_open_container (&iter,
									   DBUS_TYPE_ARRAY,
									   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
									   DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);

	for (i=0; i<wtp_num; i++) {
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;
		conf = ASD_WTP_AP[wtpid[i]]->acl_conf;
		
		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_struct);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &wtpid[i]);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->macaddr_acl);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_deny_mac);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_accept_mac);
		
		dbus_message_iter_open_container (&iter_struct,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_sub_array);
		for(j=0; j<2; j++){
			struct maclist *entry;
			if(j == 0)
				entry = conf->deny_mac;
			else 
				entry = conf->accept_mac;
			
			while (entry) {
				u8 *addr = entry->addr;
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(addr[5]));
				
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				
				entry = entry->next;
			}
		}

		dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}

	dbus_message_iter_close_container (&iter, &iter_array);
	free(wtpid);
	wtpid=NULL;
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
	return reply;

}

DBusMessage *asd_dbus_show_all_bss_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int bss_num=0;
	struct acl_config *conf;
	//struct maclist *entry;
	unsigned int *bssindex;
	int i=0, j=0;
	
	bssindex = (unsigned int*)os_zalloc(BSS_NUM*(sizeof(unsigned int)));
	if( bssindex == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	pthread_mutex_lock(&asd_g_sta_mutex);
	dbus_error_init(&err);

	while(i<BSS_NUM){
		if(ASD_BSS[i] != NULL)	{
			bssindex[bss_num] = ASD_BSS[i]->BSSIndex;
			bss_num++;
		}
		i++;
	}
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"bss_num = %d\n",bss_num);
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &bss_num);
	
	dbus_message_iter_open_container (&iter,
									   DBUS_TYPE_ARRAY,
									   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING
									   DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);

	for (i=0; i<bss_num; i++) {
		DBusMessageIter iter_struct;
		DBusMessageIter iter_sub_array;
		unsigned int radio;
		unsigned char bss;
		unsigned char wlanid;		//mahz add 2011.5.11
		
		conf = ASD_BSS[bssindex[i]]->acl_conf;
		radio = ASD_BSS[bssindex[i]]->Radio_G_ID;
		bss = ASD_BSS[bssindex[i]]->BSSIndex%L_BSS_NUM + 1;
		wlanid = ASD_BSS[bssindex[i]]->WlanID;

		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_struct);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &radio);

		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_BYTE,
										 &wlanid);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->macaddr_acl);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_deny_mac);
		
		dbus_message_iter_append_basic (&iter_struct,
										 DBUS_TYPE_UINT32,
										 &conf->num_accept_mac);
		
		dbus_message_iter_open_container (&iter_struct,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											   DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_sub_array);

		for(j=0; j<2; j++){
			struct maclist *entry;
			if(j == 0)
				entry = conf->deny_mac;
			else 
				entry = conf->accept_mac;

			while (entry) {
				u8 *addr = entry->addr;
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container(&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_BYTE,
												  &(addr[0]));
				
				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_BYTE,
												  &(addr[1]));

				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_BYTE,
												  &(addr[2]));

				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_BYTE,
												  &(addr[3]));

				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_BYTE,
												  &(addr[4]));

				dbus_message_iter_append_basic(&iter_sub_struct,
												  DBUS_TYPE_BYTE,
											 	  &(addr[5]));
				
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				
				entry = entry->next;
			}
		}

		dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}

	dbus_message_iter_close_container (&iter, &iter_array);
	free(bssindex);
	bssindex=NULL;
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;
}

DBusMessage *asd_dbus_show_wlan_wids_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	int ret = ASD_DBUS_SUCCESS;
	struct acl_config *conf;
	struct maclist *entry;
	unsigned char wlanid=0;
	
	pthread_mutex_lock(&asd_g_sta_mutex);
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	ret = ASD_CHECK_ID(ASD_WLAN_CHECK,(unsigned int)wlanid);

  	if(ret != ASD_DBUS_SUCCESS) {
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	}
	else if(ASD_WLAN[wlanid] == NULL) {
		ret = ASD_WLAN_NOT_EXIST;
		
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	}
	else {
		dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
		
		conf = ASD_WLAN[wlanid]->acl_conf;
		entry = conf->deny_mac;

		asd_printf(ASD_DBUS,MSG_DEBUG,"wlan[%d] num_wids_mac =%d\n",wlanid,conf->num_wids_mac);
		asd_printf(ASD_DBUS,MSG_DEBUG,"wlan[%d] num_deny_mac =%d\n",wlanid,conf->num_deny_mac);
		dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_BYTE,
									&wids_enable); 
		dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&conf->num_wids_mac); 
		dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&wids_mac_last_time); 

		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);

		while (entry){
			if(entry->add_reason == 1){
				DBusMessageIter iter_struct;
				u8 *addr1 = entry->addr;
				u8 *addr2 = entry->vapbssid;
				
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr1[0]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr1[1]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr1[2]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr1[3]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr1[4]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr1[5]);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &entry->add_reason);

				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr2[0]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr2[1]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr2[2]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr2[3]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr2[4]);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &addr2[5]);

				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &entry->attacktype);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &entry->frametype);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &entry->channel);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_BYTE,
												 &entry->rssi);
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &(entry->add_time_sysruntime));//qiuchen change it 2012.10.31

				asd_printf(ASD_DBUS,MSG_DEBUG,MACSTR" atype:%d ftype:%d %s\n",MAC2STR(entry->addr),entry->attacktype,entry->frametype,ctime(&entry->add_time));
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}
			entry = entry->next;
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;
	
}
/*add for ac black name list by nl  2010-08-28*/
DBusMessage *asd_dbus_ac_add_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta = NULL;

	unsigned char *mac = NULL;
	//struct hostapd_data **bss = NULL;
	//struct hostapd_data * hapd = NULL;
	struct asd_data **bss = NULL;	
	bss = malloc(BSS_NUM*(sizeof(struct asd_data *)));
	if(bss == NULL)
	{
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}
	int bss_num=0;
	int i=0;
	unsigned char list_type = 0;
	
	mac = (unsigned char*)malloc(WID_MAC_LEN);
	if(mac==NULL)
	{
		if(bss != NULL)
		{
			free(bss);
			bss = NULL;
		}
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; 
		free(bss);
		bss = NULL;
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd receive ac add mac list type %d\n" ,list_type);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================
	
	if( add_mac_in_ac_maclist(&ac_acl_conf,mac,list_type) != 0 )
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"add mac failed\n");
	bss_num = ASD_SEARCH_ALL_STA(bss);
	for(i=0;i<bss_num;i++){
		if(1==list_type && (ASD_BSS[bss[i]->BSSIndex]->acl_conf != NULL)
			&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) 
			&&(ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf != NULL) 
			&& (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 1)){
			
			sta = ap_get_sta(bss[i], mac);
			if (sta != NULL){
				bss_kick_sta(bss[i], sta);
				break;
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	if(mac != NULL){
		free(mac);
		mac = NULL;
	}
	if(bss!=NULL){
		free(bss);
		bss = NULL;
	}
	return reply;	
}

/*add for ac black name list by nl  2010-08-28*/
DBusMessage *asd_dbus_ac_del_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta = NULL;

	unsigned char list_type=0;
	unsigned char *mac = NULL;
	struct asd_data **bss = NULL;	
	bss = malloc(BSS_NUM*(sizeof(struct asd_data *)));
	if(NULL == bss)
	{
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}
	int bss_num=0;
	int i=0;
	
	mac = (unsigned char*)malloc(WID_MAC_LEN);
	if(mac==NULL)
	{
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		if(bss)
		{
			os_free(bss);
			bss = NULL;
		}
		return NULL;	
	}
	memset(mac, 0, WID_MAC_LEN);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(mac);
		mac = NULL; 
		free(bss);
		bss = NULL;
		return NULL;
	}
	
	//=======================================================================
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n++++++++++++++++++++++++++++++++++++++++++\n");
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd receive MAC:\t"MACSTR"\n",MAC2STR((u8*)mac));
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd receive ac list_type :%d\n",list_type);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"++++++++++++++++++++++++++++++++++++++++++\n");
	//=======================================================================
	
	if( del_mac_in_ac_maclist(&ac_acl_conf,mac,list_type) != 0 ){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"del mac failed\n");
		ret = ASD_UNKNOWN_ID;
	}else{
		bss_num = ASD_SEARCH_ALL_STA(bss);
	
		for(i=0;i<bss_num;i++){
			if(2==list_type && (ASD_BSS[bss[i]->BSSIndex]->acl_conf != NULL)
				&&(ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) 
				&&(ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf != NULL) 
				&& (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 2)){
				sta = ap_get_sta(bss[i], mac);
				if (sta != NULL){
					bss_kick_sta(bss[i], sta);
					break;
				}
			}
		}
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	
	if(mac != NULL){
		free(mac);
		mac = NULL;
	}

	if(bss!=NULL){
		free(bss);
		bss = NULL;
	}
	
	return reply;	
}
/*add for ac black name list by nl  2010-08-28*/
DBusMessage *asd_dbus_ac_use_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	struct sta_info *sta=NULL, *kick_sta=NULL;
	struct acl_config *conf = NULL;
	unsigned char list_type=0;	//0--none,1--black list, 2--white list
	struct asd_data **bss = NULL;
	struct maclist *entry = NULL;
	macaddr mac;
	int bss_num=0;
	int i=0;
	bss = malloc(BSS_NUM*(sizeof(struct asd_data *)));
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&list_type,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(bss);
		return NULL;
	}
	
	if((wids_enable == 1) && (list_type != 1)){ 
		ret = ASD_WIDS_OPEN;
	}

	else {
		conf = &ac_acl_conf;
		if(change_maclist_security(conf,list_type) != 0)
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"change mac list failed\n");
		
		bss_num = ASD_SEARCH_ALL_STA(bss);
		for(i=0;i<bss_num;i++){
			if((ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) 
				&& (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 0) 
				&&(conf->macaddr_acl == 1)){
				
				for(entry = conf->deny_mac; entry; entry = entry->next){
					os_memcpy(mac,entry->addr,WID_MAC_LEN);
					sta = ap_get_sta(bss[i], mac);
					if (sta != NULL){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"11111 kick sta \n");
						bss_kick_sta(bss[i], sta);
					}
				}	
			}
			
			else if((ASD_BSS[bss[i]->BSSIndex]->acl_conf->macaddr_acl == 0) 
				&& (ASD_WTP_AP[bss[i]->Radio_G_ID/L_RADIO_NUM]->acl_conf->macaddr_acl == 0) 
				&&(conf->macaddr_acl == 2)){
					
				for (sta=bss[i]->sta_list; sta!=NULL; ){
					int flag = 0;
					for(entry=conf->accept_mac; entry; entry=entry->next) {
						os_memcpy(mac,entry->addr,WID_MAC_LEN);
						if (os_memcmp(sta->addr, mac, WID_MAC_LEN) == 0){
							flag = 1;
							break;
						}
					}
					if (flag == 0){
						kick_sta = sta;
						sta=sta->next;
						bss_kick_sta(bss[i], kick_sta);
					}
					else {
						sta=sta->next;
					}
				}
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	if(bss!=NULL){
		free(bss);
		bss = NULL;
	}
	return reply;	
}
/*add for ac black name list by nl  2010-08-28*/
DBusMessage *asd_dbus_show_ac_mac_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusError err;
	
	int ret = ASD_DBUS_SUCCESS;
	struct acl_config *conf = NULL;
	conf = &ac_acl_conf;
	int i=0;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	if(ret != ASD_DBUS_SUCCESS){
		dbus_message_iter_append_basic (&iter,
											DBUS_TYPE_UINT32,
											&ret); 
	}
	else {
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 

		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
			{
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &conf->macaddr_acl);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &conf->num_deny_mac);
				
				dbus_message_iter_append_basic (&iter_struct,
												 DBUS_TYPE_UINT32,
												 &conf->num_accept_mac);
				
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				for(i=0; i<2; i++){
					struct maclist *entry = NULL;
					if(i == 0)
						entry = conf->deny_mac;
					else 
						entry = conf->accept_mac;
					
					while (entry) {
						u8 *addr = entry->addr;
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_open_container (&iter_sub_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_sub_struct);
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[0]));
						
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[1]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[2]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[3]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(addr[4]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
							 	  &(addr[5]));
						
						dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
						
						entry = entry->next;
					}
				}

				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}
		dbus_message_iter_close_container (&iter, &iter_array);
	}

	return reply;

}


DBusMessage *asd_dbus_wlan_list_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	unsigned char wlan_num=0;
	unsigned int command_num=0;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
	struct acl_config *conf;
	struct maclist *entry;
	unsigned char wlanid[WLAN_NUM];
	int i=0;
	
	//int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);

	pthread_mutex_lock(&asd_g_wlan_mutex);
	while(i<WLAN_NUM){
		if(ASD_WLAN[i] != NULL)	{
			wlanid[wlan_num] = ASD_WLAN[i]->WlanID;
			wlan_num++;
		}
		i++;
	}

	for (i=0; i<wlan_num; i++) {
		if( ASD_WLAN[wlanid[i]]&&ASD_WLAN[wlanid[i]]->acl_conf != NULL ) {
			command_num += ASD_WLAN[wlanid[i]]->acl_conf->num_accept_mac + 
				ASD_WLAN[wlanid[i]]->acl_conf->num_deny_mac + 1;
		}
		command_num+=2;
	}

	if(command_num == 0){
		showStr = (char*)os_zalloc(1);		
		memset(showStr,0,1);
		asd_printf(ASD_DBUS,MSG_DEBUG,"no wlan mac list profile\n");	
	}else{
	showStr = (char*)os_zalloc(command_num*50);
	
	if(NULL == showStr){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		pthread_mutex_unlock(&asd_g_wlan_mutex);
		exit(1);
	} else {
		memset(showStr,0,command_num*50);
		cursor = showStr;
		
		for(i=0; i<wlan_num; i++){
			conf = ASD_WLAN[wlanid[i]]->acl_conf;
			if( conf != NULL ) {
				if( conf->macaddr_acl != 0 ) {
					if(wids_enable){
						if(conf->wlan_last_macaddr_acl != 0)		//mahz add 2011.3.21
							totalLen += sprintf(cursor,"wlan %d use %s list \n",wlanid[i],(conf->wlan_last_macaddr_acl==1)?"black":"white");
					}
					else
						totalLen += sprintf(cursor,"wlan %d use %s list \n",wlanid[i],(conf->macaddr_acl==1)?"black":"white");
					cursor = showStr + totalLen;
				}
				
				for(entry=conf->accept_mac; entry; entry=entry->next) {
					totalLen += sprintf(cursor,"wlan %d add white list "MACSTR"\n",wlanid[i],MAC2STR(entry->addr));
					cursor = showStr + totalLen;
				}
				
				for(entry=conf->deny_mac; entry; entry=entry->next) {
					if(entry->add_reason == 0) {
						totalLen += sprintf(cursor,"wlan %d add black list "MACSTR"\n",wlanid[i],MAC2STR(entry->addr));
						cursor = showStr + totalLen;
					}
				}

			}

			if(ASD_WLAN[wlanid[i]]->flow_compute==1){
				totalLen += sprintf(cursor,"set wlan %u flow enable\n",wlanid[i]);
				cursor = showStr + totalLen;
			}	/*xm0714*/

			
			if(ASD_WLAN[wlanid[i]]->extern_balance==1){
				totalLen += sprintf(cursor,"set wlan %u extern balance enable\n",wlanid[i]);
				cursor = showStr + totalLen;
			}	/*xm0814*/
		}
		if(showStr!=NULL){
			if(strlen(showStr)<4){
				free(showStr);
				showStr = NULL;
			}
		}
	}

	if(showStr==NULL){
		showStr = (char*)os_zalloc(1);		
		memset(showStr,0,1);
		}
	}
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_STRING,
									&showStr); 
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	free(showStr);
	showStr = NULL;
	return reply;


}

DBusMessage *asd_dbus_wtp_list_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	unsigned int wtp_num=0;
	unsigned int command_num=0;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
	struct acl_config *conf;
	struct maclist *entry;
	unsigned int *wtpid;
	int i=0;
	
	//int ret = ASD_DBUS_SUCCESS;
	wtpid = (unsigned int*)os_zalloc(WTP_NUM*(sizeof(unsigned int)));
	if( wtpid == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	dbus_error_init(&err);

	pthread_mutex_lock(&asd_g_wtp_mutex); 	//mahz add 2011.3.24
	while(i<WTP_NUM){
		if(ASD_WTP_AP[i] != NULL) {
			wtpid[wtp_num] = ASD_WTP_AP[i]->WTPID;
			wtp_num++;
		}
		i++;
	}

	for (i=0; i<wtp_num; i++) {
		if( ASD_WTP_AP[wtpid[i]]->acl_conf != NULL ) {
			command_num += ASD_WTP_AP[wtpid[i]]->acl_conf->num_accept_mac + 
				ASD_WTP_AP[wtpid[i]]->acl_conf->num_deny_mac + 1;
		}
	}

	if(command_num == 0){
		showStr = (char*)os_zalloc(1);		
		memset(showStr,0,1);
		asd_printf(ASD_DBUS,MSG_DEBUG,"no wtp mac list profile\n");	
	}else{
	showStr = (char*)os_zalloc(command_num*50);
	
	if(NULL == showStr){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	} else {
		memset(showStr,0,command_num*50);
		cursor = showStr;
		
		for(i=0; i<wtp_num; i++){
			conf = ASD_WTP_AP[wtpid[i]]->acl_conf;
			if( conf != NULL ) {
				if( conf->macaddr_acl != 0 ) {
					totalLen += sprintf(cursor,"wtp %d use %s list \n",wtpid[i],(conf->macaddr_acl==1)?"black":"white");
					cursor = showStr + totalLen;
				}
				
				for(entry=conf->accept_mac; entry; entry=entry->next) {
					totalLen += sprintf(cursor,"wtp %d add white list "MACSTR"\n",wtpid[i],MAC2STR(entry->addr));
					cursor = showStr + totalLen;
				}
				
				for(entry=conf->deny_mac; entry; entry=entry->next) {
					totalLen += sprintf(cursor,"wtp %d add black list "MACSTR"\n",wtpid[i],MAC2STR(entry->addr));
					cursor = showStr + totalLen;
				}
			}
		}
		if(showStr!=NULL){
			if(strlen(showStr)<4){
				free(showStr);
				showStr = NULL;
			}
		}
	}

	if(showStr==NULL){
		showStr = (char*)os_zalloc(1);		
		memset(showStr,0,1);
	}
	}
	pthread_mutex_unlock(&asd_g_wtp_mutex); 	//mahz add 2011.3.24
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_STRING,
									&showStr); 
	free(showStr);
	showStr = NULL;
	free(wtpid);
	wtpid = NULL;
	return reply;


}

DBusMessage *asd_dbus_bss_list_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	//unsigned int bss_num=0;
	//unsigned int command_num=0;
	char *showStr = NULL;//,*cursor = NULL;
	/*int totalLen = 0;
	struct acl_config *conf=NULL;
	struct maclist *entry=NULL;
	unsigned int *bssid=NULL;
	unsigned int radio = 0;
	unsigned char bss = 0;
	unsigned char wlanid = 0;		//mahz add 2011.5.11
	int i=0;*/
	
	//int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);

/*weichao add
because there is no bss config need to load at present
when load the bss configure file  ASD daemon has no bss list!
asd will get the black/white list from the wid
*/
#if 0
	bssid = (unsigned int*)os_zalloc(BSS_NUM*sizeof(unsigned int));
	if( bssid == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
		//exit(1);
	}	
	pthread_mutex_lock(&asd_g_sta_mutex); 
	while(i<BSS_NUM){
		if(ASD_BSS[i] != NULL)	{
			bssid[bss_num] = ASD_BSS[i]->BSSIndex;
			bss_num++;
		}
		i++;
	}

	for (i=0; i<bss_num; i++) {
		if((ASD_BSS[bssid[i]]!= NULL) && (ASD_BSS[bssid[i]]->acl_conf != NULL) ) {
			command_num += ASD_BSS[bssid[i]]->acl_conf->num_accept_mac + 
				ASD_BSS[bssid[i]]->acl_conf->num_deny_mac + 1;
		}
	}

	if(command_num == 0){
		showStr = (char*)os_zalloc(1);		
		memset(showStr,0,1);
		asd_printf(ASD_DBUS,MSG_DEBUG,"no bss mac list profile\n");	
	}else{
		showStr = (char*)os_zalloc(command_num*50);
		
		if(NULL == showStr){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			pthread_mutex_unlock(&asd_g_sta_mutex); 
			return NULL;
			//exit(1);
		} else {
			memset(showStr,0,command_num*50);
			cursor = showStr;
			

					
			if(showStr!=NULL){
				if(strlen(showStr)<4){
					free(showStr);
					showStr = NULL;
				}
			}
		}

		if(showStr==NULL){
			showStr = (char*)os_zalloc(1);		
			memset(showStr,0,1);
		}
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
#endif
	if(showStr==NULL){
		showStr = (char*)os_zalloc(1);		
		memset(showStr,0,1);
	}	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_STRING,
									&showStr); 
	if(showStr != NULL)
	{
		free(showStr);
		showStr = NULL;
	}
/*	if(bssid != NULL)
	{
		free(bssid);
		bssid = NULL;
	}*/
	return reply;
}

//========================================================
//xm 08/10/27
DBusMessage *asd_dbus_show_sta_summary(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array,iter_array1;
	DBusError err;		
	int ret = ASD_DBUS_SUCCESS;

	unsigned int total=0, wlan_n=0, wtp_n=0; 
	unsigned int local_roam_count=0,total_unconnect_count=0;
	unsigned int j=0;
	unsigned char i=0;
	unsigned int accessed_sta_num = 0;

	unsigned int wlan_count[WLAN_NUM]={0};
	unsigned int *wtp_count;
	wtp_count = (unsigned int *)os_zalloc(WTP_NUM*sizeof(unsigned int));
	if( wtp_count == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	memset(wtp_count,0,WTP_NUM*sizeof(unsigned int));
	pthread_mutex_lock(&asd_g_wlan_mutex);
	pthread_mutex_lock(&asd_g_sta_mutex);
	total=ASD_STA_SUMMARY(wtp_count,wlan_count);
	local_roam_count = local_success_roaming_count;
	total_unconnect_count = total_sta_unconnect_count;


	for(i=0;i<WLAN_NUM;i++){
		if(wlan_count[i]!=0)
			wlan_n++;
		if(ASD_WLAN[i])
			accessed_sta_num += ASD_WLAN[i]->a_num_sta;
	}

	for(j=0;j<WTP_NUM;j++){
		if(wtp_count[j]!=0)
			wtp_n++;

	}

	dbus_error_init(&err);


	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);



	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&accessed_sta_num);

	//	total
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&total);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &local_roam_count);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &total_unconnect_count);

	//	wlan		summary
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&wlan_n);


	dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
	for(i=0;i<WLAN_NUM;i++){
		
		DBusMessageIter iter_struct;

		if(wlan_count[i]!=0){

			dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_BYTE,
									&i);

			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_UINT32,
								&wlan_count[i]);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		
	}

	dbus_message_iter_close_container (&iter, &iter_array);
	


	//	wtp		summary
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&wtp_n);


	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);
	
	for(j=0;j<WTP_NUM;j++){

		DBusMessageIter iter_struct1;
		
		if(wtp_count[j]!=0){

			dbus_message_iter_open_container (&iter_array1,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct1);
			
			dbus_message_iter_append_basic (&iter_struct1,
								DBUS_TYPE_UINT32,
								&j);

			dbus_message_iter_append_basic (&iter_struct1,
								DBUS_TYPE_UINT32,
								&wtp_count[j]);

			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}

		
		
	}

	dbus_message_iter_close_container (&iter, &iter_array1);
	
	free(wtp_count);	
	wtp_count = NULL;
	pthread_mutex_unlock(&asd_g_sta_mutex);
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	return reply;	
}

DBusMessage *asd_dbus_show_stalist(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;		
	int ret = ASD_DBUS_SUCCESS;
	unsigned char SecurityID = 0;
	struct asd_data **bss;
	bss = os_zalloc(BSS_NUM*sizeof(struct asd_data *));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	int i = 0;
	unsigned int num = 0;
//=============================
// xm 08/10/08
	struct asd_data *bss_wired[vlan_max_num];  //max vlan number 500.I think it is big enough.
	unsigned int num_wired = 0;
//=============================

	asd_printf(ASD_DBUS,MSG_DEBUG,"In the asd_dbus_show_stalist\n");
	dbus_error_init(&err);
	pthread_mutex_lock(&asd_g_sta_mutex);   

		num = ASD_SEARCH_ALL_STA(bss);
		
		asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SEARCH_ALL_STA finish \n");
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
			
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_STRING_AS_STRING
														DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i = 0; i < num ; i++){			
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			//asd_printf(ASD_DBUS,MSG_DEBUG,"sta_num:%d\n",bss[i]->num_sta);
			if(ASD_WLAN[bss[i]->WlanID])
				SecurityID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						&(bss[i]->WlanID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						&(bss[i]->Radio_G_ID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						&(bss[i]->BSSIndex));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						&(SecurityID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						&(bss[i]->num_sta));
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_STRING_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			int j = 0;			
			struct sta_info *sta;
			sta = bss[i]->sta_list;
			for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
				
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[5]));
				
			  dbus_message_iter_append_basic
						(&iter_sub_struct,
						DBUS_TYPE_STRING,
						 &(sta->in_addr));
			  
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				sta = sta->next;
			}
			
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
//xm 08/10/08
		num_wired=ASD_SEARCH_ALL_WIRED_STA(bss_wired);
		
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num_wired);
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_STRING_AS_STRING
														DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i = 0; i < num_wired; i++){ 		
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			
			
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						&(bss_wired[i]->PORTID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						&(bss_wired[i]->VLANID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						&(bss_wired[i]->SecurityID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						&(bss_wired[i]->num_sta));
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_STRING_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);

			
			int j = 0;			
			struct sta_info *sta;
			sta = bss_wired[i]->sta_list;
			for(j = 0; (j < bss_wired[i]->num_sta)&&(sta!=NULL); j++){
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[5]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						DBUS_TYPE_STRING,
						 &(sta->in_addr));
			  
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				sta = sta->next;
			}
			
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	free(bss);
	bss = NULL;
	pthread_mutex_unlock(&asd_g_sta_mutex);   
	return reply;

}
DBusMessage *asd_dbus_show_bss_bssindex(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	int ret = 0;
	int Bssindex = 0;
	int R_G_ID = 0;
	int i = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_data *wasd = NULL;
	char *br_ifname = os_zalloc(IF_NAME_MAX+1);
	if(br_ifname == NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"show bss index malloc fail!\n");
		return NULL;
	}
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&Bssindex,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	R_G_ID = Bssindex/L_BSS_NUM;
	i = Bssindex%L_BSS_NUM;
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	if(interfaces == NULL || interfaces->iface[R_G_ID] == NULL || interfaces->iface[R_G_ID]->bss[i] == NULL)
	{
		ret = ASD_BSS_NOT_EXIST;
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	else{
		wasd = interfaces->iface[R_G_ID]->bss[i];
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->own_addr[0]); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->own_addr[1]); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->own_addr[2]); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->own_addr[3]); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->own_addr[4]); 
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->own_addr[5]);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->Radio_L_ID);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wasd->Radio_G_ID);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->WlanID);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wasd->SecurityID);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wasd->VLANID);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wasd->num_sta);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wasd->hotspot_id);
		memcpy(br_ifname,wasd->br_ifname,IF_NAME_MAX);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &br_ifname);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wasd->traffic_limit);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wasd->send_traffic_limit);
	}
	if(br_ifname){
		free(br_ifname);
		br_ifname = NULL;
	}
	return reply;
}
DBusMessage *asd_dbus_show_bss_summary(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_sub_struct;
	DBusError err;	

	struct asd_bss_summary_info bss_summary_wlan[WLAN_NUM];
	struct asd_bss_summary_info bss_summary_wtp[WTP_NUM+1];
	struct asd_bss_summary_info bss_summary_radio[L_BSS_NUM+1];
	
	memset(bss_summary_wlan,0,WLAN_NUM*sizeof(struct asd_bss_summary_info));
	memset(bss_summary_wtp ,0,(WTP_NUM+1)*sizeof(struct asd_bss_summary_info));
	memset(bss_summary_radio,0,(L_BSS_NUM+1)*sizeof(struct asd_bss_summary_info));

	char type = 0;
	int ret = 0;
	int id = 0;
	int Total_bss_num = 0;
	int circlenum = 0;
	int i = 0;
	struct bss_summary_info *tmp;
	struct bss_summary_info *tmp_free;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_UINT32,&id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);	

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);

	
	if(type == 1){
		Total_bss_num = ASD_GET_BSS_BYWLAN(bss_summary_wlan,id,&circlenum);
		if(Total_bss_num == -1){
			ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 
			pthread_mutex_unlock(&asd_g_sta_mutex);
			return reply;
		}
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &Total_bss_num); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &circlenum); 
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i=0;i<WLAN_NUM;i++){
			if(bss_summary_wlan[i].bss_list != NULL){
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(bss_summary_wlan[i].ID));
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(bss_summary_wlan[i].local_bss_num));
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				tmp = bss_summary_wlan[i].bss_list;
				while(tmp){
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[0]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[1]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[2]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[3]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[4]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[5]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->WLANID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->RGID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->sta_num));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->BSSINDEX));
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					tmp_free = tmp;
					tmp = tmp->next;
					os_free(tmp_free);
				}
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);

			}
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	else if(type == 2){
		Total_bss_num = ASD_GET_BSS_BYWTP(bss_summary_wtp,id,&circlenum);

		if(Total_bss_num == -1){
			ret = ASD_DBUS_MALLOC_FAIL;
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 
			pthread_mutex_unlock(&asd_g_sta_mutex);
			return reply;
		}
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &Total_bss_num); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &circlenum); 

		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i=0;i<WTP_NUM+1;i++){
			if(bss_summary_wtp[i].bss_list != NULL){
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss_summary_wtp[i].ID));
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss_summary_wtp[i].local_bss_num));
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				tmp = bss_summary_wtp[i].bss_list;
				while(tmp){
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[0]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[1]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[2]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[3]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[4]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->BSSID[5]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->WLANID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->RGID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->sta_num));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								&(tmp->BSSINDEX));
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					tmp_free = tmp;
					tmp = tmp->next;
					os_free(tmp_free);
				}
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
		
			}
		}
		dbus_message_iter_close_container (&iter, &iter_array);

	}
	else if(type == 3){
		if(id == 0)
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"show bss summary: type = %d\n,but id = %d",type,id);
		else{
			Total_bss_num = ASD_GET_BSS_BYRADIO(bss_summary_radio,id,&circlenum);
			if(Total_bss_num == -1){
				ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &ret); 
				pthread_mutex_unlock(&asd_g_sta_mutex);
				return reply;
			}
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &Total_bss_num); 
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &circlenum); 
			
			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
														DBUS_STRUCT_END_CHAR_AS_STRING
											   DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_array);
			for(i=0;i<1;i++){
				if(bss_summary_radio[i].bss_list != NULL){
					dbus_message_iter_open_container (&iter_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_struct);
					dbus_message_iter_append_basic
								(&iter_struct,
								  DBUS_TYPE_UINT32,
								&(bss_summary_radio[i].ID));
					dbus_message_iter_append_basic
								(&iter_struct,
								  DBUS_TYPE_UINT32,
								&(bss_summary_radio[i].local_bss_num));
					dbus_message_iter_open_container (&iter_struct,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
													   DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_sub_array);
					tmp = bss_summary_radio[i].bss_list;
					while(tmp){
						dbus_message_iter_open_container (&iter_sub_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_sub_struct);
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_BYTE,
									&(tmp->BSSID[0]));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_BYTE,
									&(tmp->BSSID[1]));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_BYTE,
									&(tmp->BSSID[2]));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_BYTE,
									&(tmp->BSSID[3]));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_BYTE,
									&(tmp->BSSID[4]));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_BYTE,
									&(tmp->BSSID[5]));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_UINT32,
									&(tmp->WLANID));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_UINT32,
									&(tmp->RGID));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_UINT32,
									&(tmp->sta_num));
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									  DBUS_TYPE_UINT32,
									&(tmp->BSSINDEX));
						dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
						tmp_free = tmp;
						tmp = tmp->next;
						os_free(tmp_free);
					}
					dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
					dbus_message_iter_close_container (&iter_array, &iter_struct);
			
				}
			}
			dbus_message_iter_close_container (&iter, &iter_array);
		}
	}
	else
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"show bss summary: type = %d\n",type);
	pthread_mutex_unlock(&asd_g_sta_mutex);  
	return reply;
}
DBusMessage *asd_dbus_show_roaming_sta_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_sub_struct;
	DBusError err;	

	char type = 0;
	int ret = 0;
	int i = 0;
	int circlenum = 0;
	unsigned int Total_r_sta_num = 0;
	struct r_sta_info *tmp = NULL;
	struct r_sta_info *tmp_free = NULL;
	struct r_sta_wlan_info r_sta_wlan[WLAN_NUM];
	struct r_sta_wlan_info r_sta_wtp[WTP_NUM+1];
	struct r_sta_wlan_info r_sta_class[3];
	memset(r_sta_wlan,0,WLAN_NUM*sizeof(struct r_sta_wlan_info));
	memset(r_sta_wtp ,0,(WTP_NUM+1)*sizeof(struct r_sta_wlan_info));
	memset(r_sta_class,0,3*sizeof(struct r_sta_wlan_info));

	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);   
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 
	if(type == 1){
		Total_r_sta_num = ASD_GET_R_STA_BYWLAN(r_sta_wlan);
		circlenum = ASD_GET_CNUM(r_sta_wlan,type);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"********Total_r_sta_num:%d,circlenum:%d,line:%d********\n",Total_r_sta_num,circlenum,__LINE__);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &Total_r_sta_num); 

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &circlenum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);

		for(i=0;i<WLAN_NUM;i++){
			if(r_sta_wlan[i].r_sta_list != NULL){
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(i));
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(r_sta_wlan[i].roaming_sta_num));
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				tmp = r_sta_wlan[i].r_sta_list;
				while(tmp){
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[0]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[1]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[2]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[3]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[4]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[5]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->r_type));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->preAPID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->curAPID));
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					tmp_free = tmp;
					tmp = tmp->next;
					os_free(tmp_free);
				}
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);

			}
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	else if(type == 2){
		Total_r_sta_num = ASD_GET_R_STA_BYCLASS(r_sta_class);
		circlenum = ASD_GET_CNUM(r_sta_class,type);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"********Total_r_sta_num:%d,circlenum:%d,line:%d********\n",Total_r_sta_num,circlenum,__LINE__);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &Total_r_sta_num); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &circlenum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);

		for(i=0;i<3;i++){
			if(r_sta_class[i].r_sta_list != NULL){
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(i));
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(r_sta_class[i].roaming_sta_num));
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				tmp = r_sta_class[i].r_sta_list;
				while(tmp){
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[0]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[1]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[2]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[3]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[4]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[5]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->r_type));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->preAPID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->curAPID));
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					tmp_free = tmp;
					tmp = tmp->next;
					os_free(tmp_free);
				}
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);

			}
		}
		dbus_message_iter_close_container (&iter, &iter_array);
		
	}
	else if(type == 3){
		Total_r_sta_num = ASD_GET_R_STA_BYWTP(r_sta_wtp);
		circlenum = ASD_GET_CNUM(r_sta_wtp,type);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"********Total_r_sta_num:%d,circlenum:%d,line:%d********\n",Total_r_sta_num,circlenum,__LINE__);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &Total_r_sta_num); 
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &circlenum); 
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
													DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);

		for(i=0;i<WTP_NUM+1;i++){
			if(r_sta_wtp[i].r_sta_list != NULL){
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(i));
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
						  	&(r_sta_wtp[i].roaming_sta_num));
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				tmp = r_sta_wtp[i].r_sta_list;
				while(tmp){
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[0]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[1]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[2]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[3]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[4]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->STA_MAC[5]));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->r_type));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->preAPID));
					dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								&(tmp->curAPID));
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					tmp_free = tmp;
					tmp = tmp->next;
					os_free(tmp_free);
				}
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);

			}
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	else
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"show roaming sta type is %d\n",type);

	pthread_mutex_unlock(&asd_g_sta_mutex);   
	return reply;
}

DBusMessage *asd_dbus_show_stalist_by_group(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;		
	int pae_state, backend_state, pae_state_wired, backend_state_wired;
	unsigned char SecurityID;
	char *in_addr = NULL;
	unsigned char eap_type = 0;
	unsigned char *identify; 
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*sizeof(struct asd_data *));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	
	memset(bss,0,BSS_NUM*sizeof(struct asd_data *));
	identify = os_malloc(5);
	memset(identify,0,5);
	memcpy(identify,"none",4);
	int i = 0;
	unsigned int num = 0;
	unsigned int auth_num = 0;		//ht 110225
	unsigned int auth_fail_num = 0;
	unsigned int assoc_num = 0;		//ht 090213
	unsigned int reassoc_num = 0;
	unsigned int assoc_failure_num = 0;
	unsigned int reassoc_failure_num = 0;
	unsigned int normal_down_num = 0;  //nl091125
	unsigned int abnormal_down_num = 0;//nl091125
	int ret = ASD_DBUS_SUCCESS;
//=============================
// xm 08/10/08
	struct asd_data *bss_wired[vlan_max_num] = {NULL};  //max vlan number 500.I think it is big enough.
	unsigned int num_wired = 0;
//=============================

	asd_printf(ASD_DBUS,MSG_DEBUG,"In the asd_dbus_show_stalist\n");
	dbus_error_init(&err);
	pthread_mutex_lock(&asd_g_sta_mutex);   

		num = ASD_SEARCH_ALL_STA(bss);
		
		for(i=0; i<num; i++) {
			auth_num += bss[i]->usr_auth_tms;
			auth_fail_num += bss[i]->auth_fail;
			assoc_num += bss[i]->num_assoc;
			reassoc_num += bss[i]->num_reassoc;
			assoc_failure_num += bss[i]->num_assoc_failure;
			reassoc_failure_num += bss[i]->num_reassoc_failure;
			normal_down_num += bss[i]->normal_st_down_num;
			abnormal_down_num += bss[i]->abnormal_st_down_num;
		}

		asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SEARCH_ALL_STA finish \n");
		reply = dbus_message_new_method_return(msg);
		
		dbus_message_iter_init_append (reply, &iter);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
		
		//UpdateStaInfoToWSM(NULL,NULL,STA_INFO);	//ht add,081027
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_num);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &auth_fail_num);

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_num);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_num);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &assoc_failure_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &reassoc_failure_num);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &normal_down_num);

		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
												 &abnormal_down_num);
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING		//ht add,090213
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_STRING_AS_STRING
													DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING//
															DBUS_TYPE_UINT32_AS_STRING//sta_online_time
															DBUS_TYPE_STRING_AS_STRING
															DBUS_TYPE_UINT64_AS_STRING
															DBUS_TYPE_UINT64_AS_STRING//ht add,081025
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_STRING_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING//sta_access_time
															DBUS_TYPE_UINT32_AS_STRING//online_time
														DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i = 0; i < num ; i++){			
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			char *Binding_IF_NAME = NULL;
			unsigned int wtpid = 0;
			unsigned int securitytype = 0;
			//asd_printf(ASD_DBUS,MSG_DEBUG,"sta_num:%d\n",bss[i]->num_sta);
			if(ASD_WLAN[bss[i]->WlanID])
				SecurityID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
			if(ASD_SECURITY[SecurityID])
				securitytype = ASD_SECURITY[SecurityID]->securityType;
			wtpid = bss[i]->Radio_G_ID/4;
			Binding_IF_NAME = (char *)os_malloc(sizeof(char)*ETH_IF_NAME_LEN);
			os_memset(Binding_IF_NAME,0,ETH_IF_NAME_LEN);
			if(ASD_WTP_AP[wtpid])
				os_memcpy(Binding_IF_NAME,ASD_WTP_AP[wtpid]->Binding_IF_NAME,ETH_IF_NAME_LEN);
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
					  	&(bss[i]->WlanID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->traffic_limit));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->send_traffic_limit));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->Radio_G_ID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->BSSIndex));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
					  	&(SecurityID));
			
			dbus_message_iter_append_basic			//ht add,090213
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->num_assoc));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->num_reassoc));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->num_assoc_failure));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss[i]->num_sta));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_STRING,
					  	&Binding_IF_NAME);
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING/////
													   DBUS_TYPE_UINT32_AS_STRING//sta_online_time
													   DBUS_TYPE_STRING_AS_STRING
														DBUS_TYPE_UINT64_AS_STRING
														DBUS_TYPE_UINT64_AS_STRING//ht add,081025
													   //DBUS_TYPE_STRING_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_STRING_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING//sta_access_time
														DBUS_TYPE_UINT32_AS_STRING//online_time
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			int j = 0;			
			struct sta_info *sta;
			time_t sta_time;
			sta = bss[i]->sta_list;
			for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
				
				if(sta->eapol_sm != NULL){
					pae_state = sta->eapol_sm->auth_pae_state;
					backend_state = sta->eapol_sm->be_auth_state;	
				}else{
					pae_state = 255;
					backend_state = 255;
				}	

//=====================================
				sta_time=sta->add_time_sysruntime;//qiuchen change it 2012.10.31
//=====================================
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(sta->addr[5]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->vlan_id));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->sta_traffic_limit));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->sta_send_traffic_limit));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->flags));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(pae_state));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(backend_state));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta_time));//height
				//qiuchen add it
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->sta_online_time));
				//end

					in_addr = (char*)malloc(strlen(sta->in_addr)+1);
					os_memset(in_addr,0,strlen(sta->in_addr)+1);
					os_memcpy(in_addr,sta->in_addr,strlen(sta->in_addr));
			  dbus_message_iter_append_basic
						(&iter_sub_struct,
			  			DBUS_TYPE_STRING,
		 	 			 &(in_addr));
			  
			  dbus_message_iter_append_basic
						(&iter_sub_struct,
			  			DBUS_TYPE_UINT64,
		 	 			 &(sta->rxbytes));
			  
			  dbus_message_iter_append_basic
						(&iter_sub_struct,
			  			DBUS_TYPE_UINT64,
		 	 			 &(sta->txbytes));	//ht add,081025

				if(sta->security_type != NO_NEED_AUTH)
					dbus_message_iter_append_basic
								  (&iter_sub_struct,
								   DBUS_TYPE_UINT32,
								   &(sta->security_type));
				else 
					dbus_message_iter_append_basic
								  (&iter_sub_struct,
								   DBUS_TYPE_UINT32,
								   &(securitytype));
				
				 if((sta->eapol_sm != NULL)&&(sta->eapol_sm->identity != NULL)){
					 dbus_message_iter_append_basic
							   (&iter_sub_struct,
							   DBUS_TYPE_BYTE,
								&(sta->eapol_sm->eap_type_authsrv));
					 
					 dbus_message_iter_append_basic
							   (&iter_sub_struct,
							   DBUS_TYPE_STRING,
								&(sta->eapol_sm->identity));
				 }
				else{
					  dbus_message_iter_append_basic
								(&iter_sub_struct,
								DBUS_TYPE_BYTE,
								 &(eap_type));
					  
					  dbus_message_iter_append_basic
								(&iter_sub_struct,
								DBUS_TYPE_STRING,
								 &(identify));
				}
				
				//qiuchen add it for AXSSZFI-1373
				time_t sta_access_time,online_time,nowsys;
				get_sysruntime(&nowsys);
				online_time = nowsys - sta->add_time_sysruntime;
				sta_access_time = time(NULL) - online_time;
				
				dbus_message_iter_append_basic (&iter_sub_struct,
												 DBUS_TYPE_UINT32,
												 &(sta_access_time));
				
				dbus_message_iter_append_basic (&iter_sub_struct,
												 DBUS_TYPE_UINT32,
												 &(online_time));
				//end

				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				sta = sta->next;
				if(in_addr){
					free(in_addr);
					in_addr = NULL;
				}
			}
			
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
			if(Binding_IF_NAME)
			{
			free(Binding_IF_NAME);
			Binding_IF_NAME = NULL;
			}
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
//xm 08/10/08
		num_wired=ASD_SEARCH_ALL_WIRED_STA(bss_wired);
		
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num_wired);
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING//
															DBUS_TYPE_UINT32_AS_STRING//sta_online_time
															DBUS_TYPE_STRING_AS_STRING
															DBUS_TYPE_UINT64_AS_STRING
															DBUS_TYPE_UINT64_AS_STRING//ht add,081025
														DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		//int i = 0;
		for(i = 0; i < num_wired; i++){			
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			
			
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss_wired[i]->PORTID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss_wired[i]->VLANID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
					  	&(bss_wired[i]->SecurityID));
			
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
					  	&(bss_wired[i]->num_sta));
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING
													   DBUS_TYPE_UINT32_AS_STRING/////
													   DBUS_TYPE_UINT32_AS_STRING//sta_online_time
													   DBUS_TYPE_STRING_AS_STRING
														DBUS_TYPE_UINT64_AS_STRING
														DBUS_TYPE_UINT64_AS_STRING//ht add,081025
													   //DBUS_TYPE_STRING_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);

			
			int j = 0;			
			struct sta_info *sta;
			time_t sta_time;
			sta = bss_wired[i]->sta_list;
			for(j = 0; (j < bss_wired[i]->num_sta)&&(sta!=NULL); j++){
				
				if(sta->eapol_sm != NULL){
					pae_state_wired= sta->eapol_sm->auth_pae_state;
					backend_state_wired= sta->eapol_sm->be_auth_state;	
				}else{
					pae_state_wired= 255;
					backend_state_wired= 255;
				}	

				sta_time=sta->add_time_sysruntime;//qiuchen change it 2012.10.31

				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));

				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_BYTE,
					 	  &(sta->addr[5]));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->flags));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(pae_state_wired));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(backend_state_wired));
				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta_time));//height

				//qiuchen add it
				dbus_message_iter_append_basic
						(&iter_sub_struct,
						  DBUS_TYPE_UINT32,
					 	  &(sta->sta_online_time));
				//end				
				dbus_message_iter_append_basic
						(&iter_sub_struct,
			  			DBUS_TYPE_STRING,
		 	 			 &(sta->in_addr));
			  
			  dbus_message_iter_append_basic
						(&iter_sub_struct,
			  			DBUS_TYPE_UINT64,
		 	 			 &(sta->rxbytes));
			  
			  dbus_message_iter_append_basic
						(&iter_sub_struct,
			  			DBUS_TYPE_UINT64,
		 	 			 &(sta->txbytes));	//ht add,081025
				
				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				sta = sta->next;
			}
			
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);
	if(bss)
	{
		free(bss);
		bss=NULL;
	}
	if(identify){
		free(identify);
		identify = NULL;
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);   

	return reply;

}

DBusMessage *asd_dbus_show_sta_base_info(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;		
	
	unsigned char *mac = NULL;
	mac = (unsigned char*)malloc(WID_MAC_LEN);
	if(mac==NULL)
		return NULL;
	memset(mac, 0, WID_MAC_LEN);

	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*sizeof(struct asd_data *));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		if(mac)
		{
			free(mac);
			mac = NULL;
		}
		return NULL;	
	}	
	memset(bss,0,BSS_NUM*sizeof(struct asd_data *));
	int i = 0,k = 0;
	unsigned int num = 0;
	int ret = ASD_DBUS_SUCCESS;

	asd_printf(ASD_DBUS,MSG_DEBUG,"In the asd_dbus_show_sta_base_info\n");
	dbus_error_init(&err);
	pthread_mutex_lock(&asd_g_sta_mutex);   

		num = ASD_SEARCH_ALL_STA(bss);
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&ret); 
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&num);
		
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING//ESSID
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING//ESSID
													DBUS_TYPE_ARRAY_AS_STRING
														DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_STRING_AS_STRING
														DBUS_STRUCT_END_CHAR_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for(i = 0; i < num ; i++){			
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			unsigned int wtpid = 0;
			char essid[ESSID_DEFAULT_LEN] = {0};/*中文essid不能用字符串传输，会导致dbus线程退出AXSSZFI-1658*/
			asd_printf(ASD_DBUS,MSG_DEBUG,"sta_num:%d\n",bss[i]->num_sta);
			wtpid = bss[i]->Radio_G_ID/4;
			if(wtpid < WTP_NUM && ASD_WTP_AP[wtpid] != NULL){
				memset(mac, 0, WID_MAC_LEN);
				memcpy(mac, ASD_WTP_AP[wtpid]->WTPMAC, WID_MAC_LEN);
			}
			if(bss[i]->WlanID < WLAN_NUM && ASD_WLAN[bss[i]->WlanID] != NULL && ASD_WLAN[bss[i]->WlanID]->ESSID != NULL)
				memcpy(essid,ASD_WLAN[bss[i]->WlanID]->ESSID,strlen(ASD_WLAN[bss[i]->WlanID]->ESSID));
			
			dbus_message_iter_open_container (&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(bss[i]->Radio_G_ID));
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(bss[i]->num_sta));
			
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(mac[0]));	
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(mac[1]));	
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(mac[2]));	
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(mac[3]));	
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(mac[4]));	
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(mac[5]));	

			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(bss[i]->WlanID));
			for(k=0;k<ESSID_DEFAULT_LEN;k++)
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE,&(essid[k]));
			
			dbus_message_iter_open_container (&iter_struct,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_BYTE_AS_STRING
													   DBUS_TYPE_STRING_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_array);
			int j = 0;			
			struct sta_info *sta;
			sta = bss[i]->sta_list;
			char *in_addr = NULL;
			for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_open_container (&iter_sub_array, DBUS_TYPE_STRUCT, NULL, &iter_sub_struct);
				
				dbus_message_iter_append_basic(&iter_sub_struct,  DBUS_TYPE_BYTE, &(sta->addr[0]));
				dbus_message_iter_append_basic(&iter_sub_struct,  DBUS_TYPE_BYTE, &(sta->addr[1]));
				dbus_message_iter_append_basic(&iter_sub_struct,  DBUS_TYPE_BYTE, &(sta->addr[2]));
				dbus_message_iter_append_basic(&iter_sub_struct,  DBUS_TYPE_BYTE, &(sta->addr[3]));
				dbus_message_iter_append_basic(&iter_sub_struct,  DBUS_TYPE_BYTE, &(sta->addr[4]));
				dbus_message_iter_append_basic(&iter_sub_struct,  DBUS_TYPE_BYTE, &(sta->addr[5]));
				
				in_addr = (char*)malloc(strlen(sta->in_addr)+1);
				if(in_addr){
					os_memset(in_addr,0,strlen(sta->in_addr)+1);
					os_memcpy(in_addr,sta->in_addr,strlen(sta->in_addr));
				}else{
					in_addr = " ";
				}
 			 	dbus_message_iter_append_basic(&iter_sub_struct, DBUS_TYPE_STRING, &(in_addr));

				dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				sta = sta->next;
				if(in_addr){
					free(in_addr);
					in_addr = NULL;
				}
			}
			
			dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}		
		dbus_message_iter_close_container (&iter, &iter_array);

	if(mac){
		free(mac);
		mac=NULL;
	}
	if(bss){
		free(bss);
		bss=NULL;
	}
	pthread_mutex_unlock(&asd_g_sta_mutex);   

	return reply;
}

//fengwenchao add 20110113 for dot11WlanStationTable
DBusMessage *asd_dbus_all_of_wlan_stalist(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	DBusMessageIter iter_sub_sub_array;
	DBusMessageIter iter_sub_sub_struct;	
	DBusError err;		
	dbus_error_init(&err);	


	int i = 0;
	int j = 0;
	int wlan_num = 0;
	int ret = ASD_DBUS_SUCCESS;

	WID_WLAN **WLAN;
	WLAN = os_zalloc(WLAN_NUM*sizeof(struct WID_WLAN *));
	if(WLAN == NULL)
	{
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}
	//struct asd_data *bss[BSS_NUM];
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		if(WLAN)
		{
			free(WLAN);
			WLAN= NULL;
		}
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		return NULL;
	}

	pthread_mutex_lock(&asd_g_wlan_mutex);
	pthread_mutex_lock(&asd_g_sta_mutex);

	while(i<WLAN_NUM)
	{
		if(ASD_WLAN[i] != NULL)
		{
			WLAN[wlan_num] = ASD_WLAN[i];
			wlan_num++;
		}
		i++;
	}

	if(wlan_num ==0)
		ret = ASD_WLAN_NOT_EXIST;	

	reply = dbus_message_new_method_return(msg);		
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &wlan_num); 
	
	dbus_message_iter_open_container (&iter,
									   DBUS_TYPE_ARRAY,
									   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING               //wlanid
												DBUS_TYPE_UINT32_AS_STRING             //bss_num
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING        //sta_num
													DBUS_TYPE_ARRAY_AS_STRING
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING       //stamac[0]
														DBUS_TYPE_BYTE_AS_STRING       //stamac[1]
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[2]	
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[3]
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[4]	
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[5]	
														DBUS_TYPE_UINT32_AS_STRING     //sta_time
														DBUS_TYPE_UINT32_AS_STRING	   //sta_online_time  qiuchen add it
														DBUS_TYPE_UINT32_AS_STRING     //sta_traffic_limit
														DBUS_TYPE_UINT32_AS_STRING     //sta_send_traffic_limit
														DBUS_TYPE_UINT64_AS_STRING     //rxbytes
														DBUS_TYPE_UINT64_AS_STRING     //txbytes
														DBUS_TYPE_UINT32_AS_STRING	   //sta_access_time  qiuchen add it
														DBUS_TYPE_UINT32_AS_STRING	   //online_time
													DBUS_STRUCT_END_CHAR_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING	
									   DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);	
	for(i=0;i<wlan_num;i++)
	{
		unsigned char wlanid = 0;
		wlanid = WLAN[i]->WlanID;
		int bss_num = 0;
		bss_num = ASD_SEARCH_WLAN_STA(wlanid, bss);	
		
		dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
								  								   								   
		dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_BYTE,&(wlanid));

							
		dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32,&(bss_num));	


		dbus_message_iter_open_container (&iter_struct,
										   DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING		  //sta_num
												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_BYTE_AS_STRING	   //stamac[0]
													DBUS_TYPE_BYTE_AS_STRING	   //stamac[1]
													DBUS_TYPE_BYTE_AS_STRING	   //stamac[2]	
													DBUS_TYPE_BYTE_AS_STRING	   //stamac[3]
													DBUS_TYPE_BYTE_AS_STRING	   //stamac[4]	
													DBUS_TYPE_BYTE_AS_STRING	   //stamac[5]	
													DBUS_TYPE_UINT32_AS_STRING	   //sta_time
													DBUS_TYPE_UINT32_AS_STRING	   //sta_online_time  qiuchen add it
													DBUS_TYPE_UINT32_AS_STRING	   //sta_traffic_limit
													DBUS_TYPE_UINT32_AS_STRING	   //sta_send_traffic_limit
													DBUS_TYPE_UINT64_AS_STRING     //rxbytes
													DBUS_TYPE_UINT64_AS_STRING     //txbytes
													DBUS_TYPE_UINT32_AS_STRING	   //sta_access_time  qiuchen add it
													DBUS_TYPE_UINT32_AS_STRING	   //online_time
												DBUS_STRUCT_END_CHAR_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,	
										   &iter_sub_array);

		for(j=0;j<bss_num;j++)
		{
		
			dbus_message_iter_open_container (&iter_sub_array,DBUS_TYPE_STRUCT,NULL,&iter_sub_struct);
													   													   													   
			dbus_message_iter_append_basic(&iter_sub_struct,DBUS_TYPE_UINT32,&(bss[j]->num_sta));
							

			dbus_message_iter_open_container (&iter_sub_struct,
											   DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[0]
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[1]
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[2]	
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[3]
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[4]	
														DBUS_TYPE_BYTE_AS_STRING	   //stamac[5]	
														DBUS_TYPE_UINT32_AS_STRING	   //sta_time
														DBUS_TYPE_UINT32_AS_STRING     //sta_online_time  qiuchen add it
														DBUS_TYPE_UINT32_AS_STRING	   //sta_traffic_limit
														DBUS_TYPE_UINT32_AS_STRING	   //sta_send_traffic_limit
														DBUS_TYPE_UINT64_AS_STRING     //rxbytes
														DBUS_TYPE_UINT64_AS_STRING     //txbytes					
														DBUS_TYPE_UINT32_AS_STRING	   //sta_access_time  qiuchen add it
														DBUS_TYPE_UINT32_AS_STRING	   //online_time
													DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_sub_sub_array);		


			int k = 0;			
			struct sta_info *sta = NULL;
			sta = bss[j]->sta_list;

			//=====================================
							time_t sta_time;
			//=====================================
			
			
			for(k = 0; (k < bss[j]->num_sta)&&(sta!=NULL); k++)
			{
				


				sta_time=sta->add_time_sysruntime;//qiuchen change it 2012.10.31
				
				dbus_message_iter_open_container (&iter_sub_sub_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_sub_sub_struct);
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[0]));
				
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[1]));
	
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[2]));
	
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[3]));
	
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[4]));
	
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_BYTE,
						  &(sta->addr[5]));
								
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(sta_time));
				//qiuchen add  it
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(sta->sta_online_time));
				//end
				
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(sta->sta_traffic_limit));
				
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT32,
						  &(sta->sta_send_traffic_limit));
				
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT64,
						  &(sta->rxbytes));
				
				dbus_message_iter_append_basic
						(&iter_sub_sub_struct,
						  DBUS_TYPE_UINT64,
						  &(sta->txbytes));				
				
				//qiuchen add it for AXSSZFI-1373
				time_t sta_access_time,online_time,nowsys;
				get_sysruntime(&nowsys);
				online_time = nowsys - sta->add_time_sysruntime;
				sta_access_time = time(NULL) - online_time;
				
				dbus_message_iter_append_basic (&iter_sub_sub_struct,
												 DBUS_TYPE_UINT32,
												 &(sta_access_time));
				
				dbus_message_iter_append_basic (&iter_sub_sub_struct,
												 DBUS_TYPE_UINT32,
												 &(online_time));
				//end

				dbus_message_iter_close_container (&iter_sub_sub_array, &iter_sub_sub_struct);
				sta = sta->next;
			}										  
			dbus_message_iter_close_container (&iter_sub_struct, &iter_sub_sub_array);				 
			dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);			
		}
		dbus_message_iter_close_container (&iter_struct,&iter_sub_array);
		dbus_message_iter_close_container (&iter_array,&iter_struct);	
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	
	if(WLAN){
		free(WLAN);
		WLAN = NULL;
	}
	if(bss){
		free(bss);
		bss = NULL;
	}
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	pthread_mutex_unlock(&asd_g_sta_mutex);
	return reply;	
}
//fengwenchao add end
DBusMessage *asd_dbus_wlan_stalist(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char WlanID;
	int pae_state, backend_state;
	unsigned char SecurityID;
	unsigned int assoc_num = 0;		//ht 090213
	unsigned int reassoc_num = 0;
	unsigned int assoc_failure_num = 0;
	unsigned normal_st_down_num = 0;//nl091120
	unsigned abnormal_st_down_num = 0;//


	DBusError err;	
	struct asd_data **bss;
	bss = os_zalloc(BSS_NUM*sizeof(struct asd_data *));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}	

	unsigned int num = 0;
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&WlanID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(bss)
		{
			free(bss);
			bss = NULL;
		}
		return NULL;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);
	if(ASD_WLAN[WlanID] == NULL)
	{	
		ret = ASD_WLAN_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 				
	}else{
		
			num = ASD_SEARCH_WLAN_STA(WlanID, bss);	

			for(i=0; i<num; i++) {
				assoc_num += bss[i]->num_assoc;
				reassoc_num += bss[i]->num_reassoc;
				assoc_failure_num += bss[i]->num_assoc_failure;
				normal_st_down_num += bss[i]->normal_st_down_num;
				abnormal_st_down_num += bss[i]->abnormal_st_down_num;
				if(bss[i]->abnormal_st_down_num >= bss[i]->normal_st_down_num)
					abnormal_st_down_num -= bss[i]->normal_st_down_num;
			}
				
			reply = dbus_message_new_method_return(msg);
			
			dbus_message_iter_init_append (reply, &iter);
		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 		
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &num);

			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &assoc_num);

			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &reassoc_num);

			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &assoc_failure_num);

			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &normal_st_down_num);
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &abnormal_st_down_num);
			
			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING		//ht add,090213
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_ARRAY_AS_STRING
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING//////heith
																DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
																DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
																DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
															DBUS_STRUCT_END_CHAR_AS_STRING
											   DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_array);
			for(i = 0; i < num ; i++){			
				DBusMessageIter iter_struct;
				DBusMessageIter iter_sub_array;
				//asd_printf(ASD_DBUS,MSG_DEBUG,"sta_num:%d\n",bss[i]->num_sta);
				if(ASD_WLAN[bss[i]->WlanID])
					SecurityID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							&(bss[i]->WlanID));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->Radio_G_ID));
		
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->BSSIndex));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							&(SecurityID));

				dbus_message_iter_append_basic			//ht add,090213
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_assoc));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_reassoc));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_assoc_failure));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_sta));
				
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING/////heigh
														   DBUS_TYPE_UINT32_AS_STRING  //sta_online_time  qiuchen add it
													DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				int j = 0;			
				struct sta_info *sta;
				sta = bss[i]->sta_list;

//=====================================
				time_t sta_time;
//=====================================


				for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
					
					if(sta->eapol_sm != NULL){
						pae_state = sta->eapol_sm->auth_pae_state;
						backend_state = sta->eapol_sm->be_auth_state;	
					}else{
						pae_state = 255;
						backend_state = 255;
					}	

				 	sta_time=sta->add_time_sysruntime;//qiuchen change it 2012.10.31
					
					DBusMessageIter iter_sub_struct;
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[0]));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[1]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[2]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[3]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[4]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[5]));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(sta->flags));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(pae_state));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(backend_state));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(sta_time));

					//qiuchen add it
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(sta->sta_online_time));
					//end
					//qiuchen add it for AXSSZFI-1373
					time_t sta_access_time,online_time,nowsys;
					get_sysruntime(&nowsys);
					online_time = nowsys - sta->add_time_sysruntime;
					sta_access_time = time(NULL) - online_time;
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(sta_access_time));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(online_time));
					//end

					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					sta = sta->next;
				}
				
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}		
			dbus_message_iter_close_container (&iter, &iter_array);
		}
		free(bss);
		bss = NULL;
		pthread_mutex_unlock(&asd_g_sta_mutex);
		return reply;
}

DBusMessage *asd_dbus_extend_wtp_stalist(DBusConnection *conn, DBusMessage *msg, void *user_data){
		DBusMessage* reply; 
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		unsigned int WTPID,acc_tms=0,auth_tms=0,repauth_tms=0;
		DBusError err;		
		
		
		unsigned int deny_num = 0; 	
		struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
		unsigned int num = 0;
		int i = 0;
		int ret = ASD_DBUS_SUCCESS;
		dbus_error_init(&err);
		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&WTPID,
									DBUS_TYPE_INVALID))){
	
			asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
			if (dbus_error_is_set(&err)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		pthread_mutex_lock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
		pthread_mutex_lock(&asd_g_sta_mutex);
		if(WTPID >= WTP_NUM){
			ret = ASD_WTP_ID_LARGE_THAN_MAX;
			reply = dbus_message_new_method_return(msg);		
			dbus_message_iter_init_append (reply, &iter);
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 
		}
		else if(!AsdCheckWTPID(WTPID))
		{	
			ret = ASD_WTP_NOT_EXIST;	
			reply = dbus_message_new_method_return(msg);		
			dbus_message_iter_init_append (reply, &iter);
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 				
		}else{
				num = ASD_SEARCH_WTP_STA(WTPID, bss);	
				
				for(i=0; i<num; i++) {
					deny_num += bss[i]->th_deny_num;
				}
		
				reply = dbus_message_new_method_return(msg);
				
				dbus_message_iter_init_append (reply, &iter);
			
				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &ret); 
				
			//	UpdateStaInfoToWSM(NULL,NULL,STA_INFO);	//ht add,081027

				acc_tms=GetAccessTimes(WTPID);
				auth_tms=GetUserAuthTimes(WTPID);
				repauth_tms=GetWTPRspAuthTimes(WTPID);

				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &acc_tms);
				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &auth_tms);
				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &repauth_tms);
		
				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &num);
				
				dbus_message_iter_append_basic (&iter,
												 DBUS_TYPE_UINT32,
												 &deny_num);
	
				dbus_message_iter_open_container (&iter,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															DBUS_TYPE_UINT32_AS_STRING
															DBUS_TYPE_ARRAY_AS_STRING
																DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
																	DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_UINT16_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
				
																	DBUS_TYPE_UINT32_AS_STRING
																	DBUS_TYPE_UINT64_AS_STRING
															   		DBUS_TYPE_UINT64_AS_STRING
															   		DBUS_TYPE_UINT64_AS_STRING
																	DBUS_TYPE_UINT64_AS_STRING
																	DBUS_TYPE_UINT64_AS_STRING   //ht add 090220
																	DBUS_TYPE_UINT64_AS_STRING
																	DBUS_TYPE_UINT64_AS_STRING
																	DBUS_TYPE_UINT64_AS_STRING	 //ht add 090223
																DBUS_STRUCT_END_CHAR_AS_STRING
												   DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_array);
				for(i = 0; i < num ; i++){			
					DBusMessageIter iter_struct;
					DBusMessageIter iter_sub_array;
		
					dbus_message_iter_open_container (&iter_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_struct);
	
					dbus_message_iter_append_basic
								(&iter_struct,
								  DBUS_TYPE_UINT32,
								&(bss[i]->num_sta));
					
					dbus_message_iter_open_container (&iter_struct,
													   DBUS_TYPE_ARRAY,
													   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															   DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_UINT16_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
															DBUS_TYPE_BYTE_AS_STRING
											
															   DBUS_TYPE_UINT32_AS_STRING
															   DBUS_TYPE_UINT64_AS_STRING
															   DBUS_TYPE_UINT64_AS_STRING
															   DBUS_TYPE_UINT64_AS_STRING
																DBUS_TYPE_UINT64_AS_STRING
																DBUS_TYPE_UINT64_AS_STRING		//ht add 090220
																DBUS_TYPE_UINT64_AS_STRING
																DBUS_TYPE_UINT64_AS_STRING
																DBUS_TYPE_UINT64_AS_STRING 	 //ht add 090220
														DBUS_STRUCT_END_CHAR_AS_STRING,
													   &iter_sub_array);
					int j = 0;			
					struct sta_info *sta;
					sta = bss[i]->sta_list;
						
					for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
		

						Compute_Sta_Rate(sta);
						Get_Sta_SNR(sta);
									
						DBusMessageIter iter_sub_struct;
						dbus_message_iter_open_container (&iter_sub_array,
														   DBUS_TYPE_STRUCT,
														   NULL,
														   &iter_sub_struct);
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(sta->addr[0]));
						
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(sta->addr[1]));
			
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(sta->addr[2]));
			
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(sta->addr[3]));
			
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(sta->addr[4]));
			
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_BYTE,
								  &(sta->addr[5]));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
					 				DBUS_TYPE_BYTE,
									&(sta->mode));

						dbus_message_iter_append_basic
								(&iter_sub_struct,
								 DBUS_TYPE_BYTE,
								&(sta->channel));
						
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								 DBUS_TYPE_BYTE,
								&(sta->rssi));
						
						dbus_message_iter_append_basic
									(&iter_sub_struct,
					  				DBUS_TYPE_UINT16,
									&(sta->nRate));
						
						dbus_message_iter_append_basic
									(&iter_sub_struct,
									 DBUS_TYPE_BYTE,
									&(sta->isPowerSave));

						dbus_message_iter_append_basic
									(&iter_sub_struct,
					 				DBUS_TYPE_BYTE,
									&(sta->isQos));

						dbus_message_iter_append_basic  //xm add
								(&iter_sub_struct,
								  DBUS_TYPE_UINT32,
								  &(sta->snr));
						
						dbus_message_iter_append_basic  //xm add
								(&iter_sub_struct,
								  DBUS_TYPE_UINT64,
								  &(sta->rr));
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT64,
								  &(sta->tr));
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT64,
								  &(sta->tp));
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT64,
							  &(sta->rxpackets));
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT64,
							  &(sta->txpackets));		//ht add 090220
						dbus_message_iter_append_basic
								(&iter_sub_struct,
								  DBUS_TYPE_UINT64,
								  &(sta->retrybytes));
						dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT64,
							  &(sta->retrypackets));
						dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT64,
							  &(sta->errpackets));		//ht add 090223
						dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
						sta = sta->next;
					}
					
					dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
					dbus_message_iter_close_container (&iter_array, &iter_struct);
				}		
				dbus_message_iter_close_container (&iter, &iter_array);
			}
			pthread_mutex_unlock(&asd_g_sta_mutex);
			pthread_mutex_unlock(&asd_g_wtp_mutex); 		//mahz add 2011.4.20
			
			return reply;
	

}


DBusMessage *asd_dbus_wtp_stalist(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int WTPID;
	DBusError err;		
	int pae_state, backend_state;
	unsigned char SecurityID;
	unsigned int assoc_num = 0;		//ht 090213
	unsigned int reassoc_num = 0;
	unsigned int assoc_failure_num = 0;

	unsigned int normal_down_num = 0;  //nl091125
	unsigned int abnormal_down_num = 0;//nl091125
	struct asd_data *bss[L_BSS_NUM*L_RADIO_NUM];
	unsigned int num = 0;
	int i = 0;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&WTPID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	pthread_mutex_lock(&asd_g_sta_mutex);
	if(WTPID >= WTP_NUM){
		ret = ASD_WTP_ID_LARGE_THAN_MAX;
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 
	}
	if(!AsdCheckWTPID(WTPID))
	{	
		ret = ASD_WTP_NOT_EXIST;	
		reply = dbus_message_new_method_return(msg);		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret); 				
	}else{
		
			num = ASD_SEARCH_WTP_STA(WTPID, bss);	
			
			for(i=0; i<num; i++) {
				assoc_num += bss[i]->num_assoc;
				reassoc_num += bss[i]->num_reassoc;
				assoc_failure_num += bss[i]->num_assoc_failure;
				normal_down_num += bss[i]->normal_st_down_num;
				abnormal_down_num += bss[i]->abnormal_st_down_num;
			}

			reply = dbus_message_new_method_return(msg);
			
			dbus_message_iter_init_append (reply, &iter);
		
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &ret); 		
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &num);
			
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &assoc_num);

			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &reassoc_num);

			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &assoc_failure_num);
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &normal_down_num);
			dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &abnormal_down_num);

			dbus_message_iter_open_container (&iter,
											   DBUS_TYPE_ARRAY,
											   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_BYTE_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING		//ht add,090213
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_UINT32_AS_STRING
														DBUS_TYPE_ARRAY_AS_STRING
															DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_BYTE_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING
																DBUS_TYPE_UINT32_AS_STRING//
																DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
																DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
																DBUS_TYPE_UINT32_AS_STRING	//sta_online_time  qiuchen add it
															DBUS_STRUCT_END_CHAR_AS_STRING
											   DBUS_STRUCT_END_CHAR_AS_STRING,
											   &iter_array);
			for(i = 0; i < num ; i++){			
				DBusMessageIter iter_struct;
				DBusMessageIter iter_sub_array;
				//asd_printf(ASD_DBUS,MSG_DEBUG,"sta_num:%d\n",bss[i]->num_sta);
				if(ASD_WLAN[bss[i]->WlanID])
					SecurityID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							&(bss[i]->WlanID));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->Radio_G_ID));
		
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->BSSIndex));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							&(SecurityID));
				
				dbus_message_iter_append_basic			//ht add,090213
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_assoc));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_reassoc));
				
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_assoc_failure));

				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT32,
							&(bss[i]->num_sta));
				
				dbus_message_iter_open_container (&iter_struct,
												   DBUS_TYPE_ARRAY,
												   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_BYTE_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING
														   DBUS_TYPE_UINT32_AS_STRING/////
														   DBUS_TYPE_UINT32_AS_STRING  //sta_online_time  qiuchen add it
														   //DBUS_TYPE_STRING_AS_STRING ///yao gai
													DBUS_STRUCT_END_CHAR_AS_STRING,
												   &iter_sub_array);
				int j = 0;			
				struct sta_info *sta;
				sta = bss[i]->sta_list;

//=====================================
				time_t sta_time;
//=====================================


				
				for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
					
					if(sta->eapol_sm != NULL){
						pae_state = sta->eapol_sm->auth_pae_state;
						backend_state = sta->eapol_sm->be_auth_state;	
					}else{
						pae_state = 255;
						backend_state = 255;
					}	

					sta_time = sta->add_time_sysruntime;//qiuchen change it 2012.10.31
					
					DBusMessageIter iter_sub_struct;
					dbus_message_iter_open_container (&iter_sub_array,
													   DBUS_TYPE_STRUCT,
													   NULL,
													   &iter_sub_struct);
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[0]));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[1]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[2]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[3]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[4]));
		
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_BYTE,
							  &(sta->addr[5]));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(sta->flags));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(pae_state));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(backend_state));
					
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(sta_time));
					//qiuchen add it
					dbus_message_iter_append_basic
							(&iter_sub_struct,
							  DBUS_TYPE_UINT32,
							  &(sta->sta_online_time));
					//end
					
					//qiuchen add it for AXSSZFI-1373
					time_t sta_access_time,online_time,nowsys;
					get_sysruntime(&nowsys);
					online_time = nowsys - sta->add_time_sysruntime;
					sta_access_time = time(NULL) - online_time;
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(sta_access_time));
					
					dbus_message_iter_append_basic (&iter_sub_struct,
													 DBUS_TYPE_UINT32,
													 &(online_time));
					//end
					
					dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					sta = sta->next;
				}
				
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}		
			dbus_message_iter_close_container (&iter, &iter_array);
		}
		
		pthread_mutex_unlock(&asd_g_sta_mutex);
		return reply;


}

DBusMessage * asd_dbus_interface_update_wtp_count(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	ret = ASDReInit();
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	return reply;	
}

DBusMessage * asd_dbus_interface_set_trap_flag(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned char trapflag = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&trapflag,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	gasdtrapflag = trapflag;
	
	return reply;	

}



DBusMessage *asd_dbus_config_security(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	unsigned char SecurityID;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&SecurityID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[SecurityID] == NULL)
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
}



DBusMessage *asd_dbus_add_del_sta(DBusConnection *conn, DBusMessage *msg, void *user_data){

	return NULL;
}


DBusMessage *asd_dbus_set_sta_arp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	char *mac;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	char *ip = NULL;
	char *ifname = NULL;
	int is_add=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&is_add,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_STRING,&mac,
								DBUS_TYPE_STRING,&ifname,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	unsigned char ret_flag = 0;
	char * name = NULL;
	name = (char*)malloc(strlen(ifname)+5);
	if(name == NULL)
		return NULL;
	memset(name, 0, strlen(ifname)+5);

	if(check_ve_interface(ifname, name)){
		asd_printf(ASD_DBUS,MSG_DEBUG,"input ve interface doesn't exist!\n"); 
		ret = ASD_IFNAME_NOT_EXIST;
		ret_flag = 1;
	}
	
	if((is_add == 1)&&(ret_flag == 0)){
		printf("%s 1\n",__func__);
		ret = ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,ip, mac,name);
	}
	else if((is_add == 2)&&(ret_flag == 0)){
		printf("%s 2\n",__func__);

		ret = ipneigh_modify(RTM_DELNEIGH, 0,ip, mac,name);
	}
	else if((is_add == 3)&&(ret_flag == 0)){
		printf("%s 3\n",__func__);

		ret = ipneigh_modify(RTM_NEWNEIGH, NLM_F_REPLACE,ip, mac,name);
	}
	else if((is_add == 4)&&(ret_flag == 0)){
		printf("%s 4\n",__func__);

		ret = ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,ip, mac,name);
	}else{
		printf("%s 5\n",__func__);

		asd_printf(ASD_DBUS,MSG_DEBUG,"%s %d\n",__func__,is_add);	
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret); 
	free(name);
	name = NULL;
	return reply;	

}


DBusMessage *asd_dbus_show_security_list(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char num=0;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i=0;
	security_profile *Security[WLAN_NUM-1];

	while(i<WLAN_NUM){
		if(ASD_SECURITY[i] != NULL)
		{
			Security[num] = ASD_SECURITY[i];
			num++;
		}
		i++;
	}
	if(num == 0)
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &num);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(Security[i]->SecurityID));
			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(Security[i]->RadiusID));

		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_STRING,
					  &(Security[i]->name));
			
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_STRING,
					  &(Security[i]->host_ip));
		
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(Security[i]->securityType));
		
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(Security[i]->encryptionType));

		dbus_message_iter_close_container (&iter_array, &iter_struct);


	}
				
	dbus_message_iter_close_container (&iter, &iter_array);
				
	
	return reply;	



}


DBusMessage *asd_dbus_show_security(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned char SecurityID;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	char ch = 0;					//mahz add 2011.3.26
	int var = 0;					//mahz add 2011.3.26
	char *ip = "0.0.0.0";
	char *secret = " ";
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&SecurityID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret = ASD_CHECK_ID(ASD_SECURITY_CHECK,(unsigned int)SecurityID);

	if(ret != ASD_DBUS_SUCCESS){
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_BYTE,&ch,
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_BYTE,&ch,
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_UINT32,&var,	//	xm0701
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_BYTE,&ch,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_UINT32,&var,
					   			 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_STRING,&ip,
								 //////////////////////////
								DBUS_TYPE_UINT32,&var,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_UINT32,&var,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_UINT32,&var, //xm 08/09/03
								DBUS_TYPE_UINT32,&var,   //ht 090206
								DBUS_TYPE_UINT32,&var,	//mahz  add 2011.1.11
								 //////////////////////////
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_UINT32,&var,
								 DBUS_TYPE_UINT32,&var,
							   	 DBUS_TYPE_UINT32,&var,
					             DBUS_TYPE_UINT32,&var,	//mahz add 2011.1.11
								 DBUS_TYPE_STRING,&ip,	//mahz add 2011.1.11

									DBUS_TYPE_STRING,&ip,	//mahz add 2011.1.11
									DBUS_TYPE_BYTE,&ch,	//mahz add 2011.1.11
									DBUS_TYPE_STRING,&ip,
									DBUS_TYPE_STRING,&ip,
									DBUS_TYPE_STRING,&ip,
									DBUS_TYPE_UINT32,&var,
									DBUS_TYPE_BYTE,&ch,
									DBUS_TYPE_UINT32,&var,    //mahz add 2011.2.28
									DBUS_TYPE_UINT32,&var,	  //weichao
									DBUS_TYPE_BYTE,&ch,
									//qiuchen
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_DOUBLE,0,
									DBUS_TYPE_DOUBLE,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_STRING,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_BYTE,0,
									//end
								 DBUS_TYPE_INVALID);
		return reply;
	}
	else if(ASD_SECURITY[SecurityID] == NULL){
		ret = ASD_SECURITY_NOT_EXIST;
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 
								 DBUS_TYPE_BYTE,0,
								 DBUS_TYPE_UINT32,0,
								 DBUS_TYPE_UINT32,0,
								 
								 DBUS_TYPE_BYTE,0,
								 DBUS_TYPE_UINT32,0,
								 DBUS_TYPE_UINT32,0,	//	xm0701
								 
								 DBUS_TYPE_STRING,0,
								 DBUS_TYPE_BYTE,0,
								 DBUS_TYPE_STRING,0,
								 DBUS_TYPE_UINT32,0,
					   			 DBUS_TYPE_STRING,0,
								 DBUS_TYPE_STRING,0,
								 //////////////////////////
								DBUS_TYPE_UINT32,0,
								DBUS_TYPE_STRING,0,
								DBUS_TYPE_STRING,0,
								DBUS_TYPE_UINT32,0,
								DBUS_TYPE_STRING,0,
								DBUS_TYPE_STRING,0,
								DBUS_TYPE_UINT32,3600, //xm 08/09/03
								DBUS_TYPE_UINT32,0,   //ht 090206
								DBUS_TYPE_UINT32,60,   //ht 090727
								 //////////////////////////
								 DBUS_TYPE_UINT32,0,
								 DBUS_TYPE_STRING,0,
								 DBUS_TYPE_STRING,0,
								 DBUS_TYPE_UINT32,0,
								 DBUS_TYPE_UINT32,0,
								 DBUS_TYPE_STRING,0,
								 DBUS_TYPE_UINT32,0,
								 DBUS_TYPE_UINT32,0,
							   	 DBUS_TYPE_UINT32,0,
							     DBUS_TYPE_UINT32,0,	//mahz add 2011.1.11
								 DBUS_TYPE_STRING,0,	//mahz add 2011.1.11

									DBUS_TYPE_STRING,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_STRING,0,
									DBUS_TYPE_STRING,0,
									DBUS_TYPE_STRING,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_UINT32,0,    //mahz add 2011.2.28
									DBUS_TYPE_UINT32,0,    //weichao 
									DBUS_TYPE_BYTE,0,
									//qiuchen
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_DOUBLE,0,
									DBUS_TYPE_DOUBLE,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_UINT32,0,
									DBUS_TYPE_STRING,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_BYTE,0,
									DBUS_TYPE_BYTE,0,
									//end
								 DBUS_TYPE_INVALID);
		return reply;
	}else{
		if(ASD_SECURITY[SecurityID]->SecurityKey == NULL)
			ASD_SECURITY[SecurityID]->SecurityKey = secret;
		if(ASD_SECURITY[SecurityID]->acct.acct_ip == NULL){
			ASD_SECURITY[SecurityID]->acct.acct_ip = ip;
			ASD_SECURITY[SecurityID]->acct.acct_shared_secret = secret;
			asd_printf(ASD_DBUS,MSG_DEBUG,"auth ip %s secret %s\n",ASD_SECURITY[SecurityID]->acct.acct_ip,ASD_SECURITY[SecurityID]->acct.acct_shared_secret);
		}
		if(ASD_SECURITY[SecurityID]->keyInputType!=1&&ASD_SECURITY[SecurityID]->keyInputType!=2)
			ASD_SECURITY[SecurityID]->keyInputType=0;
		asd_printf(ASD_DBUS,MSG_DEBUG,"key input type %d \n",ASD_SECURITY[SecurityID]->keyInputType);//////
		////////////////////////////////////////////////////////
		if(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip== NULL){
			ASD_SECURITY[SecurityID]->acct.secondary_acct_ip= ip;
			ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret= secret;
			asd_printf(ASD_DBUS,MSG_DEBUG,"secondary auth ip %s secret %s\n",ASD_SECURITY[SecurityID]->acct.secondary_acct_ip,ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret);
		}
		if(ASD_SECURITY[SecurityID]->auth.auth_ip == NULL){
			ASD_SECURITY[SecurityID]->auth.auth_ip = ip;
			ASD_SECURITY[SecurityID]->auth.auth_shared_secret = secret;		
			asd_printf(ASD_DBUS,MSG_DEBUG,"auth ip %s secret %s\n",ASD_SECURITY[SecurityID]->auth.auth_ip,ASD_SECURITY[SecurityID]->auth.auth_shared_secret);
		}
		////////////////////////////////////////////////////////
		if(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip== NULL){
			ASD_SECURITY[SecurityID]->auth.secondary_auth_ip= ip;		//xm 08/09/03
			ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret= secret;		
			asd_printf(ASD_DBUS,MSG_DEBUG,"secondary auth ip %s  backup secret %s\n",ASD_SECURITY[SecurityID]->auth.secondary_auth_ip,ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret);
		}
		////////////////////////////////////////////////////////
		if(ASD_SECURITY[SecurityID]->wapi_as.as_ip== NULL){
		
			ASD_SECURITY[SecurityID]->wapi_as.as_ip= ip;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.certification_path==NULL){
			ASD_SECURITY[SecurityID]->wapi_as.certification_path=secret;
		}
		
		if(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path==NULL){
			ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path=secret;
		}
		if(ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path==NULL){
			ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path=secret;
		}
		//mahz add 2010.12.10
		if(ASD_SECURITY[SecurityID]->user_passwd == NULL){
			ASD_SECURITY[SecurityID]->user_passwd = secret;		
			asd_printf(ASD_DBUS,MSG_DEBUG,"secret %s\n",ASD_SECURITY[SecurityID]->user_passwd);
		}
		//
		//qiuchen add 2012.12.11
		char *ac_radius_name = NULL;
		if(ASD_SECURITY[SecurityID]->ac_radius_name != NULL){
			ac_radius_name = os_zalloc(strlen(ASD_SECURITY[SecurityID]->ac_radius_name)+1);
			if(ac_radius_name == NULL)
				return NULL;
			memset(ac_radius_name,0,strlen(ASD_SECURITY[SecurityID]->ac_radius_name)+1);
			memcpy(ac_radius_name,ASD_SECURITY[SecurityID]->ac_radius_name,strlen(ASD_SECURITY[SecurityID]->ac_radius_name));
		}
		else{
			ac_radius_name = os_zalloc(7);
			memcpy(ac_radius_name,"000000",6);
		}
		//end
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_ucast_rekey_method),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_ucast_rekey_para_t),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_ucast_rekey_para_p),

								DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_mcast_rekey_method),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_mcast_rekey_para_t),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_mcast_rekey_para_p),
								
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->name),
								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->SecurityID),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->host_ip),
			 					 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->auth.auth_port),			 					 
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.auth_ip),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.auth_shared_secret),
								 /////////////////////////////////////////////////////////
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->auth.secondary_auth_port),			 					 
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip),	//xm 08/09/03
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->acct.secondary_acct_port),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->eap_reauth_priod),	
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->acct_interim_interval),    
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->quiet_Period),	
								 /////////////////////////////////////////////////////////
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->acct.acct_port),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.acct_ip),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.acct_shared_secret),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->securityType),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->encryptionType),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->SecurityKey),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->keyInputType),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->extensible_auth),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wired_radius),

								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_radius_auth),	//mahz add 2010.11.25
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->user_passwd),			//mahz add 2010.12.9

									DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->wapi_as.as_ip),
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_as.multi_cert),
									DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->wapi_as.certification_path),
									DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path),
									DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path),
								 	DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_as.certification_type),
								 	
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->index),
									DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->hybrid_auth),    //mahz add 2011.2.28
									DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->mac_auth),	  //weichao 
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->eap_sm_run_activated),//Qc
									//qiuchen add it for master_bak radius server configuration 2012.12.11
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->radius_server_binding_type),
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->radius_heart_test_type),
									DBUS_TYPE_DOUBLE,&(ASD_SECURITY[SecurityID]->radius_res_fail_percent),
									DBUS_TYPE_DOUBLE,&(ASD_SECURITY[SecurityID]->radius_res_suc_percent),
									DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->radius_access_test_interval),
									DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->radius_server_change_test_timer),
									DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->radius_server_reuse_test_timer),
									DBUS_TYPE_STRING,&(ac_radius_name),
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->heart_test_on),
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->acct_server_current_use),
									DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->auth_server_current_use),
									//end
								 DBUS_TYPE_INVALID);
		if(ac_radius_name){
			free(ac_radius_name);
			ac_radius_name = NULL;
		}
	}
	
	if(strcmp(ASD_SECURITY[SecurityID]->SecurityKey, " ") == 0){
		ASD_SECURITY[SecurityID]->SecurityKey = NULL;
	}
	if(strcmp(ASD_SECURITY[SecurityID]->auth.auth_ip, "0.0.0.0") == 0){
		ASD_SECURITY[SecurityID]->auth.auth_ip = NULL;
		ASD_SECURITY[SecurityID]->auth.auth_shared_secret = NULL;
	}
	/////////////////////////////////////////////////////////
	if(strcmp(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip, "0.0.0.0") == 0){
		ASD_SECURITY[SecurityID]->auth.secondary_auth_ip= NULL;
		ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret= NULL;
	}																		//xm 08/09/03
	/////////////////////////////////////////////////////////
	if(strcmp(ASD_SECURITY[SecurityID]->acct.acct_ip, "0.0.0.0") == 0){
		ASD_SECURITY[SecurityID]->acct.acct_ip = NULL;
		ASD_SECURITY[SecurityID]->acct.acct_shared_secret = NULL;

	}
	////////////////////////////////////////////////////////
	if(strcmp(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip, "0.0.0.0") == 0){
		ASD_SECURITY[SecurityID]->acct.secondary_acct_ip= NULL;
		ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret= NULL;
																			//xm 08/09/03
	}
	////////////////////////////////////////////////////////
	if(strcmp(ASD_SECURITY[SecurityID]->wapi_as.as_ip, "0.0.0.0") == 0){
	
		ASD_SECURITY[SecurityID]->wapi_as.as_ip= NULL;//xm 08/09/03
	}
	if(strcmp(ASD_SECURITY[SecurityID]->wapi_as.certification_path, " ") == 0){
		ASD_SECURITY[SecurityID]->wapi_as.certification_path= NULL;
	}
	if(strcmp(ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path, " ") == 0){
		ASD_SECURITY[SecurityID]->wapi_as.ae_cert_path= NULL;
	}
	if(strcmp(ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path, " ") == 0){
		ASD_SECURITY[SecurityID]->wapi_as.ca_cert_path= NULL;
	}
	//mahz add 2010.12.10
	if(strcmp(ASD_SECURITY[SecurityID]->user_passwd, " ") == 0){
		ASD_SECURITY[SecurityID]->user_passwd = NULL;
	}
	//
	return reply;	


}

/*ht add 091215*/
DBusMessage *asd_dbus_show_radius(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;	
	DBusError err;
	unsigned char RadiusID;
	unsigned char SecurityID;
	char *ip = "0.0.0.0";
	char *secret = " ";
	int port = 0;
	int ret = ASD_DBUS_SUCCESS;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&RadiusID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	SecurityID = RadiusID;
	if((ASD_SECURITY[SecurityID] == NULL) 
		|| !((ASD_SECURITY[SecurityID]->securityType == IEEE8021X)||(ASD_SECURITY[SecurityID]->securityType == WPA_E)
		||(ASD_SECURITY[SecurityID]->securityType == WPA2_E)||(ASD_SECURITY[SecurityID]->securityType == MD5))){
		ret = ASD_SECURITY_NOT_EXIST;
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 
								 DBUS_TYPE_UINT32,&port,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_STRING,&secret,
								 DBUS_TYPE_UINT32,&port,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_STRING,&secret,

								 DBUS_TYPE_UINT32,&port,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_STRING,&secret,
								 DBUS_TYPE_UINT32,&port,
								 DBUS_TYPE_STRING,&ip,
								 DBUS_TYPE_STRING,&secret,

								 DBUS_TYPE_INVALID);
		return reply;
	}else{
		if(ASD_SECURITY[SecurityID]->auth.auth_ip == NULL){
			ASD_SECURITY[SecurityID]->auth.auth_ip = ip;
			ASD_SECURITY[SecurityID]->auth.auth_shared_secret = secret; 	
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"auth ip %s secret %s\n",ASD_SECURITY[SecurityID]->auth.auth_ip,ASD_SECURITY[SecurityID]->auth.auth_shared_secret);
		if(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip== NULL){
			ASD_SECURITY[SecurityID]->auth.secondary_auth_ip= ip;		
			ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret= secret;		
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"secondary auth ip %s	backup secret %s\n",ASD_SECURITY[SecurityID]->auth.secondary_auth_ip,ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret);
		if(ASD_SECURITY[SecurityID]->acct.acct_ip == NULL){
			ASD_SECURITY[SecurityID]->acct.acct_ip = ip;
			ASD_SECURITY[SecurityID]->acct.acct_shared_secret = secret;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"acct ip %s secret %s\n",ASD_SECURITY[SecurityID]->acct.acct_ip,ASD_SECURITY[SecurityID]->acct.acct_shared_secret);
		if(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip== NULL){
			ASD_SECURITY[SecurityID]->acct.secondary_acct_ip= ip;
			ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret= secret;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"secondary acct ip %s secret %s\n",ASD_SECURITY[SecurityID]->acct.secondary_acct_ip,ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret);

		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->auth.auth_port),								 
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.auth_ip),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.auth_shared_secret),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->auth.secondary_auth_port),								 
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip),	
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret),

								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->acct.acct_port),								
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.acct_ip),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.acct_shared_secret),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->acct.secondary_acct_port),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip),
								 DBUS_TYPE_STRING,&(ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret),
								 DBUS_TYPE_INVALID);

	
		if(strcmp(ASD_SECURITY[SecurityID]->auth.auth_ip, "0.0.0.0") == 0){
			ASD_SECURITY[SecurityID]->auth.auth_ip = NULL;
			ASD_SECURITY[SecurityID]->auth.auth_shared_secret = NULL;
		}
		if(strcmp(ASD_SECURITY[SecurityID]->auth.secondary_auth_ip, "0.0.0.0") == 0){
			ASD_SECURITY[SecurityID]->auth.secondary_auth_ip= NULL;
			ASD_SECURITY[SecurityID]->auth.secondary_auth_shared_secret= NULL;
		}																		
		if(strcmp(ASD_SECURITY[SecurityID]->acct.acct_ip, "0.0.0.0") == 0){
			ASD_SECURITY[SecurityID]->acct.acct_ip = NULL;
			ASD_SECURITY[SecurityID]->acct.acct_shared_secret = NULL;
		}
		if(strcmp(ASD_SECURITY[SecurityID]->acct.secondary_acct_ip, "0.0.0.0") == 0){
			ASD_SECURITY[SecurityID]->acct.secondary_acct_ip= NULL;
			ASD_SECURITY[SecurityID]->acct.secondary_acct_shared_secret= NULL;
		}

		return reply;	
	}
}

//mahz add for mib request , 2011.1.12
DBusMessage *asd_dbus_show_radius_all(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char num=0;
	DBusError err;

	char *ip = "0.0.0.0";
	char *secret = " ";
	int ret = ASD_DBUS_SUCCESS;

	dbus_error_init(&err);

	int i=0;
	security_profile *Security[WLAN_NUM-1];

	while(i<WLAN_NUM){
		if((ASD_SECURITY[i] != NULL)&&((ASD_SECURITY[i]->securityType == IEEE8021X)||(ASD_SECURITY[i]->securityType == WPA_E)
		||(ASD_SECURITY[i]->securityType == WPA2_E)||(ASD_SECURITY[i]->securityType == MD5)))
		{
			Security[num] = ASD_SECURITY[i];
			num++;
		}
		i++;
	}
	if(num == 0)
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &num);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for(i = 0; i < num; i++){
		
			DBusMessageIter iter_struct;			
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
		
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						  &(Security[i]->SecurityID));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						  &(Security[i]->RadiusID));
				
			if(Security[i]->auth.auth_ip == NULL){
				Security[i]->auth.auth_ip = ip;
				Security[i]->auth.auth_shared_secret = secret; 	
			}
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"auth ip %s secret %s\n",Security[i]->auth.auth_ip,Security[i]->auth.auth_shared_secret);
			if(Security[i]->auth.secondary_auth_ip== NULL){
				Security[i]->auth.secondary_auth_ip= ip;		
				Security[i]->auth.secondary_auth_shared_secret= secret;		
			}
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"secondary auth ip %s	backup secret %s\n",Security[i]->auth.secondary_auth_ip,Security[i]->auth.secondary_auth_shared_secret);
			if(Security[i]->acct.acct_ip == NULL){
				Security[i]->acct.acct_ip = ip;
				Security[i]->acct.acct_shared_secret = secret;
			}
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"acct ip %s secret %s\n",Security[i]->acct.acct_ip,Security[i]->acct.acct_shared_secret);
			if(Security[i]->acct.secondary_acct_ip== NULL){
				Security[i]->acct.secondary_acct_ip= ip;
				Security[i]->acct.secondary_acct_shared_secret= secret;
			}
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"secondary acct ip %s secret %s\n",Security[i]->acct.secondary_acct_ip,Security[i]->acct.secondary_acct_shared_secret);

			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_UINT32, &(Security[i]->auth.auth_port));
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->auth.auth_ip));			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->auth.auth_shared_secret));
			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_UINT32, &(Security[i]->auth.secondary_auth_port));
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->auth.secondary_auth_ip));			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->auth.secondary_auth_shared_secret));
			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_UINT32, &(Security[i]->acct.acct_port));
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->acct.acct_ip));			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->acct.acct_shared_secret));
			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_UINT32, &(Security[i]->acct.secondary_acct_port));
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->acct.secondary_acct_ip));			
			dbus_message_iter_append_basic
						(&iter_struct, DBUS_TYPE_STRING, &(Security[i]->acct.secondary_acct_shared_secret));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
					
			if(strcmp(Security[i]->auth.auth_ip, "0.0.0.0") == 0){
				Security[i]->auth.auth_ip = NULL;
				Security[i]->auth.auth_shared_secret = NULL;
			}
			if(strcmp(Security[i]->auth.secondary_auth_ip, "0.0.0.0") == 0){
				Security[i]->auth.secondary_auth_ip= NULL;
				Security[i]->auth.secondary_auth_shared_secret= NULL;
			}																		
			if(strcmp(Security[i]->acct.acct_ip, "0.0.0.0") == 0){
				Security[i]->acct.acct_ip = NULL;
				Security[i]->acct.acct_shared_secret = NULL;
			}
			if(strcmp(Security[i]->acct.secondary_acct_ip, "0.0.0.0") == 0){
				Security[i]->acct.secondary_acct_ip= NULL;
				Security[i]->acct.secondary_acct_shared_secret= NULL;
			}
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	return reply;	
}
	
DBusMessage *asd_dbus_show_security_wapi_info(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned char SecurityID;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&SecurityID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	
		ret = ASD_CHECK_ID(ASD_SECURITY_CHECK,(unsigned int)SecurityID);
	
	if(ret != ASD_DBUS_SUCCESS){
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
	}
	else if(ASD_SECURITY[SecurityID] == NULL){
		ret = ASD_SECURITY_NOT_EXIST;
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
	}
	else if((ASD_SECURITY[SecurityID]->securityType != WAPI_AUTH)&&(ASD_SECURITY[SecurityID]->securityType != WAPI_PSK)){
		ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
	}
	else if(ASD_SECURITY[SecurityID]->wapi_config==NULL){
		ret = ASD_SECURITY_PROFILE_NOT_INTEGRITY;
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
	}
	else{		
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,								
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_config->CertificateUpdateCount),	
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_config->MulticastUpdateCount),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_config->UnicastUpdateCount),
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_config->BKLifetime),												 
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_config->BKReauthThreshold),												 
								 DBUS_TYPE_UINT32,&(ASD_SECURITY[SecurityID]->wapi_config->SATimeout),	
								 
								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->WapiPreauth),
								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->MulticaseRekeyStrict),
								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->UnicastCipherEnabled),
								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->AuthenticationSuiteEnable),
 								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->MulticastCipher[0]),
 								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->MulticastCipher[1]),
 								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->MulticastCipher[2]),
 								 DBUS_TYPE_BYTE,&(ASD_SECURITY[SecurityID]->wapi_config->MulticastCipher[3]),
								 DBUS_TYPE_INVALID);			
	}
	

	return reply;	


}

/*nl add for wapi 09/12/14*/
DBusMessage *asd_dbus_show_wlan_security_wapi_conf(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;

	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	char *ip = "0.0.0.0";
	char *secret = " ";

	unsigned char wlan_id;
	unsigned char security_id;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
			
		}
		return NULL;
	}
	
	ret = ASD_CHECK_ID(ASD_WLAN_CHECK,(unsigned int)wlan_id);
	
	if(ret != ASD_DBUS_SUCCESS){
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
		return reply;
	}
	
	else if(ASD_WLAN[wlan_id]==NULL){
		ret = ASD_WLAN_NOT_EXIST;
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
		return reply;
	}
	
	security_id=ASD_WLAN[wlan_id]->SecurityID;
	
	if((security_id == 0) || (ASD_SECURITY[security_id]==NULL))	{
		ret = ASD_SECURITY_NOT_EXIST;
		
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
		
		asd_printf(ASD_DEFAULT,MSG_INFO,"the security is not exist\n");
		return reply;
	}
	
	if((ASD_SECURITY[security_id]->securityType!= WAPI_PSK)&&(ASD_SECURITY[security_id]->securityType!= WAPI_AUTH)){

		ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		
		reply = dbus_message_new_method_return(msg);
		dbus_message_append_args(reply,
								 DBUS_TYPE_UINT32,&ret,
								 DBUS_TYPE_INVALID);		
		
		asd_printf(ASD_DEFAULT,MSG_INFO,"the security is %d\n",ASD_WLAN[wlan_id]->SecurityType);
		return reply;
	}
	 
	else{		
		security_id=ASD_WLAN[wlan_id]->SecurityID;

		reply = dbus_message_new_method_return(msg);
		
		
			if(ASD_SECURITY[security_id]->wapi_as.as_ip== NULL){		
			ASD_SECURITY[security_id]->wapi_as.as_ip= ip;
			}
			if(ASD_SECURITY[security_id]->wapi_as.certification_path== NULL){	
			ASD_SECURITY[security_id]->wapi_as.certification_path= secret;
			}
			if(ASD_SECURITY[security_id]->wapi_as.ae_cert_path== NULL){	
			ASD_SECURITY[security_id]->wapi_as.ae_cert_path= secret;
			}
			if(ASD_SECURITY[security_id]->SecurityKey == NULL){
			ASD_SECURITY[security_id]->SecurityKey = secret;
			}
			dbus_message_append_args(reply,
							 	DBUS_TYPE_UINT32,&ret,								
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_config->CertificateUpdateCount),	
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_config->MulticastUpdateCount),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_config->UnicastUpdateCount),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_config->BKLifetime),												 
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_config->BKReauthThreshold),												 
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_config->SATimeout),	
								 
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->WapiPreauth),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticaseRekeyStrict),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->UnicastCipherEnabled),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->AuthenticationSuiteEnable),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[0]),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[1]),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[2]),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[3]),

								DBUS_TYPE_STRING,&(ASD_SECURITY[security_id]->wapi_as.as_ip),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_ucast_rekey_method),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_ucast_rekey_para_t),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_ucast_rekey_para_p),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_mcast_rekey_method),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_mcast_rekey_para_t),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_mcast_rekey_para_p),
								
								DBUS_TYPE_STRING,&(ASD_SECURITY[security_id]->wapi_as.certification_path),
								DBUS_TYPE_STRING,&(ASD_SECURITY[security_id]->wapi_as.ae_cert_path),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->wapi_as.certification_type),			
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->securityType),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->encryptionType),
								DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->SecurityID),
								DBUS_TYPE_STRING,&(ASD_SECURITY[security_id]->SecurityKey),
								DBUS_TYPE_UINT32,&(ASD_SECURITY[security_id]->keyInputType),							
							 	DBUS_TYPE_INVALID);			
	}
		if(strcmp(ASD_SECURITY[security_id]->SecurityKey, " ") == 0){
		ASD_SECURITY[security_id]->SecurityKey = NULL;
		}
		if(strcmp(ASD_SECURITY[security_id]->wapi_as.as_ip, "0.0.0.0") == 0){
		ASD_SECURITY[security_id]->wapi_as.as_ip= NULL;
		}
		if(strcmp(ASD_SECURITY[security_id]->wapi_as.certification_path, " ") == 0){
		ASD_SECURITY[security_id]->wapi_as.certification_path= NULL;
		}
		if(strcmp(ASD_SECURITY[security_id]->wapi_as.ae_cert_path, " ") == 0){
		ASD_SECURITY[security_id]->wapi_as.ae_cert_path= NULL;		
		}
	
	return reply;	


}


//mahz add for mib request , 2011.1.27 , dot11AKMConfigTable
DBusMessage *asd_dbus_show_security_wapi_conf_of_all_wlan(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_wlan_array;
	DBusMessageIter	 iter_wlan_struct;

	int ret = ASD_DBUS_SUCCESS;	
	int i=0;
	unsigned int wlan_num = 0;
	unsigned char security_id;

	DBusError err;
	dbus_error_init(&err);
	
	WID_WLAN  **WLAN = malloc(WLAN_NUM*(sizeof(WID_WLAN *)));
	if( WLAN == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	memset(WLAN,0,WLAN_NUM*(sizeof(WID_WLAN *)));
	//if(!WLAN)
	//	ret = -1;
	
	pthread_mutex_lock(&asd_g_wlan_mutex);
	for(i=0;i<WLAN_NUM;i++){
		if(ASD_WLAN[i]){
			security_id=ASD_WLAN[i]->SecurityID;
			if((security_id!=0)&&(ASD_SECURITY[security_id]!=NULL)&&((ASD_SECURITY[security_id]->securityType== WAPI_PSK)||(ASD_SECURITY[security_id]->securityType == WAPI_AUTH))) {
				WLAN[wlan_num++]=ASD_WLAN[i];
			}
		}
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"wlan_num: %d\n",wlan_num);

	if(wlan_num == 0)
		ret = ASD_WLAN_NOT_EXIST;	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wlan_num);
	
	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING		//wlan_id
									DBUS_TYPE_BYTE_AS_STRING		//AuthenticationSuiteEnable
									
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING
									DBUS_TYPE_BYTE_AS_STRING												 
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_wlan_array);

	//因为AKM 套件的值是固定的，在这里使用MulticastCipher 代替，作为参数传递
	pthread_mutex_lock(&asd_g_sta_mutex);	
	for(i=0;i<wlan_num;i++){	
		security_id=WLAN[i]->SecurityID;
		dbus_message_iter_open_container(&iter_wlan_array,DBUS_TYPE_STRUCT,NULL,&iter_wlan_struct);
		dbus_message_iter_append_basic(&iter_wlan_struct,DBUS_TYPE_BYTE,&(WLAN[i]->WlanID));
		dbus_message_iter_append_basic(&iter_wlan_struct,DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->AuthenticationSuiteEnable));

		dbus_message_iter_append_basic(&iter_wlan_struct,DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[0]));
		dbus_message_iter_append_basic(&iter_wlan_struct,DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[1]));
		dbus_message_iter_append_basic(&iter_wlan_struct,DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[2]));
		dbus_message_iter_append_basic(&iter_wlan_struct,DBUS_TYPE_BYTE,&(ASD_SECURITY[security_id]->wapi_config->MulticastCipher[3]));

		dbus_message_iter_close_container (&iter_wlan_array, &iter_wlan_struct);
	}
	pthread_mutex_unlock(&asd_g_sta_mutex); 
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	dbus_message_iter_close_container (&iter, &iter_wlan_array);

	if(WLAN != NULL){
		free(WLAN);
		WLAN = NULL;
	}
	return reply;	
}


DBusMessage *asd_dbus_add_del_security(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char isAdd;
	char* SecurityName;
	unsigned char SecurityID;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	int i;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&isAdd,								
								DBUS_TYPE_BYTE,&SecurityID,
								DBUS_TYPE_STRING,&SecurityName,
							//	DBUS_TYPE_STRING,&SecurityIp,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(isAdd){
		if(ASD_SECURITY[SecurityID] == NULL){
			ret = ASD_ADD_SECURITY_PROFILE(SecurityName,SecurityID);
		}else{
			ret = ASD_SECURITY_BE_USED;
		}
	}else{
		if(ASD_SECURITY[SecurityID] == NULL){
			ret = ASD_SECURITY_NOT_EXIST;
		}else{
			pthread_mutex_lock(&asd_g_wlan_mutex);
			for(i = 0; i < WLAN_NUM; i++){
				if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == SecurityID)){
					ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
					break;
				}else
					continue;
			}
			if(ret == 0){
				for(i = 0; i < WLAN_NUM; i++){
					if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 1)&&(ASD_WLAN[i]->SecurityID == SecurityID)){
						ASD_WLAN[i]->SecurityID = 0;
						continue;
					}else
						continue;
				}
				if(ASD_SECURITY[SecurityID]->heart_test_on == 1)
					ret = ASD_SECURITY_HEART_TEST_ON;
				else{
					if(ASD_WLAN_INF_OP(0, SecurityID, WID_DEL))
					ret = ASD_DELETE_SECURITY_PROFILE(SecurityID);
				}
			}
			pthread_mutex_unlock(&asd_g_wlan_mutex);
		}

	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	if(gasdtrapflag == 1)
		signal_de_key_conflict();
	
	return reply;


}


//xumin 
DBusMessage *asd_dbus_set_wapi_auth(DBusConnection *conn, DBusMessage *msg, void *user_data){
   //do some thing.
   	DBusMessage * reply;
	DBusMessageIter	 iter;
	char* as_ip;
	int i=0, isApply=0;
	unsigned char security_id;
	unsigned int type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_STRING,&as_ip,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->securityType == WAPI_AUTH){
				ret = ASD_SET_WAPI_AUTH(security_id,type,as_ip);
			}else
				ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage *asd_dbus_set_wapi_multi_cert(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	int i=0, isApply=0;
	unsigned char security_id;
	unsigned char type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_WAPI,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_WAPI,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->securityType == WAPI_AUTH){
				ASD_SECURITY[security_id]->wapi_as.multi_cert = type;
			}else
				ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


DBusMessage *asd_dbus_set_wapi_path(DBusConnection *conn, DBusMessage *msg, void *user_data){
   //do some thing.
   	DBusMessage * reply;
	DBusMessageIter	 iter;
	char* cert_path;
	int i=0, isApply=0;
	unsigned char security_id;
	unsigned int type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_STRING,&cert_path,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->securityType == WAPI_AUTH){
				ret = ASD_SET_WAPI_CERT_PATH(security_id,type,cert_path);
			}else
				ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
//weichao add 20110801
DBusMessage *asd_dbus_del_wapi_certification(DBusConnection *conn, DBusMessage *msg, void *user_data){
   	DBusMessage * reply;
	DBusMessageIter	 iter;
	int i=0, isApply=0;
	unsigned char security_id;
	unsigned int type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->securityType == WAPI_AUTH){
				ret = ASD_DEL_WAPI_CERT(security_id,type);
			}
			else
				ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

//xumin

DBusMessage *asd_dbus_set_wapi_p12_cert_path(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
   	DBusMessage *reply;
	DBusMessageIter	 iter;
	char *cert_path;
	char *pass_word;
	int i=0, isApply=0;
	unsigned char security_id;
	unsigned int type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	char newpath[128];
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_STRING,&cert_path,
								DBUS_TYPE_STRING,&pass_word,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");			
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s security_id:%d\n",__func__,security_id);
	asd_printf(ASD_DBUS,MSG_DEBUG,"cert_type: %d\n",type);
	asd_printf(ASD_DBUS,MSG_DEBUG,"cert_path: %s\n",cert_path);
	asd_printf(ASD_DBUS,MSG_DEBUG,"pass_word: %s\n",pass_word);
	memset(newpath, 0, 128);
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->securityType == WAPI_AUTH){
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SET_WAPI_P12_CERT_PATH\n");
#ifndef _DISTRIBUTION_	
				if(type == 1){
					sprintf(newpath,"/mnt/wtp/security%d_%d_as.cer",vrrid,security_id);
				}else if(type == 2){
					sprintf(newpath,"/mnt/wtp/security%d_%d_ae.cer",vrrid,security_id);
				}else if(type == 3){
					sprintf(newpath,"/mnt/wtp/security%d_%d_ca.cer",vrrid,security_id);
				}
#else				
				if(type == 1){
					sprintf(newpath,"/mnt/wtp/security%d_%d_%d_as.cer",local,vrrid,security_id);
				}else if(type == 2){
					sprintf(newpath,"/mnt/wtp/security%d_%d_%d_ae.cer",local,vrrid,security_id);
				}else if(type == 3){
					sprintf(newpath,"/mnt/wtp/security%d_%d_%d_ca.cer",local,vrrid,security_id);
				}
#endif				
				ret = pkcs12_decryption(cert_path, newpath, pass_word);
				if(ret == 0){
					char buf[128];
					char buf2[128];
					char buf3[128];
					memset(buf,0,128);
					memset(buf2,0,128);
					memset(buf3,0,128);
					sprintf(buf,"cp %s /blk/wtp",newpath);
					sprintf(buf2,"sudo mount /blk");
					sprintf(buf3,"sudo umount /blk");
					system(buf2);
					system(buf);
					system(buf3);
					ret = ASD_SET_WAPI_CERT_PATH(security_id,type,newpath);

				}
			}else
				ret = ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

//mahz add 2010.12.9
DBusMessage *asd_dbus_wapi_radius_auth_set_user_passwd(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage *reply;
	DBusMessageIter	 iter;
	int i=0, isApply=0;
	unsigned char security_id;
	char* user_passwd;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								
								DBUS_TYPE_STRING,&user_passwd,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if((ASD_SECURITY[security_id]->securityType == WAPI_AUTH) && (ASD_SECURITY[security_id]->wapi_radius_auth == 1)){
				ret = ASD_WAPI_RADIUS_AUTH_SET_USER_PASSWD(security_id,user_passwd);
				asd_printf(ASD_DBUS,MSG_DEBUG,"passwd is %s\n",user_passwd);
			}else
				ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage *asd_dbus_set_acct(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	char* acct_ip;
	int i=0, isApply=0;
	unsigned char security_id;
	unsigned int acct_port;
	char* acct_shared_secret;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&acct_port,
								DBUS_TYPE_STRING,&acct_ip,
								DBUS_TYPE_STRING,&acct_shared_secret,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				//mahz add wapi_auth 2010.11.25
				if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||((ASD_SECURITY[security_id]->securityType == WAPI_AUTH) && (ASD_SECURITY[security_id]->wapi_radius_auth == 1))||(ASD_SECURITY[security_id]->extensible_auth == 1)){
					ret = ASD_SET_ACCT(security_id,acct_port,acct_ip,acct_shared_secret);
					//qiuchen add it for master_bak radius server
					if((ASD_SECURITY[security_id]->auth.auth_ip != NULL) && (!strncmp(ASD_SECURITY[security_id]->auth.auth_ip,ASD_SECURITY[security_id]->acct.acct_ip,strlen(ASD_SECURITY[security_id]->auth.auth_ip)))){
						ASD_SECURITY[security_id]->radius_server_binding_type = RADIUS_SERVER_BINDED;
						if(ASD_SECURITY[security_id]->radius_heart_test_type== RADIUS_BOTH_TEST){
							ASD_SECURITY[security_id]->radius_heart_test_type = RADIUS_AUTH_TEST;
							ret = ASD_SECURITY_RADIUS_HEARTTEST_DEFAULT;
						}
					}
					ASD_SECURITY[security_id]->auth_server_current_use = RADIUS_DISABLE;
					ASD_SECURITY[security_id]->acct_server_current_use = RADIUS_DISABLE;
					//end
				}else
					ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


DBusMessage *asd_dbus_set_auth(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage * reply;
	DBusMessageIter	 iter;
	char* auth_ip;
	int i, isApply = 0;
	unsigned char security_id;
	unsigned int auth_port;
	char* auth_shared_secret;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&auth_port,
								DBUS_TYPE_STRING,&auth_ip,
								DBUS_TYPE_STRING,&auth_shared_secret,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				//mahz add wapi_auth 2010.11.25
				if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||((ASD_SECURITY[security_id]->securityType == WAPI_AUTH) && (ASD_SECURITY[security_id]->wapi_radius_auth == 1))||(ASD_SECURITY[security_id]->extensible_auth == 1)){
					ret = ASD_SET_AUTH(security_id,auth_port,auth_ip,auth_shared_secret);
					//qiuchen add it for master_bak radius server
					if((ASD_SECURITY[security_id]->acct.acct_ip != NULL) && (!strncmp(ASD_SECURITY[security_id]->auth.auth_ip,ASD_SECURITY[security_id]->acct.acct_ip,strlen(ASD_SECURITY[security_id]->auth.auth_ip)))){
						ASD_SECURITY[security_id]->radius_server_binding_type = RADIUS_SERVER_BINDED;
						if(ASD_SECURITY[security_id]->radius_heart_test_type == RADIUS_BOTH_TEST){
							ASD_SECURITY[security_id]->radius_heart_test_type = RADIUS_AUTH_TEST;
							ret = ASD_SECURITY_RADIUS_HEARTTEST_DEFAULT;
						}
					}
					ASD_SECURITY[security_id]->auth_server_current_use = RADIUS_DISABLE;
					ASD_SECURITY[security_id]->acct_server_current_use = RADIUS_DISABLE;
					//end
				}else
					ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

//weichao add 2011.10.31
DBusMessage *asd_dbus_set_mac_auth(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	unsigned int state;	
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;
			if(security_type == OPEN)
			{
				if((ASD_SECURITY[security_id]->hybrid_auth == 1)&&(ASD_SECURITY[security_id]->extensible_auth ==0)){
				
					ASD_SECURITY[security_id]->mac_auth = state;

				}
				else{
					ret = ASD_MAC_AUTH_NOT_SUPPORT;
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"the hybrid_auth is not 1 or the extensible_auth is not 0");
				}
			}
			else{
				
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"the security type is not open");
				ret = ASD_MAC_AUTH_NOT_SUPPORT;
			}

		}
		else
			ret = ASD_SECURITY_NOT_EXIST;
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
//weichao add 2011 .09.22
DBusMessage *asd_dbus_eap_alive_period(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int period;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&period,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
			security_type = ASD_SECURITY[security_id]->securityType;
			//if((security_type!= OPEN)&&(security_type!= SHARED)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0)))
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"The security type is %d\n",security_type);
			if((security_type == WPA_E)||(security_type == WPA2_E)||(security_type == WPA_P)||(security_type == WPA2_P)||(security_type == IEEE8021X)||(security_type == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))){
					
				ASD_SECURITY[security_id]->eap_alive_period= period;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"security_id = %d\n",security_id);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,	"ASD_SECURITY[security_id]->eap_alive_period=  %d\n",ASD_SECURITY[security_id]->eap_alive_period);
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

//weichao add 2011 .12.01
DBusMessage *asd_dbus_account_after_authorize(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int state;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
			security_type = ASD_SECURITY[security_id]->securityType;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"The security type is %d\n",security_type);
			if((security_type == WPA_E)||(security_type == WPA2_E)||(security_type == WPA_P)||(security_type == WPA2_P)||(security_type == IEEE8021X)||(security_type == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))){
					
				ASD_SECURITY[security_id]->account_after_authorize= state;
				if(state == 1)
				{
					ASD_SECURITY[security_id]->account_after_dhcp = 0;
				}
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"security_id = %d\n",security_id);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,	"ASD_SECURITY[security_id]->account_after_authorize=  %d\n",ASD_SECURITY[security_id]->account_after_authorize);
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

//weichao add 2011 .12.01
DBusMessage *asd_dbus_account_after_dhcp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int state;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
			security_type = ASD_SECURITY[security_id]->securityType;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"The security type is %d\n",security_type);
			if((security_type == WPA_E)||(security_type == WPA2_E)||(security_type == WPA_P)||(security_type == WPA2_P)||(security_type == IEEE8021X)||(security_type == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))){

				if(0 == ASD_SECURITY[security_id]->account_after_authorize)	
				{
					ASD_SECURITY[security_id]->account_after_dhcp= state;
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"security_id = %d\n",security_id);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,	"ASD_SECURITY[security_id]->account_after_dhcp=  %d\n",ASD_SECURITY[security_id]->account_after_dhcp);
				}	
				else
					ret = ASD_ACCOUNT_AFTER_AUTHORIZE;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
DBusMessage *asd_dbus_set_ap_detect_interval(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int interval; 
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&interval,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			ASD_SECURITY[security_id]->ap_max_inactivity = interval;
			asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->ap_max_inactivity %d\n",ASD_SECURITY[security_id]->ap_max_inactivity);
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	return reply;
}

DBusMessage *asd_dbus_apply_wlan(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id,wlan_id;
	unsigned int ret = ASD_DBUS_SUCCESS;
	int i = 0;
	//int len = 0;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = ASD_SECURITY_PROFILE_CHECK(security_id);
	if(ret == ASD_DBUS_SUCCESS){	
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i=0; i<5; i++) {
			if(ASD_WLAN[wlan_id] != NULL){
				if((ASD_WLAN[wlan_id]->Status == 1)){
					if(ASD_WLAN[wlan_id]->SecurityID != 0){

						ASD_WLAN[wlan_id]->OldSecurityIndex =  ASD_WLAN[wlan_id]->SecurityIndex;	
						ASD_WLAN[wlan_id]->OldSecurityIndex_flag = ASD_WLAN[wlan_id]->NowSecurityIndex_flag;
					}
				
					if(((ASD_SECURITY[security_id]->securityType == OPEN)||(ASD_SECURITY[security_id]->securityType == SHARED))&&(ASD_SECURITY[security_id]->encryptionType == WEP))
					{
						ASD_WLAN[wlan_id]->NowSecurityIndex_flag = 1;
					}
					else
					{
						ASD_WLAN[wlan_id]->NowSecurityIndex_flag = 0;
					}
					
					ASD_WLAN[wlan_id]->SecurityID = security_id;
					ASD_WLAN[wlan_id]->SecurityIndex = ASD_SECURITY[security_id]->index;  //fengwenchao add 20110310 for autelan-2200
					ASD_WLAN[wlan_id]->ap_max_inactivity = ASD_SECURITY[security_id]->ap_max_inactivity;//weichao add 
					asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_WLAN_INF_OP\n");
					if(ASD_WLAN_INF_OP(wlan_id, security_id, WID_MODIFY))
						asd_printf(ASD_DBUS,MSG_DEBUG,"update wlan security type\n");
					ret = ASD_DBUS_SUCCESS;
					break;
					/*ASDCmdMsg cmdmsg;
					memset(&cmdmsg, 0, sizeof(ASDCmdMsg));	
					cmdmsg.Op = ASD_CMD_APPLY_SECURITY;
					cmdmsg.Type = ASD_SECURITY_TYPE;
					cmdmsg.u.secinfo.SID = security_id;
					cmdmsg.u.secinfo.wlanid = wlan_id;
					len = sizeof(cmdmsg);
					if(sendto(CmdSock_s, &cmdmsg, len, 0, (struct sockaddr *) &toASD_C.addr, toASD_C.addrlen) < 0){
						perror("send(wASDSocket)");
						asd_printf(ASD_DBUS,MSG_DEBUG,"sssssssssss\n");
					}
					break;*/
				}else
					ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
			}else {
				ret = ASD_WLAN_NOT_EXIST;
				sleep(1);
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	

	signal_key_conflict();
	signal_de_key_conflict();

	return reply;
}

DBusMessage *asd_dbus_set_security_host_ip(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	char* host_ip;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_STRING,&host_ip,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){
		int i =0 , isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1){
				ret = ASD_SECURITY_HEART_TEST_ON;
			}
			else{
				if(ASD_SECURITY[security_id]->host_ip != NULL){
					free(ASD_SECURITY[security_id]->host_ip);
					ASD_SECURITY[security_id]->host_ip = NULL;
				}
				ASD_SECURITY[security_id]->host_ip = (char*)os_zalloc(strlen(host_ip)+1);
				if(ASD_SECURITY[security_id]->host_ip!=NULL){
					memset(ASD_SECURITY[security_id]->host_ip, 0, strlen(host_ip)+1);
					memcpy(ASD_SECURITY[security_id]->host_ip, host_ip, strlen(host_ip));
				}	//	0608xm
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


DBusMessage *asd_dbus_set_security_type(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&security_type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){
		int i =0 , isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1){
				ret = ASD_SECURITY_HEART_TEST_ON;
			}
			else{
				if((security_type == IEEE8021X)||(security_type == WPA_E)
					||(security_type == WPA2_E)||(security_type == MD5))
					ASD_SECURITY[security_id]->RadiusID = security_id;
				else
					ASD_SECURITY[security_id]->RadiusID = 0;	
				if(ASD_SECURITY[security_id]->securityType != security_type)
				{
					Clear_SECURITY(security_id);
					ret=ASD_SECURITY_TYPE_HAS_CHANGED;
				}
				//mahz modified 2011.3.3
				if((security_type != OPEN)&&(security_type != SHARED)){
					ASD_SECURITY[security_id]->extensible_auth = 0;
				//	ASD_SECURITY[security_id]->hybrid_auth = 0;
				}
				ASD_SECURITY[security_id]->securityType = security_type;
				SECURITY_ENCYPTION_MATCH(security_id, security_type);
				if((ASD_SECURITY[security_id]->securityType==WAPI_PSK)||
					(ASD_SECURITY[security_id]->securityType==WAPI_AUTH))
				SECURITY_WAPI_INIT(security_id);
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage *asd_dbus_set_encryption_type(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	unsigned int encryption_type;	
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&encryption_type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				security_type = ASD_SECURITY[security_id]->securityType;
				if(((security_type == OPEN)&&((encryption_type == NONE)||(encryption_type == WEP)))
					||((security_type == SHARED)&&(encryption_type == WEP))
					||((security_type == IEEE8021X)&&(encryption_type == WEP))
					||(((security_type == WPA_P)||(security_type == WPA_E)||(security_type == WPA2_P)||(security_type == WPA2_E))&&((encryption_type == AES)||(encryption_type == TKIP)))
					||((ASD_SECURITY[security_id]->securityType == MD5)&&(encryption_type == NONE))
					||(((ASD_SECURITY[security_id]->securityType == WAPI_AUTH)||(ASD_SECURITY[security_id]->securityType == WAPI_PSK))&&(encryption_type == SMS4))){				
					if(ASD_SECURITY[security_id]->encryptionType!= encryption_type)
					{
					Clear_SECURITY(security_id);
					ret=ASD_SECURITY_TYPE_HAS_CHANGED;
					}
					ASD_SECURITY[security_id]->encryptionType = encryption_type;
					if((encryption_type == NONE)&&(ASD_SECURITY[security_id]->SecurityKey != NULL)){
						free(ASD_SECURITY[security_id]->SecurityKey);
						ASD_SECURITY[security_id]->SecurityKey = NULL;
						ASD_SECURITY[security_id]->keyLen = 0;
					}
				}
				else
					ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


DBusMessage *asd_dbus_set_extensible_auth(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	unsigned int state;	
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				security_type = ASD_SECURITY[security_id]->securityType;
				if((security_type == OPEN)||((security_type == SHARED))){				
					if((ASD_SECURITY[security_id]->extensible_auth == 1)&&(state == 0)){
						if(ASD_SECURITY[security_id]->acct.acct_ip != NULL){
							free(ASD_SECURITY[security_id]->acct.acct_ip);
							ASD_SECURITY[security_id]->acct.acct_ip = NULL;
						}
						if(ASD_SECURITY[security_id]->acct.acct_shared_secret){
							free(ASD_SECURITY[security_id]->acct.acct_shared_secret);
							ASD_SECURITY[security_id]->acct.acct_shared_secret = NULL;
						}		
						if(ASD_SECURITY[security_id]->auth.auth_ip != NULL){
							free(ASD_SECURITY[security_id]->auth.auth_ip);
							ASD_SECURITY[security_id]->auth.auth_ip = NULL;
						}
						if(ASD_SECURITY[security_id]->auth.auth_shared_secret != NULL){
							free(ASD_SECURITY[security_id]->auth.auth_shared_secret);
							ASD_SECURITY[security_id]->auth.auth_shared_secret = NULL;
						}
					}
					ASD_SECURITY[security_id]->extensible_auth = state;
					if(ASD_SECURITY[security_id]->extensible_auth == 1)
						ASD_SECURITY[security_id]->mac_auth = 0;
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_SECURITY[security_id]->extensible_auth %d",ASD_SECURITY[security_id]->extensible_auth);
				}
				else
					ret = ASD_EXTENSIBLE_AUTH_NOT_SUPPORT;
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage *asd_dbus_radius_server_select(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	unsigned int state;	
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}			
			if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))){
				ASD_SECURITY[security_id]->wired_radius = state;
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->wired_radius %d\n",ASD_SECURITY[security_id]->wired_radius);
			}else
				ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


DBusMessage *asd_dbus_wlan_check(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char wlan_id;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_WLAN[wlan_id] != NULL)){
		if(ASD_WLAN[wlan_id]->SecurityID != 0)
			ret = ASD_DBUS_SUCCESS;
		else
			ret = ASD_SECURITY_PROFILE_NOT_BIND_WLAN;
	}else{
		ret = ASD_WLAN_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

/********************************************************************************/
//xm 08/09/01
DBusMessage *asd_dbus_config_port_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
		DBusMessage* reply;
		DBusError err;
		DBusMessageIter  iter;
		DBusMessageIter  iter_array;
		int ret = ASD_DBUS_SUCCESS;
		
		int rcv_count=0;
		int i=0;
		
		SLOT_PORT_ENABLE_S	spe[vlan_max_num] ;	
		memset(spe,0,sizeof(SLOT_PORT_ENABLE_S)*vlan_max_num);
				
			
		dbus_error_init(&err);
		
		dbus_message_iter_init(msg,&iter);
		dbus_message_iter_get_basic(&iter,&rcv_count);
							
		if(rcv_count> 0 ){
				
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
					
			for (i = 0; i < rcv_count; i++) {
				DBusMessageIter iter_struct;
						
				dbus_message_iter_recurse(&iter_array,&iter_struct);
					
				dbus_message_iter_get_basic(&iter_struct,&(spe[i].slot));
					
				dbus_message_iter_next(&iter_struct);
						
				dbus_message_iter_get_basic(&iter_struct,&(spe[i].port));
	
				dbus_message_iter_next(&iter_struct);
						
				dbus_message_iter_get_basic(&iter_struct,&(spe[i].stat));
					
				dbus_message_iter_next(&iter_array);			
			
			}
		
		}
	
		
		reply = dbus_message_new_method_return(msg);
			
		dbus_message_iter_init_append(reply, &iter);
			
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
			
		return reply;	

}


DBusMessage *asd_dbus_config_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter,iiter;
	DBusMessageIter  iter_array;
	int ret = ASD_DBUS_SUCCESS;
	
	int rcv_count=0;
	int i=0;
	
	PORTINDEX_VLANID_S  pvs[vlan_max_num] ;    
	memset(pvs,0,sizeof(PORTINDEX_VLANID_S)*vlan_max_num);
			
		
	dbus_error_init(&err);
	
	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&rcv_count);		
			
	if(rcv_count> 0 ){
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
				
		for (i = 0; i < rcv_count; i++) {
			DBusMessageIter iter_struct;
					
			dbus_message_iter_recurse(&iter_array,&iter_struct);
				
			dbus_message_iter_get_basic(&iter_struct,&(pvs[i].port_index));
				
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(pvs[i].vlan_id));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(pvs[i].security));
				
			dbus_message_iter_next(&iter_array);

		}
	
	}

	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iiter);
		
	dbus_message_iter_append_basic(&iiter, DBUS_TYPE_UINT32, &ret);
		
	return reply;	
}
//xm 08/09/01
/********************************************************************************/

DBusMessage *asd_dbus_set_security_key(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	char *key;
	enum key_input_type type = ASCII;
	unsigned char type_char;
	unsigned int len;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_STRING,&key,
								DBUS_TYPE_BYTE,&type_char, //xm add 08/11/25
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	asd_printf(ASD_DBUS,MSG_DEBUG,"key %s\ntpye %d\n",key,type_char);

	if(type_char==1)
		type=ASCII;
	else if(type_char==2)
		type=HEX;
		
	if((ASD_SECURITY[security_id]!= NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->SecurityKey != NULL){
				free(ASD_SECURITY[security_id]->SecurityKey);
				ASD_SECURITY[security_id]->SecurityKey = NULL;
				ASD_SECURITY[security_id]->keyLen= 0;
			}
			if(((ASD_SECURITY[security_id]->securityType == OPEN)&&(ASD_SECURITY[security_id]->encryptionType == NONE))||(ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)
				||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||(ASD_SECURITY[security_id]->securityType == WAPI_AUTH))
				ret = ASD_SECURITY_KEY_NOT_PERMIT;
			else if(ASD_SECURITY[security_id]->SecurityKey != NULL)
				ret = ASD_SECURITY_KEY_HAS_BEEN_SET;
			else if((ASD_SECURITY[security_id]->securityType == WPA_P)||(ASD_SECURITY[security_id]->securityType == WPA2_P)){
				if(type==ASCII){
					len = strlen(key);
					asd_printf(ASD_DBUS,MSG_DEBUG,"key %s keylen %d\n",key,len);
					if((len < 8)||(len > 63)) 
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						ASD_SECURITY[security_id]->keyInputType=ASCII;
					}
				}
				else if(type==HEX){
					len = strlen(key);
					asd_printf(ASD_DBUS,MSG_DEBUG,"key %s keylen %d\n",key,len);
					if(0!=CheckHex(key))
						ret=ASD_SECURITY_KEY_HEX_FORMAT;
					else if(len !=64)    
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						
						ASD_SECURITY[security_id]->keyInputType=HEX;
					}
				}
			}
			else if(ASD_SECURITY[security_id]->securityType == WAPI_PSK){
				if(type==ASCII){
					len = strlen(key);
					asd_printf(ASD_DBUS,MSG_DEBUG,"key %s keylen %d\n",key,len);
					if(len < 8) 
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						ASD_SECURITY[security_id]->keyInputType=ASCII;
					}
				}
				else if(type==HEX){
					len = strlen(key);
					asd_printf(ASD_DBUS,MSG_DEBUG,"key %s keylen %d\n",key,len);
					if(0!=CheckHex(key))
						ret=ASD_SECURITY_KEY_HEX_FORMAT;
					else if(len <16)    
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						ASD_SECURITY[security_id]->keyInputType=HEX;
					}
				}

			}
			else if(ASD_SECURITY[security_id]->securityType == SHARED){
				if(type==ASCII){
					len = strlen(key);			
					if((len != 5)&&(len != 13)&&(len != 16))
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						ASD_SECURITY[security_id]->keyInputType=ASCII;
					}
				}
				else if(type==HEX){
					len = strlen(key);	
					if(0!=CheckHex(key))
						ret=ASD_SECURITY_KEY_HEX_FORMAT;
					else if((len != 10)&&(len != 26)&&(len != 32))
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	xm	0608xm
						ASD_SECURITY[security_id]->keyInputType=HEX;
					}
				}
			}
			else if((ASD_SECURITY[security_id]->securityType == OPEN)&&(ASD_SECURITY[security_id]->encryptionType == WEP)){
				if(type==ASCII){
					len = strlen(key);
					if((len != 5)&&(len != 13)&&(len != 16))
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						ASD_SECURITY[security_id]->keyInputType=ASCII;
					}
				}
				else if(type==HEX){
					len = strlen(key);
					if(0!=CheckHex(key))
						ret=ASD_SECURITY_KEY_HEX_FORMAT;
					else if((len != 10)&&(len != 26)&&(len != 32))
						ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX;
					else{
						ASD_SECURITY[security_id]->SecurityKey = (char*)os_zalloc(len+1);
						if(ASD_SECURITY[security_id]->SecurityKey!=NULL){
							memset(ASD_SECURITY[security_id]->SecurityKey, 0, len+1);
							memcpy(ASD_SECURITY[security_id]->SecurityKey, key, len);
							ASD_SECURITY[security_id]->keyLen = len;
						}	//	0608xm
						ASD_SECURITY[security_id]->keyInputType=HEX;
					}
				}
			}	
		}		
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	signal_key_conflict();
	signal_de_key_conflict();


	return reply;
}

DBusMessage *asd_dbus_set_pre_authentication(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char security_id;
	unsigned int state;	
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type == WPA2_E)){
				ASD_SECURITY[security_id]->pre_auth = state;
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->pre_auth %d",ASD_SECURITY[security_id]->pre_auth);
			}
			else
				ret = ASD_PRE_AUTH_NOT_SUPPORT;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

/*nl  add 09/10/10*/
DBusMessage *asd_dbus_set_wtp_send_response_to_mobile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
		DBusMessage* reply; 	
		DBusMessageIter  iter;
		DBusError err;
		int ret = ASD_DBUS_SUCCESS;
		
		unsigned char mobile_open;	//1--enable, 0--disable
		
		dbus_error_init(&err);
		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_BYTE,&mobile_open,
									DBUS_TYPE_INVALID))){
		
			asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
			if (dbus_error_is_set(&err)) {
				asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
		if(mobile_open==1)	
		{asd_printf(ASD_DBUS,MSG_DEBUG,"set wtp send response to mobile open");
		WTP_SEND_RESPONSE_TO_MOBILE=1;
		}
		else if(mobile_open==0)	
		{asd_printf(ASD_DBUS,MSG_DEBUG,"set wtp send response to mobile close");
		WTP_SEND_RESPONSE_TO_MOBILE=0;
		}
		
		
		
		reply = dbus_message_new_method_return(msg);
				
		dbus_message_iter_init_append (reply, &iter);	
	
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	
		return reply;	
}
DBusMessage *asd_dbus_set_accounting_on(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int state; 
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;		//mahz modified 2011.3.21
			if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))||(ASD_SECURITY[security_id]->wapi_radius_auth == 1)){
				ASD_SECURITY[security_id]->accounting_on_disable = state;
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->accounting_on_disable %d",ASD_SECURITY[security_id]->accounting_on_disable);
			}
			else
				ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

//mahz add 2011.10.25
DBusMessage *asd_dbus_set_asd_distribute_on(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int state; 
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				security_type = ASD_SECURITY[security_id]->securityType;		//mahz modified 2011.3.21
				if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||(ASD_SECURITY[security_id]->extensible_auth == 1)||(ASD_SECURITY[security_id]->wapi_radius_auth == 1)){
					ASD_SECURITY[security_id]->distribute_off = state;
					asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->accounting_on_disable %d",ASD_SECURITY[security_id]->accounting_on_disable);
				}
				else
					ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

//mahz add 2011.10.25
DBusMessage *asd_dbus_set_asd_rdc_para(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int slot_value; 
	unsigned int inst_value; 
	unsigned int security_type;
	struct asd_ip_secret *secret_ip = NULL;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&slot_value,
								DBUS_TYPE_UINT32,&inst_value,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				security_type = ASD_SECURITY[security_id]->securityType;		//mahz modified 2011.3.21
				if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||(ASD_SECURITY[security_id]->extensible_auth == 1)||(ASD_SECURITY[security_id]->wapi_radius_auth == 1)){
					ASD_SECURITY[security_id]->slot_value = slot_value;
					ASD_SECURITY[security_id]->inst_value = inst_value;
					
					if(ASD_SECURITY[security_id]->acct.acct_ip != NULL)
					{
						secret_ip = asd_ip_secret_add(inet_addr(ASD_SECURITY[security_id]->acct.acct_ip));					
						if(secret_ip)
						{
							secret_ip->slot_value = ASD_SECURITY[security_id]->slot_value;
							secret_ip->inst_value = ASD_SECURITY[security_id]->inst_value;
						}
					}
					asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[%d]->slot_id: %d , inst_id: %d\n",security_id,
						ASD_SECURITY[security_id]->slot_value,ASD_SECURITY[security_id]->inst_value);
					
				}
				else
					ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage *asd_dbus_show_asd_rdc_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char num=0;
	DBusError err;

	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);

	int i=0;
	security_profile *Security[WLAN_NUM-1];

	while(i<WLAN_NUM){
		if((ASD_SECURITY[i] != NULL)&&((ASD_SECURITY[i]->securityType == IEEE8021X)||(ASD_SECURITY[i]->securityType == WPA_E)
		||(ASD_SECURITY[i]->securityType == WPA2_E)||(ASD_SECURITY[i]->securityType == MD5)||(ASD_SECURITY[i]->wapi_radius_auth == 1)
		||((ASD_SECURITY[i]->extensible_auth == 1)&&(ASD_SECURITY[i]->hybrid_auth == 0))))
		{
			Security[num] = ASD_SECURITY[i];
			num++;
		}
		i++;
	}
	if(num == 0)
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &num);
		
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	for(i = 0; i < num; i++){
			DBusMessageIter iter_struct;			
			dbus_message_iter_open_container (&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &(Security[i]->SecurityID));
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(Security[i]->slot_value));
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(Security[i]->inst_value));
			dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	return reply;	
}
DBusMessage *asd_dbus_set_radius_extend_attr(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int state; 
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;
			if((ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))){
				ASD_SECURITY[security_id]->radius_extend_attr = state;
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->radius_extend_attr %d",ASD_SECURITY[security_id]->radius_extend_attr);
			}
			else
				ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


//mahz add 2010.11.24
DBusMessage *asd_dbus_set_wapi_radius_auth(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int state; 
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;
			if(ASD_SECURITY[security_id]->securityType == WAPI_AUTH){
				ASD_SECURITY[security_id]->wapi_radius_auth = state;
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->wapi_radius_auth %d",ASD_SECURITY[security_id]->wapi_radius_auth);
			}
			else
				ret = ASD_SECURITY_TYPE_WITHOUT_8021X;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}


//mahz add 2011.2.18
DBusMessage *asd_dbus_set_hybrid_auth(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int state; 
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;
			//if(ASD_SECURITY[security_id]->securityType == OPEN){
				if(ASD_SECURITY[security_id]->hybrid_auth != state)
				{
					ASD_SECURITY[security_id]->hybrid_auth = state;
					if(ASD_SECURITY[security_id]->hybrid_auth == 0)
					{					
						ASD_SECURITY[security_id]->mac_auth = 0 ;
						asd_ipset_switch--;
						if(asd_ipset_switch  == 0)
						{
							eap_clean_all_user(); 
							eap_auth_exit();
						}
					}
					else
					{
						asd_ipset_switch++;
						if(asd_ipset_switch == 1)
							eap_auth_init();
					}
					asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->hybrid_auth %s\n",(ASD_SECURITY[security_id]->hybrid_auth == 1)?"enable":"disable");
				}	
			
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	return reply;
}

//mahz add 2011.7.8
DBusMessage *asd_dbus_set_fast_auth(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int state; 
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((ASD_SECURITY[security_id] != NULL)){		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++){
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id)){
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id)){
				isApply = 1;
				continue;
			}
		}
		if(ret == 0){
			if(isApply == 1){
				Clear_WLAN_APPLY(security_id);
			}
			security_type = ASD_SECURITY[security_id]->securityType;
			if((ASD_SECURITY[security_id]->securityType == WPA_E)||(ASD_SECURITY[security_id]->securityType == WPA2_E)||(ASD_SECURITY[security_id]->securityType == WPA_P)||(ASD_SECURITY[security_id]->securityType == WPA2_P)||(ASD_SECURITY[security_id]->securityType == IEEE8021X)||(ASD_SECURITY[security_id]->securityType == MD5)||(ASD_SECURITY[security_id]->extensible_auth == 1)){
				ASD_SECURITY[security_id]->fast_auth = state;
				asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_SECURITY[security_id]->fast_auth %s\n",(ASD_SECURITY[security_id]->fast_auth == 1)?"enable":"disable");
			}
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}else{
		ret = ASD_SECURITY_NOT_EXIST;
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	return reply;
}

//mahz add 2011.3.17
DBusMessage *asd_dbus_show_asd_global_variable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	dbus_error_init(&err);
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_NOTICE_STA_INFO_TO_PORTAL);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &WTP_SEND_RESPONSE_TO_MOBILE);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &dbus_count_switch);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &STA_STATIC_FDB_ABLE);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ASD_SWITCH);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &asd_sta_arp_listen);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &asd_sta_static_arp);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &asd_sta_idle_time_switch);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &asd_sta_idle_time);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &asd_bak_sta_update_time);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &asd_ipset_switch);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &asd_sta_getip_from_dhcpsnoop);
	

	return reply;
}

//mahz add 2011.10.17
DBusMessage *asd_dbus_set_asd_sock_oper(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	int len;
	unsigned char socketfd;
	unsigned char operate; 
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&socketfd,
								DBUS_TYPE_BYTE,&operate,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ASDCmdMsg cmdmsg;
	memset(&cmdmsg, 0, sizeof(ASDCmdMsg));	
	cmdmsg.Type = ASD_FD_TYPE;
	cmdmsg.u.fdinit.socketfd = socketfd;
	cmdmsg.u.fdinit.operate = operate;
	len = sizeof(cmdmsg);
	if(sendto(CmdSock_s, &cmdmsg, len, 0, (struct sockaddr *) &toASD_C.addr, toASD_C.addrlen) < 0){
		perror("send(wASDSocket)");
		asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
	}
#if 0
	if(operate == 1){
		if(socketfd == 1){
			circle_unregister_read_sock(TableSock);
			close(TableSock);
			tablesock_flag = 1;
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close TableSock : %d\n",TableSock);
		}
		else if(socketfd == 2){
			circle_unregister_read_sock(DataSock);
			close(DataSock);
			datasock_flag = 1;
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close DataSock : %d\n",DataSock);
		}
		else if(socketfd == 3){
			close(TableSend);
			tablesend_flag = 1;
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close TableSend : %d\n",TableSend);
		}
		else if(socketfd == 4){
			close(DataSend);
			datasend_flag = 1;
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close DataSend : %d\n",DataSend);
		}
	}
	else if(operate == 2){
		if((socketfd == 1)&&(tablesock_flag == 1)){
			InitPath(vrrid,AWSM_PATH);
			TableSock = socket(PF_UNIX, SOCK_DGRAM, 0);
			if (TableSock < 0) {
			   asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
			   ret = ASD_SOCK_NOT_EXIST;
			}
			local.sun_family = PF_UNIX;
			strcpy(local.sun_path,AWSM_PATH);
			unlink(local.sun_path);    
			len = strlen(local.sun_path) + sizeof(local.sun_family);	
			asd_printf(ASD_DBUS,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);   
			
			if (bind(TableSock, (struct sockaddr *) &local, len) < 0) {
				asd_printf(ASD_DBUS,MSG_CRIT,"%s bind error\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			asd_printf(ASD_DBUS,MSG_DEBUG,"after bind sock TableSock: %d\n",TableSock);   
			if (circle_register_read_sock(TableSock, handle_msg, NULL, NULL)){
				asd_printf(ASD_DBUS,MSG_WARNING,"Could not register read socket\n");
				ret = ASD_SOCK_NOT_EXIST;
			}
			tablesock_flag = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"after register read sock TableSock: %d\n",TableSock);   
		}
		else if((socketfd == 2)&&(datasock_flag == 1)){
			InitPath(vrrid,AWSM_PATHWSM);
			DataSock = socket(PF_UNIX, SOCK_DGRAM, 0);
			if (DataSock < 0 ) {
				asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			local.sun_family = PF_UNIX;
			strcpy(local.sun_path,AWSM_PATHWSM);
			unlink(local.sun_path); 
			len = strlen(local.sun_path) + sizeof(local.sun_family);
			asd_printf(ASD_DBUS,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);   

			if (bind(DataSock, (struct sockaddr *) &local, len) < 0) {
				asd_printf(ASD_DBUS,MSG_CRIT,"%s bind error\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			asd_printf(ASD_DBUS,MSG_DEBUG,"after bind sock DataSock: %d\n",DataSock);   

			if (circle_register_read_sock(DataSock, handle_read, NULL, NULL)){
				asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
				ret = ASD_SOCK_NOT_EXIST;
			}
			datasock_flag = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"after register read sock DataSock: %d\n",DataSock);   
		}
		else if((socketfd == 3)&&(tablesend_flag == 1)){
			TableSend = socket(PF_UNIX, SOCK_DGRAM, 0);
			if(TableSend < 0){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			if ((setsockopt(TableSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
				asd_printf(ASD_DBUS,MSG_CRIT,"setsockopt TableSend %d failed\n",TableSend);
				ret = ASD_SOCK_NOT_EXIST;
			}
			tablesend_flag = 0;
			fcntl(TableSend, F_SETFL, O_NONBLOCK);
			asd_printf(ASD_DBUS,MSG_DEBUG,"after setsockopt TableSend: %d\n",TableSend);   
		}
		else if((socketfd == 4)&&(datasend_flag == 1)){
			DataSend = socket(PF_UNIX, SOCK_DGRAM, 0);
			if(DataSend < 0){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			if ((setsockopt(DataSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
				asd_printf(ASD_DBUS,MSG_CRIT,"setsockopt TableSend %d failed\n",DataSend);
				ret = ASD_SOCK_NOT_EXIST;
			}
			datasend_flag = 0;
			fcntl(DataSend, F_SETFL, O_NONBLOCK);
			asd_printf(ASD_DBUS,MSG_DEBUG,"after setsockopt DataSend: %d\n",DataSend);   
		}
		else{
			ret = ASD_SOCK_NOT_EXIST;
			asd_printf(ASD_DBUS,MSG_DEBUG,"the socket should be close first\n");   
		}
	}
	else if(operate == 3){
		if(socketfd == 1){
			circle_unregister_read_sock(TableSock);
			close(TableSock);
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close TableSock : %d\n",TableSock);

			InitPath(vrrid,AWSM_PATH);
			TableSock = socket(PF_UNIX, SOCK_DGRAM, 0);
			if (TableSock < 0) {
			   asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
			   ret = ASD_SOCK_NOT_EXIST;
			}
			local.sun_family = PF_UNIX;
			strcpy(local.sun_path,AWSM_PATH);
			unlink(local.sun_path);    
			len = strlen(local.sun_path) + sizeof(local.sun_family);	
			asd_printf(ASD_DBUS,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);   
			
			if (bind(TableSock, (struct sockaddr *) &local, len) < 0) {
				asd_printf(ASD_DBUS,MSG_CRIT,"%s bind error\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			asd_printf(ASD_DBUS,MSG_DEBUG,"after bind sock TableSock: %d\n",TableSock);   
			if (circle_register_read_sock(TableSock, handle_msg, NULL, NULL)){
				asd_printf(ASD_DBUS,MSG_WARNING,"Could not register read socket\n");
				ret = ASD_SOCK_NOT_EXIST;
			}
			tablesock_flag = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"after register read sock TableSock: %d\n",TableSock);   
		}
		else if(socketfd == 2){
			circle_unregister_read_sock(DataSock);
			close(DataSock);
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close DataSock : %d\n",DataSock);

			InitPath(vrrid,AWSM_PATHWSM);
			DataSock = socket(PF_UNIX, SOCK_DGRAM, 0);
			if (DataSock < 0 ) {
				asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			local.sun_family = PF_UNIX;
			strcpy(local.sun_path,AWSM_PATHWSM);
			unlink(local.sun_path); 
			len = strlen(local.sun_path) + sizeof(local.sun_family);
			asd_printf(ASD_DBUS,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);   

			if (bind(DataSock, (struct sockaddr *) &local, len) < 0) {
				asd_printf(ASD_DBUS,MSG_CRIT,"%s bind error\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			asd_printf(ASD_DBUS,MSG_DEBUG,"after bind sock DataSock: %d\n",DataSock);   

			if (circle_register_read_sock(DataSock, handle_read, NULL, NULL)){
				asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
				ret = ASD_SOCK_NOT_EXIST;
			}
			datasock_flag = 0;
			asd_printf(ASD_DBUS,MSG_DEBUG,"after register read sock DataSock: %d\n",DataSock);   
		}
		else if(socketfd == 3){
			close(TableSend);
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close TableSend : %d\n",TableSend);

			TableSend = socket(PF_UNIX, SOCK_DGRAM, 0);
			if(TableSend < 0){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			if ((setsockopt(TableSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
				asd_printf(ASD_DBUS,MSG_CRIT,"setsockopt TableSend %d failed\n",TableSend);
				ret = ASD_SOCK_NOT_EXIST;
			}
			tablesend_flag = 0;
			fcntl(TableSend, F_SETFL, O_NONBLOCK);
			asd_printf(ASD_DBUS,MSG_DEBUG,"after setsockopt TableSend: %d\n",TableSend);   
		}
		else if(socketfd == 4){
			close(DataSend);
			asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close DataSend : %d\n",DataSend);

			DataSend = socket(PF_UNIX, SOCK_DGRAM, 0);
			if(DataSend < 0){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
				ret = ASD_SOCK_NOT_EXIST;
			}
			if ((setsockopt(DataSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
				asd_printf(ASD_DBUS,MSG_CRIT,"setsockopt TableSend %d failed\n",DataSend);
				ret = ASD_SOCK_NOT_EXIST;
			}
			datasend_flag = 0;
			fcntl(DataSend, F_SETFL, O_NONBLOCK);
			asd_printf(ASD_DBUS,MSG_DEBUG,"after setsockopt DataSend: %d\n",DataSend);   
		}
	}
#endif		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	return reply;
}



/*
  *ht add,100618
  */
DBusMessage *asd_dbus_set_sta_static_fdb_able(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned char able = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&able,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if(able == 1){
		STA_STATIC_FDB_ABLE = 1;
	}else if(able == 0)	{
		STA_STATIC_FDB_ABLE = 0;
	}
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"set sta static fdb %s\n",(able == 1)?"enable":"disable");

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	return reply;	

}


/*
  *ht add,091111
  */
DBusMessage *asd_dbus_set_asd_process_80211n_able(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned char able = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&able,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if(able == 1){
		IEEE_80211N_ENABLE = 1;
	}else if(able == 0)	{
		IEEE_80211N_ENABLE = 0;
	}
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd process 80211n %s\n",(able == 1)?"enable":"disable");

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	return reply;	

}

#if 0
/*
  *ht add,091111
  */
DBusMessage *asd_dbus_set_asd_get_sta_info_able(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned char able = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&able,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if(able == 1){
		asd_get_sta_info_able = 1;
		circle_cancel_timeout(sta_info_to_wsm_timer, NULL, NULL);
		circle_register_timeout(0, 0, sta_info_to_wsm_timer, NULL, NULL);
	}else if(able == 0)	{
		asd_get_sta_info_able = 0;
		circle_cancel_timeout(sta_info_to_wsm_timer, NULL, NULL);
	}
	
	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd get sta info %s\n",(able == 1)?"enable":"disable");

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	return reply;	

}
#endif

DBusMessage *asd_dbus_set_asd_get_sta_info_time(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned int period = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&period,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	asd_get_sta_info_time = period;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	asd_printf(ASD_DBUS,MSG_DEBUG,"set asd get sta info time %d\n",period);
	
	return reply;	

}


/*nl  add 09/11/19*/
DBusMessage *asd_dbus_set_notice_sta_info_to_proto(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

		DBusMessage* reply; 	
		DBusMessageIter  iter;
		DBusError err;
		int ret = ASD_DBUS_SUCCESS;
		
		unsigned char notice_open;	//1--enable, 0--disable
		
		dbus_error_init(&err);
		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_BYTE,&notice_open,
									DBUS_TYPE_INVALID))){
		
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
			if (dbus_error_is_set(&err)) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
		if(notice_open==1)	
		{asd_printf(ASD_DEFAULT,MSG_DEBUG,"set notice sta info to portal open");
		ASD_NOTICE_STA_INFO_TO_PORTAL=1;
		}
		else if(notice_open==0)	
		{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"set notice sta info to portal  close");
			ASD_NOTICE_STA_INFO_TO_PORTAL=0;
			//flashdisconn_del_all_sta(ASD_FDStas);
		}
		
		
		
		reply = dbus_message_new_method_return(msg);
				
		dbus_message_iter_init_append (reply, &iter);	
	
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	
		return reply;	
}

DBusMessage *asd_dbus_set_notice_sta_info_to_portal_timer(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

		DBusMessage* reply; 	
		DBusMessageIter  iter;
		DBusError err;
		int ret = ASD_DBUS_SUCCESS;
		
		unsigned int timer = 10;
		
		dbus_error_init(&err);
		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&timer,
									DBUS_TYPE_INVALID))){
		
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
			if (dbus_error_is_set(&err)) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"set notice sta info to portal timer %d\n",timer);
		ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER = timer;
		
		
		
		reply = dbus_message_new_method_return(msg);
				
		dbus_message_iter_init_append (reply, &iter);	
	
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&ret); 
	
		return reply;	
}



/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_AuthenticationSuiteEnable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned char AuthenticationSuiteEnable;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&AuthenticationSuiteEnable,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->AuthenticationSuiteEnable= AuthenticationSuiteEnable;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_UnicastCipherEnabled(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned char UnicastCipherEnabled;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&UnicastCipherEnabled,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->UnicastCipherEnabled= UnicastCipherEnabled;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}


/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_MulticaseRekeyStrict(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned char MulticaseRekeyStrict;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&MulticaseRekeyStrict,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->MulticaseRekeyStrict= MulticaseRekeyStrict;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}




/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_WapiPreauth(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned char WapiPreauth;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&WapiPreauth,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->WapiPreauth = WapiPreauth;//qiuchen change it 2012.11.22
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_CertificateUpdateCount(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int CertificateUpdateCount;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&CertificateUpdateCount,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->CertificateUpdateCount= CertificateUpdateCount;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

/*nl add 09/11/02*/

DBusMessage *asd_dbus_set_wapi_sub_attr_multicastupdatecount(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int MulticastUpdateCount;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&MulticastUpdateCount,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->MulticastUpdateCount= MulticastUpdateCount;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_unicastupdatecount(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int UnicastUpdateCount;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&UnicastUpdateCount,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->UnicastUpdateCount= UnicastUpdateCount;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_bklifetime(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int BKLifetime;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&BKLifetime,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->BKLifetime= BKLifetime;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}


/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_bkreauththreshold(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int BKReauthThreshold;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&BKReauthThreshold,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->BKReauthThreshold= BKReauthThreshold;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}


/*nl add 09/11/02*/
DBusMessage *asd_dbus_set_wapi_sub_attr_satimeout(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned int SATimeout;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&SATimeout,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
	    }

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{ 			
				ASD_SECURITY[security_id]->wapi_config->SATimeout= SATimeout;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_wpa_group_rekey_period(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int period;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&period,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
		}

		if(ret == 0)
		{
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type == WPA_P)||(security_type == WPA2_P)||(security_type == WPA_E)||(security_type == WPA2_E))
			{			
				ASD_SECURITY[security_id]->wpa_group_rekey = period;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_wpa_keyupdate_timeout_period(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int period;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&period,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
		}

		if(ret == 0)
		{
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type == WPA_P)||(security_type == WPA2_P)||(security_type == WPA_E)||(security_type == WPA2_E))
			{			
				ASD_SECURITY[security_id]->wpa_keyupdate_timeout = period;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

//mahz add 2011.1.3
DBusMessage *asd_dbus_wpa_once_group_rekey_time(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int time;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&time,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
		}

		if(ret == 0)
		{
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type == WPA_P)||(security_type == WPA2_P)||(security_type == WPA_E)||(security_type == WPA2_E))
			{			
				ASD_SECURITY[security_id]->wpa_once_group_rekey_time = time;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_set_wapi_sub_attr_multicast_cipher(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	unsigned char type;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
		}

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
				
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type== WAPI_PSK)||(security_type==WAPI_AUTH ))
			{			
				ASD_SECURITY[security_id]->wapi_config->MulticastCipher[3] = type;
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
DBusMessage *asd_dbus_traffic_limit_from_radius(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage * reply;
	DBusMessageIter  iter;
	unsigned char security_id;
	unsigned int security_type;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;

	int state;
		
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
					
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if((ASD_SECURITY[security_id] != NULL))
	{		
		int i =0, isApply = 0;
		pthread_mutex_lock(&asd_g_wlan_mutex);
		for(i = 0; i < WLAN_NUM; i++)
		{
			if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->Status == 0)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				ret = ASD_SECURITY_WLAN_SHOULD_BE_DISABLE;
				break;
			}
			else if((ASD_WLAN[i]!=NULL)&&(ASD_WLAN[i]->SecurityID == security_id))
			{
				isApply = 1;
				continue;
			}
		}

		if(ret == 0)
		{
			if(isApply == 1)
				Clear_WLAN_APPLY(security_id);
			security_type = ASD_SECURITY[security_id]->securityType;
			if((security_type == WPA_E)||(security_type == WPA2_E)||(security_type == WPA_P)||(security_type == WPA2_P)||(security_type == IEEE8021X)||(security_type == MD5)||((ASD_SECURITY[security_id]->extensible_auth == 1)&&(ASD_SECURITY[security_id]->hybrid_auth == 0))){

				if(ASD_SECURITY[security_id]->traffic_limite_radius!=state)
				{
					ASD_SECURITY[security_id]->traffic_limite_radius = state;
				}
			}
			else
				ret = ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE;
		}
		pthread_mutex_unlock(&asd_g_wlan_mutex);
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
		
		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
//qiuchen add it for master_bak radius server 2012.12.11
DBusMessage *asd_dbus_set_ac_radius_name(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	char *name = NULL;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_STRING,&name,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				if(ASD_SECURITY[security_id]->ac_radius_name){
					free(ASD_SECURITY[security_id]->ac_radius_name);
					ASD_SECURITY[security_id]->ac_radius_name = NULL;
					ASD_SECURITY[security_id]->ac_radius_name = os_zalloc(strlen(name)+1);
					if(ASD_SECURITY[security_id]->ac_radius_name == NULL)
						ret = ASD_DBUS_MALLOC_FAIL;
					else
						memcpy(ASD_SECURITY[security_id]->ac_radius_name,name,strlen(name));
				}
				else{
					ASD_SECURITY[security_id]->ac_radius_name = os_zalloc(strlen(name)+1);
					if(ASD_SECURITY[security_id]->ac_radius_name == NULL)
						ret = ASD_DBUS_MALLOC_FAIL;
					else
						memcpy(ASD_SECURITY[security_id]->ac_radius_name,name,strlen(name));
				}
			}
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
DBusMessage *asd_dbus_set_radius_res_fail_suc_percent(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned char op = 0;
	double percent = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&op,
								DBUS_TYPE_DOUBLE,&percent,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				if(op == 1){
					ASD_SECURITY[security_id]->radius_res_fail_percent = percent;
				}else if (op == 2){
					ASD_SECURITY[security_id]->radius_res_suc_percent = percent;
				}
				else
					asd_printf(ASD_DBUS,MSG_DEBUG,"<error> something wrong with the op!\n");
			}
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_set_radius_access_test_interval(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned int interval = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_UINT32,&interval,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		
		if(ASD_SECURITY[security_id]->heart_test_on == 1)
			ret = ASD_SECURITY_HEART_TEST_ON;
		else
			ASD_SECURITY[security_id]->radius_access_test_interval = interval;
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}

DBusMessage *asd_dbus_set_radius_server_change_reuse_test_timer(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned char op = 0;
	unsigned int timer = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&op,
								DBUS_TYPE_UINT32,&timer,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
		if(ASD_SECURITY[security_id]->heart_test_on == 1)
			ret = ASD_SECURITY_HEART_TEST_ON;
		else{
			if(op == 1){
				ASD_SECURITY[security_id]->radius_server_change_test_timer = timer;
			}else if (op == 2){
				ASD_SECURITY[security_id]->radius_server_reuse_test_timer = timer;
			}
			else
				asd_printf(ASD_DBUS,MSG_DEBUG,"<error> something wrong with the op!\n");
		}
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
DBusMessage *asd_dbus_set_radius_server_heart_test_type(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned char op = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&op,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				if(op == 0){
					ASD_SECURITY[security_id]->radius_heart_test_type = RADIUS_AUTH_TEST;
				}else if (op == 1){
					ASD_SECURITY[security_id]->radius_heart_test_type = RADIUS_ACCT_TEST;
				}else if (op == 2){
					if(ASD_SECURITY[security_id]->auth.auth_ip != NULL &&
						ASD_SECURITY[security_id]->acct.acct_ip != NULL &&
						!strncmp(ASD_SECURITY[security_id]->auth.auth_ip,ASD_SECURITY[security_id]->acct.acct_ip,strlen(ASD_SECURITY[security_id]->acct.acct_ip))){
						ret = ASD_DBUS_RADIUS_HEART_TEST_UNCHANGE;
					}
					else
						//if(ASD_SECURITY[security_id]->radius_server_binding_type == RADIUS_SERVER_UNBINDED)
							ASD_SECURITY[security_id]->radius_heart_test_type = RADIUS_BOTH_TEST;
						//else
							//ret = ASD_DBUS_RADIUS_BOTH_UNBINED;
				}
				else
					asd_printf(ASD_DBUS,MSG_DEBUG,"<error> something wrong with the op!\n");
			}
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
DBusMessage *asd_dbus_set_radius_heart_test_on_off(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned char op = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&op,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
			if(op == 1){
				if(ASD_SECURITY[security_id]->heart_test_on == 1)
					ret = ASD_SECURITY_HEART_TEST_ON;
				else{
					ret = ASD_SEC_RADIUS_CHECK(security_id);
					if(ret == 0){
						if(ASD_SECURITY[security_id]->radius_conf)
							asd_free_radius(ASD_SECURITY[security_id]->radius_conf);
						ret = ASD_RADIUS_INIT(security_id);
						if(ret == 0){
							ASD_SECURITY[security_id]->heart_test_on = op;
							radius_heart_test_on(security_id);
						}
						else{
							radius_test_cancel_timers(security_id);
						}
					}
				}
			}else if (op == 0){
				ASD_SECURITY[security_id]->heart_test_on = op;
				radius_heart_test_off(security_id);
				//ASD_SECURITY[security_id]->auth_server_last_use = ASD_SECURITY[security_id]->auth_server_current_use;
				//ASD_SECURITY[security_id]->acct_server_last_use = ASD_SECURITY[security_id]->acct_server_current_use;
				//ASD_SECURITY[security_id]->acct_server_current_use = RADIUS_DISABLE;
				//ASD_SECURITY[security_id]->auth_server_current_use = RADIUS_DISABLE;
				ASD_SECURITY[security_id]->heart_test_identifier_auth = 32768;
				ASD_SECURITY[security_id]->heart_test_identifier_acct = 32768;
			}
			else
				asd_printf(ASD_DBUS,MSG_DEBUG,"<error> something wrong with the op!\n");
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
DBusMessage *asd_dbus_set_radius_server_binding_enable_disable(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int ret = ASD_DBUS_SUCCESS;
	unsigned char security_id = 0;
	unsigned char op = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&security_id,
								DBUS_TYPE_BYTE,&op,
								DBUS_TYPE_INVALID))){
		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(ASD_SECURITY[security_id] != NULL){
			if(ASD_SECURITY[security_id]->heart_test_on == 1)
				ret = ASD_SECURITY_HEART_TEST_ON;
			else{
				if(op == 1){
					//if(ASD_SECURITY[security_id]->radius_heart_test_type != RADIUS_BOTH_TEST)
						ASD_SECURITY[security_id]->radius_server_binding_type = RADIUS_SERVER_BINDED;
					//else
						//ret = ASD_DBUS_RADIUS_BOTH_UNBINED;
				}else if (op == 2){
					if(ASD_SECURITY[security_id]->auth.auth_ip == NULL || ASD_SECURITY[security_id]->acct.acct_ip == NULL)
						ret = ASD_SECURITY_PROFILE_NOT_INTEGRITY;
					else{
						if(!os_strncmp(ASD_SECURITY[security_id]->auth.auth_ip,ASD_SECURITY[security_id]->acct.acct_ip,strlen(ASD_SECURITY[security_id]->auth.auth_ip)))
							ret = ASD_SECURITY_RADIUSINIT_FAIL;
						else
							ASD_SECURITY[security_id]->radius_server_binding_type = RADIUS_SERVER_UNBINDED;
					}
				}
				else
					asd_printf(ASD_DBUS,MSG_DEBUG,"<error> something wrong with the op!\n");
			}
	}
	else
		ret = ASD_SECURITY_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;

}
//end

DBusMessage *asd_dbus_config_ac_group(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	unsigned char group_id;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group_id,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(AC_GROUP[group_id] == NULL)
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
	
}

DBusMessage *asd_dbus_create_ac_group(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char group_id;
	char *name;
	char *ESSID;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group_id,
								DBUS_TYPE_STRING,&name,
								DBUS_TYPE_STRING,&ESSID,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((AC_GROUP[group_id] == NULL)){
		ret = init_ac_group_info(group_id,name,ESSID);
	}
	else{
		ret = ASD_AC_GROUP_ID_USED;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;



}

DBusMessage *asd_dbus_delete_ac_group(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char group_id;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group_id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((AC_GROUP[group_id] != NULL)){
		ret = del_ac_group_info(group_id);
	}
	else{
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;



}


DBusMessage *asd_dbus_set_host_ip(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char group_id;
	char *ip = NULL;
	unsigned int ret = ASD_DBUS_SUCCESS;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group_id,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((AC_GROUP[group_id] != NULL)){
		AC_GROUP[group_id]->host_addr.sin_family = AF_INET;	
		AC_GROUP[group_id]->host_addr.sin_port = htons(0); 
		AC_GROUP[group_id]->host_addr.sin_addr.s_addr = inet_addr(ip);
		AC_GROUP[group_id]->host_ip = inet_addr(ip);
#if 0

		if(AC_G_FIRST){
			AC_G_FIRST = 0;			
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s gsock %d\n",__func__,gsock);			
			if (circle_register_read_sock(gsock, asd_ac_group_accept_select, NULL, NULL))
			{
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s Could not register read socket\n",__func__);
				return;
			}
			pthread_t ASD_GROUP;
			pthread_attr_t attr;
			size_t ss;
			int s = PTHREAD_CREATE_DETACHED;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,s);
			if(pthread_create(&ASD_GROUP, &attr, asd_ac_group_thread, NULL) != 0) {
				asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
				return -1;
			}
		}
		#endif
		
		if(AC_G_FIRST){
			AC_G_FIRST = 0; 		
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s gsock %d\n",__func__,gsock); 		
			if (circle_register_read_sock(gsock, asd_ac_group_accept_select, NULL, NULL))
			{
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s Could not register read socket\n",__func__);
				return NULL;
			}
		}
	}
	else{
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;



}

DBusMessage *asd_dbus_add_ac_group_member(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char group_id;
	unsigned char ac_id;
	char *ip = NULL;
	Mobility_AC_Info *vAC;
	unsigned int ret = ASD_DBUS_SUCCESS;
	gThreadArg *tmp;
	int sock;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group_id,
								DBUS_TYPE_BYTE,&ac_id,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((AC_GROUP[group_id] != NULL)){
		if(AC_GROUP[group_id]->Mobility_AC[ac_id] == NULL){
			vAC = AC_GROUP[group_id]->Mobility_AC[ac_id] = (Mobility_AC_Info *)malloc(sizeof(Mobility_AC_Info)); 
			if(vAC==NULL){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
				exit(1);
			}	
			memset(vAC, 0, sizeof(Mobility_AC_Info));
			vAC->ACID = ac_id;
			vAC->vWTPID = 1024*group_id + ac_id;
			vAC->ACIP = inet_addr(ip);		
			vAC->ac_addr.sin_family = AF_INET; 
			vAC->ac_addr.sin_port = htons(10086); 
			vAC->ac_addr.sin_addr.s_addr = inet_addr(ip);
			vAC->GroupID = group_id;
			vAC->is_conn = 0;
			vAC->BSS = malloc(BSS_NUM*sizeof(Mobility_BSS_Info*));
			if(vAC->BSS==NULL){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
				exit(1);
			}	
			memset(vAC->BSS, 0, BSS_NUM*sizeof(Mobility_BSS_Info*));
			tmp = (gThreadArg*)malloc(sizeof(gThreadArg));
			if(tmp==NULL){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
				exit(1);
			}	
			memset(tmp, 0, sizeof(gThreadArg));
			tmp->group_id = group_id;
			tmp->ACID = ac_id;
			if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
			{	asd_printf(ASD_DBUS,MSG_ERROR,"%s tcp socket create failed\n",__func__);
				exit(1);
			}
			tmp->sock = sock;
			//fcntl(sock, F_SETFL, O_NONBLOCK);
			unsigned long ul = 1;
			ioctl(sock, FIONBIO, &ul);
			circle_register_timeout(0, 0,asd_synch_select, tmp, NULL);
			#if 0
			pthread_t ASD_GROUP;		
			pthread_attr_t attr;
			size_t ss;	
			int s = PTHREAD_CREATE_DETACHED;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,s);
			if(pthread_create(&ASD_GROUP, &attr, asd_synch_thread, tmp) != 0) {
				asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
				return -1;
			}
			#endif
			//G_Wsm_WTPOp(group_id,ac_id,WID_ADD);
		}else
			ret = ASD_AC_GROUP_MEMBER_EXIST;
	}
	else{
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;



}

DBusMessage *asd_dbus_del_ac_group_member(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage * reply;
	DBusMessageIter	 iter;
	unsigned char group_id;
	unsigned char ac_id;
	//Mobility_AC_Info *vAC;
	unsigned int ret = ASD_DBUS_SUCCESS;
	//gThreadArg *tmp;
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&group_id,								
								DBUS_TYPE_BYTE,&ac_id,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DBUS,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DBUS,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if((AC_GROUP[group_id] != NULL)){
		ret = del_ac_group_member(group_id,ac_id,0);
	}
	else{
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage * asd_dbus_show_ac_group_list(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	dbus_error_init(&err);
	unsigned int num=0;
	int ret = ASD_DBUS_SUCCESS;
	int i=0;
	Inter_AC_R_Group *ACGROUP[GROUP_NUM];

	while(i<GROUP_NUM){
		if(AC_GROUP[i] != NULL)
		{
			ACGROUP[num] = AC_GROUP[i];
			num++;
		}
		i++;
	}
	if(num == 0)
	{
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	}
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	if(ret == ASD_DBUS_SUCCESS)
	{	
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &num);
			
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_STRING_AS_STRING
												DBUS_TYPE_STRING_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);

		for(i = 0; i < num; i++){			
			DBusMessageIter iter_struct;
				
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						  &(ACGROUP[i]->GroupID));
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_STRING,
						  &(ACGROUP[i]->ESSID));

			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_STRING,
						  &(ACGROUP[i]->name));

			dbus_message_iter_close_container (&iter_array, &iter_struct);


		}
					
		dbus_message_iter_close_container (&iter, &iter_array);
	}			
	
	return reply;	
}

DBusMessage *asd_dbus_count_ac_roaming(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage * reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	ret = inter_ac_roaming_count;
	dbus_error_init(&err);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &inter_ac_roaming_count
									 );
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &inter_ac_roaming_in_count
									 );

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &inter_ac_roaming_out_count
									 );
	return reply;



}

DBusMessage * asd_dbus_show_ac_group(DBusConnection *conn, DBusMessage *msg, void *user_data){
	
	DBusMessage * reply;
	DBusMessageIter	 iter;
	DBusError err;
	DBusMessageIter	 iter_array;		
	unsigned char ID = 0;
	int ret=ASD_DBUS_SUCCESS;
	Mobility_AC_Info *ACM[G_AC_NUM];	
	int i;
	unsigned int num = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&ID,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	printf("1\n");
	if(AC_GROUP[ID] == NULL)
	{
		ret = ASD_AC_GROUP_ID_NOT_EXIST;
	}else{
		for(i = 0; i < G_AC_NUM; i++){
			if(AC_GROUP[ID]->Mobility_AC[i] != NULL){
				
				printf("2\n");
				ACM[num] = AC_GROUP[ID]->Mobility_AC[i];
				num++;
			}
		}
	}
	printf("3\n");
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &ret);
	printf("4\n");
	if(ret == 0)
	{
		printf("5\n");
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &(AC_GROUP[ID]->GroupID));
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &(AC_GROUP[ID]->ESSID));
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &(AC_GROUP[ID]->name));
		printf("6\n");

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(AC_GROUP[ID]->host_ip));

		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(num));
		printf("7\n");
			
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);

		for(i = 0; i < num; i++){			
			DBusMessageIter iter_struct;
				
			dbus_message_iter_open_container (&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			printf("8\n");
				
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_BYTE,
						  &(ACM[i]->ACID));
	
			dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(ACM[i]->ACIP));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
			printf("9\n");

		}
		printf("10\n");
					
		dbus_message_iter_close_container (&iter, &iter_array);		
	}			
	printf("11\n");

	return reply;	
}

/*ht add,10.03.05 */
DBusMessage *asd_dbus_set_asd_trap_able(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type = 0;
	unsigned char able = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_BYTE,&able,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if((able == 1) || (able == 0)){
		switch(type){
			case 1:
				wtp_deny_sta_able = able;
				break;
			case 2:			
				sta_verify_failed_able = able;
				break;
			case 3: 		
				sta_assoc_failed_able = able;		
				break;
			case 4: 		
				wapi_invalid_cert_able = able;		
				break;
			case 5: 		
				wapi_challenge_replay_able = able;	
				break;
			case 6: 		
				wapi_mic_juggle_able = able; 		
				break;
			case 7: 		
				wapi_low_safe_able = able;			
				break;
			case 8: 		
				wapi_addr_redirection_able = able;	
				break;
			default:
				ret = ASD_DBUS_ERROR;
				break;
		}
	}else 
		ret = ASD_DBUS_ERROR;
	
	asd_printf(ASD_DEFAULT,MSG_INFO,"set asd trap %d %s\n",type,(able == 1)?"enable":"disable");

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

	return reply;	

}

/*ht add,10.03.05 */
DBusMessage *asd_dbus_show_asd_trap_state(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &wtp_deny_sta_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &sta_verify_failed_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &sta_assoc_failed_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &wapi_invalid_cert_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &wapi_challenge_replay_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &wapi_mic_juggle_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &wapi_low_safe_able);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &wapi_addr_redirection_able);

	return reply;	

}

DBusMessage *asd_dbus_show_asd_dbus_count(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(dbus_count[0].count));
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(dbus_count[1].count));
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(dbus_count[2].count));
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(dbus_count[3].count));
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(dbus_count[4].count));
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(other_dbus_count));
		
	return reply;	

}

DBusMessage *asd_dbus_set_asd_dbus_count(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	int type;
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_INVALID))){

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	reply = dbus_message_new_method_return(msg);
	dbus_count_switch = type;
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		
	return reply;	

}


DBusMessage *asd_dbus_set_asd_sta_arp_listen(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char op = 0;	//1--listen 2--listen-and-set
	unsigned char type=0;	//1--enable, 0--disable
	int len = 0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&op,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((type == 1)&&(asd_sta_arp_listen == 0)){			
		ASDCmdMsg cmdmsg;
		memset(&cmdmsg, 0, sizeof(ASDCmdMsg)); 
		cmdmsg.Op = ASD_CMD_ADD;
		cmdmsg.Type = ASD_ARP_LISTEN;
		cmdmsg.u.arplisten.type = op;
		len = sizeof(cmdmsg);
		if(sendto(CmdSock_s, &cmdmsg, len, 0, (struct sockaddr *) &toASD_C.addr, toASD_C.addrlen) < 0){
			perror("send(wASDSocket)");
			asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
		}
#if 0
		rtnl_wilddump_request(&rth1, AF_INET, RTM_GETNEIGH);
		if (circle_register_read_sock(rth1.fd, do_asd_sta_arp_listen, NULL, NULL))
		{
			ret = ASD_DBUS_ERROR;
		}else 		
			asd_sta_arp_listen = op;
#endif		
	}else if((type == 1)&&(asd_sta_arp_listen != 0)){
		asd_sta_arp_listen = op;
	}else if((type == 0)&&(asd_sta_arp_listen != type)){	
		ASDCmdMsg cmdmsg;
		memset(&cmdmsg, 0, sizeof(ASDCmdMsg));	
		cmdmsg.Op = ASD_CMD_DEL;
		cmdmsg.Type = ASD_ARP_LISTEN;
		cmdmsg.u.arplisten.type = type;		
		len = sizeof(cmdmsg);
		if(sendto(CmdSock_s, &cmdmsg, len, 0, (struct sockaddr *) &toASD_C.addr, toASD_C.addrlen) < 0){
			perror("send(wASDSocket)");
			asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
		}
#if 0
		circle_unregister_read_sock(rth1.fd);
		asd_sta_arp_listen = type;
#endif
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}

DBusMessage *asd_dbus_set_asd_sta_get_ip_from_dhcpsnoop(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type=0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in func : %s\n",__func__);
	if(asd_sta_getip_from_dhcpsnoop != type){
		asd_sta_getip_from_dhcpsnoop = type;
		ret = ASD_DBUS_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}


DBusMessage *asd_dbus_set_asd_sta_static_arp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	unsigned char type=0;	//1--enable, 0--disable
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if((type == 1)&&(asd_sta_static_arp != type)){	
			asd_sta_static_arp = type;	
	}else if((type == 0)&&(asd_sta_static_arp != type)){
		asd_sta_static_arp = type;		
	}
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}

DBusMessage *asd_dbus_set_asd_sta_static_arp_if_group(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	
	unsigned char ID=0;	//1--enable, 0--disable
	unsigned int is_add = 0;
	char *name = NULL;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&is_add,
								DBUS_TYPE_BYTE,&ID,
								DBUS_TYPE_STRING,&name,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	unsigned char ret_flag = 0;
	char * ifname = NULL;
	ifname = (char*)malloc(strlen(name)+5);
	if(ifname == NULL)
		return NULL;
	memset(ifname, 0, strlen(name)+5);

	if(check_ve_interface(name, ifname)){
		asd_printf(ASD_DBUS,MSG_DEBUG,"input ve interface doesn't exist!\n"); 
		ret = ASD_IFNAME_NOT_EXIST;
		ret_flag = 1;
	}
	
	if((is_add == 1)&&(ret_flag == 0)){
		if(ASD_ARP_GROUP[ID] == NULL){
			ASD_ARP_GROUP[ID] = malloc(strlen(ifname)+1);
			memset(ASD_ARP_GROUP[ID], 0, strlen(ifname)+1);
			memcpy(ASD_ARP_GROUP[ID],ifname,strlen(ifname));
		}else {
			ret = ASD_ARP_GROUP_EXIST;
		}
	}else if((is_add == 2)&&(ret_flag == 0)){
		if(ASD_ARP_GROUP[ID] != NULL){
			free(ASD_ARP_GROUP[ID]);
			ASD_ARP_GROUP[ID] = NULL;
		}
	}else if((is_add == 3)&&(ret_flag == 0)){
		if(ASD_ARP_GROUP[ID] == NULL){
			ASD_ARP_GROUP[ID] = malloc(strlen(ifname)+1);
			memset(ASD_ARP_GROUP[ID], 0, strlen(ifname)+1);
			memcpy(ASD_ARP_GROUP[ID],ifname,strlen(ifname));
		}else {
			free(ASD_ARP_GROUP[ID]);
			ASD_ARP_GROUP[ID] = NULL;
			ASD_ARP_GROUP[ID] = malloc(strlen(ifname)+1);
			memset(ASD_ARP_GROUP[ID], 0, strlen(ifname)+1);
			memcpy(ASD_ARP_GROUP[ID],ifname,strlen(ifname));			
		}
	}	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
	free(ifname);
	ifname = NULL;
	return reply;	
}

DBusMessage *asd_dbus_set_asd_switch(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	unsigned int type;
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_INVALID))){

				
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	reply = dbus_message_new_method_return(msg);
	if(ASD_SWITCH != type){		
		int fd = open("/dev/wifi0", O_RDWR);		
		ret = ioctl(fd, WIFI_IOC_ASD_SWITCH, &type);
		ASD_SWITCH = type;
		close(fd);
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s ASD_SWITCH = %d.\n",__func__,ASD_SWITCH);
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		
	return reply;	

}

DBusMessage *asd_dbus_clean_wlan_sta(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char wlanid = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&wlanid,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	pthread_mutex_lock(&asd_g_wlan_mutex);
	if(ASD_WLAN[wlanid] == NULL)
		ret = ASD_WLAN_NOT_EXIST;
	else 
		wlan_free_stas(wlanid);
	pthread_mutex_unlock(&asd_g_wlan_mutex);
	
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}
//qiuchen add it 2012.10.23
DBusMessage *asd_dbus_sta_del_hotspotid(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;

	unsigned int hotspot_id = 0 ; 
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&hotspot_id,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"hotspot id = %d\n",hotspot_id);
	pthread_mutex_lock(&asd_g_hotspot_mutex);
	if((hotspot_id >= 1) && ( hotspot_id <= HOTSPOT_ID)){
		if(ASD_HOTSPOT[hotspot_id] == NULL)
			ret = ASD_DBUS_HOTSPOTID_NOT_EXIST;
		else{
			free(ASD_HOTSPOT[hotspot_id]);
			ASD_HOTSPOT[hotspot_id] = NULL;
		}
	}
	else
		ret = -1;
	
	pthread_mutex_unlock(&asd_g_hotspot_mutex);
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
	
}
//end
//qiuchen add it 2012.10.24
DBusMessage *asd_dbus_sta_show_hotspot_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int i = 1;
	unsigned int num = 0;
	
	char *identifier = NULL;
	char *nas_port = NULL;
	
	pthread_mutex_lock(&asd_g_hotspot_mutex);
	for(i=1;i<HOTSPOT_ID+1;i++){
		if((ASD_HOTSPOT[i])&&(ASD_HOTSPOT[i]->nas_identifier != NULL)&&(ASD_HOTSPOT[i]->nas_port_id != NULL))
			num++;
	}
	
	dbus_error_init(&err);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 
		
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&num);

	dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										   		DBUS_TYPE_UINT32_AS_STRING
										   		DBUS_TYPE_STRING_AS_STRING
										   		DBUS_TYPE_STRING_AS_STRING
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
	for(i=1;i<(HOTSPOT_ID+1);i++){
		if(ASD_HOTSPOT[i]){
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			
			dbus_message_iter_append_basic (&iter_struct,
									DBUS_TYPE_UINT32,
									&i);
			
			//asd_printf(ASD_DBUS,MSG_DEBUG,"nas_identifier = %s\n",ASD_HOTSPOT[i]->nas_identifier);
			//asd_printf(ASD_DBUS,MSG_DEBUG,"nas_port_id = %s\n",ASD_HOTSPOT[i]->nas_port_id);
			
			int s = 0,q = 0;
			for(s=0;s<sizeof(ASD_HOTSPOT[i]->nas_identifier);s++){
				if(ASD_HOTSPOT[i]->nas_identifier[s]!='\0')
					q++;
				else
					break;
			}
			identifier = (char*)malloc(q+1);
			memset(identifier,0,(q+1));
			memcpy(identifier,ASD_HOTSPOT[i]->nas_identifier,q);

			for(s=0;s<sizeof(ASD_HOTSPOT[i]->nas_port_id);s++){
				if(ASD_HOTSPOT[i]->nas_port_id[s]!='\0')
					q++;
				else
					break;
			}
			nas_port = (char*)malloc(q+1);
			memset(nas_port,0,(q+1));
			memcpy(nas_port,ASD_HOTSPOT[i]->nas_port_id,q);
			
			
			dbus_message_iter_append_basic (&iter_struct,
									DBUS_TYPE_STRING,
									&identifier);
	
			dbus_message_iter_append_basic (&iter_struct,
									DBUS_TYPE_STRING,
									&nas_port);

			free(identifier);
			identifier = NULL;
			free(nas_port);
			nas_port = NULL;
			
			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container (&iter, &iter_array);

	pthread_mutex_unlock(&asd_g_hotspot_mutex);
	return reply;
}
//end
DBusMessage *asd_dbus_set_asd_hotspot_map_nas_information(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;

	unsigned int hotspot_id = 0 ; 
	char *nas_port_id = NULL; 
	char *nas_id  = NULL;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&hotspot_id,
								DBUS_TYPE_STRING,&nas_port_id,
								DBUS_TYPE_STRING,&nas_id,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"hotspot id = %d\n",hotspot_id);
	asd_printf(ASD_DBUS,MSG_DEBUG,"nas_port_id = %s\n",nas_port_id);
	asd_printf(ASD_DBUS,MSG_DEBUG,"nas_id = %s\n",nas_id);
	pthread_mutex_lock(&asd_g_hotspot_mutex);//qiuchen add it
	if((hotspot_id >= 1) &&( hotspot_id <= HOTSPOT_ID)){
		if(ASD_HOTSPOT[hotspot_id] == NULL)
			ASD_HOTSPOT[hotspot_id] = malloc(sizeof(struct sta_nas_info));
		if(ASD_HOTSPOT[hotspot_id] == NULL){
			printf("malloc error!\n");
			exit(1) ;
		}
		memset(ASD_HOTSPOT[hotspot_id],0,sizeof(struct sta_nas_info));
		memcpy(ASD_HOTSPOT[hotspot_id]->nas_port_id,nas_port_id,strlen(nas_port_id));
		memcpy(ASD_HOTSPOT[hotspot_id]->nas_identifier,nas_id,strlen(nas_id));
		ASD_HOTSPOT[hotspot_id]->nasid_len = strlen(nas_id);
		
		asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_HOTSPOT[%d]->nas_port_id = %s\n",hotspot_id,ASD_HOTSPOT[hotspot_id]->nas_port_id);
		asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_HOTSPOT[%d]->nas_id = %s\n",hotspot_id,ASD_HOTSPOT[hotspot_id]->nas_identifier);
	}
	else{
		ret = -1;
	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	pthread_mutex_unlock(&asd_g_hotspot_mutex);
	return reply;	
}
DBusMessage *asd_dbus_set_asd_sta_idle_time(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int time=0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&time,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(asd_sta_idle_time!= time){
		asd_sta_idle_time = time;
		ret = ASD_DBUS_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}
DBusMessage *asd_dbus_set_asd_sta_idle_time_switch(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type =0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(asd_sta_idle_time_switch!= type){
		asd_sta_idle_time_switch = type;
		ret = ASD_DBUS_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}
DBusMessage *asd_dbus_set_asd_ipset_switch(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned char type =0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE,&type,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(asd_ipset_switch!= type){
		asd_ipset_switch = type;
		if(asd_ipset_switch)
		{			
			ret = eap_auth_init();
			if(ret != ASD_DBUS_SUCCESS)
				asd_ipset_switch = 0;
		}	
		else
		{
			ret = eap_clean_all_user();	
			if(ret == 0 )
				ret = eap_auth_exit();
			if(ret != ASD_DBUS_SUCCESS)
				asd_ipset_switch = 1;			
		}
			
	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}

DBusMessage *asd_dbus_set_asd_bak_sta_update_time(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply; 	
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int time=0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&time,
								DBUS_TYPE_INVALID))){
	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(is_secondary != 1)
	{
		if(asd_bak_sta_update_time!= time){
			asd_bak_sta_update_time = time;		
			circle_cancel_timeout(bak_update_bss_req,NULL,NULL);
			circle_register_timeout(asd_bak_sta_update_time*60, 0, bak_update_bss_req, NULL, NULL);
			ret = ASD_DBUS_SUCCESS;
		}

	}
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret); 

	return reply;	
}
DBusMessage *asd_dbus_checking_asd(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s,line=%d .\n",__func__,__LINE__);
		
	return reply;	
}

/* book add for asd deamon quit, 2011-5-23 */
DBusMessage *asd_dbus_method_quit(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = ASD_DBUS_SUCCESS;
	//delete the ipset ip
	if(asd_ipset_switch)
	{
		eap_clean_all_user(); 
		eap_auth_exit();
		asd_ipset_switch = 0;
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	asd_printf(ASD_DBUS,MSG_DEBUG,"ret = %d   %s,line=%d .\n", ret, __func__, __LINE__);
	
    exit(2);
    
	return reply;	
}


DBusMessage *asd_dbus_show_static_sta_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;

	struct sta_static_info	*sta = NULL;
	char *showStr = NULL,*cursor = NULL;
	unsigned int totalLen = 0;
	unsigned int i = 0;

	asd_printf(ASD_DBUS,MSG_INFO,("asd_dbus_show_static_sta_running_config\n"));
	if(sta_static_num == 0){		
		showStr = (char*)malloc(1); 	
		memset(showStr,0,1);
	}else{
		showStr = (char*)malloc(sta_static_num*1024);
	
		if(NULL == showStr){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}else{
			memset(showStr,0,sta_static_num*1024);
			cursor = showStr;	

			for(i=0; i<STA_HASH_SIZE; i++){
				sta = STA_STATIC_TABLE[i];
				while(sta != NULL){
					if(sta->wlan_id != 0){
						if(sta->vlan_id != 0){
							totalLen += sprintf(cursor,"set static sta "MACSTR" vlanid %d by wlan %d\n",MAC2STR(sta->addr),sta->vlan_id,sta->wlan_id);
							cursor = showStr + totalLen;
						}
						if(sta->sta_traffic_limit != 0){
							totalLen += sprintf(cursor,"set static sta "MACSTR" limit %d by wlan %d\n",MAC2STR(sta->addr),sta->sta_traffic_limit,sta->wlan_id);
							cursor = showStr + totalLen;
						}
						if(sta->sta_send_traffic_limit != 0){
							totalLen += sprintf(cursor,"set static sta "MACSTR" send_limit %d by wlan %d\n",MAC2STR(sta->addr),sta->sta_send_traffic_limit,sta->wlan_id);
							cursor = showStr + totalLen;
						}
					}
					else{
						if(sta->vlan_id != 0){
							totalLen += sprintf(cursor,"set static sta "MACSTR" vlanid %d by wlan\n",MAC2STR(sta->addr),sta->vlan_id);
							cursor = showStr + totalLen;
						}
						if(sta->sta_traffic_limit != 0){
							totalLen += sprintf(cursor,"set static sta "MACSTR" limit %d by wlan\n",MAC2STR(sta->addr),sta->sta_traffic_limit);
							cursor = showStr + totalLen;
						}
						if(sta->sta_send_traffic_limit != 0){
							totalLen += sprintf(cursor,"set static sta "MACSTR" send_limit %d by wlan\n",MAC2STR(sta->addr),sta->sta_send_traffic_limit);
							cursor = showStr + totalLen;
						}
					}
					cursor = showStr + totalLen;
					sta = sta->hnext;
				}
			}
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;


}

DBusMessage *asd_dbus_show_ac_group_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned char num=0;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
	DBusError err;
	//int ret = WID_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i = 0;
	int j;
	unsigned char *ip;
	Inter_AC_R_Group *ACGROUP[GROUP_NUM];

	while(i<GROUP_NUM){
		if(AC_GROUP[i] != NULL)
		{
			ACGROUP[num] = AC_GROUP[i];
			num++;
		}
		i++;
	}
	if(num == 0){		
		showStr = (char*)malloc(1); 	
		memset(showStr,0,1);
	}else{
		showStr = (char*)malloc(num*1024);
	
		if(NULL == showStr){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}else{
			memset(showStr,0,num*1024);
			cursor = showStr;	
			
			for(i=0; i < num; i++)
			{				
				totalLen += sprintf(cursor,"create ac-mobility-group %d %s base %s\n",ACGROUP[i]->GroupID, ACGROUP[i]->name,ACGROUP[i]->ESSID);
				cursor = showStr + totalLen;
				totalLen += sprintf(cursor,"config ac-mobility-group %d\n",ACGROUP[i]->GroupID);
				cursor = showStr + totalLen;
				if(ACGROUP[i]->host_ip != 0){
					ip = (unsigned char *)&(ACGROUP[i]->host_ip);
					totalLen += sprintf(cursor," set host ip %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
					cursor = showStr + totalLen;					
				}
				for(j = 0; j < G_AC_NUM; j++){
					if(ACGROUP[i]->Mobility_AC[j] != NULL){
						ip = (unsigned char *)&(ACGROUP[i]->Mobility_AC[j]->ACIP);
						totalLen += sprintf(cursor," add ac %d %d.%d.%d.%d as member\n",j,ip[0],ip[1],ip[2],ip[3]);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor,"exit\n");
				cursor = showStr + totalLen;
			}
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;


}

DBusMessage *asd_dbus_security_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned char num=0;
	char *showStr = NULL,*cursor = NULL;
	char *showStr_new = NULL;
	int str_len = 0;
	int totalLen = 0;
	DBusError err;
	//int ret = ASD_DBUS_SUCCESS;
	dbus_error_init(&err);
	int i=0;
	security_profile *SECURITY[WLAN_NUM];
	char SecurityType[20];
	char EncryptionType[20];				
	unsigned int state;
	while(i<WLAN_NUM){
		if(ASD_SECURITY[i] != NULL)
		{
			SECURITY[num] = ASD_SECURITY[i];
			num++;
		}
		i++;
	}
	if(num == 0){
		showStr = (char*)malloc(1024);
		str_len = 1024;
		if(NULL == showStr){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}
		else{/*nl add 09/10/10*/
			memset(showStr,0,1024);
			cursor = showStr + totalLen;			
			/*if(WTP_SEND_RESPONSE_TO_MOBILE==1){
				totalLen += sprintf(cursor,"set wtp send response to mobile enable %d\n",WTP_SEND_RESPONSE_TO_MOBILE);
				cursor = showStr + totalLen;
			}*/
			if(ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER!=10){
				totalLen += sprintf(cursor,"set notice sta info to portal timer %d\n",ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER);
				cursor = showStr + totalLen;
			}	
			if(ASD_NOTICE_STA_INFO_TO_PORTAL==1){
				totalLen += sprintf(cursor,"set notice sta info to protal enable \n");
				cursor = showStr + totalLen;
			}

			if(asd_get_sta_info_able == 1){
				totalLen += sprintf(cursor,"asd get sta info enable\n");
				cursor = showStr + totalLen;
			}				
			if(asd_get_sta_info_time != 10){
				totalLen += sprintf(cursor,"set asd get sta info time %d\n",asd_get_sta_info_time);
				cursor = showStr + totalLen;
			}			
			if(asd_sta_arp_listen == 1){
				totalLen += sprintf(cursor,"set asd sta arp listen enable\n");
				cursor = showStr + totalLen;
			}			
			else if(asd_sta_arp_listen == 2){
				totalLen += sprintf(cursor,"set asd sta arp listen_and_set enable\n");
				cursor = showStr + totalLen;
			}			
			if(asd_sta_idle_time != 8){
				totalLen += sprintf(cursor,"set asd sta idle time interval %d\n",asd_sta_idle_time);
				cursor = showStr + totalLen;
			}
			if(asd_sta_idle_time_switch == 0){
				totalLen += sprintf(cursor,"set asd sta idle time switch disable\n");
				cursor = showStr + totalLen;
			}
			/*if(asd_ipset_switch == 1)
			{
				totalLen += sprintf(cursor,"set asd  ipset switch enable\n");
				cursor = showStr +totalLen;
			}*/
			if(asd_bak_sta_update_time != 360){
				totalLen += sprintf(cursor,"set asd bak sta update interval %d\n",asd_bak_sta_update_time);
				cursor = showStr + totalLen;
			}
			if(asd_sta_getip_from_dhcpsnoop == 1){
				totalLen +=sprintf(cursor,"set asd sta get ip from dhcpsnooping enable\n");
				cursor = showStr + totalLen;
			}
			if(asd_sta_static_arp == 1){
				totalLen += sprintf(cursor,"set asd static arp op enable\n");
				cursor = showStr + totalLen;
			}
			if(STA_STATIC_FDB_ABLE == 1){
				totalLen += sprintf(cursor,"set sta static fdb enable\n");
				cursor = showStr + totalLen;
			}
			/*if(ASD_SWITCH == 0){//wuwl del.share mem has been removed,so cannot get msg from wsm.
				totalLen += sprintf(cursor,"set asd switch disable\n");
				cursor = showStr + totalLen;
			}*/
			for(i = 1; i < WLAN_NUM; i++){
				if(ASD_ARP_GROUP[i] != NULL){
					totalLen += sprintf(cursor,"add asd sta gateway %s as arp group %d\n",ASD_ARP_GROUP[i],i);
					cursor = showStr + totalLen;
				}
			}		
			for( i = 0 ; i <= HOTSPOT_ID ; i++){
				if((totalLen + 300) > str_len)
				{
					str_len  = str_len + 300; 
					showStr_new = (char*)realloc(showStr,str_len);
					if(showStr_new == NULL)
					{
						asd_printf(ASD_DBUS,MSG_INFO,"show running realloc failed\n");
						break;
					}
					else
					{
						showStr = showStr_new;
						memset(showStr+str_len-300,0,300);
						showStr_new = NULL;
					}
				}
				cursor = showStr +totalLen;
				//pthread_mutex_lock(&asd_g_hotspot_mutex);
				if(ASD_HOTSPOT[i] != NULL){
					totalLen += sprintf(cursor,"set sta hotspotid %d map nas_port_id %s nas_identifier %s\n",i,ASD_HOTSPOT[i]->nas_port_id,ASD_HOTSPOT[i]->nas_identifier);
					cursor = showStr +totalLen;
				}
				//pthread_mutex_unlock(&asd_g_hotspot_mutex);
			}
			if(wpa_debug_level == MSG_MSGDUMP) {
				totalLen += sprintf(cursor,"set asd daemonlog level dump\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_DEBUG){
				totalLen += sprintf(cursor,"set asd daemonlog level debug\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_INFO){
				totalLen += sprintf(cursor,"set asd daemonlog level info\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_NOTICE){
				totalLen += sprintf(cursor,"set asd daemonlog level notice\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_WARNING){
				totalLen += sprintf(cursor,"set asd daemonlog level warning\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_ERROR){
				totalLen += sprintf(cursor,"set asd daemonlog level error\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_CRIT){
				totalLen += sprintf(cursor,"set asd daemonlog level crit\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_ALERT){
				totalLen += sprintf(cursor,"set asd daemonlog level alert\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_EMERG){
				totalLen += sprintf(cursor,"set asd daemonlog level emerg\n");
				cursor = showStr + totalLen;
			}
		}
		asd_printf(ASD_DBUS,MSG_DEBUG,"no security profile\n");
	}
	else{
		str_len = num*1024;
		showStr = (char*)malloc(str_len);
		if(NULL == showStr){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}
		else{			
			memset(showStr,0,str_len);
			cursor = showStr + totalLen;

			if(WTP_SEND_RESPONSE_TO_MOBILE==1){
				totalLen += sprintf(cursor,"set wtp send response to mobile enable %d\n",WTP_SEND_RESPONSE_TO_MOBILE);
				cursor = showStr + totalLen;
			}				
			if(ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER!=10){
				totalLen += sprintf(cursor,"set notice sta info to portal timer %d\n",ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER);
				cursor = showStr + totalLen;
			}				
			if(ASD_NOTICE_STA_INFO_TO_PORTAL==1){
				totalLen += sprintf(cursor,"set notice sta info to portal enable\n");
				cursor = showStr + totalLen;
			}
			if(asd_get_sta_info_able == 1){
				totalLen += sprintf(cursor,"asd get sta info enable\n");
				cursor = showStr + totalLen;
			}				
			if(asd_get_sta_info_time != 10){
				totalLen += sprintf(cursor,"set asd get sta info time %d\n",asd_get_sta_info_time);
				cursor = showStr + totalLen;
			}				
			
			if(asd_sta_arp_listen == 1){
				totalLen += sprintf(cursor,"set asd sta arp listen enable\n");
				cursor = showStr + totalLen;
			}			
			else if(asd_sta_arp_listen == 2){
				totalLen += sprintf(cursor,"set asd sta arp listen_and_set enable\n");
				cursor = showStr + totalLen;
			}			
			if(asd_sta_idle_time != 8){
				totalLen += sprintf(cursor,"set asd sta idle time interval %d\n",asd_sta_idle_time);
				cursor = showStr + totalLen;
			}		
			if(asd_sta_idle_time_switch == 0){
				totalLen += sprintf(cursor,"set asd sta idle time switch disable\n");
				cursor = showStr + totalLen;
			}			
			/*if(asd_ipset_switch == 1)
			{
				totalLen += sprintf(cursor,"set asd  ipset switch enable\n");
				cursor = showStr +totalLen;
			}*/
			if(asd_bak_sta_update_time != 360){
				totalLen += sprintf(cursor,"set asd bak sta update interval %d\n",asd_bak_sta_update_time);
				cursor = showStr + totalLen;
			}
			if(asd_sta_getip_from_dhcpsnoop == 1){
				totalLen +=sprintf(cursor,"set asd sta get ip from dhcpsnooping enable\n");
				cursor = showStr + totalLen;
			}
			if(asd_sta_static_arp == 1){
				totalLen += sprintf(cursor,"set asd static arp op enable\n");
				cursor = showStr + totalLen;
			}
			if(STA_STATIC_FDB_ABLE == 1){
				totalLen += sprintf(cursor,"set sta static fdb enable\n");
				cursor = showStr + totalLen;
			}
			/*if(ASD_SWITCH == 0){//wuwl del.share mem has been removed,so cannot get msg from wsm.
				totalLen += sprintf(cursor,"set asd switch disable\n");
				cursor = showStr + totalLen;
			}*/
			for(i = 1; i < WLAN_NUM; i++){
				if(ASD_ARP_GROUP[i] != NULL){
					totalLen += sprintf(cursor,"add asd sta gateway %s as arp group %d\n",ASD_ARP_GROUP[i],i);
					cursor = showStr + totalLen;
				}
			}
			if(dbus_count_switch == 1){
				totalLen += sprintf(cursor,"set asd dbus count enable\n");
				cursor = showStr + totalLen;
			}			
			for( i = 1 ; i <= HOTSPOT_ID ; i++){
				
				if((totalLen + 300) > str_len)
				{
					str_len  = str_len + 300; 
					showStr_new = (char*)realloc(showStr,str_len);
					if(showStr_new == NULL)
					{
						asd_printf(ASD_DBUS,MSG_INFO,"show running realloc failed\n");
						break;
					}
					else
					{
						showStr = showStr_new;
						memset(showStr+str_len-300,0,300);
						showStr_new = NULL;
					}
				}
				cursor = showStr + totalLen;			
				//pthread_mutex_lock(&asd_g_hotspot_mutex);
				if(ASD_HOTSPOT[i] != NULL){
					totalLen += sprintf(cursor,"set sta hotspotid %d map nas_port_id %s nas_identifier %s\n",i,ASD_HOTSPOT[i]->nas_port_id,ASD_HOTSPOT[i]->nas_identifier);
					cursor = showStr +totalLen;
				}
				//pthread_mutex_unlock(&asd_g_hotspot_mutex);
			}
			if(wpa_debug_level == MSG_MSGDUMP) {
				totalLen += sprintf(cursor,"set asd daemonlog level dump\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_DEBUG){
				totalLen += sprintf(cursor,"set asd daemonlog level debug\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_INFO){
				totalLen += sprintf(cursor,"set asd daemonlog level info\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_NOTICE){
				totalLen += sprintf(cursor,"set asd daemonlog level notice\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_WARNING){
				totalLen += sprintf(cursor,"set asd daemonlog level warning\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_ERROR){
				totalLen += sprintf(cursor,"set asd daemonlog level error\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_CRIT){
				totalLen += sprintf(cursor,"set asd daemonlog level crit\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_ALERT){
				totalLen += sprintf(cursor,"set asd daemonlog level alert\n");
				cursor = showStr + totalLen;
			}
			else if(wpa_debug_level == MSG_EMERG){
				totalLen += sprintf(cursor,"set asd daemonlog level emerg\n");
				cursor = showStr + totalLen;
			}
			for(i=0; i<num; i++){
				if(totalLen + 1024 > str_len) {
					str_len *= 2;
					showStr_new = (char*)realloc(showStr,str_len);
					if(showStr_new == NULL){
						asd_printf(ASD_DBUS,MSG_INFO,"show running realloc failed\n");
						break;
					}else {
						showStr = showStr_new;
						memset(showStr+str_len/2,0,str_len/2);
						showStr_new = NULL;
					}
					asd_printf(ASD_DBUS,MSG_DEBUG,"show running totalLen %d realloc strlen %d\n",totalLen,str_len);
				}
					
				memset(SecurityType, 0, 20);
				memset(EncryptionType, 0, 20);
				CheckSecurityType(SecurityType, SECURITY[i]->securityType);
				CheckEncryptionType(EncryptionType, SECURITY[i]->encryptionType);
				state = SECURITY[i]->extensible_auth;
				cursor = showStr + totalLen;			
				totalLen += sprintf(cursor,"create security %d %s\n",SECURITY[i]->SecurityID,SECURITY[i]->name);
				cursor = showStr + totalLen;		
				totalLen += sprintf(cursor,"config security %d\n",SECURITY[i]->SecurityID);
				cursor = showStr + totalLen;			
				totalLen += sprintf(cursor," security type %s\n",SecurityType);
				cursor = showStr + totalLen;		
				totalLen += sprintf(cursor," encryption type %s\n",EncryptionType);
				cursor = showStr + totalLen;
				// xm add 09/01/19
				if(SECURITY[i]->eap_reauth_priod!= 3600){
					totalLen += sprintf(cursor," set eap reauth period %d\n",SECURITY[i]->eap_reauth_priod);
					cursor = showStr + totalLen;
				}
				if(SECURITY[i]->wpa_group_rekey != 600){
					totalLen += sprintf(cursor," set wpa group rekey period %d\n",SECURITY[i]->wpa_group_rekey);
					cursor = showStr + totalLen;
				}
				if(SECURITY[i]->wpa_keyupdate_timeout != 1000){
					totalLen += sprintf(cursor," set wpa keyupdate timeout period %d\n",SECURITY[i]->wpa_keyupdate_timeout);
					cursor = showStr + totalLen;
				}
				//mahz add 2011.1.3
				if(SECURITY[i]->wpa_once_group_rekey_time != 6){
					totalLen += sprintf(cursor," set wpa once group rekey time %d\n",SECURITY[i]->wpa_once_group_rekey_time);
					cursor = showStr + totalLen;
				}
				//
				if(SECURITY[i]->index!= 1){
					totalLen += sprintf(cursor," set security index %d\n",SECURITY[i]->index);
					cursor = showStr + totalLen;
				}
				
				//ht add,090206
				if(SECURITY[i]->acct_interim_interval!= 0){
					totalLen += sprintf(cursor," set acct interim interval %d\n",SECURITY[i]->acct_interim_interval);
					cursor = showStr + totalLen;
				}
				if(state == 1){
					totalLen += sprintf(cursor," extensible authentication enable\n");
					cursor = showStr + totalLen;
				}
				//weichao add 
				if(SECURITY[i]->ap_max_inactivity != 300){
					totalLen += sprintf(cursor," set ap max detect interval %d \n",SECURITY[i]->ap_max_inactivity);
					cursor = showStr + totalLen;
				}
				//mahz add 2011.3.15
				if(SECURITY[i]->hybrid_auth == 1){
					totalLen += sprintf(cursor," hybrid auth enable\n");
					cursor = showStr + totalLen;
				}//
				if(SECURITY[i]->mac_auth ==1 ){
					totalLen += sprintf(cursor," mac auth enable\n");
					cursor = showStr + totalLen;
				}
				if(SECURITY[i]->SecurityKey != NULL){
					totalLen += sprintf(cursor," security %s key %s\n",(SECURITY[i]->keyInputType==1)?"ascii":"hex",SECURITY[i]->SecurityKey);
					cursor = showStr + totalLen;
				}
				if(strncmp(SECURITY[i]->host_ip,"127.0.0.1",9)){
					totalLen += sprintf(cursor," security host_ip %s\n",SECURITY[i]->host_ip);
					cursor = showStr + totalLen;
				}
				if((SECURITY[i]->securityType == IEEE8021X)
					||(SECURITY[i]->securityType == WPA_E)
					||(SECURITY[i]->securityType == WPA2_E)
					||(SECURITY[i]->securityType == MD5)
					||(state == 1)||((SECURITY[i]->securityType == WAPI_AUTH)&&(SECURITY[i]->wapi_radius_auth == 1))){	//mahz add 2010.12.10
					
					//weichao add
					if(SECURITY[i]->eap_alive_period!= 3600){
						totalLen += sprintf(cursor," set eap alive period %d\n",SECURITY[i]->eap_alive_period);
						cursor = showStr + totalLen;
					}
					//weichao add
					if(SECURITY[i]->account_after_authorize== 0){
						
						if(SECURITY[i]->account_after_dhcp== 1){
							totalLen += sprintf(cursor," set  account start after dhcp enable\n");
							cursor = showStr + totalLen;
						}
					}
					else
					{
						totalLen += sprintf(cursor," set account start after authorized enable \n");
						cursor = showStr + totalLen;
					
					}
					if(SECURITY[i]->traffic_limite_radius == 1)
					{
						totalLen += sprintf(cursor," traffic limit from radius first \n");
						cursor = showStr + totalLen;
					}
					//ht add,090727
					if(SECURITY[i]->quiet_Period != 60){
						totalLen += sprintf(cursor,"security quiet period %d\n",SECURITY[i]->quiet_Period);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->wired_radius == 1){
						totalLen += sprintf(cursor," connect with wired radius server \n");
						cursor = showStr + totalLen;
					}

					//mahz add 
					if(SECURITY[i]->wapi_radius_auth == 1){
						totalLen += sprintf(cursor," wapi radius auth enable\n");
						cursor = showStr + totalLen;
					}
					//mahz add 2010.12.9
					if((SECURITY[i]->wapi_radius_auth == 1)&&(SECURITY[i]->user_passwd != NULL)){
						totalLen += sprintf(cursor," wapi radius auth set passwd %s\n",SECURITY[i]->user_passwd);
						cursor = showStr + totalLen;
					}
					//
					if(SECURITY[i]->auth.auth_ip != NULL){
						totalLen += sprintf(cursor," radius auth %s %d %s \n",SECURITY[i]->auth.auth_ip,SECURITY[i]->auth.auth_port,SECURITY[i]->auth.auth_shared_secret);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->acct.acct_ip != NULL){
						totalLen += sprintf(cursor," radius acct %s %d %s \n",SECURITY[i]->acct.acct_ip,SECURITY[i]->acct.acct_port,SECURITY[i]->acct.acct_shared_secret);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->auth.secondary_auth_ip != NULL){
						totalLen += sprintf(cursor," secondary radius auth %s %d %s \n",SECURITY[i]->auth.secondary_auth_ip,SECURITY[i]->auth.secondary_auth_port,SECURITY[i]->auth.secondary_auth_shared_secret);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->acct.secondary_acct_ip != NULL){
						totalLen += sprintf(cursor," secondary radius acct %s %d %s \n",SECURITY[i]->acct.secondary_acct_ip,SECURITY[i]->acct.secondary_acct_port,SECURITY[i]->acct.secondary_acct_shared_secret);
						cursor = showStr + totalLen;// xm add 09/01/19
					}
					if(SECURITY[i]->accounting_on_disable != 0){
						totalLen += sprintf(cursor," accounting on disable\n");
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->radius_extend_attr != 0){
						totalLen += sprintf(cursor," radius extend attribute enable\n");
						cursor = showStr + totalLen;	/*ht add,091027*/ 
					}
					if(SECURITY[i]->distribute_off == 1)
					{
						totalLen += sprintf(cursor," set asd distribute disable\n");
						cursor = showStr + totalLen;
					}
					else
					{
						if((SECURITY[i]->slot_value != 0)&& (SECURITY[i]->inst_value != 0)){
							totalLen += sprintf(cursor," set asd rdc slotid %d instid %d\n",SECURITY[i]->slot_value,SECURITY[i]->inst_value);
							cursor = showStr + totalLen;	
						}
					}
					//Qiuchen add it for saving radius configuration!
					if(SECURITY[i]->heart_test_on != 0)
					{	
						if(SECURITY[i]->ac_radius_name != NULL)
						{
							totalLen += sprintf(cursor," set ac_radius_name %s\n",SECURITY[i]->ac_radius_name);
							cursor = showStr + totalLen;
						}
						if(SECURITY[i]->radius_res_fail_percent != RADIUS_RES_FAIL_PERCENT)
						{
							totalLen += sprintf(cursor," set radius_res_fail percent %f\n",SECURITY[i]->radius_res_fail_percent);
							cursor = showStr + totalLen;

						}
						if(SECURITY[i]->radius_res_suc_percent != RADIUS_RES_SUC_PERCENT)
						{
							totalLen += sprintf(cursor," set radius_res_suc percent %f\n",SECURITY[i]->radius_res_suc_percent);
							cursor = showStr + totalLen;
						
						}
						if(SECURITY[i]->radius_access_test_interval != RADIUS_ACCESS_TEST_INTERVAL)
						{
							totalLen += sprintf(cursor," set radius_server access_test_interval %d\n",SECURITY[i]->radius_access_test_interval);
							cursor = showStr + totalLen;
						
						}
						if(SECURITY[i]->radius_server_change_test_timer != RADIUS_SERVER_CHANGE_TIMER)
						{
							totalLen += sprintf(cursor," set radius_server change_test timer %d\n",SECURITY[i]->radius_server_change_test_timer);
							cursor = showStr + totalLen;
						
						}
						if(SECURITY[i]->radius_server_reuse_test_timer != RADIUS_SERVER_REUSE_TIMER)
						{
							totalLen += sprintf(cursor," set radius_server reuse_test timer %d\n",SECURITY[i]->radius_server_reuse_test_timer);
							cursor = showStr + totalLen;
						
						}
						if(SECURITY[i]->radius_server_binding_type != RADIUS_SERVER_UNBINDED)
						{
							totalLen += sprintf(cursor," set radius_server binding enable\n");
							cursor = showStr + totalLen;
						
						}
						if(SECURITY[i]->radius_heart_test_type != RADIUS_AUTH_TEST)
						{
							totalLen += sprintf(cursor," set radius_server heart_test_type %s\n",(SECURITY[i]->radius_heart_test_type == RADIUS_ACCT_TEST)?"acct":"both");
							cursor = showStr + totalLen;
						
						}
						totalLen += sprintf(cursor," set radius heart test on\n");
						cursor = showStr + totalLen;
					}
				}	
				
				if(SECURITY[i]->securityType == WAPI_AUTH){
					if((SECURITY[i]->wapi_as.multi_cert == 1)){
						totalLen += sprintf(cursor," wapi multi cert enable\n");
						cursor = showStr + totalLen;
					}
					if((SECURITY[i]->wapi_as.as_ip!=NULL)
						&&(SECURITY[i]->wapi_as.certification_type == WAPI_X509 ||SECURITY[i]->wapi_as.certification_type == WAPI_GBW)
						){
						totalLen += sprintf(cursor," wapi as %s certification type %s\n",SECURITY[i]->wapi_as.as_ip,((SECURITY[i]->wapi_as.certification_type == WAPI_X509)?"X.509":"GBW"));
						cursor = showStr + totalLen;
					}
					if((SECURITY[i]->wapi_as.certification_path!=NULL)
						){
						totalLen += sprintf(cursor," wapi as certification path %s\n",SECURITY[i]->wapi_as.certification_path);
						cursor = showStr + totalLen;
					}
					if((SECURITY[i]->wapi_as.ae_cert_path!=NULL)
						){
						totalLen += sprintf(cursor," wapi ae certification path %s\n",SECURITY[i]->wapi_as.ae_cert_path);
						cursor = showStr + totalLen;
					}
					if((SECURITY[i]->wapi_as.multi_cert == 1) && (SECURITY[i]->wapi_as.ca_cert_path!=NULL)
						){
						totalLen += sprintf(cursor," wapi ca certification path %s\n",SECURITY[i]->wapi_as.ca_cert_path);
						cursor = showStr + totalLen;
					}
				}

				if(SECURITY[i]->securityType == WAPI_AUTH||SECURITY[i]->securityType == WAPI_PSK){
					if(SECURITY[i]->wapi_ucast_rekey_method!= 1){	//	xm0701
						char method_str[20];
						memset(method_str,0,20);
						GetRekeyMethod(method_str,SECURITY[i]->wapi_ucast_rekey_method);
						totalLen += sprintf(cursor," set wapi unicast rekey method %s\n",method_str);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->wapi_mcast_rekey_method!= 1){	//	xm0701
						char method_str1[20];
						memset(method_str1,0,20);
						GetRekeyMethod(method_str1,SECURITY[i]->wapi_mcast_rekey_method);
						totalLen += sprintf(cursor," set wapi multicast rekey method %s\n",method_str1);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->wapi_ucast_rekey_para_t!= 86400){	//	xm0701
						totalLen += sprintf(cursor," set wapi unicast rekey time parameter  %u\n",SECURITY[i]->wapi_ucast_rekey_para_t);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->wapi_ucast_rekey_para_p!= 100000){	//	xm0701
						totalLen += sprintf(cursor," set wapi unicast rekey packet parameter  %u\n",SECURITY[i]->wapi_ucast_rekey_para_p);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->wapi_mcast_rekey_para_t!= 86400){	//	xm0701
						totalLen += sprintf(cursor," set wapi multicast rekey time parameter  %u\n",SECURITY[i]->wapi_mcast_rekey_para_t);
						cursor = showStr + totalLen;
					}
					if(SECURITY[i]->wapi_mcast_rekey_para_p!= 100000){	//	xm0701
						totalLen += sprintf(cursor," set wapi multicast rekey packet parameter  %u\n",SECURITY[i]->wapi_mcast_rekey_para_p);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor,"exit \n");
				cursor = showStr + totalLen;
			}
			
		}		
			if(showStr==NULL){
			showStr = (char*)os_zalloc(1);		
			memset(showStr,0,1);		
			}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;

}

int check_dbus_uname(char * sender){
	int i = 0;
	int j = 0; 
	for(i = 0; i < DBUS_COUNT_NUM; i++){
		for(j = 0; j < dbus_count[i].num; j++){
			if(dbus_count[i].uname != NULL){
				printf("dbus_count[i].uname[j] %s\n",&(dbus_count[i].uname[j*SENDER_LEN]));
				printf("sender %s\n",sender);
				if(strcmp(&(dbus_count[i].uname[j*SENDER_LEN]),sender) == 0){
					dbus_count[i].count++;
					return 1;
				}
			}
		}
	}
	return 0;
}
int get_dbus_uname(int index){
	
	DBusMessage *reply;
	DBusMessage *method;
	DBusError error;
	char **list;
	int len, i;
	const char *name;

	reply = NULL;
	method = NULL;
	list = NULL;

	dbus_error_init (&error);

	method = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
									 DBUS_PATH_DBUS,
									 DBUS_INTERFACE_DBUS,
									 "ListQueuedOwners");

	if (method == NULL)
	goto out;
	name = dbus_count[index].rname;
	if (!dbus_message_append_args (method,
							 DBUS_TYPE_STRING, &(name),
							 DBUS_TYPE_INVALID))
	{
	fprintf (stderr, "Error appending args\n") ;
	goto out;
	}

	reply = dbus_connection_send_with_reply_and_block (asd_dbus_connection3,
												 method,
												 -1,
												 &error);

	if (reply == NULL)
	{
	fprintf (stderr, "Error calling ListQueuedOwners: %s\n", error.message);
	dbus_error_free (&error);
	goto out;
	}



	if (!dbus_message_get_args (reply,
						  &error,
						  DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
						  &list, &len,
						  DBUS_TYPE_INVALID))
	{
	fprintf (stderr, "Error getting args: %s\n", error.message);
	dbus_error_free (&error);
	goto out;
	}
	if(len > 0){
		if(dbus_count[index].uname != NULL){
			free(dbus_count[index].uname);
			dbus_count[index].uname = NULL;
			dbus_count[index].num = 0;
		}
		dbus_count[index].uname = malloc(len*SENDER_LEN);
		memset(dbus_count[index].uname, 0, len*SENDER_LEN);
		dbus_count[index].num = len;
	}

	for (i = 0; i < len; i++)
	{
		strcpy(&(dbus_count[index].uname[i*SENDER_LEN]),list[i]);
		printf ("%s\n", list[i]);
	}
	dbus_message_unref (method);
	dbus_message_unref (reply);
	dbus_free_string_array (list);
	return TRUE;
	out:
	if (method != NULL)
	dbus_message_unref (method);
	if (reply != NULL)
	dbus_message_unref (reply);
	if (list != NULL)
	dbus_free_string_array (list);
	return FALSE;
}
int asd_static_dbus_op(char * sender){
	int i;
	if(!check_dbus_uname(sender)){
		for(i = 0; i < DBUS_COUNT_NUM; i++){
			get_dbus_uname(i);		
		}		
		if(!check_dbus_uname(sender))
			other_dbus_count++;
	}
	return 0;
}

static DBusHandlerResult asd_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data){

	DBusMessage		*reply = NULL;
	char sender[20];
	asd_printf(ASD_DBUS,MSG_DEBUG,"message path %s\n",dbus_message_get_path(message));
	asd_printf(ASD_DBUS,MSG_DEBUG,"message interface %s\n",dbus_message_get_interface(message));
	asd_printf(ASD_DBUS,MSG_DEBUG,"message member %s\n",dbus_message_get_member(message));
	asd_printf(ASD_DBUS,MSG_DEBUG,"message destination %s\n",dbus_message_get_destination(message));	
	asd_printf(ASD_DBUS,MSG_DEBUG,"message type %d\n",dbus_message_get_type(message));

	if	(strcmp(dbus_message_get_path(message),ASD_DBUS_STA_OBJPATH) == 0)	{
//		asd_printf(ASD_DBUS,MSG_DEBUG,"path:%s\n",ASD_DBUS_STA_OBJPATH);
		if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC)) {
			reply = asd_dbus_show_sta_by_mac(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOWSTA_V2)) {
			reply = asd_dbus_show_sta_v2(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA)) {
			reply = asd_dbus_extend_show_sta(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID)) {
			reply = asd_dbus_show_wapi_info_bywtpid(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID)) {
			reply = asd_dbus_show_radio_info_bywtpid(connection,message,user_data);
		}
																	   
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_MIB_INFO_BYRDID)) {
			reply = asd_dbus_show_mib_info_byrdid(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO)) {
			reply = asd_dbus_show_traffic_limit_by_radio(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL)) {
			reply = asd_dbus_cancel_sta_traffic_limit(connection,message,user_data);
		}/*xm0723*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT)) {
			reply = asd_dbus_set_sta_traffic_limit(connection,message,user_data);
		}/*xm0723*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL)) {
			reply = asd_dbus_cancel_sta_send_traffic_limit(connection,message,user_data);
		}/*ht 090902*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT)) {
			reply = asd_dbus_set_sta_send_traffic_limit(connection,message,user_data);
		}/*ht 090902*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX)) {
			reply = asd_dbus_show_traffic_limit_by_bssindex(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_BYRDID)) {
			reply = asd_dbus_show_wapi_mib_info_byrdid(connection,message,user_data);
		}	//	xm0623
		//mahz add 2011.1.19 
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_wapi_mib_info_of_all_wtp(connection,message,user_data);
		}
		//
		//mahz add 2011.1.24
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STA_WAPI_MIB_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_sta_wapi_mib_info_of_all_wtp(connection,message,user_data);
		}
		//
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID)) {
			reply = asd_dbus_show_info_bywlanid(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_ALLWLAN)) {
			reply = asd_dbus_show_info_allwlan(connection,message,user_data);
		}    //fengwenchao  add 20101221
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID)) {
			reply = asd_dbus_show_info_bywtpid(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME)) {
			reply = asd_dbus_show_channel_access_time(connection,message,user_data);
		}
		/*------------------------------------------------for mib optimize  begini--------------------------------------------------------*/
		/*nl add 20100425*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BASIC_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_collect_info_of_all_wtp(connection,message,user_data);
		}
		/*nl add 20100508*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_stats_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_BY_WLAN_OF_ALL_WTP)) {
			reply = asd_dbus_show_wlan_stats_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_SSID_STATS_INFO_BY_WLAN_OF_ALL_WTP)) {
			reply = asd_dbus_show_wlan_ssid_stats_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALLWLAN_STA_NUM)){
			reply = asd_dbus_show_all_wlan_sta_num(connection,message,user_data);  //fengwenchao add 20101224
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_terminal_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STA_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_sta_info_of_all_wtp(connection,message,user_data);
		}
		//mahz add 2011.1.17
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_DISTINGUISH_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_distinguish_info_of_all_wtp(connection,message,user_data);
		}
		//mahz add 2011.11.9 for GuangZhou Mobile
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STA_STATIS_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_sta_statis_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_AC_STA_INFO_OF_ALL)) {
			reply = asd_dbus_show_ac_sta_info_of_all(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION)) {
			reply = asd_dbus_show_ssid_config_information_of_all_wlan(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_USER_LINK_INFO_OF_ALL_WTP)) {
			reply = asd_dbus_show_user_link_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_SECURITY_INFO_BYWTPID)) {
			reply = asd_dbus_show_security_config_info_by_wtp_of_all_wtp(connection,message,user_data);/*b21*/
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_INFORMATION)) {
			reply = asd_dbus_show_conjunction_info_of_all_wtp(connection,message,user_data);/*b22*/
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_NEW_WIRELESS_INFO_BYWTPID)) {
			reply = asd_dbus_show_radio_new_wireless_info_bywtpid(connection,message,user_data);/*b19*/
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_WIRELESS_INFO_BYWTPID)) {
			reply = asd_dbus_show_radio_wireless_info_bywtpid(connection,message,user_data);
		}

		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_BASIC_INFO_BY_WLAN_OF_ALL_WTP)) {
			reply = asd_dbus_show_wlan_wapi_basic_info_of_all_wtp(connection,message,user_data);/*bb1*/
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_EXTEND_CONFIG_INFO_BY_WLAN_OF_ALL_WTP)) {
			reply = asd_dbus_show_wlan_wapi_extend_config_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_UNICAST_INFO_BY_WLAN_OF_ALL_WTP)) {
			reply = asd_dbus_show_wlan_unicast_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_STATS_INFO_OF_ALL_WLAN)) {
			reply = asd_dbus_show_wlan_wapi_stats_performance_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_BSS_INFO_OF_ALL_WLAN)) {
			reply = asd_dbus_show_wlan_wapi_bss_performance_info_of_all_wtp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BSS_STA_NUM_BY_WLANID_AND_RADIOID)) {
			reply = asd_dbus_show_bss_sta_num_by_wlanid_and_radioid(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE)) {
			reply = asd_dbus_show_statistics_information_of_all_wtp_whole(connection,message,user_data);  //fengwenchao add 20110331
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME)) {
			reply = asd_dbus_set_asd_sta_idle_time(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME_SWITCH)) {
			reply = asd_dbus_set_asd_sta_idle_time_switch(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_IPSET_SWITCH)) {
			reply = asd_dbus_set_asd_ipset_switch(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_BAK_STA_UPDATE_TIME)) {
			reply = asd_dbus_set_asd_bak_sta_update_time(connection,message,user_data);
		}
		
		/*------------------------------------------------for mib optimize  end---------------------------------------------------------*/

		//mahz add 2011.6.3
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_RADIUS_FORCE_STA_DOWNLINE)) {
			reply = asd_dbus_radius_force_sta_downline(connection,message,user_data);
		}//		
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_KICKSTA)) {
			reply = asd_dbus_kick_sta(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_SET_STA_MAC_VLANID)) {
			reply = asd_dbus_set_sta_vlanid(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_CHECK_STA_BYMAC)) {
			reply = asd_dbus_check_sta_bymac(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_SET_STA_STATIC_INFO)) {
			reply = asd_dbus_set_sta_static_info(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_DEL_STA_STATIC_INFO)) {
			reply = asd_dbus_del_sta_static_info(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_SHOW_STA_STATIC_INFO_BYMAC)) {
			reply = asd_dbus_show_sta_static_info_bymac(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_SHOW_STA_STATIC_INFO)) {
			reply = asd_dbus_show_sta_static_info(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STATIC_STA_RUNNING_CONFIG)) {
			reply = asd_dbus_show_static_sta_running_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STALIST_NEW)) {
			reply = asd_dbus_show_stalist(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STALIST)) {
			reply = asd_dbus_show_stalist_by_group(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_ROAMING_STALIST)){
			reply = asd_dbus_show_roaming_sta_list(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_BSS_METHOD_SHOW_BSS_SUMMARY)){
			reply = asd_dbus_show_bss_summary(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_BSS_METHOD_SHOW_BSS_BSSINDEX)){
			reply = asd_dbus_show_bss_bssindex(connection,message,user_data);		
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_BASE_INFO)) {
			reply = asd_dbus_show_sta_base_info(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_SUMMARY)) {
			reply = asd_dbus_show_sta_summary(connection,message,user_data);
		}	
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_AC_FLOW	)) {
			reply = asd_dbus_set_ac_flow(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE	)) {
			reply = asd_dbus_set_extern_balance(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_ADD_MAC_LIST)) {
			reply = asd_dbus_wlan_add_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST)) {
			reply = asd_dbus_wtp_add_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST)) {
			reply = asd_dbus_bss_add_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST)) {
			reply = asd_dbus_wlan_del_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST)) {
			reply = asd_dbus_wtp_del_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST)) {
			reply = asd_dbus_bss_del_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST)) {
			reply = asd_dbus_wlan_use_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST)) {
			reply = asd_dbus_wtp_use_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST)) {
			reply = asd_dbus_bss_use_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST)) {
			reply = asd_dbus_show_wlan_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST)) {
			reply = asd_dbus_show_wtp_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST)) {
			reply = asd_dbus_show_bss_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST)) {
			reply = asd_dbus_show_all_wlan_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST)) {
			reply = asd_dbus_show_all_wtp_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST)) {
			reply = asd_dbus_show_all_bss_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST)) {
			reply = asd_dbus_show_wlan_wids_mac_list(connection,message,user_data);
		}
			/*for ac mac list begin*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_AC_ADD_MAC_LIST)) {
			reply = asd_dbus_ac_add_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_AC_DEL_MAC_LIST)) {
			reply = asd_dbus_ac_del_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_AC_USE_MAC_LIST)) {
			reply = asd_dbus_ac_use_mac_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_AC_MAC_LIST)) {
			reply = asd_dbus_show_ac_mac_list(connection,message,user_data);
		}
		/*for ac mac list end*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG)) {
			reply = asd_dbus_wlan_list_running_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG)) {
			reply = asd_dbus_wtp_list_running_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG)) {
			reply = asd_dbus_bss_list_running_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WLAN_STALIST)) {
			reply = asd_dbus_wlan_stalist(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_ALL_WLAN_STALIST)) {
			reply = asd_dbus_all_of_wlan_stalist(connection,message,user_data);  //fengwenchao add 20110113
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_WTP_STALIST)) {
			reply = asd_dbus_wtp_stalist(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST)) {
			reply = asd_dbus_extend_wtp_stalist(connection,message,user_data);
		}

		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_ADD_DEL_STA)) {
			reply = asd_dbus_add_del_sta(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO)) {
			reply = asd_dbus_get_sta_info(connection,message,user_data);//////////////
		}  //xm add 08/12/08
		/*fengwenchao add 20120323*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO_NEW)) {
			reply = asd_dbus_get_sta_info_new(connection,message,user_data);
		}
		/*fengwenchao add end*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN)) {
			reply = asd_dbus_set_asd_sta_arp_listen(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_IP_FROM_DHCPSNOOP)) {
			reply = asd_dbus_set_asd_sta_get_ip_from_dhcpsnoop(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_STA_ARP)) {
			reply = asd_dbus_set_sta_arp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP)) {
			reply = asd_dbus_set_asd_sta_static_arp(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP_IF_GROUP)) {
			reply = asd_dbus_set_asd_sta_static_arp_if_group(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SET_ASD_SWITCH)) {
			reply = asd_dbus_set_asd_switch(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_CLEAN_WLAN_STA)) {
			reply = asd_dbus_clean_wlan_sta(connection,message,user_data);
		
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_STA_HOTSPOT_MAP_NAS)) {
			reply = asd_dbus_set_asd_hotspot_map_nas_information(connection,message,user_data);
		}
		//qiuchen add it
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_DEL_HOTSPOT)){
			reply = asd_dbus_sta_del_hotspotid(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_SHOW_HOTSPOT_LIST)){
			reply =asd_dbus_sta_show_hotspot_list(connection,message,user_data);
		}
		//end
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_CONF_METHOD_CHECKING)){
			reply = asd_dbus_checking_asd(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_STA_INTERFACE,ASD_DBUS_CONF_METHOD_QUIT)){
			reply = asd_dbus_method_quit(connection,message,user_data);
		}
	}
	else if (strcmp(dbus_message_get_path(message),ASD_DBUS_SECURITY_OBJPATH) == 0){
		
//		asd_printf(ASD_DBUS,MSG_DEBUG,"path:%s\n",ASD_DBUS_SECURITY_OBJPATH);
		
		if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST);

			reply = asd_dbus_show_security_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY);

			reply = asd_dbus_show_security(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS)) {
			reply = asd_dbus_show_radius(connection,message,user_data);
		}
		//mahz add 2011.1.12
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_ALL)) {
			reply = asd_dbus_show_radius_all(connection,message,user_data);
		}	
		//
		/*nl add for wapi 09/11/03*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO)) {
			reply = asd_dbus_show_security_wapi_info(connection,message,user_data);
		}
		/*nl add for wapi 09/12/14*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_WLAN_SECURITY_WAPI_CONF)) {
			reply = asd_dbus_show_wlan_security_wapi_conf(connection,message,user_data);
		}
		//mahz add for mib request , 2011.1.27 , dot11AKMConfigTable
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_CONF_OF_ALL_WLAN)) {
			reply = asd_dbus_show_security_wapi_conf_of_all_wlan(connection,message,user_data);
		}
		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY);

			reply = asd_dbus_add_del_security(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIG_SECURITY)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_CONFIG_SECURITY);

			reply = asd_dbus_config_security(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ACCT)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_SET_ACCT);

			reply = asd_dbus_set_acct(connection,message,user_data);
		}	
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_AUTH)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_SET_ACCT);

			reply = asd_dbus_set_wapi_auth(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_MULTI_CERT)) {
			reply = asd_dbus_set_wapi_multi_cert(connection,message,user_data);
		}
		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_PATH)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_SET_ACCT);

			reply = asd_dbus_set_wapi_path(connection,message,user_data);
		}


//weichao	add 20110801	
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_DEL_WAPI_CER)) {
		
			reply = asd_dbus_del_wapi_certification(connection,message,user_data);
		}
		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_P12_CERT_PATH)) {
			reply = asd_dbus_set_wapi_p12_cert_path(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTH)) {
//			asd_printf(ASD_DBUS,MSG_DEBUG,"%s\n",ASD_DBUS_SECURITY_METHOD_SET_AUTH);

			reply = asd_dbus_set_auth(connection,message,user_data);
		}
		/**************************************************************************************************/
		//xm 08/09/02
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AUTH)) {
			reply = asd_dbus_secondary_set_auth(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_ACCT)) {
			reply = asd_dbus_secondary_set_acct(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD)) {
			reply = asd_dbus_eap_reauth_period(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_EAP_SM_RUN_ACTIVATED)){
			reply = asd_dbus_eap_sm_run_activated(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_UCAST_REKEY_METHOD)) {
			reply = asd_dbus_wapi_ucast_rekey_method(connection,message,user_data);		//	xm0701
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_REKEY_PARA)) {
			reply = asd_dbus_wapi_rekey_para(connection,message,user_data);		//	xm0701
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCT_INTERIM_INTERVAL)) {
			reply = asd_dbus_acct_interim_interval(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD)) {
			reply = asd_dbus_set_quiet_period(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_ALIVE_PERIOD)) {
			reply = asd_dbus_eap_alive_period(connection,message,user_data);
		}//weichao add 2011.09.22		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_AUTHORIZE)) {
			reply = asd_dbus_account_after_authorize(connection,message,user_data);
		}//weichao add 2011.12.01
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_DHCP)) {
			reply = asd_dbus_account_after_dhcp(connection,message,user_data);
		}//weichao add 2011.12.01
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_AP_DETECT_INTERVAL)){
			reply = asd_dbus_set_ap_detect_interval(connection,message,user_data);
		}
		//xm 08/09/02
		/**************************************************************************************************/
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_APPLY_WLAN)){
			reply = asd_dbus_apply_wlan(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_HOST_IP))	{
			reply = asd_dbus_set_security_host_ip(connection,message,user_data);
		}//ht add ,081105
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE))	{
			reply = asd_dbus_set_security_type(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE)){
			reply = asd_dbus_set_encryption_type(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_EXTENSIBLE_AUTH)){
			reply = asd_dbus_set_extensible_auth(connection,message,user_data);
		}		
		//weichao add 2011.10.31
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_MAC_AUTH)){
			reply = asd_dbus_set_mac_auth(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_SERVER_SELECT)){
			reply = asd_dbus_radius_server_select(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_WLAN_CHECK)){
			reply = asd_dbus_wlan_check(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_RUNNING_CONFIG)){
			reply = asd_dbus_security_show_running_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_KEY)){
			reply = asd_dbus_set_security_key(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WEP_INDEX_PERIOD)) {
			reply = asd_dbus_wep_index_period(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_PRE_AUTHENTICATION)){
			reply = asd_dbus_set_pre_authentication(connection,message,user_data);
		}		
		/*nl 09/10/10*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_MOBILE_OPEN)){
			reply = asd_dbus_set_wtp_send_response_to_mobile(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_NOTICE_STA_INFO_TO_PROTO_OPEN)){
			reply = asd_dbus_set_notice_sta_info_to_proto(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_NOTICE_STA_INFO_TO_PORTAL_TIMER)){
			reply = asd_dbus_set_notice_sta_info_to_portal_timer(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON)){
			reply = asd_dbus_set_accounting_on(connection,message,user_data);
		}
		//mahz add 2011.10.25
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_DISTRIBUTE_ON)){
			reply = asd_dbus_set_asd_distribute_on(connection,message,user_data);
		}
		//mahz add 2011.10.25
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RDC_PARA)){
			reply = asd_dbus_set_asd_rdc_para(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_ASD_RDC_INFO)) {
			reply = asd_dbus_show_asd_rdc_info(connection,message,user_data);
		}	
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR)){
			reply = asd_dbus_set_radius_extend_attr(connection,message,user_data);
		}

		//mahz add 2010.11.24
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_RADIUS_AUTH)){
           	reply = asd_dbus_set_wapi_radius_auth(connection,message,user_data);
		}
		//mahz add 2010.12.9
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_USER_PASSWD)){
           	reply = asd_dbus_wapi_radius_auth_set_user_passwd(connection,message,user_data);
		}
		//mahz add 2011.2.18
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_HYBRID_AUTH)){
			reply = asd_dbus_set_hybrid_auth(connection,message,user_data);
		}
		//mahz add 2011.3.17
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_ASD_GLOBAL_VARIABLE)){
			reply = asd_dbus_show_asd_global_variable(connection,message,user_data);
		}
		//mahz add 2011.7.8
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_FAST_AUTH)){
			reply = asd_dbus_set_fast_auth(connection,message,user_data);
		}
		
		/***********************************************************************************/
		//xm 08/09/01
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT_ENABLE)){
			reply = asd_dbus_config_port_enable(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT)){
			reply = asd_dbus_config_port(connection,message,user_data);///
		}
		//xm 08/09/01
		/***********************************************************************************/
		
		/***********************************************************************************/
		/***********************************************************************************/
		/*nl add 09/11/02 add for wapi */
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_CERTIFICATE_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_CertificateUpdateCount(connection,message,user_data);///
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_multicastupdatecount(connection,message,user_data);///
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICAST_COUNT_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_unicastupdatecount(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKLIFETIME_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_bklifetime(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKREAUTH_THREASHOLD_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_bkreauththreshold(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_SA_TIMEOUT_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_satimeout(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_CIPHER)){
			reply = asd_dbus_set_wapi_sub_attr_multicast_cipher(connection,message,user_data);
		}
		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_WapiPreauth(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MUTICASEREKEYSTRICT_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_MulticaseRekeyStrict(connection,message,user_data);
				}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICASTCIPHERENABLED_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_UnicastCipherEnabled(connection,message,user_data);
				}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_AUTHENTICATIONSUITEENABLE_UPDATE)){
			reply = asd_dbus_set_wapi_sub_attr_AuthenticationSuiteEnable(connection,message,user_data);
				}
		/*else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_ASD_GET_STA_INFO_ABLE)){
			reply = asd_dbus_set_asd_get_sta_info_able(connection,message,user_data);
		}*/
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_ASD_GET_STA_INFO_TIME)){
			reply = asd_dbus_set_asd_get_sta_info_time(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_ASD_PROCESS_80211N_ABLE)){
			reply = asd_dbus_set_asd_process_80211n_able(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_STA_STATIC_FDB_ABLE)){
			reply = asd_dbus_set_sta_static_fdb_able(connection,message,user_data);
		}
		

		/***********************************************************************************/

		/***********************************************************************************/
		//////////////////////////////////////////////////////////////////////////////
		//sz20080825 
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_SECURITY)){
			reply = asd_dbus_set_port_valn_append_security(connection,message,user_data);///
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_ENABLE)){
			reply = asd_dbus_set_port_valn_append_enable(connection,message,user_data);///ASD_DBUS_SECURITY_METHOD_SET_VLAN_APPEND_SECURITY
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_VLAN_APPEND_SECURITY )){
			reply = asd_dbus_set_valn_append_security(connection,message,user_data);///
		}	
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_VLAN_LIST_APPEND_ENABLE)){
			reply = asd_dbus_set_valn_list_append_enable(connection,message,user_data);///
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG)){
				reply = asd_dbus_set_asd_daemonlog_debug(connection,message,user_data);//
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_ASD_LOGGER_PRINTFLAG)){
				reply = asd_dbus_set_asd_logger_printflag(connection,message,user_data);//
		}  	
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_LEVEL)){
				reply = asd_dbus_set_asd_daemonlog_level(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_LOG_GROUP_ACTIVATED)){
				reply = set_asd_log_group_activated(connection,message,user_data);//qiuchen add it for hn_mobile 
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AC_MANAGEMENT_IP)){
			reply = set_ac_management_ip(connection,message,user_data);//qiuchen add it for hn_mobile 
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_AC_MANAGEMENT_IP)){
			reply = show_ac_management_ip(connection,message,user_data);//qiuchen add it for hn_mobile 
		}
		//sz20080825 
		/////////////////////////////////////////////////////////////////////////////
		else if(dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT)){
			reply = asd_dbus_interface_update_wtp_count(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE, ASD_DBUS_SECURITY_METHOD_TRAP_OPEN)){
			reply = asd_dbus_interface_set_trap_flag(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_TRAP_ABLE)){
			reply = asd_dbus_set_asd_trap_able(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SHOW_TRAP_STATE)){
			reply = asd_dbus_show_asd_trap_state(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SHOW_DBUS_COUNT)){
			reply = asd_dbus_show_asd_dbus_count(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_DBUS_COUNT)){
			reply = asd_dbus_set_asd_dbus_count(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WPA_GROUP_REKEY_PERIOD)) {
			reply = asd_dbus_wpa_group_rekey_period(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WPA_KEYUPDATE_TIMEOUT_PERIOD)) {
			reply = asd_dbus_wpa_keyupdate_timeout_period(connection,message,user_data);
		}
		//mahz add 2011.1.3
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_WPA_ONCE_GROUP_REKEY_TIME)) {
			reply = asd_dbus_wpa_once_group_rekey_time(connection,message,user_data);
		}
		//mahz add 2011.10.17
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ASD_SOCKET_OPERATE)) {
			reply = asd_dbus_set_asd_sock_oper(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_TRAFFIC_LIMIT_FROM_RADIUS)) {
			reply = asd_dbus_traffic_limit_from_radius(connection,message,user_data);
		}
		//qiuchen add it for master_bak radius server 2012.12.11
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AC_RADIUS_NAME)){
			reply = asd_dbus_set_ac_radius_name(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_RES_FAIL_SUC_PERCENT)){
			reply = asd_dbus_set_radius_res_fail_suc_percent(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_ACCESS_TEST_INTERVAL)){
			reply = asd_dbus_set_radius_access_test_interval(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_CHANGE_REUSE_TEST_TIMER)){
			reply = asd_dbus_set_radius_server_change_reuse_test_timer(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_HEART_TEST_TYPE)){
			reply = asd_dbus_set_radius_server_heart_test_type(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_BINDING_ENABLE_DISABLE)){
			reply = asd_dbus_set_radius_server_binding_enable_disable(connection,message,user_data);
		}	
		else if (dbus_message_is_method_call(message,ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_HEART_TEST_ON_OFF)){
			reply = asd_dbus_set_radius_heart_test_on_off(connection,message,user_data);
		}
		//end
	}
	else if(strcmp(dbus_message_get_path(message),ASD_DBUS_AC_GROUP_OBJPATH) == 0){
		
		if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_CREATE_GROUP)) {
			reply = asd_dbus_create_ac_group(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_CONFIG)) {
			reply = asd_dbus_config_ac_group(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_ADD_GROUP_MEMBER)) {
			reply = asd_dbus_add_ac_group_member(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_HOST_IP)) {
			reply = asd_dbus_set_host_ip(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_DELETE_GROUP)) {
			reply = asd_dbus_delete_ac_group(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_DEL_GROUP_MEMBER)) {
			reply = asd_dbus_del_ac_group_member(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_LIST)) {
			reply = asd_dbus_show_ac_group_list(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP)) {
			reply = asd_dbus_show_ac_group(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_ROAMING_COUNT)) {
			reply = asd_dbus_count_ac_roaming(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,ASD_DBUS_AC_GROUP_INTERFACE,ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_RUNNING_CONFIG)) {
			reply = asd_dbus_show_ac_group_running_config(connection,message,user_data);
		}		
	}else
		asd_printf(ASD_DBUS,MSG_DEBUG,"path wrong\n");
	
	if (reply) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"reply destination %s\n",dbus_message_get_destination(reply));	
		memset(sender, 0, 20);
		strcpy(sender,dbus_message_get_destination(reply));
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); // TODO    Maybe we should let main loop process the flush
		dbus_message_unref (reply);
		if(dbus_count_switch)
			asd_static_dbus_op(sender);
	}

//	dbus_message_unref(message); //TODO who should unref the incoming message? 
	return DBUS_HANDLER_RESULT_HANDLED ;
}


/** Message handler for Signals
 *  or normally there should be no signals except dbus-daemon related.
 *
 *  @param  connection          D-BUS connection
 *  @param  message             Message
 *  @param  user_data           User data
 *  @return                     What to do with the message
 */
DBusHandlerResult
asd_dbus_filter_function (DBusConnection * connection,
					   DBusMessage * message, void *user_data)
{
//	printf("entering filter.\n");
	pthread_t ASD_DBUS_T;
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */

		asd_printf(ASD_DBUS,MSG_DEBUG,"Got disconnected from the system message bus; "
				"retrying to reconnect every 3000 ms\n");
		//dbus_connection_close (asd_dbus_connection);
		asd_dbus_connection = NULL;
		if(pthread_create(&ASD_DBUS_T, NULL, asd_dbus_restart_thread, NULL) != 0) {
			asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
			return -1;
		}

		//g_timeout_add (3000, reinit_dbus, NULL);

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

		//if (services_with_locks != NULL)  service_deleted (message);
	} else
		return TRUE;
		//return hald_dbus_filter_handle_methods (connection, message, user_data, FALSE);

	return DBUS_HANDLER_RESULT_HANDLED;
}


int asd_dbus_init(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	DBusObjectPathVTable	asd_vtable = {NULL, &asd_dbus_message_handler, NULL, NULL, NULL, NULL};	

	asd_printf(ASD_DBUS,MSG_DEBUG,"asd dbus init\n");

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	asd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	asd_dbus_connection2 = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	asd_dbus_connection3 = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	asd_dbus_connection4 = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);
	if (asd_dbus_connection == NULL) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	// Use npd to handle subsection of NPD_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (asd_dbus_connection, ASD_DBUS_OBJPATH, &asd_vtable, NULL)) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	
	i = dbus_bus_request_name (asd_dbus_connection, ASD_DBUS_BUSNAME,
			       0, &dbus_error);	
	i = dbus_bus_request_name (asd_dbus_connection2, "aw.asdvrrp",
			       0, &dbus_error);
	i = dbus_bus_request_name (asd_dbus_connection3, "aw.asdreq",
			       0, &dbus_error);
	i = dbus_bus_request_name (asd_dbus_connection4, "aw.asdtrap",
				0,&dbus_error);
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd_dbus_connection %s\n",dbus_bus_get_unique_name(asd_dbus_connection));	
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd_dbus_connection2 %s\n",dbus_bus_get_unique_name(asd_dbus_connection2));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd_dbus_connection3:%s\n",dbus_bus_get_unique_name(asd_dbus_connection3));
	asd_printf(ASD_DBUS,MSG_DEBUG,"asd_dbus_connection4:%s\n",dbus_bus_get_unique_name(asd_dbus_connection4));
	
	if (dbus_error_is_set (&dbus_error)) {
		asd_printf(ASD_DBUS,MSG_DEBUG,"dbus_bus_request_name(): %s\n",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (asd_dbus_connection, asd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (asd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
//	printf("init finished\n");
	return TRUE;
  
}

int asd_dbus_reinit(void)
{	
	int i = 0;
	DBusError dbus_error;
	dbus_threads_init_default();
	DBusObjectPathVTable	asd_vtable = {NULL, &asd_dbus_message_handler, NULL, NULL, NULL, NULL};	

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd dbus init\n");

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	asd_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);

	if (asd_dbus_connection == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"dbus_bus_get(): %s\n", dbus_error.message);
		return FALSE;
	}

	// Use npd to handle subsection of NPD_DBUS_OBJPATH including slots
	if (!dbus_connection_register_fallback (asd_dbus_connection, ASD_DBUS_OBJPATH, &asd_vtable, NULL)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"can't register D-BUS handlers (fallback NPD). cannot continue.\n");
		return FALSE;
		
	}
	
	
	i = dbus_bus_request_name (asd_dbus_connection, ASD_DBUS_BUSNAME,
			       0, &dbus_error);	

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"dbus_bus_request_name:%d\n",i);
	
	if (dbus_error_is_set (&dbus_error)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"dbus_bus_request_name(): %s\n",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (asd_dbus_connection, asd_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (asd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
	
//	printf("init finished\n");
	return TRUE;
  
}

 
void *asd_dbus_thread()
{
	//int loop_count = 0;
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/
	asd_printf(ASD_DBUS,MSG_DEBUG,"begin\n");
	de_wtp_deny_sta = (unsigned char*)os_zalloc(WTP_NUM*sizeof(unsigned char));
	if(de_wtp_deny_sta==NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"de_wtp_deny_sta.\n");
	}
	if(asd_dbus_init()){
		while (dbus_connection_read_write_dispatch(asd_dbus_connection,500)) {
//			printf("entering main loop.\n");
//			printf("loop %d.\n",loop_count++);
		}
	}
	asd_printf(ASD_DBUS,MSG_DEBUG,"there is something wrong in dbus handler\n");
	return NULL;	
}
#ifdef __cplusplus
}
#endif

void *asd_dbus_restart_thread()
{
	//int loop_count = 0;
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_dbus_restart_thread begin\n");
	if(asd_dbus_reinit()){
		while (dbus_connection_read_write_dispatch(asd_dbus_connection,500)) {
//			printf("entering main loop.\n");
//			printf("loop %d.\n",loop_count++);
		}
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"there is something wrong in dbus handler\n");
	return NULL;
}


int notice_hmd_update_state_change(unsigned int vrrid,unsigned int state)
{	
	int ret;
	DBusMessage *query, *reply;
	DBusError err;
	
	asd_printf(ASD_DEFAULT,MSG_INFO,"%s,%d.\n",__func__,__LINE__);
	query = dbus_message_new_method_call(
				                         HMD_DBUS_BUSNAME,
				                         HMD_DBUS_OBJPATH,
										 HMD_DBUS_INTERFACE,
										 HMD_DBUS_METHOD_ASD_NOTIFY_HMD_UPDATE_STATE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&local,
							 DBUS_TYPE_UINT32,&vrrid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (asd_dbus_connection2,query,150000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		asd_printf(ASD_DEFAULT,MSG_ERROR,"%s,%d.dbus get fail from hmd.\n",__func__,__LINE__);
		return -1;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
		}
	}
	dbus_message_unref(reply);
	asd_printf(ASD_DEFAULT,MSG_INFO,"%s,%d.notice hmd succefull.\n",__func__,__LINE__);
	return 0;
}


void notice_vrrp_state_change(unsigned int vrrid,unsigned int state)
{	
	int ret;
	//unsigned char wlan_id,status,wlanid;
    /*wlan_id = atoi(argv[0]);*/
	DBusMessage *query, *reply;
	DBusError err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	memset(BUSNAME, 0, PATH_LEN);
	memset(OBJPATH, 0, PATH_LEN);	
	sprintf(BUSNAME,"%s%d",VRRP_DBUS_BUSNAME,vrrid);
	sprintf(OBJPATH,"%s%d",VRRP_DBUS_OBJPATH,vrrid);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_TRANSFER_STATE );
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&vrrid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (asd_dbus_connection2,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			asd_printf(ASD_DBUS,MSG_DEBUG,"dbus notice vrrp successfull");
		}
	}
	dbus_message_unref(reply);
	asd_printf(ASD_DEFAULT,MSG_INFO,"%s,%d.notice had succefull(vrrid:%d,state:%d.).\n",__func__,__LINE__,vrrid,state);
	return ;
}





